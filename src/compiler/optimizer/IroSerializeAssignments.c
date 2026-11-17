#include "compiler/IroSerializeAssignments.h"
#include "compiler/IroFlowgraph.h"
#include "compiler/IroVars.h"
#include "compiler/IroUtil.h"
#include "compiler/IroType.h"
#include "compiler/IroLinearForm.h"
#include "compiler/CFunc.h"

void IRO_SerializeAllAssignments(void) {
    IRONode* loop;
    IROLinear* start;
    IROLinear* node;
    IROLinear* first;

    for (loop = IRO_FirstNode; loop; loop = loop->nextnode) {
        first = loop->first;
        start = first;
        node = first;
        if (first != loop->last->next) {
            do {
                if (!(node->flags & 2)) {
                    fn_005A20A0(start, node);
                    start = node->next;
                }
                node = node->next;
            } while (node != loop->last->next);
        }
    }
    IRO_UpdateFlagsOnInts(IRO_FirstLinear);
}

void fn_005A20A0(IROLinear* start, IROLinear* end) {
    Boolean serialize = FALSE;
    IROLinear* node;
    IROList list;
    Type* type;
    Object* temporary;
    IROLinear* reference;

    for (node = start; node != end->next; node = node->next) {
        if ((node->flags & 2) && IRO_IsAssignment(node)) {
            serialize = TRUE;
            break;
        }
    }
    if (serialize) {
        for (node = start; node != end->next; node = node->next) {
            if (node->flags & 2) {
                if (!IRO_IsAssignment(node)) {
                    if (node->type == IROLinearFunccall) {
                        IRO_InitList(&list);
                        IRO_DuplicateExpr(node, &list);
                        type = IRO_CopyType(node->rtype);
                        temporary = create_temp_object(type);
                        IRO_FindVar(temporary, TRUE, TRUE);
                        IRO_AssignToTemp(temporary, list.tail, &list);
                        IRO_Paste(list.head, list.tail, start);
                        reference = IRO_TempReference(temporary);
                        reference->flags |= 2;
                        IRO_PasteAfter(reference->u.monadic, reference, node);
                        IRO_LocateFather_Cut_And_Paste(node, reference);
                    }
                } else {
                    fn_005A1F80(node, start);
                }
            }
        }
    }
}

void fn_005A1F80(IROLinear* expr, IROLinear* before) {
    IROList first;
    IROList duplicated;
    IROList trailing;
    Object* temporary;
    IROLinear* node;
    UInt8 type;

    IRO_InitList(&first);
    IRO_InitList(&duplicated);
    IRO_InitList(&trailing);
    type = *(volatile UInt8*)&expr->type;
    node = expr->u.monadic;
    if (type == IROLinearOp1Arg && (expr->nodetype == EPOSTINC || expr->nodetype == EPOSTDEC)) {
        temporary = create_temp_object(IRO_CopyType(node->rtype));
        IRO_FindVar(temporary, TRUE, TRUE);
        IRO_DuplicateExpr(node, &first);
        IRO_AssignToTemp(temporary, first.tail, &first);
        IRO_Paste(first.head, first.tail, before);
    }
    IRO_DuplicateExpr(expr, &duplicated);
    duplicated.tail->flags &= ~2;
    IRO_Paste(duplicated.head, duplicated.tail, before);
    if (expr->type == IROLinearOp1Arg &&
        (expr->nodetype == EPOSTINC || expr->nodetype == EPOSTDEC)) {
        node = IRO_TempReference(temporary);
        node->flags |= 2;
        IRO_PasteAfter(node->u.monadic, node, expr);
        IRO_LocateFather_Cut_And_Paste(expr, node);
    } else {
        IRO_DuplicateExpr(node, &trailing);
        IRO_PasteAfter(trailing.head, trailing.tail, expr);
        IRO_LocateFather_Cut_And_Paste(expr, trailing.tail);
    }
}
