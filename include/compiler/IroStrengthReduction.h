#ifndef COMPILER_IROSTRENGTHREDUCTION_H
#define COMPILER_IROSTRENGTHREDUCTION_H

#include "compiler/common.h"

typedef struct StrengthReductionSearch StrengthReductionSearch;
typedef struct StrengthReductionNode StrengthReductionNode;

struct StrengthReductionSearch {
    IROLinear* linear;
    UInt8 found;
};

struct StrengthReductionNode {
    IROLinear* copy;
    IROLinear* linear;
    StrengthReductionNode* next;
};

extern void IRO_MarkStrengthReductionCandidates(IROLoop* loop);
extern void fn_005B50F0(StrengthReductionNode** nodes);
extern void fn_005B5190(IROLoop* loop, StrengthReductionNode** nodes);
extern void fn_005B5BF0(IROLinear* linear, Boolean visited, void* data);
extern Boolean fn_005B5C10(
    IROLoop* loop, IROLinear* linear, void* factor, void* induction, void* operand
);
extern void fn_005B6A20(IROLinear* linear, char visited, StrengthReductionSearch* search);
extern void fn_005B6A40(IROLinear* linear, char visited);

#endif
