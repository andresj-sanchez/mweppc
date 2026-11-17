#include "compiler/common.h"
#include "compiler/CError.h"
#include "compiler/RegisterInfo.h"
#include "compiler/CParser.h"
#include "compiler/CMangler.h"
#include "compiler/CPrep.h"
#include "compiler/tokens.h"
#include "compiler/CInt64.h"
#include "compiler/CPrepTokenizer.h"
#include "compiler/Registers.h"
#include "compiler/ObjGen.h"
#include "compiler/BE_binary.h"
#include "compiler/CMachine.h"
#include "compiler/objects.h"
#include "compiler/PCode.h"
#include "compiler/CompilerTools.h"
#pragma defer_codegen off
#pragma opt_propagation off
void fn_0046D2E0(UInt32 mask[7]) {
    register SInt8 cls;
    SInt32 idx;
    PCodeBlock* block;
    PCode* instr;
    PCodeArg* arg;
    SInt32 i;

    for (cls = 0; cls < 7;) {
        idx = cls;
        cls++;
        mask[idx] = 0;
    }
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (instr = block->firstPCode; instr; instr = instr->nextPCode) {
            for (i = (SInt16)instr->argCount, arg = instr->args; i--; arg++) {
                if (arg->kind == PCOp_REGISTER && (arg->effect & 2)) {
                    mask[(SInt8)arg->arg] |= 1 << (SInt16)arg->data.reg.reg;
                }
            }
        }
    }
}

void fn_0046D200(Object* obj, UInt32 mask[7]) {
    RegisterMaskRecord* record;
    UInt32 slot;
    register SInt8 cls;

    cls = 0;
    if (cls < 7) {
        do {
            SInt32 index = cls;
            cls++;
            mask[index] = 0xFFFFFFFF;
        } while (cls < 7);
    }
    if (!CParser_HasInternalLinkage(obj)) {
        return;
    }
    slot = obj->name->hashval & 127;
    for (record = register_mask_heads[slot]; record; record = record->next) {
        if (record->object == obj) {
            break;
        }
    }
    if (!record) {
        return;
    }
    {
        cls = 0;
        if (cls < 7) {
            do {
                SInt32 index = cls;
                SInt32 value = record->mask[index];
                cls++;
                mask[index] = value;
            } while (cls < 7);
        }
    }
}

#pragma opt_propagation on
#pragma defer_codegen on

#include "driver/COSToolsCLT.h"

void open_temp_registers(void) {
    register SInt8 cls = 0;
    while (cls < 7) {
        SInt32 i = cls;
        SInt32 saved;
        saved = used_virtual_registers[i];
        last_temporary_register[i] = saved;
        saved = *(volatile SInt32*)&last_temporary_register[i];
        cls++;
        first_temporary_register[i] = saved;
    }
}

#pragma defer_codegen off
#pragma opt_propagation off
SInt32 fn_0046D3F0(void) {
    register SInt8 cls = 0;
    SInt32 count = 0;
    while (cls < 7) {
        SInt32 i = cls;
        cls++;
        count += n_scratch_registers[i];
    }
    return count;
}

#pragma opt_propagation on
#pragma defer_codegen on

SInt32 obtain_nonvolatile_register(SInt8 cls) {
    SInt32 result;
    result = -1;
    if (starting_available_registers[cls] < n_nonvolatile_registers[cls]) {
        result = nonvolatile_registers[cls][starting_available_registers[cls]++];
    }
    return result;
}

UInt32 volatile_registers(SInt8 cls) {
    UInt32 mask;
    SInt32 i;
    SInt32 reg;

    mask = 0;
    for (i = 0; i < n_scratch_registers[cls]; i++) {
        reg = scratch_registers[cls][i];
        if (scheduler_register_colors[cls][reg] == 0) {
            mask |= 1 << reg;
        }
    }
    return mask;
}

SInt32 available_registers(SInt8 cls) {
    SInt32 count;
    SInt32 i;

    count = 0;
    for (i = 0; i < n_real_registers[cls]; i++) {
        if (scheduler_register_colors[cls][i] == 0) {
            count++;
        }
    }
    return count;
}

void retain_register(Object* obj, SInt8 cls, SInt16 reg) {
    VarInfo* info;

    info = obj->u.var.info;
    scheduler_register_colors[cls][reg] = 1;
    info->flags |= 2;
    info->rclass = cls;
    info->reg = reg;
}

void close_temp_registers(void) {
    SInt8 cls = 0;
    SInt32 r;
    while (cls < 7) {
        r = used_virtual_registers[cls];
        if (last_temporary_register[cls] < r) {
            last_temporary_register[cls] = r;
        } else {
            used_virtual_registers[cls] = last_temporary_register[cls];
        }
        cls++;
    }
}

void check_temp_registers(void) {
    SInt8 cls;
    SInt8 second;
    SInt32 r;

    for (cls = 0; cls < 7; cls++) {
        r = used_virtual_registers[cls];
        if (r < 0 || r >= 0x7FF8) {
            CError_Error(335);
        }
    }
    if (optimizing) {
        return;
    }
    for (second = 0; second < 7; second++) {
        r = used_virtual_registers[second];
        if (r > last_temporary_register[second]) {
            last_temporary_register[second] = r;
        }
        if (r > 256) {
            used_virtual_registers[second] = first_temporary_register[second];
        }
    }
}

void init_registers(void) {
    volatile SInt8 cls;
    register SInt8 primary;
    SInt32 reg;

    for (primary = 0; primary < 7; primary++) {
        UInt8* row = scheduler_register_colors[primary];
        for (reg = 0; reg < 32; reg++) {
            row[reg] = 0;
        }
    }
    for (cls = 0; cls < 7; cls++) {
        starting_available_registers[cls] = 0;
    }
    optimizing = TRUE;
    init_target_registers();
    for (cls = 0; cls < 7; cls++) {
        used_virtual_registers[cls] = n_real_registers[cls];
    }
    coloring = TRUE;
}

void fn_0046D7C0(void) {}
void fn_0046D7D0(void) {}

void fn_0046D7E0(void* ctx) {
    register_code_context = ctx;
}

void fn_0046D7F0(void) {}

void fn_0046D290(Object* obj) {
    RegisterMaskRecord* record;

    record = galloc(sizeof(RegisterMaskRecord));
    record->object = obj;
    fn_0046D2E0(record->mask);
    record->next = register_mask_heads[obj->name->hashval & 127];
    register_mask_heads[obj->name->hashval & 127] = record;
}

void fn_0046D380(void) {
    SInt32 i;
    SInt32 j;

    for (i = 0; i < 128;) {
        j = i;
        i += 8;
        register_mask_heads[j] = NULL;
        register_mask_heads[j + 1] = NULL;
        register_mask_heads[j + 2] = NULL;
        register_mask_heads[j + 3] = NULL;
        register_mask_heads[j + 4] = NULL;
        register_mask_heads[j + 5] = NULL;
        register_mask_heads[j + 6] = NULL;
        register_mask_heads[j + 7] = NULL;
    }
}

void assign_register_to_variable(Object* obj, SInt8 cls) {
    VarInfo* info;
    SInt16 i;
    volatile SInt32 selected;

    info = obj->u.var.info;
    for (i = 0; i < n_nonvolatile_registers[cls]; i++) {
        if (scheduler_register_colors[cls][nonvolatile_registers[cls][i]] == 0) {
            selected = (UInt16)nonvolatile_registers[cls][i];
            scheduler_register_colors[cls][(SInt16)selected] = 1;
            starting_available_registers[cls]++;
            assignable_registers[cls]--;
            break;
        }
    }
    info->flags |= 2;
    info->rclass = cls;
    info->reg = selected;
}

void fn_0046E3F0(Object* a, void* b, void* c, UInt32 d) {
    objgen_fallback_637377 = TRUE;
    fn_0046DD50(a, b, c, d);
}

#include <string.h>

void fn_0046C530(SInt16 expected) {
    if (fn_0046C840() != expected) {
        CError_Internal(register_assert_source, 623);
    }
}

char* fn_0046C430(char* input) {
    char buffer[1024];
    char* dest;
    char* grown;
    UInt32 limit;
    UInt32 length;
    volatile UInt32 value;

    dest = buffer;
    limit = 1023;
    length = 0;
    for (;;) {
        value = (UInt8)*input;
        if (register_character_class[value]) {
            if (length >= limit) {
                grown = lalloc(limit + 1024);
                memcpy(grown, dest, limit);
                limit += 1024;
                dest = grown;
            }
            dest[length++] = value;
            input++;
        } else if (value == '\\' && input[1] == '\r') {
            if (!copts.multibyteaware || !fn_00439B50(limit, input)) {
                if (input[2] == '\n') {
                    input += 3;
                } else {
                    input += 2;
                }
                fn_00442830();
            } else {
                break;
            }
        } else {
            break;
        }
    }
    dest[length] = 0;
    tkidentifier = GetHashNameNodeExport(dest);
    return input;
}

SInt32 fn_0046C560(void) {
    char* start;
    char* ptr;
    SInt32 ch;
    start = pos;
    ptr = start;
    for (;;) {
        ch = (UInt8)*ptr++;
        switch ((SInt8)ch) {
            case 47:
                if (*ptr == 47 && (!copts.ANSIstrict || copts.cplusplus)) {
                    return 32;
                }
                if (*ptr == 42) {
                    return 32;
                }
                return 47;
            case 63:
                if (*ptr != 63 || !copts.trigraphs) {
                    return ch;
                }
                switch (ptr[1]) {
                    case 61:
                        return 35;
                    case 47:
                        ptr += 2;
                        ch = 92;
                        break;
                    case 39:
                        return 94;
                    case 40:
                        return 91;
                    case 41:
                        return 93;
                    case 33:
                        return 124;
                    case 60:
                        return 123;
                    case 62:
                        return 125;
                    case 45:
                        return 126;
                    default:
                        return ch;
                }
                break;
            case 92:
                break;
            default:
                return ch;
        }
        if (*ptr == 13) {
            if (!copts.multibyteaware || !fn_00439B50((UInt32)start, ptr - 1)) {
                if (ptr[1] == 10) {
                    ptr += 2;
                } else {
                    ptr++;
                }
                continue;
            }
        }
        return ch;
    }
}

SInt32 fn_0046C6C0(UInt32 unused, UInt8 line, Boolean consume_newlines) {
    char* cur;
    SInt32 ch;

    cur = pos;
    for (;;) {
        ch = (UInt8)*cur++;
        switch ((SInt8)ch) {
            case 0:
                pos = cur - 1;
                return 0;
            case '\\':
                if (*cur == '\r') {
                    if (copts.multibyteaware) {
                        if (fn_00439B50(unused, cur - 1)) {
                            pos = cur;
                            return ch;
                        }
                    }
                    if (line) {
                        fn_00442830();
                    } else {
                        register_newline_seen = TRUE;
                    }
                    cur += cur[1] == '\n' ? 2 : 1;
                    continue;
                }
                break;
            case '?':
                if (copts.trigraphs && *cur == '?') {
                    switch (cur[1]) {
                        case '=':
                            ch = '#';
                            break;
                        case '/':
                            cur += 2;
                            ch = '\\';
                            if (*cur == '\r') {
                                cur += cur[1] == '\n' ? 2 : 1;
                                continue;
                            }
                            pos = cur;
                            return ch;
                        case '\'':
                            ch = '^';
                            break;
                        case '(':
                            ch = '[';
                            break;
                        case ')':
                            ch = ']';
                            break;
                        case '!':
                            ch = '|';
                            break;
                        case '<':
                            ch = '{';
                            break;
                        case '>':
                            ch = '}';
                            break;
                        case '-':
                            ch = '~';
                            break;
                        default:
                            break;
                    }
                    if (ch != '?') {
                        pos = cur + 2;
                        return ch;
                    }
                }
                break;
        }
        pos = cur;
        return ch;
    }
}

#include <stdio.h>

void fn_0046D800(Object* obj, UInt32 register_id) {
    char* linkname;
    ElfGenSymbol* variable;
    ElfGenSection* dbg;
    ElfGenSection* relocations;
    ElfGenSymbol* symbol;
    PCodeBlock* block;
    PCode* instr;
    SInt32 count;
    void* encoded;
    void* ptr;
    UInt32 size;
    UInt32 address_offset;
    SInt32 long_form;
    char* label;
    struct RegisterDebugShortRecord* shortrecord;
    struct RegisterDebugLongRecord* longrecord;

    linkname = CMangler_GetLinkName(obj)->name;
    variable = fn_0046EA30(obj);
    dbg = ElfAllocateSection(register_debug_name, 0xCA2A82C2, 0);
    dbg->shdr.sh_link = variable->sym.st_shndx;
    relocations = ElfAllocateSection(register_debug_label, 9, 0);
    ElfLinkRelocation(dbg, relocations);
    dbg->shdr.sh_addralign = 4;
    count = 0;
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        instr = block->lastPCode;
        if (instr &&
            (instr->flags & 0x80000) &&
            (instr->op != 0x3B || instr->args[0].data.reg.reg != 0x1F)) {
            count++;
        }
    }
    long_form = (SInt32)register_id > 0x7FFF;
    size = long_form ? count * 4 + 12 : count * 2 + 8;
    encoded = lalloc(size);
    if (long_form) {
        longrecord = encoded;
        longrecord->format = 3;
        longrecord->reserved = 0;
        longrecord->count = count;
        longrecord->reg = register_id;
        longrecord->address = 0;
        ptr = longrecord->locations;
        address_offset = 8;
    } else {
        shortrecord = encoded;
        shortrecord->format = 2;
        shortrecord->count = count;
        shortrecord->reg = register_id;
        shortrecord->address = 0;
        ptr = shortrecord->locations;
        address_offset = 4;
    }
    if (count) {
        for (block = pcbasicblocks; block; block = block->nextBlock) {
            instr = block->lastPCode;
            if (instr &&
                (instr->flags & 0x80000) &&
                (instr->op != 0x3B || instr->args[0].data.reg.reg != 0x1F)) {
                if (long_form) {
                    *(UInt32*)ptr = instr->unk1C;
                    ptr = (UInt32*)ptr + 1;
                } else {
                    *(UInt16*)ptr = instr->unk1C;
                    ptr = (UInt16*)ptr + 1;
                }
            }
        }
    }
    label = galloc(strlen(linkname) + 10);
    sprintf(label, register_debug_format, linkname);
    symbol = ElfAllocateSymbolRecord(label, dbg->index, 0, 3, size, (UInt32)encoded, 4);
    ElfAppendSectionRelocation(2, symbol, address_offset, variable, 0);
}

SInt16 fn_0046C840(void) {
    char* start;
    char* ptr;
    SInt32 ch;

    start = pos;
    ptr = start;
    for (;;) {
        ch = (UInt8)*ptr++;
        switch ((SInt8)ch) {
            case 0:
                pos = ptr - 1;
                return 0;
            case '%':
                if (*ptr == ':' && (copts.cplusplus || copts.c9x)) {
                    ptr++;
                    pos = ptr;
                    return '#';
                }
                break;
            case '/':
                if (*ptr == '/') {
                    if (copts.ANSIstrict && !copts.cplusplus && !copts.c9x) {
                        pos = ptr + 1;
                        return '/';
                    }
                    if (!copts.cplusplus) {
                        break;
                    }
                    spaceskip = TRUE;
                    ptr++;
                    while (*ptr && *ptr != '\r') {
                        ptr++;
                    }
                    if (!*ptr && tokenstacklevel <= 0 && ptr < prep_file_end) {
                        *ptr = ' ';
                    }
                    continue;
                }
                if (*ptr == '*') {
                    spaceskip = TRUE;
                    ptr++;
                    for (;;) {
                        ch = (UInt8)*ptr++;
                        if (ch == 0 && tokenstacklevel <= 0 && ptr < prep_file_end) {
                            ptr[-1] = ' ';
                        }
                        if (ch == '\r') {
                            fn_00442830();
                        }
                        if (ch == '*' && *ptr == '/') {
                            ptr++;
                            break;
                        }
                        if (ch == 0) {
                            pos = ptr;
                            return 0;
                        }
                    }
                    continue;
                }
                break;
            case '\\':
                if (*ptr == '\r') {
                    if (!copts.multibyteaware || !fn_00439B50((UInt32)start, ptr - 1)) {
                        fn_00442830();
                        ptr += ptr[1] == '\n' ? 2 : 1;
                        continue;
                    }
                }
                break;
            case '?':
                if (copts.trigraphs && *ptr == '?') {
                    switch (ptr[1]) {
                        case '=':
                            ch = '#';
                            break;
                        case '/':
                            ptr += 2;
                            ch = '\\';
                            if (*ptr == '\r') {
                                fn_00442830();
                                ptr += ptr[1] == '\n' ? 2 : 1;
                                continue;
                            }
                            break;
                        case '\'':
                            ch = '^';
                            break;
                        case '(':
                            ch = '[';
                            break;
                        case ')':
                            ch = ']';
                            break;
                        case '!':
                            ch = '|';
                            break;
                        case '<':
                            ch = '{';
                            break;
                        case '>':
                            ch = '}';
                            break;
                        case '-':
                            ch = '~';
                            break;
                        default:
                            break;
                    }
                    if (ch != '?') {
                        ptr += 2;
                    }
                }
                break;
        }
        pos = ptr;
        return (SInt16)ch;
    }
}

void fn_0046D9E0(Object* obj, void* buffer, void* nodes, SInt32 depth, void* list) {
    char* name;
    char* prefixed_a;
    char* prefixed_b;
    ObjGenScopeEntry* scope;
    ElfGenSection* parent;
    ElfGenSection* section;
    ElfGenSymbol* entry;
    ElfGenSymbol* descriptor;
    ObjGenRelocation* ref;
    ObjGenRegisterLocation location;
    struct RegisterDebugSymbolRecord record;
    SInt32 offset;

    name = CMangler_GetLinkName(obj)->name;
    prefixed_a = galloc(strlen(name) + 5);
    memcpy(prefixed_a, register_dbg_prefix_a, 5);
    strcat(prefixed_a, name);
    prefixed_b = galloc(strlen(name) + 5);
    memcpy(prefixed_b, register_dbg_prefix_b, 5);
    strcat(prefixed_b, name);
    scope = &register_debug_scope_table[register_debug_type_index];
    parent = ElfAllocateSection((char*)scope->alias1, scope->sectionType, scope->attributes);
    scope->section = parent;
    scope = &register_debug_scope_table[register_debug_type_index];
    scope->relocation = ElfAllocateSection(scope->relocationName, scope->relocationType, 0);
    ElfLinkRelocation(scope->section, scope->relocation);
    if ((UInt32)depth < 5) {
        record.address = 0;
        record.size = (UInt32)buffer | 1;
        record.encoding = 0;
        memcpy(&record.encoding, nodes, depth);
        entry = ElfAllocateSymbolRecord(prefixed_b, parent->index, 0, 1, 12, (UInt32)&record, 4);
        ElfAppendSectionRelocation(2, entry, 0, fn_0046EA30(obj), 0);
    } else {
        scope = &register_debug_scope_table[register_debug_scope_index];
        section = ElfAllocateSection((char*)scope->alias1, scope->sectionType, scope->attributes);
        scope->section = section;
        for (ref = list; ref; ref = ref->next) {
            if (copts.unknown08 || copts.unknown07) {
                fn_0046EDD0(ref->object, &location);
                if (location.number == 23) {
                    offset = 2;
                } else if (location.number == 28) {
                    offset = 0;
                } else {
                    offset = 1;
                }
            } else {
                offset = 0;
            }
            *(UInt32*)((char*)nodes + ref->offset) += offset;
        }
        entry = ElfAllocateSymbolRecord(prefixed_a, section->index, 0, 1, depth, (UInt32)nodes, 4);
        if (list) {
            scope = &register_debug_scope_table[register_debug_scope_index];
            scope->relocation = ElfAllocateSection(scope->relocationName, scope->relocationType, 0);
            ElfLinkRelocation(scope->section, scope->relocation);
            for (ref = list; ref; ref = ref->next) {
                ElfAppendSectionRelocation(
                    2, entry, ref->offset, fn_0046EA30(ref->object), ref->addend
                );
            }
        }
        record.address = 0;
        record.size = (UInt32)buffer;
        if (copts.unknown08 || copts.unknown07) {
            switch (register_debug_scope_table[register_debug_scope_index].category) {
                case 1:
                case 2:
                    record.encoding = 1;
                    break;
                case 3:
                case 4:
                    record.encoding = 2;
                    break;
                case 5:
                case 6:
                    record.encoding = 0;
                    break;
                default:
                    CError_Internal(register_assert_source, 1126);
            }
        } else {
            record.encoding = 0;
        }
        descriptor =
            ElfAllocateSymbolRecord(prefixed_b, parent->index, 0, 1, 12, (UInt32)&record, 4);
        ElfAppendSectionRelocation(2, descriptor, 0, fn_0046EA30(obj), 0);
        ElfAppendSectionRelocation(2, descriptor, 8, entry, 0);
    }
}

void fn_0046DD50(Object* obj, void* data, void* refs, UInt32 size) {
    ObjGenScopeEntry* scope;
    ElfGenSection* section;
    ElfGenSymbol* symbol;
    ElfGenSection* pic_section;
    ElfGenSection* pid_section;
    ElfGenSymbol* pic_symbol;
    ElfGenSymbol* pid_symbol;
    ObjGenRegisterLocation location;
    ObjGenRegisterLocation self_location;
    UInt32* pic_data;
    UInt32* pid_data;
    SInt32 pic_count = 0;
    SInt32 pid_count = 0;
    SInt32 pic_index;
    SInt32 pid_index;
    SInt32 binding;
    SInt32 encoding;
    char* name;
    char* label;

    ObjGenRelocation* ref;
    void* bytes;
    SInt32 index;
    UInt32 alignment;

    index = fn_0046EF20(obj);
    symbol = fn_0046EA30(obj);
    scope = &register_debug_scope_table[index];
    if (!symbol->sym.st_shndx) {
        if (!data && scope->alias2) {
            section = ElfAllocateSection((char*)scope->alias2, scope->bssType, scope->attributes);
            scope->bss = section;
        } else {
            section =
                ElfAllocateSection((char*)scope->alias1, scope->sectionType, scope->attributes);
            scope->section = section;
        }
    } else {
        section = fn_0053F810(symbol->sym.st_shndx);
    }
    if (CParser_HasInternalLinkage(obj)) {
        binding = 0;
    } else if (obj->qual & 0x20000) {
        binding = 13;
    } else if (obj->qual & 0x40000) {
        binding = 2;
    } else if (obj->flags & 0x40) {
        binding = 15;
    } else {
        binding = 1;
    }
    if (copts.force_active || (binding == 1 && (obj->flags & 0x40)) || binding == 15) {
        section->shdr.sh_flags |= 0x20000000;
    }
    alignment = CMach_GetObjectAlign(obj);
    bytes = data;
    if (refs) {
        bytes = lalloc(size);
        memcpy(bytes, data, size);
        for (ref = refs; ref; ref = ref->next) {
            *(SInt32*)((char*)bytes + ref->offset) += ref->addend;
            ref->addend = 0;
        }
    }
    if (!symbol->sym.st_shndx) {
        ElfDefineSymbolRecord(symbol, section->index, binding, 1, size, (UInt32)bytes, alignment);
    }
    if (refs) {
        scope = &register_debug_scope_table[index];
        scope->relocation = ElfAllocateSection(scope->relocationName, scope->relocationType, 0);
        ElfLinkRelocation(scope->section, scope->relocation);
        for (ref = refs; ref; ref = ref->next) {
            if (copts.unknown07 || copts.unknown08) {
                fn_0046EDD0(ref->object, &location);
                if (copts.unknown07 && location.number == 28) {
                    pid_count++;
                }
                if (copts.unknown08 && location.number == 23) {
                    pic_count++;
                }
            }
            ElfAppendSectionRelocation(
                2, symbol, ref->offset, fn_0046EA30(ref->object), ref->addend
            );
        }
    }
    if (obj->datatype == DLOCAL || !strchr(CMangler_GetLinkName(obj)->name, '$')) {
        if (copts.unknown13A == 1) {
            fn_00541160(obj, binding != 0, symbol, size);
        }
    }
    name = CMangler_GetLinkName(obj)->name;
    fn_0046EDD0(obj, &self_location);
    if (self_location.number == 23) {
        encoding = 2;
    } else if (self_location.number == 28) {
        encoding = 0;
    } else {
        encoding = 1;
    }
    if (pic_count) {
        pic_data = lalloc(pic_count * 4);
        memset(pic_data, 0, pic_count * 4);
        scope = &register_debug_scope_table[objgen_picrelo_635FC4];
        pic_section =
            ElfAllocateSection((char*)scope->alias1, scope->sectionType, scope->attributes);
        scope->section = pic_section;
        scope = &register_debug_scope_table[objgen_picrelo_635FC4];
        scope->relocation = ElfAllocateSection(scope->relocationName, scope->relocationType, 0);
        ElfLinkRelocation(scope->section, scope->relocation);
        label = galloc(strlen(name) + 7);
        sprintf(label, objgen_pic_format, name);
        pic_symbol = ElfNewSymbolRecord(label, 0, 0, 3, 0, 0);
    }
    if (pid_count) {
        pid_data = lalloc(pid_count * 4);
        memset(pid_data, 0, pid_count * 4);
        scope = &register_debug_scope_table[objgen_pidrelo_6361A8];
        if (!data && scope->alias2) {
            pid_section =
                ElfAllocateSection((char*)scope->alias2, scope->bssType, scope->attributes);
            scope->bss = pid_section;
        } else {
            pid_section =
                ElfAllocateSection((char*)scope->alias1, scope->sectionType, scope->attributes);
            scope->section = pid_section;
        }
        scope = &register_debug_scope_table[objgen_pidrelo_6361A8];
        scope->relocation = ElfAllocateSection(scope->relocationName, scope->relocationType, 0);
        ElfLinkRelocation(scope->section, scope->relocation);
        label = galloc(strlen(name) + 7);
        sprintf(label, objgen_pid_format, name);
        pid_symbol = ElfNewSymbolRecord(label, 0, 0, 3, 0, 0);
    }
    if (pid_count + pic_count) {
        pic_index = 0;
        pid_index = 0;
        for (ref = refs; ref; ref = ref->next) {
            {
                fn_0046EDD0(ref->object, &location);
                if (copts.unknown08 && location.number == 23) {
                    pic_data[pic_index++] = ref->offset + encoding;
                }
                if (copts.unknown07 && location.number == 28) {
                    pid_data[pid_index++] = ref->offset + encoding;
                }
            }
        }
        if (pic_index) {
            ElfDefineSymbolRecord(
                pic_symbol, pic_section->index, 0, 3, pic_index * 4, (UInt32)pic_data, 4
            );
        }
        if (pid_index) {
            ElfDefineSymbolRecord(
                pid_symbol, pid_section->index, 0, 3, pid_index * 4, (UInt32)pid_data, 4
            );
        }
        pic_index = 0;
        pid_index = 0;
        for (ref = refs; ref; ref = ref->next) {
            fn_0046EDD0(ref->object, &location);
            if (copts.unknown08 && location.number == 23) {
                pic_data[pic_index] = ref->offset;
                ElfAppendSectionRelocation(2, pic_symbol, pic_index * 4, symbol, 0);
                pic_index++;
            }
            if (copts.unknown07 && location.number == 28) {
                pid_data[pid_index] = ref->offset;
                ElfAppendSectionRelocation(2, pid_symbol, pid_index * 4, symbol, 0);
                pid_index++;
            }
        }
    }
}

void fn_0046E410(Object* obj, void* data, void* buffer, void* refs) {
    ObjGenScopeEntry* scope;
    ElfGenSection* section;
    ElfGenSymbol* symbol;
    ElfGenSection* pic_section;
    ElfGenSection* pid_section;
    ElfGenSymbol* pic_symbol;
    ElfGenSymbol* pid_symbol;
    ObjGenRegisterLocation location;
    ObjGenRegisterLocation self_location;
    UInt32* pic_data;
    UInt32* pid_data;
    SInt32 pic_count = 0;
    SInt32 pid_count = 0;
    SInt32 pic_index;
    SInt32 pid_index;
    SInt32 binding;
    SInt32 encoding;
    char* name;
    char* label;

    ObjGenCodeRelocation* ref;
    SInt32 index;
    SInt32 alignment;

    index = fn_0046EF20(obj);
    scope = &register_debug_scope_table[index];
    section = ElfAllocateSection((char*)scope->alias1, scope->sectionType, scope->attributes);
    scope->section = section;
    symbol = fn_0046EA30(obj);
    if (CParser_HasInternalLinkage(obj)) {
        binding = 0;
    } else if (obj->qual & 0x20000) {
        binding = 13;
    } else if (obj->qual & 0x40000) {
        binding = 2;
    } else if (obj->flags & 0x40) {
        binding = 15;
    } else {
        binding = 1;
    }
    if (copts.force_active ||
        copts.interrupt ||
        (binding == 1 && (obj->flags & 0x40)) ||
        binding == 15) {
        section->shdr.sh_flags |= 0x20000000;
    }
    alignment = copts.unknown41 ? 16 : 4;
    if (binary_alignment > alignment) {
        alignment = binary_alignment;
    }
    ElfDefineSymbolRecord(
        symbol, section->index, binding, 2, (UInt32)buffer, (UInt32)data, alignment
    );
    if (refs) {
        scope->relocation = ElfAllocateSection(scope->relocationName, scope->relocationType, 0);
        ElfLinkRelocation(scope->section, scope->relocation);
        for (ref = refs; ref; ref = ref->next) {
            if (ref->kind == 2 && (copts.unknown07 || copts.unknown08)) {
                fn_0046EDD0(ref->object, &location);
                if (copts.unknown07 && location.number == 28) {
                    pid_count++;
                }
                if (copts.unknown08 && location.number == 23) {
                    pic_count++;
                }
            }
            if (ref->object) {
                ElfAppendSectionRelocation(
                    ref->kind, symbol, ref->offset, fn_0046EA30(ref->object), 0
                );
            }
        }
    }
    if (copts.unknown13A == 1) {
        fn_00541380(obj, binding != 0, symbol, (SInt32)buffer);
    }
    name = CMangler_GetLinkName(obj)->name;
    fn_0046EDD0(obj, &self_location);
    if (self_location.number == 23) {
        encoding = 2;
    } else if (self_location.number == 28) {
        encoding = 0;
    } else {
        encoding = 1;
    }
    if (pic_count) {
        pic_data = lalloc(pic_count * 4);
        memset(pic_data, 0, pic_count * 4);
        scope = &register_debug_scope_table[objgen_picrelo_635FC4];
        pic_section =
            ElfAllocateSection((char*)scope->alias1, scope->sectionType, scope->attributes);
        scope->section = pic_section;
        scope = &register_debug_scope_table[objgen_picrelo_635FC4];
        scope->relocation = ElfAllocateSection(scope->relocationName, scope->relocationType, 0);
        ElfLinkRelocation(scope->section, scope->relocation);
        label = galloc(strlen(name) + 7);
        sprintf(label, objgen_pic_format, name);
        pic_symbol = ElfNewSymbolRecord(label, 0, 0, 3, 0, 0);
    }
    if (pid_count) {
        pid_data = lalloc(pid_count * 4);
        memset(pid_data, 0, pid_count * 4);
        scope = &register_debug_scope_table[objgen_pidrelo_6361A8];
        if (!data && scope->alias2) {
            pid_section =
                ElfAllocateSection((char*)scope->alias2, scope->bssType, scope->attributes);
            scope->bss = pid_section;
        } else {
            pid_section =
                ElfAllocateSection((char*)scope->alias1, scope->sectionType, scope->attributes);
            scope->section = pid_section;
        }
        scope = &register_debug_scope_table[objgen_pidrelo_6361A8];
        scope->relocation = ElfAllocateSection(scope->relocationName, scope->relocationType, 0);
        ElfLinkRelocation(scope->section, scope->relocation);
        label = galloc(strlen(name) + 7);
        sprintf(label, objgen_pid_format, name);
        pid_symbol = ElfNewSymbolRecord(label, 0, 0, 3, 0, 0);
    }
    if (pid_count + pic_count) {
        pic_index = 0;
        pid_index = 0;
        for (ref = refs; ref; ref = ref->next) {
            if (ref->kind == 2) {
                fn_0046EDD0(ref->object, &location);
                if (copts.unknown08 && location.number == 23) {
                    pic_data[pic_index++] = ref->offset + encoding;
                }
                if (copts.unknown07 && location.number == 28) {
                    pid_data[pid_index++] = ref->offset + encoding;
                }
            }
        }
        if (pic_index) {
            ElfDefineSymbolRecord(
                pic_symbol, pic_section->index, 0, 3, pic_index * 4, (UInt32)pic_data, 4
            );
        }
        if (pid_index) {
            ElfDefineSymbolRecord(
                pid_symbol, pid_section->index, 0, 3, pid_index * 4, (UInt32)pid_data, 4
            );
        }
        pic_index = 0;
        pid_index = 0;
        for (ref = refs; ref; ref = ref->next) {
            fn_0046EDD0(ref->object, &location);
            if (copts.unknown08 && location.number == 23) {
                pic_data[pic_index] = ref->offset;
                ElfAppendSectionRelocation(2, pic_symbol, pic_index * 4, symbol, 0);
                pic_index++;
            }
            if (copts.unknown07 && location.number == 28) {
                pid_data[pid_index] = ref->offset;
                ElfAppendSectionRelocation(2, pid_symbol, pid_index * 4, symbol, 0);
                pid_index++;
            }
        }
    }
}

SInt32 prepskipnextchar(void) {
    SInt32 c;
    StringPtr start;
    StringPtr p;

    start = (StringPtr)pos;
    p = (StringPtr)pos;

loop:
    switch ((SInt16)(c = *p)) {
        case 0:
            if (tokenstacklevel > 0 || p >= (StringPtr)prep_file_end) {
                nextcharpos = pos = (char*)p;
                return c;
            } else {
                pos = (char*)p++;
                nextcharpos = (char*)p;
                return c;
            }
        case 4:
        case 5:
            p++;
            goto loop;
        case ' ':
        case '\t':
        case '\n':
        case '\v':
        case '\f':
            p++;
            spaceskip = TRUE;
            goto loop;
        case ';':
            pos = (char*)p++;
            if (tokenstacklevel <= 0 && cprep_in_assembler && copts.asmsemicolcomment) {
                spaceskip = TRUE;
                nextcharpos = (char*)CPrep_SkipNewComment(p);
                return CPrep_SkipNewCommentChar;
            }
            nextcharpos = (char*)p;
            return c;
        case '#':
            pos = (char*)p++;
            if (tokenstacklevel <= 0 && cprep_in_assembler && copts.asmpoundcomment) {
                spaceskip = TRUE;
                if (*p != '\r' && *p != '\n') {
                    p++;
                }
                for (;;) {
                    c = *p++;
                    if (!(SInt16)c) {
                        if (tokenstacklevel > 0 || p >= (StringPtr)prep_file_end) {
                            fn_00443640(4);
                            nextcharpos = (char*)p - 1;
                            return c;
                        }
                        p[-1] = ' ';
                    }
                    if ((SInt16)c == '\\') {
                        if (copts.multibyteaware) {
                            while (p[0] == '\\' &&
                                   !fn_00439B50((UInt32)start, (char*)p) &&
                                   p[1] == '\r') {
                                fn_00442830();
                                if (p[2] == '\n') {
                                    p += 3;
                                } else {
                                    p += 2;
                                }
                            }
                        } else {
                            while (p[0] == '\\' && p[1] == '\r') {
                                fn_00442830();
                                if (p[2] == '\n') {
                                    p += 3;
                                } else {
                                    p += 2;
                                }
                            }
                        }
                        if (*p == 'n') {
                            p--;
                            goto loop;
                        }
                    }
                    if ((SInt16)c == '"' || (SInt16)c == '\r' || (SInt16)c == '\n') {
                        p--;
                        goto loop;
                    }
                }
            }
            nextcharpos = (char*)p;
            return c;
        case '/':
            pos = (char*)p++;
            if (copts.multibyteaware) {
                while (p[0] == '\\' && !fn_00439B50((UInt32)start, (char*)p) && p[1] == '\r') {
                    fn_00442830();
                    if (p[2] == '\n') {
                        p += 3;
                    } else {
                        p += 2;
                    }
                }
            } else {
                while (p[0] == '\\' && p[1] == '\r') {
                    fn_00442830();
                    if (p[2] == '\n') {
                        p += 3;
                    } else {
                        p += 2;
                    }
                }
            }
            if (tokenstacklevel <= 0) {
                if (p[0] == '/' && (!copts.ANSIstrict || copts.cplusplus || copts.c9x)) {
                    spaceskip = TRUE;
                    nextcharpos = (char*)CPrep_SkipNewComment(p + 1);
                    return CPrep_SkipNewCommentChar;
                } else if (p[0] == '*') {
                    spaceskip = TRUE;
                    p++;
                    for (;;) {
                        if (!(SInt16)(c = *(p++))) {
                            if (tokenstacklevel > 0 || p >= (StringPtr)prep_file_end) {
                                fn_00443640(4);
                                nextcharpos = (char*)p - 1;
                                return c;
                            }
                            p[-1] = ' ';
                        }
                        if ((SInt16)c == '*') {
                            if (copts.multibyteaware) {
                                while (p[0] == '\\' &&
                                       !fn_00439B50((UInt32)start, (char*)p) &&
                                       p[1] == '\r') {
                                    fn_00442830();
                                    if (p[2] == '\n') {
                                        p += 3;
                                    } else {
                                        p += 2;
                                    }
                                }
                            } else {
                                while (p[0] == '\\' && p[1] == '\r') {
                                    fn_00442830();
                                    if (p[2] == '\n') {
                                        p += 3;
                                    } else {
                                        p += 2;
                                    }
                                }
                            }
                            if (*p == '/') {
                                p++;
                                goto loop;
                            }
                        }
                        if ((SInt16)c == '\r') {
                            fn_00442830();
                        }
                    }
                }
            }
            nextcharpos = (char*)p;
            return c;
        case '%':
            pos = (char*)p++;
            if (*p == ':' && (copts.cplusplus || copts.c9x)) {
                nextcharpos = (char*)p + 1;
                return '#';
            }
            nextcharpos = (char*)p;
            return c;
        case '\\':
            pos = (char*)p++;
mid_backslash:
            if (*p == '\r') {
                if (!copts.multibyteaware || !fn_00439B50((UInt32)start, (char*)p - 1)) {
                    fn_00442830();
                    if (p[1] == '\n') {
                        p += 2;
                    } else {
                        p += 1;
                    }
                    goto loop;
                }
            }
            if (cprep_nostring && cprep_in_assembler) {
                switch (*p) {
                    case 'n':
                    case 'r':
                    case 't':
                    case 'v':
                        p++;
                        spaceskip = TRUE;
                        goto loop;
                }
            }
            nextcharpos = (char*)p;
            return c;
        case '?':
            pos = (char*)p++;
            if (copts.trigraphs && p[0] == '?') {
                switch (p[1]) {
                    case '=':
                        nextcharpos = (char*)p + 2;
                        return '#';
                    case '/':
                        c = '\\';
                        p += 2;
                        goto mid_backslash;
                    case '\'':
                        nextcharpos = (char*)p + 2;
                        return '^';
                    case '(':
                        nextcharpos = (char*)p + 2;
                        return '[';
                    case ')':
                        nextcharpos = (char*)p + 2;
                        return ']';
                    case '!':
                        nextcharpos = (char*)p + 2;
                        return '|';
                    case '<':
                        nextcharpos = (char*)p + 2;
                        return '{';
                    case '>':
                        nextcharpos = (char*)p + 2;
                        return '}';
                    case '-':
                        nextcharpos = (char*)p + 2;
                        return '~';
                }
            }
            nextcharpos = (char*)p;
            return c;
        default:
            pos = (char*)p++;
            nextcharpos = (char*)p;
            return c;
    }
}
