#ifndef COMPILER_ELFGEN_H
#define COMPILER_ELFGEN_H

#include "compiler/common.h"
#include "compiler/CompilerTools.h"
#include "compiler/ELF_Endian.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ElfGenRelocation {
    struct ElfGenRelocation* next;
    void* target;
    UInt32 offset;
    UInt32 info;
    UInt32 addend;
};

typedef struct ElfGenSection {
    char* name; // 0x00
    UInt32 index; // 0x04
    GList data; // 0x08
    struct ElfGenSection* relocations; // 0x18
    struct ElfGenRelocation* list; // 0x1C
    struct ElfGenRelocation** listtail; // 0x20
    Elf32_Shdr shdr; // 0x24
} ElfGenSection;

typedef struct ElfDebugOffset {
    SInt32 number;
    UInt32 offset;
} ElfDebugOffset;

typedef struct ElfGenSymbol {
    struct ElfGenSymbol* next; // 0x00
    const char* name; // 0x04
    UInt32 index; // 0x08
    Elf32_Sym sym; // 0x0C
} ElfGenSymbol;

extern void ElfLinkRelocation(ElfGenSection* owner, ElfGenSection* section);
//! @reconstructed
extern void elf_insert_half(ElfGenSection* section, SInt32 offset, UInt16 val);
extern void elf_insert_word(ElfGenSection* section, SInt32 offset, UInt32 val);
extern SInt32 elf_append_byte(ElfGenSection* section, UInt8 val);
extern SInt32 elf_append_string(ElfGenSection* section, const char* str);
extern SInt32 elf_append_half(ElfGenSection* section, UInt16 val);
extern SInt32 elf_append_word(ElfGenSection* section, UInt32 val);
extern ElfGenSymbol* ElfNewSymbolRecord(
    const char* name, UInt16 index, UInt8 binding, UInt8 kind, UInt32 size, UInt32 value
);
//! @reconstructed
extern ElfGenSymbol* ElfAllocateSymbolRecord(
    const char* name,
    UInt16 index,
    UInt8 binding,
    UInt8 kind,
    UInt32 value,
    UInt32 size,
    UInt32 alignment
);
//! @reconstructed
extern void ElfDefineSymbolRecord(
    ElfGenSymbol* symbol,
    UInt16 index,
    UInt8 binding,
    UInt8 kind,
    UInt32 size,
    UInt32 data,
    UInt32 alignment
);
//! @reconstructed
extern void ElfAppendSectionRelocation(
    UInt8 kind, ElfGenSymbol* symbol, UInt32 addend, void* target, UInt32 data
);
//! @reconstructed
extern void ElfAppendSymbolData(ElfGenSection* section, Elf32_Sym* symbol);
//! @reconstructed
extern ElfGenSection* ElfAllocateSection(char* name, UInt32 kind, SInt32 initialSize);
//! @reconstructed
extern void ElfEmitSymbolTable(void); //! @reconstructed
extern void ElfTrackDebugSource(Object* object); //! @reconstructed
extern void fn_00541160(Object* object, SInt32 isGlobal, ElfGenSymbol* symbol, SInt32 size);
extern void fn_00540FB0(void);
extern void fn_00540D20(void);
extern ElfDebugOffset* fn_00541DF0(UInt32 base, SInt32 size);
extern UInt32 fn_00541EA0(DwarfFunctionState* state);
extern void fn_00541F80(DwarfNode* node);
extern void fn_00541380(Object* function, SInt32 isGlobal, ElfGenSymbol* symbol, SInt32 size);
extern ElfGenSection* fn_0053F810(UInt16 index);
extern void fn_0053F820(SInt32* writableSize, SInt32* allocatedSize, SInt32* uninitializedSize);
extern void fn_0053F8B0(void);
extern void fn_00540300(ElfGenSection* section);
extern void fn_00540340(ElfGenSection* section, void* recordArg);
extern void fn_00540200(UInt16 machine, UInt32 flags, SInt32 endian);
extern void fn_0053F8F0(GList* output);
extern void fn_0053FB50(GList* gl, const Elf32_Ehdr* hdr);
extern void fn_0053FF20(GList* gl, const Elf32_Shdr* shdr);

extern Elf32_Ehdr elfHeader_636078;
extern Elf32_Sym elfNullSymbol;
extern ElfGenSection* elfSectionNameTable_61C19C;
extern ElfGenSection* elfStringTable;
extern ElfGenSection* elfSymbolTable;
extern ElfGenSection** elfSectionTable;
extern ElfGenSymbol* elfSymbolHead;
extern ElfGenSymbol** elfSymbolListTail;
extern SInt32 elfEndian;
extern SInt32 elfSectionCapacity;
extern UInt16 elfSectionCount;
extern UInt8 elfDebugProcessFlag;
extern UInt8 elfZeroPadding[];
extern char elfNameEmpty_5EA133;
extern char elfNameShstrtab_5EA144;
extern char elfNameStrtab_5EA13C;
extern char elfNameSymtab_5EA134;
extern const char ELFgen_file_name[];
extern const char elfNullString[];
extern DwarfFunctionState* elfDebugBaseScope;
extern DwarfFunctionState* elfDebugCurrentScope;
extern DwarfFunctionState* elfDebugScopeTail;
extern DwarfFunctionState* elfDebugScopeChain;
extern DwarfNode* elfDebugScopeField;
extern void* elfDebugPendingCount;
extern DwarfPendingRecord* elfDebugPendingTail;
extern PendingObject* elfDebugSources;
extern DWInfo* elfDebugHashTable[1024];
extern UInt32 elfDebugHashRoot;
extern char elfDebugSourceContext[1024];
extern SInt32 elfDebugContext;

#ifdef __cplusplus
}
#endif

#endif
