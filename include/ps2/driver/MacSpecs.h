#ifndef DRIVER_MACSPECS_H
#define DRIVER_MACSPECS_H

#include "compiler/common.h"
#include "driver/oslib.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 2)
typedef struct FSSpec {
    SInt16 vRefNum;
    SInt32 parID;
    UInt8 name[64];
} FSSpec;
#pragma pack(pop)

extern int __stdcall OS_OSPathSpec_To_VolDir(
    const OSPathSpec* spec, SInt16* vRefNum, SInt32* dirID
);
extern int __stdcall OS_OSSpec_To_FSSpec(const OSSpec* spec, FSSpec* fss);
extern int __stdcall OS_VolDir_To_OSPathSpec(SInt16 vRefNum, SInt32 dirID, OSPathSpec* spec);
extern int __stdcall OS_FSSpec_To_OSSpec(const FSSpec* fss, OSSpec* spec);
extern int __stdcall OS_GetRsrcOSSpec(const OSSpec* spec, OSSpec* rspec, Boolean create);

#ifdef __cplusplus
}
#endif

#endif
