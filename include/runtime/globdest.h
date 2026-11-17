#ifndef RUNTIME_GLOBDEST_H
#define RUNTIME_GLOBDEST_H

typedef struct DestructorChain {
    struct DestructorChain* next;
    void(__fastcall* destructor)(void*);
    void* object;
} DestructorChain;

extern DestructorChain* __global_destructor_chain;
extern void* __register_global_object(
    void* object, void(__fastcall* destructor)(void*), DestructorChain* regmem
);
extern void __destroy_global_chain(void);
extern int __register_atexit(void (*func)(void));

#endif
