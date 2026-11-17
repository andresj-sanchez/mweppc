#ifndef RUNTIME_MAINCRT_H
#define RUNTIME_MAINCRT_H

#include "runtime/ExceptionX86.h"

struct runinit_table;

#ifdef __cplusplus
extern "C" {
#endif

extern ExceptionTableHeader maincrt_exception_table;
extern struct runinit_table __xc_a;
void mainCRTStartup(void);

#ifdef __cplusplus
}
#endif

#endif
