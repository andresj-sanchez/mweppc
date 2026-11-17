#include "compiler/IroVars.h"
#include "compiler/IroLoop.h"
#include "compiler/IroLinearForm.h"
#include "compiler/IroUtil.h"
#include "compiler/CError.h"
#include "compiler/CParser.h"
#include "compiler/IroScalarization.h"

void IRO_ScalarizeClassDataMembers(void) {
    Boolean succeeded = FALSE;

    if (copts.unknown132) {
        succeeded = fn_00595450();
    }
    if (!succeeded) {
        fn_00595E50();
    }
}

void fn_005967B0(Object* obj, Boolean* changed) {
    VarRecord* record = IRO_FindVar(obj, 1, 1);

    if (!record) {
        CError_Internal(IRO_ScalarizationSourceFile, 0x317);
    }
    if (record->noregister) {
        *changed = TRUE;
    }
}

void fn_00596A20(IROLinear* expr, void* data, int offset, Type* type) {
    IROLinear* operand;
    int firstbit;
    int lastbit;

    if (expr->type == IROLinearOp1Arg) {
        operand = expr->u.monadic;
    } else {
        operand = NULL;
    }
    if (!operand || operand->type != IROLinearOp1Arg || operand->nodetype != EBITFIELD) {
        firstbit = offset * 8;
        lastbit = firstbit - 1 + type->size * 8;
    } else {
        firstbit = offset * 8 + TYPE_BITFIELD(operand->rtype)->offset;
        lastbit = firstbit - 1 + TYPE_BITFIELD(operand->rtype)->bitlength;
    }
    fn_00596A90(expr, data, offset, type, firstbit, lastbit);
}

void IRO_RewriteBitFieldTemps(void) {
    IROLinear* node;
    Object* obj;
    VarRecord* record;
    IROLinear* replacement;
    IROList list;

    for (node = IRO_FirstLinear; node; node = node->next) {
        obj = IRO_IsVariable(node);
        if (obj && (node->flags & 0x80000)) {
            record = IRO_FindVar(obj, 0, 1);
            if (!record) {
                CError_Internal(IRO_ScalarizationSourceFile, 0xC5);
            }
            replacement = record->bitFieldReplacement;
            if (!replacement) {
                CError_Internal(IRO_ScalarizationSourceFile, 0xCB);
            }
            IRO_InitList(&list);
            IRO_DuplicateExpr(replacement, &list);
            IRO_NopOut(node->u.monadic);
            node->u.monadic = list.tail;
            IRO_Paste(list.head, list.tail, node);
        }
    }
    IRO_UpdateFlagsOnInts(IRO_FirstLinear);
}
