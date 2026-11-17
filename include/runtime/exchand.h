#ifndef RUNTIME_EXCHAND_H
#define RUNTIME_EXCHAND_H

#include "compiler/win32.h"

struct ExceptionRecordWin32 {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
};

struct ExceptionPointersWin32 {
    ExceptionRecordWin32* ExceptionRecord;
    void* ContextRecord;
};

#ifdef __cplusplus
extern "C" {
#endif

int _MWCHandler(ExceptionRecordWin32* record, void* frame, void* context, void* reserved);
int _HandleUnexpectedSEHException(ExceptionPointersWin32* pointers);

#ifdef __cplusplus
}
#endif

#endif
