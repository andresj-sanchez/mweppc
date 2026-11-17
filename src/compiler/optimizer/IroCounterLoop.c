#include "compiler/CMachine.h"
#include "compiler/IroLoop.h"
#include "compiler/IroFlowgraph.h"
#include "compiler/IroUtil.h"
#include "compiler/CompilerTools.h"
#include <string.h>
#include "compiler/IroDump.h"
#include "compiler/IroCounterLoop.h"

void IRO_MarkPossibleCounterLoop(IROLoop* loop) {
    fn_0059DCF0(loop);
    IRO_FindUniqueInLoopHeaderTarget(loop);
    if (fn_005B4150(loop)) {
        loop->unk14->last->flags |= 0x40000;
        IRO_Dump(IRO_CounterLoopFoundMsg, loop->unk10->index);
        loop->unk10->last->u.branch.counterLoop = IRO_oalloc2(sizeof(IROCounterLoop));
        memset(loop->unk10->last->u.branch.counterLoop, 0, sizeof(IROCounterLoop));
        if (!CInt64_Equal(loop->count, cint64_negone)) {
            loop->unk10->last->u.branch.counterLoop->count =
                IRO_NewIntConst(loop->count, TYPE(&stunsignedlong));
        } else {
            loop->unk10->last->u.branch.counterLoop->count = loop->countExpr.tail;
        }
        loop->unk10->last->u.branch.counterLoop->unk24 = loop->induction->unk38;
    }
}
