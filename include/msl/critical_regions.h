#ifndef _MSL_CRITICAL_REGIONS_H
#define _MSL_CRITICAL_REGIONS_H

#include "compiler/win32.h"

enum critical_regions {
    atexit_funcs_access = 0,
    malloc_pool_access = 1,
    stdin_access = 2,
    stdout_access = 3,
    stderr_access = 4,
    files_access = 5,
    console_status_access = 6,
    signal_funcs_access = 7,
    thread_access = 8,
    num_critical_regions = 9
};

extern CRITICAL_SECTION __cs[num_critical_regions];

#define __begin_critical_region(region) EnterCriticalSection(&__cs[region])
#define __end_critical_region(region) LeaveCriticalSection(&__cs[region])

inline void __init_critical_regions(void) {
    int i;
    for (i = 0; i < num_critical_regions; i++) {
        InitializeCriticalSection(&__cs[i]);
    }
}

inline void __kill_critical_regions(void) {
    int i;
    for (i = 0; i < num_critical_regions; i++) {
        DeleteCriticalSection(&__cs[i]);
    }
}

#endif
