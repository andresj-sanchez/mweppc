#include "compiler/CompilerTools.h"
#include "compiler/enode.h"
#include "compiler/IroUtil.h"
#include "compiler/IroLinearForm.h"
#include "compiler/IroLoop.h"
#include "compiler/CompilerTools.h"
#include "compiler/common.h"
#include <string.h>

IROLinear* IRO_NewLinear(IROLinearType type) {
    IROLinear* linear = IRO_oalloc2(sizeof(IROLinear));
    memset(linear, 0, sizeof(IROLinear));
    linear->stmt = CurStat;
    linear->nodetype = EPOSTINC;
    linear->next = NULL;
    linear->type = type;
    linear->rtype = NULL;
    linear->unk18 = NULL;
    linear->flags = 0;
    linear->nodeflags = 0;
    linear->expr = NULL;
    return linear;
}

void IRO_RenumberInts(void) {
    IROLinear* linear = IRO_FirstLinear;
    IRO_NumLinear = 0;
    while (linear) {
        linear->index = IRO_NumLinear++;
        linear = linear->next;
    }
}

void IRO_SaveLinearIR(IROLinearIRSave* save) {
    save->firstLinear = IRO_FirstLinear;
    save->lastLinear = IRO_LastLinear;
    save->numLinear = IRO_NumLinear;
    save->curStat = CurStat;
    save->disableDueToAsm = DisableDueToAsm;
    save->isLeafFunction = IRO_IsLeafFunction;
    save->functionHasReturn = IRO_FunctionHasReturn;
    save->nullCheckList = NullCheckList;
    save->currStmt = CurrStmt;
    save->prevStmt = PrevStmt;
    save->unk24 = (UInt32)IRO_unk61EF10;
}

void IRO_RestoreLinearIR(IROLinearIRSave* save) {
    IRO_FirstLinear = save->firstLinear;
    IRO_LastLinear = save->lastLinear;
    IRO_NumLinear = save->numLinear;
    CurStat = save->curStat;
    DisableDueToAsm = save->disableDueToAsm;
    IRO_IsLeafFunction = save->isLeafFunction;
    IRO_FunctionHasReturn = save->functionHasReturn;
    NullCheckList = save->nullCheckList;
    CurrStmt = save->currStmt;
    PrevStmt = save->prevStmt;
    IRO_unk61EF10 = (void*)save->unk24;
}
