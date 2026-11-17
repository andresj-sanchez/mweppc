#include "compiler/CompilerTools.h"
#include "compiler/common.h"

typedef void* (*IROAllocProc)(UInt32);

static SInt32 iro_pool_heads[12];
static SInt32 iro_oheap_tail;
static SInt32 iro_gheap_tail;
static SInt32 iro_oheap_ready;
static SInt32 iro_gheap_ready;

void IRO_InitializeAllocatorByHeap(IROAllocProc alloc) {
    int i;

    if (alloc == galloc) {
        if (!iro_gheap_ready) {
            for (i = 0; i < 12; i++) {
                iro_pool_heads[i] = 0;
            }
            iro_gheap_tail = 0;
            iro_gheap_ready = 1;
        }
    } else {
        if (!iro_oheap_ready) {
            for (i = 0; i < 12; i++) {
                iro_pool_heads[i] = 0;
            }
            iro_oheap_tail = 0;
            iro_oheap_ready = 1;
        }
    }
}

static void IRO_TerminateAllocatorByHeap(IROAllocProc alloc) {
    if (alloc != galloc) {
        iro_oheap_ready = 0;
    } else {
        iro_gheap_ready = 0;
    }
}

void IRO_InitializeAllocator(void) {
    IRO_InitializeAllocatorByHeap(oalloc);
    IRO_InitializeAllocatorByHeap(galloc);
}

void IRO_TerminateAllocator(void) {
    IRO_TerminateAllocatorByHeap(oalloc);
    IRO_TerminateAllocatorByHeap(galloc);
}
