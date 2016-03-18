//
// Created by root on 3/17/16.
//

#include "manage.h"
/*
 * error is preserving for future.
 * */
void* wsx_malloc(int sizes) {
    int error;
    void * alloc = NULL;
    if(sizes < 0) {
        error = MALLOC_ERR_SIZE;
        return NULL;
    }
    alloc = wsx_calloc(sizes);
    if(NULL == alloc){
        error = MALLOC_ERR_NO_MEM;
        return alloc;
    }
    return alloc;
}

void* wsx_calloc(int sizes) {
    int error;
    void * alloc = NULL;
    if(sizes < 0) {
        error = CALLOC_ERR_SIZE;
        return alloc;
    }
    alloc = calloc(1, sizes);
    if(NULL == alloc)
        error = CALLOC_ERR_NO_MEM;
    return alloc;
}

MM_STATUS wsx_free(void * pointer) {
    if(NULL == pointer)
        return FREE_ERR_EMPTY;
    free(pointer);
    return FREE_SUCCEED;
}