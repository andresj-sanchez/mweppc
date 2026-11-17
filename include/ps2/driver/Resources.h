#ifndef DRIVER_RESOURCES_H
#define DRIVER_RESOURCES_H

#include "compiler/common.h"
#include "driver/Memory.h"
#include "driver/MacSpecs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern Handle __stdcall GetResource(unsigned long theType, SInt16 theID);
extern void __stdcall ReleaseResource(Handle theResource);
extern Boolean OS_UsingMacResourceForkInfo(void);
extern void OS_UseMacResourceForkInfo(Boolean which);
extern OSErr SystemInitResources(OSSpec* spec);
extern void __stdcall OS_AddMacResourceForkRef(int ref, const OSSpec* spec);
extern void __stdcall OS_RemoveMacResourceForkRef(int ref);
extern OSSpec* __stdcall OS_GetMacResourceForkFromRef(int ref);
extern Boolean __stdcall OS_SearchMacResourceForkList(const OSSpec* rspec, int* ref);
extern int __stdcall OS_SetMacResourceForkCreatorAndType(int ref, OSType creator, OSType type);
extern int __stdcall OS_GetMacResourceForkCreatorAndType(int ref, OSType* creator, OSType* type);
extern void __stdcall OS_CleanupMacResourceForkDir(const OSPathSpec* dir);
extern OSErr OS_MacDumpResourceFork(SInt16 ref, Boolean dumpContents);
extern OSErr __stdcall FSpOpenRF(const FSSpec* fss, SInt8 permission, SInt16* refNum);
extern SInt16 __stdcall FSpOpenResFile(const FSSpec* spec, SInt8 permission);
extern void __stdcall CloseResFile(SInt16 refNum);

#ifdef __cplusplus
}
#endif

#endif
