#ifndef COMPILER_USEDEFCHAINS_H
#define COMPILER_USEDEFCHAINS_H

#include "compiler/common.h"
#include "compiler/BitVectors.h"
#include "compiler/PCodeInfo.h"
#include "compiler/Registers.h"
#include "compiler/GCCInlineAsm.h"
typedef UInt8 PCOpKind;

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 2)
typedef struct TinyValue {
    PCOpKind kind;
    SInt8 arg;
    union {
        short reg;
        Object* object;
    } u;
} TinyValue;
#pragma pack(pop)

struct UseOrDef {
    PCode* pcode;
    TinyValue v;
};

struct RegUseOrDef {
    RegUseOrDef* next;
    int id;
};

struct ObjectUseDef {
    ObjectUseDef* next;
    ObjectUseDef* leftchild;
    ObjectUseDef* rightchild;
    Object* object;
    RegUseOrDef* uses;
    RegUseOrDef* defs;
    short directdefs;
    short directuses;
    short indirectdefs;
    short indirectuses;
};

typedef struct UseDefInfo {
    SInt32 usestart;
    SInt32 usecount;
    SInt32 defstart;
    SInt32 defcount;
    BitVectorRange* defgen;
    CompressedBitVector* defkill;
    UInt32* defin;
    UInt32* defout;
    BitVectorRange* usegen;
    CompressedBitVector* usekill;
    UInt32* usein;
    UInt32* useout;
} UseDefInfo;

extern int number_of_Defs;
extern UseOrDef* Defs;
extern RegUseOrDef** reg_Defs[RegClassMax];
extern int number_of_Uses;
extern UseOrDef* Uses;
extern RegUseOrDef** reg_Uses[RegClassMax];
extern UseDefInfo* usedefinfo;
extern ObjectUseDef* objectusedefs;
extern ObjectUseDef* objectusedeflist;

extern SInt32 fn_0054B940(
    PCode* def_pcode, SInt32 defID, SInt32 reg, PCode* use_pcode, UInt32* defin, SInt32 rclass
);

extern ObjectUseDef* findobjectusedef(Object* object);
extern void computeusedefchains(int includeMemory);
extern int precedes(PCode* a, PCode* b);
extern int canidate_reaches_use(
    PCode* candidate, int defID, int reg, PCode* use, UInt32* defvec, RegClass rclass
);

extern SInt32 last_usedef_register[7];
extern Statement* first_ST_ASM;

#ifdef __cplusplus
}
#endif
extern void pruneusedefchains(SInt32 recompute); //! @reconstructed

extern void computeglobaldefuseinfo(void);
extern void computeglobalusedefinfo(void);
extern void computelocalusedefinfo(void);
extern void computeusedeflists(SInt32 includeMemory);
extern void precomputeusedefcounts(SInt32 includeMemory);
extern void precomputememoryusedefs(void);

#endif
