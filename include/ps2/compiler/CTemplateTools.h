#ifndef COMPILER_CTEMPLATETOOLS_H
#define COMPILER_CTEMPLATETOOLS_H

#include "compiler/common.h"

extern NameSpace* CTemplTool_SetupTemplateArgumentNameSpace(
    TemplParam* params, TemplArg* args, Boolean flag
);
extern void CTemplTool_SetupOuterTemplateArgumentNameSpace(NameSpace* nspace);
extern Type* CTemplTool_IsDependentTemplate(TemplClass* tmclass, TemplArg* args);
extern Boolean CTemplTool_IsSameTemplate(TemplParam* a, TemplArg* b);
extern Boolean CTemplTool_IsValueDepExpr(ENode* expr);
extern ENode* CTemplTool_DeduceDefaultArg(Object* func, ENode* expr);
extern TemplArg* CTemplTool_MakeGlobalTemplArgCopy(TemplArg* args);
extern Boolean CTemplTool_IsTypeDepExpr(ENode* expr);
extern Boolean CTemplTool_IsTypeDepType(Type* type);
extern void CTemplTool_RemoveOuterTemplateArgumentNameSpace(NameSpace* nspace);
extern void CTemplTool_PopInstance(TemplStack* stack);
extern void CTemplTool_RemoveTemplateArgumentNameSpace(
    NameSpace* nspace, TemplClassInst* inst, CScopeSave* save
);
extern Boolean CTemplTool_EqualParams(TemplParam* a, TemplParam* b, Boolean copyNames);
extern TemplArg* CTemplTool_MakeTemplArgList(DeduceInfo* info);

extern void fn_00519A00(
    void* member, void* tmemb, void* inst, void* templ, Object* object, int mode
);
extern void CTempl_GenFuncInstance(void* func, void* inst, int generate);
extern void fn_0055E3D0(void* inst, Object* object, int create);
extern TemplClass* CTemplTool_Class2Template(TypeClass* tclass);
extern TemplClassInst* CTemplTool_Class2TemplClassInst(TypeClass* tclass);
extern TypeClass* CTemplTool_Template2TemplClass(struct TemplClassTail* tail);
extern void CTempl_ParseInstanceScopeFunction(
    Object* object, TemplClassInst* inst, void* templ, TypeClass* other
);

extern Boolean CTemplTool_IsInTemplate(void);

extern void* fn_004BA370(void* node);

extern void* fn_004BADA0(void);

#endif
