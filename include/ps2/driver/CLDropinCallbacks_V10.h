#ifndef DRIVER_CLDROPINCALLBACKS_V10_H
#define DRIVER_CLDROPINCALLBACKS_V10_H

#include "compiler/common.h"
#include "driver/cmdline.h"
#include "driver/CWPlugins.h"
#include "driver/CWPluginsPrivate.h"
#include "driver/CLFiles.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 2)
typedef struct shellContextType {
    struct Plugin* plugin; // 0x0
    int cwplugin_api; // 0x4
    Boolean userAccessPathsChanged;
    Boolean systemAccessPathsChanged; // 0x9
    struct memAllocList** memory;
} shellContextType;
#pragma pack(pop)

extern CW_CALLBACK UCBFindAndLoadFile(
    CWPluginContext context, const char* infilename, IDEFileInfo* fileinfo
);
extern CW_CALLBACK UCBGetFileText(
    CWPluginContext context,
    const OSSpec* filespec,
    const char** text,
    SInt32* textLength,
    short* filedatatype
);
extern CW_CALLBACK UCBReleaseFileText(CWPluginContext context, const char* text);
extern CW_CALLBACK UCBSecretAttachHandle(
    CWPluginContext context, Handle handle, CWMemHandle* memHandle
);
extern CW_CALLBACK UCBSecretDetachHandle(
    CWPluginContext context, CWMemHandle memHandle, Handle* handle
);

#ifdef __cplusplus
}
#endif

#endif
