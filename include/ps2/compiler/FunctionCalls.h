#ifndef COMPILER_FUNCTIONCALLS_H
#define COMPILER_FUNCTIONCALLS_H

#include "compiler/common.h"

typedef ENodeList CallArgumentList;

typedef struct CallArgument CallArgument;
struct CallArgument {
    CallArgument* next;
    ENode* node;
    Operand operand;
    SInt32 stackOffset;
    SInt16 gpr;
    SInt16 fpr;
    UInt16 used;
    SInt16 flags;
    UInt16 type;
    UInt16 pad;
};

extern CallArgument* fn_004B49C0(ENode* skip, CallArgumentList* args, FuncArg* formal);
extern CallArgument* fn_004B45A0(ENode* skip, CallArgumentList* args, FuncArg* formal);
extern void fn_004B4200(CallArgument* arg);
extern void fn_004B3BD0(Object* target, int reg, UInt32* usedRegisters);
extern Boolean fn_004B4DB0(void* match, void* node);
extern void FunctionCalls_GenerateCall(ENode* node, Operand* result);
//! @reconstructed

#endif
