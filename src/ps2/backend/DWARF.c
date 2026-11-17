#include <string.h>
#include "compiler/CParser.h"
#include "compiler/DWARF.h"
#include "compiler/CError.h"
#include "compiler/CFunc.h"
#include "compiler/CMangler.h"
#include "compiler/CompilerTools.h"
#include "compiler/Registers.h"
#include "compiler/objects.h"
#include "compiler/types.h"
#include "stdio.h"
#include "string.h"

void fn_00544810(SInt32 mode) {
    DwarfFunctionState* state = galloc(sizeof(DwarfFunctionState));
    memclrw(state, sizeof(DwarfFunctionState));
    state->mode = mode;
    state->section = ElfAllocateSection(elfDebugLineSectionName, 0x70000005, 0);
    state->relocations = ElfAllocateSection(elfDebugLineRelocName, 9, 0);
    ElfLinkRelocation(state->section, state->relocations);
    state->section->shdr.sh_addralign = 0;
    if (mode == 2) {
        state->extraSection = ElfAllocateSection(elfDebugExtraSectionName, 0x70000005, 0);
        state->extraRelocations = ElfAllocateSection(elfDebugExtraRelocName, 9, 0);
        ElfLinkRelocation(state->extraSection, state->extraRelocations);
        state->extraSection->shdr.sh_addralign = 0;
    } else {
        state->symbol =
            ElfNewSymbolRecord(elfDebugAnonymousSymbol, state->section->index, 0, 3, 0, 0);
    }
    state->source = register_code_context;
    if (elfDebugScopeTail) {
        elfDebugScopeTail->next = state;
    } else {
        elfDebugScopeChain = state;
    }
    elfDebugScopeTail = state;
    state->pending = FALSE;
}

void fn_00544550(DWInfo* info, DwarfRef* ref) {
    SInt32 fundamental;
    DwarfNode* referenced = NULL;
    Type* type = info->type;
    UInt16 count;
    ref->num = 0;
    for (;;) {
        switch ((SInt8)type->type) {
            case TYPEPOINTER:
                fundamental = TYPE_POINTER(type)->qual & Q_REFERENCE ? 2 : 1;
                ref->modifiers[ref->num++] = fundamental;
                type = TYPE_POINTER(type)->target;
                if (type->type == TYPEFUNC) {
                    referenced = info->node;
                }
                info = fn_00544700(type);
                continue;
            case TYPEVOID:
            case TYPEMEMBERPOINTER:
                ref->tag = 0x55;
                ref->fundamental = 20;
                ref->size = 4;
                break;
            case TYPEINT:
            case TYPEFLOAT:
                switch (TYPE_INTEGRAL(type)->integral) {
                    case 0:
                        fundamental = 3;
                        break;
                    case 1:
                        fundamental = 1;
                        break;
                    case 2:
                        fundamental = 2;
                        break;
                    case 3:
                        fundamental = 3;
                        break;
                    case 4:
                        fundamental = 6;
                        break;
                    case 5:
                        fundamental = 5;
                        break;
                    case 6:
                        fundamental = 6;
                        break;
                    case 7:
                        fundamental = 8;
                        break;
                    case 8:
                        fundamental = 9;
                        break;
                    case 9:
                        fundamental = 11;
                        break;
                    case 10:
                        fundamental = 12;
                        break;
                    case 11:
                        fundamental = 0x8108;
                        break;
                    case 12:
                        fundamental = 0x8208;
                        break;
                    case 13:
                        fundamental = 14;
                        break;
                    case 14:
                        fundamental = 15;
                        break;
                    case 15:
                        fundamental = 15;
                        break;
                    case 16:
                        fundamental = 15;
                        break;
                    case 17:
                        fundamental = 0xA408;
                        break;
                    case 18:
                        fundamental = 0xA510;
                        break;
                    default:
                        CError_Internal((char*)DWARF_file_name, 0x14B);
                        break;
                }
                ref->tag = 0x55;
                ref->fundamental = fundamental;
                ref->size = 4;
                break;
            case TYPEENUM:
            case TYPESTRUCT:
            case TYPECLASS:
            case TYPEFUNC:
            case TYPEARRAY:
                ref->tag = 0x72;
                ref->node = referenced ? referenced : info->node;
                ref->size = 6;
                break;
            default:
                CError_Internal((char*)DWARF_file_name, 0x164);
                break;
        }
        break;
    }
    count = ref->num;
    if (count) {
        ref->size += count + 2;
        ref->tag = ref->tag == 0x55 ? 0x63 : 0x83;
    }
}

DWInfo* fn_00544700(Type* type) {
    UInt32 index = ((UInt32)type >> 2) & 0x3FF;
    DWInfo* info;
    StructMember* member;
    ClassList* base;
    ObjMemberVar* ivar;
    FuncArg* arg;
    for (info = elfDebugHashTable[index]; info; info = info->next) {
        if (info->type == type) {
            return info;
        }
    }
    info = galloc(sizeof(DWInfo));
    info->next = elfDebugHashTable[index];
    elfDebugHashTable[index] = info;
    info->type = type;
    info->marked = FALSE;
    info->node = NULL;
    elfDebugProcessFlag = TRUE;
    switch ((SInt8)type->type) {
        case TYPESTRUCT:
            for (member = TYPE_STRUCT(type)->members; member; member = member->next) {
                fn_00544700(member->type);
            }
            break;
        case TYPECLASS:
            for (base = TYPE_CLASS(type)->bases; base; base = base->next) {
                fn_00544700(TYPE(base->base));
            }
            for (ivar = TYPE_CLASS(type)->ivars; ivar; ivar = ivar->next) {
                fn_00544700(ivar->type);
            }
            break;
        case TYPEFUNC:
            fn_00544700(TYPE_FUNC(type)->functype);
            for (arg = TYPE_FUNC(type)->args; arg && arg != &elipsis && arg != &oldstyle;
                 arg = arg->next) {
                fn_00544700(arg->type);
            }
            break;
        case TYPEBITFIELD:
            fn_00544700(TYPE_BITFIELD(type)->bitfieldtype);
            break;
        case TYPEPOINTER:
            fn_00544700(TYPE_POINTER(type)->target);
            break;
        case TYPEARRAY:
            fn_00544700(TYPE_POINTER(type)->target);
            break;
        case TYPEVOID:
        case TYPEINT:
        case TYPEFLOAT:
        case TYPEENUM:
        case TYPETEMPLDEP:
        case TYPETEMPLATE:
        case TYPEMEMBERPOINTER:
        default:
            break;
    }
    return info;
}

UInt32 fn_005438C0(DwarfRef* ref) {
    UInt32 size;
    UInt32 i;
    UInt16 tag = ref->tag;
    elf_append_half(elfDebugCurrentScope->section, tag);
    size = 2;
    switch (ref->tag) {
        case 0x55:
            elf_append_half(elfDebugCurrentScope->section, ref->fundamental);
            return 4;
        case 0x72:
            elf_append_word(elfDebugCurrentScope->section, 0);
            return 6;
        case 0x63:
            elf_append_half(elfDebugCurrentScope->section, (UInt16)(ref->num + 2));
            size += 2;
            for (i = 0; i < ref->num; i++) {
                elf_append_byte(elfDebugCurrentScope->section, ref->modifiers[i]);
            }
            size += i;
            elf_append_half(elfDebugCurrentScope->section, ref->fundamental);
            return size + 2;
        case 0x83:
            elf_append_half(elfDebugCurrentScope->section, (UInt16)(ref->num + 4));
            size += 2;
            for (i = 0; i < ref->num; i++) {
                elf_append_byte(elfDebugCurrentScope->section, ref->modifiers[i]);
            }
            size += i;
            elf_append_word(elfDebugCurrentScope->section, 0);
            return size + 4;
        default:
            CError_Internal((char*)DWARF_file_name, 0x3AD);
            return 0;
    }
}

DwarfNode* fn_00544170(TypeClass* tclass) {
    ClassList* base;
    VClassList* vbase;
    DwarfNode* node;
    DwarfNode* previous = NULL;
    for (base = tclass->bases; base; base = base->next) {
        node = NewDwarfNode(0x1C);
        node->u.inheritance.offset = base->offset;
        node->u.inheritance.type = fn_00544700(TYPE(base->base));
        switch (base->access) {
            case 0:
                node->u.inheritance.access = 0x288;
                break;
            case 1:
                node->u.inheritance.access = 0x248;
                break;
            case 2:
                node->u.inheritance.access = 0x268;
                break;
        }
        if (base->is_virtual) {
            node->u.inheritance.isVirtual = 8;
        }
        if (previous) {
            previous->sibling = node;
        }
        previous = node;
        if (!base->is_virtual) {
            continue;
        }
        for (vbase = tclass->vbases; vbase; vbase = vbase->next) {
            if (!strcmp(base->base->classname->name, vbase->base->classname->name)) {
                break;
            }
        }
        node = NewDwarfNode(0xD);
        node->u.member.name = base->base->classname;
        node->u.member.offset = vbase->offset;
        node->u.member.unk34 = TRUE;
        node->u.member.access = 0x248;
        node->u.member.type = fn_00544700(TYPE(base->base));
        previous->sibling = node;
        previous = node;
    }
    return previous;
}

DwarfNode* fn_00543EF0(TypeClass* tclass, Boolean includeBases, SInt32 baseOffset) {
    ObjMemberVar* ivar;
    DwarfNode* node;
    DwarfNode* previous = NULL;
    DwarfNode* first = NULL;
    Type* memberType;
    if (includeBases) {
        previous = fn_00544170(tclass);
    }
    for (ivar = tclass->ivars; ivar; ivar = ivar->next) {
        if (ivar->name == vptr_name_node) {
            continue;
        }
        if (ivar->otype == OT_MEMBERVAR) {
            node = NewDwarfNode(0xD);
            node->u.member.name = ivar->name;
            node->u.member.offset = ivar->offset + baseOffset;
            node->u.member.unk34 = 1;
            switch (ivar->access) {
                case 0:
                    node->u.member.access = 0x288;
                    break;
                case 1:
                    node->u.member.access = 0x248;
                    break;
                case 2:
                    node->u.member.access = 0x268;
                    break;
            }
            memberType = ivar->type;
            if (memberType->type == TYPEBITFIELD) {
                node->u.member.type = fn_00544700(TYPE_BITFIELD(memberType)->bitfieldtype);
                node->u.member.bitOffset = TYPE_BITFIELD(memberType)->offset;
                node->u.member.bitSize = TYPE_BITFIELD(memberType)->bitlength;
            } else {
                node->u.member.type = fn_00544700(memberType);
            }
        } else {
            node = NewDwarfNode(6);
            node->u.func.name = ivar->name;
            node->u.func.access = (tclass->flags & CLASS_ABSTRACT) ? 0x298 : 0x308;
            node->u.func.type = fn_00544700(TYPE_FUNC(ivar->type)->functype);
            node->u.func.declaration = 1;
        }
        if (previous) {
            previous->sibling = node;
        } else {
            first = node;
        }
        previous = node;
    }
    if (previous && includeBases) {
        node = NewDwarfNode(0xFFFF);
        previous->sibling = node;
    }
    return first;
}

DwarfNode* fn_00543D40(TypeFunc* tfunc) {
    DwarfNode* root = NewDwarfNode(0x15);
    DwarfNode* child;
    FuncArg* arg;
    DwarfNode* last;
    root->u.func.type = fn_00544700(tfunc->functype);
    root->u.func.paramCount = 0;
    last = NULL;
    for (arg = tfunc->args; arg; arg = arg->next) {
        if (arg == &elipsis || arg == &oldstyle) {
            break;
        }
        root->u.func.paramCount++;
        child = NewDwarfNode(5);
        child->u.var.type = fn_00544700(arg->type);
        if (last) {
            last->sibling = child;
        }
        last = child;
    }
    if (last) {
        last->sibling = NewDwarfNode(0xFFFF);
    }
    return root;
}

void fn_00543A40(DWInfo* info) {
    Type* type = info->type;
    Type* element;
    DwarfNode* node;
    DwarfFunctionState* scope;
    DwarfNode* tail;
    ObjEnumConst* enumconst;
    char text[32];
restart:
    if (info->marked) {
        return;
    }
    info->marked = 1;
    switch ((SInt8)type->type) {
        case TYPEPOINTER:
            type = TYPE_POINTER(type)->target;
            info = fn_00544700(type);
            goto restart;
        case TYPEARRAY:
            element = TYPE_POINTER(type)->target;
            node = NewDwarfNode(1);
            node->u.array.type = fn_00544700(element);
            node->u.array.size = type->size;
            if (element->size) {
                node->u.array.bound = type->size / element->size - 1;
            } else {
                node->u.array.bound = 0;
            }
            info->node = node;
            break;
        case TYPEENUM:
            node = NewDwarfNode(4);
            node->u.type.name = TYPE_ENUM(type)->enumname;
            if (!node->u.type.name) {
                sprintf(text, dwarfAnonymousFormat, elfDebugHashRoot);
                elfDebugHashRoot++;
                node->u.type.name = GetHashNameNodeExport(text);
            }
            node->u.type.size = TYPE_ENUM(type)->enumtype->size;
            node->u.type.listSize = 0;
            node->u.type.members = TYPE_ENUM(type)->enumlist;
            for (enumconst = TYPE_ENUM(type)->enumlist; enumconst; enumconst = enumconst->next) {
                char* name = enumconst->name->name;
                UInt32 n = 0;
                while (name[n]) {
                    n++;
                }
                node->u.type.listSize += n + 1;
            }
            info->node = node;
            break;
        case TYPESTRUCT:
            info->node = fn_00544330(TYPE_STRUCT(type));
            break;
        case TYPECLASS:
            node = NewDwarfNode(2);
            node->u.type.name = TYPE_CLASS(type)->classname;
            if (!node->u.type.name) {
                sprintf(text, dwarfAnonymousFormat, elfDebugHashRoot);
                elfDebugHashRoot++;
                node->u.type.name = GetHashNameNodeExport(text);
            }
            node->u.type.size = type->size;
            fn_00543EF0(TYPE_CLASS(type), TRUE, 0);
            info->node = node;
            break;
        case TYPEFUNC:
            info->node = fn_00543D40(TYPE_FUNC(type));
            break;
        default:
            break;
    }
    node = info->node;
    if (node) {
        scope = elfDebugCurrentScope;
        tail = scope->lastType;
        if (tail) {
            tail->sibling = node;
        }
        scope->lastType = node;
    }
}

DwarfNode* fn_00544330(TypeStruct* tstruct) {
    DwarfNode* node;
    StructMember* member;
    DwarfNode* child;
    DwarfNode* last = NULL;
    HashNameNode* name;
    UInt32 anonymousNumber;
    UInt16 kind = tstruct->stype == STRUCT_TYPE_UNION ? 0x17 : 0x13;
    char text[24];
    node = NewDwarfNode(kind);
    anonymousNumber = elfDebugHashRoot;
    name = tstruct->name;
    if (!name) {
        elfDebugHashRoot++;
        sprintf(text, dwarfAnonymousFormat, anonymousNumber);
        name = GetHashNameNodeExport(text);
    }
    node->u.type.name = name;
    node->u.type.size = tstruct->size;
    for (member = tstruct->members; member; member = member->next) {
        child = NewDwarfNode(0xD);
        child->u.member.name = member->name;
        child->u.member.offset = member->offset;
        if (member->type->type == TYPEBITFIELD) {
            TypeBitfield* bitfield = TYPE_BITFIELD(member->type);
            child->u.member.type = fn_00544700(bitfield->bitfieldtype);
            child->u.member.bitOffset = bitfield->offset;
            child->u.member.bitSize = bitfield->bitlength;
        } else {
            child->u.member.type = fn_00544700(member->type);
        }
        if (last) {
            last->sibling = child;
        }
        last = child;
    }
    if (last) {
        child = NewDwarfNode(0xFFFF);
        last->sibling = child;
    }
    return node;
}

UInt32 fn_005436E0(DwarfNode* node) {
    ElfGenSection* section;
    UInt32 size;
    DwarfNode* data;
    section = elfDebugCurrentScope->section;
    data = node;
    elf_append_half(section, 0x23);
    size = 2;
    switch (data->u.var.location) {
        case 3:
            elf_append_half(section, 5);
            elf_append_byte(section, 3);
            elf_append_word(section, 0);
            return 9;
        case 1:
            elf_append_half(section, 5);
            elf_append_byte(section, 1);
            elf_append_word(section, data->u.var.reg);
            return 9;
        case 5:
            elf_append_half(section, 5);
            elf_append_byte(section, 0x80);
            elf_append_word(section, data->u.var.reg);
            return 9;
        case 2:
            elf_append_half(section, 10);
            elf_append_byte(section, 1);
            elf_append_word(section, data->u.var.reg2);
            elf_append_byte(section, 1);
            elf_append_word(section, data->u.var.reg);
            return 14;
        case 4:
            elf_append_half(section, data->u.var.symbol || data->u.var.offset ? 11 : 5);
            elf_append_byte(section, 2);
            elf_append_word(section, data->u.var.reg);
            size += 7;
            if (data->u.var.symbol || data->u.var.offset) {
                elf_append_byte(section, 4);
                elf_append_word(section, data->u.var.symbol ? 0 : data->u.var.offset);
                elf_append_byte(section, 7);
                size += 6;
            }
            return size;
        case 6:
            elf_append_half(section, 11);
            elf_append_byte(section, 2);
            elf_append_word(section, 0x1C);
            elf_append_byte(section, 4);
            elf_append_word(section, 0);
            elf_append_byte(section, 7);
            return 15;
        default:
            CError_Internal((char*)DWARF_file_name, 0x429);
            return 0;
    }
}

UInt32 fn_00543520(DwarfLocationOperand* operand, UInt16 attribute, UInt32 mode) {
    ElfGenSection* section;
    UInt32 size;
    DwarfLocationOperand* data;
    data = operand;
    section = elfDebugCurrentScope->section;
    if (data->kind == 0) {
        return 0;
    }
    elf_append_half(section, attribute);
    size = 2;
    switch (data->kind) {
        case 4: {
            UInt8 op;
            elf_append_half(section, data->offset ? 12 : 6);
            elf_append_byte(section, 2);
            elf_append_word(section, data->base);
            size += 7;
            if (data->offset) {
                elf_append_byte(section, 4);
                elf_append_word(section, data->offset);
                elf_append_byte(section, 7);
                size += 6;
            }
            if (mode == 4) {
                op = 6;
            } else {
                op = 0x82;
            }
            elf_append_byte(section, op);
            return size + 1;
        }
        case 1:
            elf_append_half(section, data->offset ? 11 : 5);
            elf_append_byte(section, 1);
            elf_append_word(section, data->reg);
            size += 7;
            if (data->offset) {
                elf_append_byte(section, 4);
                elf_append_word(section, data->offset);
                elf_append_byte(section, 7);
                size += 6;
            }
            return size;
        case 2:
            elf_append_half(section, 5);
            elf_append_byte(section, 0x80);
            elf_append_word(section, data->reg);
            return 9;
        case 3:
            elf_append_half(section, 5);
            elf_append_byte(section, 0x81);
            elf_append_word(section, data->reg);
            return 9;
        default:
            CError_Internal((char*)DWARF_file_name, 0x47B);
            return 0;
    }
}
