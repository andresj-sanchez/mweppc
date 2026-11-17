#ifndef COMPILER_GCCINLINEASM_H
#define COMPILER_GCCINLINEASM_H

#include "compiler/common.h"
#include "compiler/CInt64.h"

struct GCCNameInfo {
    char* name;
    SInt32 capacity;
    SInt32 count;
    SInt32* operands;
};

struct GCCNameEntry {
    GCCNameEntry* next;
    GCCNameInfo info;
    UInt32 unk14;
    UInt32 unk18;
};

struct GCCStaticInfo {
    char* name;
    SInt32 value;
    SInt32 rclass;
    union {
        SInt32 reg;
        Object* object;
    };
};

struct GCCStaticEntry {
    GCCStaticEntry* next;
    GCCStaticInfo info;
};

struct GCCAsmOperand {
    HashNameNode* name;
    UInt32 unknown4;
    Object* object;
    UInt32 unknownC;
    CInt64 immediate;
    Boolean isImmediate;
    UInt8 unknown21[3];
};

struct GCCAsmConstraint {
    SInt8 registerClass;
    UInt8 kind;
    UInt16 unknown2;
    UInt32 effects;
};

#ifdef __cplusplus
extern "C" {
#endif

extern Statement* first_ST_ASM;
extern GCCAsmOperand gcc_asm_operands[30];
extern struct GCCAsmConstraint gcc_asm_constraints[30];
extern SInt32 gcc_asm_operand_count;
extern Type* gcc_asm_vector_type;
extern GCCNameEntry* gcc_names_hash[256];
extern GCCStaticEntry* gcc_statics_hash[512];
extern char tk_stmt_name_5EA32B[];
extern char tk_stmt_name_5EA329[];
extern GCCNameInfo* gcc_find_name(char* name); //! @reconstructed
extern GCCStaticInfo* gcc_find_value(char* name); //! @reconstructed
extern void gcc_add_static(char* name, SInt16 value, SInt16 rclass, SInt32 reg);
//! @reconstructed
extern void gcc_clear_statics(void); //! @reconstructed
extern void gcc_clear_name_tables(void); //! @reconstructed
extern GCCNameInfo* gcc_add_value(char* name, SInt32 value); //! @reconstructed
extern void InlineAsm_gcc_parse(void);
extern void gcc_replace_arg_st_asm(Statement* stmt);
extern void gcc_allocate_operand(SInt32 mode, SInt32 kindreg, SInt32 effects, SInt32 index);
//! @reconstructed
extern void InlineAsm_SkipComment(void); //! @reconstructed

#ifdef __cplusplus
}
#endif

#endif
