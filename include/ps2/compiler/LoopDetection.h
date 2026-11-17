#ifndef COMPILER_LOOPDETECTION_H
#define COMPILER_LOOPDETECTION_H

#include "compiler/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LoopRegisterBlock LoopRegisterBlock;
typedef struct LoopRegisterNode LoopRegisterNode;

struct LoopRegisterBlock {
    LoopRegisterBlock* next;
    PCodeBlock* block;
};

struct LoopRegisterNode {
    UInt8 kind;
    UInt8 unk01;
    UInt16 index; // 0x02
    LoopRegisterNode* parent; // 0x04
    LoopRegisterNode* children; // 0x08
    LoopRegisterNode* nextSibling; // 0x0C
    PCodeBlock* block; // 0x10
    UInt8 reserved14[4];
    LoopRegisterBlock* blocks; // 0x18
    UInt32* memberblocks; // 0x1C
    UInt8 reserved20[4];
    LoopRegisterBlock* padBlockLink; // 0x24
    UInt8 reserved28[0x28];
    PCodeBlock* destinationBlock; // 0x50
    SInt32 maxRegisterCount[7]; // 0x54
};

extern void fn_004C4720(LoopRegisterNode* loop);

#pragma pack(push, 2)
struct BlockList {
    BlockList* next;
    PCodeBlock* block;
};

struct InstrList {
    InstrList* next;
    PCode* instr;
};

typedef enum LoopBound {
    LOOP_BOUND_INDETERMINATE,
    LOOP_BOUND_CONSTANT,
    LOOP_BOUND_VARIABLE
} LoopBound;

struct Loop {
    Loop* parent;
    Loop* nextSibling;
    Loop* children;
    PCodeBlock* body; // repeated block
    PCodeBlock* preheader; // block at the start of the loop
    PCodeBlock* footer; // block at the end of the loop
    PCode* inductionUpdate;
    BlockList* blocks;
    UInt32* memberblocks;
    UInt32* exitblocks;
    UInt32* exit_dominators;
    UInt32* backedge_dominators;
    BasicInductionVar* basicInductionVars;
    int loopWeight;
    SInt32 bodySize;
    SInt32 iterationCount;
    SInt32 lower;
    SInt32 upper;
    SInt32 step;
    UInt8 unknownCondition;
    Boolean has_call;
    Boolean uses_count_register;
    Boolean is_straight_line;
    Boolean isUnknownCountingLoop; // is a counting loop with non-constant iteration count
    Boolean isKnownCountingLoop;
    Boolean has_unrolled_code;
    Boolean has_indexed_load;
    Boolean has_indexed_store;
    LoopBound lowerType;
    LoopBound upperType;
    Boolean has_memory_barrier;
};

struct BasicInductionVar {
    BasicInductionVar* next;
    Loop* loop;
    InductionVar* inductionVars;
    InstrList* instrs;
    PCode* initializer;
    SInt32 step;
    short reg;
};

struct InductionVar {
    InductionVar* next;
    BasicInductionVar* basicVar;
    PCode* instr;
    PCode* replacement_instruction;
    Loop* outer_loop;
    SInt32 step;
    short base_operand;
    short source_operand;
    short source_register;
    short destination_register;
};
#pragma pack(pop)

extern Loop* loopsinflowgraph;
extern int loopdetection_nblocks;
extern struct BitVector* LoopTemp;
extern struct LoopList* LoopList_First;

extern void addblocktoloop(Loop* loop, PCodeBlock* block);
extern void insertpreheaderblock(Loop* loop);
extern void findloopsinflowgraph(void);
extern void analyzeForCountableLoops(Loop* loop);
extern void analyzeloop(Loop* loop);
extern void analyzeloopsinflowgraph(void);

extern LoopRegisterNode* loop_tree_636D58;

extern int fn_004C47E0(SInt32 unused);

extern void fn_004C4990(void* node, void (*callback)(void*), SInt8 preorder);

#ifdef __cplusplus
}
#endif
extern void fn_004C4700(void);
extern void forall_loops_in_postorder_do(void (*callback)(void*));

#endif
