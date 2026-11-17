#ifndef COMPILER_IRODEPENDSKILLS_H
#define COMPILER_IRODEPENDSKILLS_H

#include "compiler/common.h"

extern void IRO_GetKills(IROLinear* linear);
extern void fn_0057F550(IROLinear* linear, Boolean first, Boolean last);
extern char IRO_DependsKillsSourceFile[];
extern SInt32 IRO_KillTableMax_61F308;
extern BitVector* IRO_KillTable_61F30C[];
extern BitVector* IRO_KillZeroTable_620314[];
extern void IRO_GetKillsInExpr(IROLinear* expr, Boolean visited, void* data);
extern void IRO_GetUsesInExpr(IROLinear* linear, Boolean allocate, Boolean first, Boolean last);
//! @reconstructed

#endif
