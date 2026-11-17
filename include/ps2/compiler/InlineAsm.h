#ifndef COMPILER_INLINEASM_H
#define COMPILER_INLINEASM_H

#include "compiler/common.h"

typedef enum {
    IAEffect_Read,
    IAEffect_Write,
    IAEffect_Clobber,
    IAEffect_AddressOf,
    IAEffect_ReadWrite
} IAEffectType;
struct IAEffect {
    IAEffectType type;
    Object* object;
    SInt32 offset;
    SInt32 size;
};
struct IAEffects {
    UInt8 optimizationBarrier, writesMemory, readsMemory, unmodeledControlFlow, branchWithLink,
        noFallthrough, unk6, unk7, unk8;
    SInt32 numoperands, numlabels;
    IAEffect operands[16];
    CLabel* labels[16];
};
#include <setjmp.h>
#include "compiler/CInt64.h"

extern UInt8 inline_asm_setjmp_valid;
extern SInt32 inline_asm_backtracking;
extern jmp_buf inline_asm_backtrack_context;

typedef struct IALookupResult IALookupResult;
struct IALookupResult {
    HashNameNode* name;
    CLabel* label;
    Object* object;
    Type* type;
    CInt64 value;
    UInt8 has_value;
};

extern Boolean InlineAsm_LookupSymbolOrTag(HashNameNode* name, IALookupResult* result);

extern void AssemblerError(void);

extern char* InlineAsm_DumpStatement(Statement* stmt);
extern CLabel* InlineAsm_GetReferencedLabel2(Statement* stmt);
extern void InlineAsm_CheckLocalUsage(Statement* stmt);
extern void InlineAsm_Assemble(void);
extern void InlineAsm_ScanOptionalLabel(void); //! @reconstructed
extern HashNameNode* MakeLocalLabel(CInt64 num); //! @reconstructed
extern CInt64 InlineAsm_ConstantExpression64bit(void);
extern CInt64 InlineAsm_ConstantExpressionTail(CInt64 value); //! @reconstructed
extern CInt64 InlineAsm_PrimaryExpression(void); //! @reconstructed
extern CLabel* InlineAsm_DeclareLabel(HashNameNode* name);
extern CLabel* InlineAsm_LookupLabel(HashNameNode* name); //! @reconstructed

extern void* InlineAsm_GetReferencedLabelIndexed(void* p, SInt32 index, SInt32* offset);
extern void InlineAsm_UnpackAsmStatement(
    Statement* stmt, CLabel** labelArray, Boolean inlineCopy, void* data, SInt32 size
);
extern void InlineAsm_PackAsmStatement(
    Statement* stmt, Statement* first, void** output, SInt32* outsize
);
extern void InlineAsm_ScanStatements(volatile SInt16 endToken); //! @reconstructed
extern SInt32 InlineAsm_ConstantExpression32bit(void);
extern SInt32 InlineAsm_StructArrayMemberOffset(Type* type);
extern SInt32 InlineAsm_StructMemberOffset(Type* type);
extern void InlineAsm_SyntaxError(SInt16 code);

extern SInt32 ASMstmtnb;
extern UInt8 inline_asm_saved_slash_comment;
extern char InlineAsm_dumpBuffer[];
extern const char InlineAsm_file_name[];
extern const char InlineAsm_fmt_imm[];
extern const char InlineAsm_fmt_lab[];
extern const char InlineAsm_fmt_obj[];
extern const char InlineAsm_fmt_objnegative[];
extern const char InlineAsm_fmt_objpositive[];
extern const char InlineAsm_fmt_quote[];
extern const char InlineAsm_fmt_reg[];
extern const char InlineAsm_fmt_regnum[];
extern const char inline_asm_local_label_format[];

extern jmp_buf inline_asm_jump_context;
extern SInt32 inline_asm_allow_array_expressions;
extern UInt8 register_slash_comment_enabled;

#endif
