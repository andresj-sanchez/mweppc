#include "compiler/common.h"
#include "compiler/PCode.h"
#include "compiler/CError.h"
#include "compiler/IroLoop.h"
#include "compiler/IroUtil.h"
#include "compiler/objects.h"
#include "compiler/CompilerTools.h"
#include "compiler/CParser.h"
#include "compiler/LiveRegisters.h"
#include "compiler/BitVectors.h"

#include "compiler/Registers.h"

void fn_004CFA10(void) {
    VRegBlockFlags* flags;
    VRegBlockFlags* state;
    PCodeBlock* block;
    PCode* instr;
    PCode* finalInstr;
    PCLink* link;
    SInt32 changed;
    SInt32 count;
    SInt32 live;

    flags = oalloc(pcblockcount * sizeof(VRegBlockFlags));
    memclrw(flags, pcblockcount * sizeof(VRegBlockFlags));
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        state = &flags[block->blockIndex];
        for (instr = block->firstPCode; instr; instr = instr->nextPCode) {
            if ((instr->flags & 0x400) && !state->end) {
                state->start = 1;
            }
            if ((instr->flags & 0x200) && !state->start) {
                state->end = 1;
            }
        }
    }
    do {
        changed = 0;
        for (count = pcblockcount; count;) {
            block = depthfirstordering[--count];
            if (!block) {
                continue;
            }
            state = &flags[block->blockIndex];
            link = block->successors;
            if (link) {
                state->liveOut = flags[link->to->blockIndex].liveIn;
                for (link = link->nextSuccessor; link; link = link->nextSuccessor) {
                    state->liveOut |= flags[link->to->blockIndex].liveIn;
                }
            }
            live = state->start | (state->liveOut & (state->end == 0));
            if (live != state->liveIn) {
                state->liveIn = live;
                changed = 1;
            }
        }
    } while (changed);
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        live = flags[block->blockIndex].liveOut;
        if (live) {
            block->flags |= 0x40;
        } else {
            block->flags &= ~0x40;
        }
        for (finalInstr = block->lastPCode; finalInstr; finalInstr = finalInstr->prevPCode) {
            if (finalInstr->flags & 0x200) {
                if (live) {
                    finalInstr->flags |= 0x800;
                } else {
                    finalInstr->flags &= ~0x800;
                }
                live = 0;
            }
            if (finalInstr->flags & 0x400) {
                live = 1;
            }
        }
    }
}

void compute_live_registers(void) {
    SInt8 cls;
    SInt8 k;
    SInt32 nregs;
    UInt32 size;
    SInt32 i;
    SInt32 j;
    SInt32 reg;
    RegisterLiveBitmaps* bitmap;
    VRegLiveSet* info;
    VRegFinalRegisters* finalRegs;

    computedepthfirstordering();
    for (cls = 0; cls < 7; ++cls) {
        bitmap = &register_live_bitmaps_6368D0[cls];
        nregs = used_virtual_registers[cls];
        register_live_bitmaps_6368D0[cls].registerCount = nregs;
        register_live_bitmaps_6368D0[cls].blockBitmaps = oalloc(pcblockcount * sizeof(VRegLiveSet));
        size = 4 * (((UInt32)nregs + 31) >> 5);
        register_live_bitmaps_6368D0[cls].current = oalloc(size);
        for (i = 0; i < pcblockcount; i++) {
            info = &register_live_bitmaps_6368D0[cls].blockBitmaps[i];
            bitvectorinitialize(info->use = oalloc(size), nregs, 0);
            bitvectorinitialize(info->def = oalloc(size), nregs, 0);
            bitvectorinitialize(info->in = oalloc(size), nregs, 0);
            bitvectorinitialize(info->out = oalloc(size), nregs, 0);
        }
    }
    fn_004CFD30(-1);
    finalRegs = (VRegFinalRegisters*)live_registers_61B908;
    for (j = 0; j < finalRegs->count; ++j) {
        cls = finalRegs->regs[j].regClass;
        reg = finalRegs->regs[j].reg;
        bitmap = &register_live_bitmaps_6368D0[cls];
        info = &bitmap->blockBitmaps[pclastblock->blockIndex];
        info->use[reg >> 5] |= 1UL << reg;
    }
    for (k = 0; k < 7; ++k) {
        fn_004CFB80(k);
    }
    fn_004CFA10();
}

void compute_live_registers_of_class(SInt8 cls) {
    SInt32 nregs;
    UInt32 size;
    SInt32 i;
    SInt32 j;
    SInt32 reg;
    VRegLiveSet* info;
    VRegFinalRegisters* finalRegs;

    computedepthfirstordering();
    nregs = used_virtual_registers[cls];
    register_live_bitmaps_6368D0[cls].registerCount = nregs;
    register_live_bitmaps_6368D0[cls].blockBitmaps = oalloc(pcblockcount * sizeof(VRegLiveSet));
    size = 4 * (((UInt32)nregs + 31) >> 5);
    register_live_bitmaps_6368D0[cls].current = oalloc(size);
    for (i = 0; i < pcblockcount; i++) {
        info = &register_live_bitmaps_6368D0[cls].blockBitmaps[i];
        bitvectorinitialize(info->use = oalloc(size), nregs, 0);
        bitvectorinitialize(info->def = oalloc(size), nregs, 0);
        bitvectorinitialize(info->in = oalloc(size), nregs, 0);
        bitvectorinitialize(info->out = oalloc(size), nregs, 0);
    }
    fn_004CFD30(cls);
    finalRegs = (VRegFinalRegisters*)live_registers_61B908;
    for (j = 0; j < finalRegs->count; j++) {
        if (finalRegs->regs[j].regClass == cls) {
            reg = finalRegs->regs[j].reg;
            info = &register_live_bitmaps_6368D0[cls].blockBitmaps[pclastblock->blockIndex];
            info->use[reg >> 5] |= (1UL << reg);
        }
    }
    fn_004CFB80(cls);
    fn_004CFA10();
}

void fn_004CFB80(SInt8 regClass) {
    PCLink* link;
    SInt32 registerCount;
    VRegLiveSet* info;
    UInt32* use;
    UInt32* def;
    UInt32* in;
    UInt32* out;
    UInt32 words;
    SInt32 count;
    SInt32 changed;
    PCodeBlock* block;
    UInt32 value;
    SInt32 i;
    UInt32 j;
    VRegLiveSet* table;
    VRegFinalRegisters* finalRegisters;

    registerCount = register_live_bitmaps_6368D0[regClass].registerCount;
    table = register_live_bitmaps_6368D0[regClass].blockBitmaps;
    words = ((UInt32)registerCount + 31) >> 5;
    do {
        changed = 0;
        for (count = pcblockcount; count;) {
            block = depthfirstordering[--count];
            if (!block) {
                continue;
            }
            info = &table[block->blockIndex];
            link = block->successors;
            if (link) {
                UInt32* dst = info->out;
                bitvectorcopy(dst, table[link->to->blockIndex].in, registerCount);
                for (link = link->nextSuccessor; link; link = link->nextSuccessor) {
                    bitvectorunion(dst, table[link->to->blockIndex].in, registerCount);
                }
            } else {
                for (i = 0; i < ((VRegFinalRegisters*)live_registers_61B908)->count; i++) {
                    if (((VRegFinalRegisters*)live_registers_61B908)->regs[i].regClass ==
                        regClass) {
                        int reg = ((VRegFinalRegisters*)live_registers_61B908)->regs[i].reg;
                        info->out[reg >> 5] |= 1UL << reg;
                    }
                }
            }
            out = info->out;
            in = info->in;
            use = info->use;
            def = info->def;
            for (j = 0; j < words; ++j) {
                value = *use | (*out & ~*def);
                if (value != *in) {
                    *in = value;
                    changed = 1;
                }
                use++;
                def++;
                in++;
                out++;
            }
        }
    } while (changed);
}

void fn_004CFD30(SInt8 regClass) {
    PCodeBlock* block;
    PCode* instr;
    PCodeArg* arg;
    SInt32 count;
    SInt32 reg;
    SInt32 cls;
    VRegLiveSet* table;
    UInt32* bits;

    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (instr = block->firstPCode; instr; instr = instr->nextPCode) {
            for (arg = instr->args, count = instr->argCount; count--; arg++) {
                if (arg->kind == PCOp_REGISTER &&
                    (regClass == -1 || (SInt8)arg->arg == regClass) &&
                    (arg->effect & 1)) {
                    reg = (SInt16)arg->data.reg.reg;
                    cls = (SInt8)arg->arg;
                    table = register_live_bitmaps_6368D0[cls].blockBitmaps;
                    bits = table[block->blockIndex].def;
                    if (!((bits[reg >> 5] >> (reg & 31)) & 1)) {
                        table[block->blockIndex].use[reg >> 5] |= 1UL << (reg & 31);
                    }
                }
            }
            for (arg = instr->args, count = instr->argCount; count--; arg++) {
                if (arg->kind == PCOp_REGISTER &&
                    (regClass == -1 || (SInt8)arg->arg == regClass) &&
                    (arg->effect & 2)) {
                    reg = (SInt16)arg->data.reg.reg;
                    cls = (SInt8)arg->arg;
                    table = register_live_bitmaps_6368D0[cls].blockBitmaps;
                    bits = table[block->blockIndex].use;
                    if (!((bits[reg >> 5] >> (reg & 31)) & 1)) {
                        table[block->blockIndex].def[reg >> 5] |= 1UL << (reg & 31);
                    }
                }
            }
        }
    }
}

SInt32 fn_004CF6D0(PCode* instr) {
    PCodeArg* arg;
    SInt32 i;
    SInt32 cls;
    SInt32 reg;
    if (instr->flags & 0x14098D) {
        return FALSE;
    }
    if (instr->block->flags & 3) {
        return FALSE;
    }
    for (arg = instr->args, i = instr->argCount; i--; arg++) {
        if (arg->kind == 0 && (arg->effect & 2)) {
            reg = arg->data.reg.reg;
            cls = (SInt8)arg->arg;
            if ((register_live_bitmaps_6368D0[cls].current[reg >> 5] >> reg) & 1) {
                return FALSE;
            }
        }
    }
    if (copts.optimizationlevel > 0) {
        return TRUE;
    }
    return FALSE;
}

void update_live_registers(PCode* instr) {
    PCodeArg* arg;
    SInt32 i;
    SInt32 cls;
    SInt32 reg;
    for (arg = instr->args, i = instr->argCount; i--; arg++) {
        if (arg->kind == 0 && (arg->effect & 2)) {
            reg = arg->data.reg.reg;
            cls = (SInt8)arg->arg;
            register_live_bitmaps_6368D0[cls].current[reg >> 5] &= ~(1UL << (reg & 31));
        }
    }
    for (arg = instr->args, i = instr->argCount; i--; arg++) {
        if (arg->kind == 0 && (arg->effect & 1)) {
            reg = arg->data.reg.reg;
            cls = (SInt8)arg->arg;
            register_live_bitmaps_6368D0[cls].current[reg >> 5] |= (1UL << reg);
        }
    }
}

void capture_live_registers(PCodeBlock* block) {
    volatile SInt8 cls;
    RegisterLiveBitmaps* info;
    cls = 0;
    info = register_live_bitmaps_6368D0;
    while (cls < 7) {
        bitvectorcopy(
            info->current, info->blockBitmaps[block->blockIndex].out, info->registerCount
        );
        cls++;
        info++;
    }
}

void initialize_live_registers(void* data) {
    live_registers_61B908 = data;
}
