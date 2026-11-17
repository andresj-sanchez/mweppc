#ifndef COMPILER_PEEPHOLE_H
#define COMPILER_PEEPHOLE_H

#include "compiler/common.h"

typedef struct PeepholeHandler {
    struct PeepholeHandler* next;
    void* callback;
} PeepholeHandler;

extern void peepholeoptimizepcode(Object* func);

extern SInt32 fn_004BB060(PCode* first);
extern SInt32 fn_004BB070(PCode* first, PCode* second);
extern void printspillstatistics(UInt32 a, UInt32 b, UInt32 c, UInt32 d);
//! @reconstructed
extern void fn_004BB090(void);
extern void fn_004BB0A0(SInt32 low, SInt32 high);
extern void fn_004BB0B0(UInt32 index, SInt32 cost, SInt32 reg, Object* object);
extern void pclistblocks(const char* name, const char* title);
extern void fn_004BB0D0(void);
extern void fn_004BB0E0(Object* object);
extern void fn_004BB180(Object* object);
extern void fn_004BB2A0(Object* object);
extern void fn_004BB370(Object* object);
extern void peepholemergeblocks(Object* object, SInt8 flag);
extern void installpattern(UInt16 opcode, void* callback);
extern int fn_004BB790(PCodeBlock* block);
extern int usedbetween(PCode* start, PCode* end, PCodeArg* operand);
extern int definedbetween(PCode* start, PCode* end, PCodeArg* operand);
extern void computeinstructionpredecessors(PCodeBlock* block);
extern void fn_004BB9D0(void);

extern PCode* peephole_default_state;
extern PCode** peephole_register_state[];

extern PeepholeHandler* peephole_handlers[];

#endif
