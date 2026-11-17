#ifndef DRIVER_GENERIC_H
#define DRIVER_GENERIC_H

#include "compiler/common.h"
#include "driver/oslib.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char STSbuf[260];
extern OSSpec* __stdcall OS_MatchPath(const char* path);
extern char* __stdcall OS_GetFileNamePtr(char* path);
extern char* __stdcall OS_GetDirName(const OSPathSpec* spec, char* buf, int size);
extern int __stdcall OS_MakeSpec2(const char* path, const char* filename, OSSpec* spec);
extern int __stdcall OS_MakeSpecWithPath(
    OSPathSpec* path, const char* filename, Boolean noRelative, OSSpec* spec
);
extern int __stdcall OS_NameSpecChangeExtension(OSNameSpec* spec, const char* ext, Boolean append);
extern int __stdcall OS_NameSpecSetExtension(OSNameSpec* spec, const char* ext);
extern char* __stdcall OS_CompactPaths(char* buf, const char* path, const char* name, int size);
extern char* __stdcall OS_SpecToStringRelative(
    const OSSpec* spec, const OSPathSpec* cwdspec, char* path, int size
);
extern int __stdcall OS_FindFileInPath(const char* filename, const char* plist, OSSpec* spec);
extern int __stdcall OS_FindProgram(const char* filename, OSSpec* spec);
extern int __stdcall OS_CopyHandle(OSHandle* hand, OSHandle* copy);
extern int __stdcall OS_AppendHandle(OSHandle* hand, const void* data, UInt32 len);

#ifdef __cplusplus
}
#endif

#endif
