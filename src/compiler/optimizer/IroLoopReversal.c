#include "compiler/IroLoop.h"
#include "compiler/IroUtil.h"
#include "compiler/enode.h"
#include "compiler/IroDump.h"
#include "compiler/IroDependenceAnalysis.h"
#include "compiler/IroLoopReversal.h"

Boolean fn_005AC0A0(IROLoop* loop) {
    int count;
    Boolean passes = TRUE;

    IRO_BuildInOutLists(loop, FALSE, &count);
    if (count <= 0) {
        IRO_Dump(IRO_LoopReversalNoStoreMsg);
        passes = FALSE;
    } else if (IRO_LoopHasStrongStoreLoadDependencies(0, 0, 1)) {
        IRO_Dump(IRO_LoopReversalStoreLoadMsg);
        passes = FALSE;
    } else if (IRO_LoopHasStrongStoreStoreDependencies(0, 0, 1)) {
        IRO_Dump(IRO_LoopReversalStoreStoreMsg);
        passes = FALSE;
    }
    IRO_FreeLoopAssignmentList();
    return passes;
}

void fn_005AC460(IROLoop* loop) {
    IROLinear* before;
    IROLinear* upper;
    IROLinear* lower;
    IROList list;

    before = loop->cond;
    upper = IRO_PrimaryInductionUpperBound(loop);
    lower = IRO_InductionLowerBound(loop, loop->induction);
    if (before->nodetype != ENOTEQU) {
        before->nodetype = EGREATEREQU;
    }
    IRO_InitList(&list);
    IRO_DuplicateExpr(lower, &list);
    IRO_Paste(list.head, list.tail, before);
    IRO_LocateFather_Cut_And_Paste(upper, list.tail);
}
