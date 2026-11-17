#define _MSL_RESTRICT restrict
#define _MSL_USE_INLINE 1
#include "msl/setupargs.h"
#include "compiler/win32.h"
#include "msl/ctype_api.h"
#include <stdlib.h>
#include <string.h>

#pragma use_frame on

char** __argv;
char* _acmdln;
int __argc, __argv1_offs;
static int __maxargc;
static char* __command_line = 0;
static void _FreeCommandLine(void) {
    if (__command_line) {
        free(__command_line);
    }
    if (__argv && __argc) {
        free(__argv);
    }
}

#pragma use_frame off

static void _MemFailure(void) {
    static char* fakeargs[] = {NULL};
    __argv = fakeargs;
    __argc = 0;
}

#pragma use_frame on

static int _MoreArgs(void) {
    char** newargs;
    if (__argc + 1 >= __maxargc) {
        __maxargc += 16;
        newargs = (char**)realloc(__argv, __maxargc * sizeof(char*));
        if (!newargs) {
            return 0;
        }
        __argv = newargs;
    }
    return 1;
}
static void _ExpandArgument(void) {
    WIN32_FIND_DATAA fd;
    HANDLE fh;
    char* fname = __argv[__argc - 1];
    char* fnptr;
    fnptr = fname + strlen(fname);
    while (fnptr > fname && strchr("/\\:", *(fnptr - 1)) == 0L) {
        fnptr--;
    }
    fh = FindFirstFileA(fname, &fd);
    if (fh != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                char* arg;
                arg = (char*)malloc((fnptr - fname + strlen(fd.cFileName) + 1) * sizeof(char));
                strncpy(arg, fname, (fnptr - fname));
                strcpy(arg + (fnptr - fname), fd.cFileName);
                if (GetFileAttributesA(arg) == 0xFFFFFFFF) {
                    char* ptr;
                    free(arg);
                    arg = (char*)malloc(
                        (fnptr - fname + strlen(fd.cAlternateFileName) + 1) * sizeof(char)
                    );
                    strncpy(arg, fname, (fnptr - fname));
                    for (ptr = fd.cAlternateFileName; *ptr; ptr++) {
                        *ptr = tolower(*ptr);
                    }
                    strcpy(arg + (fnptr - fname), fd.cAlternateFileName);
                }
                if (!_MoreArgs()) {
                    break;
                }
                __argv[__argc - 1] = arg;
                __argc++;
            }
        } while (FindNextFileA(fh, &fd));
        __argc--;
        FindClose(fh);
    } else {
    }
}
void _SetupArgs(void) {
    char* clptr;
    char* inptr;
    char* outptr;
    int wild;
    __maxargc = 1;
    __argc = 0;
    __argv = (char**)malloc(sizeof(char*) * __maxargc);
    __argv1_offs = 0;
    if (__argv == NULL) {
        _MemFailure();
        return;
    }
    _acmdln = inptr = clptr = GetCommandLineA();
    outptr = __command_line = (char*)malloc((strlen(inptr) + 1) * sizeof(char));
    if (!outptr) {
        _MemFailure();
        return;
    }
    while (strchr(" \t\r\n\v", *inptr) != NULL) {
        inptr++;
    }
    while (*inptr) {
        int quoting;
        if (!_MoreArgs()) {
            break;
        }
        wild = 0;
        if (__argc == 1) {
            __argv1_offs = inptr - clptr;
        }
        __argv[__argc] = outptr;
        __argc++;
        quoting = 0;
        while (*inptr) {
            if (!quoting) {
                if (strchr(" \t\r\n\v", *inptr) != NULL) {
                    do {
                        inptr++;
                    } while (*inptr && strchr(" \t\r\n\v", *inptr) != NULL);
                    *outptr++ = 0;
                    break;
                } else if (*inptr == '\"') {
                    inptr++;
                    quoting = 1;
                    continue;
                }
            } else {
                if (*inptr == '\"' && *(inptr + 1) == '\"') {
                    *outptr++ = '\"';
                    inptr += 2;
                    continue;
                } else if (*inptr == '\"') {
                    inptr++;
                    quoting = 0;
                    continue;
                }
            }
            if (*inptr == '\\' && *(inptr + 1) == '\\') {
                int count = 2;
                while (inptr[count] == '\\') {
                    count++;
                }
                if (inptr[count] == '\"') {
                    count &= ~1;
                    while (count) {
                        *outptr++ = *inptr;
                        inptr += 2;
                        count -= 2;
                    }
                } else {
                    while (count--) {
                        *outptr++ = *inptr++;
                    }
                }
            } else if (*inptr == '\\' && *(inptr + 1) == '\"') {
                *outptr++ = '\"';
                inptr += 2;
            } else {
                if (!quoting && (*inptr == '*' || *inptr == '?')) {
                    wild = 1;
                }
                *outptr++ = *inptr++;
            }
        }
        if (wild) {
            *outptr = 0;
            _ExpandArgument();
        }
    }
    *outptr = 0;
    if (__argc == 1) {
        __argv1_offs = inptr - clptr;
    }
    __argv[__argc] = NULL;
    atexit(_FreeCommandLine);
}
