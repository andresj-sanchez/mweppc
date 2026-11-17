#include <stdarg.h>
#include "compiler/common.h"
#include "msl/stdio_api.h"

int __StringRead(void* isc, int ch, int Action) {
    char RetVal;
    __InStrCtrl* Iscp = (__InStrCtrl*)isc;
    switch (Action) {
        case __GetAChar:
            RetVal = *(Iscp->NextChar);
            if (RetVal == '\0') {
                Iscp->NullCharDetected = 1;
                return EOF;
            } else {
                Iscp->NextChar++;
                return (unsigned char)RetVal;
            }
        case __UngetAChar:
            if (!Iscp->NullCharDetected)
                Iscp->NextChar--;
            else
                Iscp->NullCharDetected = 0;
            return ch;
        case __TestForError:
            return Iscp->NullCharDetected;
    }
    return 0;
}

int vsscanf(const char* s, const char* format, va_list arg) {
    __InStrCtrl isc;

    isc.NextChar = (char*)s;

    if ((s == NULL) || (*isc.NextChar == '\0'))
        return EOF;

    isc.NullCharDetected = 0;

    return __sformatter(&__StringRead, (void*)&isc, format, arg);
}

int sscanf(const char* s, const char* format, ...) {
    return vsscanf(s, format, __va_start(format));
}
