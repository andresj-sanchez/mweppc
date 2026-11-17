#include "compiler/BE_binary.h"
#include "compiler/Registers.h"
#include "compiler/CParser.h"

#include "compiler/InstrSelection.h"
#include "compiler/CError.h"
#include "compiler/CFunc.h"
#include "compiler/CInt64.h"
#include "compiler/CMachine.h"
#include "compiler/CMangler.h"
#include "compiler/CodeGen.h"
#include "compiler/CompilerTools.h"
#include "compiler/DIVbyCnst.h"
#include "compiler/DWARF.h"
#include "compiler/enode.h"
#include "compiler/FunctionCalls.h"
#include "compiler/Intrinsics.h"
#include "compiler/mips.h"
#include "compiler/MIPSError.h"
#include "compiler/objects.h"
#include "compiler/Operands.h"
#include "compiler/PCode.h"
#include "compiler/PCodeInfo.h"
#include "compiler/PCodeUtilities.h"
#include "compiler/RegisterInfo.h"
#include "compiler/StructMoves.h"
#include "compiler/types.h"
#include "compiler/VReg.h"
#include <string.h>

//! @reconstructed
static inline int I8_getbit_if_32bit(CInt64 value) {
    int result;

    if (CInt64_IsInRange(value, 4)) {
        result = getbit(value.lo);
    } else {
        result = -2;
    }
    return result;
}

static inline void emit_binary_gpr_instruction(
    short opcode, ENode* left, ENode* right, Operand* result
) {
    Operand lop;
    Operand rop;

    generate_operands(left, right, &lop, &rop, 0);
    if (lop.kind) {
        Coerce_to_register(&lop, left->rtype, 0);
    }
    if (rop.kind) {
        Coerce_to_register(&rop, right->rtype, 0);
    }
    emitpcode(opcode, used_virtual_registers[RegClass_GPR]++, lop.reg, rop.reg);
    *result = operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
}

void generate_operands(ENode* left, ENode* right, Operand* lop, Operand* rop, int force) {
    if (right->cost > left->cost + 2) {
        gen_func_table[right->type](right, 0, rop);
        if (force && rop->kind) {
            Coerce_to_register(rop, right->rtype, 0);
        }
        gen_func_table[left->type](left, 0, lop);
        if (force && lop->kind) {
            Coerce_to_register(lop, left->rtype, 0);
        }
    } else {
        gen_func_table[left->type](left, 0, lop);
        if (force && lop->kind) {
            Coerce_to_register(lop, left->rtype, 0);
        }
        gen_func_table[right->type](right, 0, rop);
        if (force && rop->kind) {
            Coerce_to_register(rop, right->rtype, 0);
        }
    }
}

Operand fn_004A8EC0(short regopcode, short immopcode, ENode* left, ENode* right) {
    Operand rop;
    Operand lop;
    int shift;

    memset(&rop, 0, sizeof(rop));
    memset(&lop, 0, sizeof(lop));
    generate_operands(left, right, &lop, &rop, 0);
    if (lop.kind) {
        Coerce_to_register(&lop, left->rtype, 0);
    }
    if (rop.kind == 3) {
        emitpcode(immopcode, used_virtual_registers[RegClass_FPR]++, lop.reg, rop.immediate.lo, 15);
    } else {
        if (rop.kind) {
            Coerce_to_register(&rop, right->rtype, 0);
        }
        shift = rop.unk28 ? rop.unk28 - 1 : 11;
        emitpcode(regopcode, used_virtual_registers[RegClass_FPR]++, lop.reg, rop.reg, shift);
    }
    return operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
}

Operand fn_004A8E20(short opcode, ENode* node) {
    Operand op;

    memset(&op, 0, sizeof(op));
    gen_func_table[node->type](node, 0, &op);
    if (op.kind) {
        Coerce_to_register(&op, node->rtype, 0);
    }
    emitpcode(opcode, used_virtual_registers[RegClass_FPR]++, op.reg);
    return operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
}

void get_float_compare_ops(int op, Type* type, UInt16* branch, UInt16* compare, Boolean invert) {
    Boolean single;

    single = IS_TYPE_FLOAT32(type);
    CError_ASSERT(403, !IS_TYPE_FLOAT_SOFTWARE(type));
    switch (op) {
        case EGREATER:
            *compare = PC_C_LE_S;
            *branch = invert ? PC_BC1F : PC_BC1T;
            return;
        case ELESS:
            *compare = single ? PC_C_LT_S : PC_C_LT_D;
            *branch = invert ? PC_BC1T : PC_BC1F;
            return;
        case EGREATEREQU:
            *compare = PC_C_LT_S;
            *branch = invert ? PC_BC1F : PC_BC1T;
            return;
        case ELESSEQU:
            *compare = single ? PC_C_LE_S : PC_C_LE_D;
            *branch = invert ? PC_BC1T : PC_BC1F;
            return;
        case EEQU:
            *compare = single ? PC_C_EQ_S : PC_C_EQ_D;
            *branch = invert ? PC_BC1T : PC_BC1F;
            return;
        case ENOTEQU:
            *compare = single ? PC_C_EQ_S : PC_C_EQ_D;
            *branch = invert ? PC_BC1F : PC_BC1T;
            return;
        default:
            CError_FATAL(448);
    }
}

Object* get_float_compare_function(int op, Type* type) {
    Boolean single;

    single = IS_TYPE_FLOAT32(type);
    CError_ASSERT(456, !IS_TYPE_FLOAT_HARDWARE(type));
    switch (op) {
        case EGREATER:
            return single ? rt_f_fgt : rt_dpfgt;
        case ELESS:
            return single ? rt_f_flt : rt_dpflt;
        case EGREATEREQU:
            return single ? rt_f_fge : rt_dpfge;
        case ELESSEQU:
            return single ? rt_f_fle : rt_dpfle;
        case EEQU:
            return single ? rt_f_feq : rt_dpfeq;
        case ENOTEQU:
            return single ? rt_f_fne : rt_dpfne;
        default:
            CError_FATAL(465);
            return NULL;
    }
}

void generate_longlong_multiply(ENode* node, short reg, Operand* result) {
    ENode* left;
    ENode* right;
    ENode* effective;
    Operand lop;
    Operand rop;
    Type* type;
    Type* narrowtype;
    int left_size;
    int right_size;
    int left_signed;
    int right_signed;
    int left_shift = 0;
    int right_shift = 0;
    int dest;
    int destHi;
    unsigned long long left_value = 0;
    unsigned long long right_value = 0;

    left = node->data.diadic.left;
    right = node->data.diadic.right;
    type = node->rtype;
    CError_ASSERT(0x12D9, right->type != EINTCONST || left->type != EINTCONST);
    memclrw(&lop, sizeof(Operand));
    memclrw(&rop, sizeof(Operand));
    effective = left->type == EASS ? left->data.diadic.right : left;
    if (effective->type == ETYPCON) {
        narrowtype = effective->data.monadic->rtype;
        if (narrowtype->size < effective->rtype->size &&
            narrowtype->type != TYPEFLOAT &&
            is_unsigned(narrowtype) == is_unsigned(effective->rtype)) {
            left_size = narrowtype->size;
        } else {
            left_size = effective->rtype->size;
        }
    } else {
        left_size = effective->rtype->size;
    }
    effective = right->type == EASS ? right->data.diadic.right : right;
    if (effective->type == ETYPCON) {
        narrowtype = effective->data.monadic->rtype;
        if (narrowtype->size < effective->rtype->size &&
            narrowtype->type != TYPEFLOAT &&
            is_unsigned(narrowtype) == is_unsigned(effective->rtype)) {
            right_size = narrowtype->size;
        } else {
            right_size = effective->rtype->size;
        }
    } else {
        right_size = effective->rtype->size;
    }
    if (left->type == EINTCONST) {
        left_value = ((unsigned long long)(left_size >= 8 ? left->data.intval.unk8 : 0) << 32) |
            (UInt32)left->data.intval.lo;
    }
    if (right->type == EINTCONST) {
        right_value = ((unsigned long long)(right_size >= 8 ? right->data.intval.unk8 : 0) << 32) |
            (UInt32)right->data.intval.lo;
    }
    if (right->hascall) {
        gen_func_table[right->type](right, 0, &rop);
        if (rop.kind) {
            Coerce_to_register(&rop, right->rtype, 0);
        }
        if (left->type == EINTCONST) {
            left_shift = I8_getbit(left_value);
        }
        if (left->type != EINTCONST || left_shift <= 0) {
            gen_func_table[left->type](left, 0, &lop);
            if (lop.kind) {
                Coerce_to_register(&lop, left->rtype, 0);
            }
        }
    } else {
        if (left->type == EINTCONST) {
            left_shift = I8_getbit(left_value);
        }
        if (left->type != EINTCONST || left_shift <= 0) {
            gen_func_table[left->type](left, 0, &lop);
            if (lop.kind) {
                Coerce_to_register(&lop, left->rtype, 0);
            }
        }
        if (right->type == EINTCONST) {
            right_shift = I8_getbit(right_value);
        }
        if (right->type != EINTCONST || right_shift <= 0) {
            gen_func_table[right->type](right, 0, &rop);
            if (rop.kind) {
                Coerce_to_register(&rop, right->rtype, 0);
            }
        }
    }
    left_signed = !is_unsigned(left->rtype);
    right_signed = !is_unsigned(right->rtype);
    if (left_size < right_size) {
        ENode* swapnode = left;
        Operand swapop = lop;
        int swapsize = left_size;
        int swapsign = left_signed;
        unsigned long long swapvalue = left_value;
        int swapshift = left_shift;
        node->data.diadic.left = right;
        node->data.diadic.right = left;
        left = right;
        right = swapnode;
        lop = rop;
        rop = swapop;
        left_size = right_size;
        right_size = swapsize;
        left_signed = right_signed;
        right_signed = swapsign;
        left_value = right_value;
        right_value = swapvalue;
        left_shift = right_shift;
        right_shift = swapshift;
    }
    dest = used_virtual_registers[RegClass_GPR]++;
    destHi = used_virtual_registers[RegClass_GPR]++;
    if (left->type == EINTCONST && left_shift > 0) {
        emitpcode(0x88, dest, rop.reg, left_shift);
        emitpcode(0x48B, destHi, dest, 32);
    } else if (right->type == EINTCONST && right_shift > 0) {
        emitpcode(0x88, dest, lop.reg, right_shift);
        emitpcode(0x48B, destHi, dest, 32);
    } else {
        switch (left_size + right_size) {
            case 2:
            case 3:
            case 4:
                emitpcode(0x228, dest, lop.reg, rop.reg);
                emitpcode(0x48C, destHi, dest, 32);
                break;
            case 5:
            case 6:
            case 8:
                emitpcode(0x228, dest, lop.reg, rop.reg);
                emitpcode(left_signed || right_signed ? 0x228 : 0x228, destHi, lop.reg, rop.reg);
                break;
            default:
                emitpcode(0x228, dest, lop.reg, rop.reg);
                emitpcode(0x85, destHi, lop.regHi, rop.regHi);
                break;
        }
    }
    *result = make_register_operand(dest, 0);
    result->regHi = destHi;
}

int I8_getbit(unsigned long long value) {
    if (value == 0) {
        return -1;
    }
    if (value == 0x1ULL) {
        return 0;
    }
    if (value == 0x2ULL) {
        return 1;
    }
    if (value == 0x4ULL) {
        return 2;
    }
    if (value == 0x8ULL) {
        return 3;
    }
    if (value == 0x10ULL) {
        return 4;
    }
    if (value == 0x20ULL) {
        return 5;
    }
    if (value == 0x40ULL) {
        return 6;
    }
    if (value == 0x80ULL) {
        return 7;
    }
    if (value == 0x100ULL) {
        return 8;
    }
    if (value == 0x200ULL) {
        return 9;
    }
    if (value == 0x400ULL) {
        return 10;
    }
    if (value == 0x800ULL) {
        return 11;
    }
    if (value == 0x1000ULL) {
        return 12;
    }
    if (value == 0x2000ULL) {
        return 13;
    }
    if (value == 0x4000ULL) {
        return 14;
    }
    if (value == 0x8000ULL) {
        return 15;
    }
    if (value == 0x10000ULL) {
        return 16;
    }
    if (value == 0x20000ULL) {
        return 17;
    }
    if (value == 0x40000ULL) {
        return 18;
    }
    if (value == 0x80000ULL) {
        return 19;
    }
    if (value == 0x100000ULL) {
        return 20;
    }
    if (value == 0x200000ULL) {
        return 21;
    }
    if (value == 0x400000ULL) {
        return 22;
    }
    if (value == 0x800000ULL) {
        return 23;
    }
    if (value == 0x1000000ULL) {
        return 24;
    }
    if (value == 0x2000000ULL) {
        return 25;
    }
    if (value == 0x4000000ULL) {
        return 26;
    }
    if (value == 0x8000000ULL) {
        return 27;
    }
    if (value == 0x10000000ULL) {
        return 28;
    }
    if (value == 0x20000000ULL) {
        return 29;
    }
    if (value == 0x40000000ULL) {
        return 30;
    }
    if (value == 0x80000000ULL) {
        return 31;
    }
    if (value == 0x100000000ULL) {
        return 32;
    }
    if (value == 0x200000000ULL) {
        return 33;
    }
    if (value == 0x400000000ULL) {
        return 34;
    }
    if (value == 0x800000000ULL) {
        return 35;
    }
    if (value == 0x1000000000ULL) {
        return 36;
    }
    if (value == 0x2000000000ULL) {
        return 37;
    }
    if (value == 0x4000000000ULL) {
        return 38;
    }
    if (value == 0x8000000000ULL) {
        return 39;
    }
    if (value == 0x10000000000ULL) {
        return 40;
    }
    if (value == 0x20000000000ULL) {
        return 41;
    }
    if (value == 0x40000000000ULL) {
        return 42;
    }
    if (value == 0x80000000000ULL) {
        return 43;
    }
    if (value == 0x100000000000ULL) {
        return 44;
    }
    if (value == 0x200000000000ULL) {
        return 45;
    }
    if (value == 0x400000000000ULL) {
        return 46;
    }
    if (value == 0x800000000000ULL) {
        return 47;
    }
    if (value == 0x1000000000000ULL) {
        return 48;
    }
    if (value == 0x2000000000000ULL) {
        return 49;
    }
    if (value == 0x4000000000000ULL) {
        return 50;
    }
    if (value == 0x8000000000000ULL) {
        return 51;
    }
    if (value == 0x10000000000000ULL) {
        return 52;
    }
    if (value == 0x20000000000000ULL) {
        return 53;
    }
    if (value == 0x40000000000000ULL) {
        return 54;
    }
    if (value == 0x80000000000000ULL) {
        return 55;
    }
    if (value == 0x100000000000000ULL) {
        return 56;
    }
    if (value == 0x200000000000000ULL) {
        return 57;
    }
    if (value == 0x400000000000000ULL) {
        return 58;
    }
    if (value == 0x800000000000000ULL) {
        return 59;
    }
    if (value == 0x1000000000000000ULL) {
        return 60;
    }
    if (value == 0x2000000000000000ULL) {
        return 61;
    }
    if (value == 0x4000000000000000ULL) {
        return 62;
    }
    if (value == 0x8000000000000000ULL) {
        return 63;
    }
    return -2;
}

void fn_0049D5F0(short opcode, ENode* left, ENode* right, Operand* result) {
    int dest;
    Operand lop;
    Type* type;
    PCodeLabel* label;
    UInt32 branch;
    UInt32 compare;
    Operand rop;
    int compareOp;

    compareOp = opcode;
    dest = used_virtual_registers[RegClass_GPR]++;
    generate_operands(left, right, &lop, &rop, 1);
    type = left->rtype;
    if (IS_TYPE_FLOAT_SOFTWARE(type)) {
        pass_argument(&lop, type, 1);
        pass_argument(&rop, right->rtype, 2);
        call_runtime_function(get_float_compare_function(compareOp, right->rtype));
        *result = get_call_result_int(0);
        return;
    }
    label = makepclabel();
    get_float_compare_ops(compareOp, right->rtype, (UInt16*)&branch, (UInt16*)&compare, TRUE);
    emitpcode(compare, lop.reg, rop.reg);
    emitpcode(move_opcode, dest, load_constant(1));
    emit_branch_to_pclabel(branch, label);
    emitpcode(move_opcode, dest, 0);
    emit_label(label);
    *result = make_register_operand(dest, 0);
}

void get_dispatch_result(ENode* node, short reg, Operand* result) {
    Operand* cachedValue;

    if (!node->data.objaccess.cachedValue) {
        cachedValue = lalloc(sizeof(Operand));
        node->data.objaccess.cachedValue = cachedValue;
        gen_func_table[node->data.objaccess.expression->type](
            node->data.objaccess.expression, 0, cachedValue
        );
    }
    *result = *node->data.objaccess.cachedValue;
}

void get_objaccess_cached_value(ENode* node, short reg, Operand* result) {
    ENode* define;
    Operand* cachedValue;

    define = node->data.monadic;
    if (!define->data.objaccess.cachedValue) {
        cachedValue = lalloc(sizeof(Operand));
        define->data.objaccess.cachedValue = cachedValue;
        gen_func_table[define->data.objaccess.expression->type](
            define->data.objaccess.expression, 0, cachedValue
        );
    }
    *result = *define->data.objaccess.cachedValue;
}

void generate_increment(ENode* node, Operand* before, Operand* after) {
    Type* type;
    ENode* inner;
    TypeBitfield* bitfield = NULL;
    Operand address;
    Operand value;
    CInt64 step;
    int increment;

    inner = node->data.monadic->data.monadic;
    type = node->rtype;
    if (IS_TYPE_FLOAT(type)) {
        gen_func_table[inner->type](inner, 0, &address);
        indirect(&address, inner);
        *before = address;
        if (before->kind) {
            Coerce_to_register(before, type, 0);
        }
        if (IS_TYPE_FLOAT_HARDWARE(type)) {
            int constant_reg = used_virtual_registers[RegClass_FPR]++;
            int result_reg = used_virtual_registers[RegClass_FPR]++;
            Operand one = make_register_operand(load_constant(0x3F800000), 0);
            emitpcode(0xAD, constant_reg, one.reg);
            emitpcode(
                IS_TYPE_FLOAT32(type)
                    ? ((node->type == EPOSTINC || node->type == EPREINC) ? 0xED : 0x103)
                    : ((node->type == EPOSTINC || node->type == EPREINC) ? 0xEC : 0x102),
                result_reg,
                before->reg,
                constant_reg
            );
            *after = operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
        } else {
            CInt64 bits = {0};
            Operand one;
            bits.unk8 = 0x3FF00000;
            one = fn_0048E350(bits, type);
            pass_argument(before, type, 1);
            pass_argument(&one, type, 2);
            call_runtime_function(
                (node->type == EPOSTINC || node->type == EPREINC)
                    ? (IS_TYPE_FLOAT32(type) ? rt_f_add : rt_dpadd)
                    : (IS_TYPE_FLOAT32(type) ? rt_f_sub : rt_dpsub)
            );
            *after = get_call_result(type);
        }
        fn_004B1440(*after, &address, type);
        return;
    }
    if (type->type == TYPEPOINTER) {
        increment = ((TypePointer*)type)->target->size;
        if (node->type != EPOSTINC && node->type != EPREINC) {
            increment = -increment;
        }
    } else if (node->type != EPOSTINC && node->type != EPREINC) {
        increment = -1;
    } else {
        increment = 1;
    }
    if (inner->type == EBITFIELD) {
        bitfield = (TypeBitfield*)inner->rtype;
        inner = inner->data.monadic;
    }
    gen_func_table[inner->type](inner, 0, &address);
    indirect(&address, inner);
    *before = address;
    value = address;
    if (value.kind) {
        Coerce_to_register(&value, type, 0);
    }
    *before = value;
    if (bitfield) {
        extract_bitfield(value, bitfield, before);
    }
    CInt64_SetLong(&step, increment);
    *after = fn_0048C5B0(before, &step, type);
    if (bitfield) {
        fn_004B0B30(*after, &value, bitfield);
        fn_004B1440(value, &address, type);
    } else {
        fn_004B1440(*after, &address, type);
    }
}

void generate_postinc_postdec(ENode* node, short reg, Operand* result) {
    Operand before;
    Operand after;

    generate_increment(node, &before, &after);
    *result = before;
}

void generate_preinc_predec(ENode* node, short reg, Operand* result) {
    Operand before;
    Operand after;

    generate_increment(node, &before, &after);
    *result = after;
}

void select_indirect_operand(ENode* node, short reg, Operand* result) {
    ENode* operand;
    Type* type;

    operand = node->data.monadic;
    gen_func_table[operand->type](operand, 0, result);
    indirect(result, node);
    if (result->flags & 0x80) {
        type = node->rtype;
        if (IS_TYPE_INT(type) ||
            IS_TYPE_ENUM(type) ||
            IS_TYPE_POINTER_ONLY(type) ||
            IS_TYPE_4BYTES_MEMBERPOINTER(type) ||
            IS_TYPE_FLOAT(type) ||
            (type &&
             (IS_TYPE_ARRAY(type) ||
              IS_TYPE_STRUCT(type) ||
              IS_TYPE_CLASS(type) ||
              IS_TYPE_12BYTES_MEMBERPOINTER(type)) &&
             type->size <= register_size)) {
            if (result->kind) {
                Coerce_to_register(result, type, 0);
            }
            type = node->rtype;
            if (type &&
                (IS_TYPE_ARRAY(type) ||
                 IS_TYPE_STRUCT(type) ||
                 IS_TYPE_CLASS(type) ||
                 IS_TYPE_12BYTES_MEMBERPOINTER(type)) &&
                operand->type == EOBJREF) {
                note_object_alignment(CMach_GetTypeAlign(operand->rtype), operand->data.objref);
            }
        }
    }
}

void force_monadic_operand_register(ENode* node, short reg, Operand* result) {
    ENode* operand;
    Type* type;

    operand = node->data.monadic;
    gen_func_table[operand->type](operand, 0, result);
    type = operand->rtype;
    if (IS_TYPE_INT(type) ||
        IS_TYPE_ENUM(type) ||
        IS_TYPE_POINTER_ONLY(type) ||
        IS_TYPE_4BYTES_MEMBERPOINTER(type) ||
        IS_TYPE_FLOAT(type)) {
        if (result->kind) {
            Coerce_to_register(result, type, 1);
        }
    }
}

void emit_multiply(ENode* node, short reg, Operand* result) {
    ENode* left;
    ENode* right;
    Type* type;
    Operand lop;
    Operand rop;

    left = node->data.diadic.left;
    type = node->rtype;
    right = node->data.diadic.right;

    if (IS_TYPE_FLOAT(type)) {
        if (IS_TYPE_FLOAT_SOFTWARE(type)) {
            generate_operands(left, right, &lop, &rop, 1);
            pass_argument(&lop, type, 1);
            pass_argument(&rop, type, 2);
            call_runtime_function(IS_TYPE_FLOAT32(type) ? rt_f_mul : rt_dpmul);
            *result = get_call_result(type);
        } else if (IS_TYPE_FLOAT64(type) && !(copts.fp_library & 2)) {
            generate_operands(left, right, &lop, &rop, 1);
            pass_argument(&lop, type, 1);
            pass_argument(&rop, type, 2);
            call_runtime_function(rt_dpmul);
            *result = get_call_result(type);
        } else {
            emit_binary_fpr_instruction(
                IS_TYPE_FLOAT32(type) ? PC_MUL_S : PC_MUL_D, left, right, result
            );
        }
        return;
    }

    if (right->type == EINTCONST && CInt64_IsInRange(right->data.intval, 4)) {
        gen_func_table[left->type](left, 0, &lop);
        if ((lop.kind >= 4 || lop.kind == 2) && lop.kind) {
            Coerce_to_register(&lop, left->rtype, 0);
        }
        if (lop.kind == 3) {
            lop.immediate = CInt64_Mul(lop.immediate, right->data.intval);
            *result = lop;
            return;
        }
        *result = multiply_by_constant(&lop, right->data.intval.lo, type);
        return;
    }
    if (left->type == EINTCONST && CInt64_IsInRange(left->data.intval, 4)) {
        gen_func_table[right->type](right, 0, &rop);
        if ((rop.kind >= 4 || rop.kind == 2) && rop.kind) {
            Coerce_to_register(&rop, right->rtype, 0);
        }
        *result = multiply_by_constant(&rop, left->data.intval.lo, type);
        return;
    }
    if (TYPE_IS_8BYTES(type) && copts.unknown25) {
        generate_longlong_multiply(node, reg, result);
        return;
    }
    if (copts.unknown26) {
        generate_operands(left, right, &lop, &rop, 1);
        pass_int_argument(&lop, 0, 1);
        pass_int_argument(&rop, 0, 2);
        call_runtime_function(is_unsigned(type) ? rt_l_mulu : rt_l_muls);
        *result = get_call_result_int(0);
        return;
    }
    emit_binary_gpr_instruction(
        TYPE_IS_8BYTES(type) ? dmul3_opcode : mul3_opcode, left, right, result
    );
}

Operand unsigned_division_by_constant(Operand* x, UInt32 divisor) {
    DivisionParameters magic = compute_unsigned_division_parameters(divisor);
    Operand quotient;
    Operand difference;

    emitpcode(PC_MULTU, load_constant(magic.multiplier), x->reg);
    emitpcode(PC_MFHI, used_virtual_registers[RegClass_GPR]++);
    quotient = operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
    if (magic.addIndicator == 0) {
        if (magic.shift) {
            quotient = emit_shift_immediate(PC_SRL, &quotient, magic.shift);
        }
    } else {
        difference = emit_binary(PC_SUBU, x, &quotient, 0);
        difference = emit_shift_immediate(PC_SRL, &difference, 1);
        quotient = emit_binary(PC_ADDU, &difference, &quotient, 0);
        if (magic.shift - 1) {
            quotient = emit_shift_immediate(PC_SRL, &quotient, magic.shift - 1);
        } else {
            quotient = difference;
        }
    }
    return quotient;
}

Operand signed_division_by_constant(Operand* x, SInt32 divisor) {
    ConstInfo2 magic;
    Operand quotient;
    Operand sign;

    magic = compute_signed_division_multiplier_shift(divisor);
    emitpcode(PC_MULT, load_constant(magic.multiplier), x->reg);
    if (divisor > 0) {
        sign = emit_shift_immediate(PC_SRL, x, 31);
    }
    emitpcode(PC_MFHI, used_virtual_registers[RegClass_GPR]++);
    quotient = operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
    if (divisor > 0 && magic.multiplier < 0) {
        quotient = emit_binary(PC_ADDU, &quotient, x, 0);
    } else if (divisor < 0 && magic.multiplier > 0) {
        quotient = emit_binary(PC_SUBU, &quotient, x, 0);
    }
    if (magic.shift) {
        quotient = emit_shift_immediate(PC_SRA, &quotient, magic.shift);
    }
    if (divisor < 0) {
        sign = emit_shift_immediate(PC_SRL, &quotient, 31);
    }
    quotient = emit_binary(PC_ADDU, &quotient, &sign, 0);
    return quotient;
}

void fn_004A59A0(Type* type, Operand* op) {
    CLabel* end;
    CLabel* alternative;
    Operand destination;
    Operand branch_arg;
    Operand scratch;
    int previous;
    int gpr;
    short opcode2;

    previous = op->reg;
    {
        CLabel* conditional;
        int fpr;
        short opcode;
        fpr = used_virtual_registers[RegClass_FPR];
        used_virtual_registers[RegClass_FPR]++;
        conditional = newlabel();
        end = newlabel();
        alternative = newlabel();
        if (IS_TYPE_FLOAT32(type)) {
            opcode = 0x72;
            opcode2 = 0xED;
        } else {
            opcode = 0x6C;
            opcode2 = 0xEC;
        }
        destination = make_register_operand(used_virtual_registers[RegClass_FPR]++, 1);
        end->pclabel = makepclabel();
        if (copts.gencvtcheck) {
            alternative->pclabel = makepclabel();
            emitpcode(0x8B, used_virtual_registers[RegClass_GPR]++, previous, 23);
            branch_arg = operand_from_pcode_arg(
                pclastblock->lastPCode->args[(pclastblock->lastPCode->flags & 0x80000000) ? 1 : 0]
            );
            emit_branch(PC_BEQZ, &branch_arg, alternative);
            if (IS_TYPE_FLOAT32(type)) {
                fn_004A5EE0(op, &destination);
            } else {
                fn_004A5CA0(op, &destination);
            }
            branch_always(end->pclabel);
            emit_label(alternative->pclabel);
        }
        conditional->pclabel = makepclabel();
        emit_branch(0x24, op, conditional);
        scratch = emit_mtc1_paired(op);
        emitpcode(opcode, destination.reg, scratch.reg);
        branch_always(end->pclabel);
        emit_label(conditional->pclabel);
        emitpcode(0x8A, gpr = used_virtual_registers[RegClass_GPR]++, op->reg, 1);
        scratch = fn_0048BEB0(op, 1);
        emitpcode(PC_OR, gpr, gpr, scratch.reg);
        scratch.reg = gpr;
        scratch = emit_mtc1_paired(&scratch);
        emitpcode(opcode, fpr, scratch.reg);
        emitpcode(opcode2, destination.reg, fpr, fpr);
        *op = destination;
        emit_label(end->pclabel);
    }
}

void fn_004A55B0(Operand* op) {
    int oldreg;
    CLabel* done;
    CLabel* skip;
    Operand moved;
    Operand comparison;
    Operand narrowed;
    Operand condition;
    Operand destination;
    CInt64 constant;

    oldreg = op->reg;
    newlabel();
    done = newlabel();
    skip = newlabel();
    destination = make_register_operand(used_virtual_registers[RegClass_FPR]++, 1);
    done->pclabel = makepclabel();
    if (copts.gencvtcheck) {
        constant = cint64_5e8b70;
        destination = make_register_operand(used_virtual_registers[RegClass_FPR]++, 1);
        skip->pclabel = makepclabel();
        emitpcode(0x88, used_virtual_registers[RegClass_GPR]++, oldreg, 23);
        narrowed = operand_from_pcode_arg(
            pclastblock->lastPCode->args[(pclastblock->lastPCode->flags & 0x80000000) ? 1 : 0]
        );
        comparison = fn_0048C5B0(&narrowed, &cint64_one, (Type*)&stsignedlonglong);
        condition = fn_00489E00(&comparison, &constant, (Type*)&stsignedlonglong);
        emit_branch(PC_BNEZ, &condition, skip);
        fn_004A5EE0(op, &destination);
        branch_always(done->pclabel);
        emit_label(skip->pclabel);
    }
    moved = emit_mtc1_paired(op);
    emitpcode(0x72, destination.reg, moved.reg);
    *op = destination;
    emit_label(done->pclabel);
}

void fn_004A57B0(Operand* op) {
    int oldreg;
    CLabel* done;
    CLabel* skip;
    Operand moved;
    Operand comparison;
    Operand narrowed;
    Operand condition;
    Operand destination;
    CInt64 constant;

    oldreg = op->reg;
    done = newlabel();
    skip = newlabel();
    destination = make_register_operand(used_virtual_registers[RegClass_FPR]++, 1);
    done->pclabel = makepclabel();
    if (copts.gencvtcheck) {
        constant = cint64_5e8b60;
        destination = make_register_operand(used_virtual_registers[RegClass_FPR]++, 1);
        skip->pclabel = makepclabel();
        emitpcode(0x88, used_virtual_registers[RegClass_GPR]++, oldreg, 23);
        narrowed = make_register_operand(
            pclastblock->lastPCode->args[(pclastblock->lastPCode->flags & 0x80000000) ? 1 : 0]
                .data.reg.reg,
            0
        );
        comparison = fn_0048C5B0(&narrowed, &cint64_one, (Type*)&stsignedlonglong);
        condition = fn_00489E00(&comparison, &constant, (Type*)&stsignedlonglong);
        emit_branch(PC_BNEZ, &condition, skip);
        fn_004A5CA0(op, &destination);
        branch_always(done->pclabel);
        emit_label(skip->pclabel);
    }
    moved = emit_mtc1_paired(op);
    emitpcode(0x6C, destination.reg, moved.reg);
    *op = destination;
    emit_label(done->pclabel);
}

void fn_004A5CA0(Operand* left, Operand* right) {
    Operand a;
    Operand b;
    Operand c;
    Operand d;
    Operand e;
    Operand f;
    Operand g;
    Operand h;
    Operand i;
    CInt64 constant;
    int original_reg;

    original_reg = left->reg;
    constant = cint64_5e8b50;
    a = emit_extend(0x48C, left, 55);
    b = emit_mtc1_paired(&a);
    emitpcode(0x6C, used_virtual_registers[RegClass_FPR]++, b.reg);
    c = operand_from_pcode_arg(
        pclastblock->lastPCode->args[(pclastblock->lastPCode->flags & 0x80000000) ? 1 : 0]
    );
    emitpcode(0x8B, used_virtual_registers[RegClass_GPR]++, original_reg, 23);
    d = operand_from_pcode_arg(
        pclastblock->lastPCode->args[(pclastblock->lastPCode->flags & 0x80000000) ? 1 : 0]
    );
    e = emit_mtc1_paired(&d);
    emitpcode(0x6C, used_virtual_registers[RegClass_FPR]++, e.reg);
    f = operand_from_pcode_arg(
        pclastblock->lastPCode->args[(pclastblock->lastPCode->flags & 0x80000000) ? 1 : 0]
    );
    g = emit_mfc1_paired(&f);
    h = fn_0048C5B0(&g, &constant, (Type*)&stsignedlonglong);
    i = emit_mtc1_paired(&h);
    emitpcode(0xEC, right->reg, i.reg, c.reg);
}

void fn_004A5EE0(Operand* left, Operand* right) {
    Operand a;
    Operand b;
    Operand c;
    Operand d;
    Operand e;
    Operand f;
    Operand g;
    Operand h;
    Operand i;
    int original_reg;

    original_reg = left->reg;
    a = emit_extend(0x48C, left, 55);
    b = emit_mtc1_paired(&a);
    emitpcode(0x72, used_virtual_registers[RegClass_FPR]++, b.reg);
    c = operand_from_pcode_arg(
        pclastblock->lastPCode->args[(pclastblock->lastPCode->flags & 0x80000000) ? 1 : 0]
    );
    emitpcode(0x8B, used_virtual_registers[RegClass_GPR]++, original_reg, 23);
    d = operand_from_pcode_arg(
        pclastblock->lastPCode->args[(pclastblock->lastPCode->flags & 0x80000000) ? 1 : 0]
    );
    e = emit_mtc1_paired(&d);
    emitpcode(0x72, used_virtual_registers[RegClass_FPR]++, e.reg);
    f = operand_from_pcode_arg(
        pclastblock->lastPCode->args[(pclastblock->lastPCode->flags & 0x80000000) ? 1 : 0]
    );
    g = emit_mfc1(&f);
    h = fn_0048C8C0(&g, 0x1B800000, (Type*)&stsignedlong);
    i = emit_mtc1(&h);
    emitpcode(0xED, right->reg, i.reg, c.reg);
}

void generate_division(ENode* node, short reg, Operand* result) {
    ENode* left;
    ENode* right;
    Type* type;
    Operand lop;
    Operand rop;
    int shift;
    int div3op;
    int divop;
    PCodeLabel* notzero;
    PCodeLabel* zero;

    left = node->data.diadic.left;
    type = node->rtype;
    right = node->data.diadic.right;

    if (IS_TYPE_FLOAT(type)) {
        if (IS_TYPE_FLOAT_SOFTWARE(type)) {
            generate_operands(left, right, &lop, &rop, 1);
            pass_argument(&lop, type, 1);
            pass_argument(&rop, type, 2);
            call_runtime_function(IS_TYPE_FLOAT32(type) ? rt_f_div : rt_dpdiv);
            *result = get_call_result(type);
        } else if (IS_TYPE_FLOAT64(type) && !(copts.fp_library & 2)) {
            generate_operands(left, right, &lop, &rop, 1);
            pass_argument(&lop, type, 1);
            pass_argument(&rop, type, 2);
            call_runtime_function(rt_dpdiv);
            *result = get_call_result(type);
        } else {
            emit_binary_fpr_instruction(
                IS_TYPE_FLOAT32(type) ? PC_DIV_S : PC_DIV_D, left, right, result
            );
        }
        return;
    }

    generate_operands(left, right, &lop, &rop, 1);
    if (is_unsigned(type)) {
        if (right->type == EINTCONST) {
            if (CInt64_Equal(right->data.intval, cint64_zero)) {
                CError_Warning(40);
                *result = lop;
                return;
            }
            if (CInt64_Equal(right->data.intval, cint64_one)) {
                *result = lop;
                return;
            }
            shift = I8_getbit_if_32bit(right->data.intval);
            if (shift > 0) {
                shift_right_imm(&lop, type, shift, result);
                return;
            }
            if (!copts.optimizesize && !TYPE_IS_8BYTES(type)) {
                *result = unsigned_division_by_constant(&lop, right->data.intval.lo);
                return;
            }
            if (rop.kind == 3 && rop.kind) {
                Coerce_to_register(&rop, right->rtype, 0);
            }
        }
        div3op = TYPE_IS_8BYTES(type) ? PC_DDIVU3 : PC_DIVU3;
        divop = TYPE_IS_8BYTES(type) ? PC_DDIVU : PC_DIVU;
    } else {
        if (right->type == EINTCONST) {
            if (CInt64_Equal(right->data.intval, cint64_zero)) {
                CError_Warning(40);
                *result = lop;
                return;
            }
            if (CInt64_Equal(right->data.intval, cint64_one)) {
                *result = lop;
                return;
            }
            if (CInt64_Equal(right->data.intval, cint64_negone)) {
                *result = emit_unary(TYPE_IS_8BYTES(type) ? PC_DNEGU : PC_NEGU, &lop, 0);
                return;
            }
            shift = I8_getbit_if_32bit(right->data.intval);
            if (shift > 0 && shift < 16 && signed_div_pow2(&lop, shift, 0, result, type)) {
                return;
            }
            shift = I8_getbit_if_32bit(CInt64_Neg(right->data.intval));
            if (shift > 0 && shift < 16 && signed_div_pow2(&lop, shift, 1, result, type)) {
                return;
            }
            if (!copts.optimizesize && !TYPE_IS_8BYTES(type)) {
                *result = signed_division_by_constant(&lop, right->data.intval.lo);
                return;
            }
            if (rop.kind == 3 && rop.kind) {
                Coerce_to_register(&rop, right->rtype, 0);
            }
        }
        div3op = TYPE_IS_8BYTES(type) ? ddiv3_opcode : div3_opcode;
        divop = TYPE_IS_8BYTES(type) ? PC_DDIV : PC_DIV;
    }

    if (TYPE_IS_8BYTES(type) && copts.unknown25) {
        pass_int_argument(&lop, 1, 1);
        pass_int_argument(&rop, 1, 2);
        call_runtime_function(is_unsigned(type) ? rt_udivdi3 : rt_divdi3);
        *result = get_call_result_int(1);
        return;
    }
    if (copts.unknown26) {
        pass_int_argument(&lop, 0, 1);
        pass_int_argument(&rop, 0, 2);
        call_runtime_function(is_unsigned(type) ? rt_l_divu : rt_l_divs);
        *result = get_call_result_int(0);
        return;
    }

    if (copts.divbyzerocheck) {
        notzero = makepclabel();
        zero = makepclabel();
        emitpcode(divop, lop.reg, rop.reg);
        emitpcode(PC_BNEZ, rop.reg, notzero);
        pcaddedge(pclastblock, notzero, 0);
        pcaddedge(pclastblock, zero, 0);
        makepcblock();
        pclabelblock(pclastblock, zero);
        emitpcode(PC_BREAKC, 7);
        pcaddedge(pclastblock, notzero, 0);
        makepcblock();
        pclabelblock(pclastblock, notzero);
        emitpcode(PC_MFLO, used_virtual_registers[RegClass_GPR]++);
    } else {
        emitpcode(div3op, used_virtual_registers[RegClass_GPR]++, lop.reg, rop.reg);
    }
    *result = operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
}

void emit_three_fpr_operands(
    short opcode, ENode* left, ENode* right, ENode* third, short reg, Operand* result
) {
    Operand lop;
    Operand rop;
    Operand top;
    int dest;
    int intermediate;

    memclrw(&lop, sizeof(lop));
    memclrw(&rop, sizeof(rop));
    memclrw(&top, sizeof(top));
    if (right->cost > left->cost + 2) {
        gen_func_table[right->type](right, 0, &rop);
        if (rop.kind) {
            Coerce_to_register(&rop, right->rtype, 0);
        }
        gen_func_table[left->type](left, 0, &lop);
        if (lop.kind) {
            Coerce_to_register(&lop, left->rtype, 0);
        }
    } else {
        gen_func_table[left->type](left, 0, &lop);
        if (lop.kind) {
            Coerce_to_register(&lop, left->rtype, 0);
        }
        gen_func_table[right->type](right, 0, &rop);
        if (rop.kind) {
            Coerce_to_register(&rop, right->rtype, 0);
        }
    }
    gen_func_table[third->type](third, 0, &top);
    if (top.kind) {
        Coerce_to_register(&top, third->rtype, 0);
    }
    if (reg) {
        dest = reg;
    } else {
        dest = used_virtual_registers[RegClass_FPR]++;
    }
    intermediate = used_virtual_registers[RegClass_FPR]++;
    emitpcode(PC_MTC1, 0, intermediate);
    emitpcode(0x1DD, intermediate, top.reg);
    emitpcode(opcode, dest, lop.reg, rop.reg);
    memclrw(result, sizeof(*result));
    result->kind = 1;
    result->reg = dest;
}

void generate_modulo(ENode* node, short reg, Operand* result) {
    ENode* left;
    ENode* right;
    Type* type;
    Operand lop;
    Operand rop;
    Operand remainder;
    Operand adjusted;
    CInt64 mask;
    int shift;
    short divop;
    CLabel* label;
    PCodeLabel* notzero;
    PCodeLabel* zero;

    left = node->data.diadic.left;
    type = node->rtype;
    right = node->data.diadic.right;

    if (TYPE_IS_8BYTES(type) && copts.unknown25) {
        generate_operands(left, right, &lop, &rop, 1);
        pass_int_argument(&lop, 1, 1);
        pass_int_argument(&rop, 1, 2);
        call_runtime_function(is_unsigned(type) ? rt_umoddi3 : rt_moddi3);
        *result = get_call_result_int(1);
        return;
    }

    generate_operands(left, right, &lop, &rop, 1);
    if (TYPE_IS_8BYTES(type)) {
        divop = is_unsigned(type) ? PC_DDIVU : PC_DDIV;
    } else {
        divop = is_unsigned(type) ? PC_DIVU : PC_DIV;
    }

    if (right->type == EINTCONST) {
        shift = I8_getbit_if_32bit(right->data.intval);
        if (shift > 0) {
            mask = CInt64_Sub(right->data.intval, cint64_one);
            if (mask.lo == 0xFFFFFFFF) {
                remainder = lop;
            } else {
                remainder = emit_immediate_cint(PC_ANDI3, &lop, mask);
            }
            if (is_unsigned(left->rtype)) {
                *result = remainder;
            } else {
                label = newlabel();
                label->pclabel = makepclabel();
                emit_branch(PC_BGEZ, &lop, label);
                emit_branch(PC_BEQZ, &remainder, label);
                adjusted = emit_immediate_cint(
                    TYPE_IS_8BYTES(type) ? PC_DADDIU : PC_ADDIU,
                    &remainder,
                    CInt64_Sub(CInt64_Neg(mask), cint64_one)
                );
                emitpcode(move_opcode, remainder.reg, adjusted.reg);
                emit_label(label->pclabel);
                *result = copy_to_new_register(&remainder);
            }
        } else {
            emitpcode(divop, lop.reg, rop.reg);
            emitpcode(PC_MFHI, used_virtual_registers[RegClass_GPR]++);
            *result = operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
        }
        return;
    }

    emitpcode(divop, lop.reg, rop.reg);
    if (copts.divbyzerocheck) {
        notzero = makepclabel();
        zero = makepclabel();
        emitpcode(PC_BNEZ, rop.reg, notzero);
        pcaddedge(pclastblock, notzero, 0);
        pcaddedge(pclastblock, zero, 0);
        makepcblock();
        pclabelblock(pclastblock, zero);
        emitpcode(PC_BREAKC, 7);
        pcaddedge(pclastblock, notzero, 0);
        makepcblock();
        pclabelblock(pclastblock, notzero);
    }
    emitpcode(PC_MFHI, used_virtual_registers[RegClass_GPR]++);
    *result = operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
}

//! @reconstructed
void select_subtraction(ENode* node, short reg, Operand* result) {
    Type* type;
    ENode* left;
    ENode* right;
    Operand lop;
    Operand rop;
    short opcode;
    int dest;

    type = node->rtype;
    left = node->data.diadic.left;
    right = node->data.diadic.right;

    if (IS_TYPE_FLOAT(type) &&
        right->type == EMUL &&
        IS_TYPE_FLOAT32(left->rtype) &&
        IS_TYPE_FLOAT32(right->rtype) &&
        IS_TYPE_FLOAT32(right->data.diadic.left->rtype) &&
        IS_TYPE_FLOAT32(right->data.diadic.right->rtype) &&
        copts.usefloatacc) {
        CError_ASSERT(0x96C, IS_TYPE_FLOAT(right->rtype));
        emit_three_fpr_operands(
            0x1E2, right->data.diadic.left, right->data.diadic.right, left, reg, result
        );
        return;
    }

    generate_operands(left, right, &lop, &rop, 1);
    if (IS_TYPE_FLOAT_HARDWARE(type)) {
        opcode = IS_TYPE_FLOAT32(type) ? 0x103 : 0x102;
        dest = used_virtual_registers[RegClass_FPR]++;
        emitpcode(opcode, dest, lop.reg, rop.reg);
        *result = operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
        return;
    }
    if (IS_TYPE_FLOAT(type)) {
        pass_argument(&lop, type, 1);
        pass_argument(&rop, type, 2);
        call_runtime_function(IS_TYPE_FLOAT32(type) ? rt_f_sub : rt_dpsub);
        *result = get_call_result(type);
        return;
    }
    CError_ASSERT(0x991, lop.kind != 3 || rop.kind != 3);
    if (lop.kind == 3) {
        CError_FATAL(0x994);
    }
    if (rop.kind == 3) {
        rop = fn_0048E350(rop.immediate, type);
    }
    *result = emit_binary(TYPE_IS_8BYTES(type) ? 0x8E : PC_SUBU, &lop, &rop, 0);
}

void generate_left_shift(ENode* node, short reg, Operand* result) {
    ENode* left;
    ENode* right;
    Type* type;
    Operand op;

    left = node->data.diadic.left;
    right = node->data.diadic.right;
    type = node->rtype;
    if (right->type == EINTCONST) {
        gen_func_table[left->type](left, 0, &op);
        if ((op.kind >= 4 || op.kind == 2) && op.kind) {
            Coerce_to_register(&op, left->rtype, 0);
        }
        shift_left_imm(&op, type, right->data.intval.lo, 0, result);
    } else {
        emit_binary_gpr_instruction(TYPE_IS_8BYTES(type) ? PC_DSLLV : PC_SLLV, left, right, result);
    }
}

void select_right_shift(ENode* node, short reg, Operand* result) {
    ENode* left;
    ENode* right;
    Type* type;
    Operand op;

    left = node->data.diadic.left;
    right = node->data.diadic.right;
    type = left->rtype;
    if (right->type == EINTCONST) {
        gen_func_table[left->type](left, 0, &op);
        if ((op.kind >= 4 || op.kind == 2) && op.kind) {
            Coerce_to_register(&op, left->rtype, 0);
        }
        shift_right_imm(&op, type, right->data.intval.lo, result);
    } else if (TYPE_IS_8BYTES(type)) {
        emit_binary_gpr_instruction(is_unsigned(type) ? PC_DSRLV : PC_DSRAV, left, right, result);
    } else {
        emit_binary_gpr_instruction(is_unsigned(type) ? PC_SRLV : PC_SRAV, left, right, result);
    }
}

Operand emit_logical_operation(
    Type* type, ENode* left, ENode* right, int opcode, int immediate_opcode
) {
    Operand lop;
    Operand rop;
    generate_operands(left, right, &lop, &rop, 0);
    if (lop.kind == 3 && rop.kind == 3) {
        switch (immediate_opcode) {
            case PC_ANDI3:
                lop.immediate = CInt64_And(lop.immediate, rop.immediate);
                break;
            case PC_ORI3:
                lop.immediate = CInt64_Or(lop.immediate, rop.immediate);
                break;
            case PC_XORI3:
                lop.immediate = CInt64_Xor(lop.immediate, rop.immediate);
                break;
        }
        return lop;
    }
    if (lop.kind == 3) {
        Operand temp = lop;
        lop = rop;
        rop = temp;
    }
    if (lop.kind) {
        Coerce_to_register(&lop, left->rtype, 0);
    }
    if (rop.kind == 3 && CInt64_IsInRange(rop.immediate, 4)) {
        return emit_immediate_cint(immediate_opcode, &lop, rop.immediate);
    }
    if (rop.kind) {
        Coerce_to_register(&rop, right->rtype, 0);
    }
    return emit_binary(opcode, &lop, &rop, 0);
}

void emit_bitwise_and(ENode* node, short reg, Operand* result) {
    *result = emit_logical_operation(
        node->rtype, node->data.diadic.left, node->data.diadic.right, PC_AND, PC_ANDI3
    );
}

//! @reconstructed
void emit_bitwise_not(ENode* node, short reg, Operand* result) {
    Operand lop;
    Operand rop;
    Operand zero;
    Type* type;
    ENode* expr;

    type = node->rtype;
    expr = node->data.monadic;
    zero = make_register_operand(0, 0);
    if (expr->type != EOR) {
        gen_func_table[expr->type](expr, 0, &lop);
        if (lop.kind >= 4 || lop.kind == 2) {
            if (lop.kind) {
                Coerce_to_register(&lop, expr->rtype, 0);
            }
        }
        *result = emit_binary(0xBC, &lop, &zero, 0);
    } else {
        generate_operands(expr->data.diadic.left, expr->data.diadic.right, &lop, &rop, 1);
        *result = emit_binary(0xBC, &lop, &rop, 0);
        if (TYPE_IS_8BYTES(type)) {
            lop = get_high_half_operand(&lop);
            rop = get_high_half_operand(&rop);
            lop = emit_binary(0xBC, &lop, &rop, 0);
            result->regHi = lop.reg;
            result->kind = 0;
        }
    }
}

void gen_xor(ENode* node, short reg, Operand* result) {
    *result = emit_logical_operation(
        node->rtype, node->data.diadic.left, node->data.diadic.right, PC_XOR, PC_XORI3
    );
}

void select_or(ENode* node, short reg, Operand* result) {
    *result = emit_logical_operation(
        node->rtype, node->data.diadic.left, node->data.diadic.right, PC_OR, PC_ORI3
    );
}

void generate_block_assignment(ENode* node, short reg, Operand* result) {
    ENode* left;
    ENode* right;
    Type* type;
    Operand src;
    int align;

    right = node->data.diadic.right;
    left = node->data.diadic.left;
    type = node->rtype;
    gen_func_table[right->type](right, 0, &src);
    gen_func_table[left->type](left, 0, result);
    align = CMach_GetTypeAlign(type);
    if (left->type == EOBJREF && right->type == EOBJREF) {
        if (CMach_GetObjectAlign(left->data.objref) < CMach_GetObjectAlign(right->data.objref)) {
            align = CMach_GetObjectAlign(left->data.objref);
        } else {
            align = CMach_GetObjectAlign(right->data.objref);
        }
    }
    fn_0053CC40(result, &src, type->size, align);
}

void select_diadic_left_then_right(ENode* node, short reg, Operand* result) {
    ENode* left;
    ENode* right;

    left = node->data.diadic.left;
    right = node->data.diadic.right;
    gen_func_table[left->type](left, 0, result);
    gen_func_table[right->type](right, reg, result);
}

void generate_bitfield_operand(ENode* node, short reg, Operand* result) {
    ENode* operand;

    operand = node->data.monadic;
    gen_func_table[operand->type](operand, 0, result);
    if (result->kind == 4 && result->kind) {
        Coerce_to_register(result, TYPE(&void_ptr), 0);
    }
    result->unk20 = (SInt32)node->rtype;
}

void make_intval_operand(ENode* node, short reg, Operand* result) {
    if (CInt64_IsZero(&node->data.intval)) {
        *result = make_register_operand(0, 0);
        if (TYPE_IS_8BYTES(node->rtype)) {
            result->regHi = 0;
            result->kind = 0;
        }
    } else {
        result->kind = 3;
        result->immediate = node->data.intval;
        result->unk20 = 0;
    }
}

void select_string_constant(ENode* node, short reg, Operand* result) {
    CError_FATAL(3610);
}

//! @reconstructed
void load_float_constant(ENode* node, short reg, Operand* result) {
    union {
        float floating;
        UInt32 bits;
    } single;
    union {
        double floating;
        struct {
            UInt32 low;
            UInt32 high;
        } bits;
    } wide;
    CInt64 value;
    Operand immediate;
    Operand converted;

    if (IS_TYPE_FLOAT32(node->rtype)) {
        single.floating = node->data.floatval.value;
        *result = make_constant_operand(single.bits);
        if (IS_TYPE_FLOAT_HARDWARE(node->rtype)) {
            converted = emit_mtc1(result);
            *result = converted;
        }
    } else {
        wide.floating = node->data.floatval.value;
        value.lo = wide.bits.low;
        value.unk8 = wide.bits.high;
        value.unk4 = 0;
        value.hi = 0;
        immediate = fn_0048E350(value, (Type*)&stunsignedlonglong);
        *result = immediate;
        if (IS_TYPE_FLOAT_HARDWARE(node->rtype)) {
            *result = emit_mtc1_paired(result);
        }
    }
}

//! @reconstructed
void generate_condition_branches(
    ENode* expr, CLabel* trueLabel, CLabel* falseLabel, Boolean trueFollows, Boolean falseFollows
) {
    CLabel* label;

    label = newlabel();
    switch (expr->type) {
        case ELAND:
            label->pclabel = makepclabel();
            generate_condition_branches(expr->data.diadic.left, label, falseLabel, 1, 0);
            emit_label(label->pclabel);
            generate_condition_branches(expr->data.diadic.right, trueLabel, falseLabel, 0, 0);
            break;
        case ELOR:
            label->pclabel = makepclabel();
            generate_condition_branches(expr->data.diadic.left, trueLabel, label, 0, 1);
            emit_label(label->pclabel);
            generate_condition_branches(expr->data.diadic.right, trueLabel, falseLabel, 0, 0);
            break;
        case ELOGNOT:
            generate_condition_branches(
                expr->data.diadic.left, falseLabel, trueLabel, falseFollows, trueFollows
            );
            break;
        case ELESS:
        case EGREATER:
        case ELESSEQU:
        case EGREATEREQU:
        case EEQU:
        case ENOTEQU:
            if (trueFollows) {
                generate_comparison_branch(expr, falseLabel, 0);
            } else if (falseFollows) {
                generate_comparison_branch(expr, trueLabel, 1);
            } else {
                generate_comparison_branch(expr, trueLabel, 1);
                branch_always(falseLabel->pclabel);
            }
            break;
        default:
            CError_FATAL(3667);
    }
}

//! @reconstructed
void generate_conditional_expression(ENode* node, short reg, Operand* result) {
    ENode* expr1;
    ENode* expr2;
    Type* type;
    Type* exprType;
    CLabel* trueLabel;
    CLabel* falseLabel;
    CLabel* endLabel;
    Operand compare;
    Operand first;
    Operand second;
    UInt16 opcode;

    expr2 = node->data.cond.expr2;
    expr1 = node->data.cond.expr1;
    type = node->rtype;

    if (copts.peephole &&
        copts.conditional_move &&
        expr1->type == EINTCONST &&
        expr2->type == EINTCONST &&
        (node->data.cond.cond->type == EEQU || node->data.cond.cond->type == ENOTEQU) &&
        node->data.cond.cond->data.diadic.right->type == EINTCONST &&
        CInt64_Equal(node->data.cond.cond->data.diadic.right->data.intval, cint64_zero)) {
        opcode = node->data.cond.cond->type == ENOTEQU ? 0x111 : 0x110;
        gen_func_table[node->data.cond.cond->data.diadic.left->type](
            node->data.cond.cond->data.diadic.left, 0, &compare
        );
        if (compare.kind) {
            Coerce_to_register(&compare, node->data.cond.cond->data.diadic.left->rtype, 1);
        }
        gen_func_table[expr1->type](expr1, 0, &first);
        if (first.kind) {
            Coerce_to_register(&first, expr1->rtype, 1);
        }
        if (!first.reg) {
            first = convert_operand_type(&first, expr1->rtype);
        }
        gen_func_table[expr2->type](expr2, 0, &second);
        if (second.kind) {
            Coerce_to_register(&second, expr2->rtype, 1);
        }
        emitpcode(opcode, first.reg, second.reg, compare.reg);
        *result = operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
        return;
    }

    trueLabel = newlabel();
    falseLabel = newlabel();
    endLabel = newlabel();
    trueLabel->pclabel = makepclabel();
    falseLabel->pclabel = makepclabel();
    endLabel->pclabel = makepclabel();

    generate_condition_branches(node->data.cond.cond, trueLabel, falseLabel, 1, 0);
    emit_label(trueLabel->pclabel);
    gen_func_table[expr1->type](expr1, 0, &first);
    if (first.kind < 2) {
        first = convert_operand_type(&first, type);
    } else {
        exprType = expr1->rtype;
        if (exprType &&
            exprType->size > register_size &&
            (exprType->type == TYPEARRAY ||
             exprType->type == TYPESTRUCT ||
             exprType->type == TYPECLASS ||
             IS_TYPE_12BYTES_MEMBERPOINTER(exprType)) &&
            first.kind == 4) {
            indirect(&first, expr1);
            first.kind = 2;
        }
        if (first.kind) {
            Coerce_to_register(&first, expr1->rtype, 1);
        }
    }
    branch_always(endLabel->pclabel);
    emit_label(falseLabel->pclabel);
    gen_func_table[expr2->type](expr2, 0, &second);
    exprType = expr2->rtype;
    if (exprType &&
        exprType->size > register_size &&
        (exprType->type == TYPEARRAY ||
         exprType->type == TYPESTRUCT ||
         exprType->type == TYPECLASS ||
         IS_TYPE_12BYTES_MEMBERPOINTER(exprType)) &&
        second.kind == 4) {
        indirect(&second, expr2);
        second.kind = 2;
    }
    if (second.kind) {
        Coerce_to_register(&second, expr2->rtype, 1);
    }
    exprType = expr2->rtype;
    if (exprType->type == TYPEFLOAT &&
        (((exprType->size == 4) && (copts.fp_library & 1)) ||
         ((exprType->size == 8) && (copts.fp_library & 2)))) {
        emitpcode(exprType->size == 4 ? 0x4A3 : 0x4A4, first.reg, second.reg);
    } else {
        emitpcode(move_opcode, first.reg, second.reg);
    }
    emit_label(endLabel->pclabel);
    if (type->type == TYPEVOID || node->ignored) {
        result->kind = 3;
        result->immediate = cint64_zero;
    } else {
        *result = first;
    }
}

void generate_intrinsic_or_function_call(ENode* node, short reg, Operand* result) {
    if (is_intrinsic_function_call(node)) {
        Intrinsics_GenerateIntrinsicCall(node, reg, result);
    } else {
        binary_word_count_destination->gprmask |= 0x80000000;
        FunctionCalls_GenerateCall(node, result);
    }
}

Operand make_object_operand(Object* object) {
    Operand op;
    ObjectList* list;

    memclrw(&op, sizeof(Operand));
    op.kind = 2;
    op.reg = object_base_register(object);
    op.object = object;
    //! @matching_workaround the original zero-extends the register number and compares it as an int; written
    //! op.reg == 28, this compiler compares the 16-bit value. Tried: !op.reg, a switch, an inline test taking an
    //! int, the object in a local.
    if ((int)op.reg == 28 || (int)op.reg == 23 || !op.reg || fn_004727A0(op.object)) {
        op.immOffset = 0;
        if (object->datatype != DLOCAL && strchr(CMangler_GetLinkName(object)->name, '$')) {
            list = lalloc(sizeof(ObjectList));
            list->object = object;
            list->next = dollar_name_objects;
            dollar_name_objects = list;
        } else {
            ElfTrackDebugSource(object);
        }
    } else {
        op.immOffset = object->u.unk30;
    }
    op.unk20 = 0;
    return op;
}

void make_objref_operand(ENode* node, short reg, Operand* result) {
    *result = make_object_operand(node->data.objref);
}

void select_unexpected_node(ENode* node, short reg, Operand* result) {
    CError_FATAL(3893);
}

void shift_left_imm(Operand* x, Type* type, short count, short negate, Operand* result) {
    count &= 0x3F;
    *result = emit_shift_left(x, count, type);
    if (negate) {
        *result = emit_unary(TYPE_IS_8BYTES(type) ? PC_DNEGU : PC_NEGU, result, 0);
    }
}

void shift_right_imm(Operand* x, Type* type, short count, Operand* result) {
    count &= 0x3F;
    *result = emit_shift_right(x, count, type);
}

Boolean signed_div_pow2(Operand* x, int n, int negate, Operand* result, Type* type) {
    Boolean wide;
    int reg;
    CLabel* label;
    Operand op;

    //! @matching_workaround the original turns the test's value into a Boolean with a compare (test, setne);
    //! assigned as it is, the compiler knows it is 0 or 1 and keeps it without one.
    wide = TYPE_IS_8BYTES(type) != 0;
    reg = used_virtual_registers[RegClass_GPR]++;
    label = newlabel();

    op = emit_shift_right(x, n, type);
    emitpcode(move_opcode, reg, op.reg);
    emit_branch(PC_BGEZ, x, label);
    op = emit_immediate(wide ? PC_DADDIU : PC_ADDIU, x, (1 << n) - 1);
    op = emit_shift_right(&op, n, type);
    emitpcode(move_opcode, reg, op.reg);
    emit_label(label->pclabel);

    op = make_register_operand(reg, 0);
    *result = copy_to_new_register(&op);
    if (negate) {
        *result = emit_unary(wide ? PC_DNEGU : PC_NEGU, result, 0);
    }
    return 1;
}

void emit_binary_fpr_instruction(short opcode, ENode* left, ENode* right, Operand* result) {
    Boolean one;
    Operand lop;
    Operand rop;
    int recip;

    one = 0;
    if (left->type == EFLOATCONST) {
        one = CMach_FloatIsOne(left->data.floatval.value);
    }
    generate_operands(left, right, &lop, &rop, 0);
    if (lop.kind) {
        Coerce_to_register(&lop, left->rtype, 0);
    }
    if (rop.kind) {
        Coerce_to_register(&rop, right->rtype, 0);
    }
    if (copts.unknown0C >= 4 &&
        copts.unknown11 &&
        (opcode == PC_DIV_D || opcode == PC_DIV_S) &&
        !copts.optimizesize) {
        emitpcode(
            opcode == PC_DIV_S ? PC_RECIP_S : PC_RECIP_D,
            recip = used_virtual_registers[RegClass_FPR]++,
            rop.reg
        );
        if (!one) {
            emitpcode(
                opcode == PC_DIV_S ? PC_MUL_S : PC_MUL_D,
                used_virtual_registers[RegClass_FPR]++,
                lop.reg,
                recip
            );
        }
    } else {
        emitpcode(opcode, used_virtual_registers[RegClass_FPR]++, lop.reg, rop.reg);
    }
    *result = operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
}

void generate_comparison_branch(ENode* expr, CLabel* label, int branchIfTrue) {
    ENode* left;
    ENode* right;
    Type* type;
    Operand lop;
    Operand rop;
    Operand value;
    int opcode;
    int isFloat;
    int invert = 1;
    UInt16 branch;
    UInt16 compare;
    Boolean unsignedOp;

    left = expr->data.diadic.left;
    right = expr->data.diadic.right;
    type = left->rtype;
    isFloat = type->type == TYPEFLOAT;
    if (label == returnlabel) {
        branches_to_return = 1;
    }
    unsignedOp = is_unsigned(left->rtype) || is_unsigned(right->rtype);
    opcode = expr->type;
    value.kind = 3;
    value.immediate = cint64_zero;
    if (!branchIfTrue && !isFloat) {
        switch (opcode) {
            case ELESS:
                opcode = EGREATEREQU;
                break;
            case EGREATER:
                opcode = ELESSEQU;
                break;
            case ELESSEQU:
                opcode = EGREATER;
                break;
            case EGREATEREQU:
                opcode = ELESS;
                break;
            case EEQU:
                opcode = ENOTEQU;
                break;
            case ENOTEQU:
                opcode = EEQU;
                break;
            default:
                CError_FATAL(0x204);
                break;
        }
        invert = 0;
    }
    if (!label->pclabel) {
        label->pclabel = makepclabel();
    }
    if (isFloat) {
        generate_operands(left, right, &lop, &rop, 1);
        if (IS_TYPE_FLOAT_SOFTWARE(type)) {
            pass_argument(&lop, type, 1);
            pass_argument(&rop, right->rtype, 2);
            call_runtime_function(get_float_compare_function(opcode, right->rtype));
            value = get_call_result_int(0);
            emit_branch(branchIfTrue ? 0x2A : 0x1A, &value, label);
        } else {
            get_float_compare_ops(opcode, right->rtype, &branch, &compare, invert);
            emitpcode(compare, lop.reg, rop.reg);
            emit_branch_to_label(branch, label);
        }
        return;
    }
    generate_operands(left, right, &lop, &rop, 1);
    if (lop.kind) {
        Coerce_to_register(&lop, left->rtype, 0);
    }
    if (rop.kind) {
        Coerce_to_register(&rop, right->rtype, 0);
    }
    switch (opcode) {
        case EEQU:
            branch = 0x28;
            break;
        case ENOTEQU:
            branch = 0x2A;
            break;
        case ELESS:
            branch = unsignedOp ? 0x32 : 0x30;
            break;
        case EGREATER:
            branch = unsignedOp ? 0x34 : 0x33;
            break;
        case ELESSEQU:
            branch = unsignedOp ? 0x35 : 0x36;
            break;
        case EGREATEREQU:
            branch = unsignedOp ? 0x37 : 0x38;
            break;
        default:
            CError_FATAL(0x219);
            branch = 0x28;
            break;
    }
    emit_branch_with_two_operands(branch, &lop, &rop, label);
}

void generate_comparison_value(ENode* node, Operand* result) {
    Type* type;
    ENode* left;
    ENode* right;
    Operand lop;
    Operand rop;
    Operand temp;
    UInt32 opcode;
    Boolean unsignedOperands;

    right = node->data.diadic.right;
    left = node->data.diadic.left;
    opcode = node->type;
    type = left->rtype;
    make_register_operand(0, 0);
    if (IS_TYPE_FLOAT(type)) {
        fn_0049D5F0(opcode, left, right, result);
        return;
    }

    generate_operands(left, right, &lop, &rop, 0);
    if (lop.kind == 3) {
        temp = lop;
        lop = rop;
        rop = temp;
        switch (opcode) {
            case ELESS:
                opcode = EGREATER;
                break;
            case EGREATER:
                opcode = ELESS;
                break;
            case ELESSEQU:
                opcode = EGREATEREQU;
                break;
            case EGREATEREQU:
                opcode = ELESSEQU;
                break;
        }
    }
    if (lop.kind) {
        Coerce_to_register(&lop, left->rtype, 0);
    }
    if (rop.kind != 3 && rop.kind) {
        Coerce_to_register(&rop, right->rtype, 0);
    }

    unsignedOperands = is_unsigned(right->rtype) | is_unsigned(left->rtype);
    switch (opcode) {
        case ELESS:
            if (rop.kind == 3) {
                if (unsignedOperands) {
                    *result = fn_00489E00(&lop, &rop.immediate, type);
                } else {
                    *result = fn_0048A4F0(&lop, &rop.immediate, type);
                }
            } else {
                *result = emit_binary((unsignedOperands) ? 0xD7 : 0xD4, &lop, &rop, 0);
            }
            break;
        case EGREATER:
            if (rop.kind == 3) {
                if (unsignedOperands) {
                    *result = fn_00489D00(&lop, &rop.immediate, type);
                } else {
                    *result = fn_00489D40(&lop, &rop.immediate, type);
                }
            } else {
                *result = emit_binary((unsignedOperands) ? 0x495 : 0x493, &lop, &rop, 0);
            }
            break;
        case ELESSEQU:
            if (rop.kind == 3) {
                if (unsignedOperands) {
                    *result = fn_0048ABE0(&lop, &rop.immediate, type);
                } else {
                    *result = fn_0048AD30(&lop, &rop.immediate, type);
                }
            } else {
                *result = emit_binary((unsignedOperands) ? 0x499 : 0x497, &lop, &rop, 0);
            }
            break;
        case EGREATEREQU:
            if (rop.kind == 3) {
                if (unsignedOperands) {
                    *result = fn_00489D80(&lop, &rop.immediate, type);
                } else {
                    *result = fn_00489DC0(&lop, &rop.immediate, type);
                }
            } else {
                *result = emit_binary((unsignedOperands) ? 0x491 : 0x48F, &lop, &rop, 0);
            }
            break;
        case EEQU:
            if (rop.kind == 3) {
                *result = fn_0048B640(&lop, &rop.immediate, type);
            } else {
                *result = emit_binary(0x48D, &lop, &rop, 0);
            }
            break;
        case ENOTEQU:
            if (rop.kind == 3) {
                *result = fn_0048AE90(&lop, &rop.immediate, type);
            } else {
                *result = emit_binary(0x49D, &lop, &rop, 0);
            }
            break;
        default:
            MIPSError_Error(16);
    }
}

void generate_comparison(ENode* node, short reg, Operand* result) {
    generate_comparison_value(node, result);
}

void generate_logical_not(ENode* node, short reg, Operand* result) {
    ENode* operand;
    Operand op;

    operand = node->data.monadic;
    gen_func_table[operand->type](operand, 0, &op);
    if (op.kind >= 4 && op.kind) {
        Coerce_to_register(&op, operand->rtype, 0);
    }
    *result = emit_immediate(PC_XORI, &op, 1);
}

void generate_boolean_expression(ENode* node, short reg, Operand* result) {
    ENode* left;
    ENode* right;
    CLabel* label;
    Operand lop;
    Operand rop;

    left = node->data.diadic.left;
    right = node->data.diadic.right;
    label = newlabel();
    gen_func_table[left->type](left, 0, &lop);
    if (lop.kind) {
        Coerce_to_register(&lop, left->rtype, 1);
    }
    if (node->type == ELAND) {
        emit_branch_if_equal(&lop, 0, label);
    } else if (node->type == ELOR) {
        emit_branch_if_not_equal(&lop, 0, label);
    } else {
        CError_FATAL(4204);
    }
    gen_func_table[right->type](right, 0, &rop);
    if (rop.kind) {
        Coerce_to_register(&rop, right->rtype, 0);
    }
    emitpcode(move_opcode, lop.reg, rop.reg);
    emit_label(label->pclabel);
    *result = make_register_operand(lop.reg, 0);
}

void emit_conditional_funccall(ENode* node, short reg, Operand* result) {
    ENode* left;
    ENode* right;
    Operand lop;
    Operand rop;
    int dest;
    FunctionCallFrame* frame;
    CLabel* label;

    left = node->data.cond.cond;
    right = node->data.cond.expr1;
    dest = used_virtual_registers[RegClass_GPR]++;
    gen_func_table[left->type](left, 0, &lop);
    if (left->rtype->size < 4) {
        force_operand_register(&lop, left->rtype, 0);
    }
    if (lop.kind) {
        Coerce_to_register(&lop, left->rtype, 0);
    }
    emitpcode(move_opcode, dest, lop.reg);
    frame = lalloc(sizeof(FunctionCallFrame));
    frame->functionType = (Type*)node->data.cond.expr2;
    frame->operand = lop;
    frame->next = function_call_frames;
    function_call_frames = frame;
    label = newlabel();
    emit_branch_if_equal(&lop, 0, label);
    gen_func_table[right->type](right, 0, &rop);
    function_call_frames = function_call_frames->next;
    if (node->rtype->type && !node->ignored) {
        result->kind = 0;
        if (rop.kind) {
            Coerce_to_register(&rop, right->rtype, 0);
        }
        emitpcode(move_opcode, dest, rop.reg);
    } else {
        result->kind = 3;
        result->reg = 0;
    }
    emit_label(label->pclabel);
    *result = make_register_operand(dest, 0);
}

void get_function_type_operand(ENode* node, short reg, Operand* result) {
    FunctionCallFrame* frame;

    for (frame = function_call_frames; frame; frame = frame->next) {
        if (frame->functionType == node->data.functype) {
            break;
        }
    }
    *result = frame->operand;
}
