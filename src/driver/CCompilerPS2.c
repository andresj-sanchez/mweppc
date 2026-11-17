#include "compiler/common.h"
#include "driver/CWParserPluginsContext.h"
#include "driver/pref_structs.h"
#include <string.h>
#include "compiler/CodeGen.h"
#include "compiler/CompilerTools.h"
#include "driver/COSToolsCLT.h"
#include "driver/CWPluginsPrivate.h"
#include "driver/CCompilerPS2.h"
#include "driver/CCompiler.h"
#include "driver/DropInCompilerLinker.h"

void CCompilerSaveBrowserData(void* params, void* list) {
    CWMemHandle* destination = (CWMemHandle*)((unsigned char*)params + 8);
    ShrinkGList(list);
    *destination = COS_ConvertToCWMemHandle(list);
}

void fn_00423E00(PFrontEndCPanel* options) {
    __builtin_memset(options, 0, sizeof(PFrontEndCPanel));
    options->frontend.version = 16;
    options->frontend.cplusplus = 0;
    options->frontend.checkprotos = 0;
    options->frontend.arm = 0;
    options->frontend.trigraphs = 0;
    options->frontend.onlystdkeywords = 0;
    options->frontend.enumsalwaysint = 0;
    options->frontend.mpwpointerstyle = 0;
    options->unk40 = 0;
    options->frontend.oldprefixname[0] = 0;
    options->frontend.ansistrict = 0;
    options->frontend.mpwcnewline = 0;
    options->frontend.wchar_type = 1;
    options->frontend.enableexceptions = 1;
    options->frontend.dontreusestrings = 0;
    options->frontend.poolstrings = 0;
    options->frontend.dontinline = 0;
    options->frontend.useRTTI = 1;
    options->frontend.multibyteaware = 0;
    options->frontend.unsignedchars = 0;
    options->frontend.autoinline = 0;
    options->frontend.booltruefalse = 1;
    options->frontend.inlinelevel = 0;
    options->frontend.ecplusplus = 0;
    options->frontend.defer_codegen = 0;
    options->frontend.templateparser = 0;
    options->c99 = 0;
    options->bottomupinline = 1;
}

Boolean fn_00423BA0(CWPluginContext context) {
    CWTargetInfo tinfo;

    memset(&cparams, 0, sizeof(CompilerLinkerParamBlk));
    cparams.context = context;

    if (CWGetPluginRequest(context, &cparams.request) != cwNoErr) {
        return FALSE;
    }
    if (CWGetAPIVersion(context, &cparams.version) != cwNoErr) {
        return FALSE;
    }
    if (CWGetProjectFile(context, &cparams.targetfile) != cwNoErr) {
        return FALSE;
    }
    if (CWGetProjectFileCount(context, &cparams.numfiles) != cwNoErr) {
        return FALSE;
    }
    if (CWGetMainFileNumber(context, &cparams.whichfile) != cwNoErr) {
        return FALSE;
    }
    if (CWGetMainFileSpec(context, &cparams.sourcefile) != cwNoErr) {
        return FALSE;
    }
    if (CWGetMainFileText(context, &cparams.sourcetext, &cparams.sourcetextsize) != cwNoErr) {
        return FALSE;
    }
    if (CWIsPrecompiling(context, &cparams.precompile) != cwNoErr) {
        return FALSE;
    }
    if (CWIsAutoPrecompiling(context, &cparams.autoprecompile) != cwNoErr) {
        return FALSE;
    }
    if (CWIsPreprocessing(context, &cparams.preprocess) != cwNoErr) {
        return FALSE;
    }
    if (CWIsGeneratingDebugInfo(context, &cparams.SYMinfo) != cwNoErr) {
        return FALSE;
    }
    if (CWIsCachingPrecompiledHeaders(context, &cparams.caching_includes) != cwNoErr) {
        return FALSE;
    }
    if (CWGetBrowseOptions(context, &cparams.browseoptions) != cwNoErr) {
        return FALSE;
    }
    cparams.recordbrowseinfo = !cparams.preprocess;
    if (CWGetMainFileID(context, &cparams.browserfileID) != cwNoErr) {
        return FALSE;
    }
    if (CWGetTargetInfo(context, &tinfo) != cwNoErr) {
        return FALSE;
    }

    cparams.targetOS = tinfo.targetOS;
    cparams.targetCPU = tinfo.targetCPU;
    cparams.idetargetname = target_name;
    if (CWGetTargetName(context, cparams.idetargetname, sizeof(target_name)) != cwNoErr) {
        return FALSE;
    }
    return TRUE;
}
