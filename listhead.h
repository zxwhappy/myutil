#ifndef _LIST_HEAD_H
#define _LIST_HEAD_H

#define offset(type, member) ((size_t)&(((type *)0)->member))

#define container(ptr, type, member) ({const typeof(((type *)0)->member)  *_ptr = ptr;\
	(type *)((char *)_ptr - offset(type,member));})


struct listhead {
	struct listhead *prev;
	struct listhead *next;
};



static inline void  listhead_init(struct listhead *list)
{
	list->prev = list;
	list->next = list;
}

static inline void _listhead_add(struct listhead *newnode, struct listhead *prev, struct listhead *cur)
{
	newnode->next = cur;
	cur->prev = newnode;
	
	newnode->prev = prev;
	prev->next = newnode;
}

static inline void listhead_add(struct listhead *newnode, struct listhead *cur)
{
	assert(newnode);
	assert(cur);
	_listhead_add(newnode, cur->prev, cur);
}

static inline void _listhead_del(struct listhead *node_prev, struct listhead *node_next)
{
	node_prev->next = node_next;	
	node_next->prev = node_prev;
}

static inline void listhead_del(struct listhead *node)
{
	assert(node);
	_listhead_del(node->prev, node->next);
}

static inline int listhead_empty(struct listhead *list)
{
	if (list->next == list || list->prev == list) {
		return 1;
	} else {
		return 0;
	}
}

#endif
