#define CERROR_FILE "BE_binary.c"
#include "compiler/BE_binary.h"
#include "compiler/PCode.h"
#include "compiler/objects.h"
#include "compiler/ObjGen.h"
#include "compiler/Registers.h"
#include "compiler/MIPSError.h"
#include "compiler/CInt64.h"
#include "compiler/PCodeInfo.h"
#include "compiler/CError.h"
#include "compiler/CompilerTools.h"
#include <string.h>

void fn_004C2B60(PCode* pcode) {
    if (pcode->flags & fPCodeIsBranch) {
        if (pcode->op == PC_B) {
            PCodeLabel* label = pcode->args[0].data.label.label;
            if (label && label->block) {
                SInt32 distance = label->block->unk24 / 4 - binary_word_index - 1;
                if (distance < -32768 || distance > 32767) {
                    pcode->op = PC_J;
                    pcode->unk38 = 4;
                }
            }
        }
        fn_004C2C40(pcode, 2);
    } else if (pcode->op == PC_ADDIU && pcode->args[2].kind == PCOp_LABEL) {
        fn_004C2C40(pcode, 2);
    }
    binary_word_index++;
}

void fn_004C2BE0(void) {
    PCodeBlock* block;
    PCode* pcode;
    PCode* last;
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        last = 0;
        for (pcode = block->firstPCode; pcode; pcode = pcode->nextPCode) {
            if (pcode->flags & fPCodeInDelaySlot) {
                last = pcode;
            } else if (pcode->op < 0x40A) {
                fn_004C2C40(pcode, 1);
            }
        }
        if (last) {
            fn_004C2C40(last, 1);
        }
    }
}

void fn_004C2800(Object* section) {
    PCode* pcode = 0;
    SInt32 raw = 0;
    PCodeBlock* block;
    PCode* delayed;
    CInt64 value;

    binary_alignment = 0;
    binary_word_index = 0;
    binary_byte_offset = binary_word_index;
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        block->unk24 = binary_byte_offset;
        delayed = 0;
        pcode = block->firstPCode;
        while (pcode) {
            if (pcode->flags & fPCodeInDelaySlot) {
                delayed = pcode;
                pcode = pcode->nextPCode;
                continue;
            }
            if (pcode->flags & 0x100000) {
                PCode* dead = pcode;
                switch (pcode->op) {
                    case 0x4A9:
                    case 0x4AB:
                    case 0x4AD:
                        raw = 0;
                        goto make_data;
                    case 0x4A8:
                        raw = *(UInt8*)&pcode->args[1].data.cint.lo;
                        raw |= (raw << 24) | (raw << 16) | (raw << 8);
                        goto make_data;
                    case 0x4AA: {
                        SInt32 half = *(UInt16*)&pcode->args[1].data.cint.lo;
                        raw = half | (half << 16);
                        goto make_data;
                    }
                    case 0x4AC:
                        raw = pcode->args[1].data.cint.lo;
make_data:
                        CInt64_SetLong(&value, raw);
                        if (pcode->args[0].data.cint.lo) {
                            SInt32 alignment = 1 << pcode->args[0].data.cint.lo;
                            if (alignment > binary_alignment) {
                                binary_alignment = alignment;
                            }
                            while (binary_byte_offset & (alignment - 1)) {
                                PCode* nop;
                                if (pcode == block->firstPCode) {
                                    block->unk24 += 4;
                                }
                                nop = makepcode(0x2ED, value);
                                insertpcodebefore(pcode, nop);
                                nop->unk1C = binary_byte_offset;
                                if (nop->op < 0x40A) {
                                    binary_word_index++;
                                    binary_byte_offset += 4;
                                } else if (nop->op < 0x4AF) {
                                    MIPSError_Error(15);
                                }
                            }
                        }
                        pcode = pcode->nextPCode;
                        deletepcode(dead);
                        continue;
                    default:
                        CError_FATAL(699);
                }
            }
            pcode->unk1C = binary_byte_offset;
            if (pcode->op < 0x40A) {
                binary_word_index++;
                binary_byte_offset += 4;
            } else if (pcode->op < 0x4AF) {
                MIPSError_Error(15);
            }
            pcode = pcode->nextPCode;
        }
        if (delayed) {
            delayed->unk1C = binary_byte_offset;
            if (delayed->op < 0x40A) {
                binary_word_index++;
                binary_byte_offset += 4;
            } else if (delayed->op < 0x4AF) {
                MIPSError_Error(15);
            }
        }
    }
    {
        ObjectList* link = binary_fixup_list;
        while (link) {
            Object* data = link->object;
            PCodeArg** entries = (PCodeArg**)data->u.varptr;
            int offset;
            ObjGenRelocation* first;
            int entryCount = data->u.unk2C / 4;
            first = 0;
            offset = 0;
            while (entryCount) {
                ObjGenRelocation* node = lalloc(sizeof(ObjGenRelocation));
                PCodeLabel* code = (*entries)->data.label.label;
                PCodeBlock* target = code->block;
                node->addend = target->unk24;
                node->object = section;
                entryCount--;
                node->offset = offset;
                offset += 4;
                node->next = first;
                first = node;
                entries++;
            }
            {
                void* bytes = lalloc(data->u.unk2C);
                memset(bytes, 0, data->u.unk2C);
                fn_0046DD50(data, bytes, first, data->u.unk2C);
            }
            link = link->next;
        }
    }
    {
        PCodeBlock* block;
        PCode* delayed;
        PCode* pcode;
        binary_fixup_list = 0;
        binary_word_index = 0;
        for (block = pcbasicblocks; block; block = block->nextBlock) {
            delayed = 0;
            for (pcode = block->firstPCode; pcode; pcode = pcode->nextPCode) {
                if (pcode->flags & fPCodeInDelaySlot) {
                    delayed = pcode;
                } else {
                    fn_004C2B60(pcode);
                }
            }
            if (delayed) {
                fn_004C2B60(delayed);
            }
        }
    }
    *(SInt32*)binary_word_count_destination = binary_word_index;
}

void fn_004C2C40(PCode* pcode, int pass) {
    OpcodeInfo* info;
    SInt32 value;
    int index;
    const char* format;
    Boolean quoted;
    Boolean paired;

    index = 0;
    info = &opcode_info[pcode->op];
    format = info->format;
    pcode->unk30 = info->encoding;
    for (; *format; format++) {
        if (*format == ';') {
            break;
        }
        if (*format == '=' || *format == '+') {
            format++;
        }
        if (*format == '!') {
            format++;
        }
        if (*format == '$') {
            format++;
        }
        paired = FALSE;
        quoted = FALSE;
        if (*format == '2') {
            format++;
            if (*format == '+') {
                format++;
            }
            paired = TRUE;
        }
        if (*format == '\'') {
            format++;
            quoted = TRUE;
        }
        switch (*format) {
            case '&':
                pcode->unk30 |= pcode->args[index].data.imm.value << 21;
                index++;
                break;
            case '<': {
                int reg;
                reg = pcode->args[index].data.imm.value;
                if (reg > 31) {
                    MIPSError_Warning(31, reg);
                }
                pcode->unk30 |= (reg & 31) << 6;
                index++;
                break;
            }
            case '>': {
                int reg;
                reg = pcode->args[index].data.imm.value;
                if (reg > 31) {
                    MIPSError_Warning(31, reg);
                }
                pcode->unk30 |= (reg & 31) << 6;
                index++;
                break;
            }
            case '@': {
                int reg;
                if (pcode->op == 0x3EF) {
                    reg = pcode->args[index].data.imm.value + (((UInt32)pcode->unk30 >> 6) & 31);
                } else {
                    reg = pcode->args[index].data.imm.value;
                }
                pcode->unk30 |= ((reg - 1) & 31) << 11;
                index++;
                break;
            }
            case 'a':
                if (pass == 2 && pcode->args[index].kind == PCOp_LABEL) {
                    value = pcode->args[index].data.label.label->block->unk24 / 4;
                    if (pcode->flags & 0x4000000) {
                        value++;
                    }
                    pcode->unk30 |= value & 0x3FFFFFF;
                }
                break;
            case 'b':
                if (paired && pass == 2 && pcode->args[index].kind == PCOp_LABEL) {
                    value = pcode->args[index].data.label.label->block->unk24 / 8;
                    pcode->unk30 |= (value & 0x7FFF) << 6;
                }
                index++;
                break;
            case 'q':
                value = binary_word_index * 4;
                if (!(value & 0x8000) && (value & 0xFFFF0000)) {
                    MIPSError_Error(14);
                }
                if (value & 0xFFFF0000) {
                    MIPSError_Error(14);
                }
                pcode->unk30 |= value & 0xFFFF;
                break;
            case 'c':
                value = pcode->args[index].data.imm.value << 6;
                pcode->unk30 |= value & 0x7FFFF;
                break;
            case 'j':
                if (pcode->args[index].kind == PCOp_LABEL) {
                    if (pass == 2) {
                        UInt32 high;
                        value = pcode->args[index].data.label.label->block->unk24;
                        pcode->unk30 |= value & 0xFFFF;
                        high = value >> 16;
                        if (value & 0x8000) {
                            high++;
                        }
                        pcode->unk3C->unk30 |= high & 0xFFFF;
                    }
                } else {
                    pcode->unk30 |= (UInt16)pcode->args[index].data.cint.lo;
                }
                index++;
                break;
            case 'i': {
                SInt32 immediate = pcode->args[index].data.cint.lo;
                if (paired) {
                    pcode->unk30 |= (immediate >> 3) << 6;
                } else {
                    pcode->unk30 |= immediate & 0xFFFF;
                }
                index++;
                break;
            }
            case 'k':
                pcode->unk30 |= (pcode->args[index].data.cint.lo & 31) << 16;
                index++;
                break;
            case 'o':
                if (pcode->args[index].kind == 4) {
                    value = pcode->args[index].data.mem.offset;
                } else if (pcode->args[index].kind == 2) {
                    value = pcode->args[index].data.imm.value;
                } else if (pcode->args[index].kind == 6) {
                    CError_FATAL(250);
                } else {
                    CError_FATAL(252);
                }
                pcode->unk30 |= value & 0xFFFF;
                index++;
                break;
            case 'p':
                if (pass == 2) {
                    if (pcode->unk38 != 10) {
                        if (pcode->args[index].kind == 2) {
                            value = pcode->args[index].data.cint.lo;
                        } else {
                            value = pcode->args[index].data.label.label->block->unk24 / 4 -
                                binary_word_index - 1;
                            if (pcode->flags & 0x4000000) {
                                value++;
                            }
                            if ((SInt16)value != value) {
                                MIPSError_Error(14);
                            }
                        }
                    }
                    pcode->unk30 |= value & 0xFFFF;
                }
                index++;
                break;
            case 'd':
                pcode->unk30 |= pcode->args[index].data.reg.reg << 11;
                index++;
                break;
            case 's':
                pcode->unk30 |= pcode->args[index].data.reg.reg << 21;
                index++;
                break;
            case 'P':
                pcode->unk30 |= pcode->args[index].data.reg.reg * 2;
                index++;
                break;
            case 'u':
                value = pcode->args[index].data.cint.lo >> 16;
                pcode->unk30 |= value & 0xFFFF;
                index++;
                break;
            case 'v':
                pcode->unk30 |= pcode->args[index].data.imm.value & 15;
                index++;
                break;
            case 'C':
                value = pcode->args[index].data.imm.value;
                pcode->unk30 |= value;
                break;
            case 'B':
                value = pcode->args[index].data.imm.value;
                pcode->unk30 |= (value & 0xFFFFF) << 6;
                break;
            case 'x':
                if (paired) {
                    pcode->unk30 |= pcode->args[index].data.reg.reg << 6;
                    index++;
                    break;
                }
                CError_FATAL(341);
            case 'z':
                CError_FATAL(344);
                break;
            case 'D': {
                int reg;
                reg = pcode->args[index].data.reg.reg;
                if (pcode->args[index].effect & 0x4000) {
                    reg++;
                }
                pcode->unk30 |= reg << 6;
                index++;
                break;
            }
            case 'J': {
                int reg;
                reg = pcode->args[index].data.reg.reg;
                if (pcode->args[index].effect & 0x4000) {
                    reg++;
                }
                pcode->unk30 |= reg & 0xFF;
                index++;
                break;
            }
            case 'L': {
                int reg;
                reg = pcode->args[index].data.reg.reg;
                if (pcode->args[index].effect & 0x4000) {
                    reg++;
                }
                pcode->unk30 |= (reg & 0xFF) << 8;
                index++;
                break;
            }
            case 'M': {
                int reg;
                reg = pcode->args[index].data.reg.reg;
                if (pcode->args[index].effect & 0x4000) {
                    reg++;
                }
                pcode->unk30 |= (reg & 0xFF) << 16;
                index++;
                break;
            }
            case 'R': {
                int reg;
                if (!paired) {
                    reg = pcode->args[index].data.reg.reg;
                    if (pcode->args[index].effect & 0x4000) {
                        reg++;
                    }
                    pcode->unk30 |= reg << 21;
                }
                index++;
                break;
            }
            case 'S': {
                int reg;
                if (paired && quoted) {
                    pcode->unk30 &= ~0x600000;
                    if (pcode->args[index].effect & 0x400) {
                        pcode->unk30 |= 0x200000;
                    } else if (pcode->args[index].effect & 0x200) {
                        pcode->unk30 |= 0x400000;
                    } else if (pcode->args[index].effect & 0x100) {
                        pcode->unk30 |= 0x600000;
                    }
                }
                reg = pcode->args[index].data.reg.reg;
                if (pcode->args[index].effect & 0x4000) {
                    reg++;
                }
                pcode->unk30 |= reg << 11;
                index++;
                break;
            }
            case 'N':
            case 'T': {
                int reg;
                if (paired && quoted) {
                    pcode->unk30 &= ~0x1800000;
                    if (pcode->args[index].effect & 0x400) {
                        pcode->unk30 |= 0x800000;
                    } else if (pcode->args[index].effect & 0x200) {
                        pcode->unk30 |= 0x1000000;
                    } else if (pcode->args[index].effect & 0x100) {
                        pcode->unk30 |= 0x1800000;
                    }
                }
                reg = pcode->args[index].data.reg.reg;
                if (pcode->args[index].effect & 0x4000) {
                    reg++;
                }
                pcode->unk30 |= reg << 16;
                index++;
                break;
            }
            case 'X':
                pcode->unk30 |= (pcode->args[index].data.imm.value & 31) << 16;
                index++;
                break;
            case 't': {
                int reg;
                pcode->unk30 |= pcode->args[index].data.reg.reg << 16;
                index++;
                break;
            }
            case 'W': {
                int reg;
                reg = pcode->args[index].data.reg.reg;
                if (pcode->args[index].effect & 0x4000) {
                    reg++;
                }
                pcode->unk30 |= reg << 16;
                index++;
                break;
            }
            case 'V': {
                int reg;
                reg = pcode->args[index].data.reg.reg;
                if (pcode->args[index].effect & 0x4000) {
                    reg++;
                }
                pcode->unk30 |= reg << 11;
                index++;
                break;
            }
            case 'E': {
                int reg;
                reg = pcode->args[index].data.reg.reg;
                if (pcode->args[index].effect & 0x4000) {
                    reg++;
                }
                pcode->unk30 |= reg << 16;
                index++;
                break;
            }
            case 'G': {
                int reg;
                reg = pcode->args[index].data.reg.reg;
                if (pcode->args[index].effect & 0x4000) {
                    reg++;
                }
                pcode->unk30 |= reg << 11;
                index++;
                break;
            }
            case 'A':
            case 'U':
                index++;
                break;
            case 'I':
                if (!paired) {
                    pcode->unk30 |= pcode->args[index].data.cint.lo;
                }
                index++;
                break;
            case 'Q':
                index++;
                break;
            case 'F':
                CError_FATAL(509);
                break;
            case 'f':
                CError_FATAL(517);
                break;
            case 'l':
                CError_FATAL(521);
                break;
            default:
                CError_FATAL(537);
                break;
            case '(':
            case ')':
            case ',':
            case '-':
            case '9':
            case '|':
            case '}':
                break;
        }
    }
    if (pcode->op >= 0x2EF && pcode->op <= 0x32E) {
        pcode->unk30 |= (((UInt32)pcode->unk30 & 0xF800) >> 11) << 21;
    } else if (pcode->op >= 0x32F && pcode->op <= 0x337) {
        pcode->unk30 |= (((UInt32)pcode->unk30 & 0x1F0000) >> 16) << 21;
    } else if (pcode->op >= 0x338 && pcode->op <= 0x353) {
        pcode->unk30 |= (((UInt32)pcode->unk30 & 0xF800) >> 11) << 16;
    } else if (pcode->op >= 0x354 && pcode->op <= 0x37D) {
        pcode->unk30 |= (((UInt32)pcode->unk30 & 0x7C0) >> 6) << 11;
    }
}
