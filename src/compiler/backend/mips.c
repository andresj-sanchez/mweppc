#include "compiler/RegisterInfo.h"
#include "compiler/Registers.h"

#include "compiler/BE_macro.h"
#include "compiler/mips.h"
#include "compiler/Alias.h"
#include "compiler/AliasPropagation.h"
#include "compiler/OpcodeInfo.h"
#include "compiler/CFunc.h"
#include "compiler/CParser.h"
#include "compiler/CodeGen.h"
#include "compiler/InstrSelection.h"
#include "compiler/PCode.h"
#include "compiler/PCodeInfo.h"
#include "compiler/PCodeUtilities.h"
#include "compiler/types.h"
#include "compiler/VReg.h"
#include "compiler/objects.h"
#include "msl/ctype_api.h"

int fn_0048ECB0(Type* type) {
    UInt8 kind;
    UInt8 group;

    kind = type->type;
    if (kind == TYPEENUM) {
        if (type->size == 1) {
            return 0x47F;
        }
        if (type->size == 2) {
            return 0x480;
        }
        if (type->size == 4) {
            return 0x481;
        }
        if (type->size == 8) {
            return 0x479;
        }
        CError_Internal(mips_source_filename, 92);
        return 0;
    }
    group = kind - TYPESTRUCT;
    if (group <= 1) {
        if (type->size == 1) {
            return 0x47F;
        }
        if (type->size == 2) {
            return 0x480;
        }
        if (type->size == 4) {
            return 0x481;
        }
        if (type->size == 8) {
            return 0x479;
        }
        if (type->size == 16) {
            return 0x489;
        }
        CError_Internal(mips_source_filename, 103);
        return 0;
    }
    if (kind == TYPEMEMBERPOINTER || kind == TYPEMEMBERPOINTER + 1) {
        return 0x481;
    }
    switch (TYPE_INTEGRAL(type)->integral) {
        case 0:
            return 0x47F;
        case 1:
            return 0x47F;
        case 2:
            return 0x47F;
        case 3:
            return 0x47F;
        case 4:
            return 0x480;
        case 5:
            return 0x480;
        case 6:
            return 0x480;
        case 7:
        case 8:
        case 9:
        case 10:
            if (type->size == 4) {
                return 0x481;
            }
            if (type->size != 8) {
                CError_Internal(mips_source_filename, 130);
            }
            return 0x479;
        case 11:
            return 0x479;
        case 12:
            return 0x479;
        case 13:
            return IS_TYPE_FLOAT_HARDWARE(type) ? 0x483 : 0x481;
        case 14:
        case 15:
        case 16:
            if (!IS_TYPE_FLOAT_HARDWARE(type)) {
                return 0x479;
            }
            {
                int hardware;
                return (copts.unknown0C >= 2 && ((hardware = copts.fp_library) & 2)) ? 0x47A
                                                                                     : 0x476;
            }
        case 17:
            return (copts.unknown0C >= 2 && (copts.fp_library & 2)) ? 0x47A : 0x476;
        case 18:
            return 0x489;
        default:
            CError_Internal(mips_source_filename, 154);
            return 0;
    }
}

int fn_0048E9D0(Type* type, int* isFloat) {
    UInt8 kind;
    UInt8 group;

    *isFloat = FALSE;
    kind = type->type;
    if (kind == TYPEENUM) {
        if (type->size == 1) {
            return 0x45A;
        }
        if (type->size == 2) {
            return 0x45B;
        }
        if (type->size == 4) {
            return 0x45F;
        }
        if (type->size == 8) {
            return 0x454;
        }
        CError_Internal(mips_source_filename, 168);
        return 0;
    }
    if (kind == TYPESTRUCT || kind == TYPECLASS || kind == TYPEARRAY) {
        if (type->size == 1) {
            return 0x45A;
        }
        if (type->size == 2) {
            return 0x45B;
        }
        if (type->size <= 4) {
            return 0x45F;
        }
        if (type->size <= 8) {
            return 0x454;
        }
        if (type->size <= 16) {
            return 0x467;
        }
        CError_Internal(mips_source_filename, 179);
        return 0;
    }
    group = kind - TYPEMEMBERPOINTER;
    if (group <= 1) {
        return 0x45F;
    }
    switch (TYPE_INTEGRAL(type)->integral) {
        case 0:
            return 0x453;
        case 1:
            return copts.unsigned_char ? 0x453 : 0x45A;
        case 2:
            return 0x45A;
        case 3:
            return 0x453;
        case 4:
            return 0x45C;
        case 5:
            return 0x45B;
        case 6:
            return 0x45C;
        case 7:
        case 8:
        case 9:
        case 10:
            if (type->size == 4) {
                return 0x45F;
            }
            if (type->size == 8) {
                return 0x454;
            }
            CError_Internal(mips_source_filename, 208);
            return 0;
        case 11:
        case 12:
            return 0x454;
        case 13:
            if (IS_TYPE_FLOAT_HARDWARE(type)) {
                *isFloat = TRUE;
                return 0x460;
            }
            return 0x45F;
        case 14:
        case 15:
        case 16:
            if (IS_TYPE_FLOAT_HARDWARE(type)) {
                *isFloat = TRUE;
                {
                    int hardware;
                    return (copts.unknown0C >= 2 && ((hardware = copts.fp_library) & 2)) ? 0x455
                                                                                         : 0x452;
                }
            }
            return 0x454;
        case 17:
            *isFloat = TRUE;
            {
                int hardware;
                return (copts.unknown0C >= 2 && ((hardware = copts.fp_library) & 2)) ? 0x455
                                                                                     : 0x452;
            }
        case 18:
            return 0x467;
        default:
            CError_Internal(mips_source_filename, 247);
            return 0;
    }
}

Operand copy_to_new_register(Operand* x) {
    emitpcode(move_opcode, used_virtual_registers[RegClass_GPR]++, x->reg);
    return operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
}

Operand emit_unary(UInt16 opcode, Operand* x, char kind) {
    PCode* instr;

    instr = make_instruction(opcode, 2);
    instr->args[0] = new_register_arg(kind);
    instr->args[1] = arg_from_operand(x);
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

static inline Operand emit_binary_immediate(UInt16 opcode, Operand* a, CInt64 value) {
    PCode* instr;

    instr = make_instruction(opcode, 3);
    instr->args[0] = new_register_arg(0);
    instr->args[1] = arg_from_operand(a);
    instr->args[2] = arg_from_cint64(value);
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand emit_binary(UInt16 opcode, Operand* a, Operand* b, char kind) {
    PCode* instr;

    if (b->kind == 3) {
        return emit_binary_immediate(opcode, a, b->immediate);
    }
    instr = make_instruction(opcode, 3);
    instr->args[0] = new_register_arg(kind);
    instr->args[1] = arg_from_operand(a);
    instr->args[2] = arg_from_operand(b);
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

int load_constant(SInt32 value) {
    int high;
    SInt32 low;
    int reg;

    if (!value) {
        return 0;
    }
    reg = used_virtual_registers[RegClass_GPR]++;
    if (value >= -0x8000 && value < 0x8000) {
        emitpcode(PC_ADDIU, reg, 0, value);
    } else if (value >= 0 && value < 0x10000) {
        emitpcode(PC_ORI, reg, 0, value);
    } else if (!(low = value & 0xFFFF)) {
        emitpcode(PC_LUI, reg, (UInt16)(value >> 16));
    } else {
        emitpcode(PC_LUI, high = used_virtual_registers[RegClass_GPR]++, (UInt16)(value >> 16));
        emitpcode(PC_ORI, reg, high, low);
    }
    return reg;
}

Operand make_constant_operand(SInt32 value) {
    return make_register_operand(load_constant(value), 0);
}

Operand emit_shift_left(Operand* x, int count, Type* type) {
    int reg;

    if (!x->reg || !count) {
        return *x;
    }
    reg = used_virtual_registers[RegClass_GPR]++;
    if ((type && (TYPE_IS_8BYTES(type) || TYPE_IS_16BYTES(type))) || count > 31) {
        if (count > 31) {
            emitpcode(PC_DSLL32, reg, x->reg, count - 32);
        } else {
            emitpcode(PC_DSLL, reg, x->reg, count);
        }
    } else {
        emitpcode(PC_SLL, reg, x->reg, count);
    }
    return make_register_operand(reg, 0);
}

Operand emit_shift_right(Operand* x, int count, Type* type) {
    Boolean is_unsigned_type;
    PCode* instr;

    is_unsigned_type = type && is_unsigned(type);
    if (!x->reg || !count) {
        return *x;
    }
    if ((type && (TYPE_IS_8BYTES(type) || TYPE_IS_16BYTES(type))) || count > 31) {
        if (count <= 31) {
            instr = makepcode(
                is_unsigned_type ? PC_DSRL : PC_DSRA,
                used_virtual_registers[RegClass_GPR]++,
                x->reg,
                count
            );
        } else if (count > 31) {
            instr = makepcode(
                is_unsigned_type ? PC_DSRL32 : PC_DSRA32,
                used_virtual_registers[RegClass_GPR]++,
                x->reg,
                count - 32
            );
        }
    } else {
        instr = makepcode(
            is_unsigned_type ? PC_SRL : PC_SRA,
            used_virtual_registers[RegClass_GPR]++,
            x->reg,
            count
        );
    }
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

void emit_branch_if_equal(Operand* x, SInt32 value, CLabel* label) {
    Operand op;
    PCodeLabel* fallthrough;
    PCode* instr;

    if (value) {
        op = make_register_operand(load_constant(value), 0);
        fallthrough = makepclabel();
        if (returnlabel == label) {
            branches_to_return = 1;
        }
        instr = make_instruction(PC_BEQ, 3);
        instr->args[0] = arg_from_operand(x);
        instr->args[1] = arg_from_operand(&op);
        instr->args[2] = arg_from_label(label);
        appendpcode(pclastblock, instr);
        pcaddedge(pclastblock, label->pclabel, 0);
        pcaddedge(pclastblock, fallthrough, 0);
        makepcblock();
        pclabelblock(pclastblock, fallthrough);
        return;
    }
    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = 1;
    }
    if (!label->pclabel) {
        label->pclabel = makepclabel();
    }
    instr = make_instruction(PC_BEQZ, 2);
    instr->args[0] = arg_from_operand(x);
    instr->args[1] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

void emit_branch_if_not_equal(Operand* x, SInt32 value, CLabel* label) {
    Operand op;
    PCodeLabel* fallthrough;
    PCode* instr;

    if (value) {
        op = make_register_operand(load_constant(value), 0);
        fallthrough = makepclabel();
        if (returnlabel == label) {
            branches_to_return = 1;
        }
        instr = make_instruction(PC_BNE, 3);
        instr->args[0] = arg_from_operand(x);
        instr->args[1] = arg_from_operand(&op);
        instr->args[2] = arg_from_label(label);
        appendpcode(pclastblock, instr);
        pcaddedge(pclastblock, label->pclabel, 0);
        pcaddedge(pclastblock, fallthrough, 0);
        makepcblock();
        pclabelblock(pclastblock, fallthrough);
        return;
    }
    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = 1;
    }
    if (!label->pclabel) {
        label->pclabel = makepclabel();
    }
    instr = make_instruction(PC_BNEZ, 2);
    instr->args[0] = arg_from_operand(x);
    instr->args[1] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

Operand emit_mtc1(Operand* x) {
    emitpcode(PC_MTC1, x->reg, used_virtual_registers[RegClass_FPR]++);
    setpcodeflags(0x4000);
    return operand_from_pcode_arg(pclastblock->lastPCode->args[1]);
}

void fn_00486660(UInt16 opcode, Operand* first, Operand* op) {
    Operand value;
    PCode* instr;
    int flags;

    flags = 4;
    if (op->object) {
        if (is_volatile_object(op->object)) {
            flags |= 0x80;
        }
        if (is_const_object(op->object)) {
            flags |= 0x40;
        }
    } else {
        flags |= 0x20;
    }
    value = *op;
    instr = make_instruction(opcode, 3);
    instr->args[0] = arg_from_operand(first);
    instr->args[1] = arg_from_operand(&value);
    instr->args[2] = fn_004D02D0(op->reg, 0, 1);
    if (instr->op == 0x489 || instr->op == 0x1D2 || instr->op == 0x268) {
        instr->args[0].effect |= 0x2000;
    }
    if (value.object) {
        instr->alias = fn_0054B360(op, instr);
    }
    instr->flags |= flags;
    appendpcode(pclastblock, instr);
}

Operand emit_operand_to_register(short opcode, Operand* op, char kind) {
    Operand value;
    Operand result;
    PCode* instr;
    int flags = 2;

    value = *op;
    if (value.object) {
        if (is_volatile_object(value.object)) {
            flags |= 0x80;
        }
        if (is_const_object(op->object)) {
            flags |= 0x40;
        }
    } else {
        flags |= 0x20;
    }
    instr = make_instruction(opcode, 3);
    instr->args[0] = new_register_arg(kind);
    instr->args[1] = arg_from_operand(&value);
    instr->args[2] = fn_004D02D0(op->reg, 0, 1);
    if (instr->op == 0x467 || instr->op == 0x1D1) {
        instr->args[0].effect |= 0x2000;
    }
    if (value.object) {
        instr->alias = fn_0054B360(op, instr);
    }
    instr->flags = flags;
    appendpcode(pclastblock, instr);
    result = operand_from_pcode_arg(instr->args[0]);
    return result;
}

Operand emit_mfc1(Operand* x) {
    emitpcode(PC_MFC1, used_virtual_registers[RegClass_GPR]++, x->reg);
    setpcodeflags(0x4000);
    return operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
}

Operand emit_mfc1_paired(Operand* x) {
    int reg;
    int src;
    Operand result;

    if (copts.unknown0F) {
        emitpcode(PC_MFC1, used_virtual_registers[RegClass_GPR]++, x->reg);
        pclastblock->lastPCode->args[1].effect |= 0x8000;
        setpcodeflags(0x100);
        result = operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
        src = x->reg;
        emitpcode(PC_MFC1, reg = used_virtual_registers[RegClass_GPR]++, src);
        pclastblock->lastPCode->args[1].effect |= 0xC000;
        setpcodeflags(0x100);
        result.regHi = reg;
        return result;
    }
    emitpcode(PC_DMFC1, used_virtual_registers[RegClass_GPR]++, x->reg);
    return operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
}

Operand emit_immediate_cint(UInt16 opcode, Operand* x, CInt64 value) {
    PCode* instr;

    instr = make_instruction(opcode, 3);
    instr->args[0] = new_register_arg(0);
    instr->args[1] = arg_from_operand(x);
    instr->args[2] = arg_from_cint64(value);
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand emit_immediate(UInt16 opcode, Operand* x, SInt32 value) {
    PCode* instr;

    instr = make_instruction(opcode, 3);
    instr->args[0] = new_register_arg(0);
    instr->args[1] = arg_from_operand(x);
    instr->args[2] = arg_from_int(value);
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand emit_shift_immediate(short opcode, Operand* x, int count) {
    PCode* instr;

    instr = makepcode(opcode, used_virtual_registers[RegClass_GPR]++, x->reg, count);
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

void emit_branch_to_pclabel(UInt16 opcode, PCodeLabel* label) {
    PCodeLabel* fallthrough;

    fallthrough = makepclabel();
    emitpcode(opcode, label);
    pcaddedge(pclastblock, label, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

static inline void set_comparison_immediate_operands(PCode* instr, Operand* op, SInt32 value) {
    instr->args[1] = arg_from_operand(op);
    instr->args[2] = arg_from_int(value);
}

static inline void set_comparison_constant_args(PCode* instr, Operand* op, const CInt64* constant) {
    instr->args[0] = new_register_arg(0);
    instr->args[1] = arg_from_operand(op);
    instr->args[2] = arg_from_cint64(*constant);
}

Operand fn_0048A100(Operand* op, SInt32 value) {
    Operand immediate;
    PCode* instr;
    CInt64 constant;
    PCodeArg arg;

    if (!value) {
        return make_register_operand(load_constant(0), 0);
    }
    if (value >= -0x8000 && value < 0x8000) {
        instr = make_instruction(PC_SLTIU, 3);
        instr->args[0] = new_register_arg(0);
        set_comparison_immediate_operands(instr, op, value);
        appendpcode(pclastblock, instr);
        return operand_from_pcode_arg(instr->args[0]);
    }
    immediate = make_register_operand(load_constant(value), 0);
    if (immediate.kind == 3) {
        constant = immediate.immediate;
        instr = make_instruction(0xD7, 3);
        set_comparison_constant_args(instr, op, &constant);
    } else {
        instr = make_instruction(0xD7, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        arg = arg_from_operand(&immediate);
        instr->args[2] = arg;
    }
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand emit_unsigned_less_equal_immediate(Operand* op, SInt32 value) {
    if (value != -1) {
        return fn_0048A100(op, value + 1);
    }
    return make_register_operand(load_constant(1), 0);
}

Operand fn_0048A7F0(Operand* op, SInt32 value) {
    Operand immediate;
    PCode* instr;
    CInt64 constant;
    PCodeArg extra;

    if (value == (-0x7fffffff - 1)) {
        return make_register_operand(load_constant(0), 0);
    }
    if (value >= -0x8000 && value < 0x8000) {
        instr = make_instruction(0xD5, 3);
        instr->args[0] = new_register_arg(0);
        set_comparison_immediate_operands(instr, op, value);
        appendpcode(pclastblock, instr);
        return operand_from_pcode_arg(instr->args[0]);
    }
    immediate = make_register_operand(load_constant(value), 0);
    if (immediate.kind == 3) {
        constant = immediate.immediate;
        instr = make_instruction(0xD4, 3);
        set_comparison_constant_args(instr, op, &constant);
    } else {
        instr = make_instruction(0xD4, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        extra = arg_from_operand(&immediate);
        instr->args[2] = extra;
    }
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand emit_signed_less_equal_immediate(Operand* op, SInt32 value) {
    if (value != 0x7FFFFFFF) {
        return fn_0048A7F0(op, value + 1);
    }
    return make_register_operand(load_constant(1), 0);
}

Operand fn_0048BEB0(Operand* op, SInt32 mask) {
    int reg;
    PCode* instr;

    if (mask == 0 || !op->reg) {
        return make_register_operand(0, 0);
    }
    if (mask == 0xFF) {
        instr = makepcode(PC_EXTU3, used_virtual_registers[RegClass_GPR]++, op->reg, 8);
        appendpcode(pclastblock, instr);
        return operand_from_pcode_arg(instr->args[0]);
    }
    if (mask == 0xFFFF) {
        instr = makepcode(PC_EXTU3, used_virtual_registers[RegClass_GPR]++, op->reg, 16);
        appendpcode(pclastblock, instr);
        return operand_from_pcode_arg(instr->args[0]);
    }
    if (mask == -1) {
        emitpcode(PC_DSLL32, reg = used_virtual_registers[RegClass_GPR]++, op->reg, 0);
        instr = makepcode(PC_DSRL32, used_virtual_registers[RegClass_GPR]++, reg, 0);
        goto append_shared;
    }
    if (!(mask & 0xFFFF0000)) {
        instr = makepcode(0x07, used_virtual_registers[RegClass_GPR]++, op->reg, (UInt16)mask);
        goto append_shared;
    }
    if (mask == 0x7FFFFFFF) {
        mask = 1;
    } else if (mask == 0x3FFFFFFF) {
        mask = 2;
    } else if (mask == 0x1FFFFFFF) {
        mask = 3;
    } else if (mask == 0xFFFFFFF) {
        mask = 4;
    } else if (mask == 0x7FFFFFF) {
        mask = 5;
    } else if (mask == 0x3FFFFFF) {
        mask = 6;
    } else if (mask == 0x1FFFFFF) {
        mask = 7;
    } else if (mask == 0xFFFFFF) {
        mask = 8;
    } else if (mask == 0x7FFFFF) {
        mask = 9;
    } else if (mask == 0x3FFFFF) {
        mask = 10;
    } else if (mask == 0x1FFFFF) {
        mask = 11;
    } else if (mask == 0xFFFFF) {
        mask = 12;
    } else if (mask == 0x7FFFF) {
        mask = 13;
    } else if (mask == 0x3FFFF) {
        mask = 14;
    } else if (mask == 0x1FFFF) {
        mask = 15;
    } else {
        instr =
            makepcode(PC_AND, used_virtual_registers[RegClass_GPR]++, op->reg, load_constant(mask));
        goto append_shared;
    }

    mask = 32 - mask;
    if (mask == 64) {
        return *op;
    }
    instr = makepcode(PC_EXTU3, used_virtual_registers[RegClass_GPR]++, op->reg, mask);
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
append_shared:
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand fn_0048D270(Operand* op) {
    Operand source;
    Operand result;
    Operand saved;
    CInt64 constant;
    PCode* instr;
    Object* object;

    if (op->object) {
        goto general;
    }
    switch (op->kind) {
        case 2:
            if (op->immOffset < -0x8000 || op->immOffset > 0x7FFF) {
                goto general;
            }
            goto short_offset;
        default:
            break;
    }
general:
    source = make_register_operand(op->reg, 0);
    if (source.kind == 3) {
        constant = source.immediate;
        instr = make_instruction(PC_LA, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_cint64(constant);
    } else {
        instr = make_instruction(PC_LA, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_operand(&source);
    }
    appendpcode(pclastblock, instr);
    saved = operand_from_pcode_arg(instr->args[0]);
    result = saved;
    object = op->object;
    if (object) {
        if (object->datatype == DLOCAL && Registers_GetVarInfo(object)) {
            pclastblock->lastPCode->alias = make_alias(object, op->immOffset - object->u.unk30, 1);
        } else {
            pclastblock->lastPCode->alias = make_alias(object, op->immOffset, 1);
        }
    } else {
        pclastblock->lastPCode->alias = 0;
    }
    return result;
short_offset:
    instr = makepcode(PC_ADDIU, used_virtual_registers[RegClass_GPR]++, op->reg, op->immOffset);
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand fn_0048DB70(SInt32 upper, SInt32 lower, Type* type) {
    Operand high;
    Operand shifted;
    Operand partial;
    PCode* instr;
    CInt64 immediate;
    int copy_upper;
    int low16;
    int reg;

    copy_upper = upper;
    if (copy_upper) {
        if (copy_upper == -1 && (UInt32)lower >= 0x80000000U) {
            return make_register_operand(load_constant(lower), 0);
        }
    } else if ((UInt32)lower <= 0x7FFFFFFFU) {
        return make_register_operand(load_constant(lower), 0);
    }
    high = make_register_operand(load_constant(upper), 0);
    shifted = emit_shift_left(&high, 32, type);
    if (lower > 0) {
        high = make_register_operand(load_constant(lower), 0);
        if (high.kind == 3) {
            immediate = high.immediate;
            instr = make_instruction(PC_OR, 3);
            instr->args[0] = new_register_arg(0);
            instr->args[1] = arg_from_operand(&shifted);
            instr->args[2] = arg_from_cint64(immediate);
        } else {
            instr = make_instruction(PC_OR, 3);
            instr->args[0] = new_register_arg(0);
            instr->args[1] = arg_from_operand(&shifted);
            instr->args[2] = arg_from_operand(&high);
        }
        appendpcode(pclastblock, instr);
        return operand_from_pcode_arg(instr->args[0]);
    }
    if (!lower) {
        return shifted;
    }
    high = make_register_operand(load_constant((UInt16)(lower >> 16)), 0);
    partial = emit_shift_left(&high, 16, type);
    low16 = lower & 0xFFFF;
    if (low16) {
        if (!(low16 & 0xFFFF0000)) {
            instr = makepcode(
                PC_ORI, reg = used_virtual_registers[RegClass_GPR]++, partial.reg, (UInt16)low16
            );
        } else {
            instr = makepcode(
                PC_OR,
                reg = used_virtual_registers[RegClass_GPR]++,
                partial.reg,
                load_constant(low16)
            );
        }
        appendpcode(pclastblock, instr);
        partial = operand_from_pcode_arg(instr->args[0]);
    }
    if (partial.kind == 3) {
        immediate = partial.immediate;
        instr = make_instruction(PC_OR, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(&shifted);
        instr->args[2] = arg_from_cint64(immediate);
    } else {
        instr = make_instruction(PC_OR, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(&shifted);
        instr->args[2] = arg_from_operand(&partial);
    }
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand fn_0048E350(CInt64 value, Type* type) {
    Operand upper;
    Operand lower;
    int first;
    int second;
    int reg;

    if (!TYPE_IS_8BYTES(type) && !TYPE_IS_16BYTES(type)) {
        return make_register_operand(load_constant(value.lo), 0);
    }
    if (!TYPE_IS_16BYTES(type) && CInt64_IsInRange(value, 4)) {
        return make_register_operand(load_constant(value.lo), 0);
    }
    if (!TYPE_IS_16BYTES(type) && CInt64_IsInRange(value, 8)) {
        return fn_0048DB70(value.unk8, value.lo, type);
    }
    if (!CInt64_IsInRange(value, 16)) {
        Operand zero = {0};
        CError_Internal(mips_source_filename, 389);
        return zero;
    }
    first = 0;
    second = 0;
    if (value.hi || value.unk4) {
        upper = fn_0048DB70(value.hi, value.unk4, type);
        first = upper.reg;
    }
    if (value.unk8 || value.lo) {
        lower = fn_0048DB70(value.unk8, value.lo, type);
        second = lower.reg;
    }
    emitpcode(0x21A, reg = used_virtual_registers[RegClass_GPR]++, second, first);
    lower = make_register_operand(reg, 0);
    return lower;
}

Operand fn_00489E00(Operand* op, CInt64* value, Type* type) {
    Operand immediate;
    PCode* instr;
    CInt64 constant;

    if (!TYPE_IS_8BYTES(type)) {
        return fn_0048A100(op, value->lo);
    }
    if (CInt64_IsInRange(*value, 4)) {
        return fn_0048A100(op, value->lo);
    }
    immediate = fn_0048E350(*value, type);
    if (immediate.kind == 3) {
        constant = immediate.immediate;
        instr = make_instruction(0xD7, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_cint64(constant);
    } else {
        instr = make_instruction(0xD7, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_operand(&immediate);
    }
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand fn_0048ABE0(Operand* a, CInt64* value, Type* type) {
    CInt64 incremented;

    if (TYPE_IS_8BYTES(type)) {
        if (CInt64_IsInRange(*value, 4)) {
            return emit_unsigned_less_equal_immediate(a, value->lo);
        }
        if (CInt64_Equal(*value, cint64_min)) {
            return make_register_operand(load_constant(1), 0);
        }
        incremented = CInt64_Add(*value, cint64_one);
        return fn_00489E00(a, &incremented, type);
    }
    return emit_unsigned_less_equal_immediate(a, value->lo);
}

Operand fn_0048B1A0(Operand* op, SInt32 value) {
    PCode* instr;
    CInt64 constant;
    Operand zero;
    Operand modified;

    zero = make_register_operand(0, 0);
    if (!value) {
        if (op->kind == 3) {
            constant = op->immediate;
            instr = make_instruction(0xD7, 3);
            instr->args[0] = new_register_arg(0);
            instr->args[1] = arg_from_operand(&zero);
            instr->args[2] = arg_from_cint64(constant);
            appendpcode(pclastblock, instr);
            return operand_from_pcode_arg(instr->args[0]);
        } else {
            instr = make_instruction(0xD7, 3);
            instr->args[0] = new_register_arg(0);
            instr->args[1] = arg_from_operand(&zero);
            instr->args[2] = arg_from_operand(op);
            appendpcode(pclastblock, instr);
            return operand_from_pcode_arg(instr->args[0]);
        }
    }
    modified = xor_operand_constant(op, value);
    zero = make_register_operand(0, 0);
    if (modified.kind == 3) {
        return emit_immediate_cint(0xD7, &zero, modified.immediate);
    }
    instr = make_instruction(0xD7, 3);
    instr->args[0] = new_register_arg(0);
    instr->args[1] = arg_from_operand(&zero);
    instr->args[2] = arg_from_operand(&modified);
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand fn_0048C8C0(Operand* op, SInt32 value, Type* type) {
    Operand temporary;
    PCode* instr;

    if (value == 0) {
        return *op;
    }
    if (type && TYPE_IS_8BYTES(type)) {
        if (value >= -0x8000 && value < 0x8000) {
            instr = make_instruction(0x78, 3);
            instr->args[0] = new_register_arg(0);
            instr->args[1] = arg_from_operand(op);
            instr->args[2] = arg_from_int(value);
            appendpcode(pclastblock, instr);
            return operand_from_pcode_arg(instr->args[0]);
        }
        if (value >= 0x8000 && value < 0xFFFF) {
            temporary = fn_0048C8C0(op, 0x7FFF, type);
            return fn_0048C8C0(&temporary, value - 0x7FFF, type);
        }
        if (value < -0x8000 && value >= -0x10000) {
            temporary = fn_0048C8C0(op, -0x8000, type);
            return fn_0048C8C0(&temporary, value + 0x8000, type);
        }
        goto large_wide;
    }
    if (value >= -0x8000 && value < 0x8000) {
        instr = make_instruction(3, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_int(value);
        appendpcode(pclastblock, instr);
        return operand_from_pcode_arg(instr->args[0]);
    }
    if (value >= 0x8000 && value < 0xFFFF) {
        temporary = fn_0048C8C0(op, 0x7FFF, 0);
        return fn_0048C8C0(&temporary, value - 0x7FFF, 0);
    }
    if (value < -0x8000 && value >= -0x10000) {
        temporary = fn_0048C8C0(op, -0x8000, 0);
        return fn_0048C8C0(&temporary, value + 0x8000, 0);
    }
    temporary = make_register_operand(load_constant(value), 0);
    if (temporary.kind == 3) {
        CInt64 immediate;
        immediate = temporary.immediate;
        instr = make_instruction(4, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_cint64(immediate);
    } else {
        instr = make_instruction(4, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_operand(&temporary);
    }
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
large_wide:
    temporary = make_register_operand(load_constant(value), 0);
    if (temporary.kind == 3) {
        CInt64 immediate;
        immediate = temporary.immediate;
        instr = make_instruction(0x79, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_cint64(immediate);
    } else {
        instr = make_instruction(0x79, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_operand(&temporary);
    }
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand fn_0048C5B0(Operand* op, CInt64* value, Type* type) {
    Operand immediate;
    PCode* instr;
    CInt64 constant;

    if (!TYPE_IS_8BYTES(type)) {
        return fn_0048C8C0(op, value->lo, type);
    }
    if (CInt64_IsInRange(*value, 4)) {
        return fn_0048C8C0(op, value->lo, type);
    }
    immediate = fn_0048E350(*value, type);
    if (immediate.kind == 3) {
        constant = immediate.immediate;
        instr = make_instruction(0x79, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_cint64(constant);
    } else {
        instr = make_instruction(0x79, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_operand(&immediate);
    }
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand fn_0048AE90(Operand* op, CInt64* value, Type* type) {
    Operand immediate;
    PCode* instr;
    CInt64 constant;

    if (!TYPE_IS_8BYTES(type)) {
        return fn_0048B1A0(op, value->lo);
    }
    if (CInt64_IsInRange(*value, 4)) {
        return fn_0048B1A0(op, value->lo);
    }
    immediate = fn_0048E350(*value, type);
    if (immediate.kind == 3) {
        constant = immediate.immediate;
        instr = make_instruction(0x49D, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_cint64(constant);
    } else {
        instr = make_instruction(0x49D, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_operand(&immediate);
    }
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand fn_0048B640(Operand* op, CInt64* value, Type* type) {
    Operand immediate;
    PCode* instr;
    CInt64 constant;

    if (!TYPE_IS_8BYTES(type)) {
        return emit_equal_constant(op, value->lo);
    }
    if (CInt64_IsInRange(*value, 4)) {
        return emit_equal_constant(op, value->lo);
    }
    immediate = fn_0048E350(*value, type);
    if (immediate.kind == 3) {
        constant = immediate.immediate;
        instr = make_instruction(0x48D, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_cint64(constant);
    } else {
        instr = make_instruction(0x48D, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_operand(&immediate);
    }
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand emit_equal_constant(Operand* value, int mask) {
    PCode* instr;
    Operand modified;

    if (!mask) {
        instr = make_instruction(PC_SLTIU, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(value);
        instr->args[2] = arg_from_int(1);
        appendpcode(pclastblock, instr);
        return operand_from_pcode_arg(instr->args[0]);
    }
    modified = xor_operand_constant(value, mask);
    instr = make_instruction(PC_SLTIU, 3);
    instr->args[0] = new_register_arg(0);
    instr->args[1] = arg_from_operand(&modified);
    instr->args[2] = arg_from_int(1);
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand xor_operand_constant(Operand* value, int mask) {
    Operand result;
    CInt64 constant;
    PCode* instr;

    if (value->kind == 3) {
        result = *value;
        CInt64_SetLong(&constant, mask);
        result.immediate = CInt64_Xor(value->immediate, constant);
        return result;
    }
    if (mask == 0) {
        return *value;
    }
    if (mask == -1) {
        instr = makepcode(0xBC, used_virtual_registers[RegClass_GPR]++, value->reg, 0);
    } else if (!(mask & 0xffff0000)) {
        instr =
            makepcode(PC_XORI, used_virtual_registers[RegClass_GPR]++, value->reg, (UInt16)mask);
    } else {
        instr = makepcode(
            PC_XOR, used_virtual_registers[RegClass_GPR]++, value->reg, load_constant(mask)
        );
    }
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand fn_00489D00(Operand* a, CInt64* b, Type* c) {
    Operand value;
    value = fn_0048ABE0(a, b, c);
    return xor_operand_constant(&value, 1);
}

Operand fn_0048A4F0(Operand* op, CInt64* value, Type* type) {
    Operand immediate;
    PCode* instr;
    CInt64 constant;

    if (!TYPE_IS_8BYTES(type)) {
        return fn_0048A7F0(op, value->lo);
    }
    if (CInt64_IsInRange(*value, 4)) {
        return fn_0048A7F0(op, value->lo);
    }
    immediate = fn_0048E350(*value, type);
    if (immediate.kind == 3) {
        constant = immediate.immediate;
        instr = make_instruction(0xD4, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_cint64(constant);
    } else {
        instr = make_instruction(0xD4, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_operand(&immediate);
    }
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand fn_0048AD30(Operand* a, CInt64* value, Type* type) {
    CInt64 incremented;

    if (TYPE_IS_8BYTES(type)) {
        if (CInt64_IsInRange(*value, 4)) {
            return emit_signed_less_equal_immediate(a, value->lo);
        }
        if (CInt64_Equal(*value, cint64_max)) {
            return make_register_operand(load_constant(1), 0);
        }
        incremented = CInt64_Add(*value, cint64_one);
        return fn_0048A4F0(a, &incremented, type);
    }
    return emit_signed_less_equal_immediate(a, value->lo);
}

Operand fn_00489D40(Operand* a, CInt64* b, Type* c) {
    Operand value;
    value = fn_0048AD30(a, b, c);
    return xor_operand_constant(&value, 1);
}

Operand fn_00489D80(Operand* a, CInt64* b, Type* c) {
    Operand value;
    value = fn_00489E00(a, b, c);
    return xor_operand_constant(&value, 1);
}

Operand fn_00489DC0(Operand* a, CInt64* b, Type* c) {
    Operand value;
    value = fn_0048A4F0(a, b, c);
    return xor_operand_constant(&value, 1);
}

void emit_branch_to_label(UInt16 opcode, CLabel* label) {
    PCode* instr;
    PCodeLabel* fallthrough;

    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = 1;
    }
    instr = make_instruction(opcode, 1);
    instr->args[0] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

Operand emit_immediate_with_mode(UInt16 opcode, Operand* op, SInt32 value, int mode) {
    PCode* instr;

    instr = make_instruction(opcode, 3);
    instr->args[0] = new_register_arg(0);
    instr->args[1] = arg_from_operand(op);
    instr->unk38 = mode;
    instr->args[2] = arg_from_int(value);
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

void emit_paired_registers(Operand operand, int opcode, int low, Boolean both) {
    int base;

    if (low == 0) {
        base = 2;
    } else {
        base = 0;
    }
    if (both) {
        if (copts.littleendian) {
            emitpcode((short)opcode, base, operand.reg);
            emitpcode((short)opcode, base + 1, operand.regHi);
        } else {
            emitpcode((short)opcode, base + 1, operand.reg);
            emitpcode((short)opcode, base, operand.regHi);
        }
    } else {
        emitpcode((short)opcode, base, operand.reg);
    }
}

int parse_mips_operand_effects(const char* format) {
    int flags = 0;
    Boolean pair = FALSE;
    int effect;

    for (; *format; format++) {
        effect = 1;
        if (*format == '=') {
            effect++;
            format++;
        }
        if (*format == '+') {
            effect = 3;
            format++;
        }
        if (*format == '!') {
            effect |= 0x8000;
            format++;
        }
        if (*format == '$') {
            effect |= 0x2000;
            format++;
        }
        if (*format == '2') {
            pair = TRUE;
            format++;
            if (*format == '+') {
                format++;
            }
        }
        if (*format == '\'') {
            format++;
        }
        switch (*format) {
            case '0':
                if (effect & 1) {
                    flags |= 0x2000;
                }
                if (effect & 2) {
                    flags |= 0x1000;
                }
                break;
            case '1':
                if (effect & 1) {
                    flags |= 0x8000;
                }
                if (effect & 2) {
                    flags |= 0x4000;
                }
                break;
            case '3':
                if (effect & 1) {
                    flags |= 0x8000;
                }
                if (effect & 2) {
                    flags |= 0x4000;
                }
                break;
            case 'r':
                if (effect & 2) {
                    flags |= 0x800;
                }
                break;
            case 's':
                if (effect & 1) {
                    flags |= 0x2;
                }
                break;
            case 't':
                if (effect & 1) {
                    flags |= 0x1;
                }
                break;
            case 'H':
            case 'h':
                if (effect & 1) {
                    flags |= 0x10000;
                }
                if (effect & 2) {
                    flags |= 0x20000;
                }
                break;
            case 'R':
                if (!pair && (effect & 1)) {
                    flags |= 0x40;
                }
                break;
            case 'S':
                if (effect & 1) {
                    flags |= 0x100;
                }
                break;
            case 'N':
            case 'T':
                if (effect & 1) {
                    flags |= 0x10;
                }
                break;
            case 'E':
                if (effect & 1) {
                    flags |= 0xa000;
                }
                if (effect & 2) {
                    flags |= 0x5000;
                }
                break;
            case 'G':
                if (effect & 1) {
                    flags |= 0xa000;
                }
                if (effect & 2) {
                    flags |= 0x5000;
                }
                break;
            case '&':
            case '(':
            case ')':
            case ',':
            case '-':
            case '9':
            case ';':
            case '<':
            case '>':
            case '@':
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'F':
            case 'I':
            case 'J':
            case 'K':
            case 'L':
            case 'M':
            case 'O':
            case 'P':
            case 'Q':
            case 'U':
            case 'V':
            case 'W':
            case 'X':
            case 'Y':
            case 'Z':
            case '[':
            case ']':
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'i':
            case 'j':
            case 'k':
            case 'l':
            case 'o':
            case 'p':
            case 'q':
            case 'u':
            case 'v':
            case 'x':
            case 'z':
            case '|':
            case '}':
                break;
            default:
                CError_FATAL(2331);
                break;
        }
    }
    return flags;
}

//! @reconstructed
int count_mips_format_operands(const char* format) {
    int count;
    int ch;
    int is_digit;

    count = 0;
    for (; *format; format++) {
        ch = *format;
        is_digit = (ch >= 0 && ch < 256) ? (_MSL_CMAP_ACCESS[ch] & __msl_digit) : 0;
        if (!is_digit) {
            switch (*format) {
                case 'r':
                    count++;
                    format++;
                    break;
                case 's':
                    count++;
                    format++;
                    break;
                case 'd':
                case 'i':
                case 'l':
                case 'm':
                case 't':
                case 'w':
                    count++;
                    break;
                case 'p':
                    format++;
                    break;
                case '#':
                case '(':
                case ')':
                case ',':
                case ';':
                case '<':
                case '=':
                case '>':
                case '@':
                    break;
                default:
                    CError_Internal(mips_source_filename, 2136);
                    break;
            }
        }
    }
    return count;
}

int count_mips_operands(const char* format) {
    int count;

    count = 0;
    for (; *format; format++) {
        switch (*format) {
            case '&':
            case '0':
            case '1':
            case '3':
            case '<':
            case '>':
            case '@':
            case 'B':
            case 'H':
            case 'P':
            case 'X':
            case 'Y':
            case 'Z':
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'h':
            case 'i':
            case 'j':
            case 'k':
            case 'l':
            case 'p':
            case 'q':
            case 'r':
            case 's':
            case 't':
            case 'u':
            case 'v':
            case 'x':
            case 'z':
                count += 1;
                break;
            case '9':
            case 'D':
            case 'E':
            case 'G':
            case 'J':
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'R':
            case 'S':
            case 'T':
            case 'V':
            case 'W':
                count += 1;
                break;
            case 'U':
                count += 1;
                break;
            case 'A':
                count += 2;
                break;
            case 'o':
                count += 1;
                break;
            case 'C':
            case 'F':
            case 'I':
            case 'Q':
            case 'f':
                count += 1;
                break;
            case '!':
            case '$':
            case '\'':
            case '(':
            case ')':
            case '+':
            case ',':
            case '-':
            case '2':
            case ';':
            case '=':
            case '[':
            case ']':
            case '|':
            case '}':
                break;
            default:
                CError_FATAL(2083);
                break;
        }
    }
    return count;
}

Operand emit_extend(UInt16 opcode, Operand* x, int count) {
    PCode* instr;

    CError_ASSERT(1695, opcode == PC_EXT3 || opcode == PC_EXTU3);
    if (count == 64) {
        return *x;
    }
    instr = makepcode(opcode, used_virtual_registers[RegClass_GPR]++, x->reg, count);
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

void emit_branch_to_label_lazy(UInt16 opcode, CLabel* label) {
    PCode* instr;
    PCodeLabel* fallthrough;

    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = 1;
    }
    if (!label->pclabel) {
        label->pclabel = makepclabel();
    }
    instr = make_instruction(opcode, 1);
    instr->args[0] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

Operand emit_or_immediate_operand(Operand* x, SInt32 value) {
    PCode* instr;

    if (value == 0) {
        return *x;
    }
    if (!(value & 0xFFFF0000)) {
        instr = makepcode(PC_ORI, used_virtual_registers[RegClass_GPR]++, x->reg, (UInt16)value);
    } else {
        instr =
            makepcode(PC_OR, used_virtual_registers[RegClass_GPR]++, x->reg, load_constant(value));
    }
    appendpcode(pclastblock, instr);
    return operand_from_pcode_arg(instr->args[0]);
}

Operand emit_mtc1_paired(Operand* x) {
    int reg;

    if (copts.unknown0F) {
        reg = used_virtual_registers[RegClass_FPR]++;
        emitpcode(PC_MTC1, x->reg, reg);
        pclastblock->lastPCode->args[1].effect |= 0x8000;
        setpcodeflags(0x100);
        emitpcode(PC_MTC1, x->regHi, reg);
        pclastblock->lastPCode->args[1].effect |= 0xC000;
        setpcodeflags(0x100);
        return operand_from_pcode_arg(pclastblock->lastPCode->args[1]);
    }
    reg = used_virtual_registers[RegClass_FPR]++;
    emitpcode(PC_DMTC1, x->reg, reg);
    return operand_from_pcode_arg(pclastblock->lastPCode->args[1]);
}

void emit_branch(UInt16 opcode, Operand* x, CLabel* label) {
    PCode* instr;
    PCodeLabel* fallthrough;

    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = 1;
    }
    if (!label->pclabel) {
        label->pclabel = makepclabel();
    }
    instr = make_instruction(opcode, 2);
    instr->args[0] = arg_from_operand(x);
    instr->args[1] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

void fn_00489170(Operand* op, SInt32 value, CLabel* label) {
    Operand temp;
    PCodeLabel* fallthrough;
    PCode* instr;

    if (!value) {
        return;
    }
    if (value != 1) {
        temp = fn_0048A100(op, value);
        fallthrough = makepclabel();
        if (returnlabel == label) {
            branches_to_return = TRUE;
        }
        if (!label->pclabel) {
            label->pclabel = makepclabel();
        }
        instr = make_instruction(0x2A, 2);
        instr->args[0] = arg_from_operand(&temp);
        instr->args[1] = arg_from_label(label);
        appendpcode(pclastblock, instr);
        pcaddedge(pclastblock, label->pclabel, 0);
        pcaddedge(pclastblock, fallthrough, 0);
        makepcblock();
        pclabelblock(pclastblock, fallthrough);
        return;
    }
    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = TRUE;
    }
    if (!label->pclabel) {
        label->pclabel = makepclabel();
    }
    instr = make_instruction(0x1A, 2);
    instr->args[0] = arg_from_operand(op);
    instr->args[1] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

void fn_00489400(Operand* op, SInt32 value, CLabel* label) {
    PCodeLabel* fallthrough;
    PCode* instr;

    if (value == 0) {
        fallthrough = makepclabel();
        if (returnlabel == label) {
            branches_to_return = TRUE;
        }
        if (!label->pclabel) {
            label->pclabel = makepclabel();
        }
        instr = make_instruction(0x24, 2);
        instr->args[0] = arg_from_operand(op);
        instr->args[1] = arg_from_label(label);
        appendpcode(pclastblock, instr);
        pcaddedge(pclastblock, label->pclabel, 0);
        pcaddedge(pclastblock, fallthrough, 0);
        makepcblock();
        pclabelblock(pclastblock, fallthrough);
        return;
    }
    if (value == 1) {
        fallthrough = makepclabel();
        if (returnlabel == label) {
            branches_to_return = TRUE;
        }
        if (!label->pclabel) {
            label->pclabel = makepclabel();
        }
        instr = make_instruction(0x22, 2);
        instr->args[0] = arg_from_operand(op);
        instr->args[1] = arg_from_label(label);
        appendpcode(pclastblock, instr);
        pcaddedge(pclastblock, label->pclabel, 0);
        pcaddedge(pclastblock, fallthrough, 0);
        makepcblock();
        pclabelblock(pclastblock, fallthrough);
        return;
    }
    if (value == (-0x7fffffff - 1)) {
        return;
    }
    {
        Operand temp = fn_0048A7F0(op, value);
        fallthrough = makepclabel();
        if (returnlabel == label) {
            branches_to_return = TRUE;
        }
        if (!label->pclabel) {
            label->pclabel = makepclabel();
        }
        instr = make_instruction(0x2A, 2);
        instr->args[0] = arg_from_operand(&temp);
        instr->args[1] = arg_from_label(label);
        appendpcode(pclastblock, instr);
        pcaddedge(pclastblock, label->pclabel, 0);
        pcaddedge(pclastblock, fallthrough, 0);
        makepcblock();
        pclabelblock(pclastblock, fallthrough);
        return;
    }
}

void fn_004875B0(Operand* op, CInt64* value, Type* type, CLabel* label) {
    Operand constant;
    Operand inrange;
    Operand wide;
    PCodeLabel* fallthrough;
    PCode* instr;

    if (!TYPE_IS_8BYTES(type)) {
        if (value->lo) {
            inrange = make_register_operand(load_constant(value->lo), 0);
            fallthrough = makepclabel();
            if (returnlabel == label) {
                branches_to_return = TRUE;
            }
            instr = make_instruction(0x28, 3);
            instr->args[0] = arg_from_operand(op);
            instr->args[1] = arg_from_operand(&inrange);
            instr->args[2] = arg_from_label(label);
            appendpcode(pclastblock, instr);
            pcaddedge(pclastblock, label->pclabel, 0);
            pcaddedge(pclastblock, fallthrough, 0);
            makepcblock();
            pclabelblock(pclastblock, fallthrough);
            return;
        }
        fallthrough = makepclabel();
        if (returnlabel == label) {
            branches_to_return = TRUE;
        }
        if (!label->pclabel) {
            label->pclabel = makepclabel();
        }
        instr = make_instruction(0x2A, 2);
        instr->args[0] = arg_from_operand(op);
        instr->args[1] = arg_from_label(label);
        appendpcode(pclastblock, instr);
        pcaddedge(pclastblock, label->pclabel, 0);
        pcaddedge(pclastblock, fallthrough, 0);
        makepcblock();
        pclabelblock(pclastblock, fallthrough);
        return;
    }
    if (CInt64_IsInRange(*value, 4)) {
        if (value->lo) {
            wide = make_register_operand(load_constant(value->lo), 0);
            fallthrough = makepclabel();
            if (returnlabel == label) {
                branches_to_return = TRUE;
            }
            instr = make_instruction(0x28, 3);
            instr->args[0] = arg_from_operand(op);
            instr->args[1] = arg_from_operand(&wide);
            instr->args[2] = arg_from_label(label);
            appendpcode(pclastblock, instr);
            pcaddedge(pclastblock, label->pclabel, 0);
            pcaddedge(pclastblock, fallthrough, 0);
            makepcblock();
            pclabelblock(pclastblock, fallthrough);
            return;
        }
        fallthrough = makepclabel();
        if (returnlabel == label) {
            branches_to_return = TRUE;
        }
        if (!label->pclabel) {
            label->pclabel = makepclabel();
        }
        instr = make_instruction(0x2A, 2);
        instr->args[0] = arg_from_operand(op);
        instr->args[1] = arg_from_label(label);
        appendpcode(pclastblock, instr);
        pcaddedge(pclastblock, label->pclabel, 0);
        pcaddedge(pclastblock, fallthrough, 0);
        makepcblock();
        pclabelblock(pclastblock, fallthrough);
        return;
    }
    constant = fn_0048E350(*value, type);
    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = TRUE;
    }
    instr = make_instruction(0x28, 3);
    instr->args[0] = arg_from_operand(op);
    instr->args[1] = arg_from_operand(&constant);
    instr->args[2] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

void fn_00487D10(Operand* op, CInt64* value, Type* type, CLabel* label) {
    Operand constant;
    Operand inrange;
    Operand wide;
    PCodeLabel* fallthrough;
    PCode* instr;

    if (!TYPE_IS_8BYTES(type)) {
        if (value->lo) {
            inrange = make_register_operand(load_constant(value->lo), 0);
            fallthrough = makepclabel();
            if (returnlabel == label) {
                branches_to_return = TRUE;
            }
            instr = make_instruction(0x18, 3);
            instr->args[0] = arg_from_operand(op);
            instr->args[1] = arg_from_operand(&inrange);
            instr->args[2] = arg_from_label(label);
            appendpcode(pclastblock, instr);
            pcaddedge(pclastblock, label->pclabel, 0);
            pcaddedge(pclastblock, fallthrough, 0);
            makepcblock();
            pclabelblock(pclastblock, fallthrough);
            return;
        }
        fallthrough = makepclabel();
        if (returnlabel == label) {
            branches_to_return = TRUE;
        }
        if (!label->pclabel) {
            label->pclabel = makepclabel();
        }
        instr = make_instruction(0x1A, 2);
        instr->args[0] = arg_from_operand(op);
        instr->args[1] = arg_from_label(label);
        appendpcode(pclastblock, instr);
        pcaddedge(pclastblock, label->pclabel, 0);
        pcaddedge(pclastblock, fallthrough, 0);
        makepcblock();
        pclabelblock(pclastblock, fallthrough);
        return;
    }
    if (CInt64_IsInRange(*value, 4)) {
        if (value->lo) {
            wide = make_register_operand(load_constant(value->lo), 0);
            fallthrough = makepclabel();
            if (returnlabel == label) {
                branches_to_return = TRUE;
            }
            instr = make_instruction(0x18, 3);
            instr->args[0] = arg_from_operand(op);
            instr->args[1] = arg_from_operand(&wide);
            instr->args[2] = arg_from_label(label);
            appendpcode(pclastblock, instr);
            pcaddedge(pclastblock, label->pclabel, 0);
            pcaddedge(pclastblock, fallthrough, 0);
            makepcblock();
            pclabelblock(pclastblock, fallthrough);
            return;
        }
        fallthrough = makepclabel();
        if (returnlabel == label) {
            branches_to_return = TRUE;
        }
        if (!label->pclabel) {
            label->pclabel = makepclabel();
        }
        instr = make_instruction(0x1A, 2);
        instr->args[0] = arg_from_operand(op);
        instr->args[1] = arg_from_label(label);
        appendpcode(pclastblock, instr);
        pcaddedge(pclastblock, label->pclabel, 0);
        pcaddedge(pclastblock, fallthrough, 0);
        makepcblock();
        pclabelblock(pclastblock, fallthrough);
        return;
    }
    constant = fn_0048E350(*value, type);
    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = TRUE;
    }
    instr = make_instruction(0x18, 3);
    instr->args[0] = arg_from_operand(op);
    instr->args[1] = arg_from_operand(&constant);
    instr->args[2] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

void fn_00488470(Operand* op, Operand* value, CLabel* label) {
    PCode* instr;
    Operand result;
    Operand copy;
    PCodeLabel* fallthrough;
    CInt64 constant;

    if (value->kind == 3) {
        constant = value->immediate;
        instr = make_instruction(0xD7, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_cint64(constant);
        appendpcode(pclastblock, instr);
        result = operand_from_pcode_arg(instr->args[0]);
    } else {
        instr = make_instruction(0xD7, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_operand(value);
        appendpcode(pclastblock, instr);
        result = operand_from_pcode_arg(instr->args[0]);
    }
    copy = result;
    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = TRUE;
    }
    if (!label->pclabel) {
        label->pclabel = makepclabel();
    }
    instr = make_instruction(0x2A, 2);
    instr->args[0] = arg_from_operand(&copy);
    instr->args[1] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

void fn_00488860(Operand* op, Operand* value, CLabel* label) {
    PCode* instr;
    Operand result;
    Operand copy;
    PCodeLabel* fallthrough;
    CInt64 constant;

    if (value->kind == 3) {
        constant = value->immediate;
        instr = make_instruction(0xD4, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_cint64(constant);
        appendpcode(pclastblock, instr);
        result = operand_from_pcode_arg(instr->args[0]);
    } else {
        instr = make_instruction(0xD4, 3);
        instr->args[0] = new_register_arg(0);
        instr->args[1] = arg_from_operand(op);
        instr->args[2] = arg_from_operand(value);
        appendpcode(pclastblock, instr);
        result = operand_from_pcode_arg(instr->args[0]);
    }
    copy = result;
    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = TRUE;
    }
    if (!label->pclabel) {
        label->pclabel = makepclabel();
    }
    instr = make_instruction(0x2A, 2);
    instr->args[0] = arg_from_operand(&copy);
    instr->args[1] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

void emit_type_constant_comparison(Operand* result, CInt64* value, Type* type, CLabel* label) {
    Operand constant;

    if (TYPE_IS_8BYTES(type) && !CInt64_IsInRange(*value, 4)) {
        constant = fn_0048E350(*value, type);
        if (is_unsigned(type)) {
            fn_00488470(result, &constant, label);
        } else {
            fn_00488860(result, &constant, label);
        }
        return;
    }
    if (is_unsigned(type)) {
        fn_00489170(result, value->lo, label);
    } else {
        fn_00489400(result, value->lo, label);
    }
}

void emit_branch_with_immediate(UInt16 opcode, Operand* op, SInt32 value, CLabel* label) {
    Operand immediate;
    PCode* instr;
    PCodeLabel* fallthrough;

    immediate.kind = 3;
    CInt64_SetLong(&immediate.immediate, value);
    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = 1;
    }
    instr = make_instruction(opcode, 3);
    instr->args[0] = arg_from_operand(op);
    instr->args[1] = arg_from_operand(&immediate);
    instr->args[2] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

void emit_branch_with_two_operands(UInt16 opcode, Operand* a, Operand* b, CLabel* label) {
    PCode* instr;
    PCodeLabel* fallthrough;

    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = 1;
    }
    instr = make_instruction(opcode, 3);
    instr->args[0] = arg_from_operand(a);
    instr->args[1] = arg_from_operand(b);
    instr->args[2] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

void fn_004897C0(Operand* op, SInt32 value, CLabel* label) {
    Operand temp;
    PCode* instr;
    PCodeLabel* fallthrough;

    if (value) {
        if (value != -1) {
            temp = fn_0048A100(op, value + 1);
            fallthrough = makepclabel();
            if (returnlabel == label) {
                branches_to_return = TRUE;
            }
            if (!label->pclabel) {
                label->pclabel = makepclabel();
            }
            instr = make_instruction(0x2A, 2);
            instr->args[0] = arg_from_operand(&temp);
            instr->args[1] = arg_from_label(label);
            appendpcode(pclastblock, instr);
            pcaddedge(pclastblock, label->pclabel, 0);
            pcaddedge(pclastblock, fallthrough, 0);
            makepcblock();
            pclabelblock(pclastblock, fallthrough);
            return;
        }
        branch_always(label->pclabel);
        return;
    }
    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = TRUE;
    }
    if (!label->pclabel) {
        label->pclabel = makepclabel();
    }
    instr = make_instruction(0x1A, 2);
    instr->args[0] = arg_from_operand(op);
    instr->args[1] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

void fn_00489A60(Operand* op, SInt32 value, CLabel* label) {
    Operand temp;
    PCode* instr;
    PCodeLabel* fallthrough;

    if (value) {
        if (value != 0x7FFFFFFF) {
            temp = fn_0048A7F0(op, value + 1);
            fallthrough = makepclabel();
            if (returnlabel == label) {
                branches_to_return = TRUE;
            }
            if (!label->pclabel) {
                label->pclabel = makepclabel();
            }
            instr = make_instruction(0x2A, 2);
            instr->args[0] = arg_from_operand(&temp);
            instr->args[1] = arg_from_label(label);
            appendpcode(pclastblock, instr);
            pcaddedge(pclastblock, label->pclabel, 0);
            pcaddedge(pclastblock, fallthrough, 0);
            makepcblock();
            pclabelblock(pclastblock, fallthrough);
            return;
        }
        branch_always(label->pclabel);
        return;
    }
    fallthrough = makepclabel();
    if (returnlabel == label) {
        branches_to_return = TRUE;
    }
    if (!label->pclabel) {
        label->pclabel = makepclabel();
    }
    instr = make_instruction(0x1A, 2);
    instr->args[0] = arg_from_operand(op);
    instr->args[1] = arg_from_label(label);
    appendpcode(pclastblock, instr);
    pcaddedge(pclastblock, label->pclabel, 0);
    pcaddedge(pclastblock, fallthrough, 0);
    makepcblock();
    pclabelblock(pclastblock, fallthrough);
}

void emit_branch_type_constant(Operand* op, CInt64* value, Type* type, CLabel* label) {
    Operand constant;
    PCode* instr;
    PCodeLabel* fallthrough;
    UInt16 opcode;

    if (TYPE_IS_8BYTES(type) && !CInt64_IsInRange(*value, 4)) {
        constant = fn_0048E350(*value, type);
        opcode = is_unsigned(type) ? 0x423 : 0x41F;
        fallthrough = makepclabel();
        if (returnlabel == label) {
            branches_to_return = 1;
        }
        instr = make_instruction(opcode, 3);
        instr->args[0] = arg_from_operand(op);
        instr->args[1] = arg_from_operand(&constant);
        instr->args[2] = arg_from_label(label);
        appendpcode(pclastblock, instr);
        pcaddedge(pclastblock, label->pclabel, 0);
        pcaddedge(pclastblock, fallthrough, 0);
        makepcblock();
        pclabelblock(pclastblock, fallthrough);
        return;
    }
    if (is_unsigned(type)) {
        fn_004897C0(op, value->lo, label);
    } else {
        fn_00489A60(op, value->lo, label);
    }
}

Operand make_paired_register_operand(int opcode, int float_reg, Boolean paired) {
    PCode* instr;
    Operand first;
    Operand second;
    int kind;
    int reg;

    if (!float_reg) {
        kind = 2;
    } else {
        kind = 0;
    }
    if (!float_reg) {
        reg = used_virtual_registers[RegClass_GPR]++;
    } else {
        reg = used_virtual_registers[RegClass_FPR]++;
    }
    instr = makepcode(opcode, reg, kind);
    appendpcode(pclastblock, instr);
    first = operand_from_pcode_arg(instr->args[0]);
    if (paired) {
        if (!float_reg) {
            reg = used_virtual_registers[RegClass_GPR]++;
        } else {
            reg = used_virtual_registers[RegClass_FPR]++;
        }
        instr = makepcode(opcode, reg, kind + 1);
        appendpcode(pclastblock, instr);
        second = operand_from_pcode_arg(instr->args[0]);
        if (copts.littleendian) {
            first.regHi = second.reg;
            if (!float_reg) {
                first.kind = 0;
            }
            return first;
        }
        second.regHi = first.reg;
        if (!float_reg) {
            second.kind = 0;
        }
        return second;
    }
    return first;
}

void fn_00485D90(
    UInt32 opcode,
    const char* name,
    const char* format,
    UInt32 encoding,
    UInt32 flags,
    const char* intrinsicName,
    UInt32 otherFlags,
    UInt16 field2C
) {
    UInt8 width;
    UInt16 op;
    UInt32 effects;

    opcode_info[opcode].name = name;
    opcode_info[opcode].unk4 = 0;
    opcode_info[opcode].format = format;
    opcode_info[opcode].encoding = encoding;
    if ((SInt32)opcode >= 0x37E && (SInt32)opcode <= 0x3E5) {
        effects = 0;
    } else {
        effects = parse_mips_operand_effects(format);
    }
    opcode_info[opcode].flags2 = effects;
    opcode_info[opcode].unk28 = otherFlags;
    opcode_info[opcode].field2C = field2C;
    opcode_info[opcode].flags = flags;
    op = opcode;
    if (op < 0x37E || op > 0x3E5) {
        goto force_wide;
    }
    if ((opcode_info[opcode].flags & 0x1000) == 0) {
        width = (opcode_info[opcode].encoding & 0xFFFF0000) ? 4 : 2;
    } else {
        switch (op) {
            case 0x3CD:
            case 0x3D6:
            case 0x3D8:
            case 0x3DB:
            case 0x3DE:
                width = 4;
                break;
            default:
                width = 2;
                break;
        }
    }
    goto store_width;
force_wide:
    width = 4;
store_width:;
    opcode_info[opcode].instructionWidth = width;
    if ((SInt32)opcode < 0x37E || (SInt32)opcode > 0x3E5) {
        opcode_info[opcode].operandCount = count_mips_operands(format);
    } else {
        opcode_info[opcode].operandCount = count_mips_format_operands(format);
    }
    if (intrinsicName) {
        opcode_info[opcode].unk20 = intrinsicName;
        opcode_info[opcode].unk24 = mips_opcode_count_607E20;
        mips_opcode_sequence_625CD0[mips_opcode_count_607E20] = op;
        ++mips_opcode_count_607E20;
    }
}

void fn_00485CF0(
    SInt32 opcode,
    const char* name,
    const char* format,
    UInt16 macro1,
    UInt16 macro2,
    UInt16 field2C
) {
    int index;

    opcode_info[opcode].name = name;
    opcode_info[opcode].unk4 = 0;
    opcode_info[opcode].format = format;
    opcode_info[opcode].encoding = 0;
    opcode_info[opcode].flags2 = parse_mips_operand_effects(format);
    opcode_info[opcode].unk20 = 0;
    opcode_info[opcode].unk24 = 0;
    opcode_info[opcode].instructionWidth = 0;
    opcode_info[opcode].operandCount = count_mips_operands(format);
    opcode_info[opcode].field2C = field2C;
    index = opcode - 0x40A;
    mips_macro_format_636A08[index] = format;
    be_macro_expanded_opcode_6371D0[index] = macro1;
    be_macro_expanded_opcode2_637082[index] = macro2;
}
