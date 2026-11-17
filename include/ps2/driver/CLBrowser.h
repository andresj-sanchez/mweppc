#ifndef DRIVER_CLBROWSER_H
#define DRIVER_CLBROWSER_H

#include "compiler/common.h"
#include "driver/Memory.h"
#include "driver/MsDos.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int Browser_Initialize(OSHandle* browsetableptr);
extern int Browser_Terminate(OSHandle* browsetableptr);
extern int Browser_SearchFile(OSHandle* browsetable, const char* fullpath, SInt16* ID);
extern int Browser_SearchAndAddFile(OSHandle* browsetable, const char* fullpath, SInt16* ID);
extern int Browser_PackBrowseFile(
    OSHandle* browsedata, OSHandle* browsetable, OSHandle* browsefileptr
);

#ifdef __cplusplus
}
#endif

#endif
