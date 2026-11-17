#ifndef _MSL_SIGNAL_H
#define _MSL_SIGNAL_H

#include <stddef.h>

typedef void (*__signal_func_ptr)(int);

#define SIG_DFL ((__signal_func_ptr)0)
#define SIG_IGN ((__signal_func_ptr)1)
#define SIG_ERR ((__signal_func_ptr) - 1)

#define SIGABRT 1
#define SIGFPE 2
#define SIGILL 3
#define SIGINT 4
#define SIGSEGV 5
#define SIGTERM 6
#define SIGBREAK 7

#define __msl_signal_max 7

extern __signal_func_ptr signal_funcs[__msl_signal_max];
extern __signal_func_ptr ctrlc;
extern __signal_func_ptr ctrlbreak;
extern int ConsoleCtrlHandler;

extern void __check_console_handler(void);
extern int __stdcall __handler(unsigned long CtrlType);
extern __signal_func_ptr signal(int signaltype, __signal_func_ptr signal_func);
extern int raise(int signal);

#endif
