#if MWCCPS2_B52
// b52 (3.0 build 52): tools/mwccpsp's 3.0.1 header with its MWCCPSP layouts,
// packed to 2 bytes like the rest of the b69 tree (IRO_NewStatement: 0x1E bytes,
// IROLinear's rtype at +0x10). Only MWCCPS2_B69_BUILD=52 searches include/b52.
#ifndef COMPILER_IROVARS_H
#define COMPILER_IROVARS_H

#include "compiler/common.h"
#include "compiler/enode.h"
#pragma pack(push, 2)

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 2)
// 3.0.1: 0x28 bytes, natural alignment (IRO_FindVar 0x55A2C0)
struct VarRecord {
    SInt32 index; // +0
    Object* object; // +4
    void* unused; // +8: 3.0.1 keeps the lifetime vector here (func_00594C60)
    Boolean inductionState; // +0xC (TRUE when created)
    Boolean noregister; // +0xD
    Boolean usedAtCall; // +0xE
    VarRecord* next; // +0x10
    IRODef* defs; // +0x14
    IROUse* uses; // +0x18
    TypeBitfield* bitfieldType; // +0x1C
    IROLinear* bitFieldReplacement; // +0x20
    void* unk24; // +0x24: IroNonRegLoopAccesses.c's record of the variable
};
#pragma pack(pop)

extern VarRecord* IRO_FirstVar;
extern SInt32 IRO_NumVars;
extern Boolean IRO_IsBitField;
extern SInt32 IRO_BaseTerms;
extern SInt32 IRO_VarTerms;
extern Boolean IRO_IsModifyOp[IRO_MAXEXPR];
extern Boolean IRO_IsAssignOp[IRO_MAXEXPR];

extern void IRO_InitializeIRO_IsModifyOpArray(void);
extern void IRO_InitializeIRO_IsAssignOpArray(void);
extern VarRecord* IRO_FindVar(Object* object, Boolean create, Boolean allowRegister);
extern void ResetVars(void);
extern void IRO_FindAllVars(void);
extern void IRO_ZapVarPtrs(void);
extern void IRO_UpdateVars(void);
extern void IRO_AddElmToList(IROLinear* linear, IROElmList** list);
extern void IRO_DecomposeAddressExpression(IROLinear* linear, IROAddrRecord* rec);
extern void IRO_DecomposeAddressExpression_Cheap(IROLinear* linear);
extern VarRecord* IRO_FindAssigned(IROLinear* linear);
extern void IRO_GetKills(IROLinear* linear);
extern void IRO_CheckInit(void);
extern void IRO_RewriteBitFieldTemps(void);
extern void IRO_ScalarizeClassDataMembers(void);

#ifdef __cplusplus
}
#endif

#pragma pack(pop)
#endif
#else
#ifndef COMPILER_IROVARS_H
#define COMPILER_IROVARS_H

#include "compiler/common.h"
#include "compiler/enode.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !MWCCPSP
#pragma pack(push, 2)
#endif
#if MWCCPSP
// 3.0.1: 0x28 bytes, natural alignment (IRO_FindVar 0x55A2C0)
struct VarRecord {
    SInt32 index; // +0
    Object* object; // +4
    void* unused; // +8: 3.0.1 keeps the lifetime vector here (func_00594C60)
    Boolean inductionState; // +0xC (TRUE when created)
    Boolean noregister; // +0xD
    Boolean usedAtCall; // +0xE
    VarRecord* next; // +0x10
    IRODef* defs; // +0x14
    IROUse* uses; // +0x18
    TypeBitfield* bitfieldType; // +0x1C
    IROLinear* bitFieldReplacement; // +0x20
    void* unk24; // +0x24: IroNonRegLoopAccesses.c's record of the variable
};
#else
struct VarRecord {
    UInt16 index;
    Object* object;
    int unused;
    Boolean inductionState;
    Boolean noregister;
    Boolean usedAtCall;
    VarRecord* next;
    IRODef* defs;
    IROUse* uses;
    TypeBitfield* bitfieldType;
    IROLinear* bitFieldReplacement;
};
#endif
#if !MWCCPSP
#pragma pack(pop)
#endif

extern VarRecord* IRO_FirstVar;
extern SInt32 IRO_NumVars;
extern Boolean IRO_IsBitField;
extern SInt32 IRO_BaseTerms;
extern SInt32 IRO_VarTerms;
#if MWCCPSP
extern Boolean IRO_IsModifyOp[IRO_MAXEXPR];
extern Boolean IRO_IsAssignOp[IRO_MAXEXPR];
#else
extern Boolean IRO_IsModifyOp[MAXEXPR];
extern Boolean IRO_IsAssignOp[MAXEXPR];
#endif

extern void IRO_InitializeIRO_IsModifyOpArray(void);
extern void IRO_InitializeIRO_IsAssignOpArray(void);
extern VarRecord* IRO_FindVar(Object* object, Boolean create, Boolean allowRegister);
extern void ResetVars(void);
extern void IRO_FindAllVars(void);
extern void IRO_ZapVarPtrs(void);
extern void IRO_UpdateVars(void);
extern void IRO_AddElmToList(IROLinear* linear, IROElmList** list);
extern void IRO_DecomposeAddressExpression(IROLinear* linear, IROAddrRecord* rec);
extern void IRO_DecomposeAddressExpression_Cheap(IROLinear* linear);
extern VarRecord* IRO_FindAssigned(IROLinear* linear);
extern void IRO_GetKills(IROLinear* linear);
extern void IRO_CheckInit(void);
extern void IRO_RewriteBitFieldTemps(void);
extern void IRO_ScalarizeClassDataMembers(void);

#ifdef __cplusplus
}
#endif

#endif
#endif
