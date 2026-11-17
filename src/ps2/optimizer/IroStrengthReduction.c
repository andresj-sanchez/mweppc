#include "compiler/IroStrengthReduction.h"
#include "compiler/IroExprs.h"
#include "compiler/IroLoop.h"
#include "compiler/IroCSE.h"
#include "compiler/IroUtil.h"
#include "compiler/enode.h"
#include "compiler/BitVector.h"
#include "compiler/CompilerTools.h"
#include "compiler/IroFlowgraph.h"

void fn_005B6A40(IROLinear* linear, char visited) {
    if (visited && linear->expr) {
        IRO_RemoveExpr(linear->expr);
    }
}

void fn_005B5BF0(IROLinear* linear, Boolean visited, void* data) {
    if (visited && linear->unk1C) {
        linear->flags &= ~0x800;
    }
}

void fn_005B50F0(StrengthReductionNode** nodes) {
    StrengthReductionNode* node;
    IROLinear* linear;
    IROLinear* left;
    IROLinear* right;
    IROList list;

    for (node = *nodes; node; node = node->next) {
        linear = node->linear;
        if (linear->type == IROLinearOp2Arg &&
            ((UInt8)(linear->nodetype - EADD) <= 1) &&
            (left = linear->u.diadic.left, left->type == IROLinearOp2Arg) &&
            left->nodetype == EMUL &&
            (right = linear->u.diadic.right, right->type == IROLinearOp2Arg) &&
            right->nodetype == EMUL &&
            !(left->flags & 0x800) &&
            !(right->flags & 0x800)) {
            IRO_InitList(&list);
            IRO_DuplicateExpr(node->copy, &list);
            IRO_PasteAfter(list.head, list.tail, node->linear);
            IRO_LocateFather_Cut_And_Paste(node->linear, list.tail);
        }
    }
}

void IRO_MarkStrengthReductionCandidates(IROLoop* loop) {
    StrengthReductionNode* nodes;
    StrengthReductionNode* next;
    IROExpr* expr;
    UInt32 result1;
    UInt32 result2;
    UInt32 result3;
    UInt16 bit;

    fn_0059DCF0(loop);
    nodes = NULL;
    fn_005B5190(loop, &nodes);
    for (expr = IRO_FirstExpr; expr; expr = expr->next) {
        bit = expr->node->index;
        if (Bv_IsBitSet(bit, loop->bitset) &&
            !expr->notSubable &&
            (!expr->couldError || expr->node->smartMustReach) &&
            fn_005B5C10(loop, expr->linear, &result1, &result2, &result3)) {
            IRO_WalkTree(expr->linear, fn_005B5BF0, NULL);
            expr->linear->flags |= 0x800;
            expr->factor = (IROLinear*)result1;
            expr->var = (VarRecord*)result2;
            expr->indOperand = (IROLinear*)result3;
        }
    }
    fn_005B50F0(&nodes);
    while (nodes) {
        next = nodes->next;
        IRO_ofree(nodes);
        nodes = next;
    }
}
