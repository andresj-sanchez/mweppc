#include "compiler/ValueNumbering.h"
#include "compiler/common.h"
#include "compiler/Alias.h"
#include "compiler/Registers.h"
#include "compiler/CompilerTools.h"
#include "compiler/PCode.h"
#include "compiler/PCodeInfo.h"
#include "compiler/CInt64.h"
#include "compiler/LiveRegisters.h"
#include "compiler/LoopDetection.h"
#include "compiler/Scheduler.h"

#pragma defer_codegen off
#pragma opt_propagation off
void initializecsedatastructures(void) {
    SInt32 i;
    SInt32 index;
    SInt8 cls;
    RegValue* record;

    nextvaluenumber = 0;
    i = 0;
    do {
        index = i;
        i += 8;
        opvalue[index] = 0;
        opvalue[index + 1] = 0;
        opvalue[index + 2] = 0;
        opvalue[index + 3] = 0;
        opvalue[index + 4] = 0;
        opvalue[index + 5] = 0;
        opvalue[index + 6] = 0;
        opvalue[index + 7] = 0;
    } while (i < 0x4a7);
    cls = 0;
    opvalue[i] = 0;
    opvalue[i + 1] = 0;
    opvalue[i + 2] = 0;
    opvalue[i + 3] = 0;
    opvalue[i + 4] = 0;
    opvalue[i + 5] = 0;
    opvalue[i + 6] = 0;
    while (cls < 7) {
        index = cls;
        record = regvalue[index];
        i = 0;
        if (used_virtual_registers[index] > 0) {
            do {
                record->number = nextvaluenumber++;
                record->copyreg = 0;
                record->available = 0;
                record++;
                i++;
            } while (i < used_virtual_registers[index]);
        }
        cls++;
    }
    initialize_alias_values();
    stackedvalues = NULL;
    valueceiling = 0x7FFFFFFF;
}

#pragma opt_propagation on
#pragma defer_codegen on

void fn_004BBBC0(void) {
    SInt32 i;
    AvailableValue* node;
    AvailableValue** cursor;
    AvailableValue** list;
    StackedValue* undo;
    PCodeArg* arg;
    RegValue* entry;
    ValueLabel* use;
    ValueLabel** usecursor;
    AvailableValue* oldnode;

    i = 0;
    list = (AvailableValue**)opvalue;
    do {
        cursor = list;
        node = *list;
        while (node) {
            if (node->killedregister >= valueceiling) {
                *cursor = node->next;
            } else {
                cursor = &node->next;
            }
            node = *cursor;
        }
        i++;
        list++;
    } while (i < 0x4AF);
    for (undo = stackedvalues; undo; undo = undo->next) {
        arg = &undo->op;
        if (arg->kind == 4) {
            undo->alias->valuenumber = undo->value.number;
            undo->alias->valuepcode = undo->valuepcode;
        } else {
            entry = &regvalue[(SInt8)arg->arg][(SInt16)arg->data.reg.reg];
            oldnode = (AvailableValue*)entry->available;
            if (oldnode) {
                usecursor = &oldnode->labelled;
                for (use = *usecursor; use; use = *usecursor) {
                    if ((UInt16)use->op.data.reg.reg == (UInt16)arg->data.reg.reg) {
                        *usecursor = use->next;
                    } else {
                        usecursor = &use->next;
                    }
                }
            }
            entry->number = undo->value.number;
            entry->copyreg = undo->value.copyreg;
            entry->available = undo->value.available;
            oldnode = (AvailableValue*)entry->available;
            if (oldnode) {
                use = oalloc(sizeof(ValueLabel));
                use->op = *arg;
                use->next = oldnode->labelled;
                oldnode->labelled = use;
            }
        }
    }
}

void fn_004BBAE0(PCodeBlock* block) {
    PCLink* edge;
    PCodeBlock* successor;
    SInt32 count;
    StackedValue* savedUndo;
    SInt32 previousBoundary;

    fn_004BBCE0(block);
    while ((edge = block->successors) && !edge->nextSuccessor) {
        successor = edge->to;
        if (!successor->predecessors || successor->predecessors->nextPredecessor) {
            break;
        }
        fn_004BBCE0(block = successor);
    }
    count = 0;
    for (;;) {
        if (!edge) {
            break;
        }
        if (!(edge->to->flags & 4) &&
            edge->to->predecessors &&
            !edge->to->predecessors->nextPredecessor) {
            count++;
        }
        edge = edge->nextSuccessor;
    }
    if (!count) {
        return;
    }
    {
        previousBoundary = valueceiling;
        savedUndo = stackedvalues;
        stackedvalues = NULL;
        valueceiling = nextvaluenumber;
        edge = block->successors;
        for (;;) {
            if (!edge) {
                break;
            }
            successor = edge->to;
            if (!(successor->flags & 4) &&
                successor->predecessors &&
                !successor->predecessors->nextPredecessor) {
                fn_004BBAE0(successor);
                fn_004BBBC0();
            }
            edge = edge->nextSuccessor;
        }
        stackedvalues = savedUndo;
        valueceiling = previousBoundary;
    }
}

void removecommonsubexpressions(SInt32 moreAggressive) {
    volatile SInt8 cls;
    PCodeBlock* block;

    moreaggressiveoptimization = moreAggressive;
    cg_codegen_setup_6359B0 = 0;
    gather_alias_info();
    for (cls = 0; cls < 7; cls++) {
        SInt32 index = cls;
        regvalue[index] = oalloc(used_virtual_registers[index] * sizeof(RegValue));
    }
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        block->flags &= ~4;
    }
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        if (!(block->flags & 4)) {
            initializecsedatastructures();
            fn_004BBAE0(block);
        }
    }
    freeoheap();
}

void fn_004BDAD0(Object* proc) {
    volatile SInt8 cls;

    cg_codegen_flags_6364C8 = 0;
    if (fn_004C47E0(0)) {
        gather_alias_info();
        compute_live_registers();
        fn_004C4700();
        for (cls = 0; cls < 7; cls++) {
            SInt32 index = cls;
            value_register_uses_60B6EC[index] =
                oalloc(used_virtual_registers[index] * sizeof(SInt32));
            value_register_defs_60B6D0[index] =
                oalloc(used_virtual_registers[index] * sizeof(SInt32));
        }
        forall_loops_in_postorder_do(fn_004BDB70);
        freeoheap();
    }
}

SInt32 fn_004BDE20(PCode* instr, LoopRegisterNode* context) {
    volatile UInt32 flags;
    PCodeArg* arg;
    UInt32 index;
    SInt16 reg;
    SInt32 cls;
    UInt32* bitmap;
    PCodeBlock* block;

    flags = instr->flags;
    if (flags & 0x140D8D) {
        return 0;
    }
    arg = instr->args;
    index = (SInt16)instr->argCount;
    while (index--) {
        if (arg->kind == PCOp_REGISTER && (arg->effect & 2)) {
            cls = (SInt8)arg->arg;
            reg = (SInt16)arg->data.reg.reg;
            if (value_register_uses_60B6EC[cls][reg] > 1) {
                return 0;
            }
            block = context->destinationBlock;
            bitmap = register_live_bitmaps_6368D0[cls].blockBitmaps[block->blockIndex].out;
            if ((bitmap[reg >> 5] >> reg) & 1) {
                return 0;
            }
        }
        arg++;
    }
    if (instr->block != context->block) {
        if (flags & 0x10000) {
            return 0;
        }
        if ((flags & 0x60006) && (flags & 0x20)) {
            return 0;
        }
    }
    return 1;
}

void fn_004BDF30(void* loop) {
    LoopRegisterNode* savedBase;
    SInt8 cls;
    SInt32 i;
    LoopRegisterBlock* link;
    PCode* instr;
    PCodeArg* arg;
    SInt32 count;

    cls = 0;
    savedBase = (LoopRegisterNode*)loop;
    do {
        SInt32 index = cls;
        i = 0;
        if (used_virtual_registers[index] > 0) {
            do {
                SInt32 offset = i;
                value_register_uses_60B6EC[index][offset] = 0;
                i = offset + 1;
                value_register_defs_60B6D0[index][offset] = 0;
            } while (i < used_virtual_registers[index]);
        }
        cls++;
    } while (cls < 7);
    for (link = savedBase->blocks; link; link = link->next) {
        for (instr = link->block->firstPCode; instr; instr = instr->nextPCode) {
            count = instr->argCount;
            arg = instr->args;
            while (count--) {
                if (arg->kind == 0) {
                    if (arg->effect & 2) {
                        value_register_uses_60B6EC[(SInt8)arg->arg][(SInt16)arg->data.reg.reg]++;
                    }
                    if (arg->effect & 1) {
                        value_register_defs_60B6D0[(SInt8)arg->arg][(SInt16)arg->data.reg.reg]++;
                    }
                }
                arg++;
            }
        }
    }
}

void killmemory(Alias* record, PCode* pcode) {
    StackedValue* undo;

    if (record->valuenumber < valueceiling && nextvaluenumber >= valueceiling) {
        undo = oalloc(0x30);
        undo->next = stackedvalues;
        stackedvalues = undo;
        undo->op.kind = 4;
        undo->alias = record;
        undo->value.number = record->valuenumber;
        undo->valuepcode = record->valuepcode;
    }
    if (pcode) {
        record->valuenumber =
            regvalue[(SInt8)pcode->args[0].arg][(SInt16)pcode->args[0].data.reg.reg].number;
        record->valuepcode = pcode;
    } else {
        record->valuenumber = nextvaluenumber++;
        record->valuepcode = NULL;
    }
}

void fn_004BC860(PCode* volatile instr) {
    volatile SInt32 i;
    PCodeArg* arg;
    PCodeArg* matchArg;
    RegValue* entry;
    StackedValue* undo;
    AvailableValue* node;
    ValueLabel* use;
    ValueLabel** cursor;

    i = 0;
    arg = instr->args;
    if (instr->argCount > 0) {
        do {
            if (arg->kind == PCOp_REGISTER && (arg->effect & 2)) {
                matchArg = arg;
                entry = &regvalue[(SInt8)arg->arg][(SInt16)arg->data.reg.reg];
                if (entry->number < valueceiling && nextvaluenumber >= valueceiling) {
                    undo = oalloc(0x30);
                    undo->next = stackedvalues;
                    stackedvalues = undo;
                    undo->op = *arg;
                    undo->value = *entry;
                }
                node = (AvailableValue*)entry->available;
                if (node) {
                    cursor = &node->labelled;
                    for (use = *cursor; use; use = *cursor) {
                        if ((UInt16)use->op.data.reg.reg == (UInt16)matchArg->data.reg.reg) {
                            *cursor = use->next;
                        } else {
                            cursor = &use->next;
                        }
                    }
                }
                entry->available = 0;
                entry->copyreg = 0;
                entry->number = nextvaluenumber++;
            }
            i++;
            arg++;
        } while (i < instr->argCount);
    }
    if (coloring) {
        update_all_alias_values();
        fn_004BD7E0();
    } else {
        update_alias_value((void*)instr->alias, NULL);
    }
}

void fn_004BD7E0(void) {
    SInt32 i;
    AvailableValue* node;
    ValueLabel* aliasUse;
    RegValue* entry;
    StackedValue* undo;
    AvailableValue* valueNode;
    ValueLabel* use;
    ValueLabel** cursor;

    i = 0;
    do {
        for (node = (AvailableValue*)opvalue[i]; node; node = node->next) {
            for (aliasUse = node->labelled; aliasUse; aliasUse = aliasUse->next) {
                entry = &regvalue[(SInt8)aliasUse->op.arg][(SInt16)aliasUse->op.data.reg.reg];
                if (entry->number < valueceiling && nextvaluenumber >= valueceiling) {
                    undo = oalloc(0x30);
                    undo->next = stackedvalues;
                    stackedvalues = undo;
                    undo->op = aliasUse->op;
                    undo->value = *entry;
                }
                valueNode = (AvailableValue*)entry->available;
                if (valueNode) {
                    cursor = &valueNode->labelled;
                    for (use = *cursor; use; use = *cursor) {
                        if ((UInt16)use->op.data.reg.reg == (UInt16)aliasUse->op.data.reg.reg) {
                            *cursor = use->next;
                        } else {
                            cursor = &use->next;
                        }
                    }
                }
                entry->available = 0;
                entry->copyreg = 0;
                entry->number = nextvaluenumber++;
            }
        }
        i++;
    } while (i < 0x4AF);
}

void fn_004BCFC0(PCode* instr) {
    PCodeArg* arg;
    PCodeArg* dest;
    AvailableValue* node;
    ValueLabel* use;
    ValueLabel** cursor;
    RegValue* entry;
    StackedValue* undo;
    SInt32 i;
    SInt32 destIndex;

    node = oalloc(instr->argCount * 4 + 0x14);
    node->labelled = NULL;
    node->pcode = instr;
    i = 0;
    arg = instr->args;
    if (instr->argCount > 0) {
        do {
            if (arg->kind == PCOp_REGISTER) {
                node->opnumbers[i] = regvalue[(SInt8)arg->arg][(SInt16)arg->data.reg.reg].number;
            }
            i++;
            arg++;
        } while (i < instr->argCount);
    }
    if (instr->flags & 0x20002) {
        node->aliasnumber = instr->alias->valuenumber;
    }
    if (instr->flags & 0x80000000) {
        destIndex = 1;
    } else {
        destIndex = 0;
    }
    dest = &instr->args[destIndex];
    entry = &regvalue[(SInt8)dest->arg][(SInt16)dest->data.reg.reg];
    if (entry->number < valueceiling && nextvaluenumber >= valueceiling) {
        undo = oalloc(0x30);
        undo->next = stackedvalues;
        stackedvalues = undo;
        undo->op = *dest;
        undo->value = *entry;
    }
    if (entry->available) {
        cursor = &((AvailableValue*)entry->available)->labelled;
        for (use = *cursor; use; use = *cursor) {
            if ((UInt16)use->op.data.reg.reg == (UInt16)dest->data.reg.reg) {
                *cursor = use->next;
            } else {
                cursor = &use->next;
            }
        }
    }
    entry->available = 0;
    entry->copyreg = 0;
    entry->number = nextvaluenumber;
    {
        SInt32 newVersion = entry->number;
        nextvaluenumber++;
        node->killedregister = newVersion;
    }
    use = oalloc(sizeof(ValueLabel));
    use->op = *dest;
    use->next = node->labelled;
    node->labelled = use;
    entry->available = node;
    node->next = (AvailableValue*)opvalue[instr->op];
    opvalue[instr->op] = node;
}

void fn_004BDD00(PCode* instr, LoopRegisterNode* context) {
    PCodeBlock* block;
    PCode* last;
    PCodeArg* arg;
    UInt32 count;

    block = context->destinationBlock;
    deletepcode(instr);
    last = block->lastPCode;
    if (!last || !(last->flags & 1)) {
        appendpcode(block, instr);
    } else {
        insertpcodebefore(last, instr);
    }
    count = instr->argCount;
    arg = instr->args;
    while (count--) {
        if (arg->kind == PCOp_REGISTER) {
            if (arg->effect & 2) {
                value_register_uses_60B6EC[(SInt8)arg->arg][(SInt16)arg->data.reg.reg]--;
            }
            if (arg->effect & 1) {
                value_register_defs_60B6D0[(SInt8)arg->arg][(SInt16)arg->data.reg.reg]--;
            }
        }
        arg++;
    }
    count = instr->argCount;
    arg = instr->args;
    while (count--) {
        if (arg->kind == PCOp_REGISTER) {
            if ((arg->effect & 2) &&
                value_register_defs_60B6D0[(SInt8)arg->arg][(SInt16)arg->data.reg.reg] > 0) {
                (context->maxRegisterCount[(SInt8)arg->arg])++;
            }
            if ((arg->effect & 1) &&
                value_register_defs_60B6D0[(SInt8)arg->arg][(SInt16)arg->data.reg.reg] == 0) {
                (context->maxRegisterCount[(SInt8)arg->arg])--;
            }
        }
        arg++;
    }
    cg_codegen_flags_6364C8 = 1;
}

void fn_004BDB70(void* context) {
    LoopRegisterNode* ctx;
    LoopRegisterBlock* link;
    LoopRegisterBlock* otherLink;
    PCode* instr;
    PCode* next;
    PCode* otherInstr;
    PCodeArg* arg;
    SInt32 count;
    SInt32 hasScheduled;
    SInt32 changed;

    ctx = (LoopRegisterNode*)context;
    if (!ctx->destinationBlock) {
        return;
    }
    hasScheduled = FALSE;
    for (link = ctx->blocks; link; link = link->next) {
        for (instr = link->block->firstPCode; instr; instr = instr->nextPCode) {
            if (instr->flags & 8) {
                hasScheduled = TRUE;
            }
        }
    }
    if (hasScheduled) {
        return;
    }
    fn_004BDF30(ctx);
    do {
        changed = FALSE;
        for (link = ctx->blocks; link; link = link->next) {
            for (instr = link->block->firstPCode; instr; instr = next) {
                next = instr->nextPCode;
                {
                    SInt16 allowed;
                    count = instr->argCount;
                    arg = instr->args;
                    while (count--) {
                        if (arg->kind == PCOp_REGISTER &&
                            (arg->effect & 1) &&
                            value_register_uses_60B6EC[(SInt8)arg->arg][(SInt16)arg->data.reg.reg] >
                                0) {
                            allowed = FALSE;
                            goto test_eligibility;
                        }
                        arg++;
                    }
                    if (instr->flags & 0x20002) {
                        for (otherLink = ctx->blocks; otherLink; otherLink = otherLink->next) {
                            for (otherInstr = otherLink->block->firstPCode; otherInstr;
                                 otherInstr = otherInstr->nextPCode) {
                                if ((otherInstr->flags & 0x40004) && may_alias(instr, otherInstr)) {
                                    allowed = FALSE;
                                    goto test_eligibility;
                                }
                            }
                        }
                    }
                    allowed = TRUE;
test_eligibility:
                    if (allowed && fn_004BDE20(instr, context)) {
                        fn_004BDD00(instr, context);
                        changed = TRUE;
                    }
                }
            }
        }
    } while (changed);
}

void fn_004BC580(PCode* instr, SInt8 useExisting) {
    UInt32 flags;
    PCodeArg* dest;
    PCodeArg* arg;
    PCodeArg other;
    PCode* newInstr;
    RegValue* entry;
    StackedValue* undo;
    AvailableValue* node;
    ValueLabel** cursor;
    ValueLabel* use;
    SInt32 count;
    SInt32 i;
    SInt32 unique;

    flags = instr->flags;
    {
        SInt32 index;
        if (flags & 0x80000000U) {
            index = 1;
        } else {
            index = 0;
        }
        dest = &instr->args[index];
    }
    if (flags & 0x4000) {
        PCodeArg* scanDest = &instr->args[(flags & 0x80000000U) ? 1 : 0];
        if (!(flags & 0x180)) {
            i = 0;
            count = instr->argCount;
            arg = instr->args;
            if (count > 0) {
                do {
                    if (arg != scanDest &&
                        arg->kind == scanDest->kind &&
                        arg->arg == scanDest->arg &&
                        arg->data.reg.reg == scanDest->data.reg.reg) {
                        goto not_unique;
                    }
                    i++;
                    arg++;
                } while (i < count);
            }
            unique = TRUE;
            goto checked_unique;
        }
not_unique:
        unique = FALSE;
checked_unique:
        if (unique && !(flags & 0x80)) {
            if (useExisting && fn_004BD1D0(instr, &other)) {
                if (regvalue[(SInt8)dest->arg][(SInt16)dest->data.reg.reg].number !=
                    regvalue[(SInt8)other.arg][(SInt16)other.data.reg.reg].number) {
                    newInstr = makecopyinstruction(&other, dest);
                    insertpcodebefore(instr, newInstr);
                    fn_004BD690(&other, dest);
                }
                deletepcode(instr);
                cg_codegen_setup_6359B0 = 1;
                return;
            }
            fn_004BCFC0(instr);
            return;
        }
    }
    i = 0;
    arg = instr->args;
    if (instr->argCount > 0) {
        do {
            if (arg->kind == PCOp_REGISTER && (arg->effect & 2)) {
                entry = &regvalue[(SInt8)arg->arg][(SInt16)arg->data.reg.reg];
                if (entry->number < valueceiling && nextvaluenumber >= valueceiling) {
                    undo = oalloc(0x30);
                    undo->next = stackedvalues;
                    stackedvalues = undo;
                    undo->op = *arg;
                    undo->value = *entry;
                }
                node = (AvailableValue*)entry->available;
                if (node) {
                    cursor = &node->labelled;
                    for (use = *cursor; use; use = *cursor) {
                        if ((UInt16)use->op.data.reg.reg == (UInt16)arg->data.reg.reg) {
                            *cursor = use->next;
                        } else {
                            cursor = &use->next;
                        }
                    }
                }
                entry->available = 0;
                entry->copyreg = 0;
                entry->number = nextvaluenumber++;
            }
            i++;
            arg++;
        } while (i < instr->argCount);
    }
}

SInt32 fn_004BD400(AvailableValue* node, PCode* instr) {
    PCode* original = (PCode*)node->pcode;
    PCodeArg* oldArg = original->args;
    PCodeArg* arg = instr->args;
    UInt32 destination;
    SInt32 i;
    if (instr->flags & 0x80000000) {
        destination = 1;
    } else {
        destination = 0;
    }

    if (instr->alias != original->alias || instr->unk38 != original->unk38) {
        return FALSE;
    }
    for (i = 0; i < instr->argCount; i++, arg++, oldArg++) {
        if (i == destination) {
            continue;
        }
        switch (oldArg->kind) {
            case 0:
                if (node->opnumbers[i] !=
                    regvalue[(SInt8)arg->arg][(SInt16)arg->data.reg.reg].number) {
                    return FALSE;
                }
                break;
            case 1:
                if (arg->data.reg.reg != oldArg->data.reg.reg) {
                    return FALSE;
                }
                break;
            case 4:
                if (arg->kind != 4) {
                    return FALSE;
                }
                if (arg->data.mem.obj != oldArg->data.mem.obj) {
                    return FALSE;
                }
                if (arg->data.mem.offset != oldArg->data.mem.offset) {
                    return FALSE;
                }
                break;
            case 5:
                if (arg->kind != 5) {
                    return FALSE;
                }
                if (arg->unknown14 != oldArg->unknown14) {
                    return FALSE;
                }
                if (!CInt64_Equal(arg->data.cint, oldArg->data.cint)) {
                    return FALSE;
                }
                break;
            case 2:
                if (arg->kind != 2) {
                    return FALSE;
                }
                if (arg->data.imm.value != oldArg->data.imm.value) {
                    return FALSE;
                }
                break;
            case 3:
                if (arg->kind != 3) {
                    return FALSE;
                }
                if (!CInt64_Equal(arg->data.cint, oldArg->data.cint)) {
                    return FALSE;
                }
                break;
            case 6:
                if (arg->kind != 6) {
                    return FALSE;
                }
                if (arg->data.label.label != oldArg->data.label.label) {
                    return FALSE;
                }
                break;
        }
    }
    if ((instr->flags & 0x20002) && instr->alias->valuenumber != node->aliasnumber) {
        return FALSE;
    }
    return TRUE;
}

SInt32 fn_004BD1D0(PCode* instr, PCodeArg* result) {
    AvailableValue* node;
    ValueLabel* use;
    PCodeArg saved;
    PCodeArg* destination;
    SInt32 equivalent;

    for (node = (AvailableValue*)opvalue[instr->op]; node; node = node->next) {
        if (node->labelled &&
            ((PCode*)node->pcode)->flags == instr->flags &&
            ((PCode*)node->pcode)->argCount == instr->argCount) {
            equivalent = fn_004BD400(node, instr);
            if (equivalent) {
                goto found;
            }
            if (instr->flags & 0x2000) {
                saved = instr->args[1];
                instr->args[1] = instr->args[2];
                instr->args[2] = saved;
                equivalent = fn_004BD400(node, instr);
                if (equivalent) {
                    goto found;
                }
                saved = instr->args[1];
                instr->args[1] = instr->args[2];
                instr->args[2] = saved;
            }
            continue;
found:
            {
                {
                    PCode* previous = (PCode*)node->pcode;
                    SInt32 index;
                    if (previous->flags & 0x80000000) {
                        index = 1;
                    } else {
                        index = 0;
                    }
                    destination = &previous->args[index];
                }
                for (use = node->labelled; use; use = use->next) {
                    if ((SInt16)use->op.data.reg.reg == destination->data.reg.reg) {
                        *result = use->op;
                        goto success;
                    }
                }
                *result = node->labelled->op;
success:
                return TRUE;
            }
        }
    }
    return FALSE;
}

#define INVALIDATE_VALUE_DESTINATIONS() \
    do { \
        i = 0; \
        arg = instr->args; \
        while (i < instr->argCount) { \
            if (arg->kind == PCOp_REGISTER && (arg->effect & 2)) { \
                entry = &regvalue[(SInt8)arg->arg][(SInt16)arg->data.reg.reg]; \
                if (entry->number < valueceiling && nextvaluenumber >= valueceiling) { \
                    undo = oalloc(0x30); \
                    undo->next = stackedvalues; \
                    stackedvalues = undo; \
                    undo->op = *arg; \
                    undo->value = *entry; \
                } \
                node = (AvailableValue*)entry->available; \
                if (node) { \
                    cursor = &node->labelled; \
                    for (use = *cursor; use; use = *cursor) { \
                        if ((UInt16)use->op.data.reg.reg == (UInt16)arg->data.reg.reg) { \
                            *cursor = use->next; \
                        } else { \
                            cursor = &use->next; \
                        } \
                    } \
                } \
                entry->available = 0; \
                entry->copyreg = 0; \
                entry->number = nextvaluenumber++; \
            } \
            i++; \
            arg++; \
        } \
    } while (0)

void fn_004BBCE0(PCodeBlock* block) {
    PCode* instr;
    PCode* next;
    PCode* move;
    PCodeArg* arg;
    PCodeArg* src;
    PCodeArg* dest;
    PCodeArg existing;
    RegValue* entry;
    StackedValue* undo;
    AvailableValue* node;
    ValueLabel* use;
    ValueLabel** cursor;
    SInt32 i;
    SInt32 cls;
    SInt32 prefer;
    SInt32 unique;
    UInt32 flags;
    UInt16 op;
    SInt8 useExisting;

    useExisting = !(block->flags & 0x180) && block->pcodeCount != 0;
    for (instr = block->firstPCode; instr; instr = next) {
        next = instr->nextPCode;
        if (useExisting) {
            for (i = 0, arg = instr->args; i < instr->argCount; i++, arg++) {
                if (arg->kind == PCOp_REGISTER && (arg->effect & 0xB) == 1) {
                    cls = (SInt8)arg->arg;
                    if ((SInt16)arg->data.reg.reg >= n_real_registers[cls]) {
                        entry = &regvalue[cls][(SInt16)arg->data.reg.reg];
                        prefer = entry->copyreg;
                        if (prefer &&
                            prefer >= n_real_registers[cls] &&
                            entry->number == regvalue[cls][prefer].number) {
                            arg->data.reg.reg = prefer;
                        }
                    }
                }
            }
        }
        flags = instr->flags;
        if (flags & 0x10) {
            op = instr->op;
            if (op >= 0x21F && op <= 0x2E7) {
                dest = &instr->args[1];
                src = &instr->args[2];
            } else {
                dest = &instr->args[0];
                src = &instr->args[1];
            }
            if (useExisting &&
                regvalue[(SInt8)src->arg][(SInt16)src->data.reg.reg].number ==
                    regvalue[(SInt8)dest->arg][(SInt16)dest->data.reg.reg].number) {
                deletepcode(instr);
                cg_codegen_setup_6359B0 = 1;
            } else {
                fn_004BD690(src, dest);
            }
        } else if (flags & 8) {
            fn_004BC860(instr);
        } else if (flags & 2) {
            if (flags & 0x20) {
                fn_004BC9B0(instr, useExisting);
            } else {
                fn_004BCC20(instr, useExisting);
            }
        } else if (flags & 4) {
            update_alias_value((void*)instr->alias, instr);
            INVALIDATE_VALUE_DESTINATIONS();
        } else if (flags & 0x20000) {
            fn_004BC580(instr, useExisting);
        } else if (flags & 0x40000) {
            update_alias_value((void*)instr->alias, instr);
            INVALIDATE_VALUE_DESTINATIONS();
        } else if (!(flags & 0x4000)) {
            INVALIDATE_VALUE_DESTINATIONS();
        } else {
            dest = &instr->args[(flags & 0x80000000) != 0];
            unique = !(flags & 0x180);
            if (unique) {
                for (i = 0, arg = instr->args; i < instr->argCount; i++, arg++) {
                    if (arg != dest &&
                        arg->kind == dest->kind &&
                        arg->arg == dest->arg &&
                        (SInt16)arg->data.reg.reg == (SInt16)dest->data.reg.reg) {
                        unique = FALSE;
                        break;
                    }
                }
            }
            if (unique) {
                if (useExisting && fn_004BD1D0(instr, &existing)) {
                    if (regvalue[(SInt8)dest->arg][(SInt16)dest->data.reg.reg].number !=
                        regvalue[(SInt8)existing.arg][(SInt16)existing.data.reg.reg].number) {
                        move = makecopyinstruction(&existing, dest);
                        insertpcodebefore(instr, move);
                        fn_004BD690(&existing, dest);
                    }
                    deletepcode(instr);
                    cg_codegen_setup_6359B0 = 1;
                } else {
                    fn_004BCFC0(instr);
                }
            } else {
                INVALIDATE_VALUE_DESTINATIONS();
            }
        }
    }
    block->flags |= 4;
}
#undef INVALIDATE_VALUE_DESTINATIONS

void fn_004BCC20(PCode* instr, SInt8 useExisting) {
    SInt32 destIndex;
    UInt32 flags;
    PCodeArg* dest;
    PCodeArg* arg;
    PCodeArg other;
    PCode* earlier;
    PCode* replacement;
    RegValue* entry;
    StackedValue* undo;
    AvailableValue* node;
    ValueLabel* use;
    ValueLabel** cursor;
    SInt32 numOutputs;
    SInt32 i;
    SInt32 valid;
    volatile SInt32 result;

    flags = instr->flags;
    if (flags & 0x80000000) {
        destIndex = 1;
    } else {
        destIndex = 0;
    }
    dest = &instr->args[destIndex];
    if (flags & 0x200080) {
        goto invalidate;
    }
    if (flags & 0x180) {
        valid = FALSE;
    } else {
        numOutputs = 0;
        arg = instr->args;
        for (i = 0; i < instr->argCount; i++, arg++) {
            if (arg->kind == PCOp_REGISTER && (arg->effect & 2)) {
                numOutputs++;
            }
        }
        valid = numOutputs == 1;
    }
    if (valid) {
        if (useExisting && fn_004BD1D0(instr, &other)) {
            if (regvalue[(SInt8)dest->arg][(SInt16)dest->data.reg.reg].number !=
                regvalue[(SInt8)other.arg][(SInt16)other.data.reg.reg].number) {
                replacement = makecopyinstruction(&other, dest);
                insertpcodebefore(instr, replacement);
                fn_004BD690(&other, dest);
            }
            deletepcode(instr);
            cg_codegen_setup_6359B0 = 1;
            return;
        }
        earlier = instr->alias->valuepcode;
        if (!earlier || (earlier->flags & 0x200000)) {
            earlier = NULL;
        } else if (instr->alias->valuenumber !=
                   regvalue[(SInt8)earlier->args[0].arg][(SInt16)earlier->args[0].data.reg.reg]
                       .number) {
            earlier = NULL;
        } else {
            other = earlier->args[0];
        }
        if (earlier) {
            result = can_reuse_stored_value(earlier, instr);
        }
        if (!useExisting || !earlier || !result) {
            fn_004BCFC0(instr);
            return;
        }
        if (result != 1 ||
            regvalue[(SInt8)dest->arg][(SInt16)dest->data.reg.reg].number !=
                regvalue[(SInt8)other.arg][(SInt16)other.data.reg.reg].number) {
            replacement = makecopyforload(result, &other, dest);
            insertpcodebefore(instr, replacement);
            if (!(replacement->flags & 0x10)) {
                fn_004BCFC0(replacement);
            } else {
                fn_004BD690(&other, dest);
            }
        }
        deletepcode(instr);
        cg_codegen_setup_6359B0 = 1;
        return;
    }
invalidate:
    i = 0;
    arg = instr->args;
    if (instr->argCount > 0) {
        do {
            if (arg->kind == PCOp_REGISTER && (arg->effect & 2)) {
                entry = &regvalue[(SInt8)arg->arg][(SInt16)arg->data.reg.reg];
                if (entry->number < valueceiling && nextvaluenumber >= valueceiling) {
                    undo = oalloc(0x30);
                    undo->next = stackedvalues;
                    stackedvalues = undo;
                    undo->op = *arg;
                    undo->value = *entry;
                }
                node = (AvailableValue*)entry->available;
                if (node) {
                    cursor = &node->labelled;
                    for (use = *cursor; use; use = *cursor) {
                        if ((UInt16)use->op.data.reg.reg == (UInt16)arg->data.reg.reg) {
                            *cursor = use->next;
                        } else {
                            cursor = &use->next;
                        }
                    }
                }
                entry->available = 0;
                entry->copyreg = 0;
                entry->number = nextvaluenumber++;
            }
            i++;
            arg++;
        } while (i < instr->argCount);
    }
}

void fn_004BC9B0(PCode* instr, SInt8 useExisting) {
    SInt32 destIndex;
    UInt32 flags;
    PCodeArg* dest;
    PCodeArg* arg;
    PCodeArg* scan;
    PCodeArg other;
    PCode* move;
    RegValue* entry;
    StackedValue* undo;
    AvailableValue* node;
    ValueLabel* use;
    ValueLabel** cursor;
    SInt32 numOutputs;
    SInt32 i;
    SInt32 valid;

    flags = instr->flags;
    if (flags & 0x80000000) {
        destIndex = 1;
    } else {
        destIndex = 0;
    }
    dest = &instr->args[destIndex];
    if (!(flags & 0x200080)) {
        if (flags & 0x180) {
            valid = FALSE;
        } else {
            numOutputs = 0;
            scan = instr->args;
            for (i = 0; i < instr->argCount; i++, scan++) {
                if (scan->kind == PCOp_REGISTER && (scan->effect & 2)) {
                    numOutputs++;
                }
            }
            valid = numOutputs == 1;
        }
        if (valid) {
            if (useExisting && fn_004BD1D0(instr, &other)) {
                if (regvalue[(SInt8)dest->arg][(SInt16)dest->data.reg.reg].number !=
                    regvalue[(SInt8)other.arg][(SInt16)other.data.reg.reg].number) {
                    move = makecopyinstruction(&other, dest);
                    insertpcodebefore(instr, move);
                    fn_004BD690(&other, dest);
                }
                deletepcode(instr);
                cg_codegen_setup_6359B0 = 1;
                return;
            }
            fn_004BCFC0(instr);
            return;
        }
    }
    i = 0;
    arg = instr->args;
    if (instr->argCount > 0) {
        do {
            if (arg->kind == PCOp_REGISTER && (arg->effect & 2)) {
                entry = &regvalue[(SInt8)arg->arg][(SInt16)arg->data.reg.reg];
                if (entry->number < valueceiling && nextvaluenumber >= valueceiling) {
                    undo = oalloc(0x30);
                    undo->next = stackedvalues;
                    stackedvalues = undo;
                    undo->op = *arg;
                    undo->value = *entry;
                }
                node = (AvailableValue*)entry->available;
                if (node) {
                    cursor = &node->labelled;
                    for (use = *cursor; use; use = *cursor) {
                        if ((UInt16)use->op.data.reg.reg == (UInt16)arg->data.reg.reg) {
                            *cursor = use->next;
                        } else {
                            cursor = &use->next;
                        }
                    }
                }
                entry->available = 0;
                entry->copyreg = 0;
                entry->number = nextvaluenumber++;
            }
            i++;
            arg++;
        } while (i < instr->argCount);
    }
}

void fn_004BD690(PCodeArg* source, PCodeArg* destination) {
    RegValue* sourceEntry;
    RegValue* destEntry;
    StackedValue* undo;
    AvailableValue* node;
    ValueLabel* use;
    ValueLabel** cursor;
    SInt32 reg;

    sourceEntry = &regvalue[(SInt8)source->arg][(SInt16)source->data.reg.reg];
    destEntry = &regvalue[(SInt8)destination->arg][(SInt16)destination->data.reg.reg];
    if (destEntry->number < valueceiling && nextvaluenumber >= valueceiling) {
        undo = oalloc(0x30);
        undo->next = stackedvalues;
        stackedvalues = undo;
        undo->op = *destination;
        undo->value = *destEntry;
    }
    node = (AvailableValue*)destEntry->available;
    if (node) {
        cursor = &node->labelled;
        for (use = *cursor; use; use = *cursor) {
            if ((UInt16)use->op.data.reg.reg == (UInt16)destination->data.reg.reg) {
                *cursor = use->next;
            } else {
                cursor = &use->next;
            }
        }
    }
    destEntry->available = sourceEntry->available;
    node = (AvailableValue*)destEntry->available;
    if (node) {
        use = oalloc(sizeof(ValueLabel));
        use->op = *destination;
        use->next = node->labelled;
        node->labelled = use;
    }
    destEntry->number = sourceEntry->number;
    reg = sourceEntry->copyreg;
    if (reg == 0 || sourceEntry->number != regvalue[(SInt8)source->arg][reg].number) {
        destEntry->copyreg = (SInt16)source->data.reg.reg;
    } else {
        destEntry->copyreg = reg;
    }
}
