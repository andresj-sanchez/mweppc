#ifndef _MSL_STDIO_API_H
#define _MSL_STDIO_API_H

#include <stdarg.h>
#include <stddef.h>

#define EOF -1L

enum __ReadProcActions {
    __GetAChar,
    __UngetAChar,
    __TestForError
};

#define fetch() (count++, (*ReadProc)(ReadProcArg, 0, __GetAChar))
#define unfetch(c) (*ReadProc)(ReadProcArg, c, __UngetAChar)
#define final_state(scan_state) ((scan_state) & (finished | failure))

typedef struct {
    char* NextChar;
    int NullCharDetected;
} __InStrCtrl;

extern int __StringRead(void* isc, int ch, int action);
extern int __sformatter(
    int (*ReadProc)(void*, int, int), void* ReadProcData, const char* format, va_list arg
);

struct _FILE;
extern size_t __fwrite(const void* ptr, size_t size, size_t count, struct _FILE* file);

#endif
