#include "compiler/CodeGen.h"
#include "compiler/common.h"
#include "compiler/Peephole.h"
#include "compiler/InterferenceGraph.h"
#include "compiler/CompilerTools.h"
#include "compiler/CParser.h"
#include "compiler/CRTTI.h"
#include "compiler/InlineAsmMIPS.h"
#include "compiler/LiveRegisters.h"
#include "compiler/LoopOptimization.h"
#include "compiler/MIPS_Peephole.h"
#include "compiler/PCodeInfo.h"
#include "compiler/Registers.h"
#include "compiler/UseDefChains.h"
#include "compiler/ValueNumbering.h"
#include "compiler/DeadCodeElimination.h"

SInt32 fn_004BB060(PCode* first) {
    return 0;
}

SInt32 fn_004BB070(PCode* first, PCode* second) {
    return 0;
}

void printspillstatistics(UInt32 a, UInt32 b, UInt32 c, UInt32 d) {}

void fn_004BB090(void) {}

void fn_004BB0A0(SInt32 low, SInt32 high) {}

void fn_004BB0B0(UInt32 index, SInt32 cost, SInt32 reg, Object* object) {}

#pragma dont_inline on
void pclistblocks(const char* name, const char* title) {}
#pragma dont_inline reset

void fn_004BB0D0(void) {}

void installpattern(UInt16 opcode, void* callback) {
    PeepholeHandler* entry = oalloc(8);
    entry->callback = callback;
    entry->next = peephole_handlers[opcode];
    peephole_handlers[opcode] = entry;
}

#include "compiler/PCode.h"

int usedbetween(PCode* start, PCode* end, PCodeArg* operand) {
    PCode* instruction;
    PCodeArg* argument;
    UInt32 index;

    for (instruction = start->prevPCode; instruction != end; instruction = instruction->prevPCode) {
        index = instruction->argCount;
        argument = instruction->args;
        while (index--) {
            if (argument->kind == operand->kind &&
                argument->data.reg.reg == operand->data.reg.reg &&
                argument->arg == operand->arg &&
                (argument->effect & 1)) {
                return 1;
            }
            argument++;
        }
    }
    return 0;
}

int definedbetween(PCode* start, PCode* end, PCodeArg* operand) {
    PCode* instruction;
    PCodeArg* argument;
    UInt32 index;

    for (instruction = start->prevPCode; instruction != end; instruction = instruction->prevPCode) {
        index = instruction->argCount;
        argument = instruction->args;
        while (index--) {
            if (argument->kind == operand->kind &&
                argument->data.reg.reg == operand->data.reg.reg &&
                argument->arg == operand->arg &&
                (argument->effect & 2)) {
                return 1;
            }
            argument++;
        }
    }
    return 0;
}

int fn_004BB790(PCodeBlock* block) {
    int changed = 0;
    PCode* pcode;
    PeepholeHandler* pattern;

    capture_live_registers(block);
    for (pcode = block->lastPCode; pcode; pcode = pcode->prevPCode) {
        if (fn_004CF6D0(pcode)) {
            deletepcode(pcode);
        } else {
            pattern = peephole_handlers[pcode->op];
            while (pattern) {
                if (((int (*)(PCode*))pattern->callback)(pcode)) {
                    changed = 1;
                    if (!pcode->block) {
                        break;
                    }
                    pattern = peephole_handlers[pcode->op];
                } else {
                    pattern = pattern->next;
                }
            }
            if (pcode->block) {
                update_live_registers(pcode);
            }
        }
    }
    return changed;
}

void computeinstructionpredecessors(PCodeBlock* block) {
    SInt8 cls;
    SInt32 i;
    PCode* pcode;
    PCodeArg* argument;

    cls = 0;
    do {
        i = 0;
        if (used_virtual_registers[cls] > 0) {
            do {
                peephole_register_state[cls][i] = peephole_default_state;
                i++;
            } while (i < used_virtual_registers[cls]);
        }
        cls++;
    } while (cls < 7);
    for (pcode = block->firstPCode; pcode; pcode = pcode->nextPCode) {
        for (i = 0, argument = pcode->args; i < pcode->argCount; i++, argument++) {
            if (argument->kind == PCOp_REGISTER && (argument->effect & 1)) {
                pcode->args[i].data.reg.definer =
                    peephole_register_state[(SInt8)argument->arg][argument->data.reg.reg];
            }
        }
        for (i = 0, argument = pcode->args; i < pcode->argCount; i++, argument++) {
            if (argument->kind == PCOp_REGISTER && (argument->effect & 2)) {
                peephole_register_state[(SInt8)argument->arg][argument->data.reg.reg] = pcode;
            }
        }
    }
}

void fn_004BB9D0(void) {
    volatile SInt8 cls;
    SInt32 index;

    for (cls = 0; cls < 7; cls++) {
        index = cls;
        peephole_register_state[index] = oalloc(used_virtual_registers[index] * sizeof(PCode*));
    }
    peephole_default_state = makepcode(get_nop_opcode());
}

void peepholeoptimizepcode(Object* unused) {
    volatile SInt8 cls;
    int index;
    int i;
    PCodeBlock* block;

    for (cls = 0; cls < 7; cls++) {
        index = cls;
        peephole_register_state[index] = oalloc(used_virtual_registers[index] * sizeof(PCode*));
    }
    peephole_default_state = makepcode(get_nop_opcode());
    for (i = 0; i < 0x4AF; i++) {
        peephole_handlers[i] = NULL;
    }
    fn_00557180();
    compute_live_registers();
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        if (!(block->flags & 0x180) && block->pcodeCount != 0) {
            do {
                computeinstructionpredecessors(block);
            } while (fn_004BB790(block));
        }
    }
    freeoheap();
}

#include "compiler/Alias.h"
#include "compiler/CMangler.h"

void fn_004BB370(Object* object) {
    const char* name = CMangler_GetLinkName(object)->name;
    if (copts.commonsubs) {
        removecommonsubexpressions(0);
        if (copts.debuglisting) {
            pclistblocks(name, "After removecommonsubexpressions");
        }
    } else {
        gather_alias_info();
    }
    fn_00555180(object, 0);
    if (copts.debuglisting) {
        pclistblocks(name, "After propagatecopyinstructions");
    }
    if (copts.peephole) {
        peepholeoptimizepcode(object);
        if (copts.debuglisting) {
            pclistblocks(name, "After peephole");
        }
    }
    fn_00555E80(object);
    if (copts.debuglisting) {
        pclistblocks(name, "After pipeline1");
    }
}

void fn_004BB2A0(Object* object) {
    const char* name = CMangler_GetLinkName(object)->name;
    if (copts.commonsubs) {
        removecommonsubexpressions(0);
        if (copts.debuglisting) {
            pclistblocks(name, "After removecommonsubexpressions");
        }
    } else {
        gather_alias_info();
    }
    fn_00555180(object, 0);
    if (copts.debuglisting) {
        pclistblocks(name, "After propagatecopyinstructions");
    }
    if (copts.peephole) {
        peepholemergeblocks(object, 0);
        peepholeoptimizepcode(object);
        if (copts.debuglisting) {
            pclistblocks(name, "After peephole");
        }
    }
    fn_00555E80(object);
    if (copts.debuglisting) {
        pclistblocks(name, "After pipeline1");
    }
    eliminatedeadcode(object);
    fn_004BE090();
}

void fn_004BB180(Object* object) {
    const char* name = CMangler_GetLinkName(object)->name;
    if (copts.commonsubs) {
        removecommonsubexpressions(0);
        if (copts.debuglisting) {
            pclistblocks(name, "After removecommonsubexpressions");
        }
    } else {
        gather_alias_info();
    }
    fn_00555180(object, 0);
    if (copts.debuglisting) {
        pclistblocks(name, "After propagatecopyinstructions");
    }
    if (copts.peephole) {
        peepholemergeblocks(object, 0);
        peepholeoptimizepcode(object);
        if (copts.debuglisting) {
            pclistblocks(name, "After peephole");
        }
    }
    fn_00555E80(object);
    if (copts.debuglisting) {
        pclistblocks(name, "After pipeline1");
    }
    computeusedefchains(1);
    if (fn_00556500()) {
        if (copts.debuglisting) {
            pclistblocks(name, "AFTER MEMORY STORE-LOAD OPTIMIZATION");
        }
        freeoheap();
        pruneusedefchains(1);
        fn_00555180(object, 0);
        if (copts.debuglisting) {
            pclistblocks(name, "After propagatecopyinstructions #2");
        }
        computeusedefchains(1);
    }
}

void fn_004BB0E0(Object* object) {
    SInt8 mode = copts.optimizationlevel;
    if (mode == 1) {
        const char* name = CMangler_GetLinkName(object)->name;
        if (copts.peephole) {
            gather_alias_info();
            peepholeoptimizepcode(object);
            if (copts.debuglisting) {
                pclistblocks(name, "After peephole");
            }
        }
    } else if (mode == 2 || (cg_codegen_reg_635FA0 && (SInt8)mode > 2)) {
        fn_004BB370(object);
    } else if (mode == 3) {
        fn_004BB2A0(object);
    } else if (mode == 4) {
        fn_004BB180(object);
    }
}

void peepholemergeblocks(Object* object, SInt8 flag) {
    Boolean guarded;
    PCodeBlock* to;
    PCLink* link;
    PCode* after;
    PCode* instr;
    PCodeBlock* next;
    PCodeBlock* block;

    block = pcbasicblocks;
    if (block) {
        do {
            guarded = FALSE;
            next = block->nextBlock;
            if (!next) {
                goto end_function;
            }
            if (!flag) {
                if ((guarded = next != NULL)) {
                    guarded = (next->flags & 2) != 0;
                }
                if (block->flags & 1) {
                    goto next_block;
                }
            }
            if (block->pcodeCount > 0) {
                instr = block->firstPCode;
                if (instr) {
                    do {
                        if (instr->flags & 0x108) {
                            break;
                        }
                        instr = instr->nextPCode;
                    } while (instr);
                }
                if (!instr) {
                    instr = block->lastPCode;
                    if (instr && instr->op == 0x39) {
                        if (instr->args[0].kind != PCOp_LABEL ||
                            instr->args[0].data.label.label->block != next) {
                            goto next_block;
                        }
                        deletepcode(instr);
                    }
                    instr = block->lastPCode;
                    if (!instr || !(instr->flags & 1) || instr->op == 0x39) {
                        while (block->successors->to == next &&
                               block->successors &&
                               !block->successors->nextSuccessor &&
                               next &&
                               next->predecessors &&
                               next->predecessors->from == block &&
                               next->predecessors &&
                               !next->predecessors->nextPredecessor &&
                               !guarded &&
                               block->pcodeCount + next->pcodeCount <= 100) {
                            if (next->pcodeCount > 0) {
                                for (instr = next->firstPCode; instr; instr = after) {
                                    after = instr->nextPCode;
                                    if (instr->flags & 0x108) {
                                        break;
                                    }
                                    deletepcode(instr);
                                    if (instr->op != 0x39 ||
                                        instr->args[0].kind != PCOp_LABEL ||
                                        instr->args[0].data.label.label->block != next->nextBlock) {
                                        appendpcode(block, instr);
                                    }
                                }
                            }
                            if (next->pcodeCount || next == cg_codegen_cursor_636018) {
                                break;
                            }
                            for (link = next->successors; link; link = link->nextSuccessor) {
                                to = link->to;
                                insertedge(block, to);
                                deleteedge(next, to);
                            }
                            deleteedge(block, next);
                            deleteblock(next);
                            next = block->nextBlock;
                            if (!flag) {
                                if ((guarded = next != NULL)) {
                                    guarded = (next->flags & 2) != 0;
                                }
                            }
                        }
                    }
                }
            }
next_block:
            block = block->nextBlock;
        } while (block);
    }
end_function:
    return;
}
