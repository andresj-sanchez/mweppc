#ifndef _MSL_POOL_ALLOC_H
#define _MSL_POOL_ALLOC_H

#include <stddef.h>

extern void* __sys_alloc(size_t size);
extern void __sys_free(void* ptr);
extern size_t __sys_pointer_size(void* ptr);

#endif
