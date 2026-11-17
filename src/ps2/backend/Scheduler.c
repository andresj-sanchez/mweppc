#include "compiler/common.h"
#include "compiler/CompilerTools.h"
#include "compiler/Alias.h"
#include "compiler/RegisterInfo.h"
#include "compiler/Registers.h"
#include "compiler/Scheduler.h"
#include "compiler/LiveRegisters.h"

void fn_004C0510(MachineInfo* machine) {
    scheduler_machine_info = machine;
}

#include "compiler/PCode.h"
#include "compiler/CError.h"

void fn_004C13B0(PCode* instr) {
    SInt32 cls;
    UInt16 reg;
    PCodeArg* arg;
    UInt32 i;

    for (i = (SInt16)instr->argCount, arg = instr->args; i--; arg++) {
        if (arg->kind == PCOp_REGISTER && (arg->effect & 1)) {
            cls = (SInt8)arg->arg;
            reg = arg->data.reg.reg;
            if ((SInt16)reg >= n_real_registers[cls] ||
                scheduler_register_colors[cls][(SInt16)reg] != 2) {
                if (--scheduler_register_counts[cls][(SInt16)reg] == 0) {
                    scheduler_available_registers[(SInt8)arg->arg]--;
                }
            }
        }
    }
    for (i = (SInt16)instr->argCount, arg = instr->args; i--; arg++) {
        if (arg->kind == PCOp_REGISTER && (arg->effect & 2)) {
            cls = (SInt8)arg->arg;
            reg = arg->data.reg.reg;
            if ((SInt16)reg >= n_real_registers[cls] ||
                scheduler_register_colors[cls][(SInt16)reg] != 2) {
                if (scheduler_register_counts[cls][(SInt16)reg] && !(arg->effect & 1)) {
                    scheduler_available_registers[cls]++;
                }
            }
        }
    }
}

void fn_004C0F60(void) {
    volatile SInt8 cls;
    SInt32 i;

    for (cls = 0; cls < 7; cls++) {
        SInt32 bank = cls;
        scheduler_register_uses[bank] = oalloc(used_virtual_registers[bank] * sizeof(void*));
        scheduler_register_defs[bank] = oalloc(used_virtual_registers[bank] * sizeof(void*));
        for (i = 0; i < used_virtual_registers[bank]; i++) {
            scheduler_register_uses[bank][i] = scheduler_register_defs[bank][i] = NULL;
        }
    }
    scheduler_memory_uses = oalloc(128);
    scheduler_memory_defs = oalloc(128);
    for (i = 0; i < 32; i++) {
        scheduler_memory_uses[i] = scheduler_memory_defs[i] = NULL;
    }
    scheduler_memory_dep_defs = scheduler_memory_dep_uses = NULL;
    scheduler_volatile_defs = scheduler_volatile_uses = scheduler_barriers = NULL;
    scheduler_mem_writes = NULL;
    scheduler_mem_reads = NULL;
    scheduler_critical_path = 0;
}

void fn_004C14A0(PCodeBlock* block) {
    volatile SInt8 cls;
    SInt32 i;
    PCode* instr;
    PCodeArg* arg;
    SInt32 count;

    for (cls = 0; cls < 7; cls++) {
        SInt8 bank = cls;
        scheduler_register_classes[bank] = available_registers(cls);
        scheduler_available_registers[bank] = 0;
        for (i = 0; i < used_virtual_registers[bank]; i++) {
            if ((register_live_bitmaps_6368D0[bank].blockBitmaps[block->blockIndex].in[i >> 5] >>
                 i) &
                1) {
                if (i >= n_real_registers[bank] || scheduler_register_colors[bank][i] != 2) {
                    scheduler_available_registers[bank]++;
                }
            }
        }
    }
    for (cls = 0; cls < 7; cls++) {
        SInt8 bank = cls;
        scheduler_register_counts[bank] = oalloc(used_virtual_registers[bank] * sizeof(SInt32));
        for (i = 0; i < used_virtual_registers[bank]; i++) {
            scheduler_register_counts[bank][i] =
                (register_live_bitmaps_6368D0[bank].blockBitmaps[block->blockIndex].out[i >> 5] >>
                 i) &
                    1
                ? 1
                : 0;
        }
    }
    for (instr = block->firstPCode; instr; instr = instr->nextPCode) {
        count = (SInt16)instr->argCount;
        arg = instr->args;
        while (count--) {
            if (arg->kind == PCOp_REGISTER && (arg->effect & 1)) {
                scheduler_register_counts[(SInt8)arg->arg][(SInt16)arg->data.reg.reg]++;
            }
            arg++;
        }
    }
}

SInt32 fn_004C1160(PCode* instr) {
    PCodeArg* arg;
    SInt32 writes;
    SInt32 i;
    SInt32 reads;

    if (instr->flags & 0x10) {
        return 0;
    }
    if (instr->flags & 2) {
        return 3;
    }
    writes = reads = 0;
    arg = instr->args;
    i = (SInt16)instr->argCount;
    while (i--) {
        if (arg->kind == PCOp_REGISTER) {
            if (arg->effect & 2) {
                writes++;
            }
            if (arg->effect & 1) {
                reads++;
            }
        }
        arg++;
    }
    if (writes) {
        if (!reads) {
            return 4;
        }
        return 2;
    }
    return 1;
}

SInt32 fn_004C11F0(PCode* instr) {
    PCodeArg* arg;
    SInt32 i;
    SInt32 cls;
    SInt16 reg;
    SInt32 score = 16;
    SInt32 available;

    arg = instr->args;
    i = (SInt16)instr->argCount;
    while (i--) {
        if (arg->kind == PCOp_REGISTER && (arg->effect & 1)) {
            scheduler_register_counts[(SInt8)arg->arg][(SInt16)arg->data.reg.reg]--;
        }
        arg++;
    }
    for (arg = instr->args, i = (SInt16)instr->argCount; i--; arg++) {
        if (arg->kind == PCOp_REGISTER && (arg->effect & 1)) {
            cls = (SInt8)arg->arg;
            reg = arg->data.reg.reg;
            if ((SInt16)reg >= n_real_registers[cls] ||
                scheduler_register_colors[cls][(SInt16)reg] != 2) {
                if (scheduler_register_counts[cls][(SInt16)reg] == 0) {
                    available =
                        scheduler_available_registers[cls] - scheduler_register_classes[cls];
                    score--;
                    available = available < 2 ? 1 : available;
                    score -= available;
                }
            }
        }
    }
    for (arg = instr->args, i = (SInt16)instr->argCount; i--; arg++) {
        if (arg->kind == PCOp_REGISTER && (arg->effect & 2)) {
            cls = (SInt8)arg->arg;
            reg = arg->data.reg.reg;
            if ((SInt16)reg >= n_real_registers[cls] ||
                scheduler_register_colors[cls][(SInt16)reg] != 2) {
                if (scheduler_register_counts[cls][(SInt16)reg] && !(arg->effect & 1)) {
                    available =
                        scheduler_available_registers[cls] - scheduler_register_classes[cls];
                    available = available < 2 ? 1 : available;
                    score += available;
                }
            }
        }
    }
    for (arg = instr->args, i = (SInt16)instr->argCount; i--; arg++) {
        if (arg->kind == PCOp_REGISTER && (arg->effect & 1)) {
            scheduler_register_counts[(SInt8)arg->arg][(SInt16)arg->data.reg.reg]++;
        }
    }
    return score;
}

void fn_004C0EC0(SchedulerDGNode* node, SchedulerDGNode* successor, Boolean hasLatency) {
    SchedulerDGSuccessor* scan;
    SchedulerDGSuccessor* entry;
    UInt16 latency;
    void (*hook)(PCode*, PCode*);
    SInt32 length;

    hook = scheduler_machine_info->dependence;
    if (hook) {
        hook(node->instr, successor->instr);
    }
    for (scan = node->successors; scan; scan = scan->next) {
        if (scan->node == successor) {
            return;
        }
    }
    entry = oalloc(sizeof(SchedulerDGSuccessor));
    entry->node = successor;
    entry->next = node->successors;
    node->successors = entry;
    if (hasLatency) {
        latency = node->latency;
    } else {
        latency = 0;
    }
    entry->latency = latency;
    successor->predCount++;
    length = (UInt16)successor->height + (UInt16)entry->latency;
    if (length > node->height) {
        node->height = length;
    }
}

void fn_004C0D10(SchedulerDGNode* node) {
    SchedulerDGNodeList* list;

    for (list = (SchedulerDGNodeList*)scheduler_memory_dep_defs; list; list = list->next) {
        if (may_alias(node->instr, list->node->instr)) {
            fn_004C0EC0(node, list->node, TRUE);
        }
    }
    for (list = (SchedulerDGNodeList*)scheduler_memory_dep_uses; list; list = list->next) {
        if (may_alias(node->instr, list->node->instr)) {
            fn_004C0EC0(node, list->node, TRUE);
        }
    }
    list = oalloc(sizeof(SchedulerDGNodeList));
    list->node = node;
    list->next = (SchedulerDGNodeList*)scheduler_memory_dep_uses;
    scheduler_memory_dep_uses = list;
    if (node->instr->flags & 0x40000) {
        list = oalloc(sizeof(SchedulerDGNodeList));
        list->node = node;
        list->next = (SchedulerDGNodeList*)scheduler_memory_dep_defs;
        scheduler_memory_dep_defs = list;
    }
}

void fn_004C0DC0(
    SInt32 hasHazards,
    SchedulerDGNode* node,
    SchedulerDGNodeList** defs,
    SchedulerDGNodeList** uses,
    SInt32 isWrite
) {
    SchedulerDGNodeList* list;
    SInt32 hasLatency;

    if (isWrite) {
        for (list = *uses; list; list = list->next) {
            if (list->node != node) {
                fn_004C0EC0(node, list->node, TRUE);
            }
        }
        for (list = *defs; list; list = list->next) {
            if (list->node != node) {
                hasLatency = hasHazards || !scheduler_machine_info->renames_registers;
                fn_004C0EC0(node, list->node, hasLatency);
            }
        }
        list = *defs;
        if (list) {
            list->node = node;
        } else {
            list = oalloc(sizeof(SchedulerDGNodeList));
            list->node = node;
            list->next = *defs;
            *defs = list;
        }
        *uses = NULL;
    } else {
        for (list = *defs; list; list = list->next) {
            if (list->node != node) {
                hasLatency = hasHazards || !scheduler_machine_info->renames_registers;
                fn_004C0EC0(node, list->node, hasLatency);
            }
        }
        list = oalloc(sizeof(SchedulerDGNodeList));
        list->node = node;
        list->next = *uses;
        *uses = list;
    }
}

SchedulerDGNode* fn_004C0740(SchedulerDGNode* nodes, UInt16 clock) {
    SchedulerDGNode* best;
    SchedulerDGNode* candidate;
    SchedulerDGNodeList* list;
    SchedulerDGSuccessor* succ;
    SInt32 bestScore;
    SInt32 score;
    SInt32 bestSuccessors;
    SInt32 candidateSuccessors;
    SInt8 cls;
    SInt8 hasRegisters;

    for (best = nodes; best; best = best->next) {
        if (!best->predCount &&
            best->latest <= clock &&
            scheduler_machine_info->can_issue(best->instr)) {
            break;
        }
    }
    if (!best) {
        return NULL;
    }
    if (coloring) {
        for (cls = 0; cls < 7; cls++) {
            if (scheduler_available_registers[cls] >= scheduler_register_classes[cls]) {
                hasRegisters = TRUE;
                goto have_registers;
            }
        }
        hasRegisters = FALSE;
have_registers:
        if (hasRegisters) {
            bestScore = fn_004C11F0(best->instr);
            for (candidate = best->next; candidate; candidate = candidate->next) {
                if (!candidate->predCount &&
                    candidate->latest <= clock &&
                    scheduler_machine_info->can_issue(candidate->instr)) {
                    score = fn_004C11F0(candidate->instr);
                    if (score < bestScore) {
                        bestScore = fn_004C11F0(candidate->instr);
                        best = candidate;
                    }
                }
            }
            return best;
        }
    }
    for (candidate = best->next; candidate; candidate = candidate->next) {
        if (candidate->predCount ||
            candidate->latest > clock ||
            !scheduler_machine_info->can_issue(candidate->instr)) {
            continue;
        }
        if (best->unk16 <= clock) {
            if (candidate->unk16 > clock) {
                continue;
            }
        }
        if (best->unk16 > clock) {
            if (candidate->unk16 <= clock) {
                best = candidate;
                continue;
            }
        }
        candidateSuccessors = 0;
        for (succ = candidate->successors; succ; succ = succ->next) {
            if (succ->node->predCount == 1) {
                candidateSuccessors++;
            }
        }
        bestSuccessors = 0;
        for (succ = best->successors; succ; succ = succ->next) {
            if (succ->node->predCount == 1) {
                bestSuccessors++;
            }
        }
        if (bestSuccessors <= candidateSuccessors) {
            if (bestSuccessors < candidateSuccessors) {
                best = candidate;
            } else if (best->height <= candidate->height) {
                if (best->height < candidate->height) {
                    best = candidate;
                } else if (coloring && best->earliest > candidate->earliest) {
                    best = candidate;
                }
            }
        }
    }
    return best;
}

void fn_004C0520(PCodeBlock* block) {
    PCode* instr;
    PCode* scheduled;
    SchedulerDGNode* node;
    SchedulerDGNode* old;
    SchedulerDGSuccessor* successor;
    UInt16 cycle;
    UInt16 criticalPath;
    SInt32 issueCount;
    SInt32 deadline;

    scheduler_machine_info->initialize();
    fn_004C0F60();
    fn_004C14A0(block);
    scheduler_default_successor = NULL;
    scheduler_all_nodes = NULL;
    for (instr = block->lastPCode; instr; instr = instr->prevPCode) {
        node = oalloc(sizeof(SchedulerDGNode));
        node->next = NULL;
        node->prev = NULL;
        node->successors = NULL;
        node->instr = instr;
        node->height = scheduler_machine_info->latency(instr);
        node->latency = node->height;
        node->earliest = fn_004C1160(instr);
        node->latest = 0;
        node->unk16 = 0;
        node->predCount = 0;
        fn_004C0920(node);
        if (instr->flags & 1) {
            scheduler_default_successor = node;
        }
        if (scheduler_all_nodes) {
            scheduler_all_nodes->prev = node;
        }
        node->next = scheduler_all_nodes;
        scheduler_all_nodes = node;
    }
    criticalPath = scheduler_critical_path;
    for (node = scheduler_all_nodes; node; node = node->next) {
        node->unk16 = criticalPath - node->height;
    }
    block->lastPCode = NULL;
    block->firstPCode = block->lastPCode;
    block->pcodeCount = 0;
    cycle = 0;
    while (scheduler_all_nodes) {
        issueCount = 0;
        while ((UInt16)issueCount < scheduler_machine_info->issue_width) {
            if (!scheduler_all_nodes) {
                break;
            }
            node = fn_004C0740(scheduler_all_nodes, cycle);
            if (!node) {
                break;
            }
            scheduled = node->instr;
            successor = node->successors;
            if (successor) {
                for (; successor; successor = successor->next) {
                    successor->node->predCount--;
                    deadline = (UInt16)successor->latency + (UInt16)cycle;
                    if (successor->node->latest < deadline) {
                        successor->node->latest = deadline;
                    }
                }
            }
            appendpcode(block, scheduled);
            scheduler_machine_info->issue(scheduled);
            fn_004C13B0(scheduled);
            if (node->prev) {
                node->prev->next = node->next;
            } else {
                scheduler_all_nodes = node->next;
            }
            if (node->next) {
                node->next->prev = node->prev;
            }
            issueCount++;
        }
        scheduler_machine_info->advance_clock();
        cycle++;
    }
}

void fn_004C0920(SchedulerDGNode* node) {
    PCode* instr;
    PCodeArg* arg;
    SInt32 i;
    SInt8 cls;
    SInt32 reg;
    SchedulerDGNodeList* list;
    SchedulerDGNodeList* reads;
    SchedulerDGNodeList* barriers;
    SchedulerDGNodeList* entry;
    SchedulerDGNode* other;

    instr = node->instr;
    for (i = 0, arg = instr->args; i < (SInt16)instr->argCount; i++, arg++) {
        switch (arg->kind) {
            case 0:
                if (arg->effect & 2) {
                    cls = (SInt8)arg->arg;
                    reg = (SInt16)arg->data.reg.reg;
                    fn_004C0DC0(
                        FALSE,
                        node,
                        &scheduler_register_defs[cls][reg],
                        &scheduler_register_uses[cls][reg],
                        arg->effect & 2
                    );
                }
                break;
            case 1:
                reg = (SInt16)arg->data.reg.reg;
                fn_004C0DC0(
                    TRUE,
                    node,
                    &scheduler_memory_defs[reg],
                    &scheduler_memory_uses[reg],
                    arg->effect & 2
                );
                break;
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                break;
            default:
                CError_Internal(scheduler_source_file, 0x303);
                break;
        }
    }
    for (i = 0, arg = instr->args; i < (SInt16)instr->argCount; i++, arg++) {
        switch (arg->kind) {
            case 0:
                if (arg->effect & 1) {
                    cls = (SInt8)arg->arg;
                    reg = (SInt16)arg->data.reg.reg;
                    fn_004C0DC0(
                        FALSE,
                        node,
                        &scheduler_register_defs[cls][reg],
                        &scheduler_register_uses[cls][reg],
                        arg->effect & 2
                    );
                }
                break;
            case 1:
                reg = (SInt16)arg->data.reg.reg;
                fn_004C0DC0(
                    TRUE,
                    node,
                    &scheduler_memory_defs[reg],
                    &scheduler_memory_uses[reg],
                    arg->effect & 2
                );
                break;
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                break;
            default:
                CError_Internal(scheduler_source_file, 0x31A);
                break;
        }
    }
    if (instr->flags & 0x20002) {
        for (list = scheduler_memory_dep_uses; list; list = list->next) {
            if (may_alias(node->instr, list->node->instr)) {
                fn_004C0EC0(node, list->node, TRUE);
            }
        }
        entry = oalloc(sizeof(SchedulerDGNodeList));
        entry->node = node;
        entry->next = scheduler_memory_dep_defs;
        scheduler_memory_dep_defs = entry;
    } else if (instr->flags & 0x40004) {
        fn_004C0D10(node);
    }
    if (instr->flags & 0x80) {
        for (reads = scheduler_mem_reads; reads; reads = reads->next) {
            if (reads->node != node) {
                fn_004C0EC0(node, reads->node, FALSE);
            }
        }
        entry = oalloc(sizeof(SchedulerDGNodeList));
        entry->node = node;
        entry->next = scheduler_mem_reads;
        scheduler_mem_reads = entry;
    }
    if (instr->flags & 0x200) {
        for (barriers = scheduler_barriers; barriers; barriers = barriers->next) {
            if (barriers->node != node) {
                fn_004C0EC0(node, barriers->node, FALSE);
            }
        }
        entry = oalloc(sizeof(SchedulerDGNodeList));
        entry->node = node;
        entry->next = scheduler_volatile_defs;
        scheduler_volatile_defs = entry;
    }
    if (instr->flags & 0x400) {
        for (list = scheduler_volatile_defs; list; list = list->next) {
            if (list->node != node) {
                fn_004C0EC0(node, list->node, FALSE);
            }
        }
        entry = oalloc(sizeof(SchedulerDGNodeList));
        entry->node = node;
        entry->next = scheduler_volatile_uses;
        scheduler_volatile_uses = entry;
    }
    if (instr->flags & 0x800) {
        for (list = scheduler_volatile_uses; list; list = list->next) {
            if (list->node != node) {
                fn_004C0EC0(node, list->node, FALSE);
            }
        }
        entry = oalloc(sizeof(SchedulerDGNodeList));
        entry->node = node;
        entry->next = scheduler_barriers;
        scheduler_barriers = entry;
    }
    if (instr->flags & 0x100108) {
        for (other = scheduler_all_nodes; other; other = other->next) {
            fn_004C0EC0(node, other, FALSE);
        }
        entry = oalloc(sizeof(SchedulerDGNodeList));
        entry->node = node;
        entry->next = scheduler_mem_writes;
        scheduler_mem_writes = entry;
    }
    if (scheduler_mem_writes) {
        for (list = scheduler_mem_writes; list; list = list->next) {
            if (list->node != node) {
                fn_004C0EC0(node, list->node, FALSE);
            }
        }
    }
    if (!node->successors && scheduler_default_successor) {
        fn_004C0EC0(node, scheduler_default_successor, FALSE);
    }
    if (node->height > scheduler_critical_path) {
        scheduler_critical_path = node->height;
    }
}
