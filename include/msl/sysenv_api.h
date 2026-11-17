#ifndef _MSL_SYSENV_API_H
#define _MSL_SYSENV_API_H

#include <stddef.h>
#include <string.h>

extern char** _tenviron;

extern void _SetupEnv(void);
extern int __msl_putenv(const char* setting);

extern size_t strlen(const char* s);

#endif
