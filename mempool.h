#ifndef _MEMPOOL_HEAD_H
#define _MEMPOOL_HEAD_H
#include "listhead.h"
//new add
#ifdef _REENTRANT
#include <pthread.h>
#endif

#define MAX_MEMPOOL_SLAB_NUM        (1024 * 1024 * 1024)

typedef struct _mempool_entry_t {
	int32_t node_size;
	int32_t node_count;
	int32_t slab_count;
	int32_t max_slab_count;
//new add
#ifdef _REENTRANT
	pthread_mutex_t e_mutex;
#endif
	struct listhead notfull_slab;
	struct listhead full_slab;
} mempool_entry_t;

typedef struct _mempool_slab_t {
	mempool_entry_t *entry;
	char *node_buffer;
	int32_t used_node_count;	
	int32_t slab_type;
	struct listhead unused_list;
	struct listhead notfull_slab;
	struct listhead full_slab;
} mempool_slab_t;

typedef struct _mempool_node_t {
	int32_t magic;
	int node_status;
	mempool_slab_t *slab;
	struct listhead unused_list;
	char mem[0];
} mempool_node_t;

typedef int32_t  mempool_hd_t;
#define MEMPOOL_HD_INVALID	(-1)

//external interface
mempool_hd_t mempool_create(int32_t node_size, int32_t node_num, int32_t max_slab_num);
void mempool_destroy(mempool_hd_t hd);
void *mempool_malloc(mempool_hd_t hd);
void mempool_free(void *mem);

#endif
