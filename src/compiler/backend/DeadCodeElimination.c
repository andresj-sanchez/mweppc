#include "compiler/common.h"
#include "compiler/PCode.h"
#include "compiler/LiveRegisters.h"
#include "compiler/DeadCodeElimination.h"

void eliminatedeadcode(Object* proc) {
    PCodeBlock* block;
    PCode* instr;
    SInt32 changed;

    do {
        changed = FALSE;
        compute_live_registers();
        for (block = pcbasicblocks; block; block = block->nextBlock) {
            if (!(block->flags & 0x180) && block->pcodeCount) {
                capture_live_registers(block);
                for (instr = block->lastPCode; instr; instr = instr->prevPCode) {
                    if (fn_004CF6D0(instr)) {
                        changed = TRUE;
                        deletepcode(instr);
                    } else {
                        update_live_registers(instr);
                    }
                }
            }
        }
        freeoheap();
    } while (changed);
}
