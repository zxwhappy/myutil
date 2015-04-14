/*
 * 
 *	This is free software. You can redistribute it and/or modify under
 *	the terms of the GNU General Public License version 2.
 *
 * 	Copyright (C) 1998 by kra
 * 
 */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "list_n.h"

void list_init(struct list *l, int next_offset, int prev_offset)
{
	l->l_first = l->l_last = NULL;
	l->l_noff = next_offset;
	l->l_poff = prev_offset;
	l->l_iter = NULL;
	l->l_produce_done = 0;
#ifdef _REENTRANT
	l->l_locked = 0;
	pthread_mutex_init(&l->l_mutex, NULL);
	pthread_cond_init(&l->l_notempty, NULL);
	l->l_locked_thr = (pthread_t) 0;
#endif
}

static inline void __lock(struct list *l)
{
#ifdef _REENTRANT
	if (!l->l_locked || l->l_locked_thr != pthread_self())
		pthread_mutex_lock(&l->l_mutex);
#endif
}

static inline void __unlock(struct list *l)
{
#ifdef _REENTRANT	
	if (!l->l_locked || l->l_locked_thr != pthread_self())
		pthread_mutex_unlock(&l->l_mutex);
#endif
}

void list_flush(struct list *l)
{
	struct list_iterator *i;
	
	__lock(l);
	l->l_first = l->l_last = NULL;
	for (i = l->l_iter; i; i = i->i_next) {
		i->i_cur = NULL;
	}
	__unlock(l);
}

static inline void __update_iterators(struct list *l, void *old_item, void *new_item)
{
	struct list_iterator *i;
	
	for (i = l->l_iter; i; i = i->i_next) {
		if (i->i_cur == old_item)
			i->i_cur = new_item;
	}
}

void list_push(struct list *l, void *m)
{
	__lock(l);
	*LIST_PREV_PTR(l, m) = NULL;		/* m->prev = NULL */
	*LIST_NEXT_PTR(l, m) = NULL;		/* m->next = NULL */
	if (!(*LIST_NEXT_PTR(l, m) = l->l_first))
		l->l_last = m;
	else
		*LIST_PREV_PTR(l, l->l_first) = m;

	__update_iterators(l, l->l_first, m);
	l->l_first = m;
	__unlock(l);
}

static inline void __enqueue(struct list *l, void *m)
{
	*LIST_PREV_PTR(l, m) = NULL;		/* m->prev = NULL */
	*LIST_NEXT_PTR(l, m) = NULL;		/* m->next = NULL */
	if (l->l_last) {
		*LIST_NEXT_PTR(l, l->l_last) = m; /* l->l_last->next = m */
		*LIST_PREV_PTR(l, m) = l->l_last; /* m->prev = l->l_last */
	} else
		l->l_first = m;
	l->l_last = m;
}

void list_insert_at(struct list *l, int nr, void *m)
{
	void *pp;
	
	__lock(l);
	
	*LIST_NEXT_PTR(l, m) = NULL;
	*LIST_PREV_PTR(l, m) = NULL;

	if (!l->l_first) { /* insert at NULL list */
		l->l_first = l->l_last = m;
	} else {
		pp = l->l_first;
		while (pp && nr--) {
			pp = *LIST_NEXT_PTR(l, pp);
		}
		
		if (pp == NULL) { 		/* insert at list tail */
			*LIST_PREV_PTR(l, m) = l->l_last; 
			*LIST_NEXT_PTR(l, l->l_last) = m;
			l->l_last = m;
		} else if (pp == l->l_first) { 	/* insert at list head */
			*LIST_NEXT_PTR(l, m) = l->l_first;
			*LIST_PREV_PTR(l, l->l_first) = m;
			l->l_first = m;
		} else { 			/* insert at middle */
			void *right = pp;
			void *left = *LIST_PREV_PTR(l, pp);

			*LIST_NEXT_PTR(l, m) = right; 
			*LIST_PREV_PTR(l, m) = left;

			*LIST_NEXT_PTR(l, left) = m;
			*LIST_PREV_PTR(l, right) = m;
		}
	}
	
//	p = &l->l_first;
//	while (*p && nr--) {
//		p = &(*LIST_NEXT_PTR(l, *p));
//	}
//	
//	*LIST_NEXT_PTR(l, m) = *p;
//	if (*p)
//		*p = m;
//	else
//		__enqueue(l, m);
	__update_iterators(l, *LIST_NEXT_PTR(l, m), m);
	__unlock(l);
}

void list_enqueue(struct list *l, void *m)
{
	__lock(l);
	__enqueue(l, m);
	__unlock(l);
}

void list_produce(struct list *l, void *m)
{
	__lock(l);
	__enqueue(l, m);
#ifdef _REENTRANT
	pthread_cond_signal(&l->l_notempty);
#endif
	__unlock(l);
}

void list_produce_start(struct list *l)
{
	__lock(l);
	l->l_produce_done = 0;
	__unlock(l);
}

void list_produce_done(struct list *l)
{
	__lock(l);
	l->l_produce_done = 1;
#ifdef _REENTRANT
	pthread_cond_signal(&l->l_notempty);
#endif
	__unlock(l);
}

static inline void *__pop(struct list *l)
{
	void *retval;
	
	if ((retval = l->l_first)) {
		if (!(l->l_first = *LIST_NEXT_PTR(l, retval)))
			l->l_last = NULL;
		else
			*LIST_PREV_PTR(l, l->l_first) = NULL;

		__update_iterators(l, retval, l->l_first);
	}
	return retval;
}

void *list_pop(struct list *l)
{
	void *retval;

	__lock(l);
	retval = __pop(l);
	__unlock(l);
	return retval;
}

void *list_pop_back(struct list *l)
{
	void *retval;

	__lock(l);

	if ((retval = l->l_last)) {
		if (!(l->l_last = *LIST_PREV_PTR(l, retval)))
			l->l_first = NULL;
		else
			*LIST_NEXT_PTR(l, l->l_last) = NULL;

		__update_iterators(l, retval, l->l_last);
	}
	
	__unlock(l);
	return retval;
}

static void *__list_consume(struct list *l, const struct timespec *absts)
{
	int ret = 0;
	void *retval;
	struct timespec	ts;

	__lock(l);
#ifdef _REENTRANT
	while (!l->l_first && !l->l_produce_done)
		if (absts) {
			if ((ret = pthread_cond_timedwait(&l->l_notempty,
			    &l->l_mutex, absts)) == ETIMEDOUT || ret == EINTR)
				break;
		} else {
#if 0
			pthread_cond_wait(&l->l_notempty, &l->l_mutex);
#else
			/*
			 * it can be interrupted through signal
			 */
			ts.tv_sec = 2000000000;
			ts.tv_nsec = 0;
			ret = pthread_cond_timedwait(&l->l_notempty,
				&l->l_mutex, &ts);
			if (ret == ETIMEDOUT || ret == EINTR)
				break;
#endif
		}
#endif
	retval = __pop(l);
	__unlock(l);
	return retval;
}

void *list_consume(struct list *l, const struct timespec *absts)
{
	return __list_consume(l, absts);
}

void *list_consume_rel(struct list *l, const struct timespec *relts)
{
	struct timeval now;
	struct timespec absts;
	
	gettimeofday(&now, NULL);
	absts.tv_sec = now.tv_sec + relts->tv_sec;
	absts.tv_nsec = now.tv_usec * 1000 + relts->tv_nsec;
	if (absts.tv_nsec >= 1000000000) {
		absts.tv_nsec -= 1000000000;
		absts.tv_sec++;
	}
	return __list_consume(l, &absts);
}
	
void *list_peek(struct list *l)
{
	void *retval;

	__lock(l);
	retval = l->l_first;
	__unlock(l);
	return retval;
}

void *list_at(struct list *l, int nr)
{
	void *retval, *p;
	int i;
	
	__lock(l);
	for (p = l->l_first, i = 0; p && i < nr; p = *LIST_NEXT_PTR(l, p), i++)
		;
	if (p)
		retval = p;
	else
		retval = NULL;
	__unlock(l);
	return retval;
}

static inline int __func_remove(int nr, void *p, void *m)
{
	if (p == m)
		return 1;
	else
		return 0;
}

static inline int __func_remove_at(int nr, void *p, void *m)
{
	if (nr == (int) m)
		return 1;
	else
		return 0;
}

static inline void *__list_remove(struct list *l, 
				  int (*func)(int nr, void *, void *m), void *m)
{
	void *retval = NULL;
//	void **p;
	void *pp;
	int nr;
	
	nr = 0;
	__lock(l);
	pp = l->l_first;
	while (pp) {
		if (func(nr, pp, m))
			break;

		pp = *LIST_NEXT_PTR(l, pp);
		nr++;
	}
	if (pp) {
		void *left = *LIST_PREV_PTR(l, pp);
		void *right = *LIST_NEXT_PTR(l, pp);
		
		/* remove this node */
		if (left)
			*LIST_NEXT_PTR(l, left) = right;
		else 
			l->l_first = right;
		if (right)
			*LIST_PREV_PTR(l, right) = left;
		else 
			l->l_last = left;
		
		*LIST_NEXT_PTR(l, pp) = NULL;
		*LIST_PREV_PTR(l, pp) = NULL;

		__update_iterators(l, pp, right);

		retval = pp;
	}

	__unlock(l);
	return retval;

//	p = &l->l_first;
//	while (*p) {
//		if (func(nr, *p, m)) { /*  if (*p == member)  */
//			retval = *p;
//			
//			/*  *p = (*p)->next */
//			if (!(*p = *LIST_NEXT_PTR(l, *p))) {
//				if (!l->l_first)
//					l->l_last = NULL;
//				else
//					l->l_last = LIST_THIS_PTR(l, p);
//			}
//			__update_iterators(l, retval, *p);
//			__unlock(l);
//			return retval;
//		}
//		p = &(*LIST_NEXT_PTR(l, *p));	/* p = &(*p)->next */
//		nr++;
//	}
//	__unlock(l);
//	return *p;	/* NULL */
}

//void *list_remove(struct list *l, void *m)
//{
//	return __list_remove(l, __func_remove, m);
//}

void *list_remove_at(struct list *l, int nr)
{
	return __list_remove(l, __func_remove_at, (void *) nr);
}

void *list_remove_func(struct list *l, 
		       int (*func)(int nr, void *, void *m), void *m)
{
	return __list_remove(l, func, m);
}

void *list_remove(struct list *l, void *m)
{
	void *retval = NULL;
	
	__lock(l);
	if (m) {
		void *left = *LIST_PREV_PTR(l, m);
		void *right = *LIST_NEXT_PTR(l, m);
		
		/* remove this node */
		if (left)
			*LIST_NEXT_PTR(l, left) = right;
		else 
			l->l_first = right;
		if (right)
			*LIST_PREV_PTR(l, right) = left;
		else 
			l->l_last = left;
		
		*LIST_NEXT_PTR(l, m) = NULL;
		*LIST_PREV_PTR(l, m) = NULL;

		__update_iterators(l, m, right);

		retval = m;
	}

	__unlock(l);
	return retval;
}

int list_count(struct list *l)
{
	int i;
	void *p;

	__lock(l);
	for (i = 0, p = l->l_first; p; i++, p = *LIST_NEXT_PTR(l, p)) ; /* p = p->next */
	__unlock(l);
	return i;
}

void list_lock(struct list *l)
{
#ifdef _REENTRANT
	if (!l->l_locked || l->l_locked_thr != pthread_self()) {
		pthread_mutex_lock(&l->l_mutex);
		l->l_locked_thr = pthread_self();
		l->l_locked = 1;
	} else
		l->l_locked++;
#endif
}

void list_unlock(struct list *l)
{
#ifdef _REENTRANT
	if (--l->l_locked == 0)
		pthread_mutex_unlock(&l->l_mutex);
#endif
}

/*
 * list_iter
 */
void list_iter_set(struct list_iterator *i, struct list *l)
{
	__lock(l);
	i->i_list = l;
	i->i_cur = l->l_first;
	i->i_next = l->l_iter;
	l->l_iter = i;
	__unlock(l);
}

void list_iter_end(struct list_iterator *i)
{
	struct list_iterator **p;
	struct list *l;
	
	l = i->i_list;
	__lock(l);
	p = &l->l_iter;
	while (*p) {
		if (*p == i) {
			*p = i->i_next;
			break;
		}
		p = &(*p)->i_next;
	}
	__unlock(l);
	i->i_cur = NULL;
	i->i_next = NULL;
	i->i_list = NULL;
}

void *list_iter_get(struct list_iterator *i)
{
	void *retval;
	struct list *l;
	
	l = i->i_list;
	__lock(l);
	retval = i->i_cur;
	if (retval)
		i->i_cur = *LIST_NEXT_PTR(l, retval);
	__unlock(l);
	return retval;
}
