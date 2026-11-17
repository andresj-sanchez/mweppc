#include "driver/CLLoadAndCache.h"
#include "driver/CLIncludeFileCache.h"
#include "driver/MacFileTypes.h"
#include "driver/MemUtils.h"
#include "driver/MsDos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void CopyFileText(OSHandle* src, char** text, SInt32* textsize) {
    char* tptr;

    OS_GetHandleSize(src, (UInt32*)textsize);
    *text = xmalloc(NULL, *textsize);

    tptr = OS_LockHandle(src);
    memcpy(*text, tptr, *textsize);
    *textsize -= 1;
    OS_UnlockHandle(src);
}

int LoadAndCacheFile(OSSpec* spec, OSHandle** texthandle, Boolean* precomp) {
    OSHandle* h;
    int err;
    OSType mactype;
    int refnum;
    UInt32 ltxtlen;
    char* tptr;

    if ((*texthandle = CachedIncludeFile(spec, precomp))) {
        return 0;
    }

    *precomp = OS_GetMacFileType(spec, &mactype) == 0 && mactype != 'TEXT';
    *texthandle = NULL;

    err = OS_Open(spec, OSReadOnly, &refnum);
    if (err) {
        return err;
    }
    err = OS_GetSize(refnum, &ltxtlen);
    if (err) {
        return err;
    }

    h = xmalloc(NULL, sizeof(OSHandle));
    if (OS_NewHandle(ltxtlen + 1, h)) {
        fprintf(stderr, "\n*** Out of memory\n");
        exit(-23);
    }

    tptr = OS_LockHandle(h);
    err = OS_Read(refnum, tptr, &ltxtlen);
    if (err) {
        OS_FreeHandle(h);
        xfree(h);
        OS_Close(refnum);
        return err;
    }

    OS_Close(refnum);
    tptr[ltxtlen] = 0;
    OS_UnlockHandle(h);

    if (!*precomp) {
        FixTextHandle(h);
    }

    CacheIncludeFile(spec, h, *precomp);
    *texthandle = h;
    return 0;
}

short FixTextHandle(OSHandle* txt) {
    char* scan;
    UInt32 tlen;
    char* tptr;

    tptr = OS_LockHandle(txt);
    OS_GetHandleSize(txt, &tlen);

    scan = tptr;
    while (scan < (tptr + tlen)) {
        if (scan[0] == '\r') {
            if (scan[1] == '\n') {
                scan += 2;
            } else {
                scan += 1;
            }
        } else if (scan[0] == '\n') {
            scan[0] = '\r';
            scan++;
        } else {
            scan++;
        }
    }

    OS_UnlockHandle(txt);
    return 0;
}
