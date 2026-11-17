#include "compiler/BitVector.h"
#include "compiler/BitVectors.h"
#include "compiler/LoopDetection.h"
#include "compiler/MIPS_Peephole.h"

#include "compiler/CompilerTools.h"
#include "compiler/PCode.h"
#include "compiler/PCodeUtilities.h"
#include "compiler/common.h"

#pragma dont_inline on
void fn_004C4720(LoopRegisterNode* loop) {
    volatile SInt8 cls;
    LoopRegisterBlock* entry;
    SInt32 value;

    switch (loop->kind) {
        case 1:
            for (cls = 0; cls < 7; cls++) {
                loop->maxRegisterCount[cls] =
                    peephole_register_state_636220[cls].words[loop->block->blockIndex];
            }
            break;
        case 0:
        case 2:
            for (cls = 0; cls < 7; cls++) {
                loop->maxRegisterCount[cls] = 0;
            }
            for (entry = loop->blocks; entry; entry = entry->next) {
                for (cls = 0; cls < 7; cls++) {
                    value = peephole_register_state_636220[cls].words[entry->block->blockIndex];
                    if ((UInt32)value > (UInt32)loop->maxRegisterCount[cls]) {
                        loop->maxRegisterCount[cls] = value;
                    }
                }
            }
            break;
    }
}
#pragma dont_inline reset

void fn_004C4700(void) {
    fn_0055D750();
    forall_loops_in_postorder_do((void (*)(void*))fn_004C4720);
}

#pragma dont_inline on
void forall_loops_in_postorder_do(void (*callback)(void*)) {
    LoopRegisterNode* node;

    for (node = loop_tree_636D58; node; node = node->nextSibling) {
        fn_004C4990(node, callback, 0);
    }
}
#pragma dont_inline reset

static UInt32** dominators;

static void computedominators(void) {
    SInt32 blockCount;
    PCLink* link;
    UInt32* myvec;
    PCodeBlock* block;
    int i;
    int changed;

    blockCount = pcblockcount;
    changed = TRUE;

    dominators = oalloc(sizeof(UInt32*) * pcblockcount);
    for (i = 0; i < pcblockcount; i++) {
        dominators[i] = oalloc(4 * ((blockCount + 31) >> 5));
    }

    myvec = oalloc(4 * ((blockCount + 31) >> 5));

    bitvectorinitialize(dominators[pcbasicblocks->blockIndex], blockCount, 0);
    bitvectorsetbit(0, dominators[pcbasicblocks->blockIndex]);

    for (block = pcbasicblocks->nextBlock; block; block = block->nextBlock) {
        bitvectorinitialize(dominators[block->blockIndex], blockCount, 0xFFFFFFFF);
    }

    computedepthfirstordering();

    while (changed) {
        changed = FALSE;
        for (i = 0; i < pcblockcount; i++) {
            block = depthfirstordering[i];
            if (block && block->blockIndex != pcbasicblocks->blockIndex) {
                bitvectorcopy(myvec, dominators[block->predecessors->from->blockIndex], blockCount);
                for (link = block->predecessors->nextPredecessor; link;
                     link = link->nextPredecessor) {
                    bitvectorintersect(myvec, dominators[link->from->blockIndex], blockCount);
                }
                bitvectorsetbit(block->blockIndex, myvec);

                if (bitvectorchanged(dominators[block->blockIndex], myvec, blockCount)) {
                    bitvectorcopy(dominators[block->blockIndex], myvec, blockCount);
                    changed = TRUE;
                }
            }
        }
    }
}
