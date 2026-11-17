#ifndef MSL_SETJMP_X86_H
#define MSL_SETJMP_X86_H

#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

int setjmp(jmp_buf env);

extern int setjmp(jmp_buf buf);

#ifdef __cplusplus
}
#endif

#endif
