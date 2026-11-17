#ifndef COMPILER_CSCOPE_H
#define COMPILER_CSCOPE_H

#include "compiler/common.h"

struct NameResult {
    struct NameSpace* nspace;
    struct HashNameNode* name;
    struct Type* type;
    UInt32 qual;
    struct ObjBase* object;
    struct NameSpaceObjectList* objects;
    struct BClassList* basePath;
    void* unk20;
    Boolean is_destructor;
    Boolean is_qualified;
    Boolean isambig;
    Boolean unused;
    Boolean is_type;
    Boolean is_conv_operator;
};

extern NameSpaceObjectList* CScope_CopyNameSpaceObjectList(
    NameSpaceObjectList* list, Boolean globalcopy
);
extern NameSpaceObjectList* CScope_FindObjectList(NameResult* result, HashNameNode* name);
extern Boolean CScope_FindClassMemberObject(TypeClass* tclass, NameResult* pr, HashNameNode* name);
extern NameSpaceObjectList* CScope_FindName(NameSpace* nspace, HashNameNode* name);

extern void CScope_RestoreScope(CScopeSave* save);
extern void CScope_SetNameSpaceScope(NameSpace* nspace, CScopeSave* save);

struct CScopeSave {
    struct NameSpace* current; /* 0x00 */
    struct TypeClass* currentclass; /* 0x04 */
    struct Object* currentfunc; /* 0x08 */
    Boolean is_member_func; /* 0x0C */
};

struct CScopeObjectIterator {
    struct NameSpace* nspace;
    struct NameSpaceName* nextname;
    struct NameSpaceObjectList* currlist;
    int hashindex;
};

extern struct TypeClass* cscope_currentclass;
extern Boolean cscope_is_member_func;
extern struct Object* cscope_currentfunc;
extern unsigned char global_access;

extern void CScope_Setup(void);
extern void CScope_Cleanup(void);
extern void CScope_GetScope(CScopeSave* save);
extern void CScope_SetClassScope(struct TypeClass* cls, CScopeSave* save);
extern void CScope_SetClassDefScope(struct TypeClass* cls, CScopeSave* save);
extern void CScope_SetFunctionScope(struct Object* function, CScopeSave* save);
extern void CScope_SetMethodScope(
    struct Object* function, struct TypeClass* cls, Boolean is_static, CScopeSave* save
);
extern NameSpace* CScope_NewHashNameSpace(HashNameNode* name);
extern NameSpace* CScope_NewListNameSpace(HashNameNode* name, Boolean is_global);
extern NameSpace* CScope_FindNonClassNonFunctionNS(NameSpace* nspace);
extern NameSpace* CScope_FindGlobalNS(NameSpace* nspace);
extern Boolean CScope_IsEmptyNameSpace(NameSpace* nspace);
extern void CScope_AddGlobalObject(struct Object* obj);
extern void CScope_InitObjectIterator(CScopeObjectIterator* iterator, NameSpace* nspace);
extern struct ObjBase* CScope_NextObjectIteratorObject(CScopeObjectIterator* iterator);
extern NameSpaceObjectList* CScope_NextObjectIteratorObjectList(CScopeObjectIterator* iterator);
extern void CScope_DefineTypeTag(NameSpace* nspace, HashNameNode* name, Type* type);
extern void CScope_AddObject(NameSpace* nspace, HashNameNode* name, struct ObjBase* obj);

extern Type* CScope_GetLocalTagType(NameSpace*, HashNameNode*);

#endif
