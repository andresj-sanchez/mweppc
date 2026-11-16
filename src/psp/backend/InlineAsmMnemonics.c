// b69's mnemonic table (InlineAsmMnemonics.c, its assert file name;
// 0x50C1B0..0x50C2A3, after GCCInlineAsm.c): 256 buckets (0x56A118) of
// records holding a mnemonic's opcodes inline. The MIPS assembler's set-up
// (0x4B4690) clears them and inserts every opcode's name and alias; its
// instruction scanner (0x4B67D0) looks the mnemonic up and tries the opcodes
// in turn (count at +4 and opcodes at +8 from the returned &name). 3.0.1
// keeps a growable array instead (InlineAsmTables.c).
#include "compiler/common.h"
#include "compiler/CError.h"
#include "compiler/CompilerTools.h"
#include <string.h>

typedef struct HashedMnemonic {
    struct HashedMnemonic* next;
    char* name; // +4: the lookup returns &name
    SInt32 count; // +8
    SInt32 opcodes[102]; // +C
} HashedMnemonic; // 0x1A4

static HashedMnemonic* hashedmnemonics[256];

void InlineAsm_InitializeMnemonics(void) {
    int i;

    for (i = 0; i < 256; i++) {
        hashedmnemonics[i] = NULL;
    }
}

void InlineAsm_InsertMnemonic(char* name, SInt32 opcode) {
    HashedMnemonic* hm;
    HashedMnemonic** ptr;

    ptr = hashedmnemonics + (CHash(name) & 255);
    for (hm = *ptr; hm; hm = hm->next) {
        if (!strcmp(hm->name, name)) {
            CError_ASSERT(63, hm->count != 101);
            hm->opcodes[hm->count++] = opcode;
            return;
        }
    }

    hm = lalloc(sizeof(HashedMnemonic));
    hm->name = name;
    hm->count = 1;
    hm->opcodes[0] = opcode;
    hm->next = *ptr, *ptr = hm; // -inline off keeps the comma's value
}

void* InlineAsm_LookupMnemonic(char* name) {
    HashedMnemonic* hm;

    for (hm = hashedmnemonics[CHash(name) & 255]; hm; hm = hm->next) {
        if (!strcmp(hm->name, name)) {
            return &hm->name;
        }
    }

    return NULL;
}
