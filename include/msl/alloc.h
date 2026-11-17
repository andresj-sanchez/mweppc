#ifndef MSL_ALLOC_H
#define MSL_ALLOC_H

#include <stddef.h>

typedef struct __mem_pool {
    void* reserved[14];
} __mem_pool;

extern void __init_pool_obj(__mem_pool* pool);
extern void* __pool_alloc(__mem_pool* pool, size_t size);
extern void __pool_free(__mem_pool* pool, void* ptr);
extern void* __pool_realloc(__mem_pool* pool, void* ptr, size_t size);
extern void* __pool_alloc_clear(__mem_pool* pool, size_t size);
extern void __pool_free_all(__mem_pool* pool);
extern void __malloc_free_all(void);

#endif
