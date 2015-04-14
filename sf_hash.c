#include <stdio.h>
#include <stdlib.h>
#include "sf_hash.h"

//new add
static inline void __lock_table(hash_table *ht)
{
#ifdef _REENTRANT
	pthread_mutex_lock(&ht->t_mutex);
#endif
}

static inline void __unlock_table(hash_table *ht)
{
#ifdef _REENTRANT	
	pthread_mutex_unlock(&ht->t_mutex);
#endif
}

static inline void __lock_hash(hash_t *hash)
{
#ifdef _REENTRANT
	pthread_mutex_lock(&hash->h_mutex);
#endif
}

static inline void __unlock_hash(hash_t *hash)
{
#ifdef _REENTRANT	
	pthread_mutex_unlock(&hash->h_mutex);
#endif
}

uint32_t BKDRHash(void* key, uint32_t len)
{
    uint32_t seed = 131; /// 31 131 1313 13131 131313 etc.. 
    uint32_t hash = 0;
    uint32_t i    = 0;
    int8_t *str = (int8_t *)key;

    for(i = 0; i < len; str++, i++)
    {
        hash = (hash * seed) + (*str);

    }

    return hash;

}

uint32_t hash_init(hash_t *hash, uint32_t size)
{
	uint32_t table_size = 0;
	hash->used = 0;
	hash->collisions = 0;

	hash->hashkey = NULL;
	hash->keydestroy = NULL;

	hash->compare = NULL;

	table_size = (size > 0 && size <= HASH_TABLE_MAXSIZE)? size : DEFAULT_TABLE_SIZE;
	hash->size = table_size;
//new add
#ifdef _REENTRANT
    	pthread_mutex_init(&hash->h_mutex, NULL);
#endif
	hash->table = (struct _hash_table *)malloc(hash->size*sizeof(struct _hash_table)); 
	if (NULL != hash->table){
#ifdef _REENTRANT
    		pthread_mutex_init(&hash->table->t_mutex, NULL);
#endif
		return 1;
	}
	return 0;
}

void hash_destroy(hash_t *hash)
{
	uint32_t i = 0;
	struct _hash_table *ht = NULL;
	struct _hash_item *hi = NULL;
	struct _hash_item *tmp = NULL;

	for (i = 0; i < hash->size; i++) {
		ht = &(hash->table[i]);
		if (ht->count <= 0) {
			continue;
		}
		hi = ht->items; 
		while (NULL != hi) {
			if (hash->keydestroy != NULL && NULL != hi->key) {
				hash->keydestroy(&(hi->key));
			}

			tmp = hi;
			hi = hi->next;
			free(tmp);
		}
		ht->count = 0;
	}

	free(hash->table);
	hash->table = NULL;

	return;
}

hash_item *hash_find(hash_t *hash, void *key)
{
	uint32_t ht_index = 0;
	hash_table *ht = NULL;
	hash_item *hi = NULL;

	if (NULL == key) {
		return NULL;
	}
	
	ht_index = hash->hashkey(key, strlen(key))%hash->size;
	ht = &(hash->table[ht_index]);
	if (NULL == ht || ht->count <= 0) {
		return NULL;
	}
	hi = ht->items;
	while (hi) {
		if (0 == hash->compare(hi->key, key)) {
			return hi;
		}
		hi = hi->next;
	}
	return NULL;
}

uint32_t hash_add(hash_t *hash, void *key, void *data)
{
	hash_item *hi = NULL;
	hash_table *ht = NULL;
	uint32_t ht_index = 0;
	int32_t keylen = 0;

	if (NULL == key || NULL == data) {
		return 0;
	}
	keylen = strlen((char *)key);	
	if (keylen <= 0) {
		return 0;
	}
	ht_index = hash->hashkey(key, strlen(key))%hash->size;
	ht = &(hash->table[ht_index]);
	if (NULL == ht) {
		return 0;
	}
	__lock_table(ht);
	hi = hash_find(hash, key);
	if (NULL == hi) {
		//create a new item.
		hi = (hash_item *)malloc(sizeof(hash_item));
		if (NULL == hi) {
			return 0;
		}
		
		if (hash->keydestroy != NULL) {
			hi->key = strdup(key);
		} else {
			hi->key = key;
		}
		hi->data = data;

		hi->next = NULL;
		//insert into ht
	
		hi->next = ht->items;
		ht->items = hi;
		if (ht->count > 0) {
			hash->collisions++;
		} else {
			hash->used++;
		}
		ht->count++;
	} else {
	}
	__unlock_table(ht);
	return 1;
}

uint32_t hash_exists(hash_t *hash, void *key)
{
	if (NULL == hash_find(hash, key)) {
		return 0;	
	} else {
		return 1;
	}
}
void *hash_search(hash_t *hash, void *key)
{
	hash_item *hi = NULL;
	hi = hash_find(hash, key);

	return (hi==NULL)? NULL : hi->data;
}

void hash_delete(hash_t *hash, void *key)
{
	hash_item *hi = NULL;
	hash_item *cursor = NULL;
	uint32_t ht_index = 0;
	hash_table *ht = NULL;

	if (NULL == key) {
		return;
	}
	__lock_hash(hash);	
	hi = hash_find(hash, key);
	if (NULL == hi) {
	} else {
		if (NULL != hi->next) {
			cursor = hi->next;
			if (hash->keydestroy) {
				hash->keydestroy(&(hi->key));
			}
			
			hi->data = cursor->data;
			hi->key = cursor->key;
			hi->next = cursor->next;
			free(cursor);
			cursor = NULL;
			
			ht_index = hash->hashkey(key, strlen(key))%hash->size;
			ht = &(hash->table[ht_index]);
			if (NULL == ht) {
				__unlock_hash(hash);
				return;
			}
			ht->count--;
		} else {
			ht_index = hash->hashkey(key, strlen(key))%hash->size;
			ht = &(hash->table[ht_index]);
			cursor = ht->items;
			if (hi == cursor) {
				if (hash->keydestroy) {
					hash->keydestroy(&(hi->key));
				}

				free(hi);
				hi = NULL;
				
				//fix bug, ht->items = the wild pointer. should reset it to NULL. 
				ht->items = NULL;
				//end
				
				ht->count--;
				hash->used--;
			} else {
				while (cursor) {
					if (cursor->next == hi) {
						break;
					}
					cursor = cursor->next;
				}
				if (cursor) {
					cursor->next = hi->next;
					if (hash->keydestroy) {
						hash->keydestroy(&(hi->key));
					}
					
					free(hi);
					hi = NULL;

					ht->count--;
				}
			}
		}
	}
	__unlock_hash(hash);
	return;
}

/* End Of BKDR Hash Function */
