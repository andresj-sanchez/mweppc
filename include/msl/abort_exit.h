#ifndef _MSL_ABORT_EXIT_H
#define _MSL_ABORT_EXIT_H

#include "runtime/globdest.h"

extern void (*__stdio_exit)(void);
extern void (*__console_exit)(void);

extern int __aborting;
extern int __atexit(void (*func)(void));
extern void __exit(int status);

#endif
