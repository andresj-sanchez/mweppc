#ifndef COMPILER_SCANFLOATX86_H
#define COMPILER_SCANFLOATX86_H

#include "compiler/common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern UInt32 ScanFloat(void* input, double* destination, UInt8* status);

#ifdef __cplusplus
}
#endif

#endif
