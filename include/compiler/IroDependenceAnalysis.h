#ifndef COMPILER_IRODEPENDENCEANALYSIS_H
#define COMPILER_IRODEPENDENCEANALYSIS_H

#include "compiler/common.h"
#include "compiler/IroFlowgraph.h"

typedef struct DependenceAssignment {
    SInt32 index; /* 0x00 */
    void* unk04;
    void* unk08;
    IROLinear* assignment; /* 0x0C */
    void* unk10;
    void* unk14;
    void* unk18;
    struct DependenceAssignment* next; /* 0x1C */
    UInt8 unk20;
    void* unk24;
} DependenceAssignment;

extern void IRO_FreeLoopAssignmentList(void);
extern void fn_005B6B50(IROLinear* linear);
extern Boolean IRO_LoopHasStrongStoreStoreDependencies(int a, int b, int c);
extern Boolean IRO_LoopHasStrongStoreLoadDependencies(int a, int b, int c);
extern UInt32 IRO_DependenceFlags;
extern UInt32 IRO_DependenceNumAssignments;
extern IROLoop* IRO_DependenceLoop;
extern UInt32 IRO_DependenceMode;
extern DependenceAssignment* IRO_LoopAssignmentListHead;
extern DependenceAssignment* IRO_DependenceHeadRecord;
extern void* IRO_DependenceResult;

extern IRONode* IRO_DependenceCurrentNode;
extern void IRO_BuildInOutLists(IROLoop* loop, UInt32 mode, int* count);

#endif
