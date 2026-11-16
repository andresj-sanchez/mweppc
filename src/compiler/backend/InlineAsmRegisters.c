#include "compiler/InlineAsmRegisters.h"
#include "compiler/CompilerTools.h"
#include <string.h>

#if MWCCPS2_B52
// b69: InlineAsm_InitializeRegisters 0x50ABA0 (64 buckets at 0x56A018),
// InsertRegister 0x50ABC0 (0x14-byte records: name, class and number as
// words, the object), LookupRegister 0x50AC10 (returns &name); the MIPS
// assembler's set-up (0x4B4690) inserts its 393 names through it
#else
#pragma pack(push, 2)
#endif
typedef struct HashedRegister {
    struct HashedRegister* next;
#if MWCCPS2_B52
    char* name;
    SInt32 rclass;
    SInt32 num;
    Object* object;
#else
    IARegister reg;
#endif
} HashedRegister;
#if !MWCCPS2_B52
#pragma pack(pop)
#endif

static HashedRegister* hashedregisters[64];

void InlineAsm_InitializeRegisters(void) {
    SInt32 i;

    for (i = 0; i < 64; i++) {
        hashedregisters[i] = NULL;
    }
}

#if MWCCPS2_B52
void InlineAsm_InsertRegister(char* name, short rclass, short num, Object* object) {
#else
void InlineAsm_InsertRegister(char* name, RegClass rclass, short num, Object* object) {
#endif
    HashedRegister** ptr;
    HashedRegister* hr;

    ptr = hashedregisters + (CHash(name) & 63);
    hr = lalloc(sizeof(HashedRegister));
#if MWCCPS2_B52
    hr->name = name;
    hr->rclass = rclass;
    hr->num = num;
    hr->object = object;
#else
    hr->reg.name = name;
    hr->reg.rclass = rclass;
    hr->reg.num = num;
    hr->reg.object = object;
#endif

#if MWCCPS2_B52
    hr->next = *ptr, *ptr = hr; // -inline off keeps the comma's value
#else
    hr->next = *ptr;
    *ptr = hr;
#endif
}

#if MWCCPS2_B52
void* InlineAsm_LookupRegister(char* name) {
#else
IARegister* InlineAsm_LookupRegister(char* name) {
#endif
    HashedRegister* scan;
#if MWCCPS2_B52
    char** reg;
#else
    IARegister* reg;
#endif

    for (scan = hashedregisters[CHash(name) & 63]; scan; scan = scan->next) {
#if MWCCPS2_B52
        reg = &scan->name;
        if (!strcmp(*reg, name)) {
#else
        reg = &scan->reg;
        if (!strcmp(reg->name, name)) {
#endif
            return reg;
        }
    }

    return NULL;
}
