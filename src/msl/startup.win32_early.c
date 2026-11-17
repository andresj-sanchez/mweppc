#include "compiler/win32.h"
#include "msl/startup.win32.h"
#include "msl/threadlocaldata.h"
#include <errno.h>

#pragma avoid_ebp on
unsigned long __dup_core(void* in_handle, void** out_handle) {
    if (in_handle != 0 && in_handle != INVALID_HANDLE_VALUE &&
        DuplicateHandle(GetCurrentProcess(), in_handle, GetCurrentProcess(), out_handle,
                        0x100000, 1, 2)) {
        return 0;
    }
    *out_handle = INVALID_HANDLE_VALUE;
    return GetLastError();
}

#pragma avoid_ebp off

int __set_errno(unsigned long error) {
    if (error != 0) {
        _doserrno = error;
    }
    switch (error) {
    case 0:
        break;
    case 1:
        errno = 22;
        break;
    case 2:
    case 3:
    case 53:
    case 55:
        errno = 2;
        break;
    case 4:
        errno = 23;
        break;
    case 5:
    case 32:
    case 33:
        errno = 1;
        break;
    case 6:
        errno = 9;
        break;
    case 8:
    case 14:
    case 36:
        errno = 12;
        break;
    case 10:
        errno = 22;
        break;
    case 11:
        errno = 8;
        break;
    case 12:
    case 13:
        errno = 22;
        break;
    case 15:
        errno = 2;
        break;
    case 16:
        errno = 17;
        break;
    case 17:
        errno = 18;
        break;
    case 18:
        errno = 24;
        break;
    case 19:
        errno = 13;
        break;
    case 21:
        errno = 6;
        break;
    case 22:
        errno = 19;
        break;
    case 23:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
        errno = 5;
        break;
    case 24:
        errno = 36;
        break;
    case 39:
        errno = 28;
        break;
    case 50:
        errno = 38;
        break;
    case 54:
        errno = 16;
        break;
    case 111:
        errno = 34;
        break;
    default:
        errno = 99;
        break;
    }
    return errno;
}
