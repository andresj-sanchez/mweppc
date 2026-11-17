#ifndef _MSL_STRTOUL_H
#define _MSL_STRTOUL_H

#include <stdarg.h>
#include "compiler/common.h"
#include <errno.h>
#include "msl/ctype_api.h"
#include "msl/stdio_api.h"

#include <limits.h>

extern unsigned long __strtoul(
    int base,
    int max_width,
    int (*ReadProc)(void*, int, int),
    void* ReadProcArg,
    int* chars_scanned,
    int* negative,
    int* overflow
);

#endif
