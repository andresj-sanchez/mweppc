#ifndef COMPILER_STRUCTMOVES_H
#define COMPILER_STRUCTMOVES_H

#include "compiler/common.h"

extern void fn_0053CC40(Operand* dst, Operand* src, SInt32 size, SInt32 alignment);
extern void fn_0053D440(
    UInt16 loadOpcode,
    UInt16 storeOpcode,
    SInt32 step,
    Operand* output,
    Operand* input,
    Operand* destination,
    Operand* source,
    SInt32 size
);
extern void fn_0053DE80(Operand* dst, Operand* src, SInt32 count);
extern void fn_0053E560(Operand* dst, Operand* src, SInt32 count);
extern void fn_0053EB60(Operand* dst, Operand* src, SInt32 count);
extern void fn_0053EFB0(Operand* dst, Operand* src, SInt32 count);
extern void fn_0053F230(Operand* dst, Operand* src, SInt32 count);
extern void fn_0053F420(Operand* dst, Operand* src, int size);
extern void fn_0053F4D0(Operand* op);

#endif
