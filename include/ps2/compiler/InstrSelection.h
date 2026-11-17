#ifndef COMPILER_INSTRSELECTION_H
#define COMPILER_INSTRSELECTION_H

#include "compiler/common.h"
#include "compiler/CInt64.h"

struct Operand {
    UInt8 kind;
    UInt8 unk1;
    UInt16 reg;
    UInt16 regHi;
    UInt16 regOffset;
    SInt32 immOffset;
    CInt64 immediate;
    Object* object;
    union {
        SInt32 unk20;
        TypeBitfield* bitfield;
    };
    UInt32 flags;
    SInt32 unk28;
};

typedef struct ConstInfo2 {
    SInt32 multiplier;
    SInt32 shift;
} ConstInfo2;

typedef struct DivisionParameters {
    SInt32 multiplier;
    SInt32 shift;
    SInt32 addIndicator;
} DivisionParameters;

extern void (*gen_func_table[])(ENode* node, short reg, Operand* result);
//! @reconstructed

typedef struct FunctionCallFrame {
    struct FunctionCallFrame* next;
    Type* functionType;
    Operand operand;
} FunctionCallFrame;

extern FunctionCallFrame* function_call_frames;

extern ObjectList* dollar_name_objects; //! @reconstructed

extern int I8_getbit(unsigned long long value);
extern void generate_longlong_multiply(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void generate_comparison_value(ENode* node, Operand* result);
//! @reconstructed
extern void emit_conditional_funccall(ENode* node, short reg, Operand* result); //! @reconstructed
extern void get_function_type_operand(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void generate_boolean_expression(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void generate_logical_not(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void generate_comparison(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void emit_binary_fpr_instruction(short opcode, ENode* left, ENode* right, Operand* result);
//! @reconstructed
extern Boolean signed_div_pow2(Operand* x, int n, int negate, Operand* result, Type* type);
//! @reconstructed
extern void shift_right_imm(Operand* x, Type* type, short count, Operand* result);
//! @reconstructed
extern void select_unexpected_node(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void make_objref_operand(ENode* node, short reg, Operand* result);
//! @reconstructed
extern Operand make_object_operand(Object* object); //! @reconstructed
extern void generate_intrinsic_or_function_call(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void select_string_constant(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void generate_bitfield_operand(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void select_diadic_left_then_right(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void select_or(ENode* node, short reg, Operand* result); //! @reconstructed
extern void gen_xor(ENode* node, short reg, Operand* result); //! @reconstructed
extern void emit_bitwise_and(ENode* node, short reg, Operand* result);
//! @reconstructed
extern Operand emit_logical_operation(
    Type* type, ENode* left, ENode* right, int opcode, int immediate_opcode
);
//! @reconstructed
extern void emit_multiply(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void force_monadic_operand_register(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void generate_preinc_predec(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void generate_postinc_postdec(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void generate_increment(ENode* node, Operand* before, Operand* after);
//! @reconstructed
extern void get_objaccess_cached_value(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void get_dispatch_result(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void generate_block_assignment(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void shift_left_imm(Operand* x, Type* type, short count, short negate, Operand* result);
//! @reconstructed
extern void generate_condition_branches(
    ENode* expr, CLabel* trueLabel, CLabel* falseLabel, Boolean trueFollows, Boolean falseFollows
);
extern void make_intval_operand(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void select_indirect_operand(ENode* node, short reg, Operand* result);
//! @reconstructed
extern Object* get_float_compare_function(int op, Type* type); //! @reconstructed
extern void get_float_compare_ops(
    int op, Type* type, UInt16* branch, UInt16* compare, Boolean invert
);
//! @reconstructed
extern void generate_comparison_branch(ENode* expr, CLabel* label, int branchIfTrue);
//! @reconstructed
extern void generate_left_shift(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void select_right_shift(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void emit_three_fpr_operands(
    short opcode, ENode* left, ENode* right, ENode* third, short reg, Operand* result
); //! @reconstructed
extern void generate_modulo(ENode* node, short reg, Operand* result);
//! @reconstructed
extern void generate_division(ENode* node, short reg, Operand* result);
//! @reconstructed
extern Operand signed_division_by_constant(Operand* x, SInt32 divisor);
//! @reconstructed
extern Operand unsigned_division_by_constant(Operand* x, UInt32 divisor);
//! @reconstructed

extern void generate_operands(ENode* left, ENode* right, Operand* lop, Operand* rop, int force);
//! @reconstructed

extern Operand fn_004A8E20(short opcode, ENode* node);
extern Operand fn_004A8EC0(short regopcode, short immopcode, ENode* left, ENode* right);

extern CInt64 cint64_5e8b50;
extern CInt64 cint64_5e8b60;
extern CInt64 cint64_5e8b70;

extern void fn_004A5CA0(Operand* left, Operand* right);
extern void fn_004A5EE0(Operand* left, Operand* right);

#endif
