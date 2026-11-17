#include "compiler/common.h"
#include "compiler/Registers.h"
#include "compiler/RegisterInfo.h"
#include "compiler/CompilerTools.h"
#include "compiler/PCode.h"
#include "compiler/PCodeInfo.h"
#include "compiler/InterferenceGraph.h"
#include "compiler/RegListQueries.h"
#include "compiler/objects.h"
#include "compiler/CError.h"
#include "compiler/CParser.h"
#include "compiler/LiveRegisters.h"
#include "compiler/Peephole.h"

SInt32 interferes(UInt32 lhs, UInt32 rhs) {
    IGEdge* edge;
    UInt32 index;

    if (lhs < rhs) {
        index = ((((rhs - 1) * rhs) >> 1) + lhs) & 0x3FFF;
        for (edge = interference_hash[index]; edge; edge = edge->next) {
            if (edge->low == lhs && edge->high == rhs) {
                return TRUE;
            }
        }
    } else if (lhs > rhs) {
        index = ((((lhs - 1) * lhs) >> 1) + rhs) & 0x3FFF;
        for (edge = interference_hash[index]; edge; edge = edge->next) {
            if (edge->high == lhs && edge->low == rhs) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void makeinterfere(UInt32 lhs, UInt32 rhs) {
    IGEdge* edge;
    IGNode* node1;
    IGNode* node2;
    UInt32 index;

    if (lhs < rhs) {
        index = ((((rhs - 1) * rhs) >> 1) + lhs) & 0x3FFF;
        for (edge = interference_hash[index]; edge; edge = edge->next) {
            if (edge->low == lhs && edge->high == rhs) {
                return;
            }
        }
        node1 = &interferencegraph[lhs];
        node2 = &interferencegraph[rhs];
        edge = oalloc(sizeof(IGEdge));
        edge->next = interference_hash[index];
        interference_hash[index] = edge;
        edge->nextLow = node1->edges;
        node1->edges = edge;
        edge->nextHigh = node2->edges;
        node2->edges = edge;
        edge->low = lhs;
        edge->high = rhs;
        node1->liveEdges++;
        node2->liveEdges++;
        node1->degree++;
        node2->degree++;
        if ((node1->flags & 0x200) || (node2->flags & 0x200)) {
            node1->degree++;
            node2->degree++;
        }
    } else if (lhs > rhs) {
        index = ((((lhs - 1) * lhs) >> 1) + rhs) & 0x3FFF;
        for (edge = interference_hash[index]; edge; edge = edge->next) {
            if (edge->high == lhs && edge->low == rhs) {
                return;
            }
        }
        node1 = &interferencegraph[lhs];
        node2 = &interferencegraph[rhs];
        edge = oalloc(sizeof(IGEdge));
        edge->next = interference_hash[index];
        interference_hash[index] = edge;
        edge->nextLow = node2->edges;
        node2->edges = edge;
        edge->nextHigh = node1->edges;
        node1->edges = edge;
        edge->low = rhs;
        edge->high = lhs;
        node1->liveEdges++;
        node2->liveEdges++;
        node1->degree++;
        node2->degree++;
        if ((node1->flags & 0x200) || (node2->flags & 0x200)) {
            node1->degree++;
            node2->degree++;
        }
    }
}

void fn_004BE090(void) {
    UInt32 regs;
    UInt32 i;

    coloring_class = 0;
    do {
        regs = used_virtual_registers[coloring_class];
        if (regs != n_real_registers[coloring_class]) {
            coalesced = lalloc(regs * sizeof(SInt16));
            for (i = 0; i < regs; i++) {
                coalesced[i] = i;
            }
            do {
                freeoheap();
                fn_004BEE10();
                fn_004BE9B0();
            } while (fn_004BE5C0());
            fn_004BE350();
            fn_004BE4B0();
            freeoheap();
        }
        coloring_class++;
    } while (coloring_class < 7);
}

void buildinterferencegraph(Object* proc) {
    UInt32 regs;
    UInt32 i;

    regs = used_virtual_registers[coloring_class];
    coalesced = lalloc(regs * sizeof(SInt16));
    for (i = 0; i < regs; i++) {
        coalesced[i] = i;
    }
    do {
        freeoheap();
        fn_004BEE10();
        fn_004BE9B0();
    } while (fn_004BE5C0());
    fn_004BE350();
    fn_004BE4B0();
    fn_004BE240();
}

void fn_004BE240(void) {
    UInt32 i;
    PCodeBlock* block;
    PCode* instr;
    PCodeArg* arg;
    UInt32 reg;
    UInt32 regs;
    IGNode* node;

    regs = used_virtual_registers[coloring_class];
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (instr = block->lastPCode; instr; instr = instr->prevPCode) {
            for (i = (SInt16)instr->argCount, arg = instr->args; i--; arg++) {
                if (arg->kind != PCOp_REGISTER ||
                    arg->arg != *(volatile UInt8*)&coloring_class ||
                    !(arg->effect & 2) ||
                    (SInt16)arg->data.reg.reg < n_real_registers[coloring_class]) {
                    continue;
                }
                reg = (SInt16)arg->data.reg.reg;
                node = &interferencegraph[reg];
                if (node->flags & 0x30 || node->flags & 0x40) {
                    continue;
                }
                if (!node->instr) {
                    node->instr = instr;
                } else {
                    node->instr = NULL;
                    node->flags |= 0x40;
                }
            }
        }
    }
    for (i = 0; i < regs; i++) {
        node = &interferencegraph[i];
        if (node->instr && !is_location_independent(node->instr)) {
            node->instr = NULL;
        }
    }
}

void fn_004BE350(void) {
    Object* record;
    VarInfo* pair;
    ObjectList* list;
    UInt32 i;
    for (i = 0; i < n_real_registers[coloring_class]; i++) {
        interferencegraph[i].color = i;
    }
    for (list = ig_reglist_a; list; list = list->next) {
        record = list->object;
        pair = record->u.var.info;
        if (record->datatype == DLOCAL && pair->reg != 0 && pair->rclass == coloring_class) {
            interferencegraph[(SInt16)pair->reg].spillTemporary = record;
            if (record->datatype == DLOCAL && record->u.var.info->regHi != 0) {
                interferencegraph[(SInt16)pair->reg].flags |= 0x20;
                interferencegraph[(SInt16)pair->regHi].spillTemporary = record;
                interferencegraph[(SInt16)pair->regHi].flags |= 0x10;
            }
        }
    }
    for (list = ig_reglist_b; list; list = list->next) {
        record = list->object;
        pair = record->u.var.info;
        if (record->datatype == DLOCAL && pair->reg != 0 && pair->rclass == coloring_class) {
            interferencegraph[(SInt16)pair->reg].spillTemporary = record;
            if (record->datatype == DLOCAL && record->u.var.info->regHi != 0) {
                interferencegraph[(SInt16)pair->reg].flags |= 0x20;
                interferencegraph[(SInt16)pair->regHi].spillTemporary = record;
                interferencegraph[(SInt16)pair->regHi].flags |= 0x10;
            }
        }
    }
}

void fn_004BE4B0(void) {
    UInt32 regs;
    UInt32 i;
    UInt32 offset;
    IGNode* node;
    SInt16 reg;
    Object* record;

    regs = used_virtual_registers[coloring_class];
    i = 0;
    if (regs != 0) {
        offset = 0;
        do {
            node = (IGNode*)((UInt8*)interferencegraph + offset);
            if (coalesced[i] != i) {
                node->flags |= 4;
                reg = i;
                if ((UInt32)(SInt16)reg > (UInt32)used_virtual_registers[coloring_class]) {
                    CError_Internal(ig_source_file, 400);
                }
                while (reg != coalesced[reg]) {
                    reg = coalesced[reg];
                }
                interferencegraph[reg].flags |= 8;
                if ((UInt32)(SInt16)reg < (UInt32)n_real_registers[coloring_class]) {
                    node->color = reg;
                }
            }
            i++;
            offset += 32;
        } while (i < regs);
    }
    i = 0;
    if (i < regs) {
        offset = 0;
        do {
            node = (IGNode*)((UInt8*)interferencegraph + offset);
            if (node->spillTemporary && (node->flags & 8)) {
                record = node->spillTemporary;
                record->u.var.info->flags |= 0x10;
            }
            i++;
            offset += 32;
        } while (i < regs);
    }
}

void fn_004BEE10(void) {
    UInt32 regs;
    UInt32 i;
    IGNode* node;

    regs = used_virtual_registers[coloring_class];
    interferencegraph = oalloc(regs * sizeof(IGNode));
    for (i = 0; i < regs; i++) {
        node = &interferencegraph[i];
        node->next = NULL;
        node->spillTemporary = NULL;
        node->instr = NULL;
        node->spillCost = 0;
        node->reg = i;
        node->color = -1;
        node->flags = 0;
        node->degree = 0;
        node->liveEdges = 0;
        node->edges = NULL;
    }
    for (i = 0; i < 16384; i++) {
        interference_hash[i] = NULL;
    }
}

SInt32 fn_004BE5C0(void) {
    PCodeBlock* block;
    PCode* instr;
    PCodeArg* arg;
    PCodeArg* first;
    PCodeArg* second;
    IGEdge* edge;
    UInt32 i;
    SInt16 src;
    SInt16 dst;
    SInt16 low;
    SInt16 high;
    SInt16 other;
    SInt32 changed;

    changed = FALSE;
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (instr = block->firstPCode; instr; instr = instr->nextPCode) {
            if (!(instr->flags & 0x10)) {
                continue;
            }
            if (instr->op > 0x21E && instr->op <= 0x2E7) {
                first = &instr->args[2];
            } else {
                first = &instr->args[1];
            }
            if ((UInt8)coloring_class != first->arg) {
                continue;
            }
            src = first->data.reg.reg;
            if ((UInt32)(SInt16)src > (UInt32)used_virtual_registers[coloring_class]) {
                CError_Internal(ig_source_file, 400);
            }
            while (src != coalesced[src]) {
                src = coalesced[src];
            }
            if (instr->op > 0x21E && instr->op <= 0x2E7) {
                second = &instr->args[1];
            } else {
                second = &instr->args[0];
            }
            dst = second->data.reg.reg;
            if ((UInt32)(SInt16)dst > (UInt32)used_virtual_registers[coloring_class]) {
                CError_Internal(ig_source_file, 400);
            }
            while (dst != coalesced[dst]) {
                dst = coalesced[dst];
            }
            if (src == dst) {
                deletepcode(instr);
                continue;
            }
            if (interferes(src, dst) || fn_00472800(src, dst)) {
                continue;
            }
            low = src < dst ? src : dst;
            high = src > dst ? src : dst;
            if (ig_graph_positions[high] < ig_graph_count &&
                ig_graph_ids[ig_graph_positions[high]] == high) {
                continue;
            }
            coalesced[high] = low;
            for (edge = interferencegraph[high].edges; edge;) {
                if (interferencegraph[high].reg < edge->high) {
                    other = edge->high;
                    makeinterfere(low, other);
                    edge = edge->nextLow;
                } else {
                    other = edge->low;
                    makeinterfere(low, other);
                    edge = edge->nextHigh;
                }
            }
            deletepcode(instr);
            changed = TRUE;
            if (copts.debuglisting) {
                fn_004BB0A0(low, high);
            }
        }
    }
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (instr = block->firstPCode; instr; instr = instr->nextPCode) {
            for (i = (SInt16)instr->argCount, arg = instr->args; i--; arg++) {
                if (arg->kind == PCOp_REGISTER &&
                    arg->arg == (UInt8)coloring_class &&
                    arg->data.reg.reg != coalesced[(SInt16)arg->data.reg.reg]) {
                    src = arg->data.reg.reg;
                    if ((UInt32)(SInt16)src > (UInt32)used_virtual_registers[coloring_class]) {
                        CError_Internal(ig_source_file, 400);
                    }
                    while (src != coalesced[src]) {
                        src = coalesced[src];
                    }
                    arg->data.reg.reg = src;
                }
            }
        }
    }
    return changed;
}

void fn_004BE9B0(void) {
    UInt32 regs;
    UInt32 i;
    UInt32 j;
    UInt32 count;
    UInt32* active;
    UInt32* positions;
    UInt32* bitmap;
    PCodeBlock* block;
    PCode* instr;
    PCodeArg* arg;
    SInt16 reg;
    SInt32 cls;
    SInt32 start;
    UInt32 k;

    cls = coloring_class;
    regs = used_virtual_registers[cls];
    ig_graph_size = regs;
    ig_graph_count = 0;
    ig_graph_ids = oalloc(regs * sizeof(UInt32));
    ig_graph_positions = oalloc(regs * sizeof(UInt32));
    for (i = 0; i < (UInt32)n_real_registers[cls]; i++) {
        for (j = 0; j < (UInt32)n_real_registers[cls]; j++) {
            if (i != j) {
                makeinterfere(i, j);
            }
        }
    }
    compute_live_registers_of_class(coloring_class);
    active = oalloc(regs * sizeof(UInt32));
    positions = oalloc(regs * sizeof(UInt32));
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        bitmap = register_live_bitmaps_6368D0[coloring_class].blockBitmaps[block->blockIndex].out;
        count = 0;
        for (i = 0; i < regs; i++) {
            if ((bitmap[i >> 5] >> i) & 1) {
                positions[i] = count;
                active[count++] = i;
            }
        }
        for (instr = block->lastPCode; instr; instr = instr->prevPCode) {
            for (i = (SInt16)instr->argCount, arg = instr->args; i--; arg++) {
                if (arg->kind == PCOp_REGISTER &&
                    arg->arg == (UInt8)coloring_class &&
                    (arg->effect & 2)) {
                    for (j = 0; j < count; j++) {
                        if ((instr->flags & 0x10) &&
                            active[j] ==
                                (SInt16)((instr->op > 0x21E && instr->op <= 0x2E7)
                                             ? instr->args[2].data.reg.reg
                                             : instr->args[1].data.reg.reg)) {
                            continue;
                        }
                        makeinterfere((SInt16)arg->data.reg.reg, active[j]);
                    }
                }
            }
            for (i = (SInt16)instr->argCount, arg = instr->args; i--; arg++) {
                if (arg->kind == PCOp_REGISTER &&
                    arg->arg == (UInt8)coloring_class &&
                    (arg->effect & 2)) {
                    reg = arg->data.reg.reg;
                    j = positions[reg];
                    if (j < count && active[j] == reg) {
                        count--;
                        active[j] = active[count];
                        positions[active[j]] = j;
                    }
                }
            }
            for (i = (SInt16)instr->argCount, arg = instr->args; i--; arg++) {
                if (arg->kind == PCOp_REGISTER &&
                    arg->arg == (UInt8)coloring_class &&
                    (arg->effect & 1)) {
                    reg = arg->data.reg.reg;
                    j = positions[reg];
                    if (j >= count || active[j] != reg) {
                        arg->effect |= 4;
                    }
                    j = positions[reg];
                    if (j >= count || active[j] != reg) {
                        positions[reg] = count;
                        active[count++] = reg;
                    }
                }
            }
            if (instr->flags & 8) {
                start = fn_004727F0();
                start = fn_0046D3F0() + 2 - start;
                for (i = start, arg = &instr->args[start]; i < (UInt32)(SInt16)instr->argCount;
                     i++, arg++) {
                    if (arg->arg != (UInt8)coloring_class) {
                        continue;
                    }
                    for (k = 0; k < (UInt32)n_scratch_registers[coloring_class]; k++) {
                        makeinterfere(
                            (SInt16)arg->data.reg.reg, scratch_registers[coloring_class][k]
                        );
                    }
                    reg = arg->data.reg.reg;
                    j = ig_graph_positions[reg];
                    if (j >= ig_graph_count || ig_graph_ids[j] != reg) {
                        ig_graph_positions[reg] = ig_graph_count;
                        ig_graph_ids[ig_graph_count++] = reg;
                    }
                }
            }
        }
    }
}
