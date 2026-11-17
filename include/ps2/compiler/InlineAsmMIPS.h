#ifndef COMPILER_INLINEASMMIPS_H
#define COMPILER_INLINEASMMIPS_H

#include "compiler/common.h"
#include "compiler/CInt64.h"
#include "compiler/InlineAsm.h"

typedef struct InlineAsmMIPSRecord InlineAsmMIPSRecord;
typedef struct InlineAsmMIPSOperand InlineAsmMIPSOperand;
typedef IALookupResult InlineAsmMIPSLookupResult;

struct InlineAsmMIPSOperand {
    UInt8 kind;
    UInt8 flags;
    UInt16 effect;
    union {
        UInt32 value;
        CLabel* label;
    };
    union {
        UInt32 unk8;
        Object* object;
        struct {
            UInt16 operandMode;
            SInt16 registerClass;
        } registerOptions;
    };
    union {
        UInt32 unkC;
        UInt8 componentMask;
        struct {
            UInt16 operandMode;
            SInt16 registerClass;
        } extendedRegisterOptions;
        struct {
            SInt16 effect;
            SInt16 rclass;
        } reg;
    };
    union {
        UInt32 unk10;
        UInt8 extendedComponentMask;
    };
};

struct InlineAsmMIPSRecord {
    UInt32 opcode;
    UInt16 flags;
    SInt16 argcount;
    InlineAsmMIPSOperand args[0];
};

typedef struct InlineAsmMIPSOptionsFrame InlineAsmMIPSOptionsFrame;
struct InlineAsmMIPSOptionsFrame {
    InlineAsmMIPSOptionsFrame* previous;
    UInt8 reorder;
    UInt8 volatileasm;
    UInt8 macro;
    UInt8 noat;
    UInt16 flags;
};

#ifdef __cplusplus
extern "C" {
#endif

extern UInt8 cmach_context_flag_63736A;
extern UInt8 asm_flag_637388;
extern SInt16 cg_codelist_reg_kind_637332;
extern UInt8 cg_codegen_emit_flag_63735A;
extern PCode* asm_pending_branch_63603C;
extern PCodeBlock* cg_codegen_cursor_636018;
extern InlineAsmMIPSOptionsFrame* asm_option_stack_608DD0;
extern PCode* fn_00491320(InlineAsmMIPSRecord* ia, int argcount);

extern const char* InlineAsm_GetMnemonic(const SInt32* opcode);
extern void InlineAsm_TranslateIRtoPCode(Statement* stmt);
extern void InlineAsm_ScanAssemblyInstruction(void);
extern void InlineAsm_ScanAssemblyDirective(void);
extern void InlineAsm_Initialize(void);
extern InlineAsmMIPSRecord* fn_004946C0(UInt16 opcode);
extern void fn_00494EC0(InlineAsmMIPSOperand* operand, SInt32 label_allowed);
extern void fn_004944A0(char* name);
extern void fn_00495730(InlineAsmMIPSOperand* operand, UInt8 allow_immediate, SInt16 effect);
extern void fn_004950D0(
    InlineAsmMIPSOperand* operand, SInt8 register_type, SInt32 mode, SInt32 effect
);
extern void fn_00495930(InlineAsmMIPSOperand* operand);
extern void fn_00495A00(InlineAsmMIPSOperand* operand, SInt32 low, SInt32 high);
extern UInt8 asm_instruction_suffix_63737D;
extern UInt8 asm_instruction_extra_6373A9;
extern UInt8 asm_lexer_p_63739F;
extern UInt8 inline_asm_setjmp_valid;

extern SInt16 asm_option_flags_637336;
extern UInt8 asm_macro_flag_63735B;
extern UInt8 asm_noat_flag_637374;
extern double asm_tkfloat_623D58;

#ifdef __cplusplus
}
#endif

#endif
