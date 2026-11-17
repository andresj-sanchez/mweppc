#ifndef _MSL_STARTUP_WIN32_H
#define _MSL_STARTUP_WIN32_H

#include <stddef.h>

extern int _doserrno;

extern int __set_errno(unsigned long err);

extern unsigned long __dup_core(void* handle, void** dup_handle);

extern int* __get_MSL_init_count(void);

extern void _CleanUpMSL(void);

#endif
