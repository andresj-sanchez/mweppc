#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

#ifndef _MSL_RESTRICT
#define _MSL_RESTRICT
#endif

char* strchr(const char* s, int c);
void* memcpy(void* dst, const void* src, size_t n);
char* strcpy(char* _MSL_RESTRICT dst, const char* _MSL_RESTRICT src);
#ifndef __cplusplus
int strcmp(const char* a, const char* b) __attribute__((pure));
#else
int strcmp(const char* a, const char* b);
#endif
char* strncpy(char* _MSL_RESTRICT dst, const char* _MSL_RESTRICT src, unsigned long n);
char* strcat(char* dst, const char* src);
char* strncat(char* dst, const char* src, size_t n);
int strncmp(const char* a, const char* b, size_t n);
char* strrchr(const char* s, int c);
char* strpbrk(const char* s, const char* set);
char* strstr(const char* s, const char* needle);
void* memmove(void* dst, const void* src, size_t n);
void* __memrchr(const void* src, int c, size_t n);
char* strerror(int errnum);

size_t __builtin_strlen(const char* s) __attribute__((pure));
int __builtin_memcmp(const void* a, const void* b, size_t n) __attribute__((pure));
void* __builtin_memset(void* dst, int c, size_t n);
#define strlen(s) __builtin_strlen(s)
#define memcmp(a, b, n) __builtin_memcmp(a, b, n)
#define memset(d, c, n) __builtin_memset(d, c, n)

#endif
