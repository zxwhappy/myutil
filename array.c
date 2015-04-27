#include "array.h"

//当array_t是在堆上声明时，初始化时调用
array_t *
array_create(unsigned int n, size_t size)
{
    array_t *a;

    a = malloc(sizeof(array_t));
    if (a == NULL) {
        return NULL;
    }

    if (array_init(a, n, size) != 0) {
        return NULL;
    }

    return a;
}

//当array_t是在栈上声明时，销毁的时候调用
void
array_destroy(array_t *a)
{
    if (a == NULL){
        return;
    }

    if (a->elts != NULL){
        free(a->elts);
        a->elts = NULL;
    }
}

//当array_t是在堆上声明时，销毁的时候调用
void
array_free(array_t **a)
{
    if (*a == NULL){
        return;
    }

    if ((*a)->elts != NULL){
        free((*a)->elts);
        (*a)->elts = NULL;
    }

    free(*a);
    (*a) = NULL;
}

void *
array_push(array_t *a)
{
    void        *elt;
    size_t       size;

    if (a->nelts == a->nalloc) {

        /* the array is full */

        size = a->size * a->nalloc;

        a->elts = realloc(a->elts, 2 * size);
        if (a->elts == NULL) {
            return NULL;
        }

        a->nalloc *= 2;
    }

    elt = (unsigned char *) a->elts + a->size * a->nelts;
    a->nelts++;

    return elt;
}


void *
array_push_n(array_t *a, unsigned int n)
{
    void        *elt;
    size_t       size;
    unsigned int   nalloc;

    size = n * a->size;

    if (a->nelts + n > a->nalloc) {

        /* the array is full */

        nalloc = 2 * ((n >= a->nalloc) ? n : a->nalloc);

        a->elts = realloc(a->elts, nalloc * a->size);
        if (a->elts == NULL) {
            return NULL;
        }

        a->nalloc = nalloc;
    }

    elt = (unsigned char *) a->elts + a->size * a->nelts;
    a->nelts += n;

    return elt;
}
/*
typedef struct test{
    int a;
    int b;
} test_t;

void main(){
    
    array_t *arr = NULL;
    arr = array_create(2,sizeof(test_t));
    printf("creat:%d,%d,%d,%d,%d\n", arr->size, arr->nalloc, arr->nelts, 0, 0);
    test_t *t1 = array_push(arr);
    t1->a = 1;
    t1->b = 2;
    printf("creat:%d,%d,%d,%d,%d\n", arr->size, arr->nalloc, arr->nelts, t1->a, t1->b);
    test_t *t2 = array_push(arr);
    t2->a = 3;
    t2->b = 4;
    printf("creat:%d,%d,%d,%d,%d\n", arr->size, arr->nalloc, arr->nelts, t2->a, t2->b);
    test_t *t3 = array_push(arr);
    t3->a = 5;
    t3->b = 6;
    printf("creat:%d,%d,%d,%d,%d\n", arr->size, arr->nalloc, arr->nelts, t3->a, t3->b);
    array_destroy(&arr);
}
*/
