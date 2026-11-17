#ifndef COMPILER_CDECL_H
#define COMPILER_CDECL_H

#include "compiler/common.h"
#include "compiler/CMachine.h"
#include "compiler/types.h"

struct DeclInfo {
    Type* thetype; /* 0x00 */
    UInt32 qual; /* 0x04 */
    UInt32 unknown8; /* 0x08 */
    NameSpace* nspace; /* 0x0C */
    SInt32 unk10; /* 0x10 */
    HashNameNode* name; /* 0x14 */
    UInt8 unk18[0x14]; /* 0x18 */
    ENode* arrayBound; /* 0x2C */
    UInt8 unk30[0x1C]; /* 0x30 */
    SInt16 storageclass; /* 0x4C */
    UInt8 section; /* 0x4E */
    UInt8 exportflags; /* 0x4F */
    UInt8 unk50[2]; /* 0x50 */
    Boolean isNewExpression; /* 0x52 */
    Boolean hasArrayDimension; /* 0x53 */
    UInt8 unk54; /* 0x54 */
    Boolean isType; /* 0x55 */
    UInt8 unk56[4]; /* 0x56 */
    Boolean is_extern_c; /* 0x5A */
    UInt8 unk5B; /* 0x5B */
    Boolean allowZeroLengthArray; /* 0x5C */
    UInt8 unk5D[9]; /* 0x5D */
    Boolean unk66; /* 0x66 */
    Boolean unk67; /* 0x67 */
    UInt8 unk68[4]; /* 0x68 */
    struct CPrepFileInfo* file; /* 0x6C */
    struct CPrepFileInfo* file2; /* 0x70 */
    SInt32 sourceoffset; /* 0x74 */
    UInt8 unk78[4]; /* 0x78 */
    struct ObjRegInfo* reginfo; /* 0x7C */
};

extern Type* CDecl_NewStructType(SInt32 size, SInt16 align);
extern Type* CDecl_NewArrayType(Type* type, SInt32 size);
extern Type* CDecl_NewPointerType(Type* type);
extern Type* CDecl_NewRefPointerType(Type* type);
extern TypeTemplDep* CDecl_NewTemplDepType(TypeTemplDepType tdt);
extern void CDecl_SetResultReg(TypeFunc* tfunc);
extern void CDecl_SetFuncFlags(TypeFunc* tfunc, UInt32 flags);
extern void CDecl_NewConvFuncType(DeclInfo* declinfo);
extern void CDecl_CompleteType(Type* type);
extern Boolean IsCompleteType(Type* type);
extern Boolean CanAllocObject(Type* type);
extern Boolean CanCreateObject(Type* type);
extern void makethetypepointer(DeclInfo* declinfo, UInt32 qual);
extern void CDecl_AddThisPointerArgument(TypeFunc* tfunc, TypeClass* tclass);
extern void CDecl_MakePTMFuncType(TypeFunc* tfunc);
extern void CDecl_AddArgument(TypeFunc* tfunc, Type* argtype);
extern Boolean CDecl_CheckArrayIntegr(Type* type);
extern void makememberpointertype(DeclInfo* declinfo, TypeClass* tclass, UInt32 qual);
extern void scandeclarator(DeclInfo* declinfo);
extern FuncArg* parameter_type_list(DeclInfo* declinfo);

extern Type stillegal;
extern Type sttemplexpr;
extern void CDecl_ParseDirectFuncDecl(DeclInfo* declinfo);
extern void conversion_type_name(DeclInfo* declinfo);

#endif
