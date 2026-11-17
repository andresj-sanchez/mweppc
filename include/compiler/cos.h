#ifndef COMPILER_COS_H
#define COMPILER_COS_H

#include "compiler/common.h"

extern Handle COS_NewOSHandle(SInt32 size);
extern Handle COS_NewHandle(SInt32 size);
extern void COS_FreeHandle(Handle handle);
extern Boolean COS_ResizeHandle(Handle handle, SInt32 size);
extern void COS_LockHandle(Handle handle);
extern void COS_LockHandleHi(Handle handle);
extern void COS_UnlockHandle(Handle handle);

struct CWWinFileSpec;
extern UInt32 COS_FileOpen(const struct CWWinFileSpec* spec, void** refNum);
extern UInt32 COS_FileGetSize(void* refNum, SInt32* size);
extern UInt32 COS_FileRead(void* refNum, void* data, SInt32 size);
extern void COS_FileClose(void* refNum);

#endif
