#ifndef COMPILER_UNMANGLE_H
#define COMPILER_UNMANGLE_H

#include <stddef.h>
#include "compiler/common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void MWUnmangleClassName(const char* input, char* output, size_t maxLen);
extern void MWUnmangle(const char* input, char* output, size_t maxLen);

extern char UnmanglePrefix_005E9E7D;

#ifdef __cplusplus
}
#endif

#endif
