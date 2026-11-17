#ifndef COMPILER_PCODE_H
#define COMPILER_PCODE_H

#include "compiler/common.h"
#include "compiler/CInt64.h"

struct PCodeLabel {
    struct PCodeLabel* nextLabel;
    struct PCodeBlock* block;
    SInt16 resolved;
    UInt16 index;
};

struct PCodeArg {
    UInt8 kind;
    UInt8 arg;
    UInt16 effect;
    union {
        struct {
            SInt16 reg;
            struct PCode* definer;
        } reg;
        struct {
            SInt32 value;
        } imm;
        struct {
            SInt32 offset;
            Object* obj;
        } mem;
        struct {
            struct PCodeLabel* label;
            SInt32 unknown8;
        } label;
        CInt64 cint;
    } data;
    SInt32 unknown14;
};

struct PCode {
    struct PCode* nextPCode;
    struct PCode* prevPCode;
    struct PCodeBlock* block;
    UInt32 flags;
    SInt32 useID; /* 0x10 */
    SInt32 defID; /* 0x14 */
    union {
        struct Alias* alias; /* 0x18 */
        SInt32 unk18;
    };
    SInt32 unk1C;
    SInt32 sourceoffset;
    SInt32 unk24;
    UInt16 op;
    SInt16 argCount;
    SInt32 unk2C;
    SInt32 unk30;
    Object* object; /* 0x34 */
    SInt32 unk38;
    struct PCode* unk3C;
    PCodeArg args[0];
};

enum {
    PCOp_REGISTER = 0,
    PCOp_IMMEDIATE = 2,
    PCOp_CINT64 = 3,
    PCOp_MEMORY = 4,
    PCOp_LABEL = 6,
    PCOp_PLACEHOLDEROPERAND = 7
};

enum {
    fPCodeIsBranch = 1, //! @reconstructed
    fPCodeReadsMemory = 2, //! @reconstructed
    fPCodeLikely = 0x800000, //! @reconstructed
    fPCodeFlag2000000 = 0x2000000, //! @reconstructed
    fPCodeFlag4000000 = 0x4000000, //! @reconstructed
    fPCodeFlag8000000 = 0x8000000, //! @reconstructed
    fPCodeConditionalBranch = 0x10000000, //! @reconstructed
    fPCodeHasDelaySlot = 0x20000000, //! @reconstructed
    fPCodeInDelaySlot = 0x40000000 //! @reconstructed
};

struct PCLink {
    struct PCLink* nextHash; //! @reconstructed
    struct PCLink* nextSuccessor; //! @reconstructed
    struct PCLink* nextPredecessor; //! @reconstructed
    struct PCodeBlock* from; //! @reconstructed
    struct PCodeBlock* to; //! @reconstructed
    SInt32 index; //! @reconstructed
    UInt16 hash; //! @reconstructed
    UInt16 flags; //! @reconstructed
};

typedef struct PCodeLabelBranch { //! @reconstructed
    struct PCodeLabelBranch* next;
    struct PCodeBlock* block;
    UInt16 flags;
} PCodeLabelBranch;

struct PCodeBlock {
    struct PCodeBlock* nextBlock;
    struct PCodeBlock* prevBlock;
    PCodeLabel* labels;
    struct PCLink* predecessors;
    struct PCLink* successors;
    PCode* firstPCode;
    PCode* lastPCode;
    SInt32 blockIndex;
    SInt32 codeOffset;
    SInt32 unk24;
    UInt32 loopWeight;
    SInt16 pcodeCount;
    UInt16 flags;
};

enum {
    fVisited = 4,
    fScheduled = 8,
    fPCBlockFlag10 = 0x10,
    fDeleted = 0x20,
    fPCBlockFlag80 = 0x80, //! @reconstructed
    fPCBlockFlag4000 = 0x4000, //! @reconstructed
    fPCBlockFlag100 = 0x100 //! @reconstructed
};

#define PCLINK_HASH_SIZE 1997 //! @reconstructed

extern PCodeBlock* pcbasicblocks;
extern PCodeBlock* pclastblock;
extern PCodeBlock** depthfirstordering;
extern int pcblockcount;
extern UInt32 pcloopweight;
extern UInt16 pclabelcount;
extern int depthfirstorder;
extern PCLink* pclinkhash[PCLINK_HASH_SIZE]; //! @reconstructed
extern int pclinkcount; //! @reconstructed

extern void initpcode(void);
extern PCode* makepcode(short op, ...);
extern void emitpcode(short op, ...);
extern PCode* copypcode(PCode* pcode);
extern PCodeLabel* makepclabel(void);
extern PCodeBlock* makepcblock(void);
extern void pclabelblock(PCodeBlock* block, PCodeLabel* label);
extern void pcaddedge(PCodeBlock* block, PCodeLabel* label, UInt16 flags);
extern void emit_label(PCodeLabel* label);
extern void new_basic_block(void); //! @reconstructed
extern void deleteblock(PCodeBlock* block); //! @reconstructed
extern void deleteunreachableblocks(void);
extern void appendpcode(PCodeBlock* block, PCode* pcode);
extern void deletepcode(PCode* pcode);
extern void insertpcodebefore(PCode* anchor, PCode* newpcode);
extern void insertpcodeafter(PCode* anchor, PCode* newpcode);
extern void setpcodeflags(UInt32 flags);
extern void computedepthfirstordering(void);
extern PCodeBlock* insertblockbetween(PCodeBlock* from, PCodeBlock* to); //! @reconstructed
extern PCLink* insertedge(PCodeBlock* from, PCodeBlock* to); //! @reconstructed
extern void deleteedge(PCodeBlock* from, PCodeBlock* to); //! @reconstructed

#endif
