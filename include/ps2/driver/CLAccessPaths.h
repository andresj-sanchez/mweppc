#ifndef DRIVER_CLACCESSPATHS_H
#define DRIVER_CLACCESSPATHS_H

#include "compiler/common.h"
#include "driver/cmdline.h"
#include "driver/oslib.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Path {
    OSPathSpec* spec;
    struct Paths* recursive;
    SInt16 flags;
};
struct Paths {
    Path** pathsArray;
    UInt16 arraySize;
    UInt16 pathsCount;
};

extern Path* Path_Init(const OSPathSpec* dir, Path* path);
extern Path* Path_New(const OSPathSpec* dir);
extern void Path_Free(Path* path);
extern Boolean Paths_Initialize(Paths* paths);
extern Boolean Paths_Terminate(Paths* paths);
extern Boolean Paths_AddPath(Paths* paths, Path* path);
extern Boolean Paths_InsertPath(Paths* paths, UInt16 index, Path* path);
extern Boolean Paths_RemovePath(Paths* paths, UInt16 index);
extern Boolean Paths_DeletePath(Paths* paths, UInt16 index);
extern Path* Paths_GetPath(Paths* paths, UInt16 pathnum);
extern UInt16 Paths_Count(const Paths* paths);
extern Boolean Paths_FindPath(const Paths* paths, const Path* path);
//! @reconstructed
extern Path* Paths_FindPathSpec(const Paths* paths, const OSPathSpec* dir);
extern Boolean Paths_GatherRecurse(Path* path);
extern int Paths_CountRecurse(Paths* paths);
extern void Paths_CopyRecurseSpecs(OSSpec* list, Paths* paths, UInt16 count);

#ifdef __cplusplus
}
#endif

#endif
