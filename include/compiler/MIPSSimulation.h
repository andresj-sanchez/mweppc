#ifndef COMPILER_MIPSSIMULATION_H
#define COMPILER_MIPSSIMULATION_H

#include "compiler/PCode.h"
#include "compiler/Scheduler.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int (*simulation_callback)(PCode*);
extern SInt32 simulation_wait_special[2];
extern SInt32 simulation_special_pending[2];
extern SInt32 simulation_wait_pair[2][2];
extern PCode* simulation_pair_pending[2][2];
extern SInt32 simulation_last_cycle[6];
extern PCode* simulation_last_reg[6];
extern SInt32 simulation_wait_reg[6];
extern PCode* simulation_class_use_0_array[6];
extern SInt32 simulation_tick;
extern MachineInfo mipssim_machine_5E9068;
extern MachineInfo mipssim_machine_5E9088;

extern SInt32 fn_004BF3E0(PCode* pcode);
extern int fn_004BF530(PCode* pcode);
extern void fn_004BF840(PCode* pcode);
extern void fn_004BFC00(void);
extern void fn_004BFD70(void);
extern void fn_004BFE60(PCode* pcode, PCode* delay);
extern int fn_004C0050(PCode* pcode);
extern int fn_004C0340(PCode* pcode);
extern int fn_004C0470(PCode* pcode);
extern void fn_004C04D0(void);

#ifdef __cplusplus
}
#endif

#endif
