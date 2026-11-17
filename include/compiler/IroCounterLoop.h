#ifndef COMPILER_IROCOUNTERLOOP_H
#define COMPILER_IROCOUNTERLOOP_H

#include "compiler/common.h"

typedef struct IROCounterLoop {
    UInt8 unk0[0xC];
    IROLinear* count; /* 0x0C */
    UInt8 unk10[0x14];
    void* unk24; /* 0x24 */
    UInt8 unk28[4];
} IROCounterLoop;

extern Boolean fn_005B4150(IROLoop* loop);
extern char IRO_CounterLoopFoundMsg[];

#endif
