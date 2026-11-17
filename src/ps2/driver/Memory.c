#include "driver/Memory.h"
#include "driver/Generic.h"
#include "driver/MsDos.h"
#include "driver/oslib.h"
#include <stdio.h>
#include <stdlib.h>

static OSErr memError;

typedef struct HandleStructure {
    char* addr;
    UInt32 magic;
    OSHandle hand;
} HandleStructure;

#define HS(h) ((HandleStructure*)(h))

OSHandle* OS_PeekMacHandle(Handle h) {
    return &HS(h)->hand;
}

Handle OS_UnpeekMacHandle(UInt32* magic) {
    HandleStructure* hs = (HandleStructure*)((char*)magic - 4);
    if (hs->magic == 0xFEE1600D) {
        return (Handle)hs;
    }
    return 0;
}

static void FixHandlePtr(Handle h, void* ptr) {
    HS(h)->addr = ptr;
}

Handle OS_CreateMacHandle(OSHandle* h) {
    HandleStructure* hs = malloc(sizeof(HandleStructure));
    if (!hs) {
        memError = memFullErr;
        return 0;
    }

    hs->hand = *h;
    OS_InvalidateHandle(h);
    hs->addr = OS_LockHandle(&hs->hand);
    OS_UnlockHandle(&hs->hand);
    hs->magic = 0xFEE1600D;
    return (Handle)hs;
}

Handle OS_CreateMacHandleDup(OSHandle* h) {
    HandleStructure* hs = malloc(sizeof(HandleStructure));
    if (!hs) {
        memError = memFullErr;
        return 0;
    }
    OS_CopyHandle(h, &hs->hand);
    hs->addr = OS_LockHandle(&hs->hand);
    OS_UnlockHandle(&hs->hand);
    hs->magic = 0xFEE1600D;
    return (Handle)hs;
}

void OS_DestroyMacHandle(Handle h, OSHandle* ret) {
    *ret = *OS_PeekMacHandle(h);
    free(h);
}

OSErr MemError(void) {
    return memError;
}

Handle __stdcall NewHandle(SInt32 dataSize) {
    int err;
    OSHandle oh;

    err = OS_NewHandle(dataSize, &oh);
    if (err) {
        memError = OS_OSErrorToMacError(err);
        return 0;
    }

    return OS_CreateMacHandle(&oh);
}

void __stdcall HLock(Handle h) {
    void* buffer;

    if (h) {
        buffer = OS_LockHandle(&HS(h)->hand);
        FixHandlePtr(h, buffer);
        memError = 0;
    } else {
        memError = nilHandleErr;
    }
}

void __stdcall HUnlock(Handle h) {
    if (h) {
        memError = 0;
        OS_UnlockHandle(&HS(h)->hand);
    } else {
        memError = nilHandleErr;
    }
}

SInt32 __stdcall GetHandleSize(Handle h) {
    UInt32 sz;

    if (h) {
        memError = OS_OSErrorToMacError(OS_GetHandleSize(&HS(h)->hand, &sz));
    } else {
        sz = 0;
        memError = nilHandleErr;
    }

    return sz;
}

void __stdcall SetHandleSize(Handle h, SInt32 newSize) {
    OSHandle* oh;

    memError = OS_OSErrorToMacError(OS_ResizeHandle(oh = &HS(h)->hand, newSize));
    FixHandlePtr(h, OS_LockHandle(oh));
    OS_UnlockHandle(oh);
}

OSErr __stdcall PtrAndHand(const void* srcPtr, Handle destHandle, SInt32 size) {
    OSErr err;
    OSHandle* oh;

    err = OS_OSErrorToMacError(OS_AppendHandle(oh = &HS(destHandle)->hand, srcPtr, size));
    FixHandlePtr(destHandle, OS_LockHandle(oh));
    OS_UnlockHandle(oh);

    return err;
}

OSErr __stdcall HandAndHand(Handle src, Handle dst) {
    OSErr err;
    SInt32 size;

    if (src && HS(src)->magic != 0xFEE1600D) {
        memError = -113;
        return -113;
    }
    if (dst && HS(dst)->magic != 0xFEE1600D) {
        memError = -113;
        return -113;
    }

    HLock(src);
    size = GetHandleSize(src);
    err = PtrAndHand(*src, dst, size);
    HUnlock(src);
    return err;
}
