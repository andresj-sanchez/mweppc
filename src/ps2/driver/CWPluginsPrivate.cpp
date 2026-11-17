#include "driver/CWPluginsPrivateExtended.h"
#include "driver/CWPlugins.h"
#include "driver/CLPluginAPI.h"
#include "driver/oslib.h"
#include "driver/MsDos.h"
#include <string.h>

extern "C" {

static Boolean ValidateContext(CWPluginContext context) {
    return (context && context->shellSignature == 'CWIE') &&
        !(context->request == reqInitialize ||
          context->request == reqTerminate ||
          context->request == reqIdle);
}

static Boolean ValidateInitTermContext(CWPluginContext context) {
    return context &&
        (context->shellSignature == 'CWIE') &&
        ((context->request == reqInitialize) ||
         (context->request == reqTerminate) ||
         (context->request == reqIdle));
}

static Boolean IsVCSContext(CWPluginContext context) {
    return context && (context->pluginType == CWDROPINVCSTYPE);
}

void ConvertFSSpecToNativePath(CWPluginContext context, const OSSpec* oss, CWFileSpec* fss) {
    CL_OSSpecToCWFileSpec(CL_PluginAPI(context), oss, fss);
}

void ConvertNativePathToFSSpec(CWPluginContext context, const CWFileSpec* fss, OSSpec* oss) {
    CL_CWFileSpecToOSSpec(CL_PluginAPI(context), fss, oss);
}

void ConvertMacTimeToNativeTime(CWPluginContext context, OSTime time, CWFileTime* ft) {
    CL_OSTimeToCWFileTime(CL_PluginAPI(context), &time, ft);
}

void ConvertNativeTimeToMacTime(CWPluginContext context, const CWFileTime* ft, OSTime* time) {
    CL_CWFileTimeToOSTime(CL_PluginAPI(context), ft, time);
}
}

template <typename FileInfo>
void ConvertIDEFileInfoToOtherFileInfo(CWPluginContext context, IDEFileInfo& from, FileInfo* to) {
    to->filedata = from.filedata;
    to->filedatalength = from.filedatalength;
    to->filedatatype = from.filedatatype;
    to->fileID = from.fileID;
    ConvertFSSpecToNativePath(
        context, &from.filespec, reinterpret_cast<CWFileSpec*>(&to->filespec)
    );
    to->alreadyincluded = from.alreadyincluded;
    to->recordbrowseinfo = from.recordbrowseinfo;
}

template <typename ProjectFileInfo>
void ConvertIDEProjectFileInfoToOtherProjectFileInfo(
    CWPluginContext context, IDEProjectFileInfo& from, ProjectFileInfo* to
) {
    ConvertFSSpecToNativePath(
        context, &from.filespec, reinterpret_cast<CWFileSpec*>(&to->filespec)
    );
    ConvertMacTimeToNativeTime(context, from.moddate, reinterpret_cast<CWFileTime*>(&to->moddate));
    to->segment = from.segment;
    to->hasobjectcode = from.hasobjectcode;
    to->hasresources = from.hasresources;
    to->isresourcefile = from.isresourcefile;
    to->weakimport = from.weakimport;
    to->initbefore = from.initbefore;
    to->gendebug = from.gendebug;
    ConvertMacTimeToNativeTime(
        context, from.objmoddate, reinterpret_cast<CWFileTime*>(&to->objmoddate)
    );
    OS_NameSpecToString(&from.dropinname, to->dropinname, MAX_PATH);
    to->fileID = from.fileID;
    to->recordbrowseinfo = from.recordbrowseinfo;
    to->filetype = from.filetype;
    to->filecreator = from.filecreator;
    to->hasunitdata = from.hasunitdata;
    to->mergeintooutput = from.mergeintooutput;
    to->unitdatadependencytag = from.unitdatadependencytag;
}

extern "C" {

CW_CALLBACK CWGetPluginRequest(CWPluginContext context, SInt32* request) {
    if (!ValidateContext(context) && !ValidateInitTermContext(context)) {
        return cwErrInvalidParameter;
    }
    if (request == NULL) {
        return cwErrInvalidParameter;
    }

    *request = context->request;
    return cwNoErr;
}

CW_CALLBACK CWGetAPIVersion(CWPluginContext context, SInt32* version) {
    if (!ValidateContext(context) && !ValidateInitTermContext(context)) {
        return cwErrInvalidParameter;
    }
    if (version == NULL) {
        return cwErrInvalidParameter;
    }

    *version = context->apiVersion;
    return cwNoErr;
}

CW_CALLBACK CWGetProjectFile(CWPluginContext context, CWFileSpec* projectSpec) {
    if (IsVCSContext(context) || !ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    if (projectSpec == NULL) {
        return cwErrInvalidParameter;
    }

    ConvertFSSpecToNativePath(context, &context->projectFile, projectSpec);
    return cwNoErr;
}

CW_CALLBACK CWGetTargetName(CWPluginContext context, char* name, short maxLength) {
    if (IsVCSContext(context) || !ValidateContext(context)) {
        return cwErrInvalidParameter;
    }

    if (context->apiVersion >= 8) {
        return context->callbacks->cbGetTargetName(context, name, maxLength);
    } else {
        return cwErrRequestFailed;
    }
}

CW_CALLBACK CWGetProjectFileCount(CWPluginContext context, SInt32* count) {
    if (IsVCSContext(context) || !ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    if (count == NULL) {
        return cwErrInvalidParameter;
    }

    *count = context->numFiles;
    return cwNoErr;
}

CW_CALLBACK CWGetFileInfo(
    CWPluginContext context,
    SInt32 whichfile,
    Boolean checkFileLocation,
    CWProjectFileInfo* fileinfo
) {
    if (IsVCSContext(context) || !ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    if (fileinfo == NULL) {
        return cwErrInvalidParameter;
    }

    IDEProjectFileInfo info;
    CWResult result;

    memset(&info, 0, sizeof(info));
    result = context->callbacks->cbGetFileInfo(context, whichfile, checkFileLocation, &info);
    if (result != cwNoErr) {
        return result;
    }
    if (CL_PluginAPI(context) == 1) {
        ConvertIDEProjectFileInfoToOtherProjectFileInfo(
            context, info, reinterpret_cast<CWMacProjectFileInfo*>(fileinfo)
        );
    } else {
        ConvertIDEProjectFileInfoToOtherProjectFileInfo(context, info, fileinfo);
    }
    return cwNoErr;
}

CW_CALLBACK CWFindAndLoadFile(CWPluginContext context, const char* filename, CWFileInfo* fileinfo) {
    if (IsVCSContext(context) || !ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    if (filename == NULL) {
        return cwErrInvalidParameter;
    }
    if (fileinfo == NULL) {
        return cwErrInvalidParameter;
    }

    IDEFileInfo info;
    CWResult result;

    memset(&info, 0, sizeof(info));
    info.fullsearch = fileinfo->fullsearch;
    info.dependencyType = fileinfo->dependencyType;
    info.isdependentoffile = fileinfo->isdependentoffile;
    info.suppressload = fileinfo->suppressload;
    result = context->callbacks->cbFindAndLoadFile(context, filename, &info);
    if (result != cwNoErr) {
        return result;
    }
    if (CL_PluginAPI(context) == 1) {
        ConvertIDEFileInfoToOtherFileInfo(
            context, info, reinterpret_cast<CWMacFileInfo*>(fileinfo)
        );
    } else {
        ConvertIDEFileInfoToOtherFileInfo(context, info, fileinfo);
    }
    return cwNoErr;
}

static CWResult EnsureCachedAccessPaths(CWPluginContext context) {
    if (!context->accessPathList) {
        CWResult res = context->callbacks->cbCacheAccessPathList(context);
        if (res) {
            return res;
        }
        if (!context->accessPathList) {
            return cwErrRequestFailed;
        }
    }
    return cwNoErr;
}

CW_CALLBACK CWGetAccessPathListInfo(CWPluginContext context, CWAccessPathListInfo* pathListInfo) {
    if (!ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    if (pathListInfo == NULL) {
        return cwErrInvalidParameter;
    }
    if (context->apiVersion < 10) {
        return cwErrInvalidCallback;
    }
    CWResult res = EnsureCachedAccessPaths(context);
    if (res) {
        return res;
    }
    pathListInfo->systemPathCount = context->accessPathList->systemPathCount;
    pathListInfo->userPathCount = context->accessPathList->userPathCount;
    pathListInfo->alwaysSearchUserPaths = context->accessPathList->alwaysSearchUserPaths;
    pathListInfo->convertPaths = context->accessPathList->convertPaths;
    return cwNoErr;
}

CW_CALLBACK CWGetFileText(
    CWPluginContext context,
    const CWFileSpec* filespec,
    const char** text,
    SInt32* textLength,
    short* filedatatype
) {
    if (!ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    if (!filespec) {
        return cwErrInvalidParameter;
    }
    if (!text) {
        return cwErrInvalidParameter;
    }
    if (!textLength) {
        return cwErrInvalidParameter;
    }
    if (!filedatatype) {
        return cwErrInvalidParameter;
    }
    OSSpec spec;
    ConvertNativePathToFSSpec(context, filespec, &spec);
    return context->callbacks->cbGetFileText(context, &spec, text, textLength, filedatatype);
}

CW_CALLBACK CWAlert(
    CWPluginContext context, const char* msg1, const char* msg2, const char* msg3, const char* msg4
) {
    if (!ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    return context->callbacks->cbAlert(context, msg1, msg2, msg3, msg4);
}

CW_CALLBACK CWShowStatus(CWPluginContext context, const char* line1, const char* line2) {
    if (!ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    return context->callbacks->cbShowStatus(context, line1, line2);
}

CW_CALLBACK CWUserBreak(CWPluginContext context) {
    if (!ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    return context->callbacks->cbUserBreak(context);
}

CW_CALLBACK CWGetNamedPreferences(
    CWPluginContext context, const char* prefsname, CWMemHandle* prefsdata
) {
    if (!ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    if (!prefsname) {
        return cwErrInvalidParameter;
    }
    if (!prefsdata) {
        return cwErrInvalidParameter;
    }
    return context->callbacks->cbGetNamedPreferences(context, prefsname, prefsdata);
}

CW_CALLBACK CWSetModDate(
    CWPluginContext context, const CWFileSpec* filespec, CWFileTime* moddate, Boolean isGenerated
) {
    if (IsVCSContext(context) || !ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    if (!filespec) {
        return cwErrInvalidParameter;
    }
    OSSpec spec;
    OSTime time;
    ConvertNativePathToFSSpec(context, filespec, &spec);
    ConvertNativeTimeToMacTime(context, moddate, &time);
    return context->callbacks->cbSetModDate(context, &spec, &time, isGenerated);
}

CW_CALLBACK CWAddProjectEntry(
    CWPluginContext context,
    const CWFileSpec* fileSpec,
    Boolean isGenerated,
    const CWNewProjectEntryInfo* projectEntryInfo,
    SInt32* whichfile
) {
    if (IsVCSContext(context) || !ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    if (!fileSpec) {
        return cwErrInvalidParameter;
    }
    if (context->apiVersion < 8) {
        return cwErrRequestFailed;
    }
    OSSpec spec;
    ConvertNativePathToFSSpec(context, fileSpec, &spec);
    return context->callbacks->cbAddProjectEntry(
        context, &spec, isGenerated, projectEntryInfo, whichfile
    );
}

CW_CALLBACK CWCreateNewTextDocument(CWPluginContext context, const CWNewTextDocumentInfo* docinfo) {
    if (!ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    if (!docinfo) {
        return cwErrInvalidParameter;
    }
    return context->callbacks->cbCreateNewTextDocument(context, docinfo);
}

CW_CALLBACK CWAllocMemHandle(
    CWPluginContext context, SInt32 size, Boolean useTempMemory, CWMemHandle* handle
) {
    if (!ValidateContext(context) && !ValidateInitTermContext(context)) {
        return cwErrInvalidParameter;
    }
    if (!handle) {
        return cwErrInvalidParameter;
    }
    return context->callbacks->cbAllocMemHandle(context, size, useTempMemory, handle);
}

CW_CALLBACK CWFreeMemHandle(CWPluginContext context, CWMemHandle handle) {
    if (!ValidateContext(context) && !ValidateInitTermContext(context)) {
        return cwErrInvalidParameter;
    }
    return context->callbacks->cbFreeMemHandle(context, handle);
}

CW_CALLBACK CWLockMemHandle(
    CWPluginContext context, CWMemHandle handle, Boolean moveHi, void** ptr
) {
    if (!ValidateContext(context) && !ValidateInitTermContext(context)) {
        return cwErrInvalidParameter;
    }
    if (!ptr) {
        return cwErrInvalidParameter;
    }
    return context->callbacks->cbLockMemHandle(context, handle, moveHi, ptr);
}

CW_CALLBACK CWUnlockMemHandle(CWPluginContext context, CWMemHandle handle) {
    if (!ValidateContext(context) && !ValidateInitTermContext(context)) {
        return cwErrInvalidParameter;
    }
    return context->callbacks->cbUnlockMemHandle(context, handle);
}

CW_CALLBACK CWDonePluginRequest(CWPluginContext, CWResult resultCode) {
    return resultCode;
}

CW_CALLBACK CWCheckoutLicense(
    CWPluginContext context,
    const char* featureName,
    const char* licenseVersion,
    SInt32 flags,
    void* reserved,
    SInt32* cookie
) {
    if (!ValidateContext(context) && !ValidateInitTermContext(context)) {
        return cwErrInvalidParameter;
    }
    return context->callbacks->cbCheckoutLicense(
        context, featureName, licenseVersion, flags, reserved, cookie
    );
}

CW_CALLBACK CWCheckinLicense(CWPluginContext context, SInt32 cookie) {
    if (!ValidateContext(context) && !ValidateInitTermContext(context)) {
        return cwErrInvalidParameter;
    }
    return context->callbacks->cbCheckinLicense(context, cookie);
}
}
