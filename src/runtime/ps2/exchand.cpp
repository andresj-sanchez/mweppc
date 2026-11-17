#include "runtime/exchand.h"
#include "msl/threadlocaldata.h"

extern "C" void unexpected(void);

#pragma use_frame on
extern "C" int _HandleUnexpectedSEHException(ExceptionPointersWin32* pointers) {
    if (!(pointers->ExceptionRecord->ExceptionFlags & 0x12)) {
        _ThreadLocalData* tld = _GetThreadLocalData(1);
        if (tld->user_se_translator) {
            ((void (*)(unsigned int, ExceptionPointersWin32*))tld->user_se_translator)(
                pointers->ExceptionRecord->ExceptionCode, pointers);
        }
    }
    return 0;
}

extern "C" int _MWCHandler(ExceptionRecordWin32* record, void*, void*, void*) {
    if (record->ExceptionFlags & 0x66) {
        return 1;
    }
    if (record->ExceptionCode == 0xE06D7363) {
        unexpected();
    }
    return 1;
}
#pragma use_frame off
