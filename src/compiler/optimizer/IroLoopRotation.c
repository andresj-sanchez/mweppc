#include "compiler/IroLoop.h"
#include "compiler/IroUtil.h"
#include "compiler/IroFlowgraph.h"
#include "compiler/IroLinearForm.h"
#include "compiler/CInt64.h"
#include "compiler/CParser.h"

Boolean IRO_RotateLoop(IROLoop* loop) {
    Boolean rotated;
    IROLinear* linear;
    IRONode* preheader;
    IRONode* newnode;
    CLabel* label;
    IROList list;

    fn_0059DCF0(loop);
    rotated = FALSE;
    if (loop->unk10 == loop->unk14 && !(loop->flags & 0x200000)) {
        if (!copts.optimizesize || (loop->countKnown && CInt64_Greater(loop->count, cint64_zero))) {
            IRO_FindUniqueInLoopHeaderTarget(loop);
            preheader = (IRONode*)loop->preheader;
            if (preheader &&
                preheader->last->type == IROLinearGoto &&
                preheader->nextnode == loop->unk20 &&
                loop->unk20 != loop->unk10) {
                linear = IRO_FindLabelNode(preheader->last->u.label, preheader->last);
                IRO_InitList(&list);
                IRO_DuplicateExprRange(linear->next, loop->cond, &list);
                IRO_DuplicateExpr(loop->cond, &list);
                IRO_Paste(list.head, list.tail, preheader->last);
                if (((IRONode*)loop->unk10)->last->type == IROLinearIf) {
                    preheader->last->type = IROLinearIfNot;
                } else {
                    preheader->last->type = IROLinearIf;
                }
                label = IRO_NewLabel();
                preheader->last->u.label = label;
                preheader->last->u.diadic.right = list.tail;
                newnode = IRO_NewFlowGraphNode();
                linear = IRO_NewLinear(IROLinearLabel);
                linear->index = IRO_NumLinear++;
                linear->next = NULL;
                linear->u.label = label;
                linear->flags |= 1;
                label->stmt = (Statement*)newnode;
                newnode->first = linear;
                newnode->last = linear;
                linear->next = ((IRONode*)loop->unk10)->last->next;
                ((IRONode*)loop->unk10)->last->next = linear;
                newnode->nextnode = ((IRONode*)loop->unk10)->nextnode;
                ((IRONode*)loop->unk10)->nextnode = newnode;
                loop->preheader = IRO_CreatePreHeader(loop, preheader->nextnode, preheader);
                if (loop->preheader) {
                    rotated = TRUE;
                }
                loop->unk10 = loop->unk20;
            }
        }
    }
    if (rotated) {
        IRO_BuildNodeTable();
    }
    return rotated;
}
