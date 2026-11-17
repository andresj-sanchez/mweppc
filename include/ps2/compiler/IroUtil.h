#ifndef COMPILER_IROUTIL_H
#define COMPILER_IROUTIL_H

#include "compiler/common.h"
#include "compiler/CInt64.h"
#include "compiler/CFunc.h"
#include "compiler/enode.h"
#include "compiler/InlineAsm.h"

extern ENode* IRO_NewENodeFloatConst(Float value, Type* type);
extern IROLinear* IRO_NewIntConst(CInt64 value, Type* type); //! @reconstructed
extern int IRO_HasSideEffect(IROLinear* linear); //! @reconstructed

typedef void (*IROChildExpressionFunc)(IROLinear* linear, Boolean isEntry, void* data);
extern void IRO_ActOnChildExpressions(IROLinear* linear, IROChildExpressionFunc func, void* data);
//! @reconstructed

extern Boolean IRO_ENodeExprHasSideEffect(ENode* expr);
extern void IRO_WalkENodeTree(
    ENode** expr, Boolean (*callback)(ENode**, SInt32, void*), SInt32 flags, void* data
);
extern Boolean IRO_ENodeExprsSame(ENode* a, ENode* b);
extern ENode* IRO_DuplicateENode(ENode* node);
extern IROLinear* IRO_LocateFather_Cut_And_Paste_Without_Nopping(IROLinear* obj, IROLinear* newobj);
extern IROLinear* IRO_LocateFather(IROLinear* self);
extern IROLinear* IRO_TempReference(Object* object); //! @reconstructed
extern IROLinear* IRO_LocateFather_Cut_And_Paste(IROLinear* nd, IROLinear* replacement);
extern IROLinear* IRO_DuplicateExpr(IROLinear* node, IROList* list);
extern void IRO_AddToList(IROLinear* linear, IROList* list);
extern void IRO_InitList(IROList* list); //! @reconstructed
extern void IRO_TermLinearList(IROList* list);
extern void IRO_Paste(IROLinear* head, IROLinear* tail, IROLinear* before);
//! @reconstructed
extern void IRO_PasteAfter(IROLinear* first, IROLinear* last, IROLinear* after);
extern void IRO_NopNonSideEffects(IROLinear* linear, SInt32 level);
extern void IRO_NopOut(IROLinear* linear);
extern void IRO_CutAndPasteAfter(IROLinear* first, IROLinear* last, IROLinear* before);
//! @reconstructed
extern IROLinear* IRO_FindFirst(IROLinear* linear); //! @reconstructed
extern IROLinear* IRO_LogicalizeExpr(IROLinear* node, Boolean flag);
extern IROLinear* IRO_LogicalNotExpr(IROLinear* node, Boolean flag);

extern Boolean IRO_ExprsSameSemantically(IROLinear* a, IROLinear* b);
extern Boolean IRO_ExprsSame(IROLinear* a, IROLinear* b);
extern Object* FunctionName;
extern ENodeType IRO_ComplementaryOp[];
extern ENodeType IRO_AssignmentOp[];
extern ENodeType IRO_ComplementaryLogicalComparison[];
extern Boolean IRO_IsLogicalExpr(IROLinear* node);

extern Boolean IRO_IsConstantNegativeOne(IROLinear* linear);
extern Boolean IRO_IsConstantOne(IROLinear* node);
extern Boolean IRO_IsConstantZero(IROLinear* node);
extern void IRO_CheckForUserBreak(void);
extern Boolean IRO_IsFloatConstant(IROLinear* node);
extern Boolean IRO_IsIntConstant(IROLinear* node); //! @reconstructed
extern Boolean IRO_IsENodeIntConstant(ENode* node);
extern ENode* IRO_NewENode(ENodeType type);
extern Statement* IRO_NewStatement(StatementType type, ENode* expr);
extern void IRO_CopyCommonStatementFields(Statement* dst, Statement* src);
extern ENode* IRO_NewENodeIntConst(CInt64 value, Type* type);
extern Boolean IRO_IsPowerOfTwo(IROLinear* node, SInt32* bit); //! @reconstructed
extern Boolean IRO_IsConstant(IROLinear* linear);
extern Boolean IRO_ObjectIsAFunction(Object* object);
extern void IRO_SetupForUserBreakChecking(void);
extern Boolean IRO_FunctionCallMightThrowException(IROLinear* linear);
extern Boolean IRO_LinearNodeMightThrowException(IROLinear* linear);
extern void IRO_ClipExpr(IROExpr* expr);
struct IROElmList {
    void* value;
    IROElmList* next;
};

struct IROAddrRecord {
    IROLinear* linear;
    SInt32 numObjRefs;
    IROElmList* objRefs;
    SInt32 numMisc;
    IROElmList* misc;
    SInt32 numInts;
    IROElmList* ints;
    SInt32 unused;
};

extern Boolean IRO_StatementsHaveSameExceptionActions(Statement* a, Statement* b);
extern IROLinear* IRO_RootExpressionOfStatement(IROLinear* linear);
extern void IRO_ActOnExpressions(IROLinear* linear, IROChildExpressionFunc func, void* data);
extern Object* IRO_IsVariable(IROLinear* linear);
//! @reconstructed

extern void IRO_WalkTree(IROLinear* linear, IROChildExpressionFunc func, void* data);
extern IROLinear* IRO_FindStart(IROLinear* linear);
extern Boolean IRO_IsAssignOp[];
extern IROLinear* IRO_NewLabelLinearNode(IROList* list);
extern CLabel* IRO_NewLabel(void);
extern IROLinear* IRO_FindLabelNode(CLabel* label, IROLinear* linear);
extern IROLinear* IRO_FindPreced(IROLinear* a);
extern Boolean IRO_IsAssignment(IROLinear* linear);
extern void IRO_Cut(IROLinear* first, IROLinear* last);
extern void IRO_DuplicateExprRange(IROLinear* start, IROLinear* end, IROList* list);
extern void IRO_TermAddressRecord(IROAddrRecord* rec, void (*freefn)(void*));
extern void IRO_InitAddressRecord(void* rec, void* value);
extern Boolean IRO_ObjectIsRestrictQualified(Object* obj);
extern Boolean IRO_IsIntGenKillCandidate(IROLinear* linear);
extern Object* IRO_IsVariableEquIntConst(IROLinear* linear);
extern void IRO_WalkInts(
    IROLinear* first, IROLinear* last, IROChildExpressionFunc func, void* data
);
extern void IRO_WalkTreeToPropagateFlags(
    IROLinear* linear, IROChildExpressionFunc func, void* data
);
extern void IRO_MoveExpression(IROExpr* expr, IROLinear* linear);
extern Boolean IRO_StatementContainsSideEffect(IROLinear* linear);
extern Boolean IRO_StatementContainsUse(IROLinear* linear, IROLinear* target);
extern IROLinear* IRO_CheckSideEffect(IROLinear* linear);
extern CInt64 IRO_GetSelfAssignmentVal(IROLinear* linear);
extern void IRO_DecomposeAddressExpression_Cheap(
    IROLinear* linear, IROLinear** obj, SInt32* objCount, SInt32* otherCount
);
extern void IRO_ReplaceReference(IROLinear* linear, Object* obj, IROLinear* after);
extern void IRO_InitializeIRO_IsCommutativeENodeTypeArray(void);
extern void IRO_InitializeIRO_IsAssociativeENodeTypeArray(void);
extern void IRO_InitializeIRO_ComplementaryLogicalComparisonArray(void);
extern void IRO_InitializeIRO_ComplementaryOpArray(void);
extern void IRO_InitializeIRO_AssignmentOpArray(void);
extern void IRO_InitializeIRO_NonAssignmentOpArray(void);
extern void IRO_InitializeIRO_IsModifyOpArray(void);
extern void IRO_InitializeIRO_IsAssignOpArray(void);
extern IROLinear* IRO_AssignToTemp(Object* obj, IROLinear* expression, IROList* list);
extern void IRO_ReplaceReferenceWithNode(IROLinear* linear, IROLinear* replacement);
extern void IRO_FindReffedLabels(IROLinear* first, IROLinear* last);
extern IROLinear* fn_00565FF0(IROLinear* linear);
extern void IRO_CheckUnorderedRegionsForDefs(
    IROLinear* linear, IROLinear* excluded, void* kills, void* uses, Boolean* found
);
extern void IRO_CheckUnorderedRegionForDefs(
    IROLinear* linear, BitVector* kills, BitVector* uses, Boolean* found
);
//! @reconstructed
extern Boolean IRO_ExprsSameSemantically_Recursive(IROLinear* a, IROLinear* b, Boolean allowSwaps);
//! @reconstructed
extern void WalkTreeHelper(
    IROLinear* linear, IROChildExpressionFunc func, Boolean propagate, void* data
);
extern IROLinear* fn_00562650(IROLinear* expr);
extern void fn_00563AE0(
    IROLinear* linear, IROLinear* excluded, BitVector* kills, BitVector* uses, Boolean* found
);
extern Boolean fn_00562780(IROLinear* a, IROLinear* b, IROList* output);
extern Boolean fn_00561E60(IROLinear* linear);
extern Boolean fn_005621D0(IROLinear* linear);

extern void (*IRO_GetAsmEffects)(Statement* stmt, IAEffects* effects);
extern IAEffects IRO_EmptyAsmEffects;
extern void (*IRO_PropagateIntoAsm)(Statement* stmt, Object* object, ENode* expr);
extern Boolean IRO_IsAssociativeENodeTypeArray[IRO_MAXEXPR];
extern UInt32 IRO_GotoTargetCount;
extern UInt8 IRO_IsCommutativeENodeTypeArray[IRO_MAXEXPR];
extern void* IRO_CallArgumentsFlag;
extern void* IRO_GotoTargetSentinel;

extern void* (*IRO_CopyAsmStat)(void*);

#endif
