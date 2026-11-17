#ifndef COMPILER_IROVARS_H
#define COMPILER_IROVARS_H

#include "compiler/common.h"

struct VarRecord {
    UInt32 index; /* 0x00 */
    Object* object; /* 0x04 */
    int unused; /* 0x08 */
    Boolean inductionState; /* 0x0C */
    Boolean noregister; /* 0x0D */
    Boolean usedAtCall; /* 0x0E */
    VarRecord* next; /* 0x10 */
    IRODef* defs; /* 0x14 */
    IROUse* uses; /* 0x18 */
    void* bitfieldType; /* 0x1C */
    IROLinear* bitFieldReplacement; /* 0x20 */
    struct NonRegCandidate* candidate; /* 0x24 */
};

extern VarRecord* IRO_FirstVar;

extern VarRecord* IRO_FindVar(Object* object, Boolean create, Boolean allowRegister);
extern Boolean IRO_IsRegable(Object* obj);
extern void IRO_ZapVarPtrs(void);

extern VarRecord* IRO_FindAssigned(IROLinear*);
extern SInt32 IRO_NumVars;
extern Boolean IRO_IsModifyOp[124];

#endif
