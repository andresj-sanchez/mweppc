#ifndef COMPILER_CMANGLER_H
#define COMPILER_CMANGLER_H

#include "compiler/common.h"
#include "compiler/CompilerTools.h"

extern GList name_mangle_list;

extern HashNameNode* GetHashNameNodeExport(const char* name);
extern char* CMangler_GetOperator(HashNameNode* name);

extern HashNameNode* constructor_name_node;
extern HashNameNode* destructor_name_node;
extern HashNameNode* asop_name_node;

extern void CMangler_Setup(void);
extern HashNameNode* CMangler_OperatorName(short token);
extern HashNameNode* CMangler_VTableName(TypeClass* theclass);
extern HashNameNode* CMangler_RTTIObjectName(Type* type, UInt32 qual);
extern HashNameNode* CMangler_ThunkName(
    Object* vfunc, SInt32 this_delta, SInt32 return_delta, SInt32 ctoroffset
);
extern HashNameNode* CMangler_TemplateInstanceName(HashNameNode* basename, TemplArg* args);
extern void CMangler_MangleType(Type* type, UInt32 qual, Boolean keep_size);
extern HashNameNode* CMangler_ConversionFuncName(Type* type, UInt32 qual);
extern HashNameNode* CMangler_GetCovariantFunctionName(Object* dobj, TypeClass* theclass);
extern HashNameNode* CMangler_GetLinkName(Object* object);
extern void CMangler_SetupLocalVarName(Object* local, Object* func, SInt32 varnum);
extern void CMangler_SetupGuardVarName(Object* local, Object* guarded);
extern void CMangler_SetupLocalStringName(Object* local, Object* func, SInt32 varnum);
extern void fn_004ACE00(void);
extern void fn_004ACDF0(void);
extern void fn_004ACDE0(void* ir, const char* functionName, const char* stage);

#endif
