#include <stddef.h>
#include "compiler/win32.h"
#include "msl/pool_alloc.h"

void* __sys_alloc(size_t size) {
    char* ptr;

    size += 8;

    ptr = (char*)GlobalAlloc(GMEM_FIXED, size);

    if (ptr == 0)
        return 0;

    if ((size_t)ptr % 8 == 0) {
        ptr += 4;
        *(size_t*)ptr = 0;
    } else {
        *(size_t*)ptr = 1;
    }

    ptr += 4;

    return (void*)ptr;
}

void __sys_free(void* p) {
    char* ptr = (char*)p;

    ptr -= 4;

    if (*(size_t*)ptr == 0)
        ptr -= 4;

    GlobalFree((HGLOBAL)ptr);
}
