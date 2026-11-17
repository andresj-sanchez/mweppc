#ifndef COMPILER_CCLASS_H
#define COMPILER_CCLASS_H
#include "compiler/common.h"
extern void CClass_NewAccessCheck(int zero, TypeClass* a, TypeClass* b, int c, int d, int e);
extern Boolean CClass_IsBaseClass(
    TypeClass* a, TypeClass* b, short* founddepth, Boolean pathcheckflag, Boolean ambigerrorflag
);
extern Type* CClass_PointerTypeCopy(Type* type);
extern Type* CClass_CombineClassAccessQualifiers(
    Type* type, UInt32 qual1, UInt32 qual2, UInt32* outflags
);
extern Boolean CClass_IsTrivialCopyClass(TypeClass* tclass);
extern Object* CClass_HasCopyConstructor(TypeClass* tclass);
extern Boolean CClass_ReferenceArgument(TypeClass* tclass);
extern UInt8 CClass_GetOverrideKind(Object* a, Object* b, Boolean errorflag);
extern TypeClass* main_class;
extern void* cclass_root;
extern void* CClass_BuildOVClassTree(void* ovclass, TypeClass* tclass, SInt32 a, SInt32 b);
extern void CClass_OverrideOVClassTree(void* tree);
extern SInt32 CClass_CheckClass(void* tree, Boolean flag);
extern void CClass_CheckHideVirtual(void* tree, void* tree2);
extern void CClass_CheckVirtualBaseOverrides(void* tree, void* tree2, Boolean flag);
extern ObjectList* CClass_DeclareCovariantFuncs(
    ObjectList* objlist, Object* object, TypeClass* tclass
);
extern void CClass_CheckOverrides(TypeClass* tclass);
extern Boolean CClass_OverridesBaseMember(TypeClass* tclass, HashNameNode* name, Object* obj);
extern SInt32 CClass_GetPathOffset(BClassList* path);
extern AccessType CClass_GetPathAccess(BClassList* path);
extern Boolean CClass_IsMoreAccessiblePath(BClassList* path1, BClassList* path2);
extern BClassList* CClass_GetBasePathRec(TypeClass* a, TypeClass* b, SInt32 offset, short depth);
extern BClassList* CClass_GetBasePath(
    TypeClass* a, TypeClass* b, short* depth, Boolean* isambigbase
);
extern ENode* CClass_AccessPathCast(BClassList* path, ENode* expr, Boolean reverse);
extern ENode* CClass_DirectBasePointerCast(ENode* expr, TypeClass* a, TypeClass* b);
extern void CClass_Init(void);
extern void CClass_ClassDefaultFuncAction(TypeClass* tclass);
extern Object* CClass_AssignmentOperator(TypeClass* tclass);
extern NameSpaceObjectList* CClass_MemberObject(TypeClass* tclass, HashNameNode* name);
extern NameSpaceObjectList* CClass_Constructor(TypeClass* tclass);
extern Object* CClass_Destructor(TypeClass* tclass);
extern Boolean CClass_IsConstructor(Object* obj);
extern Boolean CClass_IsDestructor(Object* obj);

extern Boolean CClass_IsPODClass(TypeClass* tclass);

extern Object* CClass_DefaultConstructor(TypeClass* tclass);

#endif
