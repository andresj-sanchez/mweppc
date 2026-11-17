#ifndef _MSL_THREADLOCALDATA_H
#define _MSL_THREADLOCALDATA_H

#include <time.h>
#include "msl/locale.h"

typedef struct _ThreadLocalData {
    struct _ThreadLocalData* next;
    int errno;
    unsigned long int random_next;
    unsigned char* strtok_n;
    unsigned char* strtok_s;
    void* thread_handle;
    struct tm gmtime_tm;
    struct tm localtime_tm;
    char asctime_result[26];
    char temp_name[256];
    struct lconv* unused;
    char* locale_name;
    struct __locale _current_locale;
    void (*user_se_translator)();
    struct lconv __lconv;
    wchar_t wtemp_name[256];
} _ThreadLocalData;

#ifdef __cplusplus
extern "C" {
#endif

extern int _InitializeThreadDataIndex(void);
extern void _DisposeThreadDataIndex(void);
extern int _InitializeThreadData(void* inThreadHandle);
extern void _DisposeThreadData(void);
extern void _DisposeAllThreadData(void);
extern int _InitializeMainThreadData(void);
extern _ThreadLocalData* _GetThreadLocalData(int main_only);
extern void _DisposeThreadDataIndex(void);

#ifdef __cplusplus
}
#endif

#endif
