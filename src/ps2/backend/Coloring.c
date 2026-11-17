#include "compiler/CParser.h"
#include "compiler/Coloring.h"
#include "compiler/Registers.h"
#include "compiler/CompilerTools.h"
#include "compiler/Spill.h"
#include "compiler/Peephole.h"
#define CERROR_FILE "Coloring.c"
#include "compiler/common.h"
#include "compiler/RegisterInfo.h"
#include "compiler/PCode.h"
#include "compiler/objects.h"
#include "compiler/CError.h"
#include "compiler/InterferenceGraph.h"

static void rewritepcode(void) {
    PCodeBlock* block;
    PCode* instr;
    PCodeArg* arg;
    UInt32 i;
    IGNode* node;
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (instr = block->firstPCode; instr; instr = instr->nextPCode) {
            arg = instr->args;
            i = instr->argCount;
            while (i--) {
                if (arg->kind == PCOp_REGISTER && arg->arg == *(volatile UInt8*)&coloring_class) {
                    arg->data.reg.reg = interferencegraph[arg->data.reg.reg].color;
                }
                arg++;
            }
            if ((instr->flags & 0x10) &&
                ((instr->op > 0x21E && instr->op <= 0x2E7) ? &instr->args[2] : &instr->args[1])
                        ->arg == *(volatile UInt8*)&coloring_class &&
                ((instr->op > 0x21E && instr->op <= 0x2E7) ? &instr->args[2] : &instr->args[1])
                        ->data.reg.reg ==
                    ((instr->op > 0x21E && instr->op <= 0x2E7) ? &instr->args[1] : &instr->args[0])
                        ->data.reg.reg) {
                deletepcode(instr);
            }
        }
    }
    for (i = n_real_registers[coloring_class]; i < used_virtual_registers[coloring_class]; i++) {
        node = &interferencegraph[i];
        if (node->spillTemporary && !(node->flags & 1)) {
            if (node->flags & 0x10) {
                node->spillTemporary->u.var.info->regHi = node->color;
            } else {
                node->spillTemporary->u.var.info->reg = node->color;
            }
        }
    }
}

static int colorgraph(IGNode* stack) {
    IGNode* other;
    IGEdge* edge;
    volatile UInt32 mask;
    UInt32 workingMask;
    SInt32 result;
    SInt32 i;
    short reg;

    starting_available_registers[coloring_class] = used_regs_before_coloring;
    result = 1;
    mask = volatile_registers(coloring_class);
    while (stack != NULL) {
        workingMask = mask;
        edge = stack->edges;
        while (edge) {
            int neighbor = stack->reg < edge->high ? edge->high : edge->low;
            other = &interferencegraph[neighbor];
            reg = other->color;
            if (reg != -1) {
                if (other->flags & 0x200) {
                    workingMask &= ~(3U << reg);
                } else {
                    workingMask &= ~(1U << reg);
                }
            }
            edge = stack->reg < edge->high ? edge->nextLow : edge->nextHigh;
        }
        if (workingMask) {
            if (stack->flags & 0x200) {
                for (i = 0; i < n_real_registers[coloring_class]; i += 2) {
                    if ((workingMask & (3U << i)) == (3U << i)) {
                        stack->color = i;
                        break;
                    }
                }
            } else {
                for (i = 0; i < n_real_registers[coloring_class]; i++) {
                    if (workingMask & (1U << i)) {
                        stack->color = i;
                        break;
                    }
                }
            }
        }
        if (stack->color == -1 && (i = (SInt16)obtain_nonvolatile_register(coloring_class)) != -1) {
            mask |= (1U << i);
        } else {
            if (stack->color == -1) {
                stack->flags |= 1;
                result = 0;
            }
            stack = stack->next;
        }
    }
    return result;
}

static IGNode* simplifygraph(void) {
    SInt32 availableRegs;
    IGNode* spilled;
    IGNode* pushed;
    IGNode* node;
    IGNode* other;
    IGNode* best;
    IGEdge* edge;
    SInt32 changed;
    UInt32 index;
    float bestCost;
    float cost;

    availableRegs = available_registers(coloring_class);
    pushed = NULL;
    do {
        spilled = NULL;
        changed = 0;
        for (index = n_real_registers[coloring_class];
             index < used_virtual_registers[coloring_class];
             index++) {
            node = &interferencegraph[index];
            if (!(node->flags & 6)) {
                if (node->degree < availableRegs) {
                    edge = node->edges;
                    while (edge) {
                        short neighbor = node->reg < edge->high ? edge->high : edge->low;
                        other = &interferencegraph[neighbor];
                        if ((node->flags & 0x200) || (other->flags & 0x200)) {
                            other->degree -= 2;
                        } else {
                            other->degree -= 1;
                        }
                        edge = node->reg < edge->high ? edge->nextLow : edge->nextHigh;
                    }
                    node->flags |= 2;
                    node->next = pushed;
                    pushed = node;
                    changed = 1;
                } else {
                    node->next = spilled;
                    spilled = node;
                }
            }
        }
    } while (changed);

    if (spilled) {
        estimatespillcosts();
    }

    while (spilled) {
        best = spilled;
        bestCost = (best->flags & 0x80)
            ? 3.402823466e38f
            : ((best->flags & 0x400) ? 1.7014117331926443e38f
                                     : (float)best->spillCost / (float)best->degree);
        for (node = spilled->next; node; node = node->next) {
            cost = (node->flags & 0x80)
                ? 3.402823466e38f
                : ((node->flags & 0x400) ? 1.7014117331926443e38f
                                         : (float)node->spillCost / (float)node->degree);
            if (cost < bestCost) {
                best = node;
                bestCost = cost;
            }
        }
        edge = best->edges;
        while (edge) {
            short neighbor = best->reg < edge->high ? edge->high : edge->low;
            other = &interferencegraph[neighbor];
            if ((best->flags & 0x200) || (other->flags & 0x200)) {
                other->degree -= 2;
            } else {
                other->degree -= 1;
            }
            edge = best->reg < edge->high ? edge->nextLow : edge->nextHigh;
        }
        best->flags |= 2;
        best->next = pushed;
        pushed = best;
        do {
            spilled = NULL;
            changed = 0;
            for (index = n_real_registers[coloring_class];
                 index < used_virtual_registers[coloring_class];
                 index++) {
                node = &interferencegraph[index];
                if (!(node->flags & 6)) {
                    if (node->degree < availableRegs) {
                        edge = node->edges;
                        while (edge) {
                            short neighbor = node->reg < edge->high ? edge->high : edge->low;
                            other = &interferencegraph[neighbor];
                            if ((node->flags & 0x200) || (other->flags & 0x200)) {
                                other->degree -= 2;
                            } else {
                                other->degree -= 1;
                            }
                            edge = node->reg < edge->high ? edge->nextLow : edge->nextHigh;
                        }
                        node->flags |= 2;
                        node->next = pushed;
                        pushed = node;
                        changed = 1;
                    } else {
                        node->next = spilled;
                        spilled = node;
                    }
                }
            }
        } while (changed);
    }
    return pushed;
}

void colorinstructions(Object* proc) {
    volatile signed char rclass;
    int index;
    int spilled;

    rclass = 0;
    while (rclass < 7) {
        coloring_class = rclass;
        index = rclass;
        if (used_virtual_registers[index] != n_real_registers[index]) {
            spill_statistics_0 = spill_statistics_1 = spill_statistics_2 = spill_statistics_3 = 0;
            used_regs_before_coloring = starting_available_registers[index];
            spill_passes = 0;
            do {
                CError_ASSERT(408, spill_passes <= 10);
                buildinterferencegraph(proc);
                spilled = !colorgraph(simplifygraph());
                if (spilled) {
                    insertspillcode();
                    ++spill_passes;
                } else {
                    rewritepcode();
                }
                freeoheap();
            } while (spilled);
            used_virtual_registers[index] = n_real_registers[index];
        }
        rclass++;
    }
    if (copts.debuglisting &&
        (spill_statistics_0 > 0 ||
         spill_statistics_1 > 0 ||
         spill_statistics_2 > 0 ||
         spill_statistics_3 > 0)) {
        printspillstatistics(
            spill_statistics_0, spill_statistics_2, spill_statistics_1, spill_statistics_3
        );
    }
    coloring = FALSE;
}
