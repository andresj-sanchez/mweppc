#ifndef COMPILER_BE_SCHEDULE_H
#define COMPILER_BE_SCHEDULE_H

#include "compiler/common.h"

extern Boolean blocks_changed;
extern PCodeBlock* previous_block;
extern void insert_division_nops(void); //! @reconstructed
extern void optimize_blocks(void); //! @reconstructed
extern void fill_delay_slots(Object* function, int schedule, Boolean fromTarget);
//! @reconstructed

#endif
