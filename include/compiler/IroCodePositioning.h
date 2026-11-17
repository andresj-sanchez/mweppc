#ifndef COMPILER_IROCODEPOSITIONING_H
#define COMPILER_IROCODEPOSITIONING_H

#include "compiler/common.h"

typedef struct IROPositionBlock {
    unsigned char unk0[0x14];
    IROLinear* first;
    IROLinear* last;
    unsigned char unk1C[0x18];
    void* label;
} IROPositionBlock;

extern void IRO_DumpLoopFlags(UInt32 flags);

extern void fn_00597100(IROPositionBlock* block, void* label, IROPositionBlock* next);

#endif
