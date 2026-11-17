#include "compiler/Operands.h"
#include "compiler/AliasPropagation.h"
#include "compiler/Registers.h"
#include "compiler/RegisterInfo.h"
#include "compiler/CodeGen.h"
#include "compiler/CParser.h"
#include "compiler/InstrSelection.h"
#include "compiler/mips.h"
#include "compiler/PCodeInfo.h"
#include "compiler/PCode.h"
#include "compiler/VReg.h"
#include "compiler/types.h"
#include "compiler/objects.h"
#include "compiler/Switch.h"
#include "compiler/enode.h"
#include "compiler/CFunc.h"
#include "compiler/CDecl.h"
#include "compiler/CompilerTools.h"
#include "compiler/PCodeUtilities.h"
#include "compiler/BE_macro.h"
#include "compiler/CMachine.h"
#include "compiler/FunctionCalls.h"
#include "compiler/ObjGen.h"
#include "compiler/BE_binary.h"

Operand get_high_half_operand(Operand* operand) {
    Operand result;

    CError_ASSERT(54, operand->kind != 3);
    result.kind = operand->kind;
    result.reg = operand->regHi;
    return result;
}

Operand convert_operand_type(Operand* op, Type* type) {
    PCode* instr;
    Operand result;
    short opcode;

    if (IS_TYPE_FLOAT_HARDWARE(type)) {
        opcode = IS_TYPE_FLOAT32(type) ? 0x4A3 : 0x4A4;
        instr = makepcode(opcode, used_virtual_registers[RegClass_FPR]++, op->reg);
        result = operand_from_pcode_arg(instr->args[0]);
        appendpcode(pclastblock, instr);
        return result;
    }
    if (TYPE_IS_16BYTES(type)) {
        emitpcode(move_opcode_637350, used_virtual_registers[RegClass_GPR]++, op->reg);
        return operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
    }
    return copy_to_new_register(op);
}

int move_fpr_register(int dest, int source, char kind, int width) {
    int result;
    int reg;
    short opcode;

    if (kind == 1 && (dest == source || dest < 0)) {
        result = source;
    } else {
        if (dest < 0) {
            reg = used_virtual_registers[RegClass_FPR]++;
        } else {
            reg = dest;
        }
        result = reg;
        switch (kind) {
            case 0:
                opcode = width == 8 ? PC_DMTC1 : PC_MTC1;
                emitpcode(opcode, source, reg);
                break;
            case 1:
                opcode = width == 8 ? 0xF8 : 0xF9;
                emitpcode(opcode, reg, source);
                break;
            case 5:
            case 6: {
                int gpr = move_operand_register(-1, source, kind, width);
                opcode = width == 8 ? PC_DMTC1 : PC_MTC1;
                emitpcode(opcode, gpr, reg);
                break;
            }
            default:
                CError_FATAL(226);
                break;
        }
    }
    return result;
}

int move_operand_register(int dest, int source, char kind, int width) {
    short opcode;
    int result;
    int newreg;

    if (kind == 0 && (dest == source || dest < 0)) {
        result = source;
    } else {
        if (dest < 0) {
            newreg = used_virtual_registers[RegClass_GPR]++;
        } else {
            newreg = dest;
        }
        result = newreg;
        switch (kind) {
            case 0:
                opcode = width == 16 ? move_opcode_637350 : move_opcode;
                emitpcode(opcode, dest, source);
                break;
            case 1:
                opcode = width == 8 ? 0x7F : PC_MFC1;
                emitpcode(opcode, newreg, source);
                break;
            case 5:
                emitpcode(0x264, newreg, source);
                break;
            case 6:
                if (source == 1) {
                    emitpcode(0x1CF, newreg, 0x15);
                } else if (source == 2) {
                    emitpcode(0x1CF, newreg, 0x16);
                } else if (source == 4) {
                    emitpcode(0x1CF, newreg, 0x14);
                } else {
                    CError_FATAL(180);
                }
                break;
            default:
                CError_FATAL(186);
                break;
        }
    }
    return result;
}

void fn_004B1440(Operand source, Operand* target, Type* type) {
    SInt16 opcode;
    Object* object;
    VarInfo* location;
    ObjRegInfo* reginfo;
    Operand value;
    SInt8 kind;
    SInt8 dest_kind;
    int dest;
    int reg;
    int width;
    Type* member;

    object = target->object;
    if (object && object->datatype == DLOCAL) {
        location = Registers_GetVarInfo(object);
    } else {
        location = NULL;
    }
    if (source.object && source.object->datatype == DLOCAL) {
        Registers_GetVarInfo(source.object);
    }
    if (target->object) {
        is_volatile_object(target->object);
    }
    switch (target->kind) {
        case 2:
        case 4:
            if (location && location->reg) {
                if (IS_TYPE_FLOAT_HARDWARE(type)) {
                    opcode = IS_TYPE_FLOAT32(type) ? 0x4A3 : 0x4A4;
                    emitpcode(opcode, (SInt16)location->reg, source.reg);
                    return;
                }
                opcode = fn_0048ECB0(type);
                switch ((UInt16)opcode) {
                    case 0x47F:
                        if (source.reg && type->type != TYPEENUM) {
                            value = emit_extend(is_unsigned(type) ? PC_EXTU3 : PC_EXT3, &source, 8);
                        } else {
                            value = source;
                        }
                        break;
                    case 0x480:
                        if (source.reg && type->type != TYPEENUM) {
                            value =
                                emit_extend(is_unsigned(type) ? PC_EXTU3 : PC_EXT3, &source, 16);
                        } else {
                            value = source;
                        }
                        break;
                    case 0x479:
                    case 0x481:
                    case 0x489:
                        value = source;
                        break;
                    default:
                        CError_FATAL(0x28D);
                        break;
                }
                if (value.kind == 0) {
                    kind = 0;
                } else if (value.kind == 1) {
                    kind = 1;
                } else if (value.kind == 5) {
                    kind = 5;
                } else {
                    CError_FATAL(0x29C);
                    return;
                }
                dest_kind = (SInt8)location->unknown2E;
                dest = location->reg;
                reg = value.reg;
                width = type->size;
                switch (dest_kind) {
                    case 0:
                        move_operand_register(dest, reg, kind, width);
                        break;
                    case 1:
                        move_fpr_register(dest, reg, kind, width);
                        break;
                    case 5:
                        if (dest >= 0 && kind == 5) {
                            if (dest != reg) {
                                emitpcode(0x2AF, 15, dest, reg);
                            }
                        } else {
                            if (kind != 0) {
                                reg = move_operand_register(-1, reg, kind, width);
                            }
                            if (dest < 0) {
                                dest = used_virtual_registers[5]++;
                            }
                            emitpcode(0x266, reg, dest);
                        }
                        break;
                    case 6:
                        if (kind != 0) {
                            reg = move_operand_register(-1, reg, kind, width);
                        }
                        if (dest == 1) {
                            emitpcode(0x1D0, 0x15, reg);
                        } else if (dest == 2) {
                            emitpcode(0x1D0, 0x16, reg);
                        } else if (dest == 4) {
                            emitpcode(0x1CF, 0x14, reg);
                        } else {
                            CError_FATAL(0x11B);
                        }
                        break;
                    default:
                        CError_FATAL(0x140);
                        break;
                }
                return;
            }
            if (fn_004727A0(target->object)) {
                reginfo = Registers_GetVarInfo(target->object);

                if (IS_TYPE_FLOAT_HARDWARE(type)) {
                    opcode = IS_TYPE_FLOAT32(type) ? 0x4A3 : 0x4A4;
                    emitpcode(opcode, (SInt16)reginfo->reg, source.reg);
                    return;
                }
                opcode = fn_0048ECB0(type);
                switch ((UInt16)opcode) {
                    case 0x479:
                    case 0x481:
                        value = source;
                        break;
                    case 0x47F:
                        if (source.reg == 0) {
                            value = source;
                        } else if (type->type == TYPEENUM) {
                            value = source;
                        } else {
                            value = emit_extend(is_unsigned(type) ? PC_EXTU3 : PC_EXT3, &source, 8);
                        }
                        break;
                    case 0x480:
                        if (source.reg && type->type != TYPEENUM) {
                            value =
                                emit_extend(is_unsigned(type) ? PC_EXTU3 : PC_EXT3, &source, 16);
                        } else {
                            value = source;
                        }
                        break;
                    case 0x489:
                        value = source;
                        break;
                    default:
                        CError_FATAL(0x2F1);
                        return;
                }
                if (value.kind == 0) {
                    kind = 0;
                } else if (value.kind == 1) {
                    kind = 1;
                } else if (value.kind == 5) {
                    kind = 5;
                } else {
                    CError_FATAL(0x2FF);
                    return;
                }
                dest_kind = (SInt8)reginfo->rclass;
                dest = reginfo->reg;
                reg = value.reg;
                width = type->size;
                switch (dest_kind) {
                    case 0:
                        move_operand_register(dest, reg, kind, width);
                        break;
                    case 1:
                        move_fpr_register(dest, reg, kind, width);
                        break;
                    case 5:
                        if (dest >= 0 && kind == 5) {
                            if (dest != reg) {
                                emitpcode(0x2AF, 15, dest, reg);
                            }
                        } else {
                            if (kind != 0) {
                                reg = move_operand_register(-1, reg, kind, width);
                            }
                            if (dest < 0) {
                                dest = used_virtual_registers[5]++;
                            }
                            emitpcode(0x266, reg, dest);
                        }
                        break;
                    case 6:
                        if (kind != 0) {
                            reg = move_operand_register(-1, reg, kind, width);
                        }
                        if (dest == 1) {
                            emitpcode(0x1D0, 0x15, reg);
                        } else if (dest == 2) {
                            emitpcode(0x1D0, 0x16, reg);
                        } else if (dest == 4) {
                            emitpcode(0x1CF, 0x14, reg);
                        } else {
                            CError_FATAL(0x11B);
                        }
                        break;
                    default:
                        CError_FATAL(0x140);
                        break;
                }
                return;
            }
            if (copts.unknown15 && (type->type == TYPESTRUCT || type->type == TYPECLASS)) {
                if ((member = fn_0046FF70(type)) != NULL) {
                    opcode = fn_0048ECB0(member);
                } else if ((member = fn_00470030(type)) != NULL) {
                    opcode = fn_0048ECB0(member);
                } else {
                    opcode = fn_0048ECB0(type);
                }
            } else {
                opcode = fn_0048ECB0(type);
            }
            break;
        default:
            CError_FATAL(0x317);
            break;
    }
    if (opcode == 0x489 && source.kind == 5) {
        fn_00486660(0x268, &source, target);
    } else {
        fn_00486660((UInt16)opcode, &source, target);
    }
}

void Coerce_to_register(Operand* op, Type* type, SInt16 output_reg) {
    PCode* instr;
    VarInfo* location;
    ObjRegInfo* reginfo;
    Operand result;
    int reg;
    int input_reg;
    int flags = 0;
    int isFloat;
    SInt16 opcode;

    switch (op->kind) {
        case 0:
        case 1:
            return;
        case 5: {
            register SInt8 kind;
            int width;
            int source;
            if (type->type == TYPEFLOAT) {
                kind = 1;
            } else {
                kind = 0;
            }
            width = type->size;
            source = op->reg;
            switch (kind) {
                case 0:
                    reg = move_operand_register(-1, source, 5, width);
                    break;
                case 1:
                    reg = move_fpr_register(-1, source, 5, width);
                    break;
                case 5:
                    reg = move_operand_register(-1, source, 5, width);
                    input_reg = reg;
                    reg = used_virtual_registers[5]++;
                    emitpcode(0x266, input_reg, reg);
                    break;
                case 6:
                    move_operand_register(-1, source, 5, width);
                    CError_FATAL(0x11B);
                    reg = -1;
                    break;
                default:
                    CError_FATAL(0x140);
                    break;
            }
            *op = make_register_operand(reg, kind);
            return;
        }
        case 3:
            *op = fn_0048E350(op->immediate, type);
            if (output_reg) {
                *op = convert_operand_type(op, type);
            }
            return;
        case 2:
            if (op->immOffset != 0 || op->reg < n_real_registers[0]) {
                *op = fn_0048D270(op);
            } else {
                *op = operand_from_pcode_arg(fn_004D02D0(op->reg, 0, 1));
            }
            return;
        case 4:
            location = NULL;
            if (!op->object) {
                flags |= 0x20;
            }
            if (op->object && is_volatile_object(op->object)) {
                flags |= 0x80;
            }
            flags |= op->flags;
            if (op->object && !(op->object->qual & 0x10000)) {
                if (fn_004727A0(op->object)) {
                    reginfo = Registers_GetVarInfo(op->object);
                    if (reginfo) {
                        SInt8 kind;
                        SInt8 source_kind;
                        int source;
                        int width;
                        if (type->type == TYPEFLOAT) {
                            kind = 1;
                        } else {
                            kind = 0;
                        }
                        source_kind = reginfo->rclass;
                        source = reginfo->reg;
                        width = type->size;
                        switch (kind) {
                            case 0:
                                reg = move_operand_register(-1, source, source_kind, width);
                                break;
                            case 1:
                                reg = move_fpr_register(-1, source, source_kind, width);
                                break;
                            case 5:
                                reg = source;
                                if (source_kind != 0) {
                                    reg = move_operand_register(-1, reg, source_kind, width);
                                }
                                input_reg = reg;
                                reg = used_virtual_registers[5]++;
                                emitpcode(0x266, input_reg, reg);
                                break;
                            case 6:
                                if (source_kind != 0) {
                                    move_operand_register(-1, source, source_kind, width);
                                }
                                CError_FATAL(0x11B);
                                reg = -1;
                                break;
                            default:
                                CError_FATAL(0x140);
                                break;
                        }
                        *op = make_register_operand(reg, kind);
                        return;
                    }
                    CError_FATAL(0x18F);
                }
                if (op->object->datatype == DLOCAL) {
                    location = Registers_GetVarInfo(op->object);
                }
            }
            if (location && location->reg) {
                if (location->rclass == 1) {
                    if (output_reg) {
                        opcode = IS_TYPE_FLOAT32(type) ? 0x4A3 : 0x4A4;
                        instr = makepcode(
                            opcode, used_virtual_registers[RegClass_FPR]++, location->reg
                        );
                        *op = operand_from_pcode_arg(instr->args[0]);
                        appendpcode(pclastblock, instr);
                    } else {
                        *op = make_register_operand(location->reg, 1);
                    }
                } else {
                    if (output_reg) {
                        if (TYPE_IS_16BYTES(type)) {
                            instr = makepcode(
                                move_opcode_637350,
                                used_virtual_registers[RegClass_GPR]++,
                                location->reg
                            );
                        } else {
                            instr = makepcode(
                                move_opcode, used_virtual_registers[RegClass_GPR]++, location->reg
                            );
                        }
                        *op = operand_from_pcode_arg(instr->args[0]);
                        appendpcode(pclastblock, instr);
                    } else {
                        *op = make_register_operand(location->reg, 0);
                    }
                }
                return;
            }
            opcode = fn_0048E9D0(type, &isFloat);
            instr = make_instruction((UInt16)opcode, 3);
            instr->args[0] = new_register_arg((SInt8)isFloat);
            if (opcode ==
                (SInt16)((copts.unknown0C >= 2 && (copts.fp_library & 2)) ? 0x455 : 0x452)) {
                instr->args[0].effect |= 0x8000;
                if (copts.unknown1A) {
                    instr->args[0].effect |= 0x20;
                }
            }
            instr->args[1] = arg_from_operand(op);
            instr->args[2] = fn_004D02D0(op->reg, 0, 1);
            if (op->object) {
                instr->alias = fn_0054B360(op, instr);
            }
            result = operand_from_pcode_arg(instr->args[0]);
            instr->flags |= flags;
            appendpcode(pclastblock, instr);
            if (op->bitfield) {
                extract_bitfield(result, op->bitfield, op);
            } else {
                *op = result;
            }
            if (!instr->alias) {
                instr->flags |= 0x20;
            }
            return;
        default:
            CError_FATAL(0x216);
            return;
    }
}
void force_operand_register(Operand* operand, Type* type, int reg) {
    if (operand->kind) {
        Coerce_to_register(operand, type, 1);
    }
}

void extract_bitfield(Operand operand, TypeBitfield* tbitfield, Operand* result) {
    Type* type;
    int bitlength;
    Boolean isUnsigned;
    int used;
    int shift;

    type = tbitfield->bitfieldtype;
    bitlength = tbitfield->bitlength;
    isUnsigned = is_unsigned(type);
    if (!copts.littleendian) {
        used = type->size * 8 - tbitfield->offset;
        shift = register_size * 8 - used;
        if (shift > 0 && used > 0) {
            *result = emit_shift_left(&operand, shift, type);
        } else {
            *result = operand;
        }
        shift = register_size * 8 - bitlength;
        if (shift > 0) {
            *result = emit_shift_right(result, shift, type);
        }
    } else if (tbitfield->offset - 0 > 0 || tbitfield->offset - 0 < 0) {
        shift = register_size * 8 - (bitlength + tbitfield->offset);
        if (shift > 0) {
            *result = emit_shift_left(&operand, shift, type);
        } else {
            *result = operand;
        }
        shift = register_size * 8 - bitlength;
        if (shift > 0) {
            *result = emit_shift_right(result, shift, type);
        }
    } else if (register_size * 8 - bitlength > 0) {
        *result = emit_extend(isUnsigned ? PC_EXTU3 : PC_EXT3, &operand, bitlength);
    } else {
        *result = operand;
    }
}

void indirect(Operand* op, ENode* expr) {
    switch (op->kind) {
        case 4:
            if (op->kind) {
                Coerce_to_register(op, TYPE(&void_ptr), 0);
            }
        case 0:
        case 1:
            op->immOffset = 0;
            op->object = NULL;
        case 2:
            op->kind = 4;
            break;
        case 3:
            if (CInt64_IsInRange(op->immediate, 4) || CInt64_IsInURange(op->immediate, 4)) {
                op->reg = 0;
                op->immOffset = op->immediate.lo;
            } else {
                CError_FATAL(90);
            }
            op->object = NULL;
            op->kind = 4;
            break;
        default:
            CError_FATAL(96);
            break;
    }
    op->flags = 0;
    if (expr) {
        if (CParserIsVolatileExpr(expr)) {
            op->flags |= 0x80;
        }
        if (CParserIsConstExpr(expr)) {
            op->flags |= 0x40;
        }
    }
}
void fn_004B0B30(Operand source, Operand* target, TypeBitfield* bitfield) {
    Type* type;
    SInt32 width;
    SInt32 pos;
    SInt32 shift;
    SInt32 reg;
    Operand extended;

    width = bitfield->bitlength;
    type = bitfield->bitfieldtype;
    extended = emit_extend(0x48C, &source, width);
    if (!copts.littleendian) {
        Operand result;
        CInt64 amount;
        CInt64 mask;
        pos = type->size * 8 - bitfield->offset;
        shift = pos - width;
        extended = emit_shift_left(&extended, shift, type);
        if (pos > 31) {
            CInt64_SetLong(&amount, width);
            mask = CInt64_Sub(CInt64_Shl(cint64_one, amount), cint64_one);
            CInt64_SetLong(&amount, shift);
            mask = CInt64_Inv(CInt64_Shl(mask, amount));
            result = fn_0048E350(mask, (Type*)&stsignedlonglong);
            reg = used_virtual_registers[RegClass_GPR]++;
            emitpcode(6, reg, target->reg, result.reg);
            target->reg = reg;
        } else {
            result = fn_0048BEB0(target, ~(((1 << width) - 1) << shift));
            *target = result;
        }
    } else {
        Operand result;
        CInt64 amount;
        CInt64 mask;
        shift = bitfield->offset;
        extended = emit_shift_left(&extended, shift, type);
        if (width + shift > 31) {
            CInt64_SetLong(&amount, width);
            mask = CInt64_Sub(CInt64_Shl(cint64_one, amount), cint64_one);
            CInt64_SetLong(&amount, shift);
            mask = CInt64_Inv(CInt64_Shl(mask, amount));
            result = fn_0048E350(mask, (Type*)&stsignedlonglong);
            reg = used_virtual_registers[RegClass_GPR]++;
            emitpcode(6, reg, target->reg, result.reg);
            target->reg = reg;
        } else {
            result = fn_0048BEB0(target, ~(((1 << width) - 1) << shift));
            *target = result;
        }
    }
    reg = used_virtual_registers[RegClass_GPR]++;
    emitpcode(0xBE, reg, target->reg, extended.reg);
    *target = operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
}

void pass_argument(Operand* operand, Type* type, int reg) {
    SInt32 isDouble;
    UInt8 integral;
    SInt32 index;
    SInt32 words;
    SInt32 bytes;

    integral = TYPE_INTEGRAL(type)->integral;
    isDouble = integral == IT_DOUBLE || (type->type == TYPEFLOAT && type->size == 8);
    if (type->type == TYPEFLOAT &&
        (((integral == IT_FLOAT || (type->type == TYPEFLOAT && type->size == 4)) &&
          !(copts.fp_library & 1)) ||
         ((integral == IT_DOUBLE || (type->type == TYPEFLOAT && type->size == 8)) &&
          !(copts.fp_library & 2)))) {
        words = 1;
        index = 4;
        index += reg - 1;
        emitpcode(move_opcode, index, (UInt16)operand->reg);
        be_macro_arg_register_mask_609070 |= 1U << index;
    } else {
        index = 12;
        index += (reg - 1) * 2;
        if ((UInt8)isDouble) {
            words = 2;
            emitpcode(0x4A4, index, (UInt16)operand->reg);
            be_macro_arg_float_mask_609074 |= 3U << index;
        } else {
            words = 1;
            emitpcode(0x4A3, index, (UInt16)operand->reg);
            be_macro_arg_float_mask_609074 |= 1U << index;
        }
    }
    if (!copts.unknown15) {
        bytes = words * register_size;
        if (be_macro_adjust_offset_6356A4 > bytes) {
            bytes = be_macro_adjust_offset_6356A4;
        }
        be_macro_adjust_offset_6356A4 = (bytes + 7) & ~7;
    }
}

void fn_004B00C0(ENode* expr, SwitchInfo* info) {
    CInt64 current;
    CInt64 lowest = cint64_max;
    CInt64 highest = cint64_min;
    CInt64 span;
    CInt64 three;
    Type* type;
    double density;
    SInt32 entries;
    SInt32 count;

    type = expr->rtype;
    switch_count_6090D4 = 0;
    for (switch_current_6366D0 = info->cases; switch_current_6366D0;
         switch_current_6366D0 = switch_current_6366D0->next) {
        if (!switch_current_6366D0->label->pclabel) {
            switch_current_6366D0->label->pclabel = makepclabel();
        }
        lowest = CInt64_Less(lowest, switch_current_6366D0->bounds.low)
            ? lowest
            : switch_current_6366D0->bounds.low;
        highest = CInt64_Greater(highest, switch_current_6366D0->bounds.low)
            ? highest
            : switch_current_6366D0->bounds.low;
        highest = CInt64_Greater(highest, switch_current_6366D0->bounds.high)
            ? highest
            : switch_current_6366D0->bounds.high;
        switch_count_6090D4++;
    }
    CInt64_SetLong(&three, 3);
    if (CInt64_Greater(lowest, cint64_zero) && CInt64_Less(lowest, three)) {
        CInt64_SetLong(&lowest, 0);
    }
    span = CInt64_Sub(highest, lowest);
    if (CInt64_IsZero(&span)) {
        density = 0.0;
    } else {
        density = (double)switch_count_6090D4 / CInt64_ConvertUToLongDouble(&span);
    }
    if (!info->defaultlabel->pclabel) {
        info->defaultlabel->pclabel = makepclabel();
    }
    gen_func_table[expr->type](expr, 0, &switch_operand_635BB4);
    if (switch_operand_635BB4.kind) {
        Coerce_to_register(&switch_operand_635BB4, type, 0);
    }
    if (density <= switch_table_density_5E8F70 || switch_count_6090D4 < 6) {
        switch_current_6366D0 = info->cases;
        for (count = 0; count < switch_count_6090D4; count++) {
            if (!CInt64_Greater(
                    switch_current_6366D0->bounds.high, switch_current_6366D0->bounds.low
                )) {
                fn_00487D10(
                    &switch_operand_635BB4,
                    &switch_current_6366D0->bounds.low,
                    type,
                    switch_current_6366D0->label
                );
            } else {
                CLabel* next = newlabel();
                next->pclabel = makepclabel();
                emit_type_constant_comparison(
                    &switch_operand_635BB4, &switch_current_6366D0->bounds.low, type, next
                );
                emit_branch_type_constant(
                    &switch_operand_635BB4,
                    &switch_current_6366D0->bounds.high,
                    type,
                    switch_current_6366D0->label
                );
                emit_label(next->pclabel);
            }
            switch_current_6366D0 = switch_current_6366D0->next;
        }
        branch_always(info->defaultlabel->pclabel);
    } else {
        PCodeArg** table;
        PCodeArg* defaultArg;
        Object* tableObject;
        ObjectList* tableLink;
        Operand tableOperand;
        SInt32 index;
        SInt32 reg;
        SInt32 reg2;
        SInt32 reg3;

        entries = span.lo + 1;
        table = lalloc(entries * sizeof(PCodeArg*));
        defaultArg = lalloc(sizeof(PCodeArg));
        *defaultArg = arg_from_label(info->defaultlabel);
        for (index = 0; index < entries; index++) {
            table[index] = defaultArg;
        }
        switch_current_6366D0 = info->cases;
        for (count = 0; count < switch_count_6090D4; count++) {
            three = CInt64_Sub(switch_current_6366D0->bounds.low, lowest);
            {
                PCodeArg* arg = lalloc(sizeof(PCodeArg));
                *arg = arg_from_label(switch_current_6366D0->label);
                table[three.lo] = arg;
            }
            if (CInt64_Greater(
                    switch_current_6366D0->bounds.high, switch_current_6366D0->bounds.low
                )) {
                for (current = CInt64_Add(switch_current_6366D0->bounds.low, cint64_one);
                     CInt64_LessEqual(current, switch_current_6366D0->bounds.high);
                     current = CInt64_Add(switch_current_6366D0->bounds.low, cint64_one)) {
                    PCodeArg* arg;
                    three = CInt64_Sub(current, lowest);
                    arg = lalloc(sizeof(PCodeArg));
                    *arg = arg_from_label(switch_current_6366D0->label);
                    table[three.lo] = arg;
                }
            }
            switch_current_6366D0 = switch_current_6366D0->next;
        }
        tableObject = galloc(0x44);
        memclrw(tableObject, 0x44);
        tableLink = galloc(sizeof(ObjectList));
        memclrw(tableLink, sizeof(ObjectList));
        tableLink->object = tableObject;
        tableLink->next = binary_fixup_list;
        binary_fixup_list = tableLink;
        tableObject->name = CParser_GetUniqueName();
        tableObject->type = CDecl_NewArrayType((Type*)&void_ptr, entries * 4);
        tableObject->sclass = 0x102;
        tableObject->datatype = DDATA;
        tableObject->qual = 1;
        tableObject->section = objgen_string_scope_6364D4;
        tableObject->u.unk2C = entries * 4;
        tableObject->u.varptr = table;
        tableOperand.kind = 2;
        tableOperand.reg = 0x1c;
        tableOperand.object = tableObject;
        tableOperand.immOffset = 0;
        if (!CInt64_IsZero(&lowest)) {
            switch_operand_635BB4 = emit_immediate_cint(0x4A0, &switch_operand_635BB4, lowest);
        }
        emit_branch_with_immediate(0x415, &switch_operand_635BB4, entries, info->defaultlabel);
        if (tableOperand.kind) {
            Coerce_to_register(&tableOperand, NULL, 0);
        }
        reg = used_virtual_registers[RegClass_GPR]++;
        emitpcode(0xD2, reg, switch_operand_635BB4.reg, 2);
        reg2 = used_virtual_registers[RegClass_GPR]++;
        emitpcode(4, reg2, (SInt16)reg, tableOperand.reg);
        reg3 = used_virtual_registers[RegClass_GPR]++;
        emitpcode(0x9D, (SInt16)reg3, 0, 0, (SInt16)reg2);
        emitpcode(0x3B, (SInt16)reg3);
        for (switch_current_6366D0 = info->cases; switch_current_6366D0;
             switch_current_6366D0 = switch_current_6366D0->next) {
            pcaddedge(pclastblock, switch_current_6366D0->label->pclabel, 0);
        }
        pcaddedge(pclastblock, info->defaultlabel->pclabel, 0);
        new_basic_block();
    }
}

SInt16 object_base_register(Object* object) {
    ObjGenRegisterLocation location;
    fn_0046EDD0(object, &location);
    return location.number;
}

void call_runtime_function(Object* object) {
    codegen_register_option_637385 = 0;
    fn_004B3BD0(object, 0, &be_macro_arg_register_mask_609070);
}

Operand get_call_result(Type* type) {
    UInt8 integral;
    UInt16 opcode;

    integral = TYPE_INTEGRAL(type)->integral;
    if (type->type == TYPEFLOAT &&
        (((integral == IT_FLOAT || (type->type == TYPEFLOAT && type->size == 4)) &&
          !(copts.fp_library & 1)) ||
         ((integral == IT_DOUBLE || (type->type == TYPEFLOAT && type->size == 8)) &&
          !(copts.fp_library & 2)))) {
        return make_paired_register_operand(move_opcode, 0, FALSE);
    }
    opcode = (integral == IT_FLOAT || (type->type == TYPEFLOAT && type->size == 4)) ? 0x4A3 : 0x4A4;
    return make_paired_register_operand(opcode, 1, FALSE);
}

void pass_int_argument(Operand* operand, int unused, int reg) {
    SInt32 index;
    UInt32 bytes;

    index = 4;
    index += reg - 1;
    emitpcode(move_opcode, index, (UInt16)operand->reg);
    be_macro_arg_register_mask_609070 |= 1U << index;
    if (!copts.unknown15) {
        bytes = register_size;
        if (be_macro_adjust_offset_6356A4 > (SInt32)bytes) {
            bytes = be_macro_adjust_offset_6356A4;
        }
        be_macro_adjust_offset_6356A4 = (bytes + 7) & ~7;
    }
}

Operand get_call_result_int(int wide) {
    return make_paired_register_operand(move_opcode, 0, FALSE);
}

void fn_004AFDE0(void) {
    be_macro_accumulated_stack_6361CC = 0;
    be_macro_adjust_offset_6356A4 = 0;
    be_macro_stack_base_635A40 = 0;
    be_macro_frame_padding_636438 = 0;
    be_macro_frame_padding2_6364CC = 0;
}

void assign_local_memory(Object* object) {
    SInt16 alignment;

    alignment = CMach_GetObjectAlign(object);
    be_macro_stack_base_635A40 += object->type->size;
    be_macro_stack_base_635A40 += alignment - 1;
    be_macro_stack_base_635A40 &= ~(alignment - 1);
    object->u.unk30 = -be_macro_stack_base_635A40;
}

SInt32 fn_004AFC70(Object* location) {
    SInt32 extra = 0;
    SInt32 offset;
    SInt32 mask;

    if (copts.unknown15) {
        mask = stack_alignment_63560C - 1;
        extra = ((codegen_operand_max_6363EC + mask) & ~mask) - codegen_operand_max_6363EC;
    }
    offset = asm_stack_frame_offset_636E78;
    if (location->u.var.info->flags & 1) {
        offset = asm_stack_frame_offset_636E78 + extra;
    }
    return location->u.unk30 + offset;
}

SInt32 fn_004AFCC0(Operand* operand) {
    Operand temporary;
    UInt32 alignment;
    UInt32 offset;
    if (operand->kind == 3) {
        offset = operand->immediate.lo;
        if (offset == 0) {
            offset = 0;
        } else {
            alignment = stack_alignment_63560C - 1;
            offset = (~alignment) & (alignment + offset);
        }
        fn_0048F530(0x4A0, 0x1D, 0x1D, offset);
    } else {
        if (operand->kind != 0) {
            Coerce_to_register(operand, (Type*)&stsignedint, 0);
        }
        alignment = stack_alignment_63560C - 1;
        temporary = emit_immediate(3, operand, alignment);
        temporary = fn_0048BEB0(&temporary, ~(stack_alignment_63560C - 1));
        emitpcode(0xDD, 0x1D, 0x1D, temporary.reg);
    }
    temporary = make_register_operand(0x1D, 0);
    temporary = emit_immediate_with_mode(3, &temporary, 0, 0x11);
    return temporary.reg;
}
