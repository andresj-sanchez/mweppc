#ifndef COMPILER_CABI_H
#define COMPILER_CABI_H

#include "compiler/common.h"

typedef enum CABIDestroyMode {
    CABIDestroyBase = 0,
    CABIDestroyComplete = 1,
    CABIDestroyDelete = 2,
    CABIDestroySDelete = 3
} CABIDestroyMode;

extern Object* CABI_NewGuardVariable(Object* object);
extern ENode* CABI_AcquireGuardVariable(Object* object, int unused);
extern ENode* CABI_ReleaseGuardVariable(Object* object, int unused);
extern Object* CABI_GetDestructorObject(Object* obj, CABIDestroyMode mode);
extern ENode* CABI_DestroyObject(
    Object* dtor, ENode* objexpr, CABIDestroyMode mode, Boolean nonvirtual, Boolean dealloc
);
extern Type* CABI_GetSizeTType(void);
extern Type* CABI_GetPtrDiffTType(void);
extern short CABI_GetStructResultArgumentIndex(TypeFunc* tfunc);
extern SInt16 CABI_StructSizeAlignValue(Type* type, SInt32 size);
extern void CABI_ReverseBitField(TypeBitfield* tbitfield);
extern SInt32 CABI_GetVTableOffset(TypeClass* tclass);
extern ENode* CABI_MakeThisExpr(TypeClass* tclass, SInt32 offset);

extern void CABI_MakeDefaultArgConstructor(TypeClass* tclass, Object* func);
extern void CABI_MakeDefaultAssignmentOperator(TypeClass* tclass, Object* func);
extern void CABI_MakeDefaultConstructor(TypeClass* tclass, Object* func);
extern void CABI_MakeDefaultCopyConstructor(TypeClass* tclass, Object* func);
extern void CABI_MakeDefaultDestructor(TypeClass* tclass, Object* func);

extern ENode* fn_0052DC40(ENode* expr);
extern void fn_0052DEC0(void);

#endif
