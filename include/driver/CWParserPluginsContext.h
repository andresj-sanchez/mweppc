#ifndef DRIVER_CWPARSERPLUGINSCONTEXT_H
#define DRIVER_CWPARSERPLUGINSCONTEXT_H

#include "compiler/common.h"
#include "driver/cmdline.h"
#include "driver/CWPlugins.h"
#include "driver/DropInCompilerLinker.h"
#include "driver/CWPluginsPrivate.h"
#include "driver/MacSpecs.h"
#include "driver/oslib.h"

#ifdef __cplusplus
extern "C" {
#endif

extern CW_CALLBACK CWGetPluginRequest(CWPluginContext context, SInt32* request);
extern CW_CALLBACK CWSetCWPluginAPI(CWPluginContext context, SInt32 api);
extern CW_CALLBACK CWGetAPIVersion(CWPluginContext context, SInt32* version);
extern CW_CALLBACK CWGetProjectFile(CWPluginContext context, CWFileSpec* projectSpec);
extern CW_CALLBACK CWGetOutputFileDirectory(
    CWPluginContext context, CWFileSpec* outputFileDirectory
);
extern CW_CALLBACK CWGetTargetName(CWPluginContext context, char* name, short maxLength);
extern CW_CALLBACK CWGetProjectFileCount(CWPluginContext context, SInt32* count);
extern CW_CALLBACK CWGetFileInfo(
    CWPluginContext context,
    SInt32 whichfile,
    Boolean checkFileLocation,
    CWProjectFileInfo* fileinfo
);
extern CW_CALLBACK CWFindAndLoadFile(
    CWPluginContext context, const char* filename, CWFileInfo* fileinfo
);
extern CW_CALLBACK CWGetAccessPathListInfo(
    CWPluginContext context, CWAccessPathListInfo* pathListInfo
);
extern CW_CALLBACK CWGetFileText(
    CWPluginContext context,
    const CWFileSpec* filespec,
    const char** text,
    SInt32* textLength,
    short* filedatatype
);
extern CW_CALLBACK CWReleaseFileText(CWPluginContext context, const char* text);
extern CW_CALLBACK CWReportMessage(
    CWPluginContext context,
    const CWMessageRef* msgRef,
    const char* line1,
    const char* line2,
    short errorlevel,
    SInt32 errorNumber
);
extern CW_CALLBACK CWAlert(
    CWPluginContext context, const char* msg1, const char* msg2, const char* msg3, const char* msg4
);
extern CW_CALLBACK CWShowStatus(CWPluginContext context, const char* line1, const char* line2);
extern CW_CALLBACK CWUserBreak(CWPluginContext context);
extern CW_CALLBACK CWGetNamedPreferences(
    CWPluginContext context, const char* prefsname, CWMemHandle* prefsdata
);
extern CW_CALLBACK CWSetModDate(
    CWPluginContext context, const CWFileSpec* filespec, CWFileTime* moddate, Boolean isGenerated
);
extern CW_CALLBACK CWAddProjectEntry(
    CWPluginContext context,
    const CWFileSpec* fileSpec,
    Boolean isGenerated,
    const CWNewProjectEntryInfo* projectEntryInfo,
    SInt32* whichfile
);
extern CW_CALLBACK CWCreateNewTextDocument(
    CWPluginContext context, const CWNewTextDocumentInfo* docinfo
);
extern CW_CALLBACK CWAllocMemHandle(
    CWPluginContext context, SInt32 size, Boolean useTempMemory, CWMemHandle* handle
);
extern CW_CALLBACK CWFreeMemHandle(CWPluginContext context, CWMemHandle handle);
extern CW_CALLBACK CWLockMemHandle(
    CWPluginContext context, CWMemHandle handle, Boolean moveHi, void** ptr
);
extern CW_CALLBACK CWUnlockMemHandle(CWPluginContext context, CWMemHandle handle);
extern CW_CALLBACK CWDonePluginRequest(CWPluginContext context, CWResult resultCode);
extern CW_CALLBACK CWCheckoutLicense(
    CWPluginContext context,
    const char* featureName,
    const char* licenseVersion,
    SInt32 flags,
    void* reserved,
    SInt32* cookie
);
extern CW_CALLBACK CWCheckinLicense(CWPluginContext context, SInt32 cookie);

#ifdef __cplusplus
}
#endif

#endif
