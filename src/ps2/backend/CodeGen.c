#include "compiler/BE_macro.h"
#include "compiler/CInline.h"
#include "compiler/InlineAsmMIPS.h"
#include "driver/TargetOptimizerHelpers-mips.h"
#include "compiler/RegisterInfo.h"
#include "compiler/common.h"
#include "compiler/tokens.h"
#include "compiler/Alias.h"
#include "compiler/OpcodeInfo.h"
#include "compiler/objects.h"
#include "compiler/CError.h"
#include "compiler/CParser.h"
#include "compiler/VReg.h"
#include "compiler/Peephole.h"
#include "compiler/mips.h"
#include "compiler/ExprPreparation.h"
#include "compiler/PCodeUtilities.h"
#include "compiler/Peephole.h"
#include "compiler/CMachine.h"
#include "compiler/ObjGen.h"
#include "compiler/CFunc.h"
#include "compiler/BE_binary.h"
#include "compiler/CDecl.h"
#include "compiler/CInit.h"
#include "compiler/CInt64.h"
#include "compiler/CMachine.h"
#include "compiler/CMangler.h"
#include "compiler/COptimizer.h"
#include "compiler/CPrepTokenizer.h"
#include "compiler/CTemplateTools.h"
#include "compiler/CodeGen.h"
#include "compiler/Exceptions.h"
#include "compiler/InterferenceGraph.h"
#include "compiler/Intrinsics.h"
#include "compiler/Operands.h"
#include "compiler/PCode.h"
#include "compiler/PCodeInfo.h"
#include "compiler/RegListQueries.h"
#include "compiler/Registers.h"
#include "compiler/ValueNumbering.h"
#include "driver/COSToolsCLT.h"
#include "compiler/ConditionalMoveOptimization.h"
#include "compiler/LiveRegisters.h"
#include "compiler/MIPSSimulation.h"
#include "compiler/DeadCodeElimination.h"
#include "compiler/BE_schedule.h"
#include <string.h>
#include <stdio.h>
#include "compiler/Coloring.h"
#include "compiler/FunctionCalls.h"
#include "driver/CCompiler.h"
#include "compiler/Scheduler.h"
#include "driver/CWParserPluginsContext.h"

SInt32 fn_00438D70(void) {
    return copts.unknown04;
}

void assign_labels(Statement* stmt) {
    Statement* previous = NULL;
    for (; stmt; stmt = stmt->next) {
        if (stmt->type == ST_LABEL && !stmt->label->pclabel) {
            if (previous && previous->type == ST_LABEL) {
                stmt->label->pclabel = previous->label->pclabel;
            } else {
                stmt->label->pclabel = makepclabel();
            }
        }
        previous = stmt;
    }
}

Boolean CodeGen_IsPublicRuntimeObject(Object* obj) {
    return Intrinsics_IsPublicRuntimeObject(obj);
}

Boolean CodeGen_ParseAttribute(void) {
    CError_Internal("CodeGen.c", 5019);
    return FALSE;
}

#include "compiler/CPrep.h"
#include "compiler/CompilerTools.h"

VarInfo* CodeGen_GetNewVarInfo(void) {
    VarInfo* vi;
    if (CTemplTool_IsInTemplate()) {
        vi = galloc(0x38);
    } else {
        vi = lalloc(0x38);
    }
    memclrw(vi, 0x38);
    vi->deftoken = *CPrep_CurStreamElement();
    vi->varnumber = codegen_varnumber_637340++;
    return vi;
}

void ReleaseLicense(CWPluginContext context) {
    if (codegen_license_context_636D3C && codegen_license_active_600BA0) {
        CWCheckinLicense(context, codegen_license_context_636D3C);
    }
    codegen_license_active_600BA0 = FALSE;
    codegen_license_context_636D3C = 0;
}

__declspec(dllimport) UInt16 __stdcall GetSystemDefaultLangID(void);
void CodeGen_GetLicense(CWPluginContext context) {
    if (!codegen_license_context_636D3C) {
        if (GetSystemDefaultLangID() == 0x411 ||
            (CWCheckoutLicense(
                 context, "Win32_Plugins_MIPS_PS2", "1", 0, NULL, &codegen_license_context_636D3C
             ),
             !codegen_license_context_636D3C)) {
            CWCheckoutLicense(
                context, "Win32_Plugins_MIPS_PS2", "3", 0, NULL, &codegen_license_context_636D3C
            );
        }
        codegen_license_active_600BA0 = TRUE;
    }
}

void fn_00435C00(void) {
    codegen_register_init_flag_635AB0 = 1;
    init_registers();
}

void fn_00433EC0(HashNameNode* name, DeclInfo* declinfo) {
    SInt32 index = fn_0046F770(name->name);
    if (index) {
        declinfo->section = index;
    } else {
        CError_Error(77);
    }
}

#include "compiler/types.h"

void fn_00437D40(Object* obj, SInt16 reg) {
    VarInfo* info = Registers_GetVarInfo(obj);
    Type* type = obj->type;
    if (!reg || !info->used || info->noregister) {
        return;
    }
    if ((type->type == 1 ||
         type->type == 3 ||
         type->type == 11 ||
         (type->type == 10 && type->size == 4)) &&
        type->size <= 4) {
        retain_register(obj, 0, reg);
    } else {
        retain_register(obj, 1, reg);
    }
}

#pragma dont_inline on
SInt32 fn_004336B0(void) {
    fn_00443640(55);
    skipendofline();
    return FALSE;
}

SInt32 fn_004336C0(void) {
    fn_00443640(6);
    skipendofline();
    return FALSE;
}

#pragma dont_inline reset
void fn_00435BF0(void) {}

Object* fn_00435040(Object* obj) {
    Type* type;
    if (obj) {
        if (obj->otype == 0x1e || obj->otype == 0x32 || obj->otype == 0x34) {
            return obj;
        }
        type = obj->type;
        if (type->type == 1 && type->size == 16) {
            CError_Error(57);
        }
    }
    return obj;
}

void fn_00434FF0(Boolean useAlternate) {
    if (useAlternate) {
        peephole_opcode_636FCC = 0x3E5;
        peephole_opcode_637054 = 0x3E4;
        move_opcode = 0x3DF;
        codegen_mode_635B68 = 1;
    } else {
        peephole_opcode_636FCC = 0x48C;
        peephole_opcode_637054 = 0x48B;
        codegen_mode_635B68 = 0;
    }
}

void fn_00435080(void) {
    copts.globaloptimizer = copts.optimizationlevel >= 1;
    copts.peephole = copts.optimizationlevel >= 1;
    copts.schedule = copts.optimizationlevel >= 3;
    copts.tailcall = copts.optimizationlevel >= 3;
    copts.unknown36 = copts.optimizationlevel >= 3;
    copts.optimizewithasm = copts.optimizationlevel >= 1;
}

SInt32 fn_00433660(void) {
    SInt16 token;
    SInt32 reg;
    if (!notendofline()) {
        if (copts.warn_illpragma) {
            CPrep_Warning(87);
        }
        skipendofline();
        return 0;
    }
    token = plex();
    if (token == -1) {
        reg = codegen_reg_634814;
        if (reg < 0 || reg > 25) {
            return fn_004336B0();
        }
        copts.unknown48 = reg;
        init_registers();
        return 1;
    }
    return fn_004336C0();
}

SInt32 fn_004337A0(void) {
    UInt32 bits;
    SInt32 value;
    SInt32 check;

    tk = plex();
    if (tk == -7) {
        objgen_record_635B98 = 0;
        return 1;
    }
    if (tk == '(') {
        tk = plex();
        if (tk == -1) {
            value = codegen_reg_634814;
            objgen_record_635B98 = value;
            bits = 0x2000;
            check = value;
            for (; bits; bits >>= 1) {
                if (check == bits) {
                    tk = plex();
                    if (tk == ')') {
                        return 1;
                    }
                    break;
                }
            }
        }
    }
    objgen_record_635B98 = 0;
    return 0;
}

SInt32 fn_00433830(void) {
    UInt32 value;
    tk = plex();
    if (tk == -7) {
        cmach_zero_6363F4 = 0;
        return 1;
    }
    if (tk == '(') {
        tk = plex();
        if (tk == -1) {
            value = codegen_reg_634814;
            cmach_zero_6363F4 = value;
            if (value == 0 || value == 1 || value == 2 || value == 4 || value == 8 || value == 16) {
                tk = plex();
                if (tk == ')') {
                    return 1;
                }
            }
        }
    }
    cmach_zero_6363F4 = 0;
    return 0;
}

SInt32 fn_00433D30(void) {
    SInt32 value;
    tk = plex();
    if (tk == -7) {
        copts.unknown33 = 4;
        return 1;
    }
    if (tk == '(') {
        tk = plex();
        if (tk == -1) {
            value = codegen_reg_634814;
            if (value >= 1 && value <= 6) {
                copts.unknown33 = value;
                tk = plex();
                if (tk == ')') {
                    return 1;
                }
            }
        }
    }
    copts.unknown33 = 4;
    return 0;
}

SInt32 fn_00433DC0(void) {
    SInt32 index;
    const char* name;
    tk = plex();
    if (tk != -3) {
        return 0;
    }
    index = fn_0046F770(tkidentifier->name);
    tk = plex();
    if (tk != -3) {
        return 0;
    }
    name = tkidentifier->name;
    if (!__builtin_memcmp(name, "begin", 6)) {
        if (!index || objgen_context_6364B4) {
            return 0;
        }
        objgen_context_6364B4 = index;
    } else if (!__builtin_memcmp(name, "end", 4)) {
        if (!index || objgen_context_6364B4 != index) {
            return 0;
        }
        objgen_context_6364B4 = 0;
    } else {
        return 0;
    }
    do {
        tk = plex();
    } while (tk != -7 && tk != 0);
    return 1;
}

SInt32 fn_00433A30(void) {
    const char* name;
    tk = plex();
    if (tk != -3) {
        return 0;
    }
    name = tkidentifier->name;
    if (!__builtin_memcmp(name, "VR4100", 7)) {
        copts.unknown02 = 0x11;
    } else if (!__builtin_memcmp(name, "VR4111", 7)) {
        copts.unknown02 = 0x12;
    } else if (!__builtin_memcmp(name, "VR4121", 7)) {
        copts.unknown02 = 0x13;
    } else if (!__builtin_memcmp(name, "VR4200", 7)) {
        copts.unknown02 = 0x14;
    } else if (!__builtin_memcmp(name, "VR4300", 7)) {
        copts.unknown02 = 0x15;
    } else if (!__builtin_memcmp(name, "VR5000", 7)) {
        copts.unknown02 = 0x16;
    } else if (!__builtin_memcmp(name, "VR5400", 7)) {
        copts.unknown02 = 0x17;
    } else if (!__builtin_memcmp(name, "RM7000", 7)) {
        copts.unknown02 = 0x1b;
    } else if (!__builtin_memcmp(name, "R5900", 6)) {
        copts.unknown02 = 0x1d;
    } else if (!__builtin_memcmp(name, "PSP", 4)) {
        copts.unknown02 = 0x1e;
    } else if (!__builtin_memcmp(name, "IDT3081", 8)) {
        copts.unknown02 = 0x8;
    } else if (!__builtin_memcmp(name, "IDT4640", 8)) {
        copts.unknown02 = 0xc;
    } else if (!__builtin_memcmp(name, "IDT4650", 8)) {
        copts.unknown02 = 0xd;
    } else if (!__builtin_memcmp(name, "IDT4700", 8)) {
        copts.unknown02 = 0xb;
    } else if (!__builtin_memcmp(name, "IDT32364", 9)) {
        copts.unknown02 = 0xe;
    } else if (!__builtin_memcmp(name, "SONY4500", 9)) {
        copts.unknown02 = 0x3;
    } else if (!__builtin_memcmp(name, "TR4101", 7)) {
        copts.unknown02 = 0x1c;
    } else {
        return 0;
    }
    fn_004354E0();
    do {
        tk = plex();
    } while (tk != -7 && tk != 0);
    return 1;
}

void fn_00437300(SInt32 maximum, UInt32 weight) {
    PCodeBlock* block = pclastblock;
    pcloopweight = weight;
    if (block && !block->pcodeCount) {
        block->loopWeight = weight;
    }
    if (maximum > 0 && maximum > asm_source_offset_6361A0) {
        asm_source_offset_6361A0 = maximum;
    }
}

#pragma defer_codegen off
void fn_004379F0(Object* obj, SInt16 preset) {
    SInt32 align;
    SInt32 mask;
    SInt32 offset;
    Type* type;
    if (preset) {
        return;
    }
    align = CMach_GetTypeAlign(obj->type);
    codegen_stack_offset_600B9C = (align - 1 + codegen_stack_offset_600B9C) & ~(align - 1);
    offset = codegen_stack_offset_600B9C;
    obj->datatype = 1;
    ((VarInfo*)Registers_GetVarInfo(obj))->flags |= 1;
    obj->u.unk30 = offset;
    type = obj->type;
    mask = register_size - 1;
    codegen_stack_offset_600B9C += (type->size + mask) & ~mask;
}

#pragma opt_repositioncode off
SInt32 fn_004336D0(void) {
    SInt16 token;
    SInt32 reg;
    if (!notendofline()) {
        if (copts.warn_illpragma) {
            CPrep_Warning(87);
        }
        skipendofline();
        return 0;
    }
    token = plex();
    if (token == -1) {
        reg = codegen_reg_634814;
        if (reg < 0 || reg > 32) {
            goto badregister;
        }
        copts.vu0_register_number = reg;
        copts.enable_vu0_registers = TRUE;
        init_registers();
        return 1;
    } else if (token == -3) {
        if (!__builtin_memcmp(tkidentifier->name, "reset", 6)) {
            copts.vu0_register_number = 32;
            copts.enable_vu0_registers = TRUE;
            return 1;
        }
    }
    goto badtoken;
badregister:
    fn_00443640(55);
    skipendofline();
    return 0;
badtoken:
    fn_00443640(6);
    skipendofline();
    return 0;
}

#pragma opt_repositioncode on
#pragma defer_codegen on
void fn_00434490(SInt32 value, SInt32 reg) {
    UInt32 low;
    if (!value) {
        CError_Internal("CodeGen.c", 3964);
        return;
    }
    if (value >= -32768 && value < 32768) {
        emitpcode(3, reg, 0, value);
    } else if (value >= 0 && value < 65536) {
        emitpcode(0xBF, reg, 0, value);
    } else {
        low = value & 0xFFFF;
        if (low) {
            emitpcode(0x90, reg, (UInt16)(value >> 16));
            emitpcode(0xBF, reg, reg, low);
        } else {
            emitpcode(0x90, reg, (UInt16)(value >> 16));
        }
    }
}

void fn_00436FA0(register Object* function, SInt32 emit) {
    binary_word_count_destination = galloc(12);
    memclrw(binary_word_count_destination, 12);
    function->unk1C = binary_word_count_destination;
    dollar_name_objects = NULL;
    if (emit) {
        emitpcode(0x4A5);
        pclastblock->lastPCode->sourceoffset = functionbodyoffset;
    }
    fn_00437300(functionbodyoffset, 0);
}

#include "compiler/InstrSelection.h"

Operand fn_00438330(Operand* op) {
    Operand result = *op;
    result.immOffset += fn_0046FF50(op->object);
    if (result.immOffset < codegen_operand_max_6363EC) {
        result.immOffset = codegen_operand_max_6363EC;
    }
    return result;
}

void fn_00437DB0(void) {
    ObjectList* list;
    Object* object;
    VarInfo* info;
    if (codegen_register_scan_636020) {
        if (!(cg_debug_register_mask_636208 & 2)) {
            for (list = ig_reglist_a; list; list = list->next) {
                object = list->object;
                info = Registers_GetVarInfo(object);
                if (!info->reg && info->used && !info->noregister) {
                    is_volatile_object(object);
                }
            }
        }
        if (!(cg_debug_register_mask_636208 & 2)) {
            for (list = ig_reglist_b; list; list = list->next) {
                object = list->object;
                info = Registers_GetVarInfo(object);
                if (!info->reg && info->used && !info->noregister) {
                    is_volatile_object(object);
                }
            }
        }
    }
}

void fn_00437A60(register Object* obj, SInt16 enabled) {
    VarInfo* info;
    ObjectList* list;
    SInt32 align;
    SInt32 offset;
    Type* type;
    if (!enabled) {
        return;
    }
    if (!cg_pcode_uses_registers_637392 && optimizing) {
        info = Registers_GetVarInfo(obj);
        if (is_volatile_object(obj)) {
            goto list;
        }
        type = obj->type;
        if (!type) {
            goto checkInfo;
        }
        if (type->type == 12 ||
            type->type == 4 ||
            type->type == 5 ||
            (type->type == 10 && type->size == 12)) {
            goto list;
        }
        goto checkInfo;
    }
    align = CMach_GetTypeAlign(obj->type);
    codegen_operand_max_6363EC = (align - 1 + codegen_operand_max_6363EC) & ~(align - 1);
    offset = codegen_operand_max_6363EC;
    obj->datatype = 1;
    ((VarInfo*)Registers_GetVarInfo(obj))->flags |= 1;
    obj->u.unk30 = offset;
    type = obj->type;
    codegen_operand_max_6363EC += (type->size + 15) & ~15;
    return;
checkInfo:
    if (!info->noregister) {
        return;
    }
list:
    list = lalloc(8);
    memclrw(list, 8);
    list->object = obj;
    list->next = codegen_allocated_objects_600B90;
    codegen_allocated_objects_600B90 = list;
}

UInt8 fn_00436C70(Type* type) {
    UInt8 integral;
    if (!type) {
        return 0;
    }
    if (type->type == 2) {
        integral = TYPE_INTEGRAL(type)->integral;
        if ((integral == 13 || (type->type == 2 && type->size == 4)) && (copts.fp_library & 1)) {
            return 4;
        }
        if ((integral == 15 || (type->type == 2 && type->size == 8)) && (copts.fp_library & 2)) {
            return 4;
        }
    }
    if (type->type == 10 && type->size == 4) {
        if (fn_00470390(type)) {
            return 1;
        }
        return 2;
    }
    if (copts.unknown15 && (UInt8)(type->type - 4) <= 1) {
        if (fn_00470390(type)) {
            return 1;
        }
        if (fn_0046FF70(type)) {
            return 4;
        }
        if (fn_00470030(type)) {
            return 2;
        }
    }
    if (type->type == 5) {
        return 1;
    }
    if (type &&
        (type->type == 12 ||
         type->type == 4 ||
         type->type == 5 ||
         (type->type == 10 && type->size == 12))) {
        return 1;
    }
    return 2;
}

Object* fn_00438B60(Object* original) {
    SInt32 bytes = 0;
    register char* name;
    char* originalName;
    HashNameNode* linkname;
    ObjectList* list;
    Object* object;
    TypePointer* type;
    ObjectList* newList;
    ObjGenRelocation* fixup;

    linkname = CMangler_GetLinkName(original);
    originalName = linkname->name;
    name = lalloc(__builtin_strlen(originalName) + 5);
    sprintf(name, "@dp@%s", originalName);
    for (list = objgen_state_6355C4; list; list = list->next) {
        object = list->object;
        if (strcmp(CMangler_GetLinkName(object)->name, name) == 0) {
            return object;
        }
    }
    linkname = CMangler_GetLinkName(original);
    originalName = linkname->name;
    name = galloc(__builtin_strlen(originalName) + 5);
    linkname = CMangler_GetLinkName(original);
    sprintf(name, "@dp@%s", linkname->name);

    type = galloc(16);
    memclrw(type, 16);
    type->type = TYPEPOINTER;
    type->size = 4;
    type->target = (Type*)&stsignedint;

    object = galloc(0x44);
    memclrw(object, 0x44);
    object->section = fn_0046F770("sdata");
    object->type = (Type*)type;
    object->name = GetHashNameNodeExport(name);
    object->sclass = 0x103;
    object->datatype = 0;
    object->qual = original->qual | 0x20000;

    fixup = galloc(16);
    memclrw(fixup, 16);
    fixup->object = original;
    fixup->offset = 0;
    fn_0046DD50(object, &bytes, fixup, type->size);

    newList = galloc(8);
    memclrw(newList, 8);
    newList->next = objgen_state_6355C4;
    objgen_state_6355C4 = newList;
    newList->object = object;
    return object;
}

void fn_00436DB0(Object* obj) {
    PCodeBlock* block;
    PCode* instr;
    codegen_deferred_list_first_600B88 = 0;
    codegen_deferred_list_last_600B8C = codegen_deferred_list_first_600B88;
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (instr = block->firstPCode; instr; instr = instr->nextPCode) {
            if (instr->unk38 != 6 && instr->unk38 != 0) {
                fn_00436E50(
                    obj,
                    instr,
                    &codegen_deferred_list_last_600B8C,
                    (struct CodeGenEmitRecord**)&codegen_deferred_list_first_600B88
                );
            }
        }
    }
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (instr = block->firstPCode; instr; instr = instr->nextPCode) {
            if (instr->unk38 == 6) {
                fn_00436E50(
                    obj,
                    instr,
                    &codegen_deferred_list_last_600B8C,
                    (struct CodeGenEmitRecord**)&codegen_deferred_list_first_600B88
                );
            }
        }
    }
}

void fn_00436E50(
    Object* object, PCode* instr, struct CodeGenEmitRecord** head, struct CodeGenEmitRecord** tail
) {
    struct CodeGenEmitRecord* node;
    struct CodeGenEmitRecord* record;
    if (!*(Object* volatile*)&instr->object) {
        goto no_reference;
    }
    if (instr->unk38 == 6) {
        for (node = *head; node; node = node->next) {
            if (node->kind == 5 && node->key == instr->object) {
                record = lalloc(20);
                record->kind = instr->unk38;
                record->value = instr->unk1C;
                record->reserved = 0;
                record->key = instr->object;
                record->next = node->next;
                node->next = record;
                return;
            }
        }
        CError_Internal("CodeGen.c", 2074);
    }
main:
    record = lalloc(20);
    if (*tail) {
        (*tail)->next = record;
    } else {
        *head = record;
    }
    record->next = 0;
    *tail = record;
    record->kind = instr->unk38;
    record->value = instr->unk1C;
    record->reserved = 0;
    record->key = instr->object;
    if (instr->unk38 == 5) {
        goto append_second;
    }
done:
    return;
no_reference:
    if (instr->unk38 == 4) {
        instr->object = object;
        instr->alias = worst_case_ptr;
    } else {
        CError_Internal("CodeGen.c", 2054);
    }
    goto main;
append_second:
    {
        record = lalloc(20);
        node = *tail;
        node->next = record;
        record->next = 0;
        *tail = record;
        record->kind = instr->unk3C->unk38;
        record->value = instr->unk3C->unk1C;
        record->reserved = 0;
        record->key = instr->object;
        instr->unk3C->unk38 = 0;
    }
    goto done;
}

#include "compiler/InstrSelection.h"

Operand fn_004383F0(register Operand* op) {
    if (op->kind != 3) {
        CError_Internal("CodeGen.c", 897);
    }
    if (op->immediate.lo == 2) {
        return make_constant_operand(codegen_register_6361E8 - 4);
    }
    if (op->immediate.lo == 3) {
        return make_constant_operand(codegen_register_6361E4 - 12);
    }
    {
        Operand fallback = {0};
        CError_Internal("CodeGen.c", 908);
        return fallback;
    }
}

void makeregaliasformem(Object* original, UInt8 cls, SInt32 reg) {
    Object* object;
    Type* type = original->type;
    VarInfo* info;
    ObjectList* entry;
    char* name;
    object = lalloc(0x44);
    memclrw(object, 0x44);
    name = lalloc(__builtin_strlen(original->name->name) + 22);
    sprintf(name, "%s'%d", original->name->name, reg);
    object->otype = OT_OBJECT;
    object->access = 0;
    object->datatype = 1;
    object->type = type;
    object->name = GetHashNameNodeExport(name);
    info = CodeGen_GetNewVarInfo();
    object->u.var.info = info;
    object->u.var.info->func = codegen_current_function_636498;
    object->u.var.info->usage = 1;
    object->u.var.info->used = 1;
    object->u.var.info->flags |= 2;
    object->u.var.info->rclass = cls;
    object->u.var.info->reg = reg;
    entry = lalloc(8);
    memclrw(entry, 8);
    entry->next = ig_reglist_b;
    entry->object = object;
    ig_reglist_b = entry;
}

SInt32 fn_004338D0(void) {
    char* first;
    char* second;
    char* third;
    char* fourth;
    char* fifth;
    first = 0;
    third = 0;
    fourth = 0;
    fifth = 0;
    tk = plex();
    if (tk != -3) {
        return 0;
    }
    first = tkidentifier->name;
    tk = plex();
    if (tk != -4) {
        return 0;
    }
    second = tkstring;
    tk = plex();
    if (tk == -4) {
        third = tkstring;
        tk = plex();
    }
    if (tk == -3) {
        fourth = tkidentifier->name;
        tk = plex();
    }
    if (tk == -3) {
        fifth = tkidentifier->name;
        tk = plex();
    }
    if (!fn_0046F3F0(first, second, third, fourth, fifth)) {
        return 0;
    }
    if (tk == -7 || tk == 0) {
        return 1;
    }
    do {
        tk = plex();
    } while (tk != -7 && tk != 0);
    return 0;
}

void fn_00433340(HashNameNode* pragma) {
    SInt16 token;
    UInt32 reg;
    SInt32 i;
    SInt32 handled;
    if (!__builtin_memcmp(pragma->name, "pack", 5)) {
        if (fn_00433830()) {
            return;
        }
    } else if (!__builtin_memcmp(pragma->name, "cgswitch", 9)) {
        tk = plex();
        if (tk == -7) {
            for (i = 0; i < 16; i++) {
                codegen_reserved_registers_622580[i] = 0;
            }
            handled = TRUE;
        } else if (tk == '(') {
            tk = plex();
            if (tk == -1 && (UInt32)codegen_reg_634814 < 16) {
                codegen_reserved_registers_622580[codegen_reg_634814] = 1;
                handled = TRUE;
            } else {
                handled = FALSE;
            }
        } else {
            handled = FALSE;
        }
        if (handled) {
            return;
        }
    } else if (!__builtin_memcmp(pragma->name, "align", 6)) {
        if (fn_004337A0()) {
            return;
        }
    } else if (!__builtin_memcmp(pragma->name, "processor", 10)) {
        if (fn_00433A30()) {
            return;
        }
    } else if (!__builtin_memcmp(pragma->name, "maxtempforblkmv", 16)) {
        if (fn_00433D30()) {
            return;
        }
    } else if (!__builtin_memcmp(pragma->name, "vu0_register_number", 20)) {
        if (fn_004336D0()) {
            return;
        }
    } else if (!__builtin_memcmp(pragma->name, "vu0_mmi_reg_binding", 20)) {
        if (!notendofline()) {
            return;
        }
        token = plex();
        if (token != -3) {
            return;
        }
        if (!__builtin_memcmp(tkidentifier->name, "on", 3)) {
            copts.unknown47 = 1;
        } else if (!__builtin_memcmp(tkidentifier->name, "off", 4)) {
            copts.unknown47 = 0;
        }
        return;
    } else if (!__builtin_memcmp(pragma->name, "vu0_max_used_nvreg", 19)) {
        if (fn_00433660()) {
            return;
        }
    } else if (!__builtin_memcmp(pragma->name, "section", 8)) {
        if (fn_00433DC0()) {
            return;
        }
    } else if (!__builtin_memcmp(pragma->name, "define_section", 15)) {
        if (fn_004338D0()) {
            return;
        }
    }
    if (copts.warn_illpragma) {
        CPrep_Warning(87);
    }
    do {
        token = plex();
    } while (token != -7 && token != 0);
}

void fn_00437E50(void) {
    ObjectList* list;
    Object* candidate;
    Object* selected;
    VarInfo* info;
    Type* type;
    SInt32 usage;
    SInt32 highest;
    UInt8 kind;
    UInt8 integral;
    while (assignable_registers[0]) {
        selected = NULL;
        highest = -1;
        if (!(cg_debug_register_mask_636208 & 2)) {
            for (list = ig_reglist_b; list; list = list->next) {
                candidate = list->object;
                info = Registers_GetVarInfo(candidate);
                type = candidate->type;
                if (info->reg || !info->used || info->noregister || is_volatile_object(candidate)) {
                    continue;
                }
                usage = info->usage;
                if (usage < highest || usage < 1) {
                    continue;
                }
                kind = candidate->type->type;
                if (kind != 1 && kind != 3 && kind != 11 && !(kind == 10 && type->size == 4)) {
                    continue;
                }
                if (kind == 2) {
                    integral = TYPE_INTEGRAL(type)->integral;
                    if ((integral == 13 || (kind == 2 && type->size == 4)) &&
                        !(copts.fp_library & 1)) {
                        continue;
                    }
                    if ((integral == 15 || (kind == 2 && type->size == 8)) &&
                        !(copts.fp_library & 2)) {
                        continue;
                    }
                }
                selected = candidate;
                highest = usage;
            }
        }
        if (!selected) {
            return;
        }
        assign_register_to_variable(selected, 0);
        info = Registers_GetVarInfo(selected);
        binary_word_count_destination->gprmask |= 1U << info->reg;
    }
}

Object* fn_00436790(void) {
    char fileInfo[0x100];
    char* name;
    char* companionName;
    TypeFunc* functionType;
    Object* function;
    Object* companion;
    OLinkList* fixup;
    SInt32 bytes;
    COS_FileGetFileSpecInfo(&cparamblkptr->sourcefile, 0, 0x104, fileInfo, 0x100);
    name = lalloc((UInt8)fileInfo[0] + 10);
    sprintf(name, "__sinit_%*.*s", -(UInt8)fileInfo[0], (UInt8)fileInfo[0], fileInfo);
    functionType = galloc(sizeof(TypeFunc));
    memclrw(functionType, sizeof(TypeFunc));
    functionType->type = TYPEFUNC;
    functionType->functype = &stvoid;
    functionType->args = NULL;
    functionType->flags = FUNC_DEFINED;
    function = galloc(sizeof(Object));
    memclrw(function, sizeof(Object));
    function->type = (Type*)functionType;
    function->name = GetHashNameNodeExport(name);
    function->sclass = 0x102;
    function->datatype = DFUNC;
    function->section = objgen_init_635F44;
    companion = galloc(sizeof(Object));
    memclrw(companion, sizeof(Object));
    companionName = lalloc(__builtin_strlen(name) + 4);
    sprintf(companionName, ".p%s", name);
    companion->type = CDecl_NewPointerType((Type*)functionType);
    companion->name = GetHashNameNodeExport(companionName);
    companion->sclass = 0x102;
    companion->datatype = DLOCAL;
    companion->section = objgen_ctor_635FE8;
    fixup = galloc(sizeof(OLinkList));
    memclrw(fixup, sizeof(OLinkList));
    fixup->obj = function;
    bytes = 0;
    fn_0046DD50(companion, &bytes, fixup, companion->type->size);
    return function;
}

void fn_004384D0(register Operand* result) {
    Operand immediate;
    PCode* instruction;
    UInt8 threshold;
    immediate = make_constant_operand(codegen_register_6361E8 - 4);
    fn_00486660(0x47f, &immediate, result);
    result->immOffset++;
    immediate = make_constant_operand(codegen_register_6361E4 - 12);
    fn_00486660(0x47f, &immediate, result);
    result->immOffset++;
    result->immOffset += 2;
    instruction = make_instruction(0x44e, 3);
    instruction->args[0].kind = PCOp_REGISTER;
    instruction->args[0].arg = 0;
    instruction->args[0].effect = 2;
    instruction->args[0].data.reg.reg = used_virtual_registers[RegClass_GPR];
    used_virtual_registers[RegClass_GPR]++;
    instruction->args[1].kind = PCOp_MEMORY;
    instruction->args[1].data.mem.obj = galloc(sizeof(Object));
    memclrw(instruction->args[1].data.mem.obj, sizeof(Object));
    instruction->args[1].data.mem.obj->datatype = DLOCAL;
    instruction->args[1].data.mem.offset = codegen_operand_max_6363EC;
    instruction->args[2].kind = PCOp_REGISTER;
    instruction->args[2].arg = 0;
    instruction->args[2].effect = 1;
    instruction->args[2].data.reg.reg = 29;
    appendpcode(pclastblock, instruction);
    codegen_result_636784 = operand_from_pcode_arg(pclastblock->lastPCode->args[0]);
    fn_00486660(0x481, &codegen_result_636784, result);
    result->immOffset += 4;
    threshold = copts.unknown15 ? 19 : 15;
    if (codegen_register_6361E4 < threshold) {
        fn_00486660(0x481, &codegen_operand_635F6C, result);
    }
    result->immOffset += 4;
    threshold = copts.unknown15 ? 11 : 7;
    if (codegen_register_6361E8 < threshold) {
        fn_00486660(0x481, &codegen_operand_636958, result);
    }
}

void fn_00438170(Object* obj, SInt16 preserve) {
    VarInfo* info = obj->u.var.info;
    Type* type = obj->type;
    UInt8 kind;
    UInt8 integral;
    if (!info->used) {
        return;
    }
    if (info->noregister || is_volatile_object(obj)) {
        codegen_flag_637383 = TRUE;
        return;
    }
    kind = type->type;
    if (kind != 1 &&
        kind != 3 &&
        kind != 11 &&
        !(kind == 10 && type->size == 4) &&
        kind != 2 &&
        !fn_0046FF70(type)) {
        codegen_flag_637383 = TRUE;
        return;
    }
    if (!preserve && info->usage < 4) {
        info->noregister = 1;
        codegen_flag_637383 = TRUE;
        return;
    }
    kind = type->type;
    if (kind == 1 || kind == 3 || kind == 11 || (kind == 10 && type->size == 4)) {
        goto gpr;
    }
    if (kind == 2) {
        integral = TYPE_INTEGRAL(type)->integral;
        if ((integral == 13 || (kind == 2 && type->size == 4)) && !(copts.fp_library & 1)) {
            goto gpr;
        }
        if ((integral == 15 || (kind == 2 && type->size == 8)) && !(copts.fp_library & 2)) {
            goto gpr;
        }
        info->rclass = 1;
        info->reg = used_virtual_registers[RegClass_FPR]++;
        return;
    }
    if (!fn_0046FF70(type)) {
        return;
    }
    info->rclass = 1;
    info->reg = used_virtual_registers[RegClass_FPR]++;
    return;
gpr:
    info->rclass = 0;
    info->reg = used_virtual_registers[RegClass_GPR]++;
}

void fn_00438670(void (*callback)(Object*, SInt16, SInt32), SInt32 emit) {
    ObjectList* list;
    Object* object;
    Type* type;
    PCode* instruction;
    int gpr = 4;
    int slot = 12;
    UInt8 maximum;
    PCodeArg arg;
    for (list = ig_reglist_a; list; list = list->next) {
        object = list->object;
        type = object->type;
        if (type->type == 2) {
            UInt8 integral = TYPE_INTEGRAL(type)->integral;
            if ((integral == 13 || (type->type == 2 && type->size == 4)) &&
                (copts.fp_library & 1)) {
                goto floating;
            }
            if ((integral == 15 || (type->type == 2 && type->size == 8)) &&
                (copts.fp_library & 2)) {
                goto floating;
            }
        }
        goto general;
floating:
        if (copts.unknown15) {
            maximum = 19;
        } else {
            maximum = 15;
        }
        callback(object, (SInt16)slot <= maximum ? (SInt16)slot : 0, 0);
        slot++;
next:
        continue;
    }
    codegen_register_6361E8 = (SInt16)gpr;
    codegen_register_6361E4 = (SInt16)slot;
    if (!emit) {
        goto done;
    }
    codegen_flag_637383 = TRUE;
    for (;;) {
        if (copts.unknown15) {
            maximum = 11;
        } else {
            maximum = 7;
        }
        if ((SInt16)gpr > maximum) {
            goto done;
        }
        instruction = make_instruction(0x479, 3);
        instruction->args[0].kind = PCOp_REGISTER;
        instruction->args[0].data.reg.reg = gpr;
        instruction->args[0].arg = 0;
        instruction->args[0].effect = 1;
        instruction->args[1].kind = PCOp_MEMORY;
        instruction->args[1].data.mem.offset = codegen_operand_max_6363EC;
        instruction->args[1].data.mem.obj = galloc(sizeof(Object));
        __builtin_memset(instruction->args[1].data.mem.obj, 0, sizeof(Object));
        instruction->args[1].data.mem.obj->sclass = 0x102;
        instruction->args[1].data.mem.obj->datatype = DLOCAL;
        arg = fn_004D02D0(29, 0, 1);
        instruction->args[2] = arg;
        appendpcode(pclastblock, instruction);
        codegen_operand_max_6363EC += register_size;
        gpr++;
    }
    goto done;
general:
    if (fn_0046FF70(type)) {
        goto floating;
    }
    if (copts.unknown15) {
        maximum = 11;
    } else {
        maximum = 7;
    }
    callback(object, (SInt16)gpr <= maximum ? (SInt16)gpr : 0, 1);
    gpr++;
    goto next;
done:
    return;
}

void fn_00438890(void (*callback)(Object*, SInt16, SInt32), SInt32 emit) {
    ObjectList* list;
    Object* object;
    Type* type;
    PCode* instruction;
    int gpr = 4;
    int slot = 12;
    UInt8 maximum;
    PCodeArg arg;
    for (list = ig_reglist_a; list; list = list->next) {
        object = list->object;
        type = object->type;
        if (type->type != TYPEFLOAT) {
            goto aggregate;
        }
        if (type->type == TYPEFLOAT) {
            if ((TYPE_INTEGRAL(type)->integral == IT_FLOAT ||
                 (type->type == 2 && type->size == 4)) &&
                (copts.fp_library & 1)) {
                goto floating;
            }
            if ((TYPE_INTEGRAL(type)->integral == IT_DOUBLE ||
                 (type->type == 2 && type->size == 8)) &&
                (copts.fp_library & 2)) {
                goto floating;
            }
        }
        goto classify;
floating:
        if (copts.unknown15) {
            maximum = 19;
        } else {
            maximum = 15;
        }
        callback(object, (SInt16)slot <= maximum ? (SInt16)slot : 0, 0);
        slot++;
next:
        continue;
    }
    codegen_register_6361E8 = (SInt16)gpr;
    codegen_register_6361E4 = (SInt16)slot;
    if (!emit) {
        goto done;
    }
    codegen_flag_637383 = TRUE;
    for (;;) {
        if (copts.unknown15) {
            maximum = 11;
        } else {
            maximum = 7;
        }
        if ((SInt16)gpr > maximum) {
            goto done;
        }
        instruction = make_instruction(0x479, 3);
        instruction->args[0].kind = PCOp_REGISTER;
        instruction->args[0].data.reg.reg = gpr;
        instruction->args[0].arg = 0;
        instruction->args[0].effect = 1;
        instruction->args[1].kind = PCOp_MEMORY;
        instruction->args[1].data.mem.offset = codegen_operand_max_6363EC;
        instruction->args[1].data.mem.obj = galloc(sizeof(Object));
        __builtin_memset(instruction->args[1].data.mem.obj, 0, sizeof(Object));
        instruction->args[1].data.mem.obj->sclass = 0x102;
        instruction->args[1].data.mem.obj->datatype = DLOCAL;
        arg = fn_004D02D0(29, 0, 1);
        instruction->args[2] = arg;
        appendpcode(pclastblock, instruction);
        codegen_operand_max_6363EC += register_size;
        gpr++;
    }
    goto done;
classify:
    if (fn_0046FF70(type)) {
        goto floating;
    }
    if (type->type == TYPEFLOAT) {
        if ((TYPE_INTEGRAL(type)->integral == IT_FLOAT ||
             (type->type == TYPEFLOAT && type->size == 4)) &&
            !(copts.fp_library & 1)) {
            goto general;
        }
        if ((TYPE_INTEGRAL(type)->integral == IT_DOUBLE ||
             (type->type == TYPEFLOAT && type->size == 8)) &&
            !(copts.fp_library & 2)) {
            goto general;
        }
    }
    CError_Internal("CodeGen.c", 591);
    goto next;
general:
    if (copts.unknown15) {
        maximum = 11;
    } else {
        maximum = 7;
    }
    callback(object, (SInt16)gpr <= maximum ? (SInt16)gpr : 0, 1);
    gpr++;
    goto next;
aggregate:
    CMach_GetTypeAlign(type);
    if (copts.unknown15) {
        maximum = 11;
    } else {
        maximum = 7;
    }
    callback(object, (SInt16)gpr <= maximum ? (SInt16)gpr : 0, 1);
    gpr++;
    goto next;
done:
    return;
}

void fn_00433EF0(Object* obj) {
    RegInfo* list;
    ObjRegInfo* info;
    ObjRegInfo* other;
    if (copts.unknown47 && fn_004727A0(obj)) {
        list = reg_info_list;
        info = obj->u.data.reginfo;
        for (; list; list = list->next) {
            other = list->reginfo;
            if (info->rclass == other->rclass) {
                if (info->reg == other->reg) {
                    list->object = obj;
                }
                break;
            }
        }
    }
    if (obj->type && obj->type->type == 6) {
        if (obj->name && !__builtin_memcmp(obj->name->name, codegen_builtin_next_arg_name, 19)) {
            obj->u.intrinsicid = 7;
        }
        return;
    }
    if (!obj->section) {
        obj->section = objgen_context_6364B4;
    }
    if (objgen_record_635B98 && !(obj->qual & 0x1e000000)) {
        switch (objgen_record_635B98) {
            case 1:
                obj->qual |= 0x02000000;
                break;
            case 2:
                obj->qual |= 0x04000000;
                break;
            case 4:
                obj->qual |= 0x06000000;
                break;
            case 8:
                obj->qual |= 0x08000000;
                break;
            case 16:
                obj->qual |= 0x0a000000;
                break;
            case 32:
                obj->qual |= 0x0c000000;
                break;
            case 64:
                obj->qual |= 0x10000000;
                break;
            case 128:
                obj->qual |= 0x12000000;
                break;
            case 256:
                obj->qual |= 0x14000000;
                break;
            case 512:
                obj->qual |= 0x16000000;
                break;
            case 1024:
                obj->qual |= 0x18000000;
                break;
            case 2048:
                obj->qual |= 0x1a000000;
                break;
            case 4096:
                obj->qual |= 0x1c000000;
                break;
            case 8192:
                obj->qual |= 0x1e000000;
                break;
            default:
                CError_Internal("CodeGen.c", 0x1062);
                break;
        }
    }
}

void fn_00437B50(void) {
    SInt32 mask;
    ObjectList* list;
    Object* object;
    VarInfo* info;
    SInt32 reg;
    if (codegen_register_option_637385 && !optimizing && !copts.unknown15) {
        fn_00438890((void (*)(Object*, SInt16, SInt32))fn_00437D40, 0);
    }
    if (optimizing) {
        fn_00437FA0();
    } else {
        fn_00437E50();
        if ((SInt32)(*(volatile SInt8*)&copts.fp_library) != 0) {
            fn_00437DB0();
        }
    }
    be_macro_stack_base_635A40 = 0;
    if (ig_reglist_b) {
        mask = 1;
        do {
            for (list = ig_reglist_b; list; list = list->next) {
                object = list->object;
                reg = object->datatype == DLOCAL ? object->u.var.info->reg : 0;
                if (!reg) {
                    info = Registers_GetVarInfo(object);
                    if (info->used && CMach_GetObjectAlign(object) == mask) {
                        assign_local_memory(object);
                    }
                }
            }
            mask *= 2;
        } while (mask <= 0x2000);
    }
    if (be_macro_stack_base_635A40 > 0x7fff) {
        be_macro_stack_base_635A40 = 0;
        for (list = ig_reglist_b; list; list = list->next) {
            object = list->object;
            reg = object->datatype == DLOCAL ? object->u.var.info->reg : 0;
            if (!reg) {
                info = Registers_GetVarInfo(object);
                if (info->used && object->type->size > 8) {
                    assign_local_memory(object);
                }
            }
        }
        mask = 1;
        do {
            for (list = ig_reglist_b; list; list = list->next) {
                object = list->object;
                reg = object->datatype == DLOCAL ? object->u.var.info->reg : 0;
                if (!reg) {
                    info = Registers_GetVarInfo(object);
                    if (info->used &&
                        object->type->size <= 8 &&
                        CMach_GetObjectAlign(object) == mask) {
                        assign_local_memory(object);
                    }
                }
            }
            mask *= 2;
        } while (mask <= 0x2000);
    }
    while (codegen_allocated_objects_600B90) {
        assign_local_memory(codegen_allocated_objects_600B90->object);
        codegen_allocated_objects_600B90 = codegen_allocated_objects_600B90->next;
    }
}

void fn_00437FA0(void) {
    ObjectList* list;
    Object* object;
    VarInfo* info;
    Type* type;
    UInt8 kind;
    UInt8 integral;
    if (copts.unknown15) {
        fn_00438670((void (*)(Object*, SInt16, SInt32))fn_00438170, 0);
    } else {
        fn_00438890((void (*)(Object*, SInt16, SInt32))fn_00438170, 0);
    }
    for (list = ig_reglist_b; list; list = list->next) {
        object = list->object;
        info = Registers_GetVarInfo(object);
        type = object->type;
        if (!info->used) {
            continue;
        }
        if (info->noregister) {
            codegen_flag_637383 = TRUE;
            continue;
        }
        if (is_volatile_object(object)) {
            codegen_flag_637383 = TRUE;
            continue;
        }
        if (info->unknown2D &&
            (type->type == TYPEINT || type->type == TYPEENUM) &&
            type->size == 16) {
            info->rclass = 5;
            info->reg = used_virtual_registers[5]++;
            continue;
        }
        kind = type->type;
        if (kind != 1 && kind != 3 && kind != 11 && !(kind == 10 && type->size == 4) && kind != 2) {
            codegen_flag_637383 = TRUE;
            continue;
        }
        if (kind == 1 || kind == 3 || kind == 11 || (kind == 10 && type->size == 4)) {
            goto gpr;
        }
        if (kind == 2) {
            integral = TYPE_INTEGRAL(type)->integral;
            if ((integral == 13 || (kind == 2 && type->size == 4)) && !(copts.fp_library & 1)) {
                goto gpr;
            }
            if ((integral == 15 || (kind == 2 && type->size == 8)) && !(copts.fp_library & 2)) {
                goto gpr;
            }
            goto fpr;
        }
        codegen_flag_637383 = TRUE;
        continue;
gpr:
        info->rclass = 0;
        info->reg = used_virtual_registers[RegClass_GPR]++;
        continue;
fpr:
        info->rclass = 1;
        info->reg = used_virtual_registers[RegClass_FPR]++;
    }
}

void fn_00436920(Object* object) {
    ObjectList* list;
    UInt8 mode;
    int index;
    ObjRegInfo* info;
    RegInfo* reg;
    codegen_live_register_count_600740 = 0;
    mode = fn_00436C70(TYPE_FUNC(object->type)->functype);
    switch (mode) {
        case 2:
            index = codegen_live_register_count_600740;
            if (index > 128) {
                CError_Internal("CodeGen.c", 0x8ff);
            }
            codegen_live_registers_600744[index].cls = 0;
            codegen_live_registers_600744[index].reg = 2;
            codegen_live_register_count_600740 = index + 1;
            break;
        case 3:
            index = codegen_live_register_count_600740;
            if (index > 128) {
                CError_Internal("CodeGen.c", 0x8ff);
            }
            codegen_live_registers_600744[index].cls = 0;
            codegen_live_registers_600744[index].reg = 2;
            codegen_live_register_count_600740 = index + 1;
            index = codegen_live_register_count_600740;
            if (index > 128) {
                CError_Internal("CodeGen.c", 0x8ff);
            }
            codegen_live_registers_600744[index].cls = 0;
            codegen_live_registers_600744[index].reg = 3;
            codegen_live_register_count_600740 = index + 1;
            break;
        case 4:
            index = codegen_live_register_count_600740;
            if (index > 128) {
                CError_Internal("CodeGen.c", 0x8ff);
            }
            codegen_live_registers_600744[index].cls = 1;
            codegen_live_registers_600744[index].reg = 0;
            codegen_live_register_count_600740 = index + 1;
            break;
        case 5:
            index = codegen_live_register_count_600740;
            if (index > 128) {
                CError_Internal("CodeGen.c", 0x8ff);
            }
            codegen_live_registers_600744[index].cls = 1;
            codegen_live_registers_600744[index].reg = 0;
            codegen_live_register_count_600740 = index + 1;
            index = codegen_live_register_count_600740;
            if (index > 128) {
                CError_Internal("CodeGen.c", 0x8ff);
            }
            codegen_live_registers_600744[index].cls = 1;
            codegen_live_registers_600744[index].reg = 1;
            codegen_live_register_count_600740 = index + 1;
            break;
        case 6:
            index = codegen_live_register_count_600740;
            if (index > 128) {
                CError_Internal("CodeGen.c", 0x8ff);
            }
            codegen_live_registers_600744[index].cls = 1;
            codegen_live_registers_600744[index].reg = 0;
            codegen_live_register_count_600740 = index + 1;
            index = codegen_live_register_count_600740;
            if (index > 128) {
                CError_Internal("CodeGen.c", 0x8ff);
            }
            codegen_live_registers_600744[index].cls = 1;
            codegen_live_registers_600744[index].reg = 2;
            codegen_live_register_count_600740 = index + 1;
            break;
        case 0:
        case 1:
            break;
        default:
            CError_Internal("CodeGen.c", 0x93d);
            break;
    }
    index = codegen_live_register_count_600740;
    if (index > 128) {
        CError_Internal("CodeGen.c", 0x8ff);
    }
    codegen_live_registers_600744[index].cls = 0;
    codegen_live_registers_600744[index].reg = 29;
    codegen_live_register_count_600740 = index + 1;
    for (reg = reg_info_list; reg; reg = reg->next) {
        if (reg->object) {
            info = reg->reginfo;
            index = codegen_live_register_count_600740;
            if (index > 128) {
                CError_Internal("CodeGen.c", 0x8ff);
            }
            codegen_live_registers_600744[index].cls = info->rclass;
            codegen_live_registers_600744[index].reg = info->reg;
            codegen_live_register_count_600740 = index + 1;
        }
    }
    initialize_live_registers(&codegen_live_register_count_600740);
}

#include "compiler/InstrSelection.h"

void fn_00437030(ENode* node, Boolean bypass) {
    Type* type;
    UInt8 value_class;
    Operand operand;
    ObjectList* list;
    UInt16 opcode;
    SInt32 is_aggregate;
    Type* coerce_type;

    if (node != NULL) {
        codegen_active_type_600B98 = node->rtype;
        type = node->rtype;
        value_class = fn_00436C70(type);
        gen_func_table[node->type](node, 0, &operand);
        if (copts.unknown01) {
            call_runtime_function(rt_profile_e);
        }
        if (value_class == 4) {
            if (operand.kind) {
                is_aggregate = TRUE;
                if (type->type != TYPESTRUCT && type->type != TYPECLASS) {
                    is_aggregate = FALSE;
                }
                coerce_type = is_aggregate ? (Type*)fn_00470030(type) : type;
                Coerce_to_register(&operand, coerce_type, 0);
            }
            opcode = TYPE_INTEGRAL(type)->integral == IT_FLOAT ||
                    (type->type == TYPEFLOAT && type->size == 4)
                ? 0x4A3
                : 0x4A4;
            emit_paired_registers(operand, opcode, 1, 0);
        } else if (value_class == 5) {
            if (operand.kind) {
                Coerce_to_register(&operand, type, 0);
            }
            opcode = TYPE_INTEGRAL(type)->integral == IT_FLOAT ||
                    (type->type == TYPEFLOAT && type->size == 4)
                ? 0x4A3
                : 0x4A4;
            emit_paired_registers(operand, opcode, 1, 0);
        } else if (value_class == 2 || value_class == 3) {
            if (operand.kind) {
                Coerce_to_register(&operand, type, 0);
            }
            opcode = ((type->type == TYPEINT || type->type == TYPEENUM) && type->size == 16)
                ? move_opcode_637350
                : move_opcode;
            emit_paired_registers(operand, opcode, 0, 0);
        } else if (value_class != 1) {
            CError_Internal("CodeGen.c", 0x7AE);
        }
    } else {
        if (copts.unknown01) {
            call_runtime_function(rt_profile_e);
        }
    }

    if (!bypass) {
        if (!returnlabel->pclabel) {
            returnlabel->pclabel = makepclabel();
        }
        branch_always(returnlabel->pclabel);
        branches_to_return = TRUE;
        for (list = codegen_temp_objects_600B94; list; list = list->next) {
            list->object->u.unk30 = 0;
        }
    }
}

void fn_004350E0(void* unused) {
    CWMemHandle handle;
    void* pref;
    SInt32 error;
    UInt32 i;

    for (i = 0; i < 16; i++) {
        codegen_reserved_registers_622580[i] = 0;
    }
    error = CWGetNamedPreferences(cparams.context, "PS2 MIPS CodeGen", &handle);
    if (!error) {
        if (!CWLockMemHandle(cparams.context, handle, 0, &pref)) {
            __builtin_memcpy(&codegen_prefbyte_5C9A82, pref, 20);
            CWUnlockMemHandle(cparams.context, handle);
        }
    }
    copts.stdc_fenv_access = 1;
    if (copts.optimizesize) {
        cg_option_00637387 = 0;
    }
    copts.readonly_strings = codegen_prefbyte_5C9A82;
    copts.inline_intrinsics = inlineIntrinsicsEnabled;
    copts.structalignment = 2;
    copts.fp_library = 0;
    copts.unknown0C = 1;
    copts.unknown01 = 0;
    copts.unknown0F = 0;
    copts.unknown10 = 1;
    copts.unknown15 = 0;
    copts.unknown17 = 0;
    cg_option_00636780 = 0;
    copts.unknown33 = 4;
    copts.unknown39 = 2;
    copts.warn_inlineasm = 0;
    copts.cse_hard_reg_gpr = 1;
    copts.cse_hard_reg_fpr = 1;
    copts.cse_hard_reg_special = 0;
    copts.unknown13A = 1;
    copts.unknown02 = 29;
    copts.unknown19 = 1;
    copts.littleendian = 1;
    copts.unknown11 = 1;
    copts.unknown15 = 1;
    copts.unknown10 = 0;
    copts.unknown0F = 1;
    copts.gcc_extensions = 1;
    copts.padloop = 1;
    copts.dividefix = 1;
    copts.fastmath = 1;
    copts.enable_vu0_registers = 1;
    copts.vu0_register_number = 32;
    copts.unknown48 = 25;
    copts.unknown47 = 0;
    copts.cse_hard_reg_vu0 = 0;
    copts.cse_vector_type = 0;
    copts.volatileasm = 0;
    if (!CWGetNamedPreferences(cparams.context, "MIPS Bare Linker Panel", &handle)) {
        if (!CWLockMemHandle(cparams.context, handle, 0, &pref)) {
            CWUnlockMemHandle(cparams.context, handle);
        }
        CWFreeMemHandle(cparams.context, handle);
    }
    copts.unknown139 = codegen_prefbyte_5C9A91;
    if (CWGetNamedPreferences(cparams.context, "MIPS Bare Project", &handle)) {
        copts.unknown04 = *(UInt16*)&handle;
    }
    copts.cats = 1;
    mul3_opcode = 1129;
    cg_option_00637326 = 1129;
    dmul3_opcode = 1092;
    cg_option_0063734E = 1092;
    div3_opcode = 1087;
    ddiv3_opcode = 1083;
    peephole_opcode_636FCC = 1164;
    peephole_opcode_637054 = 1163;
    cg_option_0063706A = 1119;
    cg_option_00637322 = 157;
    fn_004354E0();
}

Boolean CodeGen_ReInitRuntimeObjects(Boolean init) {
    rt_divdi3->name = GetHashNameNodeExport("__divdi3");
    rt_udivdi3->name = GetHashNameNodeExport("__udivdi3");
    rt_moddi3->name = GetHashNameNodeExport("__moddi3");
    rt_umoddi3->name = GetHashNameNodeExport("__umoddi3");
    rt_muldi3->name = GetHashNameNodeExport("__muldi3");
    rt_umuldi3->name = GetHashNameNodeExport("__muldi3");
    rt_ashrdi3->name = GetHashNameNodeExport("__ashrdi3");
    rt_lshrdi3->name = GetHashNameNodeExport("__lshrdi3");
    rt_ashldi3->name = GetHashNameNodeExport("__ashldi3");
    rt_l_muls->name = GetHashNameNodeExport("_l_muls");
    rt_l_mulu->name = GetHashNameNodeExport("_l_mulu");
    rt_l_divs->name = GetHashNameNodeExport("_l_divs");
    rt_l_divu->name = GetHashNameNodeExport("_l_divu");
    rt_l_mod->name = GetHashNameNodeExport("_l_mod");
    rt_l_modu->name = GetHashNameNodeExport("_l_modu");
    rt_fixdfdi->name = GetHashNameNodeExport("__fixdfdi");
    rt_fixunsdfdi->name = GetHashNameNodeExport("__fixunsdfdi");
    rt_floatdidf->name = GetHashNameNodeExport("__floatdidf");
    rt_d_ulltod->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_d_ulltod"
                                                                             : "_dflt_d_ulltod")
    );
    rt_fixsfdi->name = GetHashNameNodeExport("__fixsfdi");
    rt_fixunssfdi->name = GetHashNameNodeExport("__fixunssfdi");
    rt_floatdisf->name = GetHashNameNodeExport("__floatdisf");
    rt_f_ulltof->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_ulltof"
                                                                             : "_dflt_f_ulltof")
    );
    rt_dptoli->name = GetHashNameNodeExport("dptoli");
    rt_dptoul->name = GetHashNameNodeExport("dptoul");
    rt_litodp->name = GetHashNameNodeExport("litodp");
    rt_d_utod->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_d_utod"
                                                                             : "_dflt_d_utod")
    );
    rt_fptosi->name = GetHashNameNodeExport("fptosi");
    rt_fptoui->name = GetHashNameNodeExport("fptoui");
    rt_f_itof->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_itof"
                                                                             : "_dflt_f_itof")
    );
    rt_f_utof->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_utof"
                                                                             : "_dflt_f_utof")
    );
    rt_fp_round->name = GetHashNameNodeExport("_fp_round");
    rt_fp_get_fpscr->name = GetHashNameNodeExport("_fp_get_fpscr");
    rt_fp_set_fpscr->name = GetHashNameNodeExport("_fp_set_fpscr");
    rt_dpadd->name = GetHashNameNodeExport("dpadd");
    rt_dpsub->name = GetHashNameNodeExport("dpsub");
    rt_dpmul->name = GetHashNameNodeExport("dpmul");
    rt_dpdiv->name = GetHashNameNodeExport("dpdiv");
    rt_dpcmp->name = GetHashNameNodeExport("_dpcmp");
    rt_dpcmpe->name = GetHashNameNodeExport("_dpcmpe");
    rt_dpfeq->name = GetHashNameNodeExport("_dpfeq");
    rt_dpfne->name = GetHashNameNodeExport("_dpfne");
    rt_dpflt->name = GetHashNameNodeExport("_dpflt");
    rt_dpfle->name = GetHashNameNodeExport("_dpfle");
    rt_dpfgt->name = GetHashNameNodeExport("_dpfgt");
    rt_dpfge->name = GetHashNameNodeExport("_dpfge");
    rt_dpfor->name = GetHashNameNodeExport("_dpfor");
    rt_dpfun->name = GetHashNameNodeExport("_dpfun");
    rt_f_add->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_add"
                                                                             : "_dflt_f_add")
    );
    rt_f_sub->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_sub"
                                                                             : "_dflt_f_sub")
    );
    rt_f_mul->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_mul"
                                                                             : "_dflt_f_mul")
    );
    rt_f_div->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_div"
                                                                             : "_dflt_f_div")
    );
    rt_f_cmp->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_cmp"
                                                                             : "_dflt_f_cmp")
    );
    rt_f_cmpe->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_cmpe"
                                                                             : "_dflt_f_cmpe")
    );
    rt_f_feq->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_feq"
                                                                             : "_dflt_f_feq")
    );
    rt_f_fne->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_fne"
                                                                             : "_dflt_f_fne")
    );
    rt_f_flt->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_flt"
                                                                             : "_dflt_f_flt")
    );
    rt_f_fle->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_fle"
                                                                             : "_dflt_f_fle")
    );
    rt_f_fgt->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_fgt"
                                                                             : "_dflt_f_fgt")
    );
    rt_f_fge->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_fge"
                                                                             : "_dflt_f_fge")
    );
    rt_f_for->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_for"
                                                                             : "_dflt_f_for")
    );
    rt_f_fun->name = GetHashNameNodeExport(
        ((copts.fp_library && copts.stdc_fenv_access) || copts.unknown0C < 3 ? "_f_fun"
                                                                             : "_dflt_f_fun")
    );
    rt_dptofp->name = GetHashNameNodeExport("dptofp");
    rt_fptodp->name = GetHashNameNodeExport("fptodp");
    rt_profile_s->name = GetHashNameNodeExport("_profile_s");
    rt_profile_e->name = GetHashNameNodeExport("_profile_e");
    rt_strcpy->name = GetHashNameNodeExport("strcpy");
    rt_memcpy->name = GetHashNameNodeExport("memcpy");
    rt_MIPS16_prtglue32->name = GetHashNameNodeExport("__MIPS16_prtglue32");
    rt_MIPS16_prtglue16->name = GetHashNameNodeExport("__MIPS16_prtglue16");
    fn_00467F50();
    return fn_00459690(init);
}

void CodeGen_SetupRuntimeObjects(void) {
    rt_divdi3 = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_udivdi3 = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_moddi3 = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_umoddi3 = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_muldi3 = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_umuldi3 = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_ashrdi3 = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_lshrdi3 = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_ashldi3 = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_l_muls = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_l_mulu = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_l_divs = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_l_divu = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_l_mod = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_l_modu = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_fixdfdi = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_fixunsdfdi = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_floatdidf = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_d_ulltod = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_fixsfdi = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_fixunssfdi = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_floatdisf = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_ulltof = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dptoli = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dptoul = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_litodp = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_d_utod = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_fptosi = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_fptoui = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_itof = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_utof = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_fp_round = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_fp_get_fpscr = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_fp_set_fpscr = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpadd = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpsub = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpmul = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpdiv = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpcmp = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpcmpe = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpfeq = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpfne = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpflt = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpfle = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpfgt = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpfge = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpfor = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dpfun = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_add = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_sub = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_mul = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_div = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_cmp = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_cmpe = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_feq = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_fne = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_flt = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_fle = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_fgt = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_fge = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_for = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_f_fun = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_dptofp = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_fptodp = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_profile_s = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_profile_e = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_strcpy = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_memcpy = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_MIPS16_prtglue32 = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    rt_MIPS16_prtglue16 = CParser_NewRTFunc(TYPE(&stvoid), 0, 2, 0);
    Intrinsics_SetupRuntimeObjects();
    fn_00467F90();
}

static const UInt8 cg_arch_float_table[4] = {0, 1, 3, 7};

void fn_004354E0(void) {
    move_opcode = 0;
    move_opcode_637350 = 0;
    switch (((SInt16)copts.unknown02) & 0xFFF) {
        case 1:
            copts.no_branch_likely = 1;
            copts.unknown2E = 1;
            copts.floatsp32 = 1;
            copts.prefetch = 1;
            copts.unknown2F = 1;
            copts.unknown30 = 1;
            copts.fp_library = 7;
            copts.sqrt = 1;
            copts.late_conditional_move = 1;
            copts.unknown31 = 1;
            copts.unknown1A = 1;
            break;
        case 2:
            copts.no_branch_likely = 1;
            copts.unknown2E = 1;
            copts.prefetch = 1;
            copts.unknown2F = 1;
            copts.unknown30 = 1;
            copts.fp_library = 7;
            copts.sqrt = 1;
            copts.unknown23 = 1;
            copts.late_conditional_move = 1;
            copts.unknown0C = 5;
            copts.unknown31 = 1;
            break;
        case 17:
            copts.unknown02 |= 8192;
            copts.fp_library = 0;
            copts.unknown0C = 2;
            copts.unknown2E = 1;
            break;
        case 18:
            copts.unknown02 |= 8192;
            copts.fp_library = 0;
            copts.unknown0C = 3;
            copts.unknown2E = 1;
            break;
        case 19:
            copts.unknown02 |= 8192;
            copts.fp_library = 0;
            copts.unknown2E = 1;
            copts.unknown0C = 3;
            break;
        case 20:
        case 21:
            copts.unknown02 |= 8192;
            copts.fp_library = 7;
            copts.trunc = 1;
            copts.unknown2E = 1;
            copts.unknown0C = 3;
            break;
        case 22:
            copts.unknown02 |= 16384;
            copts.fp_library = 7;
            copts.trunc = 1;
            copts.unknown2E = 1;
            copts.unknown0C = 3;
            copts.gencvtcheck = 1;
            break;
        case 23:
            copts.unknown02 |= 16384;
            copts.fp_library = 7;
            copts.trunc = 1;
            copts.unknown2E = 1;
            copts.unknown0C = 4;
            copts.gencvtcheck = 1;
            copts.unknown2F = 1;
            mul3_opcode = 391;
            cg_option_00637326 = 392;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 9:
            copts.unknown02 |= 4096;
            copts.fp_library = 0;
            copts.unknown0C = 1;
            copts.no_branch_likely = 1;
            break;
        case 8:
            copts.unknown02 |= 4096;
            copts.fp_library = 3;
            copts.unknown0C = 1;
            copts.no_branch_likely = 1;
            break;
        case 12:
        case 13:
            copts.unknown02 |= 8192;
            copts.fp_library = 1;
            mul3_opcode = 374;
            copts.unknown2E = 1;
            copts.unknown0C = 3;
            copts.floatsp32 = 1;
            break;
        case 10:
        case 11:
        case 15:
            copts.unknown02 |= 8192;
            copts.fp_library = 7;
            copts.trunc = 1;
            copts.unknown2E = 1;
            copts.unknown0C = 3;
            break;
        case 14:
            copts.unknown02 |= 8192;
            mul3_opcode = 374;
            copts.unknown0C = 2;
            copts.unknown2E = 1;
            copts.fp_library = 0;
            copts.late_conditional_move = 1;
            copts.prefetch = 1;
            copts.unknown31 = 1;
            break;
        case 16:
            copts.unknown02 |= 16384;
            copts.fp_library = 7;
            copts.trunc = 1;
            copts.unknown2E = 1;
            copts.unknown0C = 4;
            copts.unknown2F = 1;
            break;
        case 3:
            copts.unknown02 |= 8192;
            copts.fp_library = 0;
            copts.unknown2E = 1;
            mul3_opcode = 382;
            cg_option_00637326 = 383;
            dmul3_opcode = 384;
            cg_option_0063734E = 385;
            div3_opcode = 388;
            ddiv3_opcode = 386;
            copts.unknown0C = 3;
            break;
        case 27:
            copts.unknown02 |= 16384;
            copts.fp_library = 7;
            copts.trunc = 1;
            copts.unknown2E = 1;
            mul3_opcode = 748;
            copts.unknown0C = 3;
            break;
        case 28:
            copts.unknown26 = 0;
            copts.unknown0C = 2;
            copts.unknown2E = 1;
            copts.unknown02 |= 8192;
            cg_option_00636780 = 1;
            copts.fp_library = 0;
            copts.unknown16 = 1;
            peephole_opcode_636FCC = 997;
            peephole_opcode_637054 = 996;
            move_opcode = 991;
            break;
        case 29:
            stack_alignment_63560C = 16;
            copts.unknown02 |= 16384;
            copts.fp_library = 1;
            copts.unknown39 = 3;
            copts.unknown2E = 1;
            copts.unknown0C = 3;
            copts.unknown41 = 1;
            copts.conditional_move = 0;
            copts.late_conditional_move = 1;
            copts.prefetch = 1;
            copts.sqrt = 1;
            copts.unknown23 = 1;
            copts.floatsp32 = 1;
            copts.unknown25 = 1;
            copts.usefloatacc = 1;
            mul3_opcode = 475;
            cg_option_00637326 = 476;
            move_opcode_637350 = 460;
            move_opcode = 171;
            break;
        case 24:
        case 25:
        case 26:
            copts.unknown02 |= 16384;
            copts.fp_library = 7;
            copts.late_conditional_move = 1;
            copts.trunc = 1;
            copts.unknown2E = 1;
            copts.unknown0C = 4;
            copts.unknown2F = 1;
            copts.sqrt = 1;
            copts.unknown23 = 1;
            mul3_opcode = 379;
            break;
        default:
            if ((SInt16)copts.unknown02 == 0x1000) {
                copts.unknown0C = 1;
                copts.fp_library = cg_arch_float_table[((UInt16)cg_arch_register_type_5C9A7E) & 3];
                copts.no_branch_likely = 1;
            } else if ((SInt16)copts.unknown02 == 0x2000) {
                copts.unknown0C = cg_arch_register_type_5C9A80 + 1;
                copts.fp_library = cg_arch_float_table[((UInt16)cg_arch_register_type_5C9A7E) & 3];
                copts.unknown2E = 1;
            } else if ((SInt16)copts.unknown02 == 0x4000) {
                copts.unknown0C = cg_arch_register_type_5C9A80 + 1;
                copts.fp_library = cg_arch_float_table[((UInt16)cg_arch_register_type_5C9A7E) & 3];
                copts.unknown2E = 1;
            } else {
                CError_Internal("CodeGen.c", 0xCA7);
            }
            break;
    }
    if (copts.unknown0C >= 2) {
        copts.unknown2E = 1;
        if (copts.fp_library) {
            copts.sqrt = 1;
        }
    }
    if (copts.unknown0C >= 4) {
        copts.late_conditional_move = 1;
        copts.prefetch = 1;
        copts.unknown23 = 1;
        copts.unknown2F = 1;
    }
    if (copts.unknown0C >= 5) {
        copts.unknown30 = 1;
    }
    copts.has8bytebitfields = 1;
    copts.unknown39 = 3;
    register_size = 8;
    objgen_alignment_mask_636430 = 7;
    cg_option_0063646C = 3;
    cg_option_0063731C = 0x454;
    target_register_637328 = 0x479;
    be_macro_saved_gp_reg_63732E = 0x93;
    be_macro_saved_fp_reg_637338 = 0xC4;
    if (!move_opcode) {
        move_opcode = 0xAB;
    }
    fn_0047CC40();
    CMach_Configure();
}
