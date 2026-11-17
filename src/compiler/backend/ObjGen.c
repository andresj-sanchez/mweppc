#include "compiler/RegisterInfo.h"
#include "compiler/common.h"
#include "compiler/CDecl.h"
#include "compiler/ObjGen.h"
#include "compiler/CMachine.h"
#include "compiler/types.h"
#include "compiler/CompilerTools.h"
#include "compiler/objects.h"
#include "compiler/CParser.h"
#include "compiler/CError.h"
#include "compiler/CInit.h"
#include "compiler/CMangler.h"
#include "compiler/CPrep.h"
#include "compiler/ELFgen.h"
#include "compiler/IrOptimizer.h"
#include "compiler/Peephole.h"
#include "compiler/Registers.h"
#include "driver/COSToolsCLT.h"
#include "compiler/CodeGen.h"
#include <string.h>
#include "compiler/FEInterface.h"
#include "compiler/BE_binary.h"

SInt32 fn_0046FF50(void* node) {
    Type* type = ((Object*)node)->type;
    return (type->size + objgen_alignment_mask_636430) & ~objgen_alignment_mask_636430;
}

void fn_0046EAE0(void) {
    FreeGList((GList*)objgen_list_603190);
    fn_0053F8B0();
    IRO_Cleanup();
    fn_004ACDF0();
    fn_004BB0D0();
    reg_info_list = 0;
}

static inline SInt32 get_type(void* node) {
    UInt8 kind = ((Type*)node)->type;
    if (kind == TYPESTRUCT) {
        SInt32 result;
        StructMember* ptr = TYPE_STRUCT(node)->members;
        if (!ptr) {
            result = 0;
        } else if (ptr->next) {
            result = 0;
        } else {
            Type* membertype = ptr->type;
            if (membertype->type == TYPEFLOAT) {
                result = (SInt32)membertype;
            } else {
                result = 0;
            }
        }
        return result;
    }
    if (kind == TYPECLASS) {
        return (SInt32)fn_00470080(TYPE_CLASS(node));
    }
    return 0;
}

Type* fn_00470030(Type* type) {
    UInt8 kind = type->type;
    if (kind == TYPESTRUCT) {
        Type* result;
        StructMember* ptr = TYPE_STRUCT(type)->members;
        if (!ptr) {
            result = NULL;
        } else if (ptr->next) {
            result = NULL;
        } else {
            Type* membertype = ptr->type;
            if (membertype->type == TYPEFLOAT) {
                result = membertype;
            } else {
                result = NULL;
            }
        }
        return result;
    }
    if (kind == TYPECLASS) {
        return fn_00470080(TYPE_CLASS(type));
    }
    return NULL;
}

ElfGenSymbol* fn_0046EA30(Object* node) {
    HashNameNode* name;
    ObjGenSymbolEntry* entry;
    SInt32 hash;

    name = CMangler_GetLinkName(node);
    hash = name->hashval;
    entry = objgen_symbol_hash_6031A0[hash];
    while (entry) {
        if (entry->name == name) {
            return entry->symbol;
        }
        entry = entry->next;
    }
    entry = galloc(12);
    memclrw(entry, 12);
    entry->next = objgen_symbol_hash_6031A0[name->hashval];
    objgen_symbol_hash_6031A0[name->hashval] = entry;
    entry->name = name;
    if (node->datatype == DDATA) {
        entry->symbol = ElfNewSymbolRecord(name->name, 0, 1, 1, node->type->size, 0);
    } else {
        entry->symbol = ElfNewSymbolRecord(name->name, 0, 1, 0, 0, 0);
    }
    return entry->symbol;
}

SInt32 fn_0046F770(const char* name) {
    SInt32 index;
    if (*name == '.') {
        for (index = 1; index < objgen_scope_count_636778; index++) {
            if ((register_debug_scope_table[index].alias1 &&
                 !strcmp(name, register_debug_scope_table[index].alias1)) ||
                (register_debug_scope_table[index].alias2 &&
                 !strcmp(name, register_debug_scope_table[index].alias2))) {
                return index;
            }
        }
    } else {
        for (index = 1; index < objgen_scope_count_636778; index++) {
            if (!strcmp(name, register_debug_scope_table[index].name)) {
                return index;
            }
        }
    }
    return 0;
}

Type* fn_0046FF70(Type* node) {
    Type* type = NULL;
    if (copts.unknown15) {
        type = (Type*)get_type(node);
        if (type &&
            type->type == TYPEFLOAT &&
            ((IS_TYPE_FLOAT32(type) && (copts.fp_library & 1)) ||
             (IS_TYPE_FLOAT64(type) && (copts.fp_library & 2)))) {
            return type;
        }
    }
    return NULL;
}

void fn_0046EB10(void) {
    UInt8* data;
    ObjectList* entry;
    Object* object;
    Type* type;
    float single;
    double wide;
    CompilerLinkerParamBlk* reader;

    fn_00512E20();
    for (entry = objgen_float_constants_636E6C; entry; entry = entry->next) {
        object = entry->object;
        type = object->type;
        data = object->u.templ;
        if (IS_TYPE_FLOAT32(type)) {
            single = *(double*)data;
            fn_0046DD50(object, &single, NULL, 4);
        } else {
            wide = *(double*)data;
            fn_0046DD50(object, &wide, NULL, 8);
        }
    }
    if (copts.unknown13A == 1) {
        fn_00540D20();
    }
    if (InitGList((GList*)objgen_list_603190, 0x8000)) {
        CError_NoMem();
    }
    fn_0053F8F0((GList*)objgen_list_603190);
    cparamblkptr->object.objectdata = COS_ConvertToCWMemHandle(objgen_list_603190);
    *(void**)objgen_list_603190 = NULL;
    reader = cparamblkptr;
    fn_0053F820(&reader->object.codesize, &reader->object.idatasize, &reader->object.udatasize);
    fn_004ACDF0();
}

void fn_0046EC40(void) {
    SInt32 index;
    UInt32 flags;
    UInt16 version;
    UInt32 width;
    void* comment;
    objgen_flags_63683C = 0;
    if (copts.unknown0A == 1) {
        objgen_flags_63683C |= 0x20000000;
    }
    fn_004ACE00();
    fn_00439BF0();
    objgen_state_6355C4 = 0;
    register_code_context = 0;
    objgen_float_constants_636E6C = binary_fixup_list = NULL;
    objgen_context_6364B4 = 0;
    objgen_record_635B98 = 0;
    objgen_fallback_637377 = 0;
    fn_0046D380();
    *(void**)objgen_list_603190 = NULL;
    fn_0046F060();
    for (index = 0; index < 2048;) {
        SInt32 base = index;
        index += 8;
        objgen_symbol_hash_6031A0[base] = NULL;
        objgen_symbol_hash_6031A0[base + 1] = NULL;
        objgen_symbol_hash_6031A0[base + 2] = NULL;
        objgen_symbol_hash_6031A0[base + 3] = NULL;
        objgen_symbol_hash_6031A0[base + 4] = NULL;
        objgen_symbol_hash_6031A0[base + 5] = NULL;
        objgen_symbol_hash_6031A0[base + 6] = NULL;
        objgen_symbol_hash_6031A0[base + 7] = NULL;
    }
    version = copts.unknown0C;
    flags = 1;
    if ((SInt16)version >= 4) {
        flags |= 0x30000000;
    } else if ((SInt16)version >= 3) {
        flags |= 0x20000000;
    } else if ((SInt16)version >= 2) {
        flags |= 0x10000000;
    }
    if (copts.littleendian) {
        width = 1;
    } else {
        width = 2;
    }
    flags |= 0x924000;
    fn_00540200(8, flags, width);
    comment = ElfAllocateSection(".comment", 1, 0);
    elf_append_string(comment, "MW MIPS C Compiler (3.0.0)");
    if (copts.unknown13A == 1) {
        fn_00540FB0();
    }
}

void fn_0046EDD0(Object* obj, ObjGenRegisterLocation* loc) {
    ObjRegInfo* scope;
    SInt32 index;
    UInt8 kind;
    UInt8 datatype;
    loc->offset = 0;
    if (!obj ||
        (obj->datatype == DDATA &&
         (scope = obj->u.data.reginfo) != NULL &&
         (scope->reg != 0 || scope->rclass == 5 || (scope->flags != 0 && scope->rclass == 0)))) {
        loc->number = -1;
        loc->isDirect = 0;
        return;
    }
    datatype = obj->datatype;
    if (datatype == 1) {
        loc->number = stack_pointer_register;
        loc->isDirect = 0;
        return;
    }
    {
        index = fn_0046EF20(obj);
        kind = *(UInt8*)((UInt8*)register_debug_scope_table + index * 52 + 4);
        switch (kind) {
            case 1:
                loc->number = 0;
                loc->isDirect = 1;
                break;
            case 0:
            case 2:
                loc->number = 0;
                loc->isDirect = 0;
                break;
            case 3:
                if (copts.unknown08) {
                    loc->number = 23;
                    loc->isDirect = 1;
                } else {
                    CError_Internal("ObjGen.c", 677);
                }
                break;
            case 4:
                if (copts.unknown08) {
                    loc->number = 23;
                    loc->isDirect = 0;
                } else {
                    CError_Internal("ObjGen.c", 686);
                }
                break;
            case 5:
                loc->number = 28;
                loc->isDirect = 1;
                loc->offset =
                    *(UInt8*)((UInt8*)register_debug_scope_table + ((index * 13) << 2) + 48);
                break;
            case 6:
                if (copts.unknown07) {
                    loc->number = 28;
                    loc->isDirect = 0;
                } else {
                    CError_Internal("ObjGen.c", 701);
                }
                break;
        }
        return;
    }
}

SInt32 fn_0046EF20(Object* obj) {
    UInt8 datatype;
    SInt32 regclass;
    HashNameNode* name;
    const char* text;
    datatype = obj->datatype;
    regclass = obj->section;
    datatype = (UInt8)(datatype - DFUNC);
    if (datatype <= 1) {
        if (!regclass || !(register_debug_scope_table[regclass].attributes & 4)) {
            return objgen_scope_635B6C;
        }
    }
    name = obj->name;
    if (name->name[1] == '_' && !strncmp(name->name, "__vt__", 6)) {
        return objgen_vtable_scope_6355CC;
    }
    text = name->name;
    if (!strncmp(text, "@stringBase0", 12)) {
        return objgen_string_scope_6364D4;
    }
    if (!strncmp(text, "@wstringBase0", 13)) {
        return objgen_string_scope_6364D4;
    }
    if (regclass == objgen_special_scope_636190 || regclass == objgen_special_scope_63617C) {
        return regclass;
    }
    if (objgen_fallback_637377) {
        objgen_fallback_637377 = 0;
        obj->section = (UInt8)objgen_string_scope_6364D4;
        obj->qual |= 1;
        return objgen_string_scope_6364D4;
    }
    if (regclass) {
        return regclass;
    }
    if (is_const_object(obj)) {
        return objgen_string_scope_6364D4;
    }
    if (!regclass) {
        if (!(obj->type->size > 0 && obj->type->size <= fn_00438D70())) {
            regclass = objgen_small_scope_635B54;
        } else {
            regclass = objgen_large_scope_6366B8;
        }
    }
    return regclass;
}

void fn_0046F060(void) {
    SInt32 index;
    objgen_scope_count_636778 = 1;
    objgen_large_scope_6366B8 = fn_0046F3F0("sdata", ".sdata", ".sbss", "near_data", "RW");
    objgen_near_absolute_6361F0 = fn_0046F3F0("zdata", ".zdata", ".zbss", "near_absolute", "RW");
    objgen_special_scope_636190 = fn_0046F3F0("lit4", ".lit4", NULL, "near_data", "RW");
    register_debug_scope_table[objgen_special_scope_636190].offset = 1;
    objgen_special_scope_63617C = fn_0046F3F0("lit8", ".lit8", NULL, "near_data", "RW");
    register_debug_scope_table[objgen_special_scope_63617C].offset = 1;
    objgen_ctor_635FE8 = fn_0046F3F0("pinit", ".ctor", NULL, "far_absolute", "R");
    objgen_init_635F44 = fn_0046F3F0("init", ".init", NULL, "far_absolute", "RX");
    if (copts.unknown08) {
        objgen_scope_635B6C = fn_0046F3F0("text", ".text", NULL, "far_code", "RX");
        objgen_string_scope_6364D4 = fn_0046F3F0("rodata", ".rodata", NULL, "far_code", "R");
        register_debug_scope_index = fn_0046F3F0("exception", ".exception", NULL, "far_code", "R");
        register_debug_type_index = fn_0046F3F0("exceptix", ".exceptix", NULL, "far_code", "R");
        objgen_picrelo_635FC4 = fn_0046F3F0("picrelo", ".picdynrel", NULL, "far_data", "R");
    } else {
        objgen_scope_635B6C = fn_0046F3F0("text", ".text", NULL, "far_absolute", "RX");
        objgen_string_scope_6364D4 = fn_0046F3F0("rodata", ".rodata", NULL, "far_absolute", "R");
        register_debug_scope_index =
            fn_0046F3F0("exception", ".exception", NULL, "far_absolute", "R");
        register_debug_type_index = fn_0046F3F0("exceptix", ".exceptix", NULL, "far_absolute", "R");
    }
    if (copts.unknown07) {
        objgen_small_scope_635B54 = fn_0046F3F0("data", ".data", ".bss", "far_data", "RW");
        objgen_vtable_scope_6355CC = fn_0046F3F0("vtables", ".vtables", NULL, "far_data", "R");
        objgen_pidrelo_6361A8 = fn_0046F3F0("pidrelo", ".piddynrel", NULL, "far_data", "R");
    } else {
        objgen_small_scope_635B54 = fn_0046F3F0("data", ".data", ".bss", "far_absolute", "RW");
        objgen_vtable_scope_6355CC = fn_0046F3F0("vtables", ".vtables", NULL, "far_absolute", "R");
    }
    objgen_spad_636D1C = fn_0046F3F0("scratchpad", ".spad_data", ".spad_bss", "far_absolute", "RW");
    objgen_vutext_636834 = fn_0046F3F0("vutext", ".vutext", NULL, "far_absolute", "RX");
    objgen_vudata_636D50 = fn_0046F3F0("vudata", ".vudata", ".vubss", "far_absolute", "RW");
}

SInt32 fn_0046F3F0(
    const char* name,
    const char* section,
    const char* bss,
    const char* region,
    const char* permission
) {
    volatile UInt8 category;
    UInt32 attributes;
    ObjGenScopeEntry* entry;
    char* relname;
    SInt32 index;
    const char* testname;
    if (!region) {
        region = "standard";
    }
    if (!permission) {
        permission = "RWX";
    }
    if (!__builtin_memcmp(region, "standard", 9)) {
        category = 0;
    } else if (!__builtin_memcmp(region, "near_absolute", 14)) {
        category = 1;
    } else if (!__builtin_memcmp(region, "far_absolute", 13)) {
        category = 2;
    } else if (!__builtin_memcmp(region, "near_code", 10)) {
        category = 3;
    } else if (!__builtin_memcmp(region, "far_code", 9)) {
        category = 4;
    } else if (!__builtin_memcmp(region, "near_data", 10)) {
        category = 5;
    } else if (!__builtin_memcmp(region, "far_data", 9)) {
        category = 6;
    } else {
        return 0;
    }
    if (!__builtin_memcmp(permission, "R", 2)) {
        attributes = 2;
    } else if (!__builtin_memcmp(permission, "RW", 3)) {
        attributes = 3;
    } else if (!__builtin_memcmp(permission, "RX", 3)) {
        attributes = 6;
    } else if (!__builtin_memcmp(permission, "RWX", 4)) {
        attributes = 7;
    } else {
        return 0;
    }
    if (!__builtin_memcmp(name, "text16", 7)) {
        attributes += 0x80000000;
    }
    for (index = 1; index < objgen_scope_count_636778; index++) {
        entry = &register_debug_scope_table[index];
        testname = entry->name;
        if (!strcmp(name, testname)) {
            break;
        }
    }
    if (index == objgen_scope_count_636778) {
        if (objgen_scope_count_636778 == 0x40) {
            return 0;
        }
        objgen_scope_count_636778++;
    }
    entry = &register_debug_scope_table[index];
    entry->name = name;
    entry->attributes = attributes;
    entry->category = category;
    if ((UInt8)(category - 5) <= 1) {
        entry->attributes |= 0x10000000;
    } else if ((UInt8)(category - 3) <= 1) {
        entry->attributes |= 0x40000000;
    }
    entry->alias1 = section;
    entry->sectionType = 1;
    entry->section = NULL;
    entry->alias2 = bss;
    entry->bssType = 8;
    entry->bss = NULL;
    entry->relocationName = galloc(__builtin_strlen(section) + 5);
    relname = entry->relocationName;
    *(UInt32*)relname = *(const UInt32*)".rel";
    relname[4] = *(const volatile char*)(".rel" + 4);
    strcat(entry->relocationName, section);
    entry->relocationType = 9;
    entry->relocation = NULL;
    entry->offset = 0;
    return index;
}

SInt32 fn_0046F830(HashNameNode* attribute, void* unused, DeclInfo* declinfo) {
    SInt32 token;
    Type* signed_mode = NULL;
    Type* unsigned_mode = NULL;
    const char* value;
    if (!__builtin_memcmp(attribute->name, "section", 8) ||
        !__builtin_memcmp(attribute->name, "__section__", 12)) {
        tk = lex();
        if (tk != '(') {
            CError_Error(22);
            return 1;
        }
        tk = lex();
        if (tk != -4) {
            CError_Error(22);
            return 1;
        }
        token = fn_0046F770((const char*)tkstring);
        if (!token) {
            CError_Error(77);
            return 1;
        }
        declinfo->section = token;
        tk = lex();
        if (tk != ')') {
            CError_Error(22);
            return 1;
        }
        return 1;
    }
    if (!__builtin_memcmp(attribute->name, "mode", 5) ||
        !__builtin_memcmp(attribute->name, "__mode__", 9)) {
        tk = lex();
        if (tk != '(') {
            CError_Error(22);
            return 1;
        }
        tk = lex();
        if (tk != -3) {
            CError_Error(8);
            return 1;
        }
        value = tkidentifier->name;
        if (!__builtin_memcmp(value, "QI", 3) || !__builtin_memcmp(value, "__QI__", 7)) {
            signed_mode = TYPE(&stsignedchar);
            unsigned_mode = TYPE(&stunsignedchar);
        } else if (!__builtin_memcmp(value, "HI", 3) || !__builtin_memcmp(value, "__HI__", 7)) {
            signed_mode = TYPE(&stsignedshort);
            unsigned_mode = TYPE(&stunsignedshort);
        } else if (!__builtin_memcmp(value, "SI", 3) || !__builtin_memcmp(value, "__SI__", 7)) {
            signed_mode = TYPE(&stsignedint);
            unsigned_mode = TYPE(&stunsignedint);
        } else if (!__builtin_memcmp(value, "DI", 3) ||
                   !__builtin_memcmp(value, "__DI__", 7) ||
                   !__builtin_memcmp(value, "V2SI", 5) ||
                   !__builtin_memcmp(value, "__V2SI__", 9)) {
            signed_mode = TYPE(&stsignedlong);
            unsigned_mode = TYPE(&stunsignedlong);
        } else if (!__builtin_memcmp(value, "TI", 3) ||
                   !__builtin_memcmp(value, "__TI__", 7) ||
                   !__builtin_memcmp(value, "V4SF", 5) ||
                   !__builtin_memcmp(value, "__V4SF__", 9) ||
                   !__builtin_memcmp(value, "V2DI", 5) ||
                   !__builtin_memcmp(value, "__V2DI__", 9) ||
                   !__builtin_memcmp(value, "V4SI", 5) ||
                   !__builtin_memcmp(value, "__V4SI__", 9) ||
                   !__builtin_memcmp(value, "V8HI", 5) ||
                   !__builtin_memcmp(value, "__V8HI__", 9) ||
                   !__builtin_memcmp(value, "V16QI", 6) ||
                   !__builtin_memcmp(value, "__V16QI__", 10)) {
            signed_mode = TYPE(&stint128);
            unsigned_mode = TYPE(&stint128);
        } else if (!__builtin_memcmp(value, "SF", 3) || !__builtin_memcmp(value, "__SF__", 7)) {
            signed_mode = TYPE(&stfloat);
            unsigned_mode = NULL;
        } else if (!__builtin_memcmp(value, "DF", 3) || !__builtin_memcmp(value, "__DF__", 7)) {
            signed_mode = TYPE(&stdouble);
            unsigned_mode = NULL;
        } else {
            CError_Error(77);
            return 1;
        }
        tk = lex();
        if (tk != ')') {
            CError_Error(22);
            return 1;
        }
        if (is_unsigned(declinfo->thetype)) {
            if (!unsigned_mode) {
                CError_Error(77);
                return 1;
            }
            declinfo->thetype = unsigned_mode;
        } else {
            declinfo->thetype = signed_mode;
        }
        return 1;
    }
    if (!__builtin_memcmp(attribute->name, "weak", 5) ||
        !__builtin_memcmp(attribute->name, "__weak__", 9)) {
        declinfo->qual |= Q_WEAK;
        return 1;
    }
    if (!__builtin_memcmp(attribute->name, "internal", 9) ||
        !__builtin_memcmp(attribute->name, "__internal__", 13)) {
        declinfo->exportflags |= 0x10;
        return 1;
    }
    if (!__builtin_memcmp(attribute->name, "import", 7) ||
        !__builtin_memcmp(attribute->name, "__import__", 11) ||
        !__builtin_memcmp(attribute->name, "dllimport", 10) ||
        !__builtin_memcmp(attribute->name, "__dllimport__", 14)) {
        declinfo->exportflags |= 0x20;
        return 1;
    }
    if (!__builtin_memcmp(attribute->name, "export", 7) ||
        !__builtin_memcmp(attribute->name, "__export__", 11) ||
        !__builtin_memcmp(attribute->name, "dllexport", 10) ||
        !__builtin_memcmp(attribute->name, "__dllexport__", 14)) {
        declinfo->exportflags |= 0x40;
        return 1;
    }
    if (!__builtin_memcmp(attribute->name, "lib_export", 11) ||
        !__builtin_memcmp(attribute->name, "__lib_export__", 15)) {
        declinfo->exportflags |= 0x60;
        return 1;
    }
    return 0;
}
