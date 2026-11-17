#include "compiler/LiveRegisters.h"
#include "compiler/Peephole.h"
#include "compiler/BE_schedule.h"
#include "compiler/CMangler.h"
#include "compiler/CodeGen.h"
#include "compiler/CompilerTools.h"
#include "compiler/PCode.h"
#include "compiler/PCodeInfo.h"
#include "compiler/BE_binary.h"
#include "compiler/CError.h"
#include "compiler/LiveVariables.h"
#include "compiler/CParser.h"

#define IS_JUMP(op) ((op) == PC_B || (UInt16)((op) - PC_B_38A) <= 1)

//! @reconstructed
static int block_ends_in_jump(PCodeBlock* block) {
    PCode* last;
    PCodeBlock* next;

    last = block->lastPCode;
    if (!last) {
        return FALSE;
    }
    if (!block->successors) {
        return FALSE;
    }
    next = block->successors->to;
    if (!next || next->blockIndex == block->blockIndex) {
        return FALSE;
    }
    if (IS_JUMP(last->op)) {
        return TRUE;
    }
    if (last->op == PC_JR && last->args[0].data.reg.reg == 31) {
        return TRUE;
    }
    return FALSE;
}

//! @reconstructed
static void simplify_block(PCodeBlock* block) {
    PCode* last;
    PCode* first;
    PCode* pcode;
    PCode* next;
    PCodeArg* arg;
    PCodeArg* targetArg;
    PCodeBlock* target;
    PCLink* link;
    PCLink* edge;
    int moved;

    last = block->lastPCode;
    moved = FALSE;
    if (!last) {
        return;
    }
    if (last->unk38 == 5 || last->unk38 == 6) {
        return;
    }
    if (last->flags & fPCodeConditionalBranch) {
        arg = get_label_operand(last);
        target = arg->data.label.label->block;
        first = target->firstPCode;
        if (first && IS_JUMP(first->op)) {
            targetArg = get_label_operand(first);
            *arg = *targetArg;
            edge = block->successors;
            for (;;) {
                if (edge->to->blockIndex == target->blockIndex) {
                    if (edge->to == arg->data.label.label->block) {
                        return;
                    }
                    edge->to = arg->data.label.label->block;
                    blocks_changed = TRUE;
                    return;
                }
                edge = edge->nextSuccessor;
            }
        }
    }
    link = block->successors;
    if (!link || link->nextSuccessor) {
        return;
    }
    target = link->to;
    if (target->blockIndex == block->blockIndex) {
        return;
    }
    if (IS_JUMP(last->op) &&
        block->nextBlock &&
        target->blockIndex == block->nextBlock->blockIndex) {
        deletepcode(last);
        blocks_changed = TRUE;
        return;
    }
    if (!block_ends_in_jump(target)) {
        return;
    }
    if (opcode_info[last->op].flags & OPCODE_IS_CALL) {
        return;
    }
    if (last->flags & fPCodeIsBranch) {
        return;
    }
    link = target->predecessors;
    if (link && !link->nextPredecessor) {
        if (link->from->blockIndex != block->blockIndex) {
            return;
        }
        if ((pcode = target->firstPCode) != NULL) {
            moved = TRUE;
            for (; pcode; pcode = next) {
                next = pcode->nextPCode;
                deletepcode(pcode);
                appendpcode(block, pcode);
            }
        }
    }
    if (moved) {
        blocks_changed = TRUE;
        block->successors = target->successors;
    }
}

void optimize_blocks(void) {
    PCodeBlock* block;

    do {
        blocks_changed = FALSE;
        for (block = pcbasicblocks; block; block = block->nextBlock) {
            if (block->flags & fPCBlockFlag80) {
                continue;
            }
            if (block->flags & fPCBlockFlag4000) {
                continue;
            }
            simplify_block(block);
            if (blocks_changed) {
                break;
            }
        }
        if (blocks_changed) {
            for (block = pcbasicblocks; block; block = block->nextBlock) {
                block->predecessors = NULL;
            }
            deleteunreachableblocks();
        }
    } while (blocks_changed);
}

static inline PCodeBlock* next_nonempty_block(PCodeBlock* block) {
    PCodeBlock* next;

    for (next = block->nextBlock; next; next = next->nextBlock) {
        if (next->pcodeCount != 0) {
            return next;
        }
    }
    return NULL;
}

static PCode* find_delay_slot_instruction(PCodeBlock* block); //! @reconstructed
static void insert_hazard_nops(PCodeBlock* block); //! @reconstructed

//! @reconstructed
static void fill_block_delay_slot_simple(PCodeBlock* block) {
    PCode* branch;
    PCode* nop;
    PCodeBlock* target;
    PCodeBlock* next;
    UInt16 opcode;
    UInt32 flags;

    target = NULL;
    if (block->flags & fPCBlockFlag4000) {
        return;
    }
    branch = block->lastPCode;
    if (!branch || !((flags = branch->flags) & fPCodeHasDelaySlot)) {
        return;
    }
    opcode = branch->op;
    if ((opcode == PC_B ||
         opcode == PC_B_38A ||
         opcode == PC_B_38B ||
         (flags & fPCodeConditionalBranch)) &&
        branch->unk38 == 0 &&
        !(flags & fPCodeLikely)) {
        target = get_label_operand(branch)->data.label.label->block;
        next = next_nonempty_block(block);
        if (next && target && target->blockIndex == next->blockIndex) {
            deletepcode(branch);
            return;
        }
    }
    if (branch->prevPCode && (branch->prevPCode->flags & fPCodeInDelaySlot)) {
        if (target && target->blockIndex == branch->block->blockIndex && target->pcodeCount == 2) {
            nop = makepcode(get_nop_opcode());
            insertpcodebefore(branch, nop);
        }
        return;
    }
    nop = makepcode(get_nop_opcode());
    nop->sourceoffset = branch->sourceoffset;
    nop->flags |= fPCodeInDelaySlot;
    insertpcodebefore(branch, nop);
    if (target && target->blockIndex == branch->block->blockIndex && target->pcodeCount <= 7) {
        if (copts.padloop) {
            pad_short_loop(block);
        } else {
            nop = makepcode(get_nop_opcode());
            insertpcodebefore(branch, nop);
        }
    }
}

//! @reconstructed
static void fill_block_delay_slot(PCodeBlock* block, Boolean skipTarget) {
    PCodeBlock* target;
    UInt32 flags;
    UInt16 opcode;
    PCode* branch;
    PCode* first;
    PCode* copy;
    PCode* nop;
    PCode* after;
    PCode* jump;
    PCodeBlock* next;
    int i;

    target = NULL;
    if (block->flags & fPCBlockFlag4000) {
        return;
    }
    if (block->pcodeCount == 0) {
        return;
    }
    branch = block->lastPCode;
    flags = branch->flags;
    if (!(flags & fPCodeHasDelaySlot)) {
        return;
    }
    opcode = branch->op;
    if ((opcode == PC_B ||
         opcode == PC_B_38A ||
         opcode == PC_B_38B ||
         (flags & fPCodeConditionalBranch)) &&
        branch->unk38 == 0 &&
        !(flags & fPCodeLikely)) {
        target = get_label_operand(branch)->data.label.label->block;
        next = next_nonempty_block(block);
        if (next && target->blockIndex == next->blockIndex) {
            deletepcode(branch);
            return;
        }
    }
    first = find_delay_slot_instruction(block);
    if (first) {
        insertpcodebefore(branch, first);
    } else {
        if (!skipTarget && copts.unknown2E) {
            if (IS_JUMP(branch->op)) {
                target = get_label_operand(branch)->data.label.label->block;
                first = target->firstPCode;
                if (first && !(first->flags & (fPCodeInDelaySlot | fPCodeFlag8000000))) {
                    if (target->predecessors && !target->predecessors->nextPredecessor) {
                        deletepcode(first);
                        insertpcodebefore(branch, first);
                        first->flags |= fPCodeInDelaySlot;
                        freeoheap();
                        compute_live_registers();
                        return;
                    }
                    if (first->unk38 != 5) {
                        copy = copypcode(first);
                        insertpcodebefore(branch, copy);
                        copy->flags |= fPCodeInDelaySlot;
                        branch->flags |= fPCodeFlag4000000;
                        first->flags |= fPCodeFlag8000000;
                        freeoheap();
                        compute_live_registers();
                        return;
                    }
                }
            } else if ((branch->flags & fPCodeConditionalBranch) &&
                       !copts.no_branch_likely &&
                       !(branch->flags & fPCodeLikely)) {
                target = get_label_operand(branch)->data.label.label->block;
                first = target->firstPCode;
                if (first &&
                    !(first->flags & (fPCodeInDelaySlot | fPCodeFlag8000000 | 0x100)) &&
                    first->unk38 != 5) {
                    if (!target->predecessors || target->predecessors->nextPredecessor) {
                        copy = copypcode(first);
                        insertpcodebefore(branch, copy);
                        copy->flags |= fPCodeInDelaySlot;
                        branch->op++;
                        branch->flags |= fPCodeFlag4000000 | fPCodeLikely;
                        first->flags |= fPCodeFlag8000000;
                        after = first->nextPCode;
                        if (after) {
                            jump = after;
                            if (after->op != PC_B || after->args[0].data.label.unknown8 != 0) {
                                if (after->op == PC_NOP &&
                                    (after->flags & fPCodeInDelaySlot) &&
                                    after->nextPCode &&
                                    after->nextPCode->op == PC_B &&
                                    after->nextPCode->args[0].data.label.unknown8 == 0) {
                                    jump = after->nextPCode;
                                } else {
                                    jump = NULL;
                                }
                            }
                            if (jump) {
                                branch->flags &= ~fPCodeFlag4000000;
                                for (i = 0; i < branch->argCount; i++) {
                                    if (branch->args[i].kind == PCOp_LABEL) {
                                        branch->args[i].data.label.label =
                                            jump->args[0].data.label.label;
                                        if (jump->args[0].data.label.label->block->firstPCode) {
                                            jump->args[0]
                                                .data.label.label->block->firstPCode->flags |=
                                                fPCodeFlag8000000;
                                        }
                                        break;
                                    }
                                    CError_ASSERT(640, i < branch->argCount);
                                }
                            }
                        }
                    } else {
                        branch->op++;
                        branch->flags |= fPCodeLikely;
                        after = first->nextPCode;
                        deletepcode(first);
                        insertpcodebefore(branch, first);
                        first->flags |= fPCodeInDelaySlot;
                        if (after) {
                            jump = NULL;
                            if (after->op == PC_B && after->args[0].data.label.unknown8 == 0) {
                                deletepcode(after);
                                jump = after;
                            } else if (after->op == PC_NOP &&
                                       (after->flags & fPCodeInDelaySlot) &&
                                       after->nextPCode &&
                                       after->nextPCode->op == PC_B &&
                                       after->nextPCode->args[0].data.label.unknown8 == 0) {
                                jump = after->nextPCode;
                                deletepcode(after);
                                deletepcode(jump);
                            }
                            if (jump) {
                                for (i = 0; i < branch->argCount; i++) {
                                    if (branch->args[i].kind == PCOp_LABEL) {
                                        branch->args[i].data.label.label =
                                            jump->args[0].data.label.label;
                                        break;
                                    }
                                    CError_ASSERT(573, i < branch->argCount);
                                }
                            }
                        }
                    }
                    goto done;
                }
            }
        }
        nop = makepcode(get_nop_opcode());
        nop->flags |= fPCodeInDelaySlot;
        insertpcodebefore(branch, nop);
    }
done:
    if (copts.padloop &&
        target &&
        target->blockIndex == branch->block->blockIndex &&
        target->pcodeCount <= 7) {
        pad_short_loop(block);
    }
}

static PCode* find_delay_slot_instruction(PCodeBlock* block) {
    PCode* pcode;
    PCode* fallback;

    fallback = NULL;
    for (pcode = block->lastPCode; pcode; pcode = pcode->prevPCode) {
        if (pcode->flags & fPCodeInDelaySlot) {
            deletepcode(pcode);
            return pcode;
        }
    }
    for (pcode = block->lastPCode; pcode; pcode = pcode->prevPCode) {
        if (pcode->flags & (fPCodeFlag2000000 | fPCodeFlag8000000)) {
            continue;
        }
        if (!(pcode->flags & fPCodeReadsMemory)) {
            break;
        }
        fallback = pcode;
    }
    if (!pcode) {
        pcode = fallback;
    }
    if (pcode) {
        pcode->flags |= fPCodeInDelaySlot;
        deletepcode(pcode);
    }
    return pcode;
}

void fill_delay_slots(Object* function, int schedule, Boolean skipTarget) {
    PCodeBlock* block;

    if (!schedule) {
        for (block = pcbasicblocks; block; block = block->nextBlock) {
            fill_block_delay_slot_simple(block);
        }
    } else {
        compute_live_registers();
        for (block = pcbasicblocks; block; block = block->nextBlock) {
            fill_block_delay_slot(block, skipTarget);
        }
        freeoheap();
    }
    if (copts.debuglisting) {
        pclistblocks(CMangler_GetLinkName(function)->name, "After Branch Delay Slot");
    }
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        if (!(block->flags & fPCBlockFlag4000)) {
            insert_hazard_nops(block);
        }
    }
}

//! @reconstructed
static int reads_register(PCodeArg* reg, PCode* pcode) {
    PCodeArg* arg;
    int i;
    int mask;
    int number;
    int count;

    arg = pcode->args;
    number = reg->data.reg.reg;
    if (reg->arg == 1 && (number & 1)) {
        number &= -2;
    }
    if (opcode_info[pcode->op].flags & OPCODE_IS_CALL) {
        count = 2;
    } else {
        count = pcode->argCount;
    }
    for (i = 0; i < count; i++, arg++) {
        if (arg->kind == PCOp_REGISTER && arg->arg == reg->arg) {
            if (arg->arg == 1) {
                mask = -2;
            } else {
                mask = -1;
            }
            if (number == (arg->data.reg.reg & mask) && (arg->effect & 1)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

static inline Boolean writes_register_read_by(PCode* writer, PCode* reader) {
    PCodeArg* arg;
    int count;
    int i;

    arg = writer->args;
    if (opcode_info[writer->op].flags & OPCODE_IS_CALL) {
        count = 2;
    } else {
        count = writer->argCount;
    }
    for (i = 0; i < count; i++, arg++) {
        if (arg->kind == PCOp_REGISTER && (arg->effect & 2) && reads_register(arg, reader)) {
            return TRUE;
        }
    }
    return FALSE;
}

static void insert_hazard_nops(PCodeBlock* block) {
    PCode* cur;
    PCode* next;
    PCode* afterNext;
    PCode* slot;
    PCode* last;
    PCode* pcode;
    UInt32 flags;
    int nopsAfter;
    int nopsAfterNext;
    int i;

    last = block->lastPCode;
    cur = block->firstPCode;
    if (!last || !(last->flags & fPCodeHasDelaySlot)) {
        slot = NULL;
    } else {
        slot = last->prevPCode;
        if (slot && (slot->flags & fPCodeInDelaySlot)) {
            deletepcode(slot);
        }
    }
    while (cur) {
        nopsAfterNext = nopsAfter = 0;
        next = cur->nextPCode;
        afterNext = next ? next->nextPCode : NULL;
        if (cur->op >= 0x40A || !next) {
            goto advance;
        }
        if (cur->flags & fPCodeReadsMemory) {
            if (writes_register_read_by(cur, next) && !copts.unknown2E) {
                nopsAfter++;
            }
        } else {
            flags = opcode_info[cur->op].flags2;
            if (flags & 0xA000) {
                if (writes_register_read_by(next, cur)) {
                    nopsAfter++;
                }
            } else if (flags & 0x4000) {
                if (writes_register_read_by(cur, next))
                    ;
            } else if (flags & 0x5000) {
                if (writes_register_read_by(cur, next)) {
                    nopsAfter = 1;
                }
            } else if (flags & 0x20000) {
                if (afterNext && (opcode_info[afterNext->op].flags2 & 0x10000)) {
                    nopsAfterNext++;
                } else if (!afterNext && slot && (opcode_info[slot->op].flags2 & 0x10000)) {
                    nopsAfter++;
                }
            } else if (flags & 0x80000) {
                if (afterNext && (opcode_info[afterNext->op].flags2 & 0x40000)) {
                    nopsAfterNext++;
                } else if (!afterNext && slot && (opcode_info[slot->op].flags2 & 0x40000)) {
                    nopsAfter++;
                }
            } else if (cur->flags & 0x1000000) {
                if (writes_register_read_by(cur, next)) {
                    nopsAfter++;
                }
            }
        }
        if ((copts.unknown02 & 0x2000) &&
            (UInt16)(cur->op - 0xB4) <= 1 &&
            (UInt16)(next->op - 0xB4) <= 3 &&
            nopsAfter == 0) {
            nopsAfter++;
        }
        for (i = 0; i < nopsAfter; i++) {
            insertpcodeafter(cur, makepcode(get_nop_opcode()));
        }
        for (i = 0; i < nopsAfterNext; i++) {
            insertpcodeafter(next, makepcode(get_nop_opcode()));
        }
advance:
        cur = next;
    }
    last = block->lastPCode;
    pcode = last;
    if (last && last->op >= 0x4A5) {
        for (pcode = last->prevPCode; pcode && pcode->op >= 0x40A; pcode = pcode->prevPCode)
            ;
    }
    if (!pcode) {
        return;
    }
    if ((!copts.unknown2E && (pcode->flags & fPCodeReadsMemory)) ||
        (opcode_info[pcode->op].flags2 & 0x5000)) {
        insertpcodeafter(pcode, makepcode(get_nop_opcode()));
    }
    if (slot) {
        insertpcodebefore(last, slot);
    }
}

//! @reconstructed
static void pad_division_hazards(PCodeBlock* block) {
    PCode* pcode;
    PCode* last;
    Boolean fromStart;
    Boolean branchEnds;
    int position;
    int divide;
    int distance;

    last = block->lastPCode;
    fromStart = FALSE;
    if (block->prevBlock == previous_block) {
        fromStart = TRUE;
    }
    branchEnds = last->flags & 1;
    divide = -1;
    position = 1;
    for (pcode = block->firstPCode; pcode; pcode = pcode->nextPCode) {
        if (pcode->op == 0xF3 ||
            pcode->op == 0x366 ||
            pcode->op == 0x101 ||
            pcode->op == 0x37A ||
            pcode->op == 0x125) {
            if (divide == position - 1) {
                insertpcodebefore(pcode, makepcode(PC_NOP));
                position++;
            }
            if (branchEnds && (distance = block->pcodeCount - position) <= 3) {
                while (distance <= 3) {
                    insertpcodeafter(pcode, makepcode(PC_NOP));
                    distance = block->pcodeCount - position;
                }
            }
            if (fromStart && position < 3) {
                for (; position < 3; position++) {
                    insertpcodebefore(pcode, makepcode(PC_NOP));
                }
            }
        } else if (pcode->op == 0xDF) {
            divide = position;
        }
        position++;
    }
}

void insert_division_nops(void) {
    PCodeBlock* block;

    previous_block = NULL;
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        if (block->pcodeCount != 0 &&
            !(block->flags & fPCBlockFlag80) &&
            !(block->flags & fPCBlockFlag4000)) {
            pad_division_hazards(block);
        }
        previous_block = block;
    }
}
