#include "compiler/RegListQueries.h"
#include <string.h>
#include "compiler/ELFgen.h"
#include "compiler/BE_macro.h"
#include "compiler/CError.h"
#include "compiler/CFunc.h"
#include "compiler/CMangler.h"
#include "compiler/CParser.h"
#include "compiler/CPrep.h"
#include "compiler/CompilerTools.h"
#include "compiler/DWARF.h"
#include "compiler/ELF_Endian.h"
#include "compiler/InstrSelection.h"
#include "compiler/ObjGen.h"
#include "compiler/Operands.h"
#include "compiler/PCode.h"
#include "compiler/RegisterInfo.h"
#include "compiler/objects.h"
#include "driver/COSToolsCLT.h"
#include "string.h"

void ElfLinkRelocation(ElfGenSection* owner, ElfGenSection* section) {
    owner->relocations = section;
    section->shdr.sh_info = owner->index;
    section->shdr.sh_link = elfSymbolTable->index;
}

void elf_insert_half(ElfGenSection* section, SInt32 offset, UInt16 val) {
    char* dst = *section->data.data + offset;
    if (elfEndian == 1) {
        dst[0] = val;
        dst[1] = val >> 8;
    } else {
        dst[0] = val >> 8;
        dst[1] = val;
    }
}

void elf_insert_word(ElfGenSection* section, SInt32 offset, UInt32 val) {
    char* dst = *section->data.data + offset;
    if (elfEndian == 1) {
        dst[0] = val;
        dst[1] = val >> 8;
        dst[2] = val >> 16;
        dst[3] = val >> 24;
    } else {
        dst[0] = val >> 24;
        dst[1] = val >> 16;
        dst[2] = val >> 8;
        dst[3] = val;
    }
}

SInt32 elf_append_byte(ElfGenSection* section, UInt8 val) {
    SInt32 offset = section->shdr.sh_size;
    section->shdr.sh_size++;
    AppendGListNoData(&section->data, 1);
    (*section->data.data)[offset] = val;
    return offset;
}

SInt32 elf_append_string(ElfGenSection* section, const char* str) {
    SInt32 size = __builtin_strlen(str) + 1;
    SInt32 offset = section->shdr.sh_size;
    section->shdr.sh_size += size;
    AppendGListData(&section->data, str, size);
    return offset;
}

#pragma dont_inline on
SInt32 elf_append_half(ElfGenSection* section, UInt16 val) {
    SInt32 offset = section->shdr.sh_size;
    char* dst;
    section->shdr.sh_size += 2;
    AppendGListNoData(&section->data, 2);
    dst = *section->data.data + offset;
    if (elfEndian == 1) {
        dst[0] = val;
        dst[1] = val >> 8;
    } else {
        dst[0] = val >> 8;
        dst[1] = val;
    }
    return offset;
}

SInt32 elf_append_word(ElfGenSection* section, UInt32 val) {
    SInt32 offset = section->shdr.sh_size;
    char* dst;
    section->shdr.sh_size += 4;
    AppendGListNoData(&section->data, 4);
    dst = *section->data.data + offset;
    if (elfEndian == 1) {
        dst[0] = val;
        dst[1] = val >> 8;
        dst[2] = val >> 16;
        dst[3] = val >> 24;
    } else {
        dst[0] = val >> 24;
        dst[1] = val >> 16;
        dst[2] = val >> 8;
        dst[3] = val;
    }
    return offset;
}

#pragma dont_inline reset

#pragma dont_inline on

ElfGenSymbol* ElfNewSymbolRecord(
    const char* name, UInt16 index, UInt8 binding, UInt8 kind, UInt32 size, UInt32 value
) {
    ElfGenSymbol* node = galloc(sizeof(ElfGenSymbol));
    memclrw(node, sizeof(ElfGenSymbol));
    node->name = name;
    node->sym.st_shndx = index;
    node->sym.st_info = ELF32_ST_INFO(binding, kind);
    node->sym.st_size = size;
    node->sym.st_value = value;
    *elfSymbolListTail = node;
    elfSymbolListTail = &node->next;
    return node;
}

#pragma dont_inline reset

ElfGenSymbol* ElfAllocateSymbolRecord(
    const char* name,
    UInt16 index,
    UInt8 binding,
    UInt8 kind,
    UInt32 value,
    UInt32 size,
    UInt32 alignment
) {
    ElfGenSymbol* symbol = galloc(sizeof(ElfGenSymbol));
    memclrw(symbol, sizeof(ElfGenSymbol));
    symbol->name = name;
    symbol->sym.st_shndx = SHN_UNDEF;
    symbol->sym.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE);
    symbol->sym.st_size = 0;
    symbol->sym.st_value = 0;
    *elfSymbolListTail = symbol;
    elfSymbolListTail = &symbol->next;
    ElfDefineSymbolRecord(symbol, index, binding, kind, value, size, alignment);
    return symbol;
}

void ElfDefineSymbolRecord(
    ElfGenSymbol* symbol,
    UInt16 index,
    UInt8 binding,
    UInt8 kind,
    UInt32 size,
    UInt32 data,
    UInt32 alignment
) {
    ElfGenSection* section;
    UInt32 padding;
    UInt32 offset;
    if (symbol->sym.st_shndx) {
        CError_Internal((char*)ELFgen_file_name, 0x1D6);
    }
    if (index >= 0xFF00) {
        CError_Internal((char*)ELFgen_file_name, 0x1DA);
    }
    symbol->sym.st_shndx = index;
    symbol->sym.st_info = ELF32_ST_INFO(binding, kind);
    symbol->sym.st_size = size;
    section = elfSectionTable[index];
    if ((SInt32)alignment > 1) {
        padding = (alignment - 1) & section->shdr.sh_size;
        if (padding) {
            if (section->shdr.sh_type != SHT_NOBITS) {
                AppendGListData(&section->data, elfZeroPadding, alignment - padding);
            }
            section->shdr.sh_size += alignment - padding;
        }
    }
    if (alignment > section->shdr.sh_addralign) {
        section->shdr.sh_addralign = alignment;
    }
    if (section->shdr.sh_type == SHT_NOBITS) {
        offset = section->shdr.sh_size;
        section->shdr.sh_size += size;
        symbol->sym.st_value = offset;
    } else if (!data) {
        offset = section->shdr.sh_size;
        section->shdr.sh_size += size;
        while (size > 0x40) {
            AppendGListData(&section->data, elfZeroPadding, 0x40);
            size -= 0x40;
        }
        AppendGListData(&section->data, elfZeroPadding, size);
        symbol->sym.st_value = offset;
    } else {
        offset = section->shdr.sh_size;
        section->shdr.sh_size += size;
        AppendGListData(&section->data, (void*)data, size);
        symbol->sym.st_value = offset;
    }
}

#pragma dont_inline on
void ElfAppendSectionRelocation(
    UInt8 kind, ElfGenSymbol* symbol, UInt32 addend, void* target, UInt32 data
) {
    ElfGenSection* section = elfSectionTable[symbol->sym.st_shndx];
    ElfGenSection* list = section->relocations;
    struct ElfGenRelocation* entry = galloc(sizeof(struct ElfGenRelocation));
    entry->next = NULL;
    entry->target = target;
    entry->offset = symbol->sym.st_value + addend;
    entry->info = kind;
    entry->addend = data;
    *list->listtail = entry;
    list->listtail = &entry->next;
}

#pragma dont_inline reset

#define ELF_EMIT_SYMBOL_WORD(source, offset) \
    do { \
        UInt32 pos = oldsize + (offset); \
        char* dest = *buffer->data + pos; \
        UInt32 v = (source); \
        if (elfEndian == 1) { \
            dest[0] = v; \
            dest[1] = v >> 8; \
            dest[2] = v >> 16; \
            dest[3] = v >> 24; \
        } else { \
            dest[0] = v >> 24; \
            dest[1] = v >> 16; \
            dest[2] = v >> 8; \
            dest[3] = v; \
        } \
    } while (0)

void ElfAppendSymbolData(ElfGenSection* section, Elf32_Sym* symbol) {
    GList* buffer = &section->data;
    UInt32 oldsize = buffer->size;
    section->shdr.sh_size += 16;
    AppendGListNoData(buffer, 16);
    ELF_EMIT_SYMBOL_WORD(symbol->st_name, 0);
    ELF_EMIT_SYMBOL_WORD(symbol->st_value, 4);
    ELF_EMIT_SYMBOL_WORD(symbol->st_size, 8);
    (*buffer->data)[oldsize + 12] = symbol->st_info;
    (*buffer->data)[oldsize + 13] = symbol->st_other;
    {
        UInt16 value = symbol->st_shndx;
        UInt32 position = oldsize + 14;
        char* dest = *buffer->data + position;
        if (elfEndian == 1) {
            dest[0] = value;
            value = (SInt32)value >> 8;
        } else {
            dest[0] = (SInt32)value >> 8;
        }
        dest[1] = value;
    }
}

ElfGenSection* ElfAllocateSection(char* name, UInt32 kind, SInt32 initialSize) {
    ElfGenSection* section;
    register SInt32 i;
    if (elfSectionCount == elfSectionCapacity) {
        ElfGenSection** previous = elfSectionTable;
        elfSectionTable = galloc((elfSectionCapacity + 0x400) * sizeof(ElfGenSection*));
        i = 0;
        while (i < elfSectionCapacity) {
            SInt32 index = i++;
            elfSectionTable[index] = previous[index];
        }
        elfSectionCapacity += 0x400;
    }
    section = galloc(sizeof(ElfGenSection));
    memclrw(section, sizeof(ElfGenSection));
    section->name = name;
    section->index = elfSectionCount;
    section->listtail = &section->list;
    section->shdr.sh_type = kind;
    section->shdr.sh_flags = initialSize;
    if (InitGList(&section->data, 0x200)) {
        CError_NoMem();
    }
    if (kind == 4) {
        section->shdr.sh_entsize = 12;
    } else if (kind == 9) {
        section->shdr.sh_entsize = 8;
    } else if (kind == 2) {
        section->shdr.sh_entsize = 16;
    }
    elfSectionTable[section->index] = section;
    elfSectionCount++;
    return section;
}

void ElfTrackDebugSource(Object* object) {
    PendingObject* entry;
    PendingObject* previous;
    if (copts.unknown13A != 1) {
        return;
    }
    if (!copts.unknown134 || !elfDebugContext) {
        return;
    }
    if (object->name->name[0] == '@') {
        return;
    }
    if (object->type->type == TYPEMEMBERPOINTER) {
        return;
    }
    entry = elfDebugSources;
    previous = entry;
    for (; entry; entry = entry->next) {
        if (entry->object == object) {
            if (previous != entry) {
                previous->next = entry->next;
                entry->next = elfDebugSources;
                elfDebugSources = entry;
            }
            return;
        }
        previous = entry;
    }
    entry = galloc(sizeof(PendingObject));
    entry->object = object;
    entry->next = elfDebugSources;
    elfDebugSources = entry;
}

void ElfEmitSymbolTable(void) {
    ElfGenSymbol* symbol;
    UInt32 count;
    ElfGenSection* strings = elfStringTable;
    elfSymbolTable->shdr.sh_link = strings->index;
    strings->shdr.sh_size++;
    AppendGListData(&strings->data, elfNullString, 1);
    for (symbol = elfSymbolHead; symbol; symbol = symbol->next) {
        const char* name = symbol->name;
        SInt32 len = __builtin_strlen(name) + 1;
        SInt32 offset = strings->shdr.sh_size;
        strings->shdr.sh_size += len;
        AppendGListData(&strings->data, name, len);
        symbol->sym.st_name = offset;
    }
    ElfAppendSymbolData(elfSymbolTable, &elfNullSymbol);
    count = 1;
    for (symbol = elfSymbolHead; symbol; symbol = symbol->next) {
        if (ELF32_ST_BIND(symbol->sym.st_info) == STB_LOCAL) {
            symbol->index = count++;
            ElfAppendSymbolData(elfSymbolTable, &symbol->sym);
        }
    }
    elfSymbolTable->shdr.sh_info = count;
    for (symbol = elfSymbolHead; symbol; symbol = symbol->next) {
        if (ELF32_ST_BIND(symbol->sym.st_info) != STB_LOCAL) {
            symbol->index = count++;
            ElfAppendSymbolData(elfSymbolTable, &symbol->sym);
        }
    }
}

void fn_00541160(Object* object, SInt32 isGlobal, ElfGenSymbol* symbol, SInt32 size) {
    DwarfNode* node;
    ObjGenRegisterLocation location;
    ObjGenRegisterLocation boundLocation;
    UInt16 kind;
    UInt8 tagged;
    if (!copts.unknown134 || !elfDebugContext) {
        return;
    }
    if (object->debugInfo.entry) {
        return;
    }
    if (object->name->name[0] == '@') {
        return;
    }
    if (object->type->type == TYPEMEMBERPOINTER) {
        return;
    }
    fn_0046EDD0(object, &location);
    elfDebugCurrentScope = elfDebugBaseScope;
    kind = isGlobal ? 7 : 12;
    node = NewDwarfNode(kind);
    object->debugInfo.entry = node;
    node->u.var.name = object->name;
    node->u.var.linkname = CMangler_GetLinkName(object);
    node->u.var.type = fn_00544700(object->type);
    boundLocation = location;
    tagged = boundLocation.number == -1 &&
        copts.unknown47 &&
        object->datatype == DDATA &&
        object->u.data.reginfo &&
        (object->u.data.reginfo->reg ||
         object->u.data.reginfo->rclass == 5 ||
         (object->u.data.reginfo->flags && object->u.data.reginfo->rclass == 0));
    if (tagged) {
        UInt8 rclass = object->u.data.reginfo->rclass;
        if (rclass == 0) {
            node->u.var.location = 1;
        } else if (rclass == 1) {
            node->u.var.location = 5;
        } else if (rclass == 5) {
            node->u.var.location = 1;
        } else if (rclass == 6) {
            node->u.var.location = 1;
        }
    } else {
        node->u.var.location = copts.unknown07 && location.number == 28 ? 6 : 3;
    }
    node->u.var.offset = 0;
    node->u.var.symbol = symbol;
    if (elfDebugCurrentScope->lastType) {
        elfDebugCurrentScope->lastType->sibling = node;
    }
    elfDebugCurrentScope->lastType = node;
}

void fn_00540FB0(void) {
    SInt32 i;
    if (!copts.unknown134) {
        return;
    }
    elfDebugScopeChain = NULL;
    elfDebugScopeTail = NULL;
    elfDebugCurrentScope = NULL;
    elfDebugHashRoot = 0;
    elfDebugSources = NULL;
    elfDebugPendingTail = NULL;
    elfDebugPendingCount = NULL;
    elfDebugScopeField = NULL;
    fn_00544810(1);
    elfDebugBaseScope = elfDebugScopeTail;
    elfDebugCurrentScope = elfDebugBaseScope;
    NewDwarfNode(17);
    if (copts.unknown139) {
        COS_FileGetPathName(elfDebugSourceContext, &cparamblkptr->sourcefile, NULL);
    } else {
        COS_FileGetFileSpecInfo(&cparamblkptr->sourcefile, 0, 0x104, elfDebugSourceContext, 0x100);
    }
    i = 0;
    do {
        SInt32 index = i;
        i += 8;
        elfDebugHashTable[index] = NULL;
        elfDebugHashTable[index + 1] = NULL;
        elfDebugHashTable[index + 2] = NULL;
        elfDebugHashTable[index + 3] = NULL;
        elfDebugHashTable[index + 4] = NULL;
        elfDebugHashTable[index + 5] = NULL;
        elfDebugHashTable[index + 6] = NULL;
        elfDebugHashTable[index + 7] = NULL;
    } while (i < 1024);
    elfDebugContext = 1;
}

void fn_00540D20(void) {
    DwarfFunctionState* state;
    DWInfo* info;
    DwarfNode* node;
    DwarfNode* first;
    DwarfNode* function;
    ElfGenSymbol* symbol;
    SInt32 i;
    UInt32 offset;
    if (!copts.unknown134 || !elfDebugContext) {
        return;
    }
    elfDebugCurrentScope = elfDebugBaseScope;
    do {
        elfDebugProcessFlag = FALSE;
        for (i = 0; i < 1024; i++) {
            for (info = elfDebugHashTable[i]; info; info = info->next) {
                if (!elfDebugProcessFlag && !info->marked) {
                    fn_00543A40(info);
                }
            }
        }
    } while (elfDebugProcessFlag);
    if (elfDebugCurrentScope->lastType) {
        elfDebugCurrentScope->lastType->sibling = NewDwarfNode(0xFFFF);
    }
    NameHashExportReset();
    for (state = elfDebugScopeChain; state; state = state->next) {
        elfDebugCurrentScope = state;
        state->symbol->sym.st_value = state->section->data.size;
        first = state->first;
        if (state->next) {
            first->sibling = state->next->first;
        }
        if (state->mode == 2) {
            elfDebugScopeField = first->u.unit.function;
        }
        for (node = state->first; node; node = node->next) {
            fn_005427B0(node);
        }
        for (node = state->first; node; node = node->next) {
            fn_00541F80(node);
        }
        if (state->mode == 2) {
            offset = fn_00541EA0(state);
            function = first->u.unit.function;
            if (function) {
                symbol = function->u.func.symbol;
                *(UInt32*)(*first->scope->section->data.data + first->offset + 0xE) = 0;
                ElfAppendSectionRelocation(2, first->scope->symbol, first->offset + 0xE, symbol, 0);
                symbol = function->u.func.symbol;
                *(UInt32*)(*first->scope->section->data.data + first->offset + 0x14) =
                    function->u.func.size;
                ElfAppendSectionRelocation(
                    2, first->scope->symbol, first->offset + 0x14, symbol, 0
                );
                ElfAppendSectionRelocation(
                    2, state->symbol, first->offset + 0x1A, state->lineSymbol, 0
                );
                ElfAppendSectionRelocation(
                    2, state->lineSymbol, offset + 4, function->u.func.symbol, 0
                );
            }
        }
        symbol = state->symbol;
        *(UInt32*)(*first->scope->section->data.data + first->offset + 8) =
            state->section->data.size;
        ElfAppendSectionRelocation(2, first->scope->symbol, first->offset + 8, symbol, 0);
    }
}

ElfDebugOffset* fn_00541DF0(UInt32 base, SInt32 size) {
    int last = -1;
    int count = 0;
    PCodeBlock* block;
    PCode* pcode;
    ElfDebugOffset* result;
    ElfDebugOffset* current;
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (pcode = block->firstPCode; pcode; pcode = pcode->nextPCode) {
            if (!(pcode->flags & 0x40000000)) {
                if (pcode->sourceoffset > 0 && pcode->sourceoffset != last) {
                    last = pcode->sourceoffset;
                    count++;
                }
            }
        }
    }
    result = galloc((count + 1) * sizeof(ElfDebugOffset));
    result->number = count;
    result->offset = size;
    current = result;
    last = -1;
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (pcode = block->firstPCode; pcode; pcode = pcode->nextPCode) {
            if (!(pcode->flags & 0x40000000)) {
                if (pcode->sourceoffset > 0 && pcode->sourceoffset != last) {
                    last = pcode->sourceoffset;
                    current++;
                    current->number = last;
                    current->offset = pcode->unk1C + base;
                }
            }
        }
    }
    return result;
}

UInt32 fn_00541EA0(DwarfFunctionState* state) {
    DwarfNode* first = state->first;
    DwarfNode* item = first->next;
    ElfDebugOffset* offsets;
    ElfGenSection* section = state->extraSection;
    UInt32 written;
    volatile SInt32 count;
    SInt32 i;
    if (!item || (item->kind != 0x14 && item->kind != 6)) {
        CError_Internal((char*)DWARF_file_name, 0x877);
    }
    offsets = item->u.func.lines;
    count = offsets->number;
    written = elf_append_word(section, (count + 1) * 10 + 8);
    elf_append_word(section, 0);
    for (i = 1; i <= count; i++) {
        elf_append_word(section, offsets[i].number);
        elf_append_half(section, 0xFFFF);
        elf_append_word(section, offsets[i].offset);
    }
    elf_append_word(section, 0);
    elf_append_half(section, 0xFFFF);
    elf_append_word(section, offsets->offset);
    return written;
}

static UInt32 ElfDebugRefAdjustment(DwarfRef* ref, UInt32* size) {
    switch (ref->tag) {
        case 0x55:
            *size = 4;
            return 0;
        case 0x63:
            *size = ref->num + 6;
            return 0;
        case 0x72:
            *size = 6;
            return 2;
        case 0x83:
            *size = ref->num + 8;
            return ref->num + 4;
        default:
            CError_Internal((char*)DWARF_file_name, 0x3D3);
            return 0;
    }
}

static void ElfDebugPatchRef(DwarfNode* node, DwarfRef* ref, UInt32 position) {
    ElfGenSymbol* symbol = ref->node->scope->symbol;
    *(UInt32*)(*node->scope->section->data.data + node->offset + position) = ref->node->offset;
    ElfAppendSectionRelocation(2, node->scope->symbol, node->offset + position, symbol, 0);
}

void fn_00541F80(DwarfNode* node) {
    DwarfRef ref;
    ElfGenSymbol* symbol;
    UInt32 size;
    UInt32 adjustment;
    UInt32 addend;
    UInt32 position;
    PendingObject* pending;
    DwarfPendingRecord* record;
    DwarfNode* entry;
    if (node->kind == 0xFFFF || node->kind == 17) {
        return;
    }
    if (node->sibling) {
        symbol = node->scope->symbol;
        *(UInt32*)(*node->scope->section->data.data + node->offset + 8) = node->sibling->offset;
        ElfAppendSectionRelocation(2, node->scope->symbol, node->offset + 8, symbol, 0);
    }
    switch (node->kind) {
        case 13:
            fn_00544550(node->u.member.type, &ref);
            if ((adjustment = ElfDebugRefAdjustment(&ref, &size))) {
                ElfDebugPatchRef(node, &ref, adjustment + 0xC);
            }
            break;
        case 28:
            fn_00544550(node->u.inheritance.type, &ref);
            if ((adjustment = ElfDebugRefAdjustment(&ref, &size))) {
                ElfDebugPatchRef(node, &ref, adjustment + 0xC);
            }
            break;
        case 1:
            fn_00544550(node->u.array.type, &ref);
            if ((adjustment = ElfDebugRefAdjustment(&ref, &size))) {
                ElfDebugPatchRef(node, &ref, adjustment + 0x20);
            }
            break;
        case 21:
            fn_00544550(node->u.func.type, &ref);
            if ((adjustment = ElfDebugRefAdjustment(&ref, &size))) {
                ElfDebugPatchRef(node, &ref, adjustment + 0xC);
            }
            break;
        case 7:
        case 12:
            fn_00544550(node->u.var.type, &ref);
            if ((adjustment = ElfDebugRefAdjustment(&ref, &size))) {
                ElfDebugPatchRef(node, &ref, adjustment + 0xC);
            }
            symbol = node->u.var.symbol;
            if (!symbol) {
                break;
            }
            switch (node->u.var.location) {
                case 3:
                    addend = 5;
                    break;
                case 4:
                    addend = 10;
                    break;
                default:
                    addend = 0;
                    break;
            }
            if (!addend) {
                break;
            }
            if (node->u.var.location == 3) {
                position = size + 0xC + addend;
                *(UInt32*)(*node->scope->section->data.data + node->offset + position) = 0;
                ElfAppendSectionRelocation(
                    2, node->scope->symbol, node->offset + position, symbol, 0
                );
            } else {
                ElfAppendSectionRelocation(
                    2, node->scope->symbol, node->offset + 0xC + size + addend, symbol, 0
                );
            }
            break;
        case 5:
            fn_00544550(node->u.var.type, &ref);
            if ((adjustment = ElfDebugRefAdjustment(&ref, &size))) {
                ElfDebugPatchRef(node, &ref, adjustment + 0xC);
            }
            if (!node->u.var.name || !(symbol = node->u.var.symbol)) {
                break;
            }
            switch (node->u.var.location) {
                case 3:
                    addend = 5;
                    break;
                case 4:
                    addend = 10;
                    break;
                default:
                    addend = 0;
                    break;
            }
            if (!addend) {
                break;
            }
            position = size + 0xC + addend;
            *(UInt32*)(*node->scope->section->data.data + node->offset + position) = 0;
            ElfAppendSectionRelocation(2, node->scope->symbol, node->offset + position, symbol, 0);
            break;
        case 6:
            if (node->u.func.declaration) {
                break;
            }
        case 20:
            fn_00544550(node->u.func.type, &ref);
            if ((adjustment = ElfDebugRefAdjustment(&ref, &size))) {
                ElfDebugPatchRef(node, &ref, adjustment + 0x18);
            }
            symbol = node->u.func.symbol;
            *(UInt32*)(*node->scope->section->data.data + node->offset + 0xE) = 0;
            ElfAppendSectionRelocation(2, node->scope->symbol, node->offset + 0xE, symbol, 0);
            symbol = node->u.func.symbol;
            *(UInt32*)(*node->scope->section->data.data + node->offset + 0x14) = node->u.func.size;
            ElfAppendSectionRelocation(2, node->scope->symbol, node->offset + 0x14, symbol, 0);
            position = 0;
            for (pending = elfDebugCurrentScope->pendingObjects; pending; pending = pending->next) {
                UInt32 offset;
                entry = pending->object->debugInfo.entry;
                symbol = entry->scope->symbol;
                offset = size + 0x1C + position;
                *(UInt32*)(*node->scope->section->data.data + node->offset + offset) =
                    entry->offset;
                ElfAppendSectionRelocation(
                    2, node->scope->symbol, node->offset + offset, symbol, 0
                );
                position += 4;
            }
            if (elfDebugCurrentScope->pendingObjects) {
                position += 4;
            }
            if (elfDebugScopeField) {
                for (record = elfDebugCurrentScope->pendingRecords; record; record = record->next) {
                    UInt32 offset;
                    symbol = elfDebugScopeField->u.func.symbol;
                    offset = size + 0x1C + position;
                    *(UInt32*)(*node->scope->section->data.data + node->offset + offset) =
                        record->unk0;
                    ElfAppendSectionRelocation(
                        2, node->scope->symbol, node->offset + offset, symbol, 0
                    );
                    position += 6;
                }
            }
            elfDebugScopeField = NULL;
            break;
        case 11:
            symbol = node->u.block.symbol;
            *(UInt32*)(*node->scope->section->data.data + node->offset + 0xE) = node->u.block.low;
            ElfAppendSectionRelocation(2, node->scope->symbol, node->offset + 0xE, symbol, 0);
            symbol = node->u.block.symbol;
            *(UInt32*)(*node->scope->section->data.data + node->offset + 0x14) = node->u.block.high;
            ElfAppendSectionRelocation(2, node->scope->symbol, node->offset + 0x14, symbol, 0);
            break;
    }
}

void fn_00541380(Object* function, SInt32 isGlobal, ElfGenSymbol* symbol, SInt32 size) {
    DwarfNode* node;
    DwarfNode* entry;
    DwarfNode* last;
    DwarfNode* initialNode;
    DwarfNode* block;
    DwarfFunctionState* state;
    ObjectList* list;
    Object* object;
    VarInfo* info;
    Type* type;
    char* name1;
    char* name2;
    PendingObject* pending;
    SInt32 amount;
    SInt32 count;
    SInt32 align;
    UInt16 kind;
    ObjGenRegisterLocation location;
    if (!copts.unknown134 || !elfDebugContext) {
        return;
    }
    fn_00544810(2);
    state = elfDebugScopeTail;
    elfDebugCurrentScope = state;
    for (pending = elfDebugSources; pending; pending = pending->next) {
        ElfGenSymbol* objectSymbol;
        CMangler_GetLinkName(pending->object);
        objectSymbol = fn_0046EA30(pending->object);
        fn_00541160(pending->object, 1, objectSymbol, pending->object->type->size);
    }
    elfDebugCurrentScope = state;
    elfDebugCurrentScope->pendingObjects = elfDebugSources;
    elfDebugCurrentScope->pendingRecords = elfDebugPendingTail;
    elfDebugSources = NULL;
    elfDebugPendingTail = NULL;
    elfDebugPendingCount = NULL;
    node = NewDwarfNode(17);
    initialNode = node;
    kind = isGlobal ? 6 : 20;
    node = NewDwarfNode(kind);
    node->u.func.name = function->name;
    node->u.func.declaration = 0;
    node->u.func.linkname = CMangler_GetLinkName(function);
    node->u.func.type = fn_00544700(((TypeFunc*)function->type)->functype);
    node->u.func.symbol = symbol;
    node->u.func.size = size;
    node->u.func.lines = fn_00541DF0(0, size);
    node->u.func.returnOperand = be_macro_save_header_625300;
    node->u.func.registers[0] = be_macro_gp_save_records_625AA8;
    node->u.func.registers[1] = be_macro_fp_single_save_records_621280;
    node->u.func.registers[2] = be_macro_fp_save_records_621080;
    name1 = galloc(__builtin_strlen(node->u.func.linkname->name) + 8);
    strcpy(name1, ".line_");
    strcpy(name1 + 6, node->u.func.linkname->name);
    name2 = galloc(__builtin_strlen(node->u.func.linkname->name) + 9);
    strcpy(name2, ".debug_");
    strcpy(name2 + 7, node->u.func.linkname->name);
    elfDebugCurrentScope->symbol =
        ElfNewSymbolRecord(name2, elfDebugCurrentScope->section->index, 0, 3, 0, 0);
    elfDebugCurrentScope->lineSymbol =
        ElfNewSymbolRecord(name1, elfDebugCurrentScope->extraSection->index, 0, 3, 0, 0);
    amount = 0;
    count = 0;
    last = NULL;
    for (list = ig_reglist_a; list; list = list->next) {
        object = list->object;
        type = object->type;
        align = type->type == TYPEFLOAT && type->size == 8 ? 8 : 4;
        amount += type->size;
        amount = (amount + align - 1) & -align;
        info = object->u.var.info;
        if (info->used && object->name->name[0] != '@') {
            entry = NewDwarfNode(5);
            entry->u.var.name = object->name;
            entry->u.var.type = fn_00544700(object->type);
            if (info->regHi) {
                entry->u.var.location = 2;
                entry->u.var.reg2 = info->regHi;
                entry->u.var.reg = info->reg;
            } else if (info->reg) {
                if (info->rclass == 0) {
                    entry->u.var.location = 1;
                } else {
                    if (info->rclass != 1) {
                        CError_Internal((char*)DWARF_file_name, 0x94E);
                    }
                    entry->u.var.location = 5;
                }
                entry->u.var.reg = info->reg;
            } else {
                entry->u.var.location = 4;
                entry->u.var.reg = stack_pointer_register;
                entry->u.var.offset = fn_004AFC70(object);
                entry->u.var.symbol = NULL;
            }
            count++;
            if (last) {
                last->sibling = entry;
            }
            last = entry;
        }
    }
    entry = NewDwarfNode(11);
    block = entry;
    entry->u.block.symbol = symbol;
    entry->u.block.low = 0;
    entry->u.block.high = size;
    if (last) {
        last->sibling = entry;
    }
    last = NULL;
    for (list = ig_reglist_b; list; list = list->next) {
        object = list->object;
        info = object->u.var.info;
        if (!info->used || object->name->name[0] == '@') {
            continue;
        }
        entry = NewDwarfNode(12);
        entry->u.var.name = object->name;
        entry->u.var.type = fn_00544700(object->type);
        if (info->regHi) {
            entry->u.var.location = 2;
            entry->u.var.reg2 = info->regHi;
            entry->u.var.reg = info->reg;
        } else if (info->reg) {
            if (info->rclass == 0) {
                entry->u.var.location = 1;
            } else {
                if (info->rclass != 1) {
                    CError_Internal((char*)DWARF_file_name, 0x982);
                }
                entry->u.var.location = 5;
            }
            entry->u.var.reg = info->reg;
        } else {
            entry->u.var.location = 4;
            entry->u.var.reg = stack_pointer_register;
            entry->u.var.offset = fn_004AFC70(object);
            entry->u.var.symbol = NULL;
        }
        if (last) {
            last->sibling = entry;
        }
        last = entry;
    }
    for (list = dollar_name_objects; list; list = list->next) {
        object = list->object;
        if (object->debugInfo.entry) {
            continue;
        }
        entry = NewDwarfNode(12);
        object->debugInfo.entry = entry;
        if (copts.unknown07) {
            fn_0046EDD0(list->object, &location);
        }
        entry->u.var.name = object->name;
        entry->u.var.linkname = CMangler_GetLinkName(object);
        entry->u.var.type = fn_00544700(object->type);
        entry->u.var.location = copts.unknown07 && location.number == 28 ? 6 : 3;
        entry->u.var.offset = 0;
        entry->u.var.symbol = fn_0046EA30(object);
        if (last) {
            last->sibling = entry;
        }
        last = entry;
    }
    dollar_name_objects = NULL;
    if (last) {
        last->sibling = NewDwarfNode(0xFFFF);
    }
    block->sibling = NewDwarfNode(0xFFFF);
    node->sibling = NewDwarfNode(0xFFFF);
    node->u.func.frameSize = amount;
    node->u.func.frameOffset = asm_stack_frame_offset_636E78;
    node->u.func.paramCount = count;
    initialNode->u.unit.function = node;
}

ElfGenSection* fn_0053F810(UInt16 index) {
    return elfSectionTable[index];
}

void fn_0053F820(SInt32* writableSize, SInt32* allocatedSize, SInt32* uninitializedSize) {
    SInt32 i;
    ElfGenSection* section;
    UInt32 flags;
    SInt32 sectionCount;

    *writableSize = 0;
    *allocatedSize = 0;
    *uninitializedSize = 0;
    i = 1;
    sectionCount = elfSectionCount;
    if (sectionCount > 1) {
        do {
            section = elfSectionTable[i];
            flags = section->shdr.sh_flags;
            if (flags & 4) {
                *writableSize += section->shdr.sh_size;
            } else if (section->shdr.sh_type == 8 && (flags & 2)) {
                *uninitializedSize += section->shdr.sh_size;
            } else if (section->shdr.sh_type == 1 && (flags & 2)) {
                *allocatedSize += section->shdr.sh_size;
            }
            i++;
        } while (i < *(volatile UInt16*)&elfSectionCount);
    }
}

void fn_0053F8B0(void) {
    SInt32 index = 1;
    if ((SInt32)elfSectionCount > 1) {
        do {
            FreeGList(&elfSectionTable[index]->data);
            index++;
        } while (index < elfSectionCount);
    }
}

void fn_00540300(ElfGenSection* section) {
    struct ElfGenRelocation* entry;
    for (entry = section->list; entry; entry = entry->next) {
        entry->info = ((ElfGenSymbol*)entry->target)->index * 256 + (UInt8)entry->info;
        fn_00540340(section, &entry->offset);
    }
}

void fn_00540340(ElfGenSection* section, void* recordArg) {
    GList* gl;
    SInt32 size;
    SInt32 offset;
    SInt32 type;
    UInt32* record = recordArg;
    UInt32 word;
    UInt8* target;
    SInt32 nextOffset;

    gl = &section->data;
    type = section->shdr.sh_type;
    offset = gl->size;
    if (type == SHT_RELA) {
        size = 12;
    } else {
        size = 8;
    }
    section->shdr.sh_size += size;
    AppendGListNoData(gl, size);
#define STORE_RECORD_WORD(N) \
    target = (UInt8*)*gl->data + offset + (N) * 4; \
    word = record[(N)]; \
    if (elfEndian == 1) { \
        target[0] = word; \
        target[1] = word >> 8; \
        target[2] = word >> 16; \
        target[3] = word >> 24; \
    } else { \
        target[0] = word >> 24; \
        target[1] = word >> 16; \
        target[2] = word >> 8; \
        target[3] = word; \
    }
    STORE_RECORD_WORD(0)
    nextOffset = offset + 4;
    target = (UInt8*)*gl->data + nextOffset;
    word = record[1];
    if (elfEndian == 1) {
        target[0] = word;
        target[1] = word >> 8;
        target[2] = word >> 16;
        target[3] = word >> 24;
    } else {
        target[0] = word >> 24;
        target[1] = word >> 16;
        target[2] = word >> 8;
        target[3] = word;
    }
    if (type == 4) {
        nextOffset = offset + 8;
        target = (UInt8*)*gl->data + nextOffset;
        word = record[2];
        if (elfEndian == 1) {
            target[0] = word;
            target[1] = word >> 8;
            target[2] = word >> 16;
            target[3] = word >> 24;
        } else {
            target[0] = word >> 24;
            target[1] = word >> 16;
            target[2] = word >> 8;
            target[3] = word;
        }
    }
#undef STORE_RECORD_WORD
}

void fn_00540200(UInt16 machine, UInt32 flags, SInt32 endian) {
    elfEndian = endian;
    elfHeader_636078.e_ident[0] = 0x7F;
    elfHeader_636078.e_ident[1] = 'E';
    elfHeader_636078.e_ident[2] = 'L';
    elfHeader_636078.e_ident[3] = 'F';
    elfHeader_636078.e_ident[4] = 1;
    elfHeader_636078.e_ident[5] = endian;
    elfHeader_636078.e_ident[6] = 1;
    elfHeader_636078.e_type = 1;
    elfHeader_636078.e_machine = machine;
    elfHeader_636078.e_version = 1;
    elfHeader_636078.e_flags = flags;
    elfHeader_636078.e_ehsize = sizeof(Elf32_Ehdr);
    elfHeader_636078.e_shentsize = sizeof(Elf32_Shdr);
    elfSectionCount = 0;
    elfSectionCapacity = 0;
    elfSectionTable = NULL;
    elfSymbolHead = NULL;
    elfSymbolListTail = &elfSymbolHead;
    ElfAllocateSection(&elfNameEmpty_5EA133, 0, 0);
    elfSymbolTable = ElfAllocateSection(&elfNameSymtab_5EA134, 2, 0);
    elfStringTable = ElfAllocateSection(&elfNameStrtab_5EA13C, 3, 0);
    elfSectionNameTable_61C19C = ElfAllocateSection(&elfNameShstrtab_5EA144, 3, 0);
}

void fn_0053FB50(GList* gl, const Elf32_Ehdr* hdr) {
    SInt32 i = 0;
    SInt32 index;
    UInt16 half;
    UInt32 word;
    UInt8* target;
    do {
        index = i;
        (*gl->data)[index] = hdr->e_ident[index];
        index = i + 1;
        (*gl->data)[index] = hdr->e_ident[index];
        index = i + 2;
        (*gl->data)[index] = hdr->e_ident[index];
        index = i + 3;
        (*gl->data)[index] = hdr->e_ident[index];
        index = i + 4;
        (*gl->data)[index] = hdr->e_ident[index];
        index = i + 5;
        (*gl->data)[index] = hdr->e_ident[index];
        index = i + 6;
        (*gl->data)[index] = hdr->e_ident[index];
        index = i + 7;
        (*gl->data)[index] = hdr->e_ident[index];
        i += 8;
    } while (i < 16);
#define WRITE_HEADER_WORD(N, F) \
    target = (UInt8*)*gl->data + (N); \
    word = hdr->F; \
    if (elfEndian == 1) { \
        target[0] = word; \
        target[1] = word >> 8; \
        target[2] = word >> 16; \
        target[3] = word >> 24; \
    } else { \
        target[0] = word >> 24; \
        target[1] = word >> 16; \
        target[2] = word >> 8; \
        target[3] = word; \
    }
#define WRITE_HEADER_HALF(N, F) \
    target = (UInt8*)*gl->data + (N); \
    half = hdr->F; \
    if (elfEndian == 1) { \
        target[0] = half; \
        target[1] = half >> 8; \
    } else { \
        target[0] = half >> 8; \
        target[1] = half; \
    }
    WRITE_HEADER_HALF(0x10, e_type)
    WRITE_HEADER_HALF(0x12, e_machine)
    WRITE_HEADER_WORD(0x14, e_version)
    WRITE_HEADER_WORD(0x18, e_entry)
    WRITE_HEADER_WORD(0x1C, e_phoff)
    WRITE_HEADER_WORD(0x20, e_shoff)
    WRITE_HEADER_WORD(0x24, e_flags)
    WRITE_HEADER_HALF(0x28, e_ehsize)
    WRITE_HEADER_HALF(0x2A, e_phentsize)
    WRITE_HEADER_HALF(0x2C, e_phnum)
    WRITE_HEADER_HALF(0x2E, e_shentsize)
    WRITE_HEADER_HALF(0x30, e_shnum)
    WRITE_HEADER_HALF(0x32, e_shstrndx)
#undef WRITE_HEADER_WORD
#undef WRITE_HEADER_HALF
}

void fn_0053FF20(GList* gl, const Elf32_Shdr* shdr) {
    SInt32 offset = gl->size;
    UInt8* target;
    UInt32 word;
    AppendGListNoData(gl, 0x28);
#define ELF_WRITE_WORD(N, F) \
    target = (UInt8*)*gl->data + offset + (N) * 4; \
    word = shdr->F; \
    if (elfEndian == 1) { \
        target[0] = word; \
        target[1] = word >> 8; \
        target[2] = word >> 16; \
        target[3] = word >> 24; \
    } else { \
        target[0] = word >> 24; \
        target[1] = word >> 16; \
        target[2] = word >> 8; \
        target[3] = word; \
    }
    ELF_WRITE_WORD(0, sh_name)
    ELF_WRITE_WORD(1, sh_type)
    ELF_WRITE_WORD(2, sh_flags)
    ELF_WRITE_WORD(3, sh_addr)
    ELF_WRITE_WORD(4, sh_offset)
    ELF_WRITE_WORD(5, sh_size)
    ELF_WRITE_WORD(6, sh_link)
    ELF_WRITE_WORD(7, sh_info)
    ELF_WRITE_WORD(8, sh_addralign)
    ELF_WRITE_WORD(9, sh_entsize)
#undef ELF_WRITE_WORD
}
