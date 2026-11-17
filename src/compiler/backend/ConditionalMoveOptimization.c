#include "compiler/CParser.h"
#include "compiler/LoopDetection.h"
#include "compiler/ConditionalMoveOptimization.h"
#define CERROR_FILE "ConditionalMoveOptimization.c"
#include "compiler/PCode.h"
#include "compiler/PCodeInfo.h"
#include "compiler/common.h"
#include "compiler/CError.h"
#include "compiler/BE_binary.h"

void fn_004C27E0(void* loop);
void fn_004C1EC0(
    PCode* branch, int isTrue, PCodeBlock* left, PCodeBlock* right, PCodeBlock* common
);
PCode* fn_004C2030(PCode* branch, PCode* source, UInt8 cond);

#pragma dont_inline on
Boolean fn_004C2630(PCode* pcode) {
    UInt16 opcode = pcode->op;
    int reg;
    if (opcode == PC_BEQZ || opcode == PC_BNEZ) {
        return TRUE;
    }
    if (opcode == PC_BEQ || opcode == PC_BNE) {
        reg = pcode->args[1].data.reg.reg;
        if (reg == 0) {
            return TRUE;
        }
    }
    if (!copts.conditional_move || copts.unknown0C >= 4) {
        if (opcode == PC_BC1T || opcode == PC_BC1F) {
            return TRUE;
        }
    }
    return FALSE;
}

#pragma dont_inline reset
Boolean fn_004C24F0(PCodeBlock* block, int reg) {
    PCode* pcode = block->firstPCode;
    UInt16 opcode;
    UInt32 flags;
    int idx;
    if (!pcode) {
        return FALSE;
    }
    flags = pcode->flags;
    if (flags & 2) {
        return FALSE;
    }
    if (block->pcodeCount == 1) {
        opcode = block->lastPCode->op;
        if (opcode == PC_B || (UInt16)(opcode - 0x38A) <= 1) {
            if (flags & 0x80000000) {
                idx = 1;
            } else {
                idx = 0;
            }
            if (pcode->args[idx].data.reg.reg == reg) {
                return TRUE;
            }
        }
    }
    opcode = pcode->op;
    if (opcode == PC_B || (UInt16)(opcode - 0x38A) <= 1) {
        return TRUE;
    }
    return FALSE;
}
Boolean fn_004C2590(PCodeBlock* block, int reg) {
    PCode* instr;
    UInt16 count = block->pcodeCount;
    UInt16 op;
    UInt32 flags;
    UInt32 bit;
    int idx;

    instr = block->firstPCode;
    if (count == 1 ||
        (count == 2 && ((op = block->lastPCode->op) == PC_B || (UInt16)(op - 0x38A) <= 1))) {
        flags = instr->flags;
        if (flags & 0x10) {
            bit = flags & 0x80000000;
            if (bit) {
                idx = 1;
            } else {
                idx = 0;
            }
            if (instr->args[idx].data.reg.reg == reg) {
                return FALSE;
            }
            if (copts.conditional_move && copts.unknown0C < 4) {
                if (bit) {
                    idx = 1;
                } else {
                    idx = 0;
                }
                return instr->args[idx].arg == 0;
            }
            return TRUE;
        }
    }
    return FALSE;
}
void fn_004C2690(Object* proc) {
    forall_loops_in_postorder_do(fn_004C27E0);
}

void fn_004C27E0(void* loop) {
    LoopRegisterBlock* link = ((LoopRegisterNode*)loop)->padBlockLink;
    if (!link) {
        return;
    }
    pad_short_loop(link->block);
}

void pad_short_loop(PCodeBlock* block) {
    PCode* branch;
    PCode* instr;
    PCodeBlock* target;
    PCodeBlock* ptr;
    int distance = 0;
    int size = 0;
    branch = block->lastPCode;
    if (block->flags & fPCBlockFlag4000) {
        return;
    }
    if (!branch) {
        return;
    }
    if (!(branch->flags & fPCodeConditionalBranch) &&
        branch->op != 0x39 &&
        branch->op != 0x38A &&
        branch->op != 0x38B) {
        return;
    }
    target = get_label_operand(branch)->data.label.label->block;
    ptr = target;
    while (ptr->blockIndex != block->blockIndex && (ptr = ptr->nextBlock)) {
        distance++;
        if (distance > 2 || distance < -2) {
            return;
        }
    }
    if (target != block) {
        do {
            size += block->pcodeCount;
            for (instr = block->firstPCode; instr; instr = instr->nextPCode) {
                if (instr->flags & 0x100000) {
                    size--;
                }
            }
            block = block->prevBlock;
            if (!block) {
                break;
            }
            if (size >= 7) {
                return;
            }
        } while (target != block);
    }
    if (block) {
        size += block->pcodeCount;
        for (instr = block->firstPCode; instr; instr = instr->nextPCode) {
            if (instr->flags & 0x100000) {
                size--;
            }
        }
    }
    if (size >= 7) {
        return;
    }
    instr = branch->prevPCode;
    if (instr && (instr->flags & fPCodeInDelaySlot)) {
        instr->flags &= ~fPCodeInDelaySlot;
    }
    while (size < 7) {
        insertpcodebefore(branch, makepcode(0xBA));
        size++;
    }
    instr = branch->prevPCode;
    if (instr) {
        instr->flags |= fPCodeInDelaySlot;
    }
}

void fn_004C22E0(void) {
    PCodeBlock* block;
    PCodeBlock* target;
    PCodeBlock* other;
    PCodeBlock* common;
    PCode* branch;
    PCLink* edge;
    PCLink* opposite;
    PCLink* outgoing;
    PCode* ins;
    PCodeArg* selected;
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        branch = block->lastPCode;
        if (!branch || !fn_004C2630(branch)) {
            continue;
        }
        target = get_label_operand(branch)->data.label.label->block;
        edge = block->successors;
        other = edge->to;
        if (other == target) {
            edge = edge->nextSuccessor;
            if (!edge) {
                continue;
            }
            other = edge->to;
        }
        if (!other->predecessors ||
            other->predecessors->nextPredecessor ||
            !target->predecessors ||
            target->predecessors->nextPredecessor) {
            continue;
        }
        outgoing = other->successors;
        if (outgoing && !outgoing->nextSuccessor && outgoing->to == target) {
            common = target;
        } else {
            if (!outgoing || outgoing->nextSuccessor) {
                continue;
            }
            opposite = target->successors;
            if (!opposite || opposite->nextSuccessor) {
                continue;
            }
            common = outgoing->to;
            if (common != opposite->to) {
                continue;
            }
        }
        if (common == target) {
            if (fn_004C2590(other, branch->args[0].data.reg.reg)) {
                fn_004C1EC0(branch, 1, other, 0, common);
            }
        } else if (fn_004C2590(other, branch->args[0].data.reg.reg)) {
            ins = other->firstPCode;
            selected = (ins->op > 0x21E && ins->op <= 0x2E7) ? &ins->args[1] : &ins->args[0];
            if (fn_004C24F0(target, selected->data.reg.reg)) {
                fn_004C1EC0(branch, 1, other, target, common);
            }
        } else if (fn_004C2590(target, branch->args[0].data.reg.reg)) {
            ins = target->firstPCode;
            selected = (ins->op > 0x21E && ins->op <= 0x2E7) ? &ins->args[1] : &ins->args[0];
            if (fn_004C24F0(other, selected->data.reg.reg)) {
                fn_004C1EC0(branch, 0, target, other, common);
            }
        }
    }
    deleteunreachableblocks();
}

void fn_004C1EC0(
    PCode* branch, int isTrue, PCodeBlock* left, PCodeBlock* right, PCodeBlock* common
) {
    PCode* source = left->firstPCode;
    PCodeBlock* dest = branch->block;
    PCLink* edge;
    PCodeBlock* pred;
    PCode* instr;
    for (edge = dest->successors; edge; edge = edge->nextSuccessor) {
        deleteedge(dest, edge->to);
    }
    for (edge = common->predecessors; edge; edge = edge->nextPredecessor) {
        pred = edge->from;
        if (pred->blockIndex == branch->block->blockIndex ||
            pred->blockIndex == left->blockIndex ||
            (pred->blockIndex == right->blockIndex)) {
            deleteedge(pred, common);
        }
    }
    appendpcode(dest, fn_004C2030(branch, source, (UInt8)isTrue));
    if (right) {
        for (instr = right->firstPCode; instr; instr = instr->nextPCode) {
            if (instr->op != 0x39 && instr->op != 0x38A && instr->op != 0x38B) {
                appendpcode(dest, copypcode(instr));
            }
        }
    }
    if (right && (right->lastPCode->flags & 1)) {
        appendpcode(dest, copypcode(right->lastPCode));
        for (edge = right->successors; edge; edge = edge->nextSuccessor) {
            insertedge(dest, edge->to);
        }
    } else {
        if (left->lastPCode->flags & 1) {
            appendpcode(dest, copypcode(left->lastPCode));
            for (edge = left->successors; edge; edge = edge->nextSuccessor) {
                insertedge(dest, left);
            }
        }
    }
    pcaddedge(dest, common->labels, 0);
    deletepcode(branch);
    deleteunreachableblocks();
}

PCode* fn_004C2030(PCode* branch, PCode* source, UInt8 cond) {
    int destReg;
    int srcReg;
    int branchReg;
    short opcode;
    int mode;
    UInt32 flags;
    UInt8 isF8;
    PCodeArg* arg;
    int argIndex;
    flags = source->flags;
    if (flags & 0x10) {
        mode = (source->op > 0x21E && source->op <= 0x2E7);
        if (mode) {
            arg = &source->args[2];
        } else {
            arg = &source->args[1];
        }
        destReg = arg->data.reg.reg;
        if (source->op > 0x21E && source->op <= 0x2E7) {
            arg = &source->args[1];
        } else {
            arg = &source->args[0];
        }
        srcReg = arg->data.reg.reg;
    }
    isF8 = source->op == 0xF8;
    switch (branch->op) {
        case 0x18:
        case 0x1A:
            branchReg = branch->args[0].data.reg.reg;
            if (flags & 0x80000000) {
                argIndex = 1;
            } else {
                argIndex = 0;
            }
            if (source->args[argIndex].arg == 0) {
                opcode = cond ? 0x110 : 0x111;
            } else if (isF8) {
                opcode = cond ? 0x118 : 0x11A;
            } else {
                opcode = cond ? 0x119 : 0x11B;
            }
            return makepcode(opcode, srcReg, destReg, branchReg);
        case 0x28:
        case 0x2A:
            branchReg = branch->args[0].data.reg.reg;
            if (flags & 0x80000000) {
                argIndex = 1;
            } else {
                argIndex = 0;
            }
            if (source->args[argIndex].arg == 0) {
                opcode = cond ? 0x111 : 0x110;
            } else if (isF8) {
                opcode = cond ? 0x11A : 0x118;
            } else {
                opcode = cond ? 0x11B : 0x119;
            }
            return makepcode(opcode, srcReg, destReg, branchReg);
        case 0xC:
            if (flags & 0x80000000) {
                argIndex = 1;
            } else {
                argIndex = 0;
            }
            if (source->args[argIndex].arg == 0) {
                opcode = cond ? 0x113 : 0x112;
            } else if (isF8) {
                opcode = cond ? 0x116 : 0x114;
            } else {
                opcode = cond ? 0x117 : 0x115;
            }
            return makepcode(opcode, srcReg, destReg, 0);
        case 0xE:
            if (flags & 0x80000000) {
                argIndex = 1;
            } else {
                argIndex = 0;
            }
            if (source->args[argIndex].arg == 0) {
                opcode = cond ? 0x112 : 0x113;
            } else if (isF8) {
                opcode = cond ? 0x114 : 0x116;
            } else {
                opcode = cond ? 0x115 : 0x117;
            }
            return makepcode(opcode, srcReg, destReg, 0);
        default:
            CError_FATAL(232);
            return NULL;
    }
}
