/* 
Copyright (c) 2011, SecurityFree 
All rights reserved. 
 
FileName:  
        mempool.c 
Summary:   
        Work as mempool with four external interfaces.
	a).mempool_create
	it create a pool of mem. it may include several slab, and many node buffer in each slab.
	b).mempool_malloc
	it provide a node buffer from mempool, each time.	
	c).mempool_free
	free memory used by node buffer, and recreate the datastruct of mempool.
	d).mempool_destroy
	free all memory of the mempool.	
Version: 1.0 
Date:    2011-9-22 
Author:  simon_xiyao 
Email:   yaoxibiti@gmail.com
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mempool.h" 


//slab
#define SLAB_TYPE_STABLE        0
#define SLAB_TYPE_VARIABLE      1

//handle
#define MAX_MEMPOOL_HD_NUM      128
#define MEMPOOL_HD_USED         1
#define MEMPOOL_HD_UNUSED       0

//node
#define NODE_USED               0x01
#define NODE_NOT_USED           0x02
#define MEMPOOL_NODE_MAGIC      0x1a2b3c4e


//global variable
static int g_mempool_hd[MAX_MEMPOOL_HD_NUM] = {MEMPOOL_HD_UNUSED};
mempool_entry_t *g_mempool_entry[MAX_MEMPOOL_HD_NUM] = {NULL};
//new add
static inline void __lock_entry(mempool_entry_t *entry)
{
#ifdef _REENTRANT
	pthread_mutex_lock(&entry->e_mutex);
#endif
}

static inline void __unlock_entry(mempool_entry_t *entry)
{
#ifdef _REENTRANT	
	pthread_mutex_unlock(&entry->e_mutex);
#endif
}


mempool_hd_t get_free_hd()
{
	int32_t i = 0;
	mempool_hd_t hd = MEMPOOL_HD_INVALID;

	for (i = 0; i < MAX_MEMPOOL_HD_NUM; i++) {
		if (g_mempool_hd[i] == MEMPOOL_HD_UNUSED) {
			hd = (mempool_hd_t)i;
			g_mempool_hd[i] = MEMPOOL_HD_USED;
			break;
		}
	}

	return hd;
}

void put_free_hd(mempool_hd_t hd)
{
	if (0 > hd || hd >= MAX_MEMPOOL_HD_NUM) {
		return;
	}

	g_mempool_hd[hd] = MEMPOOL_HD_UNUSED;

	return;
}

void free_entry(mempool_entry_t **entry)
{
	if (NULL != *entry) {
		free(*entry);
		*entry = NULL;
	}

	return;
}

void free_slab(mempool_slab_t **slab)
{
	if (NULL != *slab) {
		free(*slab);
		*slab = NULL;
	}

	return;
}

void free_nodes_buffer(mempool_slab_t *slab)
{
	assert(slab);
	
	if (NULL != slab->node_buffer) {
		free(slab->node_buffer);	
		slab->node_buffer = NULL;
	}

	return;
}

void init_nodes_buffer(mempool_slab_t *slab)
{
	char *ptr = NULL;
	int32_t i = 0;
	mempool_node_t *node = NULL;

	assert(slab);

	ptr = slab->node_buffer;
	for (i = 0; i < slab->entry->node_count; i++) {
		node = (mempool_node_t *)ptr;
		node->magic = MEMPOOL_NODE_MAGIC; 
		node->node_status = NODE_NOT_USED;
		node->slab = slab;
		listhead_init(&(node->unused_list));
		listhead_add(&(node->unused_list), &(slab->unused_list));
		ptr += sizeof(mempool_node_t) + slab->entry->node_size;
	}

	return;
}

mempool_slab_t *slab_nodes_malloc(mempool_entry_t *entry)
{
	mempool_slab_t *new_slab = NULL;

	assert(entry);	

	if (entry->slab_count >= entry->max_slab_count) {
		printf("slab_count beyond the limited count, @%s,%d\n", __FUNCTION__, __LINE__);
		goto out;
	}

	new_slab = (mempool_slab_t *)malloc(sizeof(mempool_slab_t));
	if (NULL == new_slab) {
		printf("err malloc, @%s,%d\n", __FUNCTION__, __LINE__);
		goto out;
	}
	
	if (entry->slab_count == 0) {
		new_slab->slab_type = SLAB_TYPE_STABLE;
	} else {
		new_slab->slab_type = SLAB_TYPE_VARIABLE;
	}

	new_slab->used_node_count = 0;
	new_slab->entry = entry;
	new_slab->node_buffer = malloc((sizeof(mempool_node_t)+entry->node_size)*entry->node_count);
	if (NULL == new_slab->node_buffer) {
		free_slab(&new_slab);	
		printf("err malloc, @%s,%d\n", __FUNCTION__, __LINE__);
		goto out;
	}
	
	listhead_init(&(new_slab->unused_list));
	init_nodes_buffer(new_slab);

	listhead_init(&(new_slab->notfull_slab));
	listhead_init(&(new_slab->full_slab));
	listhead_add(&(new_slab->notfull_slab), &(entry->notfull_slab));
	entry->slab_count++;
out:
	return new_slab;
}

mempool_hd_t mempool_create(int32_t node_size, int32_t node_num, int32_t max_slab_num)
{
	mempool_hd_t hd = MEMPOOL_HD_INVALID;  
	mempool_entry_t *entry = NULL;
	mempool_slab_t *slab = NULL;

	if (node_size <= 0 || node_num <= 0 || max_slab_num <= 0) {
		printf("parameter err: %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}

	hd = get_free_hd();	
	if (MEMPOOL_HD_INVALID == hd) {
		printf("not get valid handle: %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}
	
	entry = (mempool_entry_t *)malloc(sizeof(mempool_entry_t));	
	if (NULL == entry) {
		put_free_hd(hd);
		hd = MEMPOOL_HD_INVALID;
		printf("err @ malloc: %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}

	entry->node_size = node_size;
	entry->node_count = node_num;
	entry->slab_count = 0;
	entry->max_slab_count = max_slab_num;
//new add
#ifdef _REENTRANT
    	pthread_mutex_init(&entry->e_mutex, NULL);
#endif 	
	listhead_init(&(entry->notfull_slab));
	listhead_init(&(entry->full_slab));
	
	slab = slab_nodes_malloc(entry);
	if (NULL == slab) {
		printf("err of slab_nodes_malloc: %s, %d\n", __FUNCTION__, __LINE__);
		free_entry(&entry);
		put_free_hd(hd);
		hd = MEMPOOL_HD_INVALID;
		goto out;
	}
	
	
	g_mempool_entry[hd] = entry;
	
out:
	return hd;
}

void mempool_destroy(mempool_hd_t hd)
{
	mempool_entry_t *entry = NULL;
	mempool_slab_t *slab = NULL;
	mempool_slab_t *prev = NULL;
	int32_t i = 0;

	if (hd < 0 || hd >= MAX_MEMPOOL_HD_NUM) {
		printf("err @@ %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}

	if (g_mempool_hd[hd] == MEMPOOL_HD_UNUSED) {
		printf("err @@ %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}
	
	entry = g_mempool_entry[hd];
	if (NULL == entry) {
		printf("entry=null @@ %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}
	
	put_free_hd(hd);

	slab = container((&(entry->notfull_slab))->next, mempool_slab_t, notfull_slab);
	while (&(slab->notfull_slab) != &(entry->notfull_slab)) {
		prev = slab;	
		slab = container((&(prev->notfull_slab))->next, mempool_slab_t, notfull_slab);
		free_nodes_buffer(prev);
		free_slab(&prev);
		i++;
	}

	slab = container((&(entry->full_slab))->next, mempool_slab_t, full_slab);
	while (&(slab->full_slab) != &(entry->full_slab)) {
		prev = slab;
		slab = container((&(prev->full_slab))->next, mempool_slab_t, full_slab);
		free_nodes_buffer(prev);
		free_slab(&prev);
		i++;
	}

	if (i != entry->slab_count) {
		//something must wrong
		printf("free slab num not equal entry->slab_count. @ %s, %d\n", __FUNCTION__, __LINE__);
	}

	free_entry(&entry);
out:
	return;
}

void *mempool_malloc(mempool_hd_t hd)
{
	void *buff = NULL;
	mempool_entry_t *entry = NULL;
	mempool_slab_t *slab = NULL;
	mempool_node_t *node = NULL;

	if (hd < 0 || hd >= MAX_MEMPOOL_HD_NUM) {
		printf("handle err@ %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}
	
	if (g_mempool_hd[hd] == MEMPOOL_HD_UNUSED) {
		printf("handle unused@ %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}

	entry = g_mempool_entry[hd]; 
	if (NULL == entry) {
		printf("NULL == entry@ %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}
	__lock_entry(entry);
	if (1 == listhead_empty(&(entry->notfull_slab))) { 
		slab = slab_nodes_malloc(entry);			
		if (NULL == slab) {
			printf("null of slab_nodes_malloc @ %s, %d\n", __FUNCTION__, __LINE__);
			goto out;
		}
	} else {
		slab = container((&(entry->notfull_slab))->next, mempool_slab_t, notfull_slab);
		if (NULL == slab) {
			printf("null == slab @ %s, %d\n", __FUNCTION__, __LINE__);
			goto out;
		}
		if (1 == listhead_empty(&(slab->unused_list))) {
			printf("slab->unused_list is empty @ %s, %d\n", __FUNCTION__, __LINE__);
			goto out;
		}
	}
	
	node = container((&(slab->unused_list))->next, mempool_node_t, unused_list);
	assert(node);
	if (node->magic != MEMPOOL_NODE_MAGIC) {
		printf("err of node magic check  @ %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}
	if (node->node_status != NODE_NOT_USED) {
		printf(" @ %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}

	node->node_status = NODE_USED;
	listhead_del(&(node->unused_list));
	slab->used_node_count++;
	if (slab->used_node_count == entry->node_count) {
		listhead_del(&(slab->notfull_slab));
		listhead_add(&(slab->full_slab), &(entry->full_slab));
	}
	buff = (void *)(node->mem);
out:
	__unlock_entry(entry);
	return buff;
}

void mempool_free(void *mem)
{
	mempool_entry_t *entry = NULL;
	mempool_slab_t *slab = NULL;
	mempool_node_t *node;
	
	if (NULL == mem) {
		printf("address of ptr is null, @ %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}

	node = container(mem, mempool_node_t, mem); 
	assert(node);
	slab = node->slab;
	assert(slab);
	entry = slab->entry; 
	assert(entry);
//new add
	__lock_entry(entry);
	if (node->magic != MEMPOOL_NODE_MAGIC) {
		printf("err of magic check, @ %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}

	if (node->node_status == NODE_NOT_USED) {
		printf("node is not used, @ %s, %d\n", __FUNCTION__, __LINE__);
		goto out;
	}
	
	if (slab->used_node_count == entry->node_count) {
		listhead_del(&(slab->full_slab));
		listhead_add(&(slab->notfull_slab), &(entry->notfull_slab));
	}
	node->node_status = NODE_NOT_USED;
	listhead_add(&(node->unused_list), &(slab->unused_list));
	slab->used_node_count--;

	if (0 == slab->used_node_count) {
		if (SLAB_TYPE_STABLE == slab->slab_type) {
		} else {
			listhead_del(&(slab->notfull_slab));
			free_nodes_buffer(slab);
			free_slab(&slab);
			
			entry->slab_count--;
		}
	}
out:
	__unlock_entry(entry);
	return;
}

#if 0
typedef struct _little_t {
	char data[100];
}little_t;

typedef struct _mid_t {
	char data[200];
}mid_t;

int32_t main(void)	
{
	mempool_hd_t little_hd;	
	little_t *a = NULL;
	mid_t *b = NULL;

	little_hd = mempool_create(sizeof(little_t), 10, 100);
	if (MEMPOOL_HD_INVALID == little_hd) {
		return -1;
	}

	a = (little_t *)mempool_malloc(little_hd);	
	if (NULL == a) {
		printf("err malloc\n");
	}

	snprintf(a->data, 100, "abcdefgegegegegege_little");
	printf("a->data=%s\n", a->data);

	mempool_free((void *)a);
	mempool_destroy(little_hd);

	return 0;
}
#endif
