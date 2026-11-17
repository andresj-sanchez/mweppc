#include "runtime/globdest.h"

DestructorChain* __global_destructor_chain;
int __atexit_count;
DestructorChain __atexit_table[64];

void* __register_global_object(
    void* object, void(__fastcall* destructor)(void*), DestructorChain* regmem
) {
    regmem->next = __global_destructor_chain;
    regmem->destructor = destructor;
    regmem->object = object;
    __global_destructor_chain = regmem;
    return object;
}

#pragma use_frame on
void __destroy_global_chain(void) {
    DestructorChain* chain;
    while ((chain = __global_destructor_chain) != 0) {
        __global_destructor_chain = chain->next;
        chain->destructor(chain->object);
    }
}

int __register_atexit(void (*func)(void)) {
    if (__atexit_count == 64) {
        return -1;
    }

    __register_global_object(0, (void(__fastcall*)(void*))func, &__atexit_table[__atexit_count++]);
    return 0;
}

#pragma use_frame off
