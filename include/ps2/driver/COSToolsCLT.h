#ifndef DRIVER_COSTOOLSCLT_H
#define DRIVER_COSTOOLSCLT_H

#include "compiler/common.h"
#include "driver/CWPlugins.h"

extern void COS_GetString(char* buffer, SInt16 strListID, SInt16 index);

extern void COS_FileFixDirectoryPath(char* path);
extern CWFileTime COS_GetTime(void);
extern void COS_FileGetPathName(char* buffer, const CWFileSpec* spec, CWFileTime* mdDat);

extern void COS_FileGetFileSpecInfo(
    void* ctx, SInt32 flags, SInt32 size, void* result, SInt32 limit
);

extern void* COS_ConvertToCWMemHandle(void*);

extern Boolean fn_00439B50(UInt32 length, void* start);

extern void COS_Init(CWPluginContext context);
extern void COS_Term(void);

extern Handle COSNewHandle(SInt32 byteCount, int system);

extern char* COS_PathGetFileName(const char* path);
extern SInt32 COS_GetTicks(void);

#endif
