#ifndef _SF_HASH_HEADER
#define _SF_HASH_HEADER
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/types.h>
//new add
#ifdef _REENTRANT
#include <pthread.h>
#endif
typedef struct _hash_item {
	void *key;
	void *data;
	struct _hash_item *next;
}hash_item;

typedef struct _hash_table {
	uint32_t count;
	hash_item *items;
//new add
#ifdef _REENTRANT
	pthread_mutex_t t_mutex;
#endif
} hash_table;

typedef struct _hash_t {
	struct _hash_table *table;
	uint32_t used;
	uint32_t size;
	uint32_t collisions;
	
	uint32_t (*compare)(const void *dest, const void *src);
	uint32_t (*hashkey)(void *key, uint32_t len);

	void (*keydestroy)(void **key);
//new add
#ifdef _REENTRANT
	pthread_mutex_t h_mutex;
#endif
} hash_t;

uint32_t hash_init(hash_t *hash, uint32_t size);
void hash_destroy(hash_t *hash);
uint32_t hash_add(hash_t *hash, void *key, void *data);
uint32_t hash_exists(hash_t *hash, void *key);
void *hash_search(hash_t *hash, void *key);
hash_item *hash_find(hash_t *hash, void *key);
void hash_delete(hash_t *hash, void *key);
uint32_t BKDRHash(void* key, uint32_t len);

#define DEFAULT_TABLE_SIZE	500000	
#define HASH_TABLE_MAXSIZE	999999	

#define ht_no_destructor NULL

#define hash_set_hashkey(t, f) ((t)->hashkey=(f))
#define hash_set_keydestroy(t,f) ((t)->keydestroy = (f))
#define hash_set_compare(t,f) ((t)->compare = (f))

#endif
