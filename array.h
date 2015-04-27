#ifndef _ARRAY_H_INCLUDED_
#define _ARRAY_H_INCLUDED_
#include <stdlib.h>
#include <string.h>

typedef struct _array_s {
    void        *elts;
    unsigned int   nelts;
    size_t       size;
    unsigned int   nalloc;
} array_t;


array_t *array_create(unsigned int n, size_t size);
void array_destroy(array_t *a);  //当array_t是在栈上声明时，销毁的时候调用
void array_free(array_t **a);    //当array_t是在堆上声明时，销毁的时候调用
void *array_push(array_t *a);
void *array_push_n(array_t *a, unsigned int n);

//当array_t是在栈上声明时，初始化时调用
static inline int
array_init(array_t *array, unsigned int n, size_t size)
{
    /*
     * set "array->nelts" before "array->elts", otherwise MSVC thinks
     * that "array->nelts" may be used without having been initialized
     */

    array->nelts = 0;
    array->size = size;
    array->nalloc = n;

    if(array->elts != NULL) {
        free(array->elts);
        array->elts = NULL;
    }

    array->elts = malloc(n * size);
    if (array->elts == NULL) {
        return -1;
    }

    return 0;
}


#endif /* _ARRAY_H_INCLUDED_ */
