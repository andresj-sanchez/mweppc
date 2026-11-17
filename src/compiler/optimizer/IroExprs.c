#include "compiler/IroExprs.h"
#include "compiler/IroCSE.h"
#include "compiler/IroLinearForm.h"
#include "compiler/BitVector.h"
#include "compiler/CError.h"
#include "compiler/objects.h"
#include "compiler/IroDump.h"
#include "compiler/IroFlowgraph.h"

void IRO_DumpExprs(void) {
    IROExpr* expr;

    if (IRO_Dumping()) {
        IRO_Dump(IRO_ExprsHeading);
        for (expr = IRO_FirstExpr; expr; expr = expr->next) {
            IRO_Dump(
                IRO_ExprsDisplayFormat,
                expr->index,
                expr->linear->index,
                expr->node->index,
                expr->couldError,
                expr->notSubable
            );
            Bv_Dump(IRO_ExprsDependsLabel, expr->depends);
            IRO_Dump(IRO_ExprsLineEnd);
        }
        IRO_Dump(IRO_ExprsLineEnd);
    }
}

void IRO_RemoveExpr(IROExpr* expr) {
    IROExpr* prev = NULL;
    IROExpr* scan = IRO_FirstExpr;

    while (scan != expr) {
        prev = scan;
        scan = scan->next;
        CError_ASSERT(0xD4, scan);
    }
    expr->temp->unk1C = 0;
    if (prev) {
        prev->next = expr->next;
    } else {
        IRO_FirstExpr = expr->next;
    }
}
