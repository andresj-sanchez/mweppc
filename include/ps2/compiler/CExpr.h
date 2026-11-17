#ifndef COMPILER_CEXPR_H
#define COMPILER_CEXPR_H

#include "compiler/common.h"
#include "compiler/types.h"
#include "compiler/enode.h"
#include "compiler/CScope.h"

typedef Boolean (*NameObjCheckCB)(HashNameNode*, Object*);
extern NameObjCheckCB name_obj_check;
extern Boolean disallowgreaterthan;

extern ENode* CExpr_AdjustFunctionCall(ENode* expr);
extern ENode* CExpr_GetDefaultArgument(ENode* funcexpr, FuncArg* arg);
extern ENode* integralpromote(ENode* expr);
extern ENode* promote(ENode* expr, Type* type);
extern ENode* classargument(ENode* expr);
extern ENode* fn_00522920(ENode* expr, void* a, void* b, int c, void* d);
extern short canadd(ENode* expr, SInt32 value);
extern void optimizecomm(ENode* expr);
extern ENode* argumentpromotion(ENode* expr, Type* type, short qual, Boolean checkAccess);
extern ENode* intconstnode(Type* type, SInt32 value);
extern ENode* expression(void);
extern ENode* CExpr_IntegralConstOrDepExpr(void);
extern ENode* CExpr_NewENode(ENodeType type);
extern ENode* CExpr_NewETEMPNode(Type* type, Boolean assign_id);
extern ENode* makediadicnode(ENode* left, ENode* right, ENodeType op);
extern ENode* makemonadicnode(ENode* operand, ENodeType op);
extern ENode* CExpr_AliasTransform(ENode* expr);
extern DepName* CExpr_CopyDepName(DepName* name);
typedef void (*CExprSearchCB)(ENode* expr);
extern void fn_00539780(Statement* stmt, CExprSearchCB callback);
extern ENode* fn_00539A30(ENode* expr, CExprSearchCB callback);
extern void CExpr_SearchExprTree(ENode* expr, CExprSearchCB callback);
extern ENode* funccallexpr(Object* func, ENode* arg1, ENode* arg2, ENode* arg3, ENode* arg4);
extern ENode* create_objectnode(Object* obj);
extern ENode* create_objectnode2(Object* obj);
extern ENode* CExpr_MakeObjRefNode(Object* obj, Boolean markUsed);
extern ENode* create_objectrefnode(Object* obj);
extern ENode* nullnode(void);
extern ENode* CExpr_CopyENode(ENode* expr);
extern ENode* makecommaexpression(ENode* a, ENode* b);
extern ENode* CExpr_NewTemplDepENode(TemplDepSubType t);
extern ENode* pointer_generation(ENode* expr);
extern ENode* conv_assignment_expression(void);

extern SInt32 scansizeof(void);

struct ConIteratorList {
    struct ConIteratorList* next;
    struct ConIterator* iter;
};
struct ConIterator {
    struct ConIterator* parent;
    struct ConIteratorList* children;
    struct TypeClass* tclass;
};
struct ConversionIterator {
    CScopeObjectIterator objiter;
    struct ConIterator myconiter;
    struct ConIterator* coniter;
};

extern void CExpr_ConversionIteratorInit(ConversionIterator* iter, TypeClass* tclass);
extern Object* CExpr_ConversionIteratorNext(ConversionIterator* iter);

extern ENode* CExpr_RewriteConst(ENode* expr);
extern ENode* assignment_expression(void);
extern ENode* pointer_generation2(ENode* expr);

extern Boolean CExpr_ParseAsmExpr(Object** object, CInt64* value); //! @reconstructed
extern void CExpr_CheckUnwantedAssignment(ENode* expr);

#endif
