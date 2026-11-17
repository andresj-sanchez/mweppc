#ifndef DRIVER_CLTARG_H
#define DRIVER_CLTARG_H

#include "compiler/common.h"
#include "driver/oslib.h"
#include "driver/CLAccessPaths.h"
#include "driver/CLDependencies.h"
#include "driver/CLFiles.h"
#include "driver/CLOverlays.h"
#include "driver/CLSegs.h"

#ifdef __cplusplus
extern "C" {
#endif

//! @reconstructed
typedef struct CLTargetInfo {
    OSType targetCPU; // 0x000
    OSType targetOS; // 0x004
    SInt16 outputType; // 0x008
    SInt16 linkType; // 0x00A
    Boolean canRun; // 0x00C
    Boolean canDebug; // 0x00D
    OSSpec outfile; // 0x00E
    OSSpec symfile; // 0x212
    OSSpec runfile; // 0x416
    OSSpec linkAgainstFile; // 0x61A
} CLTargetInfo;

enum {
    LinkModel_Flat = 0,
    LinkModel_Segmented = 1,
    LinkModel_Overlay1 = 2
};

typedef struct Plugin Plugin;

struct Target {
    struct BuildInfo {
        UInt32 linesCompiled;
        UInt32 codeSize;
        UInt32 iDataSize;
        UInt32 uDataSize;
    } info;
    CLTargetInfo* targetinfo;
    struct {
        Segments segs;
        Overlays overlays;
    } linkage;
    SInt32 linkmodel;
    Files files;
    Files pchs;
    Incls incls;
    Paths sysPaths;
    Paths userPaths;
    OSType lang;
    OSType cpu;
    OSType os;
    char targetName[64];
    Plugin* preLinker;
    Plugin* linker;
    Plugin* postLinker;
    UInt32 preLinkerDropinFlags;
    UInt32 linkerDropinFlags;
    UInt32 postLinkerDropinFlags;
    OSPathSpec outputDirectory;
    VFile* virtualFiles;
    struct Target* next;
};

typedef struct Target Target;

extern Target* Target_New(const char* name, OSType cpu, OSType os, OSType lang);
extern void Target_Free(Target* targ);
extern void Targets_Term(Target* list);
extern void Target_Add(Target** list, Target* targ);

#ifdef __cplusplus
}
#endif

#endif
