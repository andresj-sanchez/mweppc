#include "compiler/Operands.h"
#include "compiler/BE_macro.h"
#include "compiler/BE_binary.h"
#include "compiler/ObjGen.h"
#include "compiler/VReg.h"
#include "compiler/objects.h"
#include "compiler/PCode.h"
#include "compiler/Alias.h"
#include "compiler/CodeGen.h"
#include "compiler/PCodeUtilities.h"
#include "compiler/RegisterInfo.h"
#include "compiler/Registers.h"

#include "compiler/CInt64.h"
#include "compiler/CParser.h"

CInt64 be_macro_split_immediate = {0, 0x7FF8};

void fn_00495D60(void* object) {
    PCodeBlock* block;
    PCode* instr;

    be_macro_active_object_608E00 = object;
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        be_macro_current_block_636058 = block;
        for (instr = block->firstPCode; instr; instr = instr->nextPCode) {
            fn_00495DA0(instr);
        }
    }
}

void fn_00495DA0(PCode* pc) {
    PCode* instr;
    PCode* next;
    ObjGenRegisterLocation location;
    SInt32 value;
    UInt16 opcode;
    UInt32 nextOpcode;

    switch (pc->op) {
        case 0x451:
            instr = makepcode(
                0x450,
                used_virtual_registers[0]++,
                pc->args[1].data.cint.hi,
                pc->args[1].data.cint.unk4,
                pc->args[1].data.cint.unk8,
                pc->args[1].data.cint.lo
            );
            pc->op = 0xAD;
            pc->args[1] = pc->args[0];
            pc->args[0] = instr->args[0];
            pc->args[0].effect = 1;
            insertpcodebefore(pc, instr);
            goto done;
        case 0x45F:
        case 0x481:
            if (pc->args[1].data.mem.offset % 4 != 0) {
                if (((volatile PCode*)pc)->op == 0x45F) {
                    if (copts.littleendian) {
                        opcode = 0x465;
                        nextOpcode = 0x464;
                    } else {
                        opcode = 0x464;
                        nextOpcode = 0x465;
                    }
                } else {
                    if (copts.littleendian) {
                        opcode = 0x488;
                        nextOpcode = 0x487;
                    } else {
                        opcode = 0x487;
                        nextOpcode = 0x488;
                    }
                }
                pc->op = opcode;
                instr = make_instruction(nextOpcode, 3);
                instr->flags = pc->flags;
                instr->args[0] = pc->args[0];
                instr->args[0].effect |= 1;
                instr->args[1] = pc->args[1];
                instr->args[2] = pc->args[2];
                instr->unk38 = pc->unk38;
                instr->alias = pc->alias;
                insertpcodeafter(pc, instr);
                instr->args[1].data.mem.offset += 3;
            }
            break;
        case 0x454:
        case 0x479:
            if (pc->args[1].data.mem.offset % 8 != 0) {
                if (pc->op == 0x454) {
                    if (copts.littleendian) {
                        opcode = 0x459;
                        nextOpcode = 0x458;
                    } else {
                        opcode = 0x458;
                        nextOpcode = 0x459;
                    }
                } else {
                    if (copts.littleendian) {
                        opcode = 0x47E;
                        nextOpcode = 0x47D;
                    } else {
                        opcode = 0x47D;
                        nextOpcode = 0x47E;
                    }
                }
                pc->op = opcode;
                instr = make_instruction(nextOpcode, 3);
                instr->flags = pc->flags;
                instr->args[0] = pc->args[0];
                instr->args[0].effect |= 1;
                instr->args[1] = pc->args[1];
                instr->args[2] = pc->args[2];
                instr->unk38 = pc->unk38;
                instr->alias = pc->alias;
                insertpcodeafter(pc, instr);
                instr->args[1].data.mem.offset += 7;
            }
            break;
        case 0x437:
        case 0x438:
        case 0x453:
        case 0x455:
        case 0x456:
        case 0x457:
        case 0x458:
        case 0x459:
        case 0x45A:
        case 0x45B:
        case 0x45C:
        case 0x45D:
        case 0x45E:
        case 0x460:
        case 0x461:
        case 0x462:
        case 0x463:
        case 0x464:
        case 0x465:
        case 0x466:
        case 0x467:
        case 0x468:
        case 0x477:
        case 0x478:
        case 0x47A:
        case 0x47B:
        case 0x47C:
        case 0x47D:
        case 0x47E:
        case 0x47F:
        case 0x480:
        case 0x482:
        case 0x483:
        case 0x484:
        case 0x485:
        case 0x486:
        case 0x487:
        case 0x488:
        case 0x489:
        case 0x48A:
            break;
        default:
            goto done;
    }

    if (pc->flags & 0x200000) {
        goto done;
    }
    fn_0046EDD0(pc->args[1].data.mem.obj, &location);
    if (location.number == 28) {
        if (location.isDirect == 0) {
            instr = makepcode(
                0x90,
                used_virtual_registers[0]++,
                ((pc->args[1].data.mem.offset & 0x8000) >> 15) +
                    ((UInt32)pc->args[1].data.mem.offset >> 16)
            );
            instr->unk38 = 5;
            instr->flags &= ~0x4000;
            instr->object = pc->args[1].data.mem.obj;
            if (pc->flags & 6) {
                instr->alias = make_alias(pc->args[1].data.mem.obj, 0, 0);
            }
            insertpcodebefore(pc, instr);
            next = makepcode(4, pc->args[0].data.reg.reg, 28, pc->args[0].data.reg.reg);
            insertpcodebefore(pc, next);
            pc->op = be_macro_expanded_opcode_6371D0[pc->op - 0x40A];
            pc->args[2] = instr->args[0];
            pc->args[2].effect = 1;
            pc->args[1].data.mem.offset &= 0xFFFF;
            pc->unk38 = 6;
            pc->object = pc->args[1].data.mem.obj;
            instr->unk3C = pc;
            pc->unk3C = instr;
        } else {
            pc->op = be_macro_expanded_opcode_6371D0[pc->op - 0x40A];
            pc->unk38 = (UInt8)(location.offset == 0 ? 7 : 8);
            pc->object = pc->args[1].data.mem.obj;
        }
    } else if (location.number != stack_pointer_register || pc->args[2].data.reg.reg == 28) {
        if (location.number == 23) {
            instr = makepcode(
                0x90,
                used_virtual_registers[0]++,
                ((pc->args[1].data.mem.offset & 0x8000) >> 15) +
                    ((UInt32)pc->args[1].data.mem.offset >> 16)
            );
            instr->unk38 = 5;
            instr->flags &= ~0x4000;
            instr->object = pc->args[1].data.mem.obj;
            if (pc->flags & 6) {
                instr->alias = make_alias(pc->args[1].data.mem.obj, 0, 0);
            }
            insertpcodebefore(pc, instr);
            next = makepcode(4, instr->args[0].data.reg.reg, 23, instr->args[0].data.reg.reg);
            insertpcodebefore(pc, next);
            pc->op = be_macro_expanded_opcode_6371D0[pc->op - 0x40A];
            pc->args[2] = instr->args[0];
            pc->args[2].effect = 1;
            pc->args[1].data.mem.offset &= 0xFFFF;
            pc->unk38 = 6;
            pc->object = pc->args[1].data.mem.obj;
            instr->unk3C = pc;
            pc->unk3C = instr;
        } else if (location.number == -1) {
            value = pc->args[1].data.mem.offset;
            if (value > 32767 || value < -32768) {
                if (pc->args[2].data.reg.reg == 0) {
                    instr = makepcode(
                        0x90,
                        used_virtual_registers[0]++,
                        ((pc->args[1].data.mem.offset & 0x8000) >> 15) +
                            ((UInt32)pc->args[1].data.mem.offset >> 16)
                    );
                    insertpcodebefore(pc, instr);
                    pc->args[1].data.mem.offset &= 0xFFFF;
                    pc->args[2] = instr->args[0];
                    pc->args[2].effect = 1;
                } else {
                    instr = makepcode(
                        0x90,
                        used_virtual_registers[0]++,
                        ((pc->args[1].data.mem.offset & 0x8000) >> 15) +
                            ((UInt32)pc->args[1].data.mem.offset >> 16)
                    );
                    insertpcodebefore(pc, instr);
                    next = makepcode(
                        4,
                        instr->args[0].data.reg.reg,
                        pc->args[2].data.reg.reg,
                        instr->args[0].data.reg.reg
                    );
                    insertpcodebefore(pc, next);
                    pc->args[1].data.mem.offset &= 0xFFFF;
                    pc->args[2] = instr->args[0];
                    pc->args[2].effect = 1;
                }
            }
            pc->op = be_macro_expanded_opcode_6371D0[pc->op - 0x40A];
        } else if (location.isDirect == 0) {
            instr = makepcode(
                0x90,
                used_virtual_registers[0]++,
                ((pc->args[1].data.mem.offset & 0x8000) >> 15) +
                    ((UInt32)pc->args[1].data.mem.offset >> 16)
            );
            instr->unk38 = 5;
            instr->flags &= ~0x4000;
            instr->object = pc->args[1].data.mem.obj;
            if (pc->flags & 6) {
                instr->alias = make_alias(pc->args[1].data.mem.obj, 0, 0);
            }
            insertpcodebefore(pc, instr);
            pc->op = be_macro_expanded_opcode_6371D0[pc->op - 0x40A];
            pc->args[2] = instr->args[0];
            pc->args[2].effect = 1;
            pc->args[1].data.mem.offset &= 0xFFFF;
            pc->unk38 = 6;
            pc->object = pc->args[1].data.mem.obj;
            instr->unk3C = pc;
            pc->unk3C = instr;
        } else {
            pc->op = be_macro_expanded_opcode_6371D0[pc->op - 0x40A];
            pc->unk38 = 1;
        }
    }
done:
    return;
}

void fn_0049A4B0(PCode* pc) {
    PCode* instr;
    CInt64 value;
    SInt32 mask;

    if (codegen_flag_63738A) {
        PCode* preamble = makepcode(0xAA, 29, 30);
        insertpcodebefore(pc, preamble);
        preamble->flags |= 0x100;
    }
    if (copts.interrupt) {
        fn_0049AE70(0, pc, be_macro_mask_buffer_608DE4);
    } else {
        fn_0049A620(0, pc);
    }
    be_macro_save_header_625300.kind = 1;
    be_macro_save_header_625300.reg = (UInt8)stack_pointer_register;
    {
        SInt32 amount;
        mask = stack_alignment_63560C - 1;
        amount = ((codegen_operand_max_6363EC + mask) & ~mask) + asm_stack_frame_offset_636E78;
        be_macro_save_header_625300.offset = amount;
        if (amount) {
            CInt64_SetLong(&value, amount);
            instr = makepcode(0x40B, 29, 29, value.hi, value.unk4, value.unk8, value.lo);
            instr->flags |= 0x100;
            insertpcodebefore(pc, instr);
            fn_00496520(&instr, 0);
        }
    }
    if (copts.interrupt) {
        instr = makepcode(0x8F);
        instr->flags |= 0x80000;
        insertpcodebefore(pc, instr);
    } else if (!made_tail_call) {
        instr = makepcode(0x3B, 31);
        instr->flags |= 0x80000;
        insertpcodebefore(pc, instr);
    }
    deletepcode(pc);
}

void fn_0049A620(UInt8 save, PCode* pc) {
    SInt32 offset;
    SInt32 reg;
    SInt32 count;
    UInt16 specialOp;
    UInt16 op;
    PCode* instr;
    UInt32 fpused;

    offset =
        asm_stack_frame_offset_636E78 - be_macro_stack_base_635A40 - be_macro_frame_header_608DDC;
    if (!save) {
        op = 0x1D1;
        specialOp = be_macro_saved_gp_reg_63732E;
    } else {
        op = 0x1D2;
        specialOp = be_macro_saved_fp_reg_637338;
        be_macro_gp_save_records_625AA8.reg[31].kind = 1;
        be_macro_gp_save_records_625AA8.reg[31].reg = 31;
        be_macro_gp_save_records_625AA8.reg[31].offset = 0;
        be_macro_gp_first_6361D4 = 0;
        be_macro_fp_first_635F4C = 0;
    }
    if (binary_word_count_destination->gprmask & 0xC0FF0000) {
        for (reg = 31; reg >= 0; reg--) {
            if (copts.unknown08 && reg == 23) {
                continue;
            }
            if (binary_word_count_destination->gprmask & (1U << reg)) {
                if (!((1U << reg) & 0xC0FF0000)) {
                    continue;
                }
                offset -= 16;
                if (reg == 31) {
                    instr = makepcode(specialOp, reg, 0, offset, 29);
                } else {
                    instr = makepcode(op, reg, 0, offset, 29);
                }
                insertpcodebefore(pc, instr);
                if (save) {
                    be_macro_gp_save_records_625AA8.reg[reg].kind = 4;
                    be_macro_gp_save_records_625AA8.reg[reg].base = (UInt8)stack_pointer_register;
                    be_macro_gp_save_records_625AA8.reg[reg].offset = offset;
                    be_macro_gp_first_6361D4 = offset;
                    if (reg == 28) {
                        be_macro_gp_last_63618C = offset;
                    }
                }
            }
        }
    }
    fpused = binary_word_count_destination->fprmask;
    if (fpused & 0xFFF00000) {
        count = 0;
        for (reg = 31; reg >= 0; reg--) {
            if ((fpused & (1U << reg)) && ((1U << reg) & 0xFFF00000)) {
                count++;
            }
        }
        while (count & 3) {
            count++;
            offset -= 4;
        }
        for (reg = 31; reg >= 0; reg--) {
            if (!(binary_word_count_destination->fprmask & (1U << reg))) {
                continue;
            }
            if (!((1U << reg) & 0xFFF00000)) {
                continue;
            }
            if (copts.unknown0C >= 2 && (copts.fp_library & 2)) {
                if (reg & 1) {
                    continue;
                }
                offset -= 8;
                instr = makepcode(save ? 0xC5 : 0x96, reg, 0, offset, 29);
                insertpcodebefore(pc, instr);
                if (save) {
                    be_macro_fp_save_records_621080.reg[reg].kind = 4;
                    be_macro_fp_save_records_621080.reg[reg].base = (UInt8)stack_pointer_register;
                    be_macro_fp_save_records_621080.reg[reg].offset = offset;
                    be_macro_fp_first_635F4C = offset;
                }
            } else {
                offset -= 4;
                instr = makepcode(save ? 0xCD : 0xA1, reg, 0, offset, 29);
                insertpcodebefore(pc, instr);
                if (save) {
                    be_macro_fp_single_save_records_621280.reg[reg].kind = 4;
                    be_macro_fp_single_save_records_621280.reg[reg].base =
                        (UInt8)stack_pointer_register;
                    be_macro_fp_single_save_records_621280.reg[reg].offset = offset;
                    be_macro_fp_first_635F4C = offset;
                }
            }
        }
    }
}

void fn_0049A8F0(PCodeBlock* last) {
    SInt32 i;
    SInt32 gp_mask_index;
    SInt32 alignment;
    SInt32 total;
    SInt32 bits;
    PCodeBlock* block;
    PCode* pc;
    CInt64 shiftValue;
    CInt64 updated;

    if (copts.interrupt) {
        asm_stack_frame_offset_636E78 = fn_0049B080(last, be_macro_mask_buffer_608DE4);
    } else {
        alignment = stack_alignment_63560C - 1;
        be_macro_stack_base_635A40 = (be_macro_stack_base_635A40 + alignment) & ~alignment;
        asm_stack_frame_offset_636E78 = be_macro_stack_base_635A40;
    }
    if (codegen_flag_63738A) {
        binary_word_count_destination->gprmask |= 0x40000000;
    }
    for (i = 0; i < starting_available_registers[0]; i++) {
        binary_word_count_destination->gprmask |= 1U << nonvolatile_registers[RegClass_GPR][i];
    }
    for (i = 0; i < be_macro_debug_register_count_636930; i++) {
        binary_word_count_destination->fprmask |= 1U << be_macro_special_registers_6253B8[i];
        if (!copts.floatsp32) {
            binary_word_count_destination->fprmask |= 1U
                << (be_macro_special_registers_6253B8[i] + 1);
        }
    }
    for (gp_mask_index = 0; gp_mask_index < 32; gp_mask_index++) {
        if ((!copts.unknown08 || gp_mask_index != 23) &&
            (binary_word_count_destination->gprmask & (1U << gp_mask_index)) &&
            ((1U << gp_mask_index) & 0xC0FF0000)) {
            be_macro_frame_padding_636438 += 16;
        }
    }
    if (copts.fp_library & 1) {
        for (i = 0; i < 32; i++) {
            if ((binary_word_count_destination->fprmask & (1U << i)) && ((1U << i) & 0xFFF00000)) {
                be_macro_frame_padding2_6364CC += 4;
            }
        }
        if (be_macro_frame_padding2_6364CC) {
            alignment = stack_alignment_63560C - 1;
            be_macro_frame_padding2_6364CC =
                (be_macro_frame_padding2_6364CC + alignment) & ~alignment;
        }
    }
    be_macro_accumulated_stack_6361CC +=
        be_macro_frame_padding2_6364CC + be_macro_frame_padding_636438;
    alignment = stack_alignment_63560C - 1;
    total = (be_macro_accumulated_stack_6361CC + alignment) & ~alignment;
    if (be_macro_accumulated_stack_6361CC != total) {
        be_macro_frame_header_608DDC = stack_alignment_63560C == 16 ? 8 : 4;
    } else {
        be_macro_frame_header_608DDC = 0;
    }
    asm_stack_frame_offset_636E78 += be_macro_adjust_offset_6356A4 +
        be_macro_accumulated_stack_6361CC + be_macro_frame_header_608DDC;
    be_macro_reserved_reg_used_608E04 = 0;

    for (block = pcbasicblocks; block; block = block->nextBlock) {
        be_macro_current_block_636058 = block;
        for (pc = block->firstPCode; pc; pc = pc->nextPCode) {
            fn_0049ACB0(pc);
        }
    }
    if (!be_macro_reserved_reg_used_608E04 &&
        ((binary_word_count_destination->gprmask & 0xC0FF0000) ||
         (binary_word_count_destination->fprmask & 0xFFF00000))) {
        be_macro_reserved_reg_used_608E04 = 1;
    }
    if (!be_macro_reserved_reg_used_608E04) {
        asm_stack_frame_offset_636E78 = 0;
        be_macro_frame_padding_636438 = 0;
        be_macro_frame_padding2_6364CC = 0;
    }
    {
        for (block = pcbasicblocks; block; block = block->nextBlock) {
            be_macro_current_block_636058 = block;
            for (pc = block->firstPCode; pc; pc = pc->nextPCode) {
                if (pc->unk38 == 0x11) {
                    CInt64_SetLong(&shiftValue, be_macro_adjust_offset_6356A4);
                    updated = CInt64_Add(*(CInt64*)&pc->args[2].data, shiftValue);
                    *(CInt64*)&pc->args[2].data = updated;
                    pc->unk38 = 0;
                }
                if (pc->op >= 0x40A) {
                    be_macro_immediate_63613C = 0;
                    fn_00496520(&pc, 0);
                }
            }
        }
    }
}

void fn_0049ACB0(PCode* pc) {
    UInt32 i;
    SInt32 mask;
    SInt32 delta;
    PCodeArg* arg;

    if (copts.unknown15) {
        mask = stack_alignment_63560C - 1;
        delta = ((codegen_operand_max_6363EC + mask) & ~mask) - codegen_operand_max_6363EC;
    } else {
        delta = 0;
    }
    if (made_tail_call && (pc->flags & 8)) {
        return;
    }

    i = 0;
    if (pc->argCount > 0) {
        do {
            arg = &pc->args[i];
            if (arg->kind == 0) {
                if (stack_pointer_register == arg->data.reg.reg) {
                    be_macro_reserved_reg_used_608E04 = 1;
                }
            } else if (arg->kind == PCOp_MEMORY &&
                       arg->data.mem.obj &&
                       arg->data.mem.obj->datatype == DLOCAL) {
                if (arg->data.mem.offset >= 0) {
                    arg->data.mem.offset += delta;
                }
                arg->data.mem.offset += asm_stack_frame_offset_636E78;
            }
            i++;
        } while (i < pc->argCount);
    }
}

void fn_0049AD60(PCode* pc) {
    SInt32 amount = pc->args[2].data.imm.value;
    UInt32 mask;
    SInt32 delta;
    SInt32 reg;
    PCode* instr;

    if (amount >= 32) {
        mask = 0xFFFFFFFF;
    } else if (amount < 32) {
        mask = (1U << amount) - 1;
    }
    if (mask <= 0xFFFF) {
        pc->op = 7;
        pc->args[2].kind = 3;
        CInt64_SetLong(&pc->args[2].data.cint, mask);
    } else {
        delta = 64 - amount;
        if ((UInt32)delta >= 32) {
            pc->op = 0x85;
            reg = (SInt16)pc->args[0].data.reg.reg;
            instr = makepcode(0x8B, reg, reg, delta - 32);
            pc->args[2].data.imm.value = delta - 32;
        } else {
            pc->op = 0x84;
            reg = (SInt16)pc->args[0].data.reg.reg;
            instr = makepcode(0x8A, reg, reg, delta);
            pc->args[2].data.imm.value = delta;
        }
        insertpcodeafter(pc, instr);
    }
}

void fn_0049AE00(PCode* pc) {
    SInt32 delta = 64 - pc->args[2].data.imm.value;
    PCode* instr;
    SInt32 reg;

    if (delta >= 32) {
        pc->op = 0x85;
        reg = (SInt16)pc->args[0].data.reg.reg;
        instr = makepcode(0x88, reg, reg, delta - 32);
        pc->args[2].data.imm.value = delta - 32;
    } else {
        pc->op = 0x84;
        reg = (SInt16)pc->args[0].data.reg.reg;
        instr = makepcode(0x87, reg, reg, delta);
        pc->args[2].data.imm.value = delta;
    }
    insertpcodeafter(pc, instr);
}

void fn_0049AE70(UInt8 save, PCode* pc, UInt32* mask) {
    SInt32 offset;
    SInt32 reg;
    SInt16 op;
    PCode* instr;

    offset = 0;
    if ((mask[2] & 1) && !save) {
        instr = makepcode(
            (SInt16)be_macro_saved_gp_reg_63732E,
            be_macro_reserved_reg_608DE0,
            0,
            be_macro_reserved_offset_608DD8,
            29
        );
        insertpcodebefore(pc, instr);
        instr = makepcode(0xB2, be_macro_reserved_reg_608DE0);
        insertpcodebefore(pc, instr);
        instr = makepcode(
            (SInt16)be_macro_saved_gp_reg_63732E,
            be_macro_reserved_reg_608DE0,
            0,
            be_macro_reserved_offset_608DD8 + register_size,
            29
        );
        insertpcodebefore(pc, instr);
        instr = makepcode(0xB3, be_macro_reserved_reg_608DE0);
        insertpcodebefore(pc, instr);
    }
    for (reg = 0; reg < 32; reg++) {
        if (mask[0] & (1U << reg)) {
            op = save ? be_macro_saved_fp_reg_637338 : be_macro_saved_gp_reg_63732E;
            instr = makepcode((SInt16)op, reg, 0, offset, 29);
            insertpcodebefore(pc, instr);
            offset += register_size;
        }
    }
    if (copts.fp_library & 1) {
        for (reg = 0; reg < 32; reg++) {
            if (mask[1] & (1U << reg)) {
                op = save ? 0xC5 : 0x96;
                instr = makepcode((SInt16)op, reg, 0, offset, 29);
                insertpcodebefore(pc, instr);
                offset += register_size;
            }
        }
    }
    if (save && (mask[2] & 1)) {
        be_macro_reserved_offset_608DD8 = offset;
        instr = makepcode(0xA8, be_macro_reserved_reg_608DE0);
        insertpcodebefore(pc, instr);
        instr = makepcode(
            (SInt16)be_macro_saved_fp_reg_637338, be_macro_reserved_reg_608DE0, 0, offset, 29
        );
        insertpcodebefore(pc, instr);
        offset += register_size;
        instr = makepcode(0xA9, be_macro_reserved_reg_608DE0);
        insertpcodebefore(pc, instr);
        instr = makepcode(
            (SInt16)be_macro_saved_fp_reg_637338, be_macro_reserved_reg_608DE0, 0, offset, 29
        );
        insertpcodebefore(pc, instr);
    }
}

//! @reconstructed
static inline void reserve_macro_register(UInt32* mask, SInt32* count) {
    SInt32 i;

    for (i = 0; i < 7; i++) {
        if (mask[0] & (1U << i)) {
            be_macro_reserved_reg_608DE0 = i;
            return;
        }
    }
    mask[0] |= 2;
    be_macro_reserved_reg_608DE0 = 1;
    (*count)++;
}

SInt32 fn_0049B080(PCodeBlock* last, UInt32* mask) {
    PCode* pc;
    PCodeArg* arg;
    SInt32 i;
    PCodeBlock* block;
    SInt32 count;

    count = 0;
    mask[0] = 0;
    mask[1] = 0;
    mask[2] = 0;
    mask[3] = 0;
    mask[4] = 0;
    mask[5] = 0;
    mask[6] = 0;
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (pc = block->firstPCode; pc; pc = pc->nextPCode) {
            i = 0;
            arg = pc->args;
            if (pc->argCount > 0) {
                do {
                    if (arg->kind == 0 && (arg->effect & 2)) {
                        mask[(SInt8)arg->arg] |= 1U << arg->data.reg.reg;
                        count++;
                    }
                    i++;
                    arg++;
                } while (i < pc->argCount);
            }
        }
    }
    if (mask[2] & 1) {
        reserve_macro_register(mask, &count);
    }
    return count * register_size;
}

#undef CERROR_FILE
#define CERROR_FILE "BE_macro.c"
void fn_00496520(PCode** current, UInt8 direction) {
    PCode* pc;
    PCode* instr;
    PCode* second;
    UInt32 upper, lower;
    SInt32 amount;
    CInt64 value;
    PCodeArg saved;
    UInt16 opcode;
    int index;

    pc = *current;
    be_macro_immediate_63613C++;
    CError_ASSERT(671, be_macro_immediate_63613C < 10);
    opcode = pc->op;
    index = opcode - 0x40A;
    switch (opcode) {
        case 0x4A5:
            amount = asm_stack_frame_offset_636E78 + codegen_operand_max_6363EC;
            if (amount) {
                amount =
                    -(((codegen_operand_max_6363EC + stack_alignment_63560C - 1) &
                       ~(stack_alignment_63560C - 1)) +
                      asm_stack_frame_offset_636E78);
                CInt64_SetLong(&value, amount);
                instr = makepcode(0x40B, 29, 29, value.hi, value.unk4, value.unk8, value.lo);
                instr->flags |= 0x100;
                insertpcodebefore(pc, instr);
                fn_00496520(&instr, 0);
            }
            if (copts.interrupt) {
                fn_0049AE70(1, pc, be_macro_mask_buffer_608DE4);
            } else {
                fn_0049A620(1, pc);
            }
            if (codegen_flag_63738A) {
                instr = makepcode(0xAA, 30, 29);
                insertpcodebefore(pc, instr);
            }
            deletepcode(pc);
            break;
        case 0x454:
        case 0x479:
            CError_ASSERT(1244, pc->args[1].data.mem.offset % 8 == 0);
        case 0x45F:
        case 0x481:
            if (opcode == 0x45F || opcode == 0x481) {
                CError_ASSERT(1203, pc->args[1].data.mem.offset % 4 == 0);
            }
        case 0x437:
        case 0x438:
        case 0x453:
        case 0x455:
        case 0x456:
        case 0x457:
        case 0x458:
        case 0x459:
        case 0x45A:
        case 0x45B:
        case 0x45C:
        case 0x45D:
        case 0x45E:
        case 0x460:
        case 0x461:
        case 0x462:
        case 0x463:
        case 0x464:
        case 0x465:
        case 0x466:
        case 0x467:
        case 0x468:
        case 0x477:
        case 0x478:
        case 0x47A:
        case 0x47B:
        case 0x47C:
        case 0x47D:
        case 0x47E:
        case 0x47F:
        case 0x480:
        case 0x482:
        case 0x483:
        case 0x484:
        case 0x485:
        case 0x486:
        case 0x487:
        case 0x488:
        case 0x489:
        case 0x48A: {
            ObjGenRegisterLocation location;
            fn_0046EDD0(pc->args[1].data.mem.obj, &location);
            if (location.number == -1 &&
                (SInt32)pc->args[1].data.mem.offset >= -32768 &&
                (SInt32)pc->args[1].data.mem.offset <= 32767) {
                pc->op = be_macro_expanded_opcode_6371D0[pc->op - 0x40A];
            } else if (location.number != 28 &&
                       location.number != stack_pointer_register &&
                       location.number != 23 &&
                       location.isDirect) {
                pc->op = be_macro_expanded_opcode_6371D0[pc->op - 0x40A];
                pc->unk38 = 1;
                pc->object = pc->args[1].data.mem.obj;
            }
            break;
        }
        case 0x43C:
        case 0x43E:
        case 0x441:
        case 0x443:
        case 0x445:
        case 0x447:
        case 0x449:
        case 0x46E:
        case 0x470:
        case 0x472:
            if (CInt64_NotEqual(pc->args[2].data.cint, cint64_zero)) {
                instr = make_instruction(0x450, 2);
                instr->args[0] = fn_004D02D0(1, 0, 2);
                instr->args[1] = pc->args[2];
                insertpcodebefore(pc, instr);
                fn_00496520(&instr, direction);
                pc->args[2] = fn_004D02D0(1, 0, 1);
            } else {
                pc->args[2] = fn_004D02D0(0, 0, 1);
            }
            second = make_instruction(be_macro_expanded_opcode_6371D0[index], 3);
            second->args[0] = pc->args[0];
            second->args[1] = pc->args[1];
            second->args[2] = pc->args[2];
            insertpcodebefore(pc, second);
            deletepcode(pc);
            fn_00496520(&second, direction);
            break;
        case 0x44C:
        case 0x44D:
        case 0x49F:
        case 0x4A0: {
            CInt64 negative;
            int index = pc->op - 0x40A;
            pc->op = be_macro_expanded_opcode_6371D0[index];
            negative = CInt64_Neg(pc->args[2].data.cint);
            pc->args[2].data.cint = negative;
            fn_00496520(&pc, direction);
            break;
        }
        case 0x494:
        case 0x496:
            instr = make_instruction(0x450, 2);
            instr->args[0] = fn_004D02D0(1, 0, 2);
            instr->args[1] = pc->args[2];
            insertpcodebefore(pc, instr);
            pc->op = be_macro_expanded_opcode_6371D0[index];
            pc->args[2] = pc->args[1];
            pc->args[1] = fn_004D02D0(1, 0, 1);
            fn_00496520(&instr, direction);
            break;
        case 0x43B:
        case 0x43D:
        case 0x43F:
        case 0x440:
        case 0x442:
        case 0x444:
        case 0x44A:
        case 0x44B:
        case 0x469:
        case 0x46A:
        case 0x46B:
        case 0x46C:
        case 0x46D:
        case 0x46F:
        case 0x471:
        case 0x474:
        case 0x475: {
            PCode* first;
            PCode* second;
            int index = pc->op - 0x40A;
            first = make_instruction(be_macro_expanded_opcode_6371D0[index], 2);
            first->args[0] = pc->args[1];
            first->args[1] = pc->args[2];
            second = make_instruction(be_macro_expanded_opcode2_637082[index], 1);
            second->args[0] = pc->args[0];
            insertpcodebefore(pc, first);
            insertpcodebefore(pc, second);
            deletepcode(pc);
            break;
        }
        case 0x4A3:
            pc->op = 0xF9;
            break;
        case 0x4A4:
            if (copts.fp_library & 2) {
                pc->op = 0xF8;
            } else {
                instr = makepcode(0xF9, pc->args[0].data.reg.reg + 1, pc->args[1].data.reg.reg + 1);
                insertpcodebefore(pc, instr);
                pc->op = 0xF9;
            }
            break;
        case 0x48D:
            pc->op = 0xE4;
            instr = makepcode(0xD6, pc->args[0].data.reg.reg, pc->args[0].data.reg.reg, 1);
            insertpcodeafter(pc, instr);
            break;
        case 0x48E:
            pc->op = 0x4A1;
            fn_00496520(&pc, direction);
            instr = makepcode(0xD6, pc->args[0].data.reg.reg, pc->args[0].data.reg.reg, 1);
            insertpcodeafter(pc, instr);
            break;
        case 0x49D:
            if (pc->args[2].data.reg.reg == 0) {
                pc->args[2] = pc->args[1];
                pc->args[1] = fn_004D02D0(0, 0, 1);
                pc->op = 0xD7;
            } else if (pc->args[1].data.reg.reg == 0) {
                pc->op = 0xD7;
            } else {
                pc->op = 0xE4;
                instr = makepcode(0xD7, pc->args[0].data.reg.reg, 0, pc->args[0].data.reg.reg);
                insertpcodeafter(pc, instr);
            }
            break;
        case 0x4A6:
            fn_0049A4B0(pc);
            break;
        case 0x4A7:
            deletepcode(pc);
            break;
        case 0x48B:
            fn_0049AE00(pc);
            break;
        case 0x48C:
            fn_0049AD60(pc);
            break;
        case 0x4A2:
            instr = makepcode(0x9D, 28, 0, be_macro_gp_last_63618C, 29);
            insertpcodebefore(pc, instr);
            deletepcode(pc);
            break;
        case 0x417:
        case 0x418:
        case 0x41B:
        case 0x41C:
        case 0x41F:
        case 0x420:
        case 0x423:
        case 0x424:
            pc->op = be_macro_expanded_opcode_6371D0[index];
            saved = pc->args[0];
            pc->args[0] = pc->args[1];
            pc->args[1] = saved;
            fn_00496520(&pc, direction);
            break;
        case 0x48F:
        case 0x491:
        case 0x493:
        case 0x495:
            opcode = be_macro_expanded_opcode_6371D0[index];
            pc->op = opcode;
            saved = pc->args[1];
            pc->args[1] = pc->args[2];
            pc->args[2] = saved;
            if ((SInt16)opcode > 0x409) {
                fn_00496520(&pc, direction);
            }
            break;
        case 0x497:
        case 0x499:
            opcode = be_macro_expanded_opcode_6371D0[index];
            pc->op = opcode;
            saved = pc->args[1];
            pc->args[1] = pc->args[2];
            pc->args[2] = saved;
            instr = makepcode(0xE5, pc->args[0].data.reg.reg, pc->args[0].data.reg.reg, 1);
            insertpcodeafter(pc, instr);
            if ((SInt16)opcode > 0x409) {
                fn_00496520(&pc, direction);
            }
            break;
        case 0x490:
        case 0x492:
            opcode = be_macro_expanded_opcode_6371D0[index];
            pc->op = opcode;
            instr = makepcode(0xE5, pc->args[0].data.reg.reg, pc->args[0].data.reg.reg, 1);
            insertpcodeafter(pc, instr);
            if ((SInt16)opcode > 0x409) {
                fn_00496520(&pc, direction);
            }
            break;
        case 0x498:
        case 0x49A:
            pc->op = be_macro_expanded_opcode_6371D0[index];
            pc->args[2].data.cint = CInt64_Add(pc->args[2].data.cint, cint64_one);
            fn_00496520(&pc, direction);
            break;
        case 0x49E:
            value = CInt64_Neg(pc->args[2].data.cint);
            instr = makepcode(
                0x43A, 1, pc->args[1].data.reg.reg, value.hi, value.unk4, value.unk8, value.lo
            );
            insertpcodebefore(pc, instr);
            pc->op = 0xD7;
            pc->args[1] = fn_004D02D0(0, 0, 1);
            pc->args[2] = instr->args[0];
            pc->args[2].effect = 1;
            fn_00496520(&instr, direction);
            break;
        case 0x452:
        case 0x476:
            pc->op = be_macro_expanded_opcode_6371D0[index];
            if (copts.unknown0C < 2 || !(copts.fp_library & 2)) {
                instr = make_instruction(pc->op, 3);
                instr->flags = pc->flags;
                instr->object = pc->args[1].data.mem.obj;
                instr->unk38 = pc->unk38;
                instr->alias = pc->alias;
                instr->args[0] = pc->args[0];
                if (!copts.littleendian) {
                    instr->args[0].effect |= 0x4000;
                } else {
                    pc->args[0].effect |= 0x4000;
                }
                instr->args[1] = pc->args[1];
                insertpcodebefore(pc, instr);
                pc->args[1].data.mem.offset += 4;
                fn_00496520(&instr, direction);
            }
            fn_00496520(&pc, direction);
            break;
        case 0x40A:
        case 0x40B:
        case 0x439:
        case 0x43A: {
            SInt16 immediateOpcode = be_macro_expanded_opcode_6371D0[index];
            SInt16 registerOpcode = be_macro_expanded_opcode2_637082[index];
            if (pc->unk38 == 0 && CInt64_IsInRange(pc->args[2].data.cint, 2)) {
                pc->op = immediateOpcode;
            } else {
                if (pc->unk38 == 0 && CInt64_Greater(pc->args[2].data.cint, cint64_zero)) {
                    if (CInt64_IsInRange(
                            CInt64_Sub(pc->args[2].data.cint, be_macro_split_immediate), 2
                        )) {
                        instr = make_instruction(immediateOpcode, 3);
                        instr->args[0] = pc->args[0];
                        instr->args[1] = pc->args[0];
                        instr->args[1].effect = 1;
                        instr->args[2] = pc->args[2];
                        instr->args[2].data.cint = be_macro_split_immediate;
                        pc->args[2].data.cint =
                            CInt64_Sub(pc->args[2].data.cint, be_macro_split_immediate);
                        pc->op = immediateOpcode;
                        insertpcodeafter(pc, instr);
                        return;
                    }
                }
                if (pc->unk38 == 0 && CInt64_Less(pc->args[2].data.cint, cint64_zero)) {
                    if (CInt64_IsInRange(
                            CInt64_Add(pc->args[2].data.cint, be_macro_split_immediate), 2
                        )) {
                        instr = make_instruction(immediateOpcode, 3);
                        instr->args[0] = pc->args[0];
                        instr->args[1] = pc->args[0];
                        instr->args[1].effect = 1;
                        instr->args[2] = pc->args[2];
                        instr->args[2].data.cint = CInt64_Neg(be_macro_split_immediate);
                        pc->args[2].data.cint =
                            CInt64_Add(pc->args[2].data.cint, be_macro_split_immediate);
                        pc->op = immediateOpcode;
                        insertpcodeafter(pc, instr);
                        return;
                    }
                }
                instr = make_instruction(0x450, 2);
                instr->args[0] = fn_004D02D0(1, 0, 2);
                instr->args[1] = pc->args[2];
                insertpcodebefore(pc, instr);
                instr->unk38 = pc->unk38;
                instr->alias = pc->alias;
                pc->op = registerOpcode;
                pc->args[2] = fn_004D02D0(1, 0, 1);
                pc->unk38 = 0;
                pc->alias = NULL;
                fn_00496520(&instr, direction);
            }
            break;
        }
        case 0x40D:
        case 0x40E:
        case 0x42F:
        case 0x430:
            opcode = be_macro_expanded_opcode2_637082[index];
            if (CInt64_NotEqual(pc->args[1].data.cint, cint64_zero)) {
                instr = make_instruction(0x450, 2);
                instr->args[0] = fn_004D02D0(1, 0, 2);
                instr->args[1] = pc->args[1];
                insertpcodebefore(pc, instr);
                fn_00496520(&instr, direction);
                pc->args[1] = instr->args[0];
                pc->args[1].effect = 1;
            } else {
                pc->args[1] = fn_004D02D0(0, 0, 1);
            }
            pc->op = opcode;
            break;
        case 0x411:
        case 0x412:
        case 0x429:
        case 0x42A:
            if (CInt64_IsZero(&pc->args[1].data.cint)) {
                switch (opcode) {
                    case 0x411:
                        pc->op = 0x1C;
                        break;
                    case 0x412:
                        pc->op = 0x1D;
                        break;
                    case 0x429:
                        pc->op = 0x24;
                        break;
                    case 0x42A:
                        pc->op = 0x25;
                        break;
                }
                pc->args[0] = pc->args[1];
                pc->argCount--;
                return;
            }
        case 0x415:
        case 0x416:
        case 0x42D:
        case 0x42E: {
            SInt16 expanded = be_macro_expanded_opcode_6371D0[index];
            instr = make_instruction(expanded, 3);
            instr->args[0] = fn_004D02D0(1, 0, 2);
            instr->args[1] = pc->args[0];
            instr->args[2] = pc->args[1];
            insertpcodebefore(pc, instr);
            pc->op = be_macro_expanded_opcode2_637082[index];
            pc->args[0] = instr->args[0];
            pc->args[0].effect = 1;
            pc->args[1] = fn_004D02D0(0, 0, 1);
            if (expanded > 0x409) {
                fn_00496520(&instr, direction);
            }
            break;
        }
        case 0x421:
        case 0x422:
            if (CInt64_IsZero(&pc->args[1].data.cint)) {
                pc->op = opcode == 0x421 ? 0x22 : 0x23;
                pc->args[0] = pc->args[1];
                pc->argCount--;
                return;
            }
        case 0x419:
        case 0x41A:
        case 0x41D:
        case 0x41E:
        case 0x425:
        case 0x426:
            instr = make_instruction(be_macro_expanded_opcode_6371D0[index], 3);
            instr->args[0] = fn_004D02D0(1, 0, 2);
            instr->args[1] = pc->args[0];
            instr->args[2] = pc->args[1];
            instr->args[2].data.cint = CInt64_Add(instr->args[2].data.cint, cint64_one);
            pc->op = be_macro_expanded_opcode2_637082[index];
            pc->args[0] = instr->args[0];
            pc->args[0].effect = 1;
            pc->args[1] = fn_004D02D0(0, 0, 1);
            insertpcodebefore(pc, instr);
            fn_00496520(&instr, direction);
            break;
        case 0x40F:
        case 0x410:
        case 0x413:
        case 0x414:
        case 0x427:
        case 0x428:
        case 0x42B:
        case 0x42C:
            instr = make_instruction(be_macro_expanded_opcode_6371D0[index], 3);
            instr->args[0] = fn_004D02D0(1, 0, 2);
            instr->args[1] = pc->args[0];
            instr->args[2] = pc->args[1];
            insertpcodebefore(pc, instr);
            pc->op = be_macro_expanded_opcode2_637082[index];
            pc->args[0] = instr->args[0];
            pc->args[0].effect = 1;
            pc->args[1] = fn_004D02D0(0, 0, 1);
            break;
        case 0x40C:
        case 0x473:
        case 0x4A1:
            if (!CInt64_IsInURange(pc->args[2].data.cint, 2)) {
                instr = make_instruction(0x450, 2);
                instr->args[0] = fn_004D02D0(1, 0, 2);
                instr->args[1] = pc->args[2];
                insertpcodebefore(pc, instr);
                pc->op = be_macro_expanded_opcode2_637082[index];
                pc->args[2] = instr->args[0];
                pc->args[2].effect = 1;
                fn_00496520(&instr, direction);
            } else {
                pc->op = be_macro_expanded_opcode_6371D0[index];
            }
            break;
        case 0x431:
        case 0x432:
        case 0x433:
        case 0x434:
        case 0x435:
        case 0x436:
            if (!CInt64_IsInRange(pc->args[1].data.cint, 2) || pc->unk38 != 0) {
                instr = make_instruction(0x450, 2);
                instr->args[0] = fn_004D02D0(1, 0, 2);
                instr->args[1] = pc->args[1];
                insertpcodebefore(pc, instr);
                instr->unk38 = pc->unk38;
                instr->alias = pc->alias;
                pc->op = be_macro_expanded_opcode2_637082[index];
                pc->args[1] = instr->args[0];
                pc->args[1].effect = 1;
                pc->unk38 = 0;
                pc->alias = NULL;
                fn_00496520(&instr, direction);
            } else {
                pc->op = be_macro_expanded_opcode_6371D0[index];
            }
            break;
        case 0x49B:
        case 0x49C:
            if (!CInt64_IsInRange(pc->args[2].data.cint, 2) || pc->unk38 != 0) {
                instr = make_instruction(0x450, 2);
                instr->args[0] = fn_004D02D0(1, 0, 2);
                instr->args[1] = pc->args[2];
                insertpcodebefore(pc, instr);
                instr->unk38 = pc->unk38;
                instr->alias = pc->alias;
                pc->op = be_macro_expanded_opcode2_637082[index];
                pc->args[2] = instr->args[0];
                pc->args[2].effect = 1;
                fn_00496520(&instr, direction);
            } else {
                pc->op = be_macro_expanded_opcode_6371D0[index];
            }
            break;
        case 0x44E: {
            ObjGenRegisterLocation loc;
            SInt32 displacement;
            SInt16 dst;
            Object* object;
            if (pc->args[1].kind == 6) {
                break;
            }
            object = pc->args[1].data.mem.obj;
            displacement = pc->args[1].data.mem.offset;
            dst = pc->args[0].data.reg.reg;
            fn_0046EDD0(object, &loc);
            if (loc.number == 28) {
                if (!loc.isDirect) {
                    instr = makepcode(0x90, 1, (displacement + 0x8000) >> 16);
                    instr->unk38 = 5;
                    instr->flags &= ~0x4000;
                    instr->alias = pc->alias;
                    instr->object = object;
                    insertpcodebefore(pc, instr);
                    second = makepcode(3, 1, instr->args[0].data.reg.reg, (SInt16)displacement);
                    second->unk38 = 6;
                    second->object = object;
                    second->alias = pc->alias;
                    insertpcodebefore(pc, second);
                    instr->unk3C = second;
                    second->unk3C = instr;
                    instr = makepcode(4, dst, second->args[0].data.reg.reg, 28);
                    instr->alias = pc->alias;
                    insertpcodebefore(pc, instr);
                    deletepcode(pc);
                    return;
                }
                instr = makepcode(3, dst, 28, (SInt16)displacement);
                instr->unk38 = loc.offset ? 8 : 7;
                instr->alias = pc->alias;
                instr->object = object;
                if (direction) {
                    *current = instr;
                    return;
                }
                insertpcodebefore(pc, instr);
                deletepcode(pc);
                return;
            }
            if (loc.number == 23 || (loc.number == 0 && !loc.isDirect)) {
                instr = makepcode(0x90, dst, (displacement + 0x8000) >> 16);
                insertpcodebefore(pc, instr);
                second = makepcode(3, dst, dst, (SInt16)displacement);
                insertpcodebefore(pc, second);
                instr->unk38 = 5;
                instr->flags &= ~0x4000;
                instr->object = object;
                instr->alias = pc->alias;
                second->unk38 = 6;
                second->object = object;
                second->alias = pc->alias;
                instr->unk3C = second;
                second->unk3C = instr;
                if (loc.number == 23) {
                    PCode* third = makepcode(4, dst, dst, 23);
                    third->alias = pc->alias;
                    insertpcodebefore(pc, third);
                }
                deletepcode(pc);
                return;
            }
            if (loc.number) {
                CError_ASSERT(
                    903,
                    loc.number == stack_pointer_register ||
                        (pc->args[2].data.reg.reg > 0 && pc->args[2].data.reg.reg <= 32)
                );
                instr = make_instruction(0x40B, 3);
                instr->args[0] = pc->args[0];
                instr->args[1] = pc->args[2];
                instr->args[2] = arg_from_int(displacement);
                instr->unk38 = pc->unk38;
                instr->object = object;
                instr->alias = pc->alias;
                insertpcodebefore(pc, instr);
                deletepcode(pc);
                fn_00496520(&instr, direction);
                return;
            }
            instr = makepcode(3, dst, pc->args[2].data.reg.reg, (SInt16)displacement);
            instr->unk38 = loc.offset ? 8 : 7;
            instr->object = object;
            instr->alias = pc->alias;
            if (direction) {
                *current = instr;
                return;
            }
            insertpcodebefore(pc, instr);
            deletepcode(pc);
            return;
        }
        case 0x450:
            upper = pc->args[1].data.cint.unk8;
            lower = pc->args[1].data.cint.lo;
            if (copts.unknown0C < 3 ||
                ((upper == 0 || upper == 0xFFFFFFFF) &&
                 (upper != 0xFFFFFFFF || (lower & 0x80000000)) &&
                 (upper != 0 || !(lower & 0x80000000)))) {
                if (pc->unk38 == 2) {
                    CError_ASSERT(603, !direction);
                    instr = makepcode(0x90, 1, (lower >> 16) + ((lower & 0x8000) >> 15));
                    instr->unk38 = 5;
                    instr->flags &= ~0x4000;
                    instr->object = NULL;
                    instr->alias = pc->alias;
                    insertpcodebefore(pc, instr);
                    second = makepcode(3, pc->args[0].data.reg.reg, 1, (SInt16)lower);
                    second->unk38 = 6;
                    second->object = NULL;
                    second->alias = pc->alias;
                    insertpcodebefore(pc, second);
                    deletepcode(pc);
                    instr->unk3C = second;
                    second->unk3C = instr;
                } else {
                    if (lower == 0) {
                        instr = makepcode(move_opcode, pc->args[0].data.reg.reg, 0);
                    } else if ((SInt32)lower < -32768 || (SInt32)lower > 32767) {
                        if ((SInt32)lower < 0 || (SInt32)lower > 65535) {
                            if ((lower & 0xFFFF) == 0) {
                                pc->op = 0x90;
                                pc->args[1] = arg_from_int(lower >> 16);
                                return;
                            }
                            CError_ASSERT(603, !direction);
                            instr = makepcode(0x90, 1, lower >> 16);
                            insertpcodebefore(pc, instr);
                            second = makepcode(0xBF, pc->args[0].data.reg.reg, 1, lower & 0xFFFF);
                            insertpcodebefore(pc, second);
                            deletepcode(pc);
                            return;
                        }
                        instr = make_instruction(0xBF, 3);
                        instr->args[0] = pc->args[0];
                        instr->args[1] = fn_004D02D0(0, 0, 1);
                        instr->args[2] = pc->args[1];
                    } else {
                        instr = make_instruction(0x78, 3);
                        instr->args[0] = pc->args[0];
                        instr->args[1] = fn_004D02D0(0, 0, 1);
                        instr->args[2] = pc->args[1];
                    }
                    if (!direction) {
                        insertpcodebefore(pc, instr);
                        deletepcode(pc);
                    } else {
                        *current = instr;
                    }
                }
            } else {
                CError_ASSERT(914, pc->unk38 == 0);
                CError_ASSERT(603, !direction);
                instr = makepcode(0xBF, pc->args[0].data.reg.reg, 0, upper >> 16);
                insertpcodebefore(pc, instr);
                instr = makepcode(0x84, pc->args[0].data.reg.reg, pc->args[0].data.reg.reg, 16);
                insertpcodebefore(pc, instr);
                if (upper & 0xFFFF) {
                    instr = makepcode(
                        0xBF, pc->args[0].data.reg.reg, pc->args[0].data.reg.reg, upper & 0xFFFF
                    );
                    insertpcodebefore(pc, instr);
                }
                instr = makepcode(0x84, pc->args[0].data.reg.reg, pc->args[0].data.reg.reg, 16);
                insertpcodebefore(pc, instr);
                if ((lower >> 16) & 0xFFFF) {
                    instr = makepcode(
                        0xBF,
                        pc->args[0].data.reg.reg,
                        pc->args[0].data.reg.reg,
                        (lower >> 16) & 0xFFFF
                    );
                    insertpcodebefore(pc, instr);
                }
                instr = makepcode(0x84, pc->args[0].data.reg.reg, pc->args[0].data.reg.reg, 16);
                insertpcodebefore(pc, instr);
                if (lower & 0xFFFF) {
                    instr = makepcode(
                        0xBF, pc->args[0].data.reg.reg, pc->args[0].data.reg.reg, lower & 0xFFFF
                    );
                    insertpcodebefore(pc, instr);
                }
                deletepcode(pc);
            }
            break;
        default:
            break;
    }
}
