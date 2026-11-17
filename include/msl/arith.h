#ifndef _MSL_ARITH_H
#define _MSL_ARITH_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int __msl_add(int* result, int amount);
extern int __msl_ladd(long* result, long amount);
extern int __msl_lmul(long* result, long amount);
extern div_t __msl_div(int x, int y);
extern int __msl_mod(int x, int y);

#ifdef __cplusplus
}
#endif

#endif
