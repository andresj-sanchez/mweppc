#ifndef COMPILER_MIPS_H
#define COMPILER_MIPS_H

#include "compiler/common.h"
#include "compiler/CInt64.h"

extern void emit_branch_with_two_operands(
    UInt16 opcode, Operand* a, Operand* b, CLabel* label
); //! @reconstructed
extern void emit_branch_with_immediate(
    UInt16 opcode, Operand* op, SInt32 value, CLabel* label
); //! @reconstructed
extern void emit_branch_type_constant(
    Operand* op, CInt64* value, Type* type, CLabel* label
); //! @reconstructed
extern void emit_type_constant_comparison(
    Operand* result, CInt64* value, Type* type, CLabel* label
); //! @reconstructed
extern Operand emit_operand_to_register(short opcode, Operand* op, char kind); //! @reconstructed
extern void fn_00486660(UInt16 opcode, Operand* src, Operand* dst);
extern Operand emit_mfc1(Operand* op); //! @reconstructed
extern Operand emit_mfc1_paired(Operand* op); //! @reconstructed
extern Operand emit_mtc1(Operand* op); //! @reconstructed
extern Operand emit_mtc1_paired(Operand* op); //! @reconstructed
extern int parse_mips_operand_effects(const char* format); //! @reconstructed
extern int count_mips_operands(const char* format); //! @reconstructed
extern Operand make_paired_register_operand(
    int opcode, int float_reg, Boolean paired
); //! @reconstructed
extern void emit_paired_registers(
    Operand operand, int opcode, int low, Boolean both
); //! @reconstructed
extern void emit_branch_to_label(UInt16 opcode, CLabel* label); //! @reconstructed
extern Operand emit_immediate_with_mode(
    UInt16 opcode, Operand* op, SInt32 value, int mode
); //! @reconstructed
extern void emit_branch_to_pclabel(UInt16 opcode, PCodeLabel* label); //! @reconstructed
extern void emit_branch(UInt16 opcode, Operand* op, CLabel* label);
//! @reconstructed
extern Operand emit_or_immediate_operand(Operand* op, SInt32 value); //! @reconstructed
extern void emit_branch_to_label_lazy(UInt16 opcode, CLabel* label); //! @reconstructed
extern Operand emit_extend(UInt16 opcode, Operand* op, int width); //! @reconstructed
extern Operand emit_shift_immediate(short opcode, Operand* op, int count);
//! @reconstructed
extern Operand emit_immediate(UInt16 opcode, Operand* op, SInt32 value);
//! @reconstructed
extern void emit_branch_if_not_equal(Operand* op, SInt32 value, CLabel* label);
//! @reconstructed
extern void emit_branch_if_equal(Operand* op, SInt32 value, CLabel* label);
//! @reconstructed
extern Operand emit_immediate_cint(UInt16 opcode, Operand* op, CInt64 value);
//! @reconstructed
extern Operand emit_shift_left(Operand* op, int count, Type* type);
//! @reconstructed
extern Operand emit_shift_right(Operand* op, int count, Type* type);
//! @reconstructed
extern Operand emit_unsigned_less_equal_immediate(Operand* op, SInt32 bound); //! @reconstructed
extern Operand emit_signed_less_equal_immediate(Operand* op, SInt32 bound); //! @reconstructed
extern Operand make_constant_operand(SInt32 value); //! @reconstructed
extern Operand xor_operand_constant(Operand* value, int mask); //! @reconstructed
extern Operand emit_equal_constant(Operand* value, int expected); //! @reconstructed
extern int load_constant(SInt32 value); //! @reconstructed
extern Operand emit_unary(UInt16 opcode, Operand* op, char kind); //! @reconstructed
extern Operand emit_binary(UInt16 opcode, Operand* a, Operand* b, char kind);
//! @reconstructed
extern Operand copy_to_new_register(Operand* op); //! @reconstructed

extern Operand fn_0048BEB0(Operand* op, SInt32 mask);
extern Operand fn_0048E350(CInt64 value, Type* type);
extern int fn_0048ECB0(Type* type);
extern int fn_0048E9D0(Type* type, int* isFloat);

extern UInt16 mips_opcode_sequence_625CD0[];
extern char mips_source_filename[];
extern const char* mips_macro_format_636A08[];

extern Operand fn_00489D00(Operand* op, CInt64* value, Type* type);
extern Operand fn_00489D40(Operand* op, CInt64* value, Type* type);
extern Operand fn_00489D80(Operand* op, CInt64* value, Type* type);
extern Operand fn_00489DC0(Operand* op, CInt64* value, Type* type);
extern Operand fn_0048A4F0(Operand* op, CInt64* value, Type* type);
extern Operand fn_0048ABE0(Operand* op, CInt64* value, Type* type);
extern Operand fn_0048AD30(Operand* op, CInt64* value, Type* type);
extern Operand fn_0048AE90(Operand* op, CInt64* value, Type* type);
extern Operand fn_0048B640(Operand* op, CInt64* value, Type* type);

extern Operand fn_00489E00(Operand* op, CInt64* value, Type* type);
extern Operand fn_0048C5B0(Operand* op, CInt64* value, Type* type);

extern Operand fn_0048D270(Operand* op);
extern void fn_00487D10(Operand* op, CInt64* value, Type* type, CLabel* label);
extern Operand fn_0048C8C0(Operand* op, SInt32 value, Type* type);

extern void fn_00485D90(
    UInt32 opcode,
    const char* name,
    const char* format,
    UInt32 encoding,
    UInt32 flags,
    const char* intrinsicName,
    UInt32 otherFlags,
    UInt16 field2C
);
extern void fn_00485CF0(
    SInt32 opcode,
    const char* name,
    const char* format,
    UInt16 macro1,
    UInt16 macro2,
    UInt16 field2C
);
#endif
