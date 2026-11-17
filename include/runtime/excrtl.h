#ifndef RUNTIME_EXCRTL_H
#define RUNTIME_EXCRTL_H

#include "runtime/exchand.h"

struct SEHActionEntry {
    int previous_state;
    int (*filter)(void);
    void (*handler)(void);
};

struct SEHFrame {
    SEHFrame* next;
    void* exception_handler;
    SEHActionEntry* actions;
    int state;
    void* frame;
};

struct FloatingSaveAreaWin32 {
    DWORD ControlWord;
    DWORD StatusWord;
    DWORD TagWord;
    DWORD ErrorOffset;
    DWORD ErrorSelector;
    DWORD DataOffset;
    DWORD DataSelector;
    BYTE RegisterArea[80];
    DWORD Cr0NpxState;
};

struct ContextWin32 {
    DWORD ContextFlags;
    DWORD Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
    FloatingSaveAreaWin32 FloatSave;
    DWORD SegGs, SegFs, SegEs, SegDs;
    DWORD Edi, Esi, Ebx, Edx, Ecx, Eax, Ebp;
    DWORD Eip, SegCs, EFlags, Esp, SegSs;
    BYTE ExtendedRegisters[512];
};

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned long SEH_CPUInstructionSet;
void SEH_InitializeCPUInstructionSet(void);
int __SEHFrameHandler(
    ExceptionRecordWin32* record, SEHFrame* frame, ContextWin32* context, void* dispatcher
);
void SEH_UnwindFrame(SEHFrame* frame, ContextWin32* context, int target_state);
//! @reconstructed
int SEH_UnwindFilter(
    ExceptionRecordWin32* record, SEHFrame* frame, ContextWin32* context, SEHFrame** current
);

#ifdef __cplusplus
}
#endif

#endif
