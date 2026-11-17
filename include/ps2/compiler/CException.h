#ifndef COMPILER_CEXCEPTION_H
#define COMPILER_CEXCEPTION_H

#include "compiler/common.h"
#include "compiler/CFunc.h"
#include "compiler/Exceptions.h"
#include "compiler/types.h"

typedef struct DtorTemp {
    struct DtorTemp* next;
    Object* object;
    Object* dtor;
    Object* temp;
} DtorTemp;

extern ExceptionAction* cexcept_dobjstack;
extern Boolean cexcept_hasdobjects;
extern Boolean cexcept_magic;

extern void CExcept_Setup(void);
extern void CExcept_CheckStackRefs(ExceptionAction* actions);
extern void CExcept_CompareSpecifications(ExceptSpecList* a, ExceptSpecList* b);
extern Boolean CExcept_ActionCompare(ExceptionAction* a, ExceptionAction* b);
extern Boolean CExcept_ActionNeedsDestruction(ExceptionAction* action);
extern ENode* CExcept_RegisterDestructorObject(
    Object* local, SInt32 offset, Object* dtor, Boolean unused
);
extern void CExcept_RegisterLocalArray(
    Statement* stmt, Object* localarray, Object* dtor, SInt32 elements, SInt32 element_size
);
extern void CExcept_Terminate(void);
extern void CExcept_Magic(void);
extern void CExcept_ArrayInit(void);
extern void CExcept_RegisterMember(
    Statement* stmt, Object* objectptr, SInt32 offset, Object* dtor, Object* cond, Boolean isMember
);
extern void CExcept_RegisterMemberArray(
    Statement* stmt,
    Object* objectptr,
    SInt32 offset,
    Object* dtor,
    SInt32 elements,
    SInt32 element_size
);
extern Statement* CExcept_ActionCleanup(ExceptionAction* ea, Statement* stmt);
extern void fn_004EC8B0(Statement* stmt);

extern void* cexcept_uniqueobjs_61BAB8;

extern void CExcept_ScanExceptionSpecification(TypeFunc* tfunc);

extern Boolean CExcept_ExceptionTansform(Object* func, Statement* stmt);
extern void fn_004ECA00(Statement* stmt, void* spec);
extern void fn_004ECC50(Statement* stmt);

extern Boolean CExcept_CanThrowException(Object* obj, Boolean virtualCall);
extern int CExcept_IsSubList(struct ExceptionAction* a, struct ExceptionAction* b);

#endif
