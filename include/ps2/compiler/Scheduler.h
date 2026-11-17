#ifndef COMPILER_SCHEDULER_H
#define COMPILER_SCHEDULER_H

#include "compiler/PCode.h"

typedef struct MachineInfo {
    int issue_width; /* 0x00 */
    int renames_registers; /* 0x04 */
    int (*latency)(PCode* instr); /* 0x08 */
    void (*initialize)(void); /* 0x0C */
    int (*can_issue)(PCode* instr); /* 0x10 */
    void (*issue)(PCode* instr); /* 0x14 */
    void (*advance_clock)(void); /* 0x18 */
    /* 0x1C */
    void (*dependence)(PCode* instr, PCode* successor);
} MachineInfo;

struct SchedulerDGSuccessor {
    SchedulerDGSuccessor* next;
    SchedulerDGNode* node;
    UInt16 latency;
    UInt16 unkA;
};

struct SchedulerDGNodeList {
    SchedulerDGNodeList* next;
    SchedulerDGNode* node;
};

struct SchedulerDGNode {
    SchedulerDGNode* next;
    SchedulerDGNode* prev;
    SchedulerDGSuccessor* successors;
    PCode* instr;
    UInt16 latency;
    UInt16 earliest;
    UInt16 latest;
    UInt16 unk16;
    UInt16 height;
    UInt16 predCount;
};

extern void fn_004C0520(PCodeBlock* block);

extern SInt32 scheduler_available_registers[7];
extern SInt32 scheduler_register_classes[7];
extern SInt32* scheduler_register_counts[7];
extern UInt16 scheduler_critical_path;
extern char scheduler_source_file[];
extern SchedulerDGNodeList* scheduler_barriers;
extern MachineInfo* scheduler_machine_info;
extern SchedulerDGNodeList* scheduler_mem_reads;
extern SchedulerDGNodeList* scheduler_mem_writes;
extern SchedulerDGNodeList* scheduler_memory_dep_defs;
extern SchedulerDGNodeList* scheduler_memory_dep_uses;
extern SchedulerDGNodeList* scheduler_volatile_defs;
extern SchedulerDGNodeList* scheduler_volatile_uses;
extern SchedulerDGNodeList** scheduler_memory_defs;
extern SchedulerDGNodeList** scheduler_memory_uses;
extern SchedulerDGNodeList** scheduler_register_defs[7];
extern SchedulerDGNodeList** scheduler_register_uses[7];

extern void fn_004C0510(MachineInfo* machine);
extern SchedulerDGNode* scheduler_default_successor;
extern SchedulerDGNode* scheduler_all_nodes;
extern void fn_004C0920(SchedulerDGNode* node);
extern void fn_004C13B0(PCode* instr);
extern void fn_004C0F60(void);
extern void fn_004C14A0(PCodeBlock* block);
extern SInt32 fn_004C1160(PCode* instr);
extern SInt32 fn_004C11F0(PCode* instr);
extern void fn_004C0EC0(SchedulerDGNode* node, SchedulerDGNode* successor, Boolean hasLatency);
extern void fn_004C0D10(SchedulerDGNode* node);
extern void fn_004C0DC0(
    SInt32 hasHazards,
    SchedulerDGNode* node,
    SchedulerDGNodeList** defs,
    SchedulerDGNodeList** uses,
    SInt32 isWrite
);
extern SchedulerDGNode* fn_004C0740(SchedulerDGNode* nodes, UInt16 clock);

#endif
