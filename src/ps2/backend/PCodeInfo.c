#include "compiler/CParser.h"
#include "compiler/Alias.h"
#define CERROR_FILE "PCodeInfo.c"
#include "compiler/PCodeInfo.h"
#include "compiler/PCode.h"
#include "compiler/objects.h"
#include "compiler/CError.h"
#include "compiler/RegisterInfo.h"

#include "compiler/CodeGen.h"

#include "compiler/CompilerTools.h"
#include <string.h>

PCode* vformatpcode(UInt16 opcode, va_list args) {
    OpcodeInfo* info;
    const char* format;
    PCode* pcode;
    PCodeArg* operand;
    PCodeArg* arg;
    SInt32 count;
    UInt32 size;
    SInt32 effect;
    SInt32 number;
    Boolean pair;
    Boolean quad;
    Object* object;

    info = &opcode_info[opcode];
    format = info->format;
    count = (UInt8)info->unkC;
    pair = FALSE;
    quad = FALSE;
    if (*format == '#') {
        count += va_arg(args, SInt32);
        format++;
    }
    size = sizeof(PCode) + count * sizeof(PCodeArg);
    pcode = lalloc(size);
    memclrw(pcode, size);
    pcode->op = opcode;
    pcode->argCount = count;
    pcode->flags = info->flags;
    operand = pcode->args;
    while (*format) {
        if (*format == ',' || *format == ';' || *format == '(') {
            format++;
        }
        effect = 1;
        if (*format == '=') {
            effect = 2;
            format++;
        } else if (*format == '+') {
            effect = 3;
            format++;
        }
        if (*format == '!') {
            effect |= 0x8000;
            format++;
            if (copts.unknown1A) {
                effect |= 0x20;
            }
        }
        if (*format == '$') {
            effect |= 0x2000;
            format++;
        }
        if (*format == '2') {
            format++;
            pair = TRUE;
            if (*format == '+') {
                format++;
            }
            if (*format == '\'') {
                format++;
                quad = TRUE;
            }
        }
        switch (*format) {
            case ')':
                format++;
                break;
            case 'b':
            case 'e':
            case 'v':
                operand->kind = 2;
                operand->data.imm.value = va_arg(args, SInt32);
                format++;
                break;
            case 'P':
            case 'd':
            case 's':
            case 't':
                operand->kind = 0;
                operand->arg = 0;
                operand->data.reg.reg = va_arg(args, SInt32);
                operand->effect = effect;
                format++;
                break;
            case '9':
                operand->kind = 0;
                operand->arg = 3;
                operand->data.reg.reg = va_arg(args, SInt32);
                operand->effect = effect;
                format++;
                break;
            case 'D':
            case 'G':
            case 'N':
            case 'S':
            case 'T':
                operand->kind = 0;
                operand->arg = pair ? (UInt8)5 : (UInt8)1;
                if (quad) {
                    number = va_arg(args, SInt32);
                    if (number & 8) {
                        effect |= 0x800;
                    }
                    if (number & 4) {
                        effect |= 0x400;
                    }
                    if (number & 2) {
                        effect |= 0x200;
                    }
                    if (number & 1) {
                        effect |= 0x100;
                    }
                    if (number < 15) {
                        pcode->flags &= 0xffffbfef;
                    }
                }
                operand->effect = effect;
                operand->data.reg.reg = va_arg(args, SInt32);
                format++;
                break;
            case 'J':
            case 'L':
                operand->kind = 0;
                operand->arg = 6;
                operand->effect = effect;
                operand->data.reg.reg = va_arg(args, SInt32);
                format++;
                break;
            case 'j':
                operand->kind = 3;
                CInt64_SetLong(&operand->data.cint, va_arg(args, SInt32));
                format++;
                break;
            case 'i':
                operand->kind = 3;
                CInt64_SetLong(&operand->data.cint, va_arg(args, SInt32));
                format++;
                break;
            case '&':
            case '<':
            case '>':
            case '@':
            case 'M':
            case 'X':
                operand->kind = 2;
                operand->data.imm.value = va_arg(args, SInt32);
                if (*format == '&' && operand->data.imm.value < 15) {
                    pcode->flags &= 0xffffbfef;
                }
                format++;
                break;
            case 'h':
                operand->kind = 0;
                operand->arg = 2;
                operand->data.reg.reg = 0;
                operand->effect = effect;
                format++;
                break;
            case 'H':
                operand->kind = 0;
                operand->arg = 2;
                operand->data.reg.reg = 1;
                operand->effect = effect;
                format++;
                break;
            case 'Z':
                operand->kind = 0;
                operand->arg = 2;
                operand->data.reg.reg = 2;
                operand->effect = effect;
                format++;
                break;
            case 'A':
                operand->kind = 0;
                operand->arg = 2;
                operand->data.reg.reg = 3;
                operand->effect = effect;
                format++;
                break;
            case 'R':
                operand->kind = 0;
                operand->arg = 2;
                operand->data.reg.reg = 4;
                operand->effect = effect;
                format++;
                break;
            case 'Y':
                operand->kind = 0;
                operand->arg = 2;
                operand->data.reg.reg = 6;
                operand->effect = effect;
                format++;
                break;
            case '1':
                operand->kind = 0;
                operand->arg = 2;
                operand->data.reg.reg = 3;
                operand->effect = effect;
                format++;
                break;
            case 'o':
                object = va_arg(args, Object*);
                if (object) {
                    if (object == (Object*)-1) {
                        operand->kind = 6;
                        operand->data.label.label = va_arg(args, PCodeLabel*);
                    } else {
                        operand->kind = 4;
                        operand->data.mem.obj = object;
                        operand->data.mem.offset = va_arg(args, SInt32);
                        arg = operand;
                        CError_ASSERT(357, format[1] == '(');
                        format += 2;
                        effect = 1;
                        if (*format == '=') {
                            effect = 2;
                            format++;
                        } else if (*format == '+') {
                            effect = 3;
                            format++;
                        }
                        CError_ASSERT(367, *format == 's' || *format == 't');
                        operand++;
                        operand->kind = 0;
                        operand->arg = 0;
                        operand->data.reg.reg = va_arg(args, SInt32);
                        operand->effect = effect;
                        format++;
                        if (pcode->flags & 0x60006) {
                            if (object->datatype == DLOCAL &&
                                operand->data.reg.reg != stack_pointer_register) {
                                pcode->alias = make_alias(object, 0, 0);
                            } else if (object->datatype == DLOCAL) {
                                pcode->alias = make_alias(
                                    object,
                                    arg->data.mem.offset - arg->data.mem.obj->u.unk30,
                                    nbytes_loaded_or_stored_by(pcode->op)
                                );
                            } else if (object->datatype == DDATA) {
                                pcode->alias = make_alias(object, 0, 0);
                            } else {
                                CError_FATAL(386);
                            }
                            if (is_volatile_object(object)) {
                                pcode->flags |= 0x80;
                            }
                            if (is_const_object(object)) {
                                pcode->flags |= 0x40;
                            }
                        }
                    }
                } else {
                    operand->kind = 2;
                    operand->data.imm.value = va_arg(args, SInt32);
                    if (pcode->flags & 0x60006) {
                        pcode->flags |= 0x20;
                    }
                }
                format++;
                break;
            case 'k':
                operand->kind = 2;
                operand->data.imm.value = va_arg(args, SInt32);
                format++;
                break;
            case 'a':
            case 'q':
                CError_FATAL(413);
            case 'l':
            case 'p':
                operand->kind = 6;
                operand->data.label.label = va_arg(args, PCodeLabel*);
                format++;
                break;
            case 'Q':
                operand->kind = 0;
                operand->arg = 2;
                operand->data.reg.reg = 2;
                operand->effect = effect;
                format++;
                break;
            case 'K':
                switch (pcode->op) {
                    case 0x68:
                    case 0x69:
                    case 0x6A:
                    case 0x6B:
                    case 0x1CD:
                    case 0x1CE:
                    case 0x1CF:
                    case 0x1D0:
                        operand->kind = 0;
                        if ((UInt16)pcode->args[1].data.reg.reg == 0x14) {
                            operand->arg = 6;
                            operand->data.reg.reg = 4;
                            operand->effect = effect;
                        } else if ((UInt16)pcode->args[1].data.reg.reg == 0x15) {
                            operand->arg = 6;
                            operand->data.reg.reg = 1;
                            operand->effect = effect;
                        } else if ((UInt16)pcode->args[1].data.reg.reg == 0x16) {
                            operand->arg = 6;
                            operand->data.reg.reg = 2;
                            operand->effect = effect;
                        } else {
                            operand->kind = 7;
                        }
                        break;
                    case 0x26C:
                    case 0x273:
                    case 0x27C:
                    case 0x296:
                    case 0x29D:
                    case 0x2B3:
                    case 0x2BA:
                    case 0x2C2:
                    case 0x2C9:
                    case 0x2D4:
                    case 0x2D8:
                    case 0x2DB:
                    case 0x2E2:
                        operand->kind = 0;
                        operand->arg = 4;
                        operand->data.reg.reg = 0x16;
                        operand->effect = effect;
                        break;
                    case 0x26B:
                    case 0x272:
                    case 0x295:
                    case 0x29C:
                    case 0x2A3:
                    case 0x2B2:
                    case 0x2B9:
                    case 0x2C1:
                    case 0x2C8:
                    case 0x2DA:
                    case 0x2E1:
                        operand->kind = 0;
                        operand->arg = 4;
                        operand->data.reg.reg = 0x15;
                        operand->effect = effect;
                        break;
                    case 0x2D1:
                    case 0x2D3:
                    case 0x2D5:
                        operand->kind = 0;
                        operand->arg = 4;
                        operand->data.reg.reg = 0x14;
                        operand->effect = effect;
                        break;
                    default:
                        CError_FATAL(517);
                }
                format++;
                break;
            case 'I':
                operand->kind = 3;
                operand->data.cint = va_arg(args, CInt64);
                format++;
                break;
            case 'B':
            case 'C':
            case 'c':
                operand->kind = 2;
                operand->data.imm.value = va_arg(args, SInt32);
                format++;
                break;
            case 'z':
            default:
                CError_FATAL(551);
                break;
        }
        operand++;
    }
    return pcode;
}

short get_nop_opcode(void) {
    return PC_NOP;
}

void fn_0049BA60(void) {
    UInt32 regs[4];
    SInt8 reg;
    if (copts.optimizesize || !copts.unknown3A) {
        return;
    }
    if ((SInt32)pclastblock->loopWeight < 8) {
        return;
    }
    reg = copts.unknown39;
    if (reg > 2) {
        CInt64_SetLong((CInt64*)regs, reg);
        emitpcode(0x4AD, regs[0], regs[1], regs[2], regs[3]);
        pclastblock->lastPCode->flags |= 0x100;
    }
}

PCode* makecopyinstruction(PCodeArg* src, PCodeArg* dst) {
    if (src->kind == PCOp_REGISTER) {
        if (src->arg == 0) {
            short op = src->effect & 0x2000 ? move_opcode_637350 : move_opcode;
            return makepcode(op, dst->data.reg.reg, src->data.reg.reg);
        }
        if (src->arg == 1) {
            short op = src->effect & 0x8000 ? 0x4A4 : 0x4A3;
            return makepcode(op, dst->data.reg.reg, src->data.reg.reg);
        }
        if (src->arg == 5) {
            return makepcode(0x2AF, 15, dst->data.reg.reg, src->data.reg.reg);
        }
        if (src->arg == 3) {
            return makepcode(0x281, dst->data.reg.reg, src->data.reg.reg, 0);
        }
    }
    CError_FATAL(979);
    return 0;
}

int is_location_independent(PCode* pcode) {
    UInt16 opcode;
    SInt32 reg;
    if (pcode->flags & 0x100) {
        return 0;
    }
    if (pcode->unk38) {
        return 0;
    }
    if (pcode->argCount == 3 &&
        (UInt8)(pcode->args[2].kind - 2) <= 1 &&
        pcode->args[1].kind == PCOp_REGISTER &&
        pcode->args[1].arg == 0) {
        reg = pcode->args[1].data.reg.reg;
        if (reg == 0) {
            return 1;
        }
    }
    opcode = pcode->op;
    switch (opcode) {
        case 0x44E:
            if (pcode->args[2].kind == PCOp_REGISTER &&
                pcode->args[1].arg == 0 &&
                ((UInt16)pcode->args[2].data.reg.reg == 28 ||
                 (UInt16)pcode->args[2].data.reg.reg == (UInt16)stack_pointer_register)) {
                return 1;
            }
            return 0;
        case 0x90:
        case 0x450:
            return 1;
    }
    return 0;
}

int can_reuse_stored_value(PCode* load, PCode* store) {
    int size;
    size = nbytes_loaded_or_stored_by(load->op);
    if (size > 16) {
        return 0;
    }
    if (copts.unknown41) {
        if ((load->op == 0x1D2 && store->op == 0x1D1) ||
            (load->op == 0x489 && store->op == 0x467)) {
            return 1;
        }
    }
    if (size > 8) {
        return 0;
    }
    if ((load->op == 0xC4 && store->op == 0x93) || (load->op == 0x479 && store->op == 0x454)) {
        return 1;
    }
    if (size > 4) {
        return 0;
    }
    if ((load->op == 0xCD && store->op == 0xA1) || (load->op == 0x483 && store->op == 0x460)) {
        return 1;
    }
    if ((load->op == 0xCB && store->op == 0x9D) || (load->op == 0x481 && store->op == 0x45F)) {
        return 1;
    }
    if ((load->op == 0xCB && store->op == 0x94) || (load->op == 0x481 && store->op == 0x45B)) {
        return 5;
    }
    if ((load->op == 0xCA && store->op == 0x94) || (load->op == 0x480 && store->op == 0x45B)) {
        return 1;
    }
    if ((load->op == 0xC3 && store->op == 0x91) || (load->op == 0x47F && store->op == 0x45A)) {
        return 1;
    }
    if ((load->op == 0xCB && store->op == 0x91) || (load->op == 0x481 && store->op == 0x45A)) {
        return 5;
    }
    if ((load->op == 0xCA && store->op == 0x91) || (load->op == 0x480 && store->op == 0x45A)) {
        return 3;
    }
    return 0;
}

PCode* makecopyforload(int kind, PCodeArg* src, PCodeArg* dst) {
    switch (kind) {
        case 1:
            return makecopyinstruction(src, dst);
        case 2:
            return makepcode(0x48C, dst->data.reg.reg, src->data.reg.reg, 8);
        case 3:
            return makepcode(0x48B, dst->data.reg.reg, src->data.reg.reg, 8);
        case 4:
            return makepcode(0x48C, dst->data.reg.reg, src->data.reg.reg, 16);
        case 5:
            return makepcode(0x48B, dst->data.reg.reg, src->data.reg.reg, 16);
        case 6:
            return makepcode(0x48C, dst->data.reg.reg, src->data.reg.reg, 32);
        case 7:
            return makepcode(0x48B, dst->data.reg.reg, src->data.reg.reg, 32);
        default:
            CError_FATAL(1183);
            return 0;
    }
}

#pragma dont_inline on
int nbytes_loaded_or_stored_by(UInt16 opcode) {
    if (!(opcode_info[opcode].flags & 6)) {
        CError_FATAL(1384);
        return 0;
    }
    switch (opcode) {
        case 0x91:
        case 0x92:
        case 0xC3:
        case 0x453:
        case 0x45A:
        case 0x47F:
            return 1;
        case 0x94:
        case 0x95:
        case 0xCA:
        case 0x45B:
        case 0x45C:
        case 0x480:
            return 2;
        case 0x9B:
        case 0x9D:
        case 0x9E:
        case 0x9F:
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xC1:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
        case 0xD0:
        case 0xD1:
        case 0x45D:
        case 0x45F:
        case 0x460:
        case 0x461:
        case 0x462:
        case 0x463:
        case 0x464:
        case 0x465:
        case 0x466:
        case 0x477:
        case 0x481:
        case 0x482:
        case 0x483:
        case 0x484:
        case 0x485:
        case 0x486:
        case 0x487:
        case 0x488:
            return 4;
        case 0x93:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0x99:
        case 0x9A:
        case 0x9C:
        case 0xC2:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
        case 0xC8:
        case 0xC9:
        case 0x454:
        case 0x455:
        case 0x456:
        case 0x457:
        case 0x458:
        case 0x459:
        case 0x45E:
        case 0x478:
        case 0x479:
        case 0x47A:
        case 0x47B:
        case 0x47C:
        case 0x47D:
        case 0x47E:
            return 8;
        case 0x10B:
        case 0x10D:
            return 4;
        case 0x10A:
        case 0x10C:
            return 8;
        case 0x164:
        case 0x173:
            return 8;
        case 0x1D1:
        case 0x1D2:
        case 0x263:
        case 0x268:
        case 0x284:
        case 0x285:
        case 0x286:
        case 0x287:
        case 0x289:
        case 0x28A:
        case 0x28B:
        case 0x28C:
        case 0x467:
        case 0x468:
        case 0x489:
        case 0x48A:
            return 16;
        default:
            CError_FATAL(1380);
            return 0;
    }
}

#pragma dont_inline reset

int fn_0049B1D0(PCode* pcode) {
    switch (pcode->op) {
        case 0x91:
        case 0x45A:
            return 3;
        case 0x92:
        case 0x453:
            return 2;
        case 0x94:
        case 0x45B:
            return 5;
        case 0x95:
        case 0x45C:
            return 4;
        case 0x9D:
        case 0x45F: {
            UInt8 result;
            if (register_size == 4) {
                result = 1;
            } else {
                result = 7;
            }
            return result;
        }
        case 0xA0:
        case 0x466: {
            UInt8 result;
            if (register_size == 4) {
                result = 1;
            } else {
                result = 6;
            }
            return result;
        }
        case 0x93:
        case 0x99:
        case 0x9A:
            return 1;
        default:
            return 0;
    }
}

PCodeArg* get_label_operand(PCode* pcode) {
    int i;
    PCodeArg* arg = pcode->args;
    for (i = 0; i < pcode->argCount; i++, arg++) {
        if (arg->kind == PCOp_LABEL) {
            return arg;
        }
    }
    CError_FATAL(1509);
    return 0;
}
