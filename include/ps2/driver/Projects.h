#ifndef DRIVER_PROJECTS_H
#define DRIVER_PROJECTS_H

#include "compiler/common.h"
#include "driver/oslib.h"
#include "driver/CWPlugins.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int GetFileCount(void);
extern void SetFileOutputName(SInt32 position, SInt16 which, char* outfilename);
extern int AddFileToProject(
    OSSpec* oss, SInt16 which, char* outfilename, Boolean exists, SInt32 position
);
extern Boolean GetFileInfo(SInt32 position, OSSpec* spec, char* plugin);
extern int AddAccessPath(OSPathSpec* oss, SInt16 type, SInt32 position, Boolean recursive);
extern int MoveSystemPathsIntoUserList(void);
extern void AddVirtualFile(const char* filename, Handle* text);
extern void SetOutputFileDirectory(OSPathSpec* dir);
extern void GetOutputFileDirectory(OSPathSpec* dir);
extern void AddOverlayGroup(const char* name, CWAddr64* addr, SInt32* groupnum, SInt32* overlaynum);
//! @reconstructed
extern void AddOverlay(SInt32 groupnum, const char* name, SInt32* overlaynum);
//! @reconstructed
extern void AddSegment(const char* name, SInt16 attrs, SInt32* segmentnum);
extern void ChangeSegment(SInt32 segmentnum, const char* name, SInt16 attrs);

#ifdef __cplusplus
}
#endif

#endif
