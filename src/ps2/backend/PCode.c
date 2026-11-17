#include "compiler/BE_binary.h"
#include "compiler/PCode.h"
#include "compiler/Alias.h"
#include "compiler/CodeGen.h"
#include "compiler/CompilerTools.h"
#include "compiler/PCodeInfo.h"
#include "compiler/CError.h"
#include <string.h>

void initpcode(void) {
    int i;

    for (i = 0; i < PCLINK_HASH_SIZE; i++) {
        pclinkhash[i] = NULL;
    }
    pcbasicblocks = pclastblock = NULL;
    pclabelcount = pcblockcount = pclinkcount = 0;
    pcloopweight = 1;
    initialize_aliases();
}

PCode* makepcode(short op, ...) {
    PCode* pcode;
    va_list list;

    va_start(list, op);
    pcode = vformatpcode(op, list);
    pcode->sourceoffset = current_statement ? current_statement->sourceoffset : -1;
    return pcode;
}

void appendpcode(PCodeBlock* block, PCode* pcode) {
    if (block->firstPCode) {
        pcode->nextPCode = NULL;
        pcode->prevPCode = block->lastPCode;
        block->lastPCode->nextPCode = pcode;
        block->lastPCode = pcode;
    } else {
        block->firstPCode = block->lastPCode = pcode;
        pcode->nextPCode = pcode->prevPCode = NULL;
    }
    pcode->block = block;
    block->pcodeCount++;
}

void emitpcode(short op, ...) {
    PCode* pcode;
    va_list list;

    va_start(list, op);
    pcode = vformatpcode(op, list);
    if (!pcode->sourceoffset) {
        pcode->sourceoffset = current_statement ? current_statement->sourceoffset : -1;
    }
    appendpcode(pclastblock, pcode);
}

PCode* copypcode(PCode* pcode) {
    PCode* newpc;
    int i;

    newpc = lalloc(sizeof(PCode) + pcode->argCount * sizeof(PCodeArg));
    *newpc = *pcode;
    for (i = 0; i < pcode->argCount; i++) {
        newpc->args[i] = pcode->args[i];
    }
    return newpc;
}

PCodeLabel* makepclabel(void) {
    PCodeLabel* label;

    label = lalloc(sizeof(PCodeLabel));
    memclrw(label, sizeof(PCodeLabel));
    label->index = pclabelcount++;
    return label;
}

PCodeBlock* makepcblock(void) {
    PCodeBlock* block;

    block = lalloc(sizeof(PCodeBlock));
    memclrw(block, sizeof(PCodeBlock));
    block->codeOffset = -1;
    block->loopWeight = pcloopweight;
    block->blockIndex = pcblockcount++;
    if (pclastblock) {
        pclastblock->nextBlock = block;
        block->prevBlock = pclastblock;
    } else {
        pcbasicblocks = block;
    }
    pclastblock = block;
    return block;
}

void pclabelblock(PCodeBlock* block, PCodeLabel* label) {
    PCodeLabelBranch* branch;

    for (branch = (PCodeLabelBranch*)label->block; branch; branch = branch->next) {
        insertedge(branch->block, block)->flags |= branch->flags;
    }
    label->block = block;
    label->resolved = TRUE;
    label->nextLabel = block->labels;
    block->labels = label;
}

void pcaddedge(PCodeBlock* block, PCodeLabel* label, UInt16 flags) {
    PCLink* link;
    PCodeLabelBranch* branch;

    if (label->resolved) {
        link = insertedge(block, label->block);
        link->flags |= flags;
    } else {
        branch = lalloc(sizeof(PCodeLabelBranch));
        memclrw(branch, sizeof(PCodeLabelBranch));
        branch->block = block;
        branch->flags = flags;
        branch->next = (PCodeLabelBranch*)label->block;
        label->block = (PCodeBlock*)branch;
    }
}

void emit_label(PCodeLabel* label) {
    if (pclastblock->pcodeCount) {
        pcaddedge(pclastblock, label, 0);
        makepcblock();
    }
    pclabelblock(pclastblock, label);
}

void new_basic_block(void) {
    emit_label(makepclabel());
}

void deleteblock(PCodeBlock* block) {
    if (block->prevBlock) {
        block->prevBlock->nextBlock = block->nextBlock;
    } else {
        pcbasicblocks = block->nextBlock;
    }
    if (block->nextBlock) {
        block->nextBlock->prevBlock = block->prevBlock;
    } else {
        pclastblock = block->prevBlock;
    }
    block->flags |= fDeleted;
}

void deleteunreachableblocks(void) {
    PCodeBlock* block;

    computedepthfirstordering();
    for (block = pcbasicblocks->nextBlock; block; block = block->nextBlock) {
        if (!(block->flags & fVisited) &&
            !(block->flags & (fPCBlockFlag10 | fPCBlockFlag80 | fPCBlockFlag100))) {
            deleteblock(block);
        }
    }
}

void deletepcode(PCode* pcode) {
    PCodeBlock* block;

    block = pcode->block;
    if (pcode->prevPCode) {
        pcode->prevPCode->nextPCode = pcode->nextPCode;
    } else {
        block->firstPCode = pcode->nextPCode;
    }
    if (pcode->nextPCode) {
        pcode->nextPCode->prevPCode = pcode->prevPCode;
    } else {
        block->lastPCode = pcode->prevPCode;
    }
    pcode->block = NULL;
    block->pcodeCount--;
    block->flags &= ~fScheduled;
}

void insertpcodebefore(PCode* anchor, PCode* newpcode) {
    PCodeBlock* block;

    block = anchor->block;
    if (anchor->prevPCode) {
        anchor->prevPCode->nextPCode = newpcode;
    } else {
        block->firstPCode = newpcode;
    }
    newpcode->unk24 = anchor->unk24;
    newpcode->nextPCode = anchor;
    newpcode->prevPCode = anchor->prevPCode;
    anchor->prevPCode = newpcode;
    newpcode->sourceoffset = anchor->sourceoffset;
    newpcode->block = block;
    block->pcodeCount++;
    block->flags &= ~fScheduled;
}

void insertpcodeafter(PCode* anchor, PCode* newpcode) {
    PCodeBlock* block;

    block = anchor->block;
    if (anchor->nextPCode) {
        anchor->nextPCode->prevPCode = newpcode;
    } else {
        block->lastPCode = newpcode;
    }
    newpcode->unk24 = anchor->nextPCode ? anchor->nextPCode->unk24 : 0;
    newpcode->prevPCode = anchor;
    newpcode->nextPCode = anchor->nextPCode;
    anchor->nextPCode = newpcode;
    newpcode->sourceoffset = anchor->sourceoffset;
    newpcode->block = block;
    block->pcodeCount++;
    block->flags &= ~fScheduled;
}

void setpcodeflags(UInt32 flags) {
    pclastblock->lastPCode->flags |= flags;
}

typedef struct DFO {
    PCodeBlock* block;
    PCLink* link;
} DFO;

void computedepthfirstordering(void) {
    PCodeBlock* block;
    PCLink* link;
    DFO* dfo;
    SInt32 index;

    depthfirstordering = lalloc(sizeof(PCodeBlock*) * pcblockcount);
    depthfirstorder = pcblockcount;
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        block->flags &= ~fVisited;
    }

    dfo = oalloc(sizeof(DFO) * pcblockcount);
    index = 0;
    pcbasicblocks->flags |= fVisited;
    dfo[index].block = pcbasicblocks;
    dfo[index].link = pcbasicblocks->successors;
    index++;
    while (index) {
        if ((link = dfo[index - 1].link)) {
            dfo[index - 1].link = link->nextSuccessor;
            block = link->to;
            if (!(block->flags & fVisited)) {
                block->flags |= fVisited;
                dfo[index].block = block;
                dfo[index].link = block->successors;
                index++;
            }
        } else {
            block = dfo[--index].block;
            depthfirstordering[--depthfirstorder] = block;
        }
    }

    while (depthfirstorder) {
        depthfirstordering[--depthfirstorder] = NULL;
    }
}

PCodeBlock* insertblockbetween(PCodeBlock* from, PCodeBlock* to) {
    PCodeBlock* block;

    CError_ASSERT(611, from->nextBlock == to);
    block = lalloc(sizeof(PCodeBlock));
    memclrw(block, sizeof(PCodeBlock));
    block->codeOffset = -1;
    block->blockIndex = pcblockcount++;
    block->loopWeight = from->loopWeight;
    pclabelblock(block, makepclabel());
    from->nextBlock = block;
    block->prevBlock = from;
    block->nextBlock = to;
    to->prevBlock = block;
    deleteedge(from, to);
    insertedge(from, block);
    insertedge(block, to);
    return block;
}

PCLink* insertedge(PCodeBlock* from, PCodeBlock* to) {
    PCLink* link;
    int hash;

    hash = ((from->blockIndex << 8) ^ to->blockIndex) % PCLINK_HASH_SIZE;
    for (link = pclinkhash[hash]; link; link = link->nextHash) {
        if (link->from == from && link->to == to) {
            return link;
        }
    }
    link = lalloc(sizeof(PCLink));
    memclrw(link, sizeof(PCLink));
    link->from = from;
    link->to = to;
    link->index = pclinkcount++;
    link->hash = hash;
    link->nextHash = pclinkhash[hash];
    pclinkhash[hash] = link;
    link->nextSuccessor = from->successors;
    from->successors = link;
    link->nextPredecessor = to->predecessors;
    to->predecessors = link;
    return link;
}

//! @reconstructed
static PCLink* findlink(PCodeBlock* from, PCodeBlock* to) {
    PCLink* link;

    for (link = pclinkhash[((from->blockIndex << 8) ^ to->blockIndex) % PCLINK_HASH_SIZE]; link;
         link = link->nextHash) {
        if (link->from == from && link->to == to) {
            return link;
        }
    }
    return NULL;
}

void deleteedge(PCodeBlock* from, PCodeBlock* to) {
    PCLink* link;
    PCLink** ptr;

    link = findlink(from, to);
    CError_ASSERT(934, link);
    for (ptr = &to->predecessors; *ptr != link; ptr = &(*ptr)->nextPredecessor)
        ;
    *ptr = link->nextPredecessor;
    for (ptr = &from->successors; *ptr != link; ptr = &(*ptr)->nextSuccessor)
        ;
    *ptr = link->nextSuccessor;
    for (ptr = &pclinkhash[link->hash]; *ptr != link; ptr = &(*ptr)->nextHash)
        ;
    *ptr = link->nextHash;
}

#pragma opt_lifetimes off
//! @reconstructed
void optimize_tail_calls(void) {
    PCodeBlock* block;
    PCodeBlock* prev;
    PCodeBlock* from;
    PCLink* link;
    PCode* call;
    PCode* scan;
    PCode* pcode;
    UInt16 op;
    int count;
    SInt32 i;
    int n;
    Boolean argreg;
    Boolean stackreg;
    PCodeArg temp;

    if ((binary_word_count_destination->gprmask & 0x40FF0000) ||
        (binary_word_count_destination->fprmask & 0xFFF00000) ||
        codegen_flag_637383 ||
        codegen_flag_63738A) {
        return;
    }

    block = pcbasicblocks;
    for (;;) {
        if (!block->nextBlock) {
            break;
        }
        block = block->nextBlock;
    }
    if (!block) {
        return;
    }
    if (block->firstPCode == NULL) {
        link = block->predecessors;
        if (!link || link->nextPredecessor) {
            return;
        }
        if (!(block = link->from)) {
            return;
        }
    }
    if (!(call = block->lastPCode)) {
        return;
    }

    if (call->op == PC_END && !(call = call->prevPCode)) {
        link = block->predecessors;
        if ((from = link->from)) {
            prev = block->prevBlock;
            if (from->blockIndex == prev->blockIndex) {
                if (!(call = prev->lastPCode)) {
                    return;
                }
            } else {
                if (from->blockIndex == prev->blockIndex ||
                    !link->nextPredecessor ||
                    link->nextPredecessor->from->blockIndex != prev->blockIndex) {
                    return;
                }
                if (!(call = prev->lastPCode)) {
                    return;
                }
            }
        } else {
            return;
        }
    }

    op = call->op;
    if (op != PC_JAL && op != PC_JALR && op != PC_BAL) {
        return;
    }

    {
        PCodeBlock** walker;
        for (walker = &pcbasicblocks; *walker; walker = &(*walker)->nextBlock) {
            block = *walker;
            for (pcode = block->firstPCode; pcode; pcode = pcode->nextPCode) {
                if ((pcode->flags & 8) && pcode != call) {
                    return;
                }
            }
        }
    }

    count = 0;
    scan = call;
    for (; count <= 10; count++) {
        if (!(pcode = scan->prevPCode)) {
            break;
        }
        scan = pcode;
        if (scan->op == PC_LA) {
            argreg = 0;
            stackreg = 0;
            n = scan->argCount;
            for (i = 0; i < n; i++) {
                if (scan->args[i].kind == 0 && scan->args[i].arg == 0) {
                    if ((SInt32)scan->args[i].data.reg.reg >= 4 &&
                        (SInt32)scan->args[i].data.reg.reg <= 7) {
                        argreg = 1;
                    } else if ((SInt32)scan->args[i].data.reg.reg == 29) {
                        stackreg = 1;
                    }
                }
            }

            if (argreg == 1 && stackreg == 1) {
                return;
            }
        }
    }

    if (op == PC_JAL) {
        call->op = PC_J;
    }
    if (call->op == PC_BAL) {
        call->op = PC_B;
    } else if (call->op == PC_JALR) {
        call->op = PC_JR;
        temp = call->args[0];
        call->args[0] = call->args[1];
        call->args[1] = temp;
    }
    call->flags |= 0x80000;
    made_tail_call = 1;
    binary_word_count_destination->gprmask &= ~0x80000000;
}

#pragma opt_lifetimes on
