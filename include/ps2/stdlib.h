#ifndef _STDLIB_H
#define _STDLIB_H

typedef struct {
    int quot;
    int rem;
} div_t;
extern div_t div(int numer, int denom);
#include <stddef.h>
#ifndef NULL
#define NULL ((void*)0)
#endif
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

extern void exit(int status);
extern void abort(void);
extern int atexit(void (*func)(void));
extern char* getenv(const char* name);
extern void* malloc(unsigned long size);
extern void free(void* ptr);
extern void* realloc(void* ptr, unsigned long size);
void* calloc(unsigned long n, unsigned long size);

#endif
