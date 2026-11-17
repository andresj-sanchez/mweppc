#ifndef COMPILER_IROMALLOC_H
#define COMPILER_IROMALLOC_H

extern void IRO_gfree(void* buf);
extern void IRO_InitializeAllocator(void);
extern void IRO_TerminateAllocator(void);

extern void* IRO_malloc(UInt32 size);

#endif
