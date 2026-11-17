#include "compiler/mips.h"
#include "compiler/BE_binary.h"
#include "compiler/Registers.h"
#include "compiler/RegisterInfo.h"
#include "compiler/Exceptions.h"
#include "compiler/Alias.h"
#include "compiler/StructMoves.h"
#include "compiler/CMachine.h"
#include "compiler/VReg.h"
#include "compiler/objects.h"
#include "compiler/common.h"
#include "compiler/FunctionCalls.h"
#include "compiler/ObjGen.h"
#include "compiler/CodeGen.h"
#include "compiler/InstrSelection.h"
#include "compiler/Operands.h"
#include "compiler/PCodeUtilities.h"
#include "compiler/types.h"
#include "compiler/enode.h"
#include "compiler/CompilerTools.h"
#include "compiler/CError.h"
#include "compiler/PCode.h"
#include "compiler/PCodeUtilities.h"
#include "compiler/CParser.h"
#include "compiler/CFunc.h"
#include "compiler/CTemplateTools.h"

#pragma defer_codegen off
#pragma opt_propagation off
void fn_004B3BD0(Object* target, int reg, UInt32* usedRegisters) {
    Operand operand;
    PCode* instr;
    PCodeArg* ops;
    UInt32 mask[RegClassMax];
    SInt16 savedMove;
    SInt32 directOpcode;
    SInt32 indirectOpcode;
    SInt32 extra;
    SInt32 exceptions;
    SInt32 base;
    SInt32 j;
    SInt8 cls;
    Boolean passBase;

    savedMove = move_opcode;
    directOpcode = 0x3c;
    indirectOpcode = 0x3d;
    base = 0;
    passBase = FALSE;
    if (target && reg) {
        base = reg;
        reg = 0;
    }
    if (copts.far_call && target) {
        operand = make_object_operand(fn_00438B60(target));
        indirect(&operand, NULL);
        if (operand.kind) {
            Coerce_to_register(&operand, TYPE(&stsignedint), 0);
        }
        reg = operand.reg;
        target = NULL;
    }
    exceptions = 0;
    if (copts.cplusplus && copts.exceptions && current_statement) {
        exceptions = fn_004A98E0(current_statement->dobjstack);
    }
    extra = fn_004727F0();
    extra = fn_0046D3F0() - extra;
    if (!reg) {
        if (!base) {
            instr = make_instruction((UInt16)directOpcode, exceptions + 2 + extra);
        } else {
            emitpcode(savedMove, 0x19, (SInt16)base);
            usedRegisters[0] |= 0x2000000;
            passBase = FALSE;
            instr = make_instruction((UInt16)directOpcode, exceptions + 3 + extra);
        }
    } else if (!base) {
        instr = make_instruction((UInt16)indirectOpcode, extra + 2 + exceptions);
    } else {
        emitpcode(savedMove, 0x19, (SInt16)base);
        usedRegisters[0] |= 0x2000000;
        instr = make_instruction((UInt16)indirectOpcode, exceptions + 3 + extra);
    }
    binary_word_count_destination->gprmask |= 0x80000000;
    if (!target || target == rt_MIPS16_prtglue16 || target == rt_MIPS16_prtglue32) {
        for (cls = 0; cls < RegClassMax; cls++) {
            mask[cls] = -1;
        }
    } else {
        fn_0046D200(target, mask);
    }
    ops = &instr->args[2];
    for (cls = 0; cls < RegClassMax; cls++) {
        if (cls == 5) {
            break;
        }
        for (j = 0; j < n_scratch_registers[cls]; j++) {
            SInt32 number = scratch_registers[cls][j];
            UInt32 bit = 1U << number;
            ops->kind = PCOp_REGISTER;
            ops->arg = cls;
            ops->data.reg.reg = number;
            ops->effect = 0;
            if (mask[cls] & bit) {
                ops->effect |= 2;
            }
            if (usedRegisters[cls] & bit) {
                ops->effect |= 1;
            }
            ops++;
        }
    }
    if (passBase) {
        ops->kind = PCOp_REGISTER;
        ops->arg = 0;
        ops->data.reg.reg = 0x19;
        ops->effect = 1;
        ops++;
    }
    if (copts.cplusplus && copts.exceptions && current_statement) {
        noteexceptionactionregisters(current_statement->dobjstack, ops);
    }
    if (!copts.unknown08 || reg) {
        if (!reg) {
            instr->args[0].kind = PCOp_MEMORY;
            instr->args[0].data.mem.offset = 0;
            instr->args[0].data.mem.obj = target;
            instr->args[1] = fn_004D02D0(0x1f, 0, 2);
            instr->unk38 = 4;
            instr->alias = worst_case_func;
            instr->object = target;
        } else {
            instr->args[0] = fn_004D02D0(0x1f, 0, 2);
            instr->args[1] = fn_004D02D0(reg, 0, 1);
        }
    } else {
        instr->op = 0x38;
        instr->args[0].kind = PCOp_MEMORY;
        instr->args[0].data.mem.offset = 0;
        instr->args[0].data.mem.obj = target;
        instr->unk38 = 10;
        instr->object = target;
        instr->alias = worst_case_func;
    }
    appendpcode(pclastblock, instr);
    new_basic_block();
    if (copts.cplusplus && copts.exceptions && current_statement) {
        recordexceptionactions(instr, current_statement->dobjstack);
    }
}
#pragma opt_propagation on
#pragma defer_codegen on

void fn_004B4200(CallArgument* arg) {
    Type* type;
    UInt8 kind;
    SInt16 opcode;
    SInt16 reg;
    Operand local;

    type = arg->node->rtype;
    kind = type->type;
    if (kind == TYPEINT ||
        kind == TYPEENUM ||
        kind == TYPEPOINTER ||
        (kind == TYPEMEMBERPOINTER && type->size == 4)) {
        if (!arg->used) {
            gen_func_table[arg->node->type](arg->node, 0, &arg->operand);
        }
        if (arg->operand.kind) {
            Coerce_to_register(&arg->operand, type, 0);
        }
        fn_0048F710(target_register_637328, arg->operand.reg, 0x1d, NULL, arg->stackOffset);
    } else if (kind == TYPEFLOAT) {
        if (!arg->used) {
            gen_func_table[arg->node->type](arg->node, 0, &arg->operand);
        }
        if (arg->operand.kind) {
            Coerce_to_register(&arg->operand, type, 0);
        }
        if (IS_TYPE_FLOAT(type) &&
            ((IS_TYPE_FLOAT32(type) && !(copts.fp_library & 1)) ||
             (IS_TYPE_FLOAT64(type) && !(copts.fp_library & 2)))) {
            if (IS_TYPE_FLOAT32(type) && !(arg->flags & 0x10)) {
                if (!copts.littleendian) {
                    fn_0048F710(0x481, arg->operand.reg, 0x1d, NULL, arg->stackOffset + 4);
                } else {
                    fn_0048F710(0x481, arg->operand.reg, 0x1d, NULL, arg->stackOffset);
                }
            } else if (IS_TYPE_FLOAT64(type)) {
                fn_0048F710(0x479, arg->operand.reg, 0x1d, NULL, arg->stackOffset);
            } else if (copts.littleendian) {
                fn_0048F710(0x481, arg->operand.reg, 0x1d, NULL, arg->stackOffset);
                fn_0048F710(
                    0x481, arg->operand.regHi, 0x1d, NULL, arg->stackOffset + register_size
                );
            } else {
                fn_0048F710(0x481, arg->operand.regHi, 0x1d, NULL, arg->stackOffset);
                fn_0048F710(0x481, arg->operand.reg, 0x1d, NULL, arg->stackOffset + register_size);
            }
        } else {
            if (IS_TYPE_FLOAT32(type) && !(arg->flags & 0x10)) {
                if (!copts.littleendian) {
                    fn_0048F5D0(0x483, arg->operand.reg, 0x1d, NULL, arg->stackOffset + 4);
                } else {
                    fn_0048F5D0(0x483, arg->operand.reg, 0x1d, NULL, arg->stackOffset);
                }
            } else {
                opcode = (copts.unknown0C >= 2 && (copts.fp_library & 2)) ? 0x47a : 0x476;
                fn_0048F5D0(opcode, arg->operand.reg, 0x1d, NULL, arg->stackOffset);
            }
        }
    } else {
        local.kind = 2;
        local.reg = 0x1d;
        local.object = NULL;
        local.unk20 = 0;
        local.immOffset = arg->stackOffset;
        if (!arg->used) {
            if (arg->node->type == EINDIRECT) {
                gen_func_table[arg->node->data.monadic->type](
                    arg->node->data.monadic, 0, &arg->operand
                );
            } else {
                gen_func_table[arg->node->type](arg->node, 0, &arg->operand);
            }
        } else if (arg->node->type == EINDIRECT && arg->operand.kind == 4) {
            arg->operand.kind = 2;
        }
        reg = CMach_GetTypeAlign(type);
        fn_0053CC40(&local, &arg->operand, type->size, reg);
    }
}

CallArgument* fn_004B49C0(ENode* skip, CallArgumentList* args, FuncArg* formal) {
    CallArgument* first;
    CallArgument* last;
    CallArgument* entry;
    ENode* node;
    Type* type;
    SInt32 offset;
    SInt32 gpr;
    SInt32 fpr;

    first = NULL;
    gpr = 4;
    fpr = 12;
    offset = 0;
    for (; args; args = args->next) {
        node = args->node;
        if (node == skip) {
            if (formal) {
                formal = formal->next;
            }
            continue;
        }
        type = node->rtype;
        if (!first) {
            last = lalloc(sizeof(CallArgument));
            last->next = NULL;
            last->node = node;
            last->stackOffset = -1;
            last->gpr = -1;
            last->fpr = -1;
            last->used = 0;
            last->flags = 0;
            last->type = node->cost;
            first = last;
        } else {
            entry = lalloc(sizeof(CallArgument));
            entry->next = NULL;
            entry->node = node;
            entry->stackOffset = -1;
            entry->gpr = -1;
            entry->fpr = -1;
            entry->used = 0;
            entry->flags = 0;
            entry->type = node->cost;
            last->next = entry;
            last = last->next;
        }
        if (IS_TYPE_FLOAT(type) || fn_0046FF70(type)) {
            if (IS_TYPE_FLOAT(type) &&
                ((IS_TYPE_FLOAT32(type) && !(copts.fp_library & 1)) ||
                 (IS_TYPE_FLOAT64(type) && !(copts.fp_library & 2)))) {
                if (gpr > (copts.unknown15 ? (UInt8)11 : (UInt8)7)) {
                    last->flags |= 4;
                    codegen_flag_637383 = TRUE;
                    last->stackOffset = offset;
                    offset += register_size;
                } else {
                    last->flags |= 1;
                    last->gpr = gpr++;
                }
            } else if (!formal || formal == &elipsis || formal == &oldstyle) {
                if (fpr > (copts.unknown15 ? (UInt8)19 : (UInt8)15)) {
                    last->flags |= 4;
                    codegen_flag_637383 = TRUE;
                    last->stackOffset = offset;
                    offset += register_size;
                } else {
                    last->flags |= 2;
                    last->fpr = fpr++;
                }
            } else {
                if (fpr > (copts.unknown15 ? (UInt8)19 : (UInt8)15)) {
                    last->flags |= 4;
                    codegen_flag_637383 = TRUE;
                    last->stackOffset = offset;
                    offset += register_size;
                } else {
                    last->flags |= 2;
                    last->fpr = fpr++;
                }
            }
        } else if (type->type == TYPEINT ||
                   type->type == TYPEENUM ||
                   type->type == TYPEPOINTER ||
                   (type->type == TYPEMEMBERPOINTER && type->size == 4)) {
            if ((!formal || formal == &elipsis || formal == &oldstyle) && type->size < 4) {
                last->flags |= 8;
            }
            if (gpr > (copts.unknown15 ? (UInt8)11 : (UInt8)7)) {
                last->flags |= 4;
                codegen_flag_637383 = TRUE;
                last->stackOffset = offset;
                offset += register_size;
            } else {
                last->flags |= 1;
                last->gpr = gpr++;
            }
        } else {
            if (type->size > register_size) {
                CError_Internal("FunctionCalls.c", 410);
            }
            if (gpr > (copts.unknown15 ? (UInt8)11 : (UInt8)7)) {
                last->flags |= 4;
                codegen_flag_637383 = TRUE;
                last->stackOffset = offset;
                offset += register_size;
            } else {
                last->flags |= 1;
                last->gpr = gpr++;
            }
        }
        if (formal && formal != &elipsis && formal != &oldstyle) {
            formal = formal->next;
        }
    }
    if (offset > be_macro_adjust_offset_6356A4) {
        be_macro_adjust_offset_6356A4 = offset;
    }
    be_macro_adjust_offset_6356A4 =
        (be_macro_adjust_offset_6356A4 + stack_alignment_63560C - 1) & -stack_alignment_63560C;
    return first;
}
CallArgument* fn_004B45A0(ENode* skip, CallArgumentList* args, FuncArg* formal) {
    CallArgument* first;
    CallArgument* last;
    CallArgument* entry;
    ENode* node;
    Type* type;
    UInt32 offset;
    SInt32 gpr;
    SInt32 fpr;
    UInt8 kind;

    first = NULL;
    offset = 0;
    gpr = 4;
    fpr = 12;
    for (; args; args = args->next) {
        node = args->node;
        if (node == skip) {
            formal = formal->next;
            continue;
        }
        type = node->rtype;
        if (!first) {
            entry = lalloc(sizeof(CallArgument));
            entry->next = NULL;
            entry->node = node;
            entry->stackOffset = -1;
            entry->gpr = -1;
            entry->fpr = -1;
            entry->used = 0;
            entry->flags = 0;
            entry->type = node->cost;
            first = entry;
        } else {
            entry = lalloc(sizeof(CallArgument));
            entry->next = NULL;
            entry->node = node;
            entry->stackOffset = -1;
            entry->gpr = -1;
            entry->fpr = -1;
            entry->used = 0;
            entry->flags = 0;
            entry->type = node->cost;
            last->next = entry;
        }
        last = entry;
        kind = type->type;
        if (kind == TYPEFLOAT || fn_0046FF70(type)) {
            if (kind == TYPEFLOAT &&
                (((TYPE_INTEGRAL(type)->integral == IT_FLOAT || type->size == 4) &&
                  !(copts.fp_library & 1)) ||
                 ((TYPE_INTEGRAL(type)->integral == IT_DOUBLE || type->size == 8) &&
                  !(copts.fp_library & 2)))) {
                if (gpr > (copts.unknown15 ? (UInt8)11 : (UInt8)7)) {
                    entry->flags |= 4;
                    codegen_flag_637383 = TRUE;
                    entry->stackOffset = offset;
                    offset += register_size;
                } else {
                    entry->flags |= 1;
                    entry->gpr = gpr;
                }
                gpr++;
            } else if (!formal || formal == &elipsis || formal == &oldstyle) {
                if (fpr > (copts.unknown15 ? (UInt8)19 : (UInt8)15)) {
                    entry->flags |= 0x14;
                    codegen_flag_637383 = TRUE;
                    entry->stackOffset = offset;
                    offset += register_size;
                } else {
                    entry->flags |= 2;
                    entry->fpr = fpr;
                    fpr += 2;
                }
            } else {
                if (fpr > (copts.unknown15 ? (UInt8)19 : (UInt8)15)) {
                    entry->flags |= 4;
                    codegen_flag_637383 = TRUE;
                    entry->stackOffset = offset;
                    offset += register_size;
                } else {
                    entry->flags |= 2;
                    entry->fpr = fpr++;
                }
            }
        } else if (kind == TYPEINT ||
                   kind == TYPEENUM ||
                   kind == TYPEPOINTER ||
                   (kind == TYPEMEMBERPOINTER && type->size == 4)) {
            if ((!formal || formal == &elipsis || formal == &oldstyle) && type->size < 4) {
                entry->flags |= 8;
            }
            if ((kind == TYPEINT || kind == TYPEENUM) && type->size == 8) {
                offset = (offset + 7) & ~7U;
            }
            if (gpr > (copts.unknown15 ? (UInt8)11 : (UInt8)7)) {
                entry->flags |= 4;
                codegen_flag_637383 = TRUE;
                entry->stackOffset = offset;
                offset += register_size;
            } else {
                entry->flags |= 1;
                entry->gpr = gpr++;
            }
        } else {
            if (type->size > register_size) {
                CError_Internal("FunctionCalls.c", 594);
            }
            if (gpr > (copts.unknown15 ? (UInt8)11 : (UInt8)7)) {
                entry->flags |= 4;
                codegen_flag_637383 = TRUE;
                entry->stackOffset = offset;
                offset += register_size;
            } else {
                entry->flags |= 1;
                entry->gpr = gpr++;
            }
        }
        if (formal && formal != &elipsis && formal != &oldstyle) {
            formal = formal->next;
        }
    }
    if (offset > be_macro_adjust_offset_6356A4) {
        be_macro_adjust_offset_6356A4 = offset;
    }
    be_macro_adjust_offset_6356A4 =
        (be_macro_adjust_offset_6356A4 + stack_alignment_63560C - 1) & -stack_alignment_63560C;
    return first;
}

void FunctionCalls_GenerateCall(ENode* node, Operand* result) {
    ENode* callee;
    ENode* hidden;
    Type* returnType;
    Type* type;
    Type* effectiveType;
    TypeFunc* callInfo;
    CallArgument* params;
    CallArgument* entry;
    Operand operand;
    UInt32 usedRegs[RegClassMax];
    TypeClass* virtualClass;
    TypeClass* methodClass;
    SInt32 virtualOffset;
    SInt32 i;
    SInt32 reg;
    SInt32 offset;
    SInt32 count;
    SInt16 opcode;
    volatile SInt32 loadOpcode;
    UInt16 placement;
    UInt8 valueClass;
    SInt16 thisReg;

    callee = node->data.funccall.funcref;
    callInfo = node->data.funccall.functype;
    returnType = callInfo->functype;
    hidden = NULL;
    loadOpcode = 0x454;
    for (i = 0; i < RegClassMax; i++) {
        usedRegs[i] = 0;
    }
    if (node->data.funccall.functype->flags & FUNC_PTM) {
        if (returnType &&
            (returnType->type == TYPEARRAY ||
             returnType->type == TYPESTRUCT ||
             returnType->type == TYPECLASS ||
             (returnType->type == TYPEMEMBERPOINTER && returnType->size == 12))) {
            hidden = node->data.funccall.args->next->node;
        } else {
            hidden = node->data.funccall.args->node;
        }
    }
    if (copts.unknown15) {
        params = fn_004B49C0(hidden, node->data.funccall.args, callInfo->args);
    } else {
        params = fn_004B45A0(hidden, node->data.funccall.args, callInfo->args);
    }
    for (entry = params; entry; entry = entry->next) {
        if (entry->node->hascall) {
            gen_func_table[entry->node->type](entry->node, 0, &entry->operand);
            entry->used = TRUE;
        }
    }
    if (callee->hascall) {
        gen_func_table[callee->type](callee, 0, &operand);
        if (operand.kind) {
            Coerce_to_register(&operand, TYPE(&void_ptr), 0);
        }
    } else if (hidden && hidden->hascall) {
        gen_func_table[hidden->type](hidden, 0, &operand);
        if (operand.kind) {
            Coerce_to_register(&operand, TYPE(&void_ptr), 0);
        }
    }
    for (entry = params; entry; entry = entry->next) {
        if (entry->flags & 4) {
            fn_004B4200(entry);
        }
    }
    for (entry = params; entry; entry = entry->next) {
        if ((entry->flags & 7) == 5) {
            reg = entry->gpr;
            offset = 0;
            while (reg <= (UInt8)(copts.unknown15 ? 11 : 7)) {
                fn_0048F990(0x454, reg, 0x1d, NULL, entry->stackOffset + offset);
                reg++;
                offset += register_size;
            }
        }
    }
    for (entry = params; entry; entry = entry->next) {
        if (((entry->flags & 7) == 1 || (entry->flags & 7) == 2) && !entry->used) {
            gen_func_table[entry->node->type](entry->node, 0, &entry->operand);
        }
    }
    for (entry = params; entry; entry = entry->next) {
        placement = entry->flags & 7;
        type = entry->node->rtype;
        if (placement == 5) {
            reg = entry->gpr;
            while (reg <= (UInt8)(copts.unknown15 ? 11 : 7)) {
                fn_0048F990(
                    loadOpcode,
                    reg,
                    0x1d,
                    NULL,
                    (reg - entry->gpr) * register_size + entry->stackOffset
                );
                usedRegs[0] |= 1U << reg;
                reg++;
            }
        } else if (placement == 1 &&
                   (type->type == TYPECLASS ||
                    type->type == TYPESTRUCT ||
                    (type->type == TYPEMEMBERPOINTER && type->size == 12)) &&
                   entry->operand.kind) {
            if (entry->operand.kind == 2) {
                entry->operand.kind = 4;
            }
            if (entry->operand.kind != 4) {
                CError_Internal("FunctionCalls.c", 1235);
            }
            count = (type->size >> cg_option_0063646C) +
                ((objgen_alignment_mask_636430 & type->size) != 0);
            if (type->size < 5) {
                if (type->size < 3) {
                    opcode = type->size < 2 ? 0x45a : 0x45b;
                } else {
                    opcode = 0x45f;
                }
            } else {
                opcode = 0x454;
            }
            for (i = 0; i < count; i++) {
                fn_0048F990(
                    opcode,
                    entry->gpr + i,
                    entry->operand.reg,
                    entry->operand.object,
                    register_size * i + entry->operand.immOffset
                );
                note_object_alignment(CMach_GetTypeAlign(type), entry->operand.object);
                usedRegs[0] |= 1U << (entry->gpr + i);
            }
        } else if (placement == 1) {
            if (entry->operand.kind) {
                Coerce_to_register(&entry->operand, type, 0);
            }
            if (entry->operand.kind == 2) {
                entry->operand.kind = 4;
            }
            if (entry->operand.kind == 0) {
                opcode = type->size > 8 ? move_opcode_637350 : move_opcode;
                emitpcode(opcode, (int)entry->gpr, (UInt32)entry->operand.reg);
                usedRegs[0] |= 1U << entry->gpr;
            } else if (entry->operand.kind == 4) {
                type = entry->node->rtype;
                if (type->size > register_size) {
                    CError_Internal("FunctionCalls.c", 1271);
                }
                Coerce_to_register(&entry->operand, type, 0);
                emitpcode(move_opcode, (int)entry->gpr, (UInt32)entry->operand.reg);
                usedRegs[0] |= 1U << entry->gpr;
            } else {
                if (entry->operand.kind != 1) {
                    CError_Internal("FunctionCalls.c", 1314);
                }
                type = entry->node->rtype;
                if (IS_TYPE_FLOAT32(type)) {
                    emitpcode(0xa5, (int)entry->gpr, (UInt32)entry->operand.reg);
                    setpcodeflags(0x4000);
                    usedRegs[0] |= 1U << entry->gpr;
                } else {
                    if (!copts.littleendian) {
                        emitpcode(0xa5, entry->gpr + 1, (UInt32)entry->operand.reg);
                        pclastblock->lastPCode->args[1].effect |= 0x8000;
                        setpcodeflags(0x100);
                        emitpcode(0xa5, (int)entry->gpr, (UInt32)entry->operand.reg);
                        pclastblock->lastPCode->args[1].effect |= 0xc000;
                    } else {
                        emitpcode(0xa5, (int)entry->gpr, (UInt32)entry->operand.reg);
                        pclastblock->lastPCode->args[1].effect |= 0x8000;
                        setpcodeflags(0x100);
                        emitpcode(0xa5, entry->gpr + 1, (UInt32)entry->operand.reg);
                        pclastblock->lastPCode->args[1].effect |= 0xc000;
                    }
                    setpcodeflags(0x100);
                    usedRegs[0] |= 3U << entry->gpr;
                }
            }
        } else if (placement == 2) {
            effectiveType = fn_0046FF70(type);
            if (!effectiveType) {
                effectiveType = entry->node->rtype;
            }
            if (entry->operand.kind) {
                Coerce_to_register(&entry->operand, effectiveType, 0);
            }
            opcode = IS_TYPE_FLOAT32(effectiveType) ? 0x4a3 : 0x4a4;
            emitpcode(opcode, (int)entry->fpr, (UInt32)entry->operand.reg);
            usedRegs[1] |= 1U << entry->fpr;
        }
    }
    if (callee->type == EOBJREF) {
        if (!(callee->flags & ENODE_FLAG_NONVIRTUAL) &&
            CParser_IsVirtualFunction(callee->data.objref, &virtualClass, &virtualOffset)) {
            thisReg = returnType &&
                    (returnType->type == TYPESTRUCT ||
                     returnType->type == TYPECLASS ||
                     returnType->type == TYPEARRAY ||
                     (returnType->type == TYPEMEMBERPOINTER && returnType->size == 12))
                ? 5
                : 4;
            methodClass = TYPE_METHOD(callee->data.objref->type)->theclass;
            if (!(methodClass->flags & CLASS_HANDLEOBJECT)) {
                fn_0048F990(0x45f, 0x19, thisReg, NULL, methodClass->vtable->offset);
            } else {
                fn_0048F990(0x45f, 0x19, thisReg, NULL, 0);
                fn_0048F990(
                    0x45f,
                    0x19,
                    0x19,
                    NULL,
                    TYPE_METHOD(callee->data.objref->type)->theclass->vtable->offset
                );
            }
            fn_0048F990(0x45f, 0x19, 0x19, NULL, virtualOffset);
            operand.kind = 0;
            operand.reg = 0x19;
            usedRegs[0] |= 0x2000000 |
                (1U
                 << (returnType &&
                             (returnType->type == TYPESTRUCT ||
                              returnType->type == TYPECLASS ||
                              returnType->type == TYPEARRAY ||
                              (returnType->type == TYPEMEMBERPOINTER && returnType->size == 12))
                         ? 5
                         : 4));
            fn_004B3BD0(NULL, 0x19, usedRegs);
        } else if (!hidden) {
            fn_004B3BD0(callee->data.objref, 0, usedRegs);
        } else {
            if (!hidden->hascall) {
                gen_func_table[hidden->type](hidden, 0, &operand);
                if (operand.kind) {
                    Coerce_to_register(&operand, TYPE(&void_ptr), 0);
                }
            }
            fn_004B3BD0(callee->data.objref, operand.reg, usedRegs);
        }
    } else {
        if (!callee->hascall) {
            gen_func_table[callee->type](callee, 0, &operand);
            if (operand.kind) {
                Coerce_to_register(&operand, TYPE(&void_ptr), 0);
            }
        }
        fn_004B3BD0(NULL, operand.reg, usedRegs);
    }
    if (result) {
        valueClass = fn_00436C70(returnType);
        if (valueClass == 4 || valueClass == 5) {
            if (returnType->type == TYPESTRUCT || returnType->type == TYPECLASS) {
                returnType = fn_00470030(returnType);
            }
            opcode = IS_TYPE_FLOAT32(returnType) ? 0x4a3 : 0x4a4;
            *result = make_paired_register_operand(opcode, 1, FALSE);
        } else if (valueClass == 2 || valueClass == 3) {
            if (!copts.unknown41) {
                *result = make_paired_register_operand(move_opcode, 0, FALSE);
            } else {
                opcode = returnType->size > 8 ? move_opcode_637350 : move_opcode;
                *result = make_paired_register_operand(opcode, 0, FALSE);
            }
        } else {
            if (valueClass != 1) {
                CError_Internal("FunctionCalls.c", 1392);
            }
            result->kind = 3;
            result->immediate = cint64_zero;
        }
    }
}

Boolean fn_004B4DB0(void* match, void* node) {
    return fn_004BA370(node) == match;
}
