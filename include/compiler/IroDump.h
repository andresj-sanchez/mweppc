#ifndef COMPILER_IRODUMP_H
#define COMPILER_IRODUMP_H

#include "compiler/common.h"
#if MWCCPS2_B52
#pragma pack(push, 2)
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void IRO_InitializeNodeNamesArray(void);
#if MWCCPS2_B52
extern void IRO_DumpBits(char* name, BitVector* bv);
extern void IRO_DumpAfterPhase(char* str, Boolean dumpGraph);
extern void IRO_DumpBeforePhase(char* str, Boolean dumpGraph);
#else
extern void IRO_DumpBits(char* name, BitVector* bv);
extern void IRO_DumpAfterPhase(char* str, Boolean dumpGraph);
#if MWCCPSP
extern void IRO_DumpBeforePhase(char* str, Boolean dumpGraph);
#endif
#endif
extern void IRO_DumpFlowgraph(void);
extern void IRO_DumpExprs(void);
extern void IRO_SetupDump(void);
extern void IRO_CleanupDump(void);
extern void IRO_Dump(char* format, ...);
extern void IRO_SpellType(Type* type, char* buf);

#ifdef __cplusplus
}
#endif

#if MWCCPS2_B52
#pragma pack(pop)
#endif
#endif
