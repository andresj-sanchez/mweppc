#ifndef COMPILER_OBJGEN_H
#define COMPILER_OBJGEN_H

#include "compiler/common.h"
#include "compiler/ELFgen.h"

typedef struct ObjGenSymbolEntry {
    struct ObjGenSymbolEntry* next;
    HashNameNode* name;
    ElfGenSymbol* symbol;
} ObjGenSymbolEntry;

#ifdef __cplusplus
extern "C" {
#endif

extern SInt32 objgen_alignment_mask_636430;

extern ObjectList* objgen_float_constants_636E6C;

struct ObjGenCodeRelocation {
    ObjGenCodeRelocation* next;
    Object* object;
    UInt32 offset;
    SInt32 addend;
    SInt16 kind;
};

struct ObjGenRelocation {
    ObjGenRelocation* next;
    Object* object;
    UInt32 offset;
    SInt32 addend;
};

struct ObjGenRegisterLocation {
    SInt32 number;
    SInt32 isDirect;
    UInt8 offset;
};
extern void fn_0046EDD0(Object* obj, ObjGenRegisterLocation* loc);

struct ObjGenScopeEntry {
    const char* name;
    UInt8 category;
    UInt32 attributes;
    const char* volatile alias1;
    UInt32 sectionType;
    ElfGenSection* section;
    const char* volatile alias2;
    UInt32 bssType;
    ElfGenSection* bss;
    char* volatile relocationName;
    UInt32 relocationType;
    ElfGenSection* relocation;
    UInt8 offset;
};
extern ObjGenScopeEntry register_debug_scope_table[];
extern SInt32 objgen_scope_count_636778;
extern SInt32 objgen_scope_635B6C;
extern SInt32 objgen_vtable_scope_6355CC;
extern SInt32 objgen_string_scope_6364D4;
extern SInt32 objgen_special_scope_636190;
extern SInt32 objgen_special_scope_63617C;
extern SInt32 objgen_small_scope_635B54;
extern SInt32 objgen_large_scope_6366B8;
extern SInt32 objgen_picrelo_635FC4;
extern SInt32 objgen_pidrelo_6361A8;
extern char objgen_pic_format[];
extern char objgen_pid_format[];
extern UInt8 objgen_fallback_637377;

extern ElfGenSymbol* fn_0046EA30(Object* node);
extern void fn_0046EAE0(void);
extern void fn_0046EB10(void);
extern void fn_0046EC40(void);
extern SInt32 fn_0046EF20(Object* obj);
extern void fn_0046F060(void);
extern SInt32 fn_0046F3F0(
    const char* name,
    const char* section,
    const char* bss,
    const char* region,
    const char* permission
);
extern SInt32 fn_0046F770(const char* name);
extern SInt32 fn_0046F830(HashNameNode* attribute, void* unused, DeclInfo* declinfo);
extern SInt32 fn_0046FF50(void* node);
extern Type* fn_0046FF70(Type* node);
extern Type* fn_00470030(Type* type);

extern SInt32 objgen_context_6364B4;
extern SInt32 objgen_near_absolute_6361F0;
extern SInt32 objgen_spad_636D1C;
extern SInt32 objgen_vudata_636D50;
extern SInt32 objgen_vutext_636834;
extern UInt32 objgen_flags_63683C;
extern char objgen_list_603190[];

extern ObjGenSymbolEntry* objgen_symbol_hash_6031A0[];
extern SInt32 objgen_ctor_635FE8;
extern SInt32 objgen_init_635F44;
extern SInt32 objgen_record_635B98;
extern ObjectList* objgen_state_6355C4;

#ifdef __cplusplus
}
#endif

#endif
