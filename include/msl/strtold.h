#ifndef _MSL_STRTOLD_H
#define _MSL_STRTOLD_H

#include "compiler/common.h"
#include <stdarg.h>
#include "msl/stdio_api.h"

#include <limits.h>

extern double __builtin_fabs(double x);
#define fabs(x) __builtin_fabs(x)

#define DBL_MIN 0x1.0000000000000P-1022
#define DBL_MAX 0x1.fffffffffffffP1023

extern double __double_min;
extern double __double_max;

#include <errno.h>

#define ERANGE 34

extern double __strtold(
    int max_len, int (*ReadProc)(void*, int, int), void* ReadProcData, int* count, int* overflow
);
extern double strtod(const char* str, char** end);

#endif
