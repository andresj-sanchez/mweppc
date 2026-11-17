#ifndef COMPILER_IROUNROLLLOOP_H
#define COMPILER_IROUNROLLLOOP_H

#include "compiler/common.h"
#include "compiler/IroLoop.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void (*unroll_count_callback)(IROLoop* loop, SInt32 stride, CInt64* result);
extern void fn_00570810(IROLinear* first, IROLinear* last, CInt64 val, IROLoop* loop);
extern void fn_00571950(
    IROLoop* loop,
    SInt32 vectorStride,
    CInt64* unrollFactor,
    SInt32* leftOver,
    UInt32* needOrigLoop,
    UInt32* needUnrollBodyTest,
    UInt32* resetUnrolledFinalValue
);
extern Boolean fn_00574620(CInt64 a, CInt64 b, CInt64* result);
extern IROLinear* fn_005732A0(IROLinear* iterCount, CInt64 factor, IROList* list, IROLoop* loop);
extern IROLinear* fn_00573420(IROLinear* iterCount, CInt64 factor, IROList* list, IROLoop* loop);
extern IROLinear* fn_00573640(IROLoopInd* induction, CInt64 factorValue, IROList* list);
extern IROLinear* fn_00571120(
    IROLinear* original, CInt64 firstFactor, CInt64 otherFactor, IROList* list, IROLoop* loop
);
extern IROLinear* IRO_BuildOrigIterationCount(IROList* list, IROLoop* loop, CInt64 offset);
extern IROLinear* IRO_UpdateInductionIncrement(IROLoop* loop, CInt64 count, IROLinear* before);
extern void IRO_BuildUnrolledBodyEntryTest(
    IROLoop* loop, IROList* list, IROLinear* iter, CInt64 factor, CLabel* label
);
extern void IRO_LoopUnroller(void);

extern const char IRO_LoopUnroller_Found_5EB28B[];
extern const char IRO_LoopUnroller_Includes_5EB2B7[];

#ifdef __cplusplus
}
#endif

#endif
