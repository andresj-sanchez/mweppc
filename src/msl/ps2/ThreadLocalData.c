#include "compiler/common.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "compiler/win32.h"
#include "msl/critical_regions.h"
#include "msl/locale.h"
#include "msl/threadlocaldata.h"

static unsigned long _gThreadDataIndex = -1;

static _ThreadLocalData* firstTLD;

int _InitializeThreadDataIndex(void) {
    if (_gThreadDataIndex != -1) {
        return 1;
    }

    _gThreadDataIndex = TlsAlloc();

    if (_gThreadDataIndex == -1) {
        return 0;
    }

    return 1;
}

void _DisposeThreadDataIndex(void) {
    if (_gThreadDataIndex == -1) {
        return;
    }

    TlsFree(_gThreadDataIndex);
    _gThreadDataIndex = -1;
}

int _InitializeThreadData(void* inThreadHandle) {
    HANDLE threadHandle = (HANDLE)inThreadHandle;
    _ThreadLocalData* theThreadLocalData;

    theThreadLocalData = TlsGetValue(_gThreadDataIndex);

    if (theThreadLocalData == 0) {
        if (GetLastError() == NO_ERROR) {
            theThreadLocalData = (_ThreadLocalData*)malloc(sizeof(_ThreadLocalData));

            if (!theThreadLocalData) {
                return 0;
            }

            __begin_critical_region(thread_access);

            theThreadLocalData->next = firstTLD;
            firstTLD = theThreadLocalData;

            __end_critical_region(thread_access);

            theThreadLocalData->errno = 0;
            theThreadLocalData->random_next = 1;
            theThreadLocalData->strtok_n = (unsigned char*)"";
            theThreadLocalData->strtok_s = (unsigned char*)"";
            theThreadLocalData->thread_handle = threadHandle;
            theThreadLocalData->locale_name = "C";
            theThreadLocalData->localtime_tm.tm_sec = 0;
            theThreadLocalData->localtime_tm.tm_min = 0;
            theThreadLocalData->localtime_tm.tm_hour = 0;
            theThreadLocalData->localtime_tm.tm_mday = 1;
            theThreadLocalData->localtime_tm.tm_mon = 0;
            theThreadLocalData->localtime_tm.tm_year = 0;
            theThreadLocalData->localtime_tm.tm_wday = 1;
            theThreadLocalData->localtime_tm.tm_yday = 0;
            theThreadLocalData->localtime_tm.tm_isdst = -1;

            theThreadLocalData->gmtime_tm.tm_sec = 0;
            theThreadLocalData->gmtime_tm.tm_min = 0;
            theThreadLocalData->gmtime_tm.tm_hour = 0;
            theThreadLocalData->gmtime_tm.tm_mday = 1;
            theThreadLocalData->gmtime_tm.tm_mon = 0;
            theThreadLocalData->gmtime_tm.tm_year = 0;
            theThreadLocalData->gmtime_tm.tm_wday = 1;
            theThreadLocalData->gmtime_tm.tm_yday = 0;
            theThreadLocalData->gmtime_tm.tm_isdst = -1;

            theThreadLocalData->_current_locale.next_locale = NULL;
            strcpy(theThreadLocalData->_current_locale.locale_name, _current_locale.locale_name);
            theThreadLocalData->_current_locale.coll_cmpt_ptr = _current_locale.coll_cmpt_ptr;
            theThreadLocalData->_current_locale.ctype_cmpt_ptr = _current_locale.ctype_cmpt_ptr;
            theThreadLocalData->_current_locale.mon_cmpt_ptr = _current_locale.mon_cmpt_ptr;
            theThreadLocalData->_current_locale.num_cmpt_ptr = _current_locale.num_cmpt_ptr;
            theThreadLocalData->_current_locale.time_cmpt_ptr = _current_locale.time_cmpt_ptr;
            theThreadLocalData->_current_locale = _current_locale;

            theThreadLocalData->__lconv = __lconv;

            theThreadLocalData->user_se_translator = 0L;
            return TlsSetValue(_gThreadDataIndex, theThreadLocalData);
        }
        return 0;
    }
    return 1;
}

int _InitializeMainThreadData(void) {
    if (_InitializeThreadDataIndex()) {
        if (_InitializeThreadData(NULL)) {
            return 1;
        }
    }

    return 0;
}

void _DisposeAllThreadData(void) {
    _ThreadLocalData* current;

    __begin_critical_region(thread_access);

    current = firstTLD;
    while (current) {
        _ThreadLocalData* next = current->next;
        free(current);
        current = next;
    }
    firstTLD = NULL;

    __end_critical_region(thread_access);
}

_ThreadLocalData* _GetThreadLocalData(int inInitializeDataIfMissing) {
    _ThreadLocalData* tld;

    tld = (_ThreadLocalData*)TlsGetValue(_gThreadDataIndex);

    if (inInitializeDataIfMissing) {
        if (tld == 0) {
            _InitializeThreadData(NULL);
            tld = (_ThreadLocalData*)TlsGetValue(_gThreadDataIndex);
        }

        if (tld == 0) {
            MessageBoxA(
                NULL, "Could not allocate thread local data.", "MW Win32 Runtime", MB_OK | MB_ICONEXCLAMATION
            );
            exit(127);
        }
    }
    return tld;
}
