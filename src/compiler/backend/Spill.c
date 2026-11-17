#include "compiler/CodeGen.h"
#include "compiler/PCode.h"
#include "compiler/objects.h"
#include "compiler/CMachine.h"
#include "compiler/CompilerTools.h"
#include "compiler/CParser.h"
#include "compiler/CError.h"
#include "compiler/InterferenceGraph.h"

#include "compiler/Spill.h"
#include "compiler/RegisterInfo.h"
#include "compiler/Registers.h"
#include "compiler/Operands.h"
#include "compiler/Peephole.h"

void insertspillcode(void) {
    PCodeBlock* block;
    PCode* instr;
    PCode* nextInstr;
    SInt16 i;
    SInt16 count;
    SInt32 remaining;
    SInt32 spilled;
    PCodeArg* arg;
    PCodeArg* source;
    PCodeArg* destination;

    assign_spill_locations();
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (instr = block->firstPCode; instr; instr = nextInstr) {
            nextInstr = instr->nextPCode;
            count = instr->argCount;
            spilled = FALSE;
            arg = instr->args;
            remaining = (SInt16)count;
            while (remaining--) {
                if (arg->kind == PCOp_REGISTER &&
                    arg->arg == (*(volatile UInt8*)&coloring_class) &&
                    (interferencegraph[arg->data.reg.reg].flags & 1)) {
                    spilled = TRUE;
                    break;
                }
                arg++;
            }
            if (!spilled) {
                continue;
            }
            if (instr->flags & 0x10) {
                spillcopy(block, instr);
                continue;
            }
            if (instr->flags & 8) {
                i = fn_0046D3F0() + 2 - fn_004727F0();
                source = instr->args + i;
                destination = instr->args + i;
                while (i < count) {
                    if (source->kind == PCOp_REGISTER &&
                        source->arg == (*(volatile UInt8*)&coloring_class) &&
                        (interferencegraph[source->data.reg.reg].flags & 1)) {
                        instr->argCount--;
                    } else {
                        *destination++ = *source;
                    }
                    source++;
                    i++;
                }
            }
            spillinstruction(block, instr);
        }
    }
}

void spillcopy(PCodeBlock* block, PCode* instr) {
    IGNode* srcNode;
    IGNode* destNode;
    SInt16 reg;
    UInt16 opcode;
    SInt32 special;
    PCodeArg* source;
    PCodeArg* destination;
    PCode* newInstr;

    opcode = instr->op;
    special = 0;
    if (opcode > 0x21E && opcode <= 0x2E7) {
        special = 1;
    }
    if (special) {
        source = &instr->args[2];
    } else {
        source = &instr->args[1];
    }
    srcNode = &interferencegraph[source->data.reg.reg];
    if (opcode > 0x21E && opcode <= 0x2E7) {
        destination = &instr->args[1];
    } else {
        destination = &instr->args[0];
    }
    destNode = &interferencegraph[destination->data.reg.reg];
    if (srcNode->flags & 1) {
        if (destNode->flags & 1) {
            reg = used_virtual_registers[(SInt8)(*(volatile UInt8*)&coloring_class)]++;
            if (srcNode->instr) {
                newInstr = copypcode(srcNode->instr);
                if (newInstr->args[((newInstr->flags & 0x80000000U) ? 1 : 0)].kind !=
                    PCOp_REGISTER) {
                    CError_Internal("Spill.c", 118);
                }
                newInstr->args[((newInstr->flags & 0x80000000U) ? 1 : 0)].data.reg.reg = reg;
                insertpcodebefore(instr, newInstr);
            } else {
                insertpcodebefore(instr, fn_004729B0(reg, srcNode));
                spill_statistics_0++;
                spill_statistics_1 += block->loopWeight * 2;
            }
            insertpcodebefore(instr, fn_004729B0(reg, destNode));
            spill_statistics_0++;
            spill_statistics_1 += block->loopWeight * 2;
        } else {
            if (srcNode->instr) {
                newInstr = copypcode(srcNode->instr);
                if (newInstr->args[(newInstr->flags & 0x80000000) != 0].kind != PCOp_REGISTER) {
                    CError_Internal("Spill.c", 118);
                }
                newInstr->args[(newInstr->flags & 0x80000000) != 0].data.reg.reg =
                    destination->data.reg.reg;
                insertpcodebefore(instr, newInstr);
            } else {
                insertpcodebefore(instr, fn_004728B0(destination->data.reg.reg, srcNode));
                spill_statistics_2++;
                spill_statistics_3 += block->loopWeight;
            }
        }
    } else {
        insertpcodebefore(instr, fn_004728B0(source->data.reg.reg, destNode));
        spill_statistics_2++;
        spill_statistics_3 += block->loopWeight;
    }
    deletepcode(instr);
}

void assign_spill_locations(void) {
    SInt32 cls = (SInt8)(*(volatile UInt8*)&coloring_class);
    UInt32 limit = used_virtual_registers[cls];
    UInt32 i;
    IGNode* node;
    Object* obj;
    VarInfo* info;
    Type* type;
    Type* original;
    for (i = n_real_registers[cls]; i < limit; i++) {
        node = &interferencegraph[i];
        if (!(node->flags & 1)) {
            continue;
        }
        if (node->instr) {
            obj = node->spillTemporary;
            if (obj && obj->u.var.info) {
                obj->u.var.info->used = 0;
            }
            continue;
        }
        if (node->flags & 0x20) {
            obj = node->spillTemporary;
            if (!(obj->u.var.info->flags & 1)) {
                assign_local_memory(obj);
            }
            interferencegraph[i + 1].flags |= 1;
            goto assigned;
        }
        if (node->flags & 0x10) {
            if (interferencegraph[i - 1].flags & 1) {
                goto assigned;
            }
            interferencegraph[i - 1].flags |= 1;
            obj = node->spillTemporary;
            if (!(obj->u.var.info->flags & 1)) {
                assign_local_memory(obj);
            }
            goto assigned;
        }
        if (node->spillTemporary) {
            obj = node->spillTemporary;
            info = obj->u.var.info;
            if (!(info->flags & 1)) {
                if (info->flags & 0x10) {
                    original = obj->type;
                    type = fn_00472850(node);
                    if (original->size < type->size) {
                        obj->type = type;
                        assign_local_memory(obj);
                        obj->type = original;
                        info->flags |= 8;
                        goto assigned;
                    }
                }
                assign_local_memory(obj);
            }
        } else {
            type = fn_00472850(node);
            obj = lalloc(0x44);
            memclrw(obj, 0x44);
            obj->otype = 5;
            obj->name = CParser_GetUniqueName();
            obj->type = type;
            obj->datatype = 1;
            obj->u.var.info = CodeGen_GetNewVarInfo();
            node->spillTemporary = obj;
            assign_local_memory(obj);
        }
assigned:
        obj = node->spillTemporary;
        {
            info = obj->u.var.info;
            info->reg = 0;
            info->regHi = 0;
            if (copts.debuglisting) {
                fn_004BB0B0(i, node->spillCost, (SInt16)node->liveEdges, obj);
            }
        }
    }
}

void spillinstruction(PCodeBlock* block, PCode* instr) {
    SInt32 regs = used_virtual_registers[(SInt8)(*(volatile UInt8*)&coloring_class)];
    SInt32 i = 0;
    PCodeArg* arg = instr->args;
    IGNode* node;
    SInt32 reg;
    SInt16 newReg;
    PCodeArg* other;
    SInt32 j;
    SInt32 readCounter;
    SInt32 writeCounter;
    PCode* newInstr;

    while (i < instr->argCount) {
        if (arg->kind == PCOp_REGISTER &&
            arg->arg == (*(volatile UInt8*)&coloring_class) &&
            (reg = arg->data.reg.reg) < regs) {
            node = &interferencegraph[reg];
            if (node->flags & 1) {
                newReg = used_virtual_registers[(SInt8)(*(volatile UInt8*)&coloring_class)]++;
                writeCounter = 0;
                readCounter = 0;
                j = i;
                other = arg;
                while (j < instr->argCount) {
                    if (other->kind == PCOp_REGISTER &&
                        other->arg == (*(volatile UInt8*)&coloring_class) &&
                        other->data.reg.reg == reg) {
                        if (other->effect & 1) {
                            readCounter++;
                        }
                        if (other->effect & 2) {
                            writeCounter++;
                        }
                        other->data.reg.reg = newReg;
                        other->effect |= 0x40;
                    }
                    j++;
                    other++;
                }
                if (readCounter) {
                    if (node->instr) {
                        newInstr = copypcode(node->instr);
                        if (newInstr->args[((newInstr->flags & 0x80000000U) ? 1 : 0)].kind !=
                            PCOp_REGISTER) {
                            CError_Internal("Spill.c", 118);
                        }
                        newInstr->args[((newInstr->flags & 0x80000000U) ? 1 : 0)].data.reg.reg =
                            newReg;
                        insertpcodebefore(instr, newInstr);
                    } else {
                        insertpcodebefore(instr, fn_004729B0(newReg, node));
                        spill_statistics_0++;
                        spill_statistics_1 += block->loopWeight * 2;
                    }
                }
                if (writeCounter) {
                    if (node->instr || (instr->flags & 0x8000)) {
                        deletepcode(instr);
                    } else {
                        insertpcodeafter(instr, fn_004728B0(newReg, node));
                        spill_statistics_2++;
                        spill_statistics_3 += block->loopWeight;
                    }
                }
            }
        }
        i++;
        arg++;
    }
}

void estimatespillcosts(void) {
    PCodeBlock* block;
    PCode* instr;
    IGNode* node;
    PCodeArg* op;
    int i;
    int weight;
    int negWeight;

    for (block = pcbasicblocks; block; block = block->nextBlock) {
        if (copts.optimizesize) {
            weight = 1;
        } else {
            weight = block->loopWeight;
        }
        for (instr = block->firstPCode; instr; instr = instr->nextPCode) {
            negWeight = -weight;
            op = instr->args;
            i = instr->argCount;
            while (i--) {
                if (op->kind == PCOp_REGISTER &&
                    op->arg == (*(volatile UInt8*)&coloring_class) &&
                    (op->effect & 1)) {
                    node = &interferencegraph[op->data.reg.reg];
                    if (op->effect & 0x10) {
                        node->flags |= 0x400;
                    }
                    if (op->effect & 0x40) {
                        node->flags |= 0x80;
                    }
                    if (node->instr || copts.optimizesize) {
                        node->spillCost += weight;
                    } else {
                        node->spillCost += weight * 2;
                    }
                }
                op++;
            }
            op = instr->args;
            i = instr->argCount;
            while (i--) {
                if (op->kind == PCOp_REGISTER &&
                    op->arg == (*(volatile UInt8*)&coloring_class) &&
                    (op->effect & 2)) {
                    node = &interferencegraph[op->data.reg.reg];
                    if (op->effect & 0x10) {
                        node->flags |= 0x400;
                    }
                    if (op->effect & 0x40) {
                        node->flags |= 0x80;
                    }
                    if (node->instr || (instr->flags & 0x8000)) {
                        node->spillCost += negWeight;
                    } else {
                        node->spillCost += weight;
                    }
                }
                op++;
            }
        }
    }
}
