#include "compiler/IroLoop.h"
#include "compiler/IroUtil.h"
#include "compiler/IroType.h"
#include "compiler/enode.h"
#include "compiler/BitVector.h"
#include "compiler/IroDependenceAnalysis.h"
#include "compiler/IroVars.h"

typedef struct DependencyWalkerContext {
    UInt8 unk00[0x0C];
    IROLinear* linear;
    UInt8 unk10[0x10];
    UInt8 flags;
} DependencyWalkerContext;

typedef struct DependencyAliasState {
    Boolean* seen;
    UInt16* flags;
} DependencyAliasState;

void fn_005B74C0(IROLinear* linear, Boolean entered, DependencyAliasState* state) {
    if (!entered && linear->type == IROLinearOp1Arg && linear->nodetype == EINDIRECT) {
        Object* obj = IRO_IsVariable(linear);
        if (obj == IRO_DependenceLoop->induction->var->object) {
            if (!*state->seen) {
                *state->flags |= 0x1000;
                *state->seen = 1;
            } else {
                *state->flags |= 0x1000;
                *state->flags |= 0x100;
            }
        }
    }
}

void fn_005B7A80(IROLinear* linear, Boolean entered, void* data) {
    IROLinear* child;
    Type* copy;

    if (!entered && linear->type == IROLinearOp1Arg && linear->nodetype == ETYPCON) {
        child = linear->u.monadic;
        copy = IRO_CopyType(child->unk18);
        if (linear->unk18 != copy && !IRO_TypesEqual(linear->unk18, copy)) {
            return;
        }
        IRO_LocateFather_Cut_And_Paste_Without_Nopping(linear, child);
        linear->type = IROLinearNop;
    }
}

void fn_005B6C70(IROLinear* linear, Boolean entered, DependencyWalkerContext* ctx) {
    if (entered &&
        linear != ctx->linear &&
        linear != IRO_DependenceLoop->induction->nd &&
        (linear->flags & IROLF_Reffed) &&
        IRO_IsAssignment(linear)) {
        ctx->flags |= 0x20;
    }
}

void IRO_BuildInOutLists(IROLoop* loop, UInt32 mode, int* count) {
    IRONode* node;
    IROLinear* linear;

    IRO_DependenceFlags = 0;
    IRO_DependenceLoop = loop;
    IRO_DependenceMode = mode;
    node = IRO_FirstNode;
    IRO_LoopAssignmentListHead = NULL;
    IRO_DependenceNumAssignments = 0;
    for (; node; node = node->nextnode) {
        if (Bv_IsBitSet(node->index, loop->bitset)) {
            IRO_DependenceCurrentNode = node;
            for (linear = node->first; linear; linear = linear->next) {
                if (!(linear->flags & IROLF_Reffed) &&
                    linear != loop->induction->nd &&
                    IRO_IsAssignment(linear)) {
                    fn_005B6B50(linear);
                }
                if (linear == node->last) {
                    break;
                }
            }
        }
    }
    if (IRO_LoopAssignmentListHead) {
        IRO_DependenceResult = IRO_DependenceHeadRecord->unk18;
    } else {
        IRO_DependenceResult = NULL;
    }
    *count = IRO_DependenceNumAssignments;
}
