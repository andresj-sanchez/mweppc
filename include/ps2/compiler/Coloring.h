#ifndef COMPILER_COLORING_H
#define COMPILER_COLORING_H

#include "compiler/common.h"
#include "compiler/RegisterInfo.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void colorinstructions(Object* proc);

extern SInt32 spill_passes;

#ifdef __cplusplus
}
#endif

#endif
