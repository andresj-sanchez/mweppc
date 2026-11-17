#ifndef COMPILER_SPILL_H
#define COMPILER_SPILL_H

#include "compiler/common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void insertspillcode(void);
extern void assign_spill_locations(void);
extern void spillcopy(PCodeBlock* block, PCode* instr);
extern void spillinstruction(PCodeBlock* block, PCode* instr);
extern void estimatespillcosts(void);

extern SInt32 spill_statistics_0;
extern SInt32 spill_statistics_1;
extern SInt32 spill_statistics_2;
extern SInt32 spill_statistics_3;

#ifdef __cplusplus
}
#endif

#endif
