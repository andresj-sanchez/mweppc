#include "compiler/CError.h"
#include "compiler/LiveRegisters.h"
#include "compiler/MIPS_Peephole.h"
#include "compiler/CParser.h"
#include "compiler/common.h"
#include "compiler/PCode.h"
#include "compiler/PCodeInfo.h"
#include "compiler/CompilerTools.h"
#include "compiler/CodeGen.h"
#include "compiler/CInt64.h"
#include "compiler/Alias.h"
#include "compiler/BitVectors.h"
#include "compiler/Peephole.h"
#include "compiler/Registers.h"

//! @reconstructed
static inline Boolean writtensince(PCode* start, PCode* end, PCodeArg* op) {
    PCodeArg* arg;
    int count;

    if (definedbetween(start, end, op)) {
        return TRUE;
    }
    count = end->argCount;
    arg = end->args;
    while (count--) {
        if (arg->kind == op->kind &&
            arg->data.reg.reg == op->data.reg.reg &&
            arg->arg == op->arg &&
            (arg->effect & 2)) {
            return TRUE;
        }
        arg++;
    }
    return FALSE;
}

int fn_0055C850(PCode* pc) {
    PCode* other;
    SInt32 bits;

    other = pc->args[1].data.reg.definer;
    if (other->op != 7) {
        return FALSE;
    }
    bits = getbit((UInt32)other->args[2].data.cint.lo + 1);
    if (bits > 0) {
        if (pc->args[2].data.imm.value <= bits) {
            return FALSE;
        }
    } else {
        return FALSE;
    }
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    return TRUE;
}

int fn_0055C460(PCode* pc) {
    PCode* candidate;
    candidate = pc->args[0].data.reg.definer;
    if (candidate->op != 0x48C && candidate->op != 0x48B) {
        return 0;
    }
    if (definedbetween(pc, candidate, &candidate->args[1])) {
        return 0;
    }
    if (candidate->args[2].data.imm.value != 8) {
        return 0;
    }
    if (candidate->args[0].data.reg.reg == candidate->args[1].data.reg.reg) {
        candidate->op = move_opcode;
        candidate->flags |= 0x10;
        candidate->argCount = 2;
        return 1;
    }
    pc->args[0].data.reg.definer = candidate->args[1].data.reg.definer;
    pc->args[0] = candidate->args[1];
    return 1;
}

int fn_0055D160(PCode* pc) {
    PCode* next = pc->args[1].data.reg.definer;
    CInt64 value;
    if (next->op == 0x450) {
        value = next->args[1].data.cint;
    } else if ((next->op == 3 || next->op == 0x78) && !next->args[1].data.reg.reg) {
        value = next->args[2].data.cint;
    } else {
        return FALSE;
    }
    if (!CInt64_IsInRange(value, pc->args[2].data.reg.reg)) {
        return FALSE;
    }
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    return TRUE;
}

int fn_0055D070(PCode* pc) {
    PCode* next = pc->args[1].data.reg.definer;
    CInt64 value;
    CInt64 shift;
    if (next->op == 0x450) {
        value = next->args[1].data.cint;
    } else if (next->op == 3 && !next->args[1].data.reg.reg) {
        value = next->args[2].data.cint;
    } else {
        return FALSE;
    }
    pc->op = 0x450;
    pc->args[1].kind = 3;
    CInt64_SetLong(&shift, pc->args[2].data.imm.value);
    pc->args[1].data.cint = CInt64_Shl(value, shift);
    pc->argCount = 2;
    return TRUE;
}

int fn_0055D380(PCode* pc) {
    PCode* next = pc->args[1].data.reg.definer;
    CInt64 value;
    if (next->op == 0x450) {
        value = next->args[1].data.cint;
    } else if ((next->op == 3 || next->op == 0x78) && !next->args[1].data.reg.reg) {
        value = next->args[2].data.cint;
    } else {
        return FALSE;
    }
    pc->op = 0x450;
    pc->args[1] = pc->args[2];
    pc->args[1].data.cint = CInt64_And(pc->args[2].data.cint, value);
    pc->argCount = 2;
    return TRUE;
}

int fn_0055D220(PCode* pc) {
    PCode* next = pc->args[1].data.reg.definer;
    CInt64 value;
    CInt64 shift;
    if (next->op == 0x450) {
        value = next->args[1].data.cint;
    } else if ((next->op == 3 || next->op == 0x78) && !next->args[1].data.reg.reg) {
        value = next->args[2].data.cint;
    } else {
        return FALSE;
    }
    pc->op = 0x450;
    pc->args[1].kind = 3;
    CInt64_SetLong(&shift, pc->args[2].data.imm.value);
    pc->args[1].data.cint =
        CInt64_And(value, CInt64_Sub(CInt64_Shl(cint64_one, shift), cint64_one));
    pc->argCount = 2;
    return TRUE;
}

int fn_0055C510(PCode* pc) {
    PCode* next;
    PCodeArg* arg;
    UInt32 index;
    UInt8 blocked;

    next = pc->args[1].data.reg.definer;
    if (next->op != peephole_opcode_636FCC) {
        return FALSE;
    }
    if (definedbetween(pc, next, &next->args[1])) {
        blocked = TRUE;
    } else {
        blocked = FALSE;
        index = next->argCount;
        arg = next->args;
        while (index--) {
            if (arg->kind == next->args[1].kind &&
                arg->data.reg.reg == next->args[1].data.reg.reg &&
                arg->arg == next->args[1].arg &&
                (arg->effect & 2)) {
                blocked = TRUE;
                break;
            }
            arg++;
        }
    }
    if (blocked) {
        return FALSE;
    }
    if (pc->args[2].data.imm.value < next->args[2].data.imm.value) {
        pc->args[1].data.reg.definer = next->args[1].data.reg.definer;
        pc->args[1] = next->args[1];
        return TRUE;
    }
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    return TRUE;
}

int fn_0055C610(PCode* pc) {
    PCode* next;
    PCodeArg* arg;
    UInt32 index;
    UInt8 blocked;

    next = pc->args[1].data.reg.definer;
    if (next->op != peephole_opcode_637054) {
        return FALSE;
    }
    if (pc->args[2].data.imm.value >= next->args[2].data.imm.value) {
        pc->op = move_opcode;
        pc->flags |= 0x10;
        pc->argCount = 2;
        return TRUE;
    }
    if (pc->args[2].data.imm.value < next->args[2].data.imm.value) {
        if (definedbetween(pc, next, &next->args[1])) {
            blocked = TRUE;
        } else {
            blocked = FALSE;
            index = next->argCount;
            arg = next->args;
            while (index--) {
                if (arg->kind == next->args[1].kind &&
                    arg->data.reg.reg == next->args[1].data.reg.reg &&
                    arg->arg == next->args[1].arg &&
                    (arg->effect & 2)) {
                    blocked = TRUE;
                    break;
                }
                arg++;
            }
        }
        if (blocked) {
            return FALSE;
        }
        pc->args[1].data.reg.definer = next->args[1].data.reg.definer;
        pc->args[1] = next->args[1];
        return TRUE;
    }
    return FALSE;
}

int fn_0055C720(PCode* pc) {
    PCode* next;

    next = pc->args[1].data.reg.definer;
    if (next->op != 0xE5 && next->op != 0x4A1) {
        return FALSE;
    }
    if (CInt64_NotEqual(next->args[2].data.cint, pc->args[2].data.cint)) {
        return FALSE;
    }
    if (writtensince(pc, next, &next->args[1])) {
        return FALSE;
    }
    pc->args[1].data.reg.definer = next->args[1].data.reg.definer;
    pc->args[1] = next->args[1];
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    return TRUE;
}

int fn_0055D480(PCode* pc) {
    PCode* next;
    UInt32 flags;

    next = pc->args[1].data.reg.definer;
    if (next->op != 0xD2) {
        return FALSE;
    }
    if (writtensince(pc, next, &next->args[1])) {
        return FALSE;
    }
    if (pc->args[2].data.imm.value != next->args[2].data.imm.value) {
        return FALSE;
    }
    flags = pc->flags & 0x180E0;
    pc->op = 0x48B;
    pc->flags = peephole_flags_634060 | flags;
    pc->args[2].kind = 2;
    pc->args[2].data.imm.value = 32 - next->args[2].data.imm.value;
    pc->args[1].data.reg.definer = next->args[1].data.reg.definer;
    pc->args[1] = next->args[1];
    return TRUE;
}

int fn_0055D590(PCode* pc) {
    PCode* next;
    SInt32 shift;

    next = pc->args[1].data.reg.definer;
    if (next->op != 0xD2) {
        return FALSE;
    }
    if (writtensince(pc, next, &next->args[1])) {
        return FALSE;
    }
    shift = pc->args[2].data.imm.value + next->args[2].data.imm.value;
    if (shift >= 31) {
        return FALSE;
    }
    pc->args[1].data.reg.definer = next->args[1].data.reg.definer;
    pc->args[1] = next->args[1];
    pc->args[2].data.imm.value = shift;
    return TRUE;
}

void fn_0055D890(PCode* pc) {
    PCodeArg* arg;
    SInt32 index;

    arg = pc->args;
    index = pc->argCount;
    while (index--) {
        if (arg->kind == 0 && (arg->effect & 2)) {
            if ((register_live_bitmaps_6368D0[(SInt8)arg->arg].current[arg->data.reg.reg >> 5] >>
                 arg->data.reg.reg) &
                1) {
                peephole_register_state_636220[(SInt8)arg->arg].count--;
            }
            register_live_bitmaps_6368D0[(SInt8)arg->arg].current[arg->data.reg.reg >> 5] &=
                ~(1u << (arg->data.reg.reg & 31));
        }
        arg++;
    }
    arg = pc->args;
    index = pc->argCount;
    while (index--) {
        if (arg->kind == 0 && (arg->effect & 1)) {
            if (!((register_live_bitmaps_6368D0[(SInt8)arg->arg].current[arg->data.reg.reg >> 5] >>
                   arg->data.reg.reg) &
                  1)) {
                peephole_register_state_636220[(SInt8)arg->arg].count++;
            }
            register_live_bitmaps_6368D0[(SInt8)arg->arg].current[arg->data.reg.reg >> 5] |= 1u
                << arg->data.reg.reg;
        }
        arg++;
    }
}

void fn_0055D750(void) {
    SInt8 cls;
    PCodeBlock* block;
    PCode* pc;
    SInt32 idx;

    for (cls = 0; cls < 7; cls++) {
        peephole_register_state_636220[cls].words = oalloc(pcblockcount * 4);
        block = pcbasicblocks;
        peephole_register_state_636220[cls].available = available_registers(cls);
    }
    for (; block; block = block->nextBlock) {
        capture_live_registers(block);
        for (cls = 0; cls < 7; cls++) {
            peephole_register_state_636220[cls].count = bitvectorcount(
                register_live_bitmaps_6368D0[cls].current,
                register_live_bitmaps_6368D0[cls].registerCount
            );
        }
        cls = 0;
        do {
            SInt32 n = cls++;
            peephole_register_state_636220[n].words[block->blockIndex] =
                peephole_register_state_636220[n].count;
        } while (cls < 7);
        for (pc = block->lastPCode; pc; pc = pc->prevPCode) {
            fn_0055D890(pc);
            cls = 0;
            do {
                SInt32 n = cls;
                idx = block->blockIndex;
                if ((UInt32)peephole_register_state_636220[n].words[idx] <
                    (UInt32)peephole_register_state_636220[n].count) {
                    peephole_register_state_636220[n].words[idx] =
                        peephole_register_state_636220[n].count;
                }
                cls++;
            } while (cls < 7);
        }
    }
}

int fn_0055C8B0(PCode* pc) {
    PCode* next;
    UInt8 bit;
    SInt32 cls;
    SInt32 reg;
    SInt32 width;
    volatile SInt32 used;
    SInt32 mainop;
    UInt16 subop;
    UInt16 midop;
    UInt16 highop;
    UInt16 middleop;
    UInt16 altop;
    UInt16 nextop;
    SInt32 adjustment;

    next = pc->args[1].data.reg.definer;
    if (!(next->flags & 2)) {
        return FALSE;
    }
    reg = next->args[0].data.reg.reg;
    cls = (SInt8)next->args[0].arg;
    bit = (register_live_bitmaps_6368D0[cls].current[reg >> 5] >> reg) & 1;
    width = pc->args[2].data.imm.value;
    if (width != 8 && width != 16 && width != 32) {
        return FALSE;
    }
    used = usedbetween(pc, next, &next->args[0]);
    if (pc->op == 0x48B) {
        mainop = 0x91;
        subop = 0x45A;
        altop = 0x94;
        midop = 0x45B;
        highop = 0x9D;
        middleop = 0x45F;
    } else if (pc->op == 0x48C) {
        mainop = 0x92;
        subop = 0x453;
        altop = 0x95;
        midop = 0x45C;
        highop = 0xA0;
        middleop = 0x466;
    } else {
        CError_Internal("MIPS_Peephole.c", 575);
    }
    nextop = next->op;
    if (width == 8) {
        if (nextop == mainop || nextop == subop) {
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            return TRUE;
        }
        if (used || bit) {
            return FALSE;
        }
        if ((nextop == 0x91 || nextop == 0x92)) {
            next->op = mainop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            return TRUE;
        }
        if ((nextop == 0x45A || nextop == 0x453)) {
            next->op = subop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            return TRUE;
        }
        if ((nextop == 0x94 || nextop == 0x95)) {
            next->op = mainop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            adjustment = copts.littleendian ? 0 : 1;
            goto adjusted;
        }
        if ((nextop == 0x45B || nextop == 0x45C)) {
            next->op = subop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            adjustment = copts.littleendian ? 0 : 1;
            goto adjusted;
        }
        if ((nextop == 0x9D || nextop == 0xA0)) {
            next->op = mainop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            adjustment = copts.littleendian ? 0 : 3;
            goto adjusted;
        }
        if ((nextop == 0x45F || nextop == 0x466)) {
            next->op = subop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            adjustment = copts.littleendian ? 0 : 3;
            goto adjusted;
        }
        if (nextop == 0x93) {
            next->op = mainop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            adjustment = copts.littleendian ? 0 : 7;
            goto adjusted;
        }
        if (nextop == 0x454) {
            next->op = subop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            adjustment = copts.littleendian ? 0 : 7;
            goto adjusted;
        }
        return FALSE;
    }
    if (width == 16) {
        if (nextop == altop || nextop == midop || nextop == mainop || nextop == subop) {
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            return TRUE;
        }
        if (used || bit) {
            return FALSE;
        }
        if ((nextop == 0x94 || nextop == 0x95)) {
            next->op = altop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            return TRUE;
        }
        if ((nextop == 0x45B || nextop == 0x45C)) {
            next->op = midop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            return TRUE;
        }
        if ((nextop == 0x9D || nextop == 0xA0)) {
            next->op = altop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            adjustment = copts.littleendian ? 0 : 2;
            goto adjusted;
        }
        if ((nextop == 0x45F || nextop == 0x466)) {
            next->op = midop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            adjustment = copts.littleendian ? 0 : 2;
            goto adjusted;
        }
        if (nextop == 0x93) {
            next->op = altop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            adjustment = copts.littleendian ? 0 : 6;
            goto adjusted;
        }
        if (nextop == 0x454) {
            next->op = midop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            adjustment = copts.littleendian ? 0 : 6;
            goto adjusted;
        }
        return FALSE;
    }
    if (width == 32) {
        if (nextop == highop ||
            nextop == middleop ||
            nextop == altop ||
            nextop == midop ||
            nextop == mainop ||
            nextop == subop) {
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            return TRUE;
        }
        if (used || bit) {
            return FALSE;
        }
        if ((nextop == 0x9D || nextop == 0xA0)) {
            next->op = highop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            return TRUE;
        }
        if ((nextop == 0x45F || nextop == 0x466)) {
            next->op = middleop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            return TRUE;
        }
        if (nextop == 0x93) {
            next->op = highop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            adjustment = copts.littleendian ? 0 : 4;
            goto adjusted;
        }
        if (nextop == 0x454) {
            next->op = middleop;
            pc->op = move_opcode;
            pc->flags |= 0x10;
            pc->argCount = 2;
            adjustment = copts.littleendian ? 0 : 4;
            goto adjusted;
        }
        return FALSE;
    }
    return FALSE;
adjusted:
    next->args[1].data.mem.offset += adjustment;
    return TRUE;
}

UInt8 fn_0055D680(PCode* instruction, PCode** endRef, PCodeArg* operand) {
    PCode* end;
    PCodeArg* arg;
    UInt32 index;
    UInt8 defined;
    UInt8 used;

    defined = FALSE;
    used = FALSE;
    end = *endRef;
    while (instruction != end) {
        arg = instruction->args;
        index = instruction->argCount;
        while (index--) {
            if (arg->kind == operand->kind &&
                arg->data.reg.reg == operand->data.reg.reg &&
                arg->arg == operand->arg) {
                if (arg->effect & 2) {
                    defined = TRUE;
                } else {
                    used = TRUE;
                }
            }
            arg++;
        }
        if (defined && !used) {
            return FALSE;
        }
        if (used) {
            return TRUE;
        }
        instruction = instruction->nextPCode;
    }
    return FALSE;
}

#pragma dont_inline on
int fn_00558520(PCode* pc, UInt16 matchingOpcode, UInt16 firstOpcode, UInt16 secondOpcode) {
    PCode* first = pc->args[1].data.reg.definer;
    PCode* second = pc->args[2].data.reg.definer;
    if (first->op == matchingOpcode) {
        return fn_00558610(pc, firstOpcode, 2, first);
    }
    if (second->op != matchingOpcode) {
        return FALSE;
    }
    return fn_00558610(pc, secondOpcode, 1, second);
}

int fn_005585B0(PCode* pc, UInt16 matchingOpcode, UInt16 opcode) {
    PCode* first = pc->args[1].data.reg.definer;
    PCode* second = pc->args[2].data.reg.definer;
    if (first->op == matchingOpcode) {
        return fn_00558610(pc, opcode, 2, first);
    }
    if (second->op != matchingOpcode) {
        return FALSE;
    }
    return fn_00558610(pc, opcode, 1, second);
}

#pragma dont_inline reset

int fn_005584E0(PCode* pc) {
    return fn_00558520(pc, 0xB4, 0x11C, 0x120);
}

int fn_00558500(PCode* pc) {
    return fn_00558520(pc, 0xB5, 0x11D, 0x121);
}

int fn_00558570(PCode* pc) {
    return fn_005585B0(pc, 0xB4, 0x10E);
}

int fn_00558590(PCode* pc) {
    return fn_005585B0(pc, 0xB5, 0x10F);
}

int fn_00558C20(PCode* pc) {
    PCode* next;
    if (pc->flags & 0x100) {
        return FALSE;
    }
    next = pc->args[0].data.reg.definer;
    if (next->op != 0xA5) {
        return FALSE;
    }
    if (definedbetween(pc, next, &next->args[1])) {
        return FALSE;
    }
    pc->args[0].data.reg.definer = pc->args[1].data.reg.definer;
    pc->args[0] = pc->args[1];
    pc->args[1].data.reg.definer = next->args[1].data.reg.definer;
    pc->args[1] = next->args[1];
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    pc->op = 0x4A3;
    return TRUE;
}

int fn_00558CE0(PCode* pc) {
    PCode* next = pc->args[1].data.reg.definer;
    if (pc->flags & 0x100) {
        return FALSE;
    }
    if (next->op != 0xAD) {
        return FALSE;
    }
    if (definedbetween(pc, next, &next->args[0])) {
        return FALSE;
    }
    pc->args[1].data.reg.definer = next->args[0].data.reg.definer;
    pc->args[1] = next->args[0];
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    return TRUE;
}

int fn_00558E20(PCode* pc) {
    PCode* next = pc->args[1].data.reg.definer;
    if (next->op != 0xB1) {
        return FALSE;
    }
    if (definedbetween(pc, next, &next->args[0])) {
        return FALSE;
    }
    pc->args[1].data.reg.definer = next->args[0].data.reg.definer;
    pc->args[1] = next->args[0];
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    return TRUE;
}

int fn_00558EA0(PCode* pc) {
    PCode* end = pc->args[1].data.reg.definer;
    UInt16 opcode;
    UInt32 endflags;
    UInt32 pcflags;
    SInt32 cls;
    SInt32 reg;

    if (end->op == 0xA1) {
        opcode = 0x9D;
    } else if (end->op == 0x460) {
        opcode = 0x45F;
    } else {
        return FALSE;
    }
    if (usedbetween(pc, end, &end->args[0])) {
        return FALSE;
    }
    if (usedbetween(pc, end, &pc->args[0])) {
        return FALSE;
    }
    cls = (SInt8)end->args[0].arg;
    reg = end->args[0].data.reg.reg;
    if ((register_live_bitmaps_6368D0[cls].current[reg >> 5] >> (reg & 31)) & 1) {
        return FALSE;
    }
    endflags = end->flags & 0x180E0;
    end->op = opcode;
    end->flags = opcode_info[opcode].flags | endflags;
    end->args[0].data.reg.definer = pc->args[0].data.reg.definer;
    end->args[0] = pc->args[0];
    pcflags = pc->flags & 0x180E0;
    pc->op = 0xBA;
    pc->flags = opcode_info[0xBA].flags | pcflags;
    pc->argCount = 0;
    return TRUE;
}

int fn_00558FB0(PCode* pc) {
    PCode* end = pc->args[1].data.reg.definer;
    UInt16 opcode;
    UInt32 endflags;
    UInt32 pcflags;
    SInt32 cls;
    SInt32 reg;

    if (end->op == 0x96) {
        opcode = 0x93;
    } else if (end->op == 0x455 || end->op == 0x452) {
        opcode = 0x454;
    } else {
        return FALSE;
    }
    if (usedbetween(pc, end, &end->args[0])) {
        return FALSE;
    }
    if (usedbetween(pc, end, &pc->args[0])) {
        return FALSE;
    }
    cls = (SInt8)end->args[0].arg;
    reg = end->args[0].data.reg.reg;
    if ((register_live_bitmaps_6368D0[cls].current[reg >> 5] >> (reg & 31)) & 1) {
        return FALSE;
    }
    endflags = end->flags & 0x180E0;
    end->op = opcode;
    end->flags = opcode_info[opcode].flags | endflags;
    end->args[0].data.reg.definer = pc->args[0].data.reg.definer;
    end->args[0] = pc->args[0];
    pcflags = pc->flags & 0x180E0;
    pc->op = 0xBA;
    pc->flags = opcode_info[0xBA].flags | pcflags;
    pc->argCount = 0;
    return TRUE;
}

int fn_005590D0(PCode* pc) {
    PCode* next = pc->args[0].data.reg.definer;
    UInt32 flags;
    SInt32 result;
    if (next->op == 0xB1) {
        if (!definedbetween(pc, next, &next->args[0])) {
            flags = pc->flags;
            pc->op = 0xC4;
            pc->flags = (flags & 0x180E0) | opcode_info[0xC4].flags;
            pc->args[0].data.reg.definer = next->args[0].data.reg.definer;
            pc->args[0] = next->args[0];
            result = TRUE;
        } else {
            result = FALSE;
        }
    } else {
        result = FALSE;
    }
    return result;
}

int fn_00559150(PCode* pc) {
    PCode* next = pc->args[0].data.reg.definer;
    UInt32 flags;
    SInt32 result;
    if (next->op == 0xB1) {
        if (!definedbetween(pc, next, &next->args[0])) {
            flags = pc->flags;
            pc->op = 0x479;
            pc->flags = (flags & 0x180E0) | opcode_info[0x479].flags;
            pc->args[0].data.reg.definer = next->args[0].data.reg.definer;
            pc->args[0] = next->args[0];
            result = TRUE;
        } else {
            result = FALSE;
        }
    } else {
        result = FALSE;
    }
    return result;
}

int fn_005591D0(PCode* pc) {
    PCode* next = pc->args[0].data.reg.definer;
    UInt32 flags;
    SInt32 result;
    if (next->op == 0xAD) {
        if (!definedbetween(pc, next, &next->args[0])) {
            flags = pc->flags;
            pc->op = 0xCB;
            pc->flags = (flags & 0x180E0) | opcode_info[0xCB].flags;
            pc->args[0].data.reg.definer = next->args[0].data.reg.definer;
            pc->args[0] = next->args[0];
            result = TRUE;
        } else {
            result = FALSE;
        }
    } else {
        result = FALSE;
    }
    return result;
}

int fn_00559250(PCode* pc) {
    PCode* next = pc->args[0].data.reg.definer;
    UInt32 flags;
    SInt32 result;
    if (next->op == 0xAD) {
        if (!definedbetween(pc, next, &next->args[0])) {
            flags = pc->flags;
            pc->op = 0x481;
            pc->flags = (flags & 0x180E0) | opcode_info[0x481].flags;
            pc->args[0].data.reg.definer = next->args[0].data.reg.definer;
            pc->args[0] = next->args[0];
            result = TRUE;
        } else {
            result = FALSE;
        }
    } else {
        result = FALSE;
    }
    return result;
}

int fn_00558D70(PCode* pc) {
    PCode* next = pc->args[0].data.reg.definer;
    if (next->op != 0x7F) {
        return FALSE;
    }
    if (definedbetween(pc, next, &next->args[1])) {
        return FALSE;
    }
    pc->args[0].data.reg.definer = pc->args[1].data.reg.definer;
    pc->args[0] = pc->args[1];
    pc->args[1].data.reg.definer = next->args[1].data.reg.definer;
    pc->args[1] = next->args[1];
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    pc->op = 0x4A4;
    return TRUE;
}

//! @reconstructed
static Boolean referencedbetween(PCode* start, PCode* end, PCodeArg* op) {
    PCode* scan;
    PCodeArg* arg;
    SInt32 count;

    for (scan = start->prevPCode; scan != end; scan = scan->prevPCode) {
        arg = scan->args;
        count = scan->argCount;
        while (count--) {
            if (arg->kind == op->kind &&
                arg->data.reg.reg == op->data.reg.reg &&
                arg->arg == op->arg &&
                (arg->effect & 3)) {
                return TRUE;
            }
            arg++;
        }
    }
    return FALSE;
}

int fn_00559330(PCode* pc, UInt16 opcode1, UInt16 opcode2, UInt16 new1, UInt16 new2) {
    PCode* next = pc->args[0].data.reg.definer;
    UInt16 newOpcode;
    SInt32 cls;
    SInt32 reg;

    if (next == NULL) {
        return FALSE;
    }
    if (next->op == opcode1) {
        newOpcode = new1;
    } else if (next->op == opcode2) {
        newOpcode = new2;
    } else {
        return FALSE;
    }
    if (usedbetween(pc, next, &next->args[0])) {
        return FALSE;
    }
    cls = (SInt8)next->args[0].arg;
    reg = next->args[0].data.reg.reg;
    if ((register_live_bitmaps_6368D0[cls].current[reg >> 5] >> (reg & 31)) & 1) {
        return FALSE;
    }
    if (referencedbetween(pc, next, &pc->args[1])) {
        return FALSE;
    }
    next->op = newOpcode;
    next->args[0] = pc->args[1];
    next->args[0].data.reg.definer = next;
    next->flags |= pc->flags & 0x100;
    deletepcode(pc);
    return TRUE;
}

void fn_005592D0(PCode* pc) {
    fn_00559330(pc, 0x93, 0x454, 0x96, 0x455);
}

void fn_00559300(PCode* pc) {
    fn_00559330(pc, 0x9D, 0x45F, 0xA1, 0x460);
}

void fn_00559D00(PCode* pc) {
    fn_00559D80(pc, 0x10C);
}

void fn_00559D20(PCode* pc) {
    fn_00559D80(pc, 0x10A);
}

void fn_00559D40(PCode* pc) {
    fn_00559D80(pc, 0x10D);
}

void fn_00559D60(PCode* pc) {
    fn_00559D80(pc, 0x10B);
}

void fn_00559F80(PCode* pc) {
    fn_00559FE0(pc, 0x1D7, 0x1D8, 0x202, 0x203, 0x1D8, 1);
}

void fn_00559FB0(PCode* pc) {
    fn_00559FE0(pc, 0x1D5, 0x1D6, 0x1DB, 0x1DC, 0x1D6, 0);
}

void fn_0055A1C0(PCode* pc) {
    fn_0055AAB0(pc, 0x1E2, 0x1E3, 0x1E2);
}

void fn_0055A1E0(PCode* pc) {
    fn_0055AAB0(pc, 0x1E2, 0x1E3, 0x1DE);
}

void fn_0055A200(PCode* pc) {
    fn_0055AAB0(pc, 0x1DE, 0x1DF, 0x1E2);
}

void fn_0055A220(PCode* pc) {
    fn_0055AAB0(pc, 0x1DE, 0x1DF, 0x1DE);
}

int fn_0055A180(PCode* pc) {
    if (pc->flags & 0x100) {
        return FALSE;
    }
    if (pc->args[3].data.reg.reg == 0 &&
        (UInt16)pc->args[1].data.reg.reg == (UInt16)pc->args[2].data.reg.reg) {
        deletepcode(pc);
        return TRUE;
    }
    return FALSE;
}

void fn_0055A940(PCode* pc) {
    fn_0055AED0(pc, 0x103, 0x1E5, 0x1E2);
}

void fn_0055A960(PCode* pc) {
    fn_0055AED0(pc, 0xED, 0x1DD, 0x1E2);
}

void fn_0055A980(PCode* pc) {
    fn_0055AED0(pc, 0x103, 0x1E5, 0x1DE);
}

int fn_0055A240(PCode* pc) {
    PCode* end;
    if (pc->flags & 0x100) {
        return FALSE;
    }
    end = pc->args[0].data.reg.definer;
    if (end == NULL || end->op != 0x264) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[1])) {
        return FALSE;
    }
    if (pc->op == 0x1D2) {
        UInt32 saved = pc->flags & 0x180E0;
        UInt32 flags;
        pc->op = 0x268;
        flags = opcode_info[0x268].flags;
        pc->flags = flags | saved;
    }
    if (pc->op == 0x489) {
        UInt32 saved = pc->flags & 0x180E0;
        UInt32 flags;
        pc->op = 0x48A;
        flags = opcode_info[0x48A].flags;
        pc->flags = flags | saved;
    }
    pc->args[0].data.reg.definer = end->args[1].data.reg.definer;
    pc->args[0] = end->args[1];
    return TRUE;
}

int fn_0055A300(PCode* pc) {
    PCode* end;
    if (pc->flags & 0x100) {
        return FALSE;
    }
    end = pc->args[0].data.reg.definer;
    if (end == NULL || end->op != 0x266) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[0])) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[1])) {
        return FALSE;
    }
    if (usedbetween(pc, end, &end->args[1])) {
        return FALSE;
    }
    if (pc->op == 0x268) {
        UInt32 saved = pc->flags & 0x180E0;
        UInt32 flags;
        pc->op = 0x1D2;
        flags = opcode_info[0x1D2].flags;
        pc->flags = flags | saved;
    }
    if (pc->op == 0x48A) {
        UInt32 saved = pc->flags & 0x180E0;
        UInt32 flags;
        pc->op = 0x489;
        flags = opcode_info[0x489].flags;
        pc->flags = flags | saved;
    }
    pc->args[0].data.reg.definer = end->args[0].data.reg.definer;
    pc->args[0] = end->args[0];
    pc->args[0].effect &= 1;
    return TRUE;
}

int fn_0055A400(PCode* pc) {
    PCode* end;
    if (pc->flags & 0x100) {
        return FALSE;
    }
    end = pc->args[1].data.reg.definer;
    if (end == NULL || end->op != 0x266) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[1])) {
        return FALSE;
    }
    pc->args[1].data.reg.definer = end->args[0].data.reg.definer;
    pc->args[1] = end->args[0];
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    pc->op = move_opcode_637350;
    return TRUE;
}

int fn_0055A4A0(PCode* pc) {
    PCode* next;
    PCode* emitted;
    SInt32 cls;
    SInt32 reg;
    next = pc->args[0].data.reg.definer;
    if (next == NULL || next->op != 0x264) {
        return FALSE;
    }
    if (definedbetween(pc, next, &next->args[1])) {
        return FALSE;
    }
    cls = (SInt8)pc->args[0].arg;
    reg = pc->args[0].data.reg.reg;
    if ((register_live_bitmaps_6368D0[cls].current[reg >> 5] >> (reg & 31)) & 1) {
        return FALSE;
    }
    emitted = makepcode(0x2AF, 0xF, (int)pc->args[1].data.reg.reg, (int)next->args[1].data.reg.reg);
    emitted->args[1].effect &= 2;
    insertpcodebefore(pc, emitted);
    emitted->flags |= pc->flags & 0x100;
    deletepcode(pc);
    return TRUE;
}

int fn_0055A550(PCode* pc) {
    PCode* end;
    if (copts.inlinelevel < 0 || !copts.enable_vu0_registers) {
        return FALSE;
    }
    end = pc->args[0].data.reg.definer;
    if (end != NULL && !(end->flags & 0x40002)) {
        return FALSE;
    }
    if (end->op != 0x1D1 && end->op != 0x467) {
        return FALSE;
    }
    if (definedbetween(pc, end, &pc->args[1])) {
        return FALSE;
    }
    if (usedbetween(pc, end, &pc->args[1])) {
        return FALSE;
    }
    if (usedbetween(pc, end, &end->args[0])) {
        return FALSE;
    }
    if ((register_live_bitmaps_6368D0[(SInt8)pc->args[0].arg]
             .current[pc->args[0].data.reg.reg >> 5] >>
         (pc->args[0].data.reg.reg & 31)) &
        1) {
        return FALSE;
    }
    if (end->op == 0x1D1) {
        UInt32 saved = end->flags & 0x180E0;
        UInt32 flags;
        end->op = 0x263;
        flags = opcode_info[0x263].flags;
        end->flags = flags | saved;
    } else if (end->op == 0x467) {
        UInt32 saved = end->flags & 0x180E0;
        UInt32 flags;
        end->op = 0x468;
        flags = opcode_info[0x468].flags;
        end->flags = flags | saved;
    } else {
        return FALSE;
    }
    end->args[0].data.reg.definer = pc->args[1].data.reg.definer;
    end->args[0] = pc->args[1];
    end->args[0].effect &= 2;
    end->flags |= pc->flags & 0x100;
    deletepcode(pc);
    return TRUE;
}

int fn_0055A6B0(PCode* pc) {
    PCode* end;
    PCode* scan;
    PCode* emitted;
    if (copts.inlinelevel < 0 || !copts.enable_vu0_registers) {
        return FALSE;
    }
    if (pc->flags & 0x100) {
        return FALSE;
    }
    end = pc->prevPCode;
    if (end == NULL) {
        goto done;
    }
    for (;;) {
        if ((end->flags & 0x40004) && end->op == 0x268) {
            break;
        }
        if (end == NULL) {
            goto done;
        }
        end = end->prevPCode;
        if (end == NULL) {
            goto done;
        }
    }
    if (pc->args[1].data.mem.offset != end->args[1].data.mem.offset) {
        return FALSE;
    }
    if (pc->args[2].kind == 0 && pc->args[2].data.reg.reg != end->args[2].data.reg.reg) {
        return FALSE;
    }
    scan = pc->prevPCode;
    for (; scan != end; scan = scan->prevPCode) {
        if ((scan->flags & 0x60002) &&
            (scan->op == 0x263 || scan->op == 0x468 || scan->op == 0x1D1 || scan->op == 0x467) &&
            may_alias(pc, scan)) {
            return FALSE;
        }
    }
    if (end->flags & 0x80) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[0])) {
        return FALSE;
    }
    if (definedbetween(pc, end, &pc->args[2])) {
        return FALSE;
    }
    emitted = makepcode(0x2AF, 0xF, (int)pc->args[0].data.reg.reg, (int)end->args[0].data.reg.reg);
    insertpcodebefore(end, emitted);
    fn_0055A6B0(end);
done:
    if (end != NULL) {
        deletepcode(end);
    }
    return FALSE;
}

int fn_0055A810(PCode* pc) {
    PCode* scan;
    PCode* end = pc->prevPCode;
    PCode* emitted;
    if (copts.inlinelevel < 0 || !copts.enable_vu0_registers) {
        return FALSE;
    }
    if (pc->flags & 0x100) {
        return FALSE;
    }
    scan = end;
    while (end != NULL) {
        if ((end->flags & 0x40024) && end->op == 0x268) {
            break;
        }
        end = end->prevPCode;
    }
    if (end == NULL) {
        return FALSE;
    }
    if (end->args[1].data.mem.offset != pc->args[1].data.mem.offset) {
        return FALSE;
    }
    if (pc->args[2].kind == 0 && pc->args[2].data.reg.reg != end->args[2].data.reg.reg) {
        return FALSE;
    }
    for (; scan != end; scan = scan->prevPCode) {
        if ((scan->flags & 0x60004) && may_alias(pc, scan)) {
            return FALSE;
        }
    }
    if (end->flags & 0x80) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[0])) {
        return FALSE;
    }
    if (definedbetween(pc, end, &pc->args[2])) {
        return FALSE;
    }
    emitted = makepcode(0x2AF, 0xF, (int)pc->args[0].data.reg.reg, (int)end->args[0].data.reg.reg);
    insertpcodebefore(pc, emitted);
    deletepcode(pc);
    return FALSE;
}

void fn_0055A9A0(PCode* pc) {
    fn_0055AED0(pc, 0xED, 0x1DD, 0x1DE);
}

void fn_0055A9C0(PCode* pc) {
    fn_0055B330(pc, 0xB5, 0x1E4, 0x1E2);
}

void fn_0055A9E0(PCode* pc) {
    fn_0055B330(pc, 0xB5, 0x1E4, 0x1DE);
}

int fn_0055AA00(PCode* pc) {
    return fn_0055B800(pc, 0x1D6, mul3_opcode, cg_option_00637326, 0);
}

int fn_0055AA30(PCode* pc) {
    return fn_0055B800(pc, 0x1D6, mul3_opcode, cg_option_00637326, 0);
}

void fn_0055AA60(PCode* pc) {
    fn_0055BB50(pc, 0x1D8, 0x202, 0x203, 1);
}

int fn_0055AA80(PCode* pc) {
    return fn_0055BB50(pc, 0x1D6, mul3_opcode, cg_option_00637326, 0);
}

int fn_00559800(PCode* candidate, PCode* current) {
    UInt16 op = candidate->op;
    if (op == 0x489 && current->op == 0x467) {
        return 2;
    }
    if (op == 0x1D2 && current->op == 0x1D1) {
        return 2;
    }
    if (op == 0x479 && current->op == 0x454) {
        return 1;
    }
    if (op == 0xC4 && current->op == 0x93) {
        return 1;
    }
    if (op == 0xC5 && current->op == 0x96) {
        return 4;
    }
    if (op == 0xCD && current->op == 0xA1) {
        return 3;
    }
    if (op == 0x47A && current->op == 0x455) {
        return 4;
    }
    if (op == 0x483 && current->op == 0x460) {
        return 3;
    }
    return 0;
}

int fn_005596F0(PCode* pc) {
    PCode* previous = pc->prevPCode;
    int kind;
    if (previous == NULL) {
        return FALSE;
    }
    if (previous->block == NULL) {
        return FALSE;
    }
    if (!(previous->flags & 4)) {
        return FALSE;
    }
    kind = fn_00559800(previous, pc);
    if (kind == 0) {
        return FALSE;
    }
    if (pc->args[2].data.reg.reg != previous->args[2].data.reg.reg) {
        return FALSE;
    }
    if (pc->args[1].data.mem.obj == previous->args[1].data.mem.obj) {
        if (pc->args[1].data.mem.offset == previous->args[1].data.mem.offset) {
            if (kind == 1) {
                pc->op = move_opcode;
            } else if (kind == 2) {
                pc->op = move_opcode_637350;
            } else if (kind == 3) {
                pc->op = 0x4A3;
            } else if (kind == 4) {
                pc->op = 0x4A4;
            }
            pc->args[1].data.reg.definer = previous->args[0].data.reg.definer;
            pc->args[1] = previous->args[0];
            pc->flags &= ~2;
            pc->flags |= 0x10;
            pc->argCount = 2;
            pc->unk38 = 0;
            return TRUE;
        }
    }
    return FALSE;
}

int fn_005598D0(PCode* pc) {
    PCode* candidate = pc->args[0].data.reg.definer;
    SInt32 cls;
    SInt32 reg;
    if (candidate->block == NULL) {
        return FALSE;
    }
    if (candidate->op != 0x48B && candidate->op != 0x48C) {
        return FALSE;
    }
    if (candidate->args[2].data.imm.value != 16) {
        return FALSE;
    }
    if (candidate->args[0].data.reg.reg != candidate->args[1].data.reg.reg) {
        return FALSE;
    }
    cls = (SInt8)pc->args[0].arg;
    reg = pc->args[0].data.reg.reg;
    if ((register_live_bitmaps_6368D0[cls].current[reg >> 5] >> reg) & 1) {
        return FALSE;
    }
    pc->flags |= candidate->flags & 0x100;
    deletepcode(candidate);
    return TRUE;
}

int fn_00559A40(PCode* pc) {
    PCode* candidate = pc->args[0].data.reg.definer;
    SInt32 cls;
    SInt32 reg;
    if (candidate->block == NULL) {
        return FALSE;
    }
    if (candidate->op != 0x48B && candidate->op != 0x48C) {
        return FALSE;
    }
    if (candidate->args[2].data.imm.value != 8) {
        return FALSE;
    }
    if (candidate->args[0].data.reg.reg != candidate->args[1].data.reg.reg) {
        return FALSE;
    }
    cls = (SInt8)pc->args[0].arg;
    reg = pc->args[0].data.reg.reg;
    if ((register_live_bitmaps_6368D0[cls].current[reg >> 5] >> reg) & 1) {
        return FALSE;
    }
    pc->flags |= candidate->flags & 0x100;
    deletepcode(candidate);
    return TRUE;
}

int fn_00559970(PCode* pc) {
    PCode* candidate = pc->args[0].data.reg.definer;
    SInt32 cls;
    SInt32 reg;
    if (candidate->block == NULL) {
        return FALSE;
    }
    if (candidate->op != 7 && candidate->op != 0x40C) {
        return FALSE;
    }
    if (!CInt64_Equal(*(CInt64*)&candidate->args[2].data, peephole_const_5E614C)) {
        return FALSE;
    }
    if (candidate->args[0].data.reg.reg != candidate->args[1].data.reg.reg) {
        return FALSE;
    }
    cls = (SInt8)pc->args[0].arg;
    reg = pc->args[0].data.reg.reg;
    if ((register_live_bitmaps_6368D0[cls].current[reg >> 5] >> reg) & 1) {
        return FALSE;
    }
    pc->flags |= candidate->flags & 0x100;
    deletepcode(candidate);
    return TRUE;
}

int fn_00559AE0(PCode* pc) {
    PCode* candidate = pc->args[0].data.reg.definer;
    SInt32 cls;
    SInt32 reg;
    if (candidate->block == NULL) {
        return FALSE;
    }
    if (candidate->op != 7 && candidate->op != 0x40C) {
        return FALSE;
    }
    if (!CInt64_Equal(*(CInt64*)&candidate->args[2].data, peephole_const_5E613C)) {
        return FALSE;
    }
    if (candidate->args[0].data.reg.reg != candidate->args[1].data.reg.reg) {
        return FALSE;
    }
    cls = (SInt8)pc->args[0].arg;
    reg = pc->args[0].data.reg.reg;
    if ((register_live_bitmaps_6368D0[cls].current[reg >> 5] >> reg) & 1) {
        return FALSE;
    }
    pc->flags |= candidate->flags & 0x100;
    deletepcode(candidate);
    return TRUE;
}

int fn_0055C060(PCode* pc) {
    PCode* end = pc->args[2].data.reg.definer;
    if (end->op != (UInt16)move_opcode) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[1])) {
        return FALSE;
    }
    pc->args[2].data.reg.definer = end->args[1].data.reg.definer;
    pc->args[2] = end->args[1];
    return TRUE;
}

int fn_0055C250(PCode* pc) {
    PCode* end = pc->args[0].data.reg.definer;
    if (end->op != 7 && end->op != 0x40C) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[1])) {
        return FALSE;
    }
    if (!CInt64_Equal(end->args[2].data.cint, peephole_const_5E614C)) {
        return FALSE;
    }
    pc->args[0].data.reg.definer = end->args[1].data.reg.definer;
    pc->args[0] = end->args[1];
    return TRUE;
}

int fn_0055C300(PCode* pc) {
    PCode* end = pc->args[0].data.reg.definer;
    if (end->op != 0x48C && end->op != 0x48B) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[1])) {
        return FALSE;
    }
    if (end->args[2].data.imm.value != 16) {
        return FALSE;
    }
    if (end->args[0].data.reg.reg == end->args[1].data.reg.reg) {
        end->op = move_opcode;
        end->flags |= 0x10;
        end->argCount = 2;
        return TRUE;
    }
    pc->args[0].data.reg.definer = end->args[1].data.reg.definer;
    pc->args[0] = end->args[1];
    return TRUE;
}

int fn_0055C3B0(PCode* pc) {
    PCode* end = pc->args[0].data.reg.definer;
    if (end->op != 7 && end->op != 0x40C) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[1])) {
        return FALSE;
    }
    if (!CInt64_Equal(end->args[2].data.cint, peephole_const_5E613C)) {
        return FALSE;
    }
    pc->args[0].data.reg.definer = end->args[1].data.reg.definer;
    pc->args[0] = end->args[1];
    return TRUE;
}

int fn_0055BE00(PCode* pc) {
    switch (pc->args[1].data.reg.definer->op) {
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
        case 0x48D:
        case 0x48E:
        case 0x48F:
        case 0x490:
        case 0x491:
        case 0x492:
        case 0x493:
        case 0x494:
        case 0x495:
        case 0x496:
        case 0x497:
        case 0x498:
        case 0x499:
        case 0x49A:
        case 0x49B:
        case 0x49C:
        case 0x49D:
        case 0x49E:
            break;
        default:
            return FALSE;
    }
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    return TRUE;
}

int fn_0055BE40(PCode* pc) {
    PCode* next = pc->args[1].data.reg.definer;
    if (pc->args[2].data.imm.value != 0x20) {
        return FALSE;
    }
    switch (next->op) {
        case 0x88:
            if (next->args[2].data.imm.value != 0) {
                return FALSE;
            }
            break;
        case 0xD2:
        case 0xD3:
        case 0x189:
        case 0x18A:
        case 0x18D:
        case 0x18E:
            break;
        default:
            return FALSE;
    }
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    return TRUE;
}

int fn_0055BEB0(PCode* pc) {
    if (pc->args[2].data.reg.reg != 0) {
        return FALSE;
    }
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    return TRUE;
}

UInt8 fn_0055BDA0(PCode** first, PCode** second, int change) {
    PCode* current;
    PCode* start;
    PCodeBlock* block;
    PCode* original;
    original = *first;
    block = original->block;
    if (block == NULL) {
        return FALSE;
    }
    current = block->firstPCode;
    if (current != NULL) {
        start = original;
        do {
            if (current == start) {
                return TRUE;
            }
            if (current == *second) {
                if (!change) {
                    return FALSE;
                }
                *second = original;
                *first = current;
                return TRUE;
            }
            current = current->nextPCode;
        } while (current != NULL);
    }
    return FALSE;
}

int fn_0055C0D0(PCode* pc) {
    PCode* end = pc->args[2].data.reg.definer;
    SInt32 combined;

    if (end->op != 3) {
        return FALSE;
    }
    if (end->args[1].arg == 0 && end->args[1].data.reg.reg == 0) {
        return FALSE;
    }
    if (pc->unk38 != 0 || end->unk38 != 0) {
        return FALSE;
    }
    if (end->args[0].data.reg.reg == end->args[1].data.reg.reg) {
        return FALSE;
    }
    if (writtensince(pc, end, &end->args[1])) {
        return FALSE;
    }
    if (!CInt64_IsInRange(end->args[2].data.cint, 2)) {
        return FALSE;
    }
    combined = end->args[2].data.cint.lo + pc->args[1].data.mem.offset;
    if (combined > 32767 || combined < -32768) {
        return FALSE;
    }
    pc->args[2].data.reg.definer = end->args[1].data.reg.definer;
    pc->args[2] = end->args[1];
    pc->args[1].data.mem.offset = combined;
    return TRUE;
}

int fn_0055BEE0(PCode* pc) {
    PCode* end = pc->args[1].data.reg.definer;
    int count;
    PCodeArg* arg;
    UInt8 blocked;
    UInt8 blocked2;
    int count2;
    PCodeArg* arg2;
    if (end->op != 0xBE) {
        return FALSE;
    }
    if (pc->args[2].data.reg.reg != 0) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[1])) {
        blocked = TRUE;
    } else {
        count = end->argCount;
        arg = end->args;
        while (count--) {
            if (arg->kind == end->args[1].kind &&
                arg->data.reg.reg == end->args[1].data.reg.reg &&
                arg->arg == end->args[1].arg &&
                (arg->effect & 2)) {
                blocked = TRUE;
                goto scan1_done;
            }
            arg++;
        }
        blocked = FALSE;
    }
scan1_done:
    if (blocked) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[2])) {
        blocked2 = TRUE;
    } else {
        count2 = end->argCount;
        arg2 = end->args;
        while (count2--) {
            if (arg2->kind == end->args[2].kind &&
                arg2->data.reg.reg == end->args[2].data.reg.reg &&
                arg2->arg == end->args[2].arg &&
                (arg2->effect & 2)) {
                blocked2 = TRUE;
                goto scan2_done;
            }
            arg2++;
        }
        blocked2 = FALSE;
    }
scan2_done:
    if (blocked2) {
        return FALSE;
    }
    pc->args[1].data.reg.definer = end->args[1].data.reg.definer;
    pc->args[1] = end->args[1];
    pc->args[2].data.reg.definer = end->args[2].data.reg.definer;
    pc->args[2] = end->args[2];
    return TRUE;
}

int fn_00559BB0(PCode* pc) {
    PCode* scan;
    PCode* candidate;
    int currentKind;
    int priorKind;
    if (pc->flags & 0x20) {
        return FALSE;
    }
    scan = pc->prevPCode;
    candidate = scan;
    while (candidate != NULL &&
           (!(candidate->flags & 0x40004) ||
            (candidate->flags & 0x20) ||
            candidate->alias != pc->alias)) {
        candidate = candidate->prevPCode;
    }
    if (candidate == NULL) {
        return FALSE;
    }
    if (pc->args[1].data.mem.offset != candidate->args[1].data.mem.offset) {
        return FALSE;
    }
    if (pc->args[2].kind == 0 && pc->args[2].data.reg.reg != candidate->args[2].data.reg.reg) {
        return FALSE;
    }
    if (pc->args[2].kind == 0 &&
        pc->args[2].data.reg.reg == candidate->args[2].data.reg.reg &&
        pc->args[2].data.reg.definer != candidate->args[2].data.reg.definer) {
        return FALSE;
    }
    for (; scan != candidate; scan = scan->prevPCode) {
        if (!(scan->flags & 0x60002)) {
            if ((scan->flags & 8) && may_alias(pc, scan)) {
                return FALSE;
            }
        } else {
            if (may_alias(pc, scan)) {
                return FALSE;
            }
        }
    }
    if (candidate->flags & 0x80) {
        return FALSE;
    }
    currentKind = nbytes_loaded_or_stored_by(pc->op);
    priorKind = nbytes_loaded_or_stored_by(candidate->op);
    if (priorKind <= currentKind) {
        pc->flags |= candidate->flags & 0x100;
        deletepcode(candidate);
        return TRUE;
    }
    return FALSE;
}

int fn_005587A0(PCode* pc) {
    PCode* end = pc->args[1].data.reg.definer;
    int used;
    UInt16 op = end->op;
    if (op == 0x10E) {
        used = usedbetween(pc, end, &end->args[0]);
        if (used) {
            return FALSE;
        }
        {
            UInt32 saved = end->flags;
            end->op = 0x11E;
            saved &= 0x180E0;
            end->flags = saved | opcode_info[0x11E].flags;
            end->args[0].data.reg.definer = pc->args[0].data.reg.definer;
            end->args[0] = pc->args[0];
            {
                UInt32 savedPC = pc->flags & 0x180E0;
                pc->op = 0xBA;
                pc->flags = opcode_info[0xBA].flags | savedPC;
            }
            pc->argCount = 0;
        }
        goto success;
    }
    if (op == 0x11C) {
        used = usedbetween(pc, end, &end->args[0]);
        if (used) {
            return FALSE;
        }
        {
            UInt32 saved = end->flags;
            end->op = 0x120;
            saved &= 0x180E0;
            end->flags = saved | opcode_info[0x120].flags;
            end->args[0].data.reg.definer = pc->args[0].data.reg.definer;
            end->args[0] = pc->args[0];
            {
                UInt32 savedPC = pc->flags & 0x180E0;
                pc->op = 0xBA;
                pc->flags = opcode_info[0xBA].flags | savedPC;
            }
            pc->argCount = 0;
        }
        goto success;
    }
    if (op == 0x11E) {
        used = usedbetween(pc, end, &end->args[0]);
        if (used) {
            return FALSE;
        }
        {
            UInt32 saved = end->flags;
            end->op = 0x10E;
            saved &= 0x180E0;
            end->flags = saved | opcode_info[0x10E].flags;
            end->args[0].data.reg.definer = pc->args[0].data.reg.definer;
            end->args[0] = pc->args[0];
            {
                UInt32 savedPC = pc->flags & 0x180E0;
                pc->op = 0xBA;
                pc->flags = opcode_info[0xBA].flags | savedPC;
            }
            pc->argCount = 0;
        }
        goto success;
    }
    if (op == 0x120) {
        used = usedbetween(pc, end, &end->args[0]);
        if (used) {
            return FALSE;
        }
        {
            UInt32 saved = end->flags;
            end->op = 0x11C;
            saved &= 0x180E0;
            end->flags = saved | opcode_info[0x11C].flags;
            end->args[0].data.reg.definer = pc->args[0].data.reg.definer;
            end->args[0] = pc->args[0];
            {
                UInt32 savedPC = pc->flags & 0x180E0;
                pc->op = 0xBA;
                pc->flags = opcode_info[0xBA].flags | savedPC;
            }
            pc->argCount = 0;
        }
        goto success;
    }
    return FALSE;
success:
    return TRUE;
}

int fn_005589E0(PCode* pc) {
    PCode* end = pc->args[1].data.reg.definer;
    int used;
    UInt16 op = end->op;
    if (op == 0x10F) {
        used = usedbetween(pc, end, &end->args[0]);
        if (used) {
            return FALSE;
        }
        {
            UInt32 saved = end->flags;
            end->op = 0x11F;
            saved &= 0x180E0;
            end->flags = saved | opcode_info[0x11F].flags;
            end->args[0].data.reg.definer = pc->args[0].data.reg.definer;
            end->args[0] = pc->args[0];
            {
                UInt32 savedPC = pc->flags & 0x180E0;
                pc->op = 0xBA;
                pc->flags = opcode_info[0xBA].flags | savedPC;
            }
            pc->argCount = 0;
        }
        goto success;
    }
    if (op == 0x11D) {
        used = usedbetween(pc, end, &end->args[0]);
        if (used) {
            return FALSE;
        }
        {
            UInt32 saved = end->flags;
            end->op = 0x121;
            saved &= 0x180E0;
            end->flags = saved | opcode_info[0x121].flags;
            end->args[0].data.reg.definer = pc->args[0].data.reg.definer;
            end->args[0] = pc->args[0];
            {
                UInt32 savedPC = pc->flags & 0x180E0;
                pc->op = 0xBA;
                pc->flags = opcode_info[0xBA].flags | savedPC;
            }
            pc->argCount = 0;
        }
        goto success;
    }
    if (op == 0x11F) {
        used = usedbetween(pc, end, &end->args[0]);
        if (used) {
            return FALSE;
        }
        {
            UInt32 saved = end->flags;
            end->op = 0x10F;
            saved &= 0x180E0;
            end->flags = saved | opcode_info[0x10F].flags;
            end->args[0].data.reg.definer = pc->args[0].data.reg.definer;
            end->args[0] = pc->args[0];
            {
                UInt32 savedPC = pc->flags & 0x180E0;
                pc->op = 0xBA;
                pc->flags = opcode_info[0xBA].flags | savedPC;
            }
            pc->argCount = 0;
        }
        goto success;
    }
    if (op == 0x121) {
        used = usedbetween(pc, end, &end->args[0]);
        if (used) {
            return FALSE;
        }
        {
            UInt32 saved = end->flags;
            end->op = 0x11D;
            saved &= 0x180E0;
            end->flags = saved | opcode_info[0x11D].flags;
            end->args[0].data.reg.definer = pc->args[0].data.reg.definer;
            end->args[0] = pc->args[0];
            {
                UInt32 savedPC = pc->flags & 0x180E0;
                pc->op = 0xBA;
                pc->flags = opcode_info[0xBA].flags | savedPC;
            }
            pc->argCount = 0;
        }
        goto success;
    }
    return FALSE;
success:
    return TRUE;
}

UInt8 fn_005594E0(PCode* pc) {
    SInt32 changed = FALSE;
    volatile SInt16 kind = nbytes_loaded_or_stored_by(pc->op);
    PCode* cursor;
    PCodeArg* arg;
    int count;
    PCode* next;
    UInt32 flags;
    if (pc->flags & 0x180) {
        return FALSE;
    }
    cursor = pc->nextPCode;
    while (cursor != NULL) {
        flags = cursor->flags;
        next = cursor;
        if (flags & 0x180) {
            return FALSE;
        }
        if (!(flags & 2)) {
            if ((flags & 4) &&
                (cursor->args[1].data.mem.obj == pc->args[1].data.mem.obj ||
                 may_alias(cursor, pc))) {
                return FALSE;
            }
        } else if (cursor->args[2].data.reg.reg == pc->args[2].data.reg.reg &&
                   cursor->args[1].kind == 4 &&
                   cursor->args[1].data.mem.obj == pc->args[1].data.mem.obj &&
                   cursor->args[1].data.mem.offset == pc->args[1].data.mem.offset) {
            int replacement = can_reuse_stored_value(pc, cursor);
            if (replacement) {
                if (kind == 1) {
                    next = makepcode(
                        0x48B, (int)cursor->args[0].data.reg.reg, (int)pc->args[0].data.reg.reg, 8
                    );
                } else if (kind == 2) {
                    next = makepcode(
                        0x48B,
                        (int)cursor->args[0].data.reg.reg,
                        (int)pc->args[0].data.reg.reg,
                        0x10
                    );
                } else {
                    next = makecopyforload(replacement, &pc->args[0], &cursor->args[0]);
                }
                insertpcodebefore(cursor, next);
                deletepcode(cursor);
                changed = TRUE;
            }
        }
        count = next->argCount;
        arg = next->args;
        while (count--) {
            if (arg->kind == 0 && (arg->effect & 2)) {
                if (arg->arg == 0 &&
                    (arg->data.reg.reg == pc->args[1].data.reg.reg ||
                     arg->data.reg.reg == pc->args[2].data.reg.reg)) {
                    return FALSE;
                }
                if (arg->arg == pc->args[0].arg && arg->data.reg.reg == pc->args[0].data.reg.reg) {
                    return FALSE;
                }
            }
            arg++;
        }
        cursor = next->nextPCode;
    }
    return changed;
}

int fn_00558610(PCode* pc, UInt16 opcode, int argIndex, PCode* candidate) {
    PCodeArg* arg;
    int count;
    SInt16 reg;
    int cls;
    PCode* emitted;
    UInt8 blocked;
    int count2;
    PCodeArg* arg2;
    if (definedbetween(pc, candidate, &candidate->args[1])) {
        blocked = TRUE;
    } else {
        blocked = FALSE;
        count = candidate->argCount;
        arg = candidate->args;
        while (count--) {
            if (arg->kind == candidate->args[1].kind &&
                arg->data.reg.reg == candidate->args[1].data.reg.reg &&
                arg->arg == candidate->args[1].arg &&
                (arg->effect & 2)) {
                blocked = TRUE;
                break;
            }
            arg++;
        }
    }
    if (blocked) {
        return FALSE;
    }
    if (definedbetween(pc, candidate, &candidate->args[2])) {
        blocked = TRUE;
    } else {
        blocked = FALSE;
        count2 = candidate->argCount;
        arg2 = candidate->args;
        while (count2--) {
            if (arg2->kind == candidate->args[2].kind &&
                arg2->data.reg.reg == candidate->args[2].data.reg.reg &&
                arg2->arg == candidate->args[2].arg &&
                (arg2->effect & 2)) {
                blocked = TRUE;
                break;
            }
            arg2++;
        }
    }
    if (blocked) {
        return FALSE;
    }
    reg = candidate->args[1].data.reg.reg;
    cls = (SInt8)candidate->args[1].arg;
    register_live_bitmaps_6368D0[cls].current[reg >> 5] |= 1u << (reg & 31);
    reg = candidate->args[2].data.reg.reg;
    cls = (SInt8)candidate->args[2].arg;
    register_live_bitmaps_6368D0[cls].current[reg >> 5] |= 1u << (reg & 31);
    emitted = makepcode(
        opcode,
        (int)pc->args[0].data.reg.reg,
        (int)pc->args[argIndex].data.reg.reg,
        (int)candidate->args[1].data.reg.reg,
        (int)candidate->args[2].data.reg.reg
    );
    insertpcodebefore(pc, emitted);
    pc->op = 0xBA;
    pc->flags = opcode_info[0xBA].flags | (pc->flags & 0x180E0);
    pc->argCount = 0;
    return TRUE;
}

int fn_00559D80(PCode* pc, UInt16 opcode) {
    PCode* end = pc->args[2].data.reg.definer;
    PCodeArg* arg;
    int count;
    SInt16 reg;
    int cls;
    UInt8 blocked;
    int count2;
    PCodeArg* arg2;
    if (end->op != 4) {
        return FALSE;
    }
    if (pc->unk38 != 0) {
        return FALSE;
    }
    if (pc->args[1].data.mem.offset != 0) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[1])) {
        blocked = TRUE;
    } else {
        blocked = FALSE;
        count = end->argCount;
        arg = end->args;
        while (count--) {
            if (arg->kind == end->args[1].kind &&
                arg->data.reg.reg == end->args[1].data.reg.reg &&
                arg->arg == end->args[1].arg &&
                (arg->effect & 2)) {
                blocked = TRUE;
                break;
            }
            arg++;
        }
    }
    if (blocked) {
        return FALSE;
    }
    if (definedbetween(pc, end, &end->args[2])) {
        blocked = TRUE;
    } else {
        blocked = FALSE;
        count2 = end->argCount;
        arg2 = end->args;
        while (count2--) {
            if (arg2->kind == end->args[2].kind &&
                arg2->data.reg.reg == end->args[2].data.reg.reg &&
                arg2->arg == end->args[2].arg &&
                (arg2->effect & 2)) {
                blocked = TRUE;
                break;
            }
            arg2++;
        }
    }
    if (blocked) {
        return FALSE;
    }
    pc->op = opcode;
    pc->flags = opcode_info[opcode].flags | (pc->flags & 0x180E0);
    reg = end->args[1].data.reg.reg;
    cls = (SInt8)end->args[1].arg;
    register_live_bitmaps_6368D0[cls].current[reg >> 5] |= 1u << (reg & 31);
    reg = end->args[2].data.reg.reg;
    cls = (SInt8)end->args[2].arg;
    register_live_bitmaps_6368D0[cls].current[reg >> 5] |= 1u << (reg & 31);
    pc->args[1].data.reg.definer = end->args[1].data.reg.definer;
    pc->args[1] = end->args[1];
    pc->args[2].data.reg.definer = end->args[2].data.reg.definer;
    pc->args[2] = end->args[2];
    return TRUE;
}

void fn_00557180(void) {
    installpattern(peephole_opcode_637054, fn_0055C610);
    installpattern(peephole_opcode_636FCC, fn_0055C510);
    installpattern(0x96, fn_005596F0);
    installpattern(0x455, fn_005596F0);
    installpattern(0x460, fn_005596F0);
    installpattern(0xa1, fn_005596F0);
    installpattern(0xcb, fn_005594E0);
    installpattern(0x481, fn_005594E0);
    installpattern(0xca, fn_005594E0);
    installpattern(0x480, fn_005594E0);
    installpattern(0xc3, fn_005594E0);
    installpattern(0x47f, fn_005594E0);
    if (coloring != 0) {
        installpattern(0x48b, fn_0055BE00);
        installpattern(0x48b, fn_0055C8B0);
        installpattern(0x48b, fn_0055D160);
        installpattern(0x48c, fn_0055BE00);
        installpattern(0x48c, fn_0055C8B0);
        installpattern(0x48c, fn_0055D220);
        installpattern(0x48c, fn_0055C850);
        installpattern(0xd2, fn_0055D590);
        installpattern(0xd2, fn_0055D070);
        installpattern(0xd8, fn_0055D480);
        installpattern(7, fn_0055D380);
        installpattern(0x40c, fn_0055D380);
        installpattern(0x47f, fn_0055C460);
        installpattern(0xc3, fn_0055C460);
        installpattern(0x47f, fn_0055C3B0);
        installpattern(0xc3, fn_0055C3B0);
        installpattern(0x480, fn_0055C300);
        installpattern(0xca, fn_0055C300);
        installpattern(0x480, fn_0055C250);
        installpattern(0xca, fn_0055C250);
        installpattern(0xbe, fn_0055BEB0);
        installpattern(0xbc, fn_0055BEE0);
        installpattern(0xe5, fn_0055C720);
        installpattern(0x4a1, fn_0055C720);
        installpattern(0x45a, fn_0055C0D0);
        installpattern(0x453, fn_0055C0D0);
        installpattern(0x45b, fn_0055C0D0);
        installpattern(0x45c, fn_0055C0D0);
        installpattern(0x45f, fn_0055C0D0);
        installpattern(0x466, fn_0055C0D0);
        installpattern(0x454, fn_0055C0D0);
        installpattern(0x460, fn_0055C0D0);
        installpattern(0x455, fn_0055C0D0);
        installpattern(0x452, fn_0055C0D0);
        installpattern(0x47f, fn_0055C0D0);
        installpattern(0x480, fn_0055C0D0);
        installpattern(0x481, fn_0055C0D0);
        installpattern(0x479, fn_0055C0D0);
        installpattern(0x483, fn_0055C0D0);
        installpattern(0x47a, fn_0055C0D0);
        installpattern(0x476, fn_0055C0D0);
        installpattern(0x91, fn_0055C0D0);
        installpattern(0x92, fn_0055C0D0);
        installpattern(0x94, fn_0055C0D0);
        installpattern(0x95, fn_0055C0D0);
        installpattern(0x9d, fn_0055C0D0);
        installpattern(0xa0, fn_0055C0D0);
        installpattern(0x93, fn_0055C0D0);
        installpattern(0xa1, fn_0055C0D0);
        installpattern(0x96, fn_0055C0D0);
        installpattern(0xc3, fn_0055C0D0);
        installpattern(0xca, fn_0055C0D0);
        installpattern(0xcb, fn_0055C0D0);
        installpattern(0xc4, fn_0055C0D0);
        installpattern(0xcd, fn_0055C0D0);
        installpattern(0xc5, fn_0055C0D0);
        installpattern(0x45a, fn_0055C060);
        installpattern(0x453, fn_0055C060);
        installpattern(0x45b, fn_0055C060);
        installpattern(0x45c, fn_0055C060);
        installpattern(0x45f, fn_0055C060);
        installpattern(0x466, fn_0055C060);
        installpattern(0x454, fn_0055C060);
        installpattern(0x460, fn_0055C060);
        installpattern(0x455, fn_0055C060);
        installpattern(0x452, fn_0055C060);
        installpattern(0x47f, fn_0055C060);
        installpattern(0x480, fn_0055C060);
        installpattern(0x481, fn_0055C060);
        installpattern(0x479, fn_0055C060);
        installpattern(0x483, fn_0055C060);
        installpattern(0x47a, fn_0055C060);
        installpattern(0x476, fn_0055C060);
        installpattern(0x483, fn_00559250);
        installpattern(0xcd, fn_005591D0);
        installpattern(0x47f, fn_00559BB0);
        installpattern(0xc3, fn_00559BB0);
        installpattern(0x480, fn_00559BB0);
        installpattern(0xca, fn_00559BB0);
        installpattern(0x481, fn_00559BB0);
        installpattern(0xcb, fn_00559BB0);
        installpattern(0x479, fn_00559BB0);
        installpattern(0xc4, fn_00559BB0);
        installpattern(0x48b, fn_0055BE40);
        installpattern(0xa5, fn_00558CE0);
        installpattern(0xad, fn_00558C20);
        installpattern(0xad, fn_00559300);
        installpattern(0xa5, fn_00558EA0);
        installpattern(1, fn_0055AA80);
        installpattern(4, fn_0055AA80);
        installpattern(1, fn_00559FB0);
        installpattern(4, fn_00559FB0);
        installpattern(1, fn_0055AA60);
        installpattern(4, fn_0055AA60);
        installpattern(1, fn_00559F80);
        installpattern(4, fn_00559F80);
        installpattern(1, fn_0055AA30);
        installpattern(4, fn_0055AA00);
        installpattern(0x1d2, fn_00559BB0);
        installpattern(0x489, fn_00559BB0);
        installpattern(0x266, fn_0055A550);
        installpattern(0x268, fn_0055A300);
        installpattern(0x1d2, fn_0055A240);
        installpattern(0x489, fn_0055A240);
        installpattern(0x268, fn_00559BB0);
        installpattern(0x48a, fn_00559BB0);
        installpattern(0x266, fn_0055A4A0);
        installpattern(0x264, fn_0055A400);
        if (copts.usefloatacc != '\0') {
            installpattern(0xed, fn_0055A9E0);
            installpattern(0xed, fn_0055A220);
            installpattern(0xed, fn_0055A1E0);
            installpattern(0xed, fn_0055A9A0);
            installpattern(0xed, fn_0055A980);
            installpattern(0x103, fn_0055A9C0);
            installpattern(0x103, fn_0055A200);
            installpattern(0x103, fn_0055A1C0);
            installpattern(0x103, fn_0055A960);
            installpattern(0x103, fn_0055A940);
            installpattern(0x1dd, fn_00558170);
            installpattern(0x1dd, fn_00557DD0);
        }
        installpattern(0x1d1, fn_005596F0);
        installpattern(0x467, fn_005596F0);
        installpattern(0x467, fn_0055C0D0);
        installpattern(0x489, fn_0055C0D0);
        installpattern(0x1d1, fn_0055C0D0);
        installpattern(0x1d2, fn_0055C0D0);
        installpattern(0x468, fn_0055C0D0);
        installpattern(0x48a, fn_0055C0D0);
        installpattern(0x263, fn_0055C0D0);
        installpattern(0x268, fn_0055C0D0);
        installpattern(0x263, fn_0055A810);
        installpattern(0x268, fn_0055A6B0);
        if (copts.unknown0C < 3) {
            return;
        }
        installpattern(0x47a, fn_00559150);
        installpattern(0x476, fn_00559150);
        installpattern(0xc5, fn_005590D0);
        installpattern(0x7f, fn_00558FB0);
        installpattern(0x7f, fn_00558E20);
        installpattern(0xb1, fn_00558D70);
        installpattern(0xb1, fn_005592D0);
        if (copts.unknown0C < 4) {
            return;
        }
        if (copts.unknown11 != '\0') {
            installpattern(0xfb, fn_005589E0);
            installpattern(0xfa, fn_005587A0);
            installpattern(0xed, fn_00558590);
            installpattern(0xec, fn_00558570);
            installpattern(0x103, fn_00558500);
            installpattern(0x102, fn_005584E0);
        }
        installpattern(0x460, fn_00559D60);
        installpattern(0xa1, fn_00559D60);
        installpattern(0x452, fn_00559D20);
        installpattern(0x455, fn_00559D20);
        installpattern(0x96, fn_00559D20);
        installpattern(0x483, fn_00559D40);
        installpattern(0xcd, fn_00559D40);
        installpattern(0x476, fn_00559D00);
        installpattern(0x47a, fn_00559D00);
        installpattern(0xc5, fn_00559D00);
    } else {
        installpattern(0x47f, fn_00559A40);
        installpattern(0xc3, fn_00559A40);
        installpattern(0x47f, fn_00559AE0);
        installpattern(0xc3, fn_00559AE0);
        installpattern(0x480, fn_005598D0);
        installpattern(0xca, fn_005598D0);
        installpattern(0x480, fn_00559970);
        installpattern(0xca, fn_00559970);
        installpattern(0x48c, fn_0055C8B0);
        installpattern(0x48b, fn_0055C8B0);
        installpattern(0x1d1, fn_005596F0);
        installpattern(0x467, fn_005596F0);
        installpattern(0x26e, fn_0055A180);
        installpattern(0x26f, fn_0055A180);
        installpattern(0x270, fn_0055A180);
    }
    return;
}

int fn_0055BB50(PCode* pc, int opcode, int leftOp, int rightOp, int reg) {
    PCode* first = pc->args[1].data.reg.definer;
    PCode* second = pc->args[2].data.reg.definer;
    PCode* cursor;
    PCodeArg* arg;
    int index;
    if (first->op != leftOp && first->op != rightOp) {
        return FALSE;
    }
    if (second->op != leftOp && second->op != rightOp) {
        return FALSE;
    }
    if (first == second) {
        return FALSE;
    }
    if (!fn_0055BDA0(&first, &second, pc->flags & 0x2000)) {
        return FALSE;
    }
    for (cursor = first->nextPCode; cursor != second; cursor = cursor->nextPCode) {
        if (cursor != NULL) {
            arg = cursor->args;
            for (index = 0; index < cursor->argCount; index++) {
                if (arg->kind == 0 &&
                    arg->arg == 2 &&
                    (arg->effect & 2) &&
                    arg->data.reg.reg == reg) {
                    return FALSE;
                }
                arg++;
            }
        }
    }
    if (pc->args[0].data.reg.reg != second->args[0].data.reg.reg &&
        pc->nextPCode != NULL &&
        fn_0055D680(pc->nextPCode, (PCode**)pc->block->lastPCode, &second->args[0])) {
        return FALSE;
    }
    if (pc->prevPCode != second &&
        fn_0055D680(second->nextPCode, (PCode**)pc->prevPCode, &second->args[0])) {
        return FALSE;
    }
    second->op = opcode;
    second->flags = (second->flags & 0x180E0) | opcode_info[opcode].flags;
    second->args[second->argCount - 1].effect |= 3;
    pc->args[1] = second->args[0];
    pc->args[1].effect = 1;
    pc->args[1].data.reg.definer = second;
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    return TRUE;
}

int fn_0055B800(PCode* pc, int opcode, int typeOne, int typeTwo, int mode) {
    PCode* first = pc->args[1].data.reg.definer;
    PCode* second = pc->args[2].data.reg.definer;
    PCode* other;
    PCode* base;
    PCode* left;
    PCode* right;
    PCode* special;
    PCode* cursor;
    PCodeArg* arg;
    int index;
    UInt8 blocked;
    if (first == NULL || second == NULL) {
        return FALSE;
    }
    if (first->op != 1 &&
        first->op != 4 &&
        first->op != (UInt16)typeOne &&
        first->op != (UInt16)typeTwo) {
        return FALSE;
    }
    if (second->op != 1 &&
        second->op != 4 &&
        second->op != (UInt16)typeOne &&
        second->op != (UInt16)typeTwo) {
        return FALSE;
    }
    if ((first->op == (UInt16)typeOne || first->op == (UInt16)typeTwo) &&
        second->op != 1 &&
        second->op != 4) {
        return FALSE;
    }
    if (first->op == 1 || first->op == 4) {
        base = first;
        first = second;
        if (first->op != (UInt16)typeOne && first->op != (UInt16)typeTwo) {
            return FALSE;
        }
    } else {
        base = second;
    }
    other = first;
    left = base->args[1].data.reg.definer;
    right = base->args[2].data.reg.definer;
    if (left == NULL || right == NULL) {
        return FALSE;
    }
    if (!(((left->op == 0x1DB || left->op == 0x1DC) &&
           (right->op == 0x202 || right->op == 0x203)) ||
          ((right->op == 0x1DB || right->op == 0x1DC) &&
           (left->op == 0x202 || left->op == 0x203)))) {
        return FALSE;
    }
    base = left;
    special = right;
    if (right->op != 0x1DB && right->op != 0x1DC) {
        base = right;
        special = left;
    }
    if (!fn_0055BDA0(&other, &special, pc->flags & 0x2000)) {
        return FALSE;
    }
    cursor = other->nextPCode;
    do {
        if (cursor == special) {
            blocked = FALSE;
            break;
        }
        if (cursor == NULL) {
            blocked = FALSE;
        } else {
            blocked = FALSE;
            index = 0;
            arg = cursor->args;
            if (0 < cursor->argCount) {
                do {
                    if (arg->kind == 0 &&
                        arg->arg == 2 &&
                        (arg->effect & 2) &&
                        arg->data.reg.reg == mode) {
                        blocked = TRUE;
                        break;
                    }
                    index++;
                    arg++;
                } while (index < cursor->argCount);
            }
        }
        if (blocked) {
            blocked = TRUE;
            break;
        }
        cursor = cursor->nextPCode;
    } while (TRUE);
    if (blocked) {
        return FALSE;
    }
    special->op = opcode;
    special->flags = (special->flags & 0x180E0) | opcode_info[opcode].flags;
    special->args[special->argCount - 1].effect |= 3;
    special->args[special->argCount - 1].data.reg.definer = other;
    pc->args[1].data.reg.definer = special->args[0].data.reg.definer;
    pc->args[1] = special->args[0];
    pc->args[1].effect = 1;
    pc->args[1].data.reg.definer = special;
    pc->args[2].data.reg.definer = base->args[0].data.reg.definer;
    pc->args[2] = base->args[0];
    pc->args[2].effect = 1;
    pc->args[2].data.reg.definer = base;
    return TRUE;
}

int fn_00557DD0(PCode* pc) {
    PCode* first;
    PCode* second;
    PCode* selected;
    PCode* transfer;
    PCode* cursor;
    PCodeArg* arg;
    SInt16 reg;
    int cls;
    int count;
    int used;
    selected = NULL;
    transfer = NULL;
    if (pc->flags & 0x100) {
        return FALSE;
    }
    first = pc->args[0].data.reg.definer;
    second = pc->args[1].data.reg.definer;
    if (first->op == 0x1DE) {
        selected = first;
    } else if (second->op == 0x1DE) {
        selected = second;
    } else if (first->op == 0x1E2) {
        selected = first;
    } else if (second->op == 0x1E2) {
        selected = second;
    }
    if (first->op == 0xAD) {
        transfer = first;
    } else if (second->op == 0xAD) {
        transfer = second;
    }
    if (transfer == NULL ||
        transfer->args[0].data.reg.reg != 0 ||
        transfer->args[0].arg != 0 ||
        selected == NULL) {
        return FALSE;
    }
    for (cursor = pc->prevPCode; cursor != selected; cursor = cursor->prevPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == selected->args[0].kind &&
                arg->data.reg.reg == selected->args[0].data.reg.reg &&
                arg->arg == selected->args[0].arg &&
                (arg->effect & 3)) {
                return FALSE;
            }
            arg++;
        }
    }
    reg = selected->args[0].data.reg.reg;
    cls = (SInt8)selected->args[0].arg;
    if ((register_live_bitmaps_6368D0[cls].current[reg >> 5] >> (reg & 31)) & 1) {
        return FALSE;
    }
    used = FALSE;
    for (cursor = pc->nextPCode; cursor != NULL && !used; cursor = cursor->nextPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == selected->args[0].kind &&
                arg->data.reg.reg == selected->args[0].data.reg.reg &&
                arg->arg == selected->args[0].arg) {
                if (arg->effect & 2) {
                    used = TRUE;
                    break;
                }
                if (arg->effect & 1) {
                    return FALSE;
                }
            }
            arg++;
        }
    }
    for (cursor = pc->prevPCode; cursor != selected; cursor = cursor->prevPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == 0 && arg->arg == 2 && arg->data.reg.reg == 2 && (arg->effect & 3)) {
                return FALSE;
            }
            arg++;
        }
    }
    selected->op = selected->op == 0x1DE ? 0x1DF : 0x1E3;
    selected->flags = (selected->flags & 0x180E0) | opcode_info[selected->op].flags;
    selected->args[0] = selected->args[1];
    selected->args[1] = selected->args[2];
    selected->args[2] = selected->args[3];
    selected->args[2].kind = 0;
    selected->args[2].arg = 2;
    selected->args[2].effect = 2;
    selected->args[2].data.reg.reg = 2;
    register_live_bitmaps_6368D0[2].current[0] |= 4;
    deletepcode(pc);
    return TRUE;
}

int fn_00558170(PCode* pc) {
    PCode* first;
    PCode* second;
    PCode* selected;
    PCode* transfer;
    PCode* cursor;
    PCodeArg* arg;
    SInt16 reg;
    int cls;
    int count;
    int used;
    selected = NULL;
    transfer = NULL;
    used = FALSE;
    if (pc->flags & 0x100) {
        return FALSE;
    }
    first = pc->args[0].data.reg.definer;
    second = pc->args[1].data.reg.definer;
    if (first->op == 0xB5) {
        selected = first;
    } else if (second->op == 0xB5) {
        selected = second;
    }
    if (first->op == 0xAD) {
        transfer = first;
    } else if (second->op == 0xAD) {
        transfer = second;
    }
    if (transfer == NULL ||
        transfer->args[0].data.reg.reg != 0 ||
        transfer->args[0].arg != 0 ||
        selected == NULL) {
        return FALSE;
    }
    for (cursor = pc->prevPCode; cursor != selected; cursor = cursor->prevPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == selected->args[0].kind &&
                arg->data.reg.reg == selected->args[0].data.reg.reg &&
                arg->arg == selected->args[0].arg &&
                (arg->effect & 3)) {
                return FALSE;
            }
            arg++;
        }
    }
    reg = selected->args[0].data.reg.reg;
    cls = (SInt8)selected->args[0].arg;
    if ((register_live_bitmaps_6368D0[cls].current[reg >> 5] >> (reg & 31)) & 1) {
        return FALSE;
    }
    for (cursor = pc->nextPCode; cursor != NULL && !used; cursor = cursor->nextPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == selected->args[0].kind &&
                arg->data.reg.reg == selected->args[0].data.reg.reg &&
                arg->arg == selected->args[0].arg) {
                if (arg->effect & 2) {
                    used = TRUE;
                    break;
                }
            }
            arg++;
        }
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == selected->args[0].kind &&
                arg->data.reg.reg == selected->args[0].data.reg.reg &&
                arg->arg == selected->args[0].arg &&
                (arg->effect & 1)) {
                return FALSE;
            }
            arg++;
        }
    }
    for (cursor = pc->prevPCode; cursor != selected; cursor = cursor->prevPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == 0 && arg->arg == 2 && arg->data.reg.reg == 2 && (arg->effect & 3)) {
                return FALSE;
            }
            arg++;
        }
    }
    selected->op = 0x1E4;
    selected->flags = (selected->flags & 0x180E0) | opcode_info[selected->op].flags;
    selected->args[0] = selected->args[1];
    selected->args[1] = selected->args[2];
    selected->args[2] = selected->args[3];
    selected->args[2].kind = 0;
    selected->args[2].arg = 2;
    selected->args[2].effect = 2;
    selected->args[2].data.reg.reg = 2;
    register_live_bitmaps_6368D0[2].current[0] |= 4;
    deletepcode(pc);
    return TRUE;
}

int fn_0055B330(PCode* pc, UInt16 opcode, UInt16 newOpcode, UInt16 emitOpcode) {
    PCode* first = pc->args[1].data.reg.definer;
    PCode* second = pc->args[2].data.reg.definer;
    PCode* cursor;
    PCodeArg* arg;
    PCode* emitted;
    int count;
    SInt16 reg1;
    SInt16 reg2;
    SInt16 dest;
    if (first->op != opcode || second->op != opcode || first == second) {
        return FALSE;
    }
    if (!fn_0055BDA0(&first, &second, pc->flags & 0x2000)) {
        return FALSE;
    }
    for (cursor = first->nextPCode; cursor != pc; cursor = cursor->nextPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == 0 && arg->arg == 2 && arg->data.reg.reg == 2 && (arg->effect & 2)) {
                return FALSE;
            }
            arg++;
        }
    }
    reg1 = second->args[1].data.reg.reg;
    reg2 = second->args[2].data.reg.reg;
    dest = second->args[0].data.reg.reg;
    if (dest == reg1 || dest == reg2) {
        return FALSE;
    }
    dest = pc->args[0].data.reg.reg;
    if (dest == first->args[1].data.reg.reg ||
        dest == first->args[2].data.reg.reg ||
        dest == reg1 ||
        dest == reg2) {
        return FALSE;
    }
    if (register_live_bitmaps_6368D0[2].current[0] & 4) {
        return FALSE;
    }
    for (cursor = pc->prevPCode; cursor != first; cursor = cursor->prevPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == 0 && arg->arg == 2 && arg->data.reg.reg == 2 && (arg->effect & 3)) {
                return FALSE;
            }
            arg++;
        }
    }
    reg1 = first->args[0].data.reg.reg;
    if ((register_live_bitmaps_6368D0[(SInt8)first->args[0].arg].current[reg1 >> 5] >>
         (reg1 & 31)) &
        1) {
        return FALSE;
    }
    for (cursor = pc->prevPCode; cursor != first; cursor = cursor->prevPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == first->args[0].kind &&
                arg->arg == first->args[0].arg &&
                arg->data.reg.reg == first->args[0].data.reg.reg &&
                (arg->effect & 3)) {
                return FALSE;
            }
            arg++;
        }
    }
    if (definedbetween(pc, second, &second->args[1]) ||
        definedbetween(pc, second, &second->args[2])) {
        return FALSE;
    }
    first->op = newOpcode;
    first->flags = (first->flags & 0x180E0) | opcode_info[newOpcode].flags;
    first->args[0] = first->args[1];
    first->args[1] = first->args[2];
    first->args[2] = first->args[3];
    first->args[2].kind = 0;
    first->args[2].arg = 2;
    first->args[2].effect = 2;
    first->args[2].data.reg.reg = 2;
    register_live_bitmaps_6368D0[2].current[0] |= 4;
    emitted = makepcode(
        emitOpcode,
        (int)pc->args[0].data.reg.reg,
        (int)second->args[1].data.reg.reg,
        (int)second->args[2].data.reg.reg
    );
    insertpcodeafter(pc, emitted);
    reg1 = second->args[1].data.reg.reg;
    register_live_bitmaps_6368D0[1].current[reg1 >> 5] |= 1u << (reg1 & 31);
    reg2 = second->args[2].data.reg.reg;
    register_live_bitmaps_6368D0[1].current[reg2 >> 5] |= 1u << (reg2 & 31);
    return TRUE;
}

int fn_0055AED0(PCode* pc, UInt16 opcode, UInt16 newOpcode, UInt16 emitOpcode) {
    PCode* first = pc->args[1].data.reg.definer;
    PCode* second = pc->args[2].data.reg.definer;
    PCode* cursor;
    PCodeArg* arg;
    PCode* emitted;
    int count;
    SInt16 reg1;
    SInt16 reg2;
    SInt16 dest;
    if (!fn_0055BDA0(&first, &second, pc->flags & 0x2000)) {
        return FALSE;
    }
    if (first->op != opcode || second->op != 0xB5) {
        return FALSE;
    }
    for (cursor = first->nextPCode; cursor != pc; cursor = cursor->nextPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == 0 && arg->arg == 2 && arg->data.reg.reg == 2 && (arg->effect & 2)) {
                return FALSE;
            }
            arg++;
        }
    }
    reg1 = second->args[1].data.reg.reg;
    reg2 = second->args[2].data.reg.reg;
    dest = second->args[0].data.reg.reg;
    if (dest == reg1 || dest == reg2) {
        return FALSE;
    }
    if (register_live_bitmaps_6368D0[2].current[0] & 4) {
        return FALSE;
    }
    for (cursor = pc->prevPCode; cursor != first; cursor = cursor->prevPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == 0 && arg->arg == 2 && arg->data.reg.reg == 2 && (arg->effect & 3)) {
                return FALSE;
            }
            arg++;
        }
    }
    reg1 = first->args[0].data.reg.reg;
    if ((register_live_bitmaps_6368D0[(SInt8)first->args[0].arg].current[reg1 >> 5] >>
         (reg1 & 31)) &
        1) {
        return FALSE;
    }
    for (cursor = pc->prevPCode; cursor != first; cursor = cursor->prevPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == first->args[0].kind &&
                arg->arg == first->args[0].arg &&
                arg->data.reg.reg == first->args[0].data.reg.reg &&
                (arg->effect & 3)) {
                return FALSE;
            }
            arg++;
        }
    }
    if (definedbetween(pc, second, &second->args[1]) ||
        definedbetween(pc, second, &second->args[2])) {
        return FALSE;
    }
    first->op = newOpcode;
    first->flags = (first->flags & 0x180E0) | opcode_info[newOpcode].flags;
    first->args[0] = first->args[1];
    first->args[1] = first->args[2];
    first->args[2] = first->args[3];
    first->args[2].kind = 0;
    first->args[2].arg = 2;
    first->args[2].effect = 2;
    first->args[2].data.reg.reg = 2;
    register_live_bitmaps_6368D0[2].current[0] |= 4;
    emitted = makepcode(
        emitOpcode,
        (int)pc->args[0].data.reg.reg,
        (int)second->args[1].data.reg.reg,
        (int)second->args[2].data.reg.reg
    );
    insertpcodeafter(pc, emitted);
    deletepcode(pc);
    reg1 = second->args[1].data.reg.reg;
    register_live_bitmaps_6368D0[1].current[reg1 >> 5] |= 1u << (reg1 & 31);
    reg2 = second->args[2].data.reg.reg;
    register_live_bitmaps_6368D0[1].current[reg2 >> 5] |= 1u << (reg2 & 31);
    return TRUE;
}

int fn_00559FE0(
    PCode* pc,
    UInt16 firstOp,
    UInt16 secondOp,
    UInt16 thirdOp,
    UInt16 fourthOp,
    UInt16 replacement,
    SInt32 mode
) {
    PCode* before = pc->args[1].data.reg.definer;
    PCode* after = pc->args[2].data.reg.definer;
    PCode* cursor;
    PCodeArg* arg;
    int index;
    UInt8 blocked;
    if (!fn_0055BDA0(&after, &before, pc->flags & 0x2000)) {
        return FALSE;
    }
    if (after->op != firstOp && after->op != secondOp) {
        return FALSE;
    }
    if (before->op != thirdOp && before->op != fourthOp) {
        return FALSE;
    }
    blocked = FALSE;
    for (cursor = after->nextPCode; cursor != before; cursor = cursor->nextPCode) {
        if (cursor != NULL) {
            arg = cursor->args;
            for (index = 0; index < cursor->argCount; index++) {
                if (arg->kind == 0 &&
                    arg->arg == 2 &&
                    (arg->effect & 2) &&
                    arg->data.reg.reg == mode) {
                    blocked = TRUE;
                    break;
                }
                arg++;
            }
            if (blocked) {
                break;
            }
        }
    }
    if (blocked) {
        return FALSE;
    }
    before->op = replacement;
    before->flags = (before->flags & 0x180E0) | opcode_info[replacement].flags;
    before->args[before->argCount - 1].effect |= 3;
    pc->op = move_opcode;
    pc->flags |= 0x10;
    pc->argCount = 2;
    pc->args[1].data.reg.definer = before->args[0].data.reg.definer;
    pc->args[1] = before->args[0];
    pc->args[1].effect = 1;
    return TRUE;
}

int fn_0055AAB0(PCode* pc, UInt16 opcode, UInt16 newOpcode, UInt16 emitOpcode) {
    PCode* first = pc->args[1].data.reg.definer;
    PCode* second = pc->args[2].data.reg.definer;
    PCode* cursor;
    PCodeArg* arg;
    PCode* emitted;
    int count;
    SInt16 reg1;
    SInt16 reg2;
    SInt16 dest;
    if (!fn_0055BDA0(&first, &second, pc->flags & 0x2000)) {
        return FALSE;
    }
    if (first->op != opcode || second->op != 0xB5) {
        return FALSE;
    }
    for (cursor = first->nextPCode; cursor != pc; cursor = cursor->nextPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == 0 && arg->arg == 2 && arg->data.reg.reg == 2 && (arg->effect & 2)) {
                return FALSE;
            }
            arg++;
        }
    }
    reg1 = second->args[1].data.reg.reg;
    reg2 = second->args[2].data.reg.reg;
    dest = second->args[0].data.reg.reg;
    if (dest == reg1 || dest == reg2) {
        return FALSE;
    }
    if (register_live_bitmaps_6368D0[2].current[0] & 4) {
        return FALSE;
    }
    for (cursor = pc->prevPCode; cursor != first; cursor = cursor->prevPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == 0 && arg->arg == 2 && arg->data.reg.reg == 2 && (arg->effect & 3)) {
                return FALSE;
            }
            arg++;
        }
    }
    reg1 = first->args[0].data.reg.reg;
    if ((register_live_bitmaps_6368D0[(SInt8)first->args[0].arg].current[reg1 >> 5] >>
         (reg1 & 31)) &
        1) {
        return FALSE;
    }
    for (cursor = pc->prevPCode; cursor != first; cursor = cursor->prevPCode) {
        arg = cursor->args;
        count = cursor->argCount;
        while (count--) {
            if (arg->kind == first->args[0].kind &&
                arg->arg == first->args[0].arg &&
                arg->data.reg.reg == first->args[0].data.reg.reg &&
                (arg->effect & 3)) {
                return FALSE;
            }
            arg++;
        }
    }
    if (definedbetween(pc, second, &second->args[1]) ||
        definedbetween(pc, second, &second->args[2])) {
        return FALSE;
    }
    first->op = newOpcode;
    first->flags = (first->flags & 0x180E0) | opcode_info[newOpcode].flags;
    first->args[0] = first->args[1];
    first->args[1] = first->args[2];
    first->args[2] = first->args[3];
    first->args[2].kind = 0;
    first->args[2].arg = 2;
    first->args[2].effect = 3;
    first->args[2].data.reg.reg = 2;
    register_live_bitmaps_6368D0[2].current[0] |= 4;
    emitted = makepcode(
        emitOpcode,
        (int)pc->args[0].data.reg.reg,
        (int)second->args[1].data.reg.reg,
        (int)second->args[2].data.reg.reg
    );
    insertpcodeafter(pc, emitted);
    reg1 = second->args[1].data.reg.reg;
    register_live_bitmaps_6368D0[1].current[reg1 >> 5] |= 1u << (reg1 & 31);
    reg2 = second->args[2].data.reg.reg;
    register_live_bitmaps_6368D0[1].current[reg2 >> 5] |= 1u << (reg2 & 31);
    return TRUE;
}
