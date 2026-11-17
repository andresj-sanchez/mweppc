#include "driver/MacSpecs.h"
#include "driver/CLIO.h"
#include "driver/CWPlugins.h"
#include "driver/CLPlugins.h"
#include "driver/CLFiles.h"
#include "driver/CWParserPluginsContext.h"
#include "driver/CLTarg.h"
#include "driver/CLPluginRequests.h"
#include "driver/CLDependencies.h"
#include "driver/CLDropinCallbacks_V10.h"
#include "driver/CLErrors.h"
#include "driver/CLFileOps.h"
#include "driver/CLIncludeFileCache.h"
#include "driver/CLMain.h"
#include "driver/cmdline.h"
#include "driver/DropInCompilerLinker.h"
#include "driver/ErrMgr.h"
#include "driver/FileHandles.h"
#include "driver/Generic.h"

#include "driver/Memory.h"
#include "driver/MemUtils.h"
#include "driver/MsDos.h"
#include "driver/oslib.h"
#include "driver/pref_structs.h"
#include "driver/ParserHelpers.h"
#include "driver/Projects.h"
#include "driver/ToolHelpers.h"
#include <stdlib.h>
#include <string.h>

extern "C" {

CW_CALLBACK UCBCachePrecompiledHeader(
    CWPluginContext context, const OSSpec* filespec, CWMemHandle pchhandle
) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBCachePrecompiledHeader");
    }

    OSSpec spec;
    if (!PluginFindMemoryRef(context, pchhandle, TRUE)) {
        return cwErrInvalidParameter;
    }
    spec = *filespec;
    CacheIncludeFile(&spec, reinterpret_cast<OSHandle*>(pchhandle), TRUE);
    PluginRemoveMemoryRef(context, pchhandle, TRUE);
    return cwNoErr;
}

CW_CALLBACK UCBLoadObjectData(CWPluginContext context, SInt32 whichfile, CWMemHandle* objectdata) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBLoadObjectData");
    }

    *objectdata = NULL;
    if (whichfile == -1) {
        whichfile = static_cast<CWCompilerLinkerContext*>(context)->whichfile;
    }
    File* file = Files_GetFile(&gTarg->files, whichfile);
    if (!file) {
        return cwErrUnknownFile;
    }

    if (!file->objectdata && !RetrieveObjectData(file)) {
        return cwErrRequestFailed;
    }
    if (!file->objectdata) {
        return cwErrRequestFailed;
    }
    *objectdata = reinterpret_cast<CWMemHandle>(file->objectdata);
    if (!PluginFindMemoryRef(context, file->objectdata, TRUE)) {
        PluginAddMemoryRef(context, file->objectdata, TRUE, TRUE, FALSE, FALSE);
    }
    return cwNoErr;
}

CW_CALLBACK UCBStoreObjectData(CWPluginContext context, SInt32 whichfile, IDEObjectData* object) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBStoreObjectData");
    }

    if (CheckForUserBreak()) {
        return cwErrUserCanceled;
    }

    if (whichfile == -1) {
        whichfile = static_cast<CWCompilerLinkerContext*>(context)->whichfile;
    }
    File* filedata = Files_GetFile(&gTarg->files, whichfile);
    if (!filedata) {
        return cwErrUnknownFile;
    }

    if (!object->objectfile) {
        if (filedata->objectdata &&
            filedata->objectdata != reinterpret_cast<OSHandle*>(object->objectdata)) {
            OS_FreeHandle(filedata->objectdata);
        }
        if (filedata->browsedata &&
            filedata->browsedata != reinterpret_cast<OSHandle*>(object->browsedata)) {
            OS_FreeHandle(filedata->browsedata);
        }
        filedata->objectdata = reinterpret_cast<OSHandle*>(object->objectdata);
        filedata->browsedata = reinterpret_cast<OSHandle*>(object->browsedata);
        PluginRemoveMemoryRef(context, filedata->objectdata, TRUE);
        PluginRemoveMemoryRef(context, filedata->browsedata, TRUE);
        filedata->unk868 = FALSE;
    } else {
        if (filedata->outfileowner && filedata->outfileowner != CmdLineStageMask_Cg) {
            CLInternalError(
                "CLCompilerLinkerDropin_V10.cpp",
                240,
                "Cannot store object file spec for '%s'\n",
                filedata->srcfilename
            );
        }
        filedata->outfss = *object->objectfile;
        filedata->wroteToDisk |= CmdLineStageMask_Cg;
    }

    filedata->codesize = object->codesize;
    filedata->udatasize = object->udatasize;
    filedata->idatasize = object->idatasize;
    filedata->compiledlines = object->compiledlines;
    if (filedata->dropinflags & kGeneratesrsrcs) {
        filedata->hasresources = TRUE;
    } else {
        filedata->hasobjectcode = TRUE;
    }
    filedata->recompileDependents = object->dependencyCount && object->interfaceChanged;
    OS_GetTime(&filedata->outmoddate);
    if (object->reserved1) {
        filedata->filetype = object->reserved1;
    }

    if (object->reserved2) {
        char* newname = reinterpret_cast<char*>(object->reserved2);
        IDEFileInfo fi;
        if (OS_MakeSpec(newname, &filedata->srcfss, NULL) || OS_Status(&filedata->srcfss)) {
            fi.fullsearch = TRUE;
            fi.dependencyType = cwNoDependency;
            fi.isdependentoffile = -1;
            fi.suppressload = TRUE;
            if (UCBFindAndLoadFile(context, newname, &fi) == cwNoErr) {
                filedata->srcfss = fi.filespec;
            } else {
                char* fnptr = OS_GetFileNamePtr(newname);
                if (fnptr != newname) {
                    fi.fullsearch = TRUE;
                    fi.dependencyType = cwNoDependency;
                    fi.isdependentoffile = -1;
                    fi.suppressload = TRUE;
                    if (UCBFindAndLoadFile(context, fnptr, &fi) == cwNoErr) {
                        filedata->srcfss = fi.filespec;
                    }
                }
            }
        }
    }

    return cwNoErr;
}

CW_CALLBACK UCBFreeObjectData(CWPluginContext context, SInt32 whichfile, CWMemHandle objectdata) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBFreeObjectData");
    }

    File* file = Files_GetFile(&gTarg->files, whichfile);
    if (!file) {
        return cwErrUnknownFile;
    }

    PluginRemoveMemoryRef(context, file->objectdata, TRUE);
    FlushObjectData(file);
    return cwNoErr;
}

CW_CALLBACK UCBDisplayLines(CWPluginContext context, SInt32 nlines) {
    ShowWorking(12);

    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBDisplayLines");
    }

    if (CheckForUserBreak()) {
        return cwErrUserCanceled;
    } else {
        return cwNoErr;
    }
}

CW_CALLBACK UCBBeginSubCompile(
    CWPluginContext context, SInt32 whichfile, CWPluginContext* subContext
) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBBeginSubCompile");
    }

    CLInternalError("CLCompilerLinkerDropin_V10.cpp", 372, "UCBBeginSubCompile not implemented");
    return cwErrRequestFailed;
}

CW_CALLBACK UCBEndSubCompile(CWPluginContext subContext) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBEndSubCompile");
    }

    CLInternalError("CLCompilerLinkerDropin_V10.cpp", 384, "UCBEndSubCompile not implemented");
    return cwErrRequestFailed;
}

CW_CALLBACK UCBGetPrecompiledHeaderSpec(
    CWPluginContext context, OSSpec* pchspec, const char* intarget
) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBGetPrecompiledHeaderSpec");
    }

    CWCompilerLinkerContext* c;
    int err;
    OSSpec outfss;
    File* file;
    const CWObjectFlags* cof;
    char target[260];

    if (context->pluginType == CWDROPINCOMPILERTYPE || context->pluginType == CWDROPINLINKERTYPE) {
        c = static_cast<CWCompilerLinkerContext*>(context);
    } else {
        return cwErrInvalidCallback;
    }

    file = Files_GetFile(&gTarg->files, c->whichfile);
    OS_ASSERT(420, file != NULL);

    cof = Plugin_CL_GetObjectFlags(file->compiler);
    if (!file->outfilename[0]) {
        if (intarget) {
            if (optsCompiler.canonicalIncludes) {
                strcpy(target, intarget);
            } else if (OS_CanonPath(intarget, target) != noErr) {
                return cwErrInvalidParameter;
            }

            err = OS_MakeSpecWithPath(&file->srcfss.path, target, FALSE, &outfss);
            if (err) {
                CLReportOSError(CLStr97, err, target);
                return cwErrRequestFailed;
            }

            if (!file->outfileowner || file->outfileowner == CmdLineStageMask_Cg) {
                file->outfileowner = CmdLineStageMask_Cg;
                OS_SpecToStringRelative(&outfss, NULL, file->outfilename, sizeof(target));
            }

            if (optsCmdLine.verbose) {
                CLReport(
                    CLStr16,
                    "precompiled ",
                    OS_SpecToStringRelative(&outfss, NULL, STSbuf, sizeof(STSbuf))
                );
            }
        } else {
            OS_MakeSpecWithPath(
                &gTarg->outputDirectory,
                file->srcfilename,
                !optsCompiler.relPathInOutputDir,
                &outfss
            );
            OS_NameSpecSetExtension(
                &outfss.name, optsCompiler.pchFileExt[0] ? optsCompiler.pchFileExt : cof->pchFileExt
            );

            if (!file->outfileowner || file->outfileowner == CmdLineStageMask_Cg) {
                file->outfileowner = CmdLineStageMask_Cg;
                OS_NameSpecToString(&outfss.name, file->outfilename, sizeof(target));
            }

            CLReport(CLStr59, OS_SpecToStringRelative(&outfss, NULL, STSbuf, sizeof(STSbuf)));
        }

        *pchspec = outfss;
    } else {
        err = OS_MakeSpecWithPath(&gTarg->outputDirectory, file->outfilename, FALSE, &outfss);
        if (err) {
            CLReportOSError(CLStr97, err, file->outfilename);
            return cwErrRequestFailed;
        }

        *pchspec = outfss;

        if (intarget) {
            CLReport(
                CLStr60, OS_SpecToStringRelative(&outfss, NULL, STSbuf, sizeof(STSbuf)), intarget
            );
        }
    }

    return cwNoErr;
}

CW_CALLBACK UCBGetResourceFile(CWPluginContext context, OSSpec* filespec) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBGetResourceFile");
    }

    CLInternalError("CLCompilerLinkerDropin_V10.cpp", 514, "UCBGetResourceFile not implemented");
    return cwErrRequestFailed;
}

CW_CALLBACK UCBPutResourceFile(
    CWPluginContext context, const char* prompt, const char* name, OSSpec* filespec
) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBPutResourceFile");
    }

    CLInternalError("CLCompilerLinkerDropin_V10.cpp", 529, "UCBPutResourceFile not implemented");
    return cwErrRequestFailed;
}

static int UnitNameToSBMName(OSSpec* spec, File* srcfile) {
    const CWObjectFlags* cof;
    int err;

    cof = Plugin_CL_GetObjectFlags(srcfile->compiler);
    err = OS_NameSpecChangeExtension(
        &spec->name,
        cof->pchFileExt ? cof->pchFileExt : ".sbm",
        cof->pchFileExt ? (cof->pchFileExt[0] == '.') : FALSE
    );
    return err;
}

CW_CALLBACK UCBLookUpUnit(
    CWPluginContext context,
    const char* name,
    Boolean isdependency,
    const void** unitdata,
    SInt32* unitdatalength
) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBLookUpUnit");
    }

    IDEFileInfo includeinfo;
    OSSpec unitspec;
    OSSpec sbmspec;
    OSTime unittime;
    UInt32 unitmactime;
    char sbmpath[260];
    int err;
    CWCompilerLinkerContext* c;
    File* srcfile;
    File* sbmfile;

    if (context->pluginType == CWDROPINCOMPILERTYPE || context->pluginType == CWDROPINLINKERTYPE) {
        c = static_cast<CWCompilerLinkerContext*>(context);
    } else {
        return cwErrInvalidCallback;
    }

    srcfile = Files_GetFile(&gTarg->files, c->whichfile);
    OS_ASSERT(591, srcfile != NULL);

    *unitdata = NULL;
    *unitdatalength = 0;

    if (optsCompiler.sbmState == OptsCompilerSbmState_Rebuild ||
        optsCompiler.sbmState == OptsCompilerSbmState_Clean) {
        if (clState.pluginDebug) {
            CLPrint("UCBLookUpUnit:  sbmState == sbmRebuild or sbmClean; failing\n");
        }
        return cwErrSBMNotFound;
    }

    includeinfo.fullsearch = TRUE;
    includeinfo.dependencyType = isdependency ? cwNormalDependency : cwNoDependency;
    includeinfo.isdependentoffile = -1;
    includeinfo.suppressload = TRUE;
    if (UCBFindAndLoadFile(context, name, &includeinfo) != cwNoErr) {
        if (clState.pluginDebug) {
            CLPrint("UCBLookUpUnit:  could not find source '%s'; failing\n", name);
        }
        return cwErrFileNotFound;
    }

    unitspec = includeinfo.filespec;
    sbmspec = unitspec;
    err = UnitNameToSBMName(&sbmspec, srcfile);
    if (err) {
        if (clState.pluginDebug) {
            CLPrint(
                "UCBLookUpUnit:  could not make precompiled unit spec for '%s' (%s)\n",
                name,
                OS_GetErrText(err)
            );
        }
        return cwErrRequestFailed;
    }

    if (optsCompiler.sbmPath[0]) {
        sbmspec.path = clState.sbmPathSpec;
        OS_SpecToString(&sbmspec, sbmpath, sizeof(sbmpath));
        if (clState.pluginDebug) {
            CLPrint("UCBLookUpUnit:  Only looking at '%s'\n", sbmpath);
        }
    } else {
        OS_NameSpecToString(&sbmspec.name, sbmpath, sizeof(sbmpath));
        if (clState.pluginDebug) {
            CLPrint("UCBLookUpUnit:  searching paths for '%s'\n", sbmpath);
        }
    }

    includeinfo.fullsearch = TRUE;
    includeinfo.dependencyType = isdependency ? cwNormalDependency : cwNoDependency;
    includeinfo.isdependentoffile = -1;
    includeinfo.suppressload = FALSE;
    if (UCBFindAndLoadFile(context, sbmpath, &includeinfo) != cwNoErr || !includeinfo.filedata) {
        if (clState.pluginDebug) {
            CLPrint(
                "UCBLookUpUnit:  could not find or load precompiled unit file '%s'; failing\n",
                sbmpath
            );
        }
        return cwErrFileNotFound;
    }

    sbmspec = includeinfo.filespec;
    if (includeinfo.filedatatype != cwFileTypePrecompiledHeader) {
        CLPrint(
            "UCBLookUpUnit:  file '%s' does not appear to be precompiled\n",
            OS_SpecToString(&sbmspec, STSbuf, sizeof(STSbuf))
        );
        UCBReleaseFileText(context, includeinfo.filedata);
        return cwErrRequestFailed;
    }

    OS_GetFileTime(&unitspec, NULL, &unittime);
    OS_TimeToMac(unittime, &unitmactime);
    if (unitmactime != *reinterpret_cast<const UInt32*>(includeinfo.filedata)) {
        if (clState.pluginDebug) {
            CLPrint(
                "UCBLookUpUnit:  file '%s' does not have internal timestamp that matches source unit's timestamp (0x%8x != 0x%8x)\n",
                OS_SpecToString(&sbmspec, STSbuf, sizeof(STSbuf)),
                unitmactime,
                *reinterpret_cast<const UInt32*>(includeinfo.filedata)
            );
        }
        UCBReleaseFileText(context, includeinfo.filedata);
        return cwErrRequestFailed;
    }

    sbmfile = Files_FindFile(&gTarg->pchs, &sbmspec);
    if (!sbmfile) {
        sbmfile = File_New();
        sbmfile->srcfss = unitspec;
        OS_SpecToString(&sbmfile->srcfss, sbmfile->srcfilename, sizeof(sbmpath));
        sbmfile->outfss = sbmspec;
        OS_SpecToString(&sbmfile->outfss, sbmfile->outfilename, sizeof(sbmpath));
        sbmfile->compiler = srcfile->compiler;
        sbmfile->outfileowner = CmdLineStageMask_Cg;
        sbmfile->writeToDisk = srcfile->writeToDisk;
        sbmfile->dropinflags = srcfile->dropinflags;
        sbmfile->objectflags = srcfile->objectflags;
        sbmfile->mappingflags = srcfile->mappingflags;
        Deps_Initialize(&sbmfile->deps, &gTarg->incls);
        if (!Files_AddFile(&gTarg->pchs, sbmfile)) {
            return cwErrRequestFailed;
        }
    }

    if (clState.pluginDebug) {
        CLPrint("UCBLookUpUnit:  success for '%s'\n", name);
    }

    if (optsCmdLine.verbose) {
        CLReport(
            CLStr62,
            "unit symbol table",
            OS_SpecToStringRelative(&sbmspec, NULL, STSbuf, sizeof(STSbuf))
        );
    }

    *unitdata = includeinfo.filedata;
    *unitdatalength = includeinfo.filedatalength;
    return cwNoErr;
}

CW_CALLBACK UCBSBMfiles(CWPluginContext context, short libref) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBSBMfiles");
    }
    return cwNoErr;
}

CW_CALLBACK UCBStoreUnit(
    CWPluginContext context,
    const char* inunitname,
    CWMemHandle unitdata,
    CWDependencyTag dependencytag
) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBStoreUnit");
    }

    char unitname[260];
    OSSpec sbmspec;
    int err;
    Handle h;
    OSHandle unithand;
    OSFileHandle fhand;
    CWCompilerLinkerContext* c;
    File* srcfile;

    if (optsCompiler.canonicalIncludes) {
        strcpy(unitname, inunitname);
    } else if (OS_CanonPath(inunitname, unitname)) {
        return cwErrInvalidParameter;
    }

    if (optsCompiler.sbmState == OptsCompilerSbmState_Normal ||
        optsCompiler.sbmState == OptsCompilerSbmState_Rebuild) {
        if (context->pluginType == CWDROPINCOMPILERTYPE ||
            context->pluginType == CWDROPINLINKERTYPE) {
            c = static_cast<CWCompilerLinkerContext*>(context);
        } else {
            return cwErrInvalidCallback;
        }

        srcfile = Files_GetFile(&gTarg->files, c->whichfile);
        OS_ASSERT(791, srcfile != NULL);

        if (optsCompiler.sbmPath[0]) {
            err = OS_MakeSpecWithPath(&clState.sbmPathSpec, unitname, TRUE, &sbmspec);
            if (err) {
                if (clState.pluginDebug) {
                    CLPrint(
                        "UCBStoreUnit:  '%s' is a bad unit name (%s)\n",
                        unitname,
                        OS_GetErrText(err)
                    );
                }
                return cwErrInvalidParameter;
            }
        } else {
            err = OS_MakeFileSpec(unitname, &sbmspec);
            if (err) {
                if (clState.pluginDebug) {
                    CLPrint(
                        "UCBStoreUnit:  '%s' is a bad filename (%s)\n", unitname, OS_GetErrText(err)
                    );
                }
                return cwErrInvalidParameter;
            }
        }

        err = UnitNameToSBMName(&sbmspec, srcfile);
        if (err) {
            if (clState.pluginDebug) {
                CLPrint(
                    "UCBStoreUnit:  could not make precompiled unit form of '%s' (%s)\n",
                    unitname,
                    OS_GetErrText(err)
                );
            }
            return cwErrInvalidParameter;
        }

        if (optsCmdLine.verbose) {
            CLReport(
                CLStr61,
                "unit symbol table",
                OS_SpecToStringRelative(&sbmspec, NULL, STSbuf, sizeof(STSbuf))
            );
        }

        UCBSecretDetachHandle(context, unitdata, &h);
        OS_DestroyMacHandle(h, &unithand);
        err = OS_NewFileHandle(&sbmspec, &unithand, TRUE, &fhand);
        if (err || (err = OS_FreeFileHandle(&fhand))) {
            CLReportOSError(
                CLStr18,
                err,
                "precompiled unit",
                OS_SpecToStringRelative(&sbmspec, NULL, STSbuf, sizeof(STSbuf))
            );
            return cwErrRequestFailed;
        }
    }

    return cwNoErr;
}

CW_CALLBACK UCBReleaseUnit(CWPluginContext context, void* unitdata) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBReleaseUnit");
    }

    if (!unitdata) {
        return cwErrRequestFailed;
    }
    return UCBReleaseFileText(context, static_cast<const char*>(unitdata));
}

CW_CALLBACK UCBUnitNameToFileName(CWPluginContext context, const char* unitname, char* filename) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBUnitNameToFileName");
    }

    strcpy(filename, unitname);
    if (!OS_EqualPath(filename + strlen(filename) - 2, ".p")) {
        strcat(filename, ".p");
    }
    return cwNoErr;
}

CW_CALLBACK UCBOSAlert(CWPluginContext context, const char* message, OSErr errorcode) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBOSAlert");
    }
    if (CheckForUserBreak()) {
        return cwErrUserCanceled;
    }

    char errormessage[256];
    GetSysErrText(errorcode, errormessage);
    CLStyledMessageDispatch(
        static_cast<shellContextType*>(context->shellContext)->plugin,
        NULL,
        errorcode,
        Msg_Alert,
        "%\n(%)\n",
        message ? message : "",
        errormessage
    );
    return cwNoErr;
}

CW_CALLBACK UCBOSErrorMessage(CWPluginContext context, const char* message, OSErr errorcode) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBOSErrorMessage");
    }
    if (!errorcode) {
        return cwNoErr;
    }
    if (CheckForUserBreak()) {
        return cwErrUserCanceled;
    }

    char errormessage[256];
    GetSysErrText(errorcode, errormessage);
    CLStyledMessageDispatch(
        static_cast<shellContextType*>(context->shellContext)->plugin,
        NULL,
        errorcode,
        Msg_Alert,
        "%\n%\n",
        message ? message : "",
        errormessage
    );
    return cwNoErr;
}

CW_CALLBACK UCBGetModifiedFiles(
    CWPluginContext context, SInt32* modifiedFileCount, const SInt32** modifiedFiles
) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBGetModifiedFiles");
    }

    *modifiedFileCount = 0;
    CLInternalError("CLCompilerLinkerDropin_V10.cpp", 949, "CWGetModifiedFiles not implemented!\n");
    return cwNoErr;
}

CW_CALLBACK UCBGetSuggestedObjectFileSpec(
    CWPluginContext context, SInt32 whichfile, OSSpec* fileSpec
) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBGetSuggestedObjectFileSpec");
    }

    File* file;
    if (!(file = Files_GetFile(&gTarg->files, whichfile))) {
        return cwErrUnknownFile;
    }

    if (!GetOutputFile(file, CmdLineStageMask_Cg)) {
        return cwErrRequestFailed;
    }

    *fileSpec = file->outfss;
    return cwNoErr;
}

CW_CALLBACK UCBGetStoredObjectFileSpec(
    CWPluginContext context, SInt32 whichfile, OSSpec* fileSpec
) {
    if (optsCmdLine.verbose > 3) {
        CLPrint("Callback: %s\n", "UCBGetStoredObjectFileSpec");
    }

    File* file;
    if (!(file = Files_GetFile(&gTarg->files, whichfile))) {
        return cwErrUnknownFile;
    }
    if (file->outfileowner != CmdLineStageMask_Cg) {
        CLInternalError(
            "CLCompilerLinkerDropin_V10.cpp",
            993,
            "Lost stored object file spec for '%s'\n",
            file->srcfilename
        );
        return cwErrRequestFailed;
    } else {
        *fileSpec = file->outfss;
        return cwNoErr;
    }
}
}

CWCompilerLinkerCallbacks sCompilerLinkerCallbacks = {
    UCBCachePrecompiledHeader,
    UCBLoadObjectData,
    UCBStoreObjectData,
    UCBFreeObjectData,
    UCBDisplayLines,
    UCBBeginSubCompile,
    UCBEndSubCompile,
    UCBGetPrecompiledHeaderSpec,
    UCBGetResourceFile,
    UCBPutResourceFile,
    UCBLookUpUnit,
    UCBSBMfiles,
    UCBStoreUnit,
    UCBReleaseUnit,
    UCBUnitNameToFileName,
    UCBOSErrorMessage,
    UCBOSAlert,
    UCBGetModifiedFiles,
    UCBGetSuggestedObjectFileSpec,
    UCBGetStoredObjectFileSpec,
    NULL
};

CWCompilerLinkerContext::CWCompilerLinkerContext()
    : CWPluginPrivateContext(clState.plugintype, sizeof(CWCompilerLinkerContext)) {
    targetinfo = static_cast<IDETargetInfo*>(xmalloc("context", sizeof(IDETargetInfo)));
    memset(targetinfo, 0, sizeof(IDETargetInfo));

    if (gTarg) {
        targetinfo->targetCPU = gTarg->cpu;
        targetinfo->targetOS = gTarg->os;
    } else {
        targetinfo->targetCPU = targetinfo->targetOS = targetCPUAny;
    }

    whichfile = 0;
    memset(&sourcefile, 0, sizeof(sourcefile));
    sourcetext = NULL;
    sourcetextsize = 0;

    precompile = autoprecompile = preprocess = FALSE;
    cachingPCH = FALSE;
    debuginfo = FALSE;
    fileID = 0;
    memset(&browseoptions, 0, sizeof(browseoptions));
    reserved = NULL;
    sequenceID = 0;
    parentPB = NULL;
    targetStorage = NULL;
    texthandle = NULL;
    memset(&targetinfo_V7, 0, sizeof(targetinfo_V7));

    callbacks = &sCompilerLinkerCallbacks;
}

CWCompilerLinkerContext::~CWCompilerLinkerContext() {
    xfree(targetinfo);
}
