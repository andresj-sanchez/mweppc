#ifndef COMPILER_IROLINEARFORM_H
#define COMPILER_IROLINEARFORM_H

#include "compiler/common.h"
#include "compiler/CFunc.h"
#include "compiler/IroLoop.h"

typedef struct IROLinearIRSave {
    IROLinear* firstLinear;
    IROLinear* lastLinear;
    UInt32 numLinear;
    Statement* curStat;
    Boolean disableDueToAsm;
    Boolean isLeafFunction;
    SInt32 functionHasReturn;
    void* nullCheckList;
    Statement* currStmt;
    Statement* prevStmt;
    UInt32 unk24;
} IROLinearIRSave;

extern IROLinear* IRO_FirstLinear;

extern IROLinear* IRO_NewLinear(IROLinearType type);
extern Statement* IRO_PreLinearize(Statement* stmt);
extern void IRO_Linearize(Statement* stmt);
extern Statement* IRO_Delinearize(IRONode* node, IROLinear* linear);
extern void IRO_UpdateFlagsOnInts(IROLinear* list);
extern void IRO_SaveLinearIR(IROLinearIRSave* save);
extern void IRO_RestoreLinearIR(IROLinearIRSave* save);
extern IROLinear* IRO_LastLinear;
extern UInt32 IRO_NumLinear;
extern Statement* CurStat;
extern Boolean DisableDueToAsm;
extern Boolean IRO_IsLeafFunction;
extern SInt32 IRO_FunctionHasReturn;
extern void* NullCheckList;
extern Statement* CurrStmt;
extern Statement* PrevStmt;
extern void* IRO_unk61EF10;
extern void IRO_RenumberInts(void);

#define IS_LINEAR_ENODE(_linear, _nodetype) \
    (((_linear)->type == IROLinearOperand) && ((_linear)->u.node->type) == (_nodetype))
#define IS_LINEAR_MONADIC(_linear, _nodetype) \
    (((_linear)->type == IROLinearOp1Arg) && ((_linear)->nodetype) == (_nodetype))
#define IS_LINEAR_DIADIC(_linear, _nodetype) \
    (((_linear)->type == IROLinearOp2Arg) && ((_linear)->nodetype) == (_nodetype))

#endif
