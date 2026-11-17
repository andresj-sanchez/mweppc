#ifndef COMPILER_DWARF_H
#define COMPILER_DWARF_H

#include "compiler/common.h"
#include "compiler/ELFgen.h"
#include "compiler/CError.h"
#include "compiler/CompilerTools.h"

struct DwarfLocationOperand {
    UInt8 kind; /* 0x00 */
    UInt8 reg; /* 0x01 */
    UInt16 unk2;
    SInt32 offset; /* 0x04 */
    UInt8 base; /* 0x08 */
    UInt8 unk9;
    UInt16 unkA;
    UInt32 unkC;
};

typedef struct DwarfRegisterSet {
    DwarfLocationOperand reg[32];
} DwarfRegisterSet;

struct PendingObject {
    Object* object;
    PendingObject* next;
};

struct DwarfPendingRecord {
    UInt32 unk0;
    UInt32 unk4;
    DwarfPendingRecord* next;
};

struct DwarfFunctionState {
    DwarfFunctionState* next; /* 0x00 */
    DwarfNode* first; /* 0x04 */
    DwarfNode* last; /* 0x08 */
    DwarfNode* lastType; /* 0x0C */
    SInt32 mode; /* 0x10 */
    ElfGenSection* section; /* 0x14 */
    ElfGenSection* relocations; /* 0x18 */
    ElfGenSection* extraSection; /* 0x1C */
    ElfGenSection* extraRelocations; /* 0x20 */
    ElfGenSymbol* symbol; /* 0x24 */
    ElfGenSymbol* lineSymbol; /* 0x28 */
    PendingObject* pendingObjects; /* 0x2C */
    DwarfPendingRecord* pendingRecords; /* 0x30 */
    HashNameNode* source; /* 0x34 */
    Boolean pending; /* 0x38 */
};

struct DWInfo {
    DWInfo* next;
    Type* type;
    DwarfNode* node;
    Boolean marked;
};

struct DwarfRef {
    UInt16 tag; /* 0x00 */
    UInt16 fundamental; /* 0x02 */
    UInt16 num; /* 0x04 */
    UInt16 modifiers[33]; /* 0x06 */
    UInt32 size; /* 0x48 */
    DwarfNode* node; /* 0x4C */
};

struct DwarfNode {
    DwarfNode* next; /* 0x00 */
    DwarfNode* prev; /* 0x04 */
    DwarfNode* sibling; /* 0x08 */
    DwarfFunctionState* scope; /* 0x0C */
    SInt32 offset; /* 0x10 */
    SInt32 position; /* 0x14 */
    UInt16 kind; /* 0x18 */
    UInt16 unk1A;
    UInt32 unk1C;
    union {
        struct {
            DWInfo* type;
            SInt32 size;
            SInt32 bound;
        } array;
        struct {
            HashNameNode* name;
            SInt32 size;
            SInt32 listSize;
            ObjEnumConst* members;
        } type;
        struct {
            HashNameNode* name;
            DWInfo* type;
            UInt16 access;
            UInt16 unk2A;
            SInt32 offset;
            UInt16 bitSize;
            UInt16 bitOffset;
            Boolean unk34;
        } member;
        struct {
            DWInfo* type;
            SInt32 offset;
            UInt16 access;
            UInt8 isVirtual;
        } inheritance;
        struct {
            ElfGenSymbol* symbol;
            SInt32 low;
            SInt32 high;
        } block;
        struct {
            DwarfNode* function;
        } unit;
        struct {
            HashNameNode* name;
            HashNameNode* linkname;
            DWInfo* type;
            UInt16 location; /* 0x2C */
            UInt16 reg; /* 0x2E */
            UInt16 reg2; /* 0x30 */
            UInt16 unk32;
            SInt32 offset; /* 0x34 */
            ElfGenSymbol* symbol; /* 0x38 */
        } var;
        struct {
            HashNameNode* name;
            HashNameNode* linkname;
            DWInfo* type;
            UInt16 paramCount; /* 0x2C */
            UInt16 unk2E;
            ElfGenSymbol* symbol; /* 0x30 */
            SInt32 size; /* 0x34 */
            SInt32 frameSize; /* 0x38 */
            SInt32 frameOffset; /* 0x3C */
            ElfDebugOffset* lines; /* 0x40 */
            UInt16 access; /* 0x44 */
            Boolean declaration; /* 0x46 */
            UInt8 unk47;
            DwarfLocationOperand returnOperand; /* 0x48 */
            DwarfRegisterSet registers[3]; /* 0x58 */
        } func;
    } u;
};

extern char elfDebugAnonymousSymbol[];
extern char elfDebugExtraRelocName[];
extern char elfDebugExtraSectionName[];
extern char elfDebugLineRelocName[];
extern char elfDebugLineSectionName[];
extern const char DWARF_file_name[];
extern const char dwarfAnonymousFormat[];

extern UInt32 fn_005427B0(DwarfNode* node);
extern UInt32 fn_00543520(DwarfLocationOperand* operand, UInt16 attribute, UInt32 mode);
extern UInt32 fn_005436E0(DwarfNode* node);
extern UInt32 fn_005438C0(DwarfRef* ref);
extern void fn_00543A40(DWInfo* info);
extern DwarfNode* fn_00543D40(TypeFunc* tfunc);
extern DwarfNode* fn_00543EF0(TypeClass* tclass, Boolean includeBases, SInt32 baseOffset);
extern DwarfNode* fn_00544170(TypeClass* tclass);
extern DwarfNode* fn_00544330(TypeStruct* tstruct);
extern void fn_00544550(DWInfo* info, DwarfRef* ref);
extern DWInfo* fn_00544700(Type* type);
extern void fn_00544810(SInt32 mode);

static inline DwarfNode* NewDwarfNode(SInt32 kind) {
    DwarfNode* node;
    if (kind <= 0) {
        CError_Internal((char*)DWARF_file_name, 0x6F);
    }
    node = galloc(sizeof(DwarfNode));
    memclrw(node, sizeof(DwarfNode));
    node->kind = kind;
    if (elfDebugCurrentScope->last) {
        elfDebugCurrentScope->last->next = node;
        node->prev = elfDebugCurrentScope->last;
    } else {
        elfDebugCurrentScope->first = node;
    }
    node->sibling = NULL;
    node->scope = elfDebugCurrentScope;
    elfDebugCurrentScope->last = node;
    return elfDebugCurrentScope->last;
}

typedef SInt32 (*DwarfEnumWriter)(ElfGenSection* section, UInt32 value);
extern const char dwarfCompilerName[];
extern const char dwarfEmptyName[];

#endif
