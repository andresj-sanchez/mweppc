#include "driver/MacSpecs.h"
#include "driver/CWPlugins.h"
#include "driver/CLDropinCallbacks_V10.h"
#include "driver/CLPluginRequests.h"
#include "driver/CWParserPluginsContext.h"
#include "driver/CLFiles.h"
#include "driver/CLErrors.h"
#include "driver/CLLicenses.h"
#include "driver/CLMain.h"
#include "driver/CLOverlays.h"
#include "driver/CLPlugins.h"
#include "driver/CLTarg.h"
#include "driver/cmdline.h"

#include "driver/DropInCompilerLinker.h"

#include "driver/Generic.h"
#include "driver/MacFileTypes.h"
#include "driver/MsDos.h"
#include "driver/oslib.h"

#include "driver/CLDependencies.h"
#include "driver/CLAccessPaths.h"
#include "driver/CLIO.h"
#include "driver/pref_structs.h"
#include "driver/Files.h"
#include <string.h>

extern "C" {
static CW_CALLBACK CallPlugin(Plugin* plugin) {
    return Plugin_Call(plugin, plugin->context);
}

//! @reconstructed
static void IDETargetInfoToTargetInfo(Plugin* plugin, IDETargetInfo* from, CLTargetInfo* to) {
    to->outputType = from->outputType;
    to->linkType = from->linkType;
    to->canRun = from->canRun;
    to->canDebug = from->canDebug;
    to->targetCPU = from->targetCPU;
    to->targetOS = from->targetOS;
    to->outfile = from->outfile;
    to->symfile = from->symfile;
    to->runfile = from->runfile;
    to->linkAgainstFile = from->linkAgainstFile;
}

//! @reconstructed
static void TargetInfoToIDETargetInfo(Plugin* plugin, CLTargetInfo* from, IDETargetInfo* to) {
    to->outputType = from->outputType;
    to->linkType = from->linkType;
    to->canRun = from->canRun;
    to->canDebug = from->canDebug;
    to->targetCPU = from->targetCPU;
    to->targetOS = from->targetOS;
    to->outfile = from->outfile;
    to->symfile = from->symfile;
    to->runfile = from->runfile;
    to->linkAgainstFile = from->linkAgainstFile;
}

static void SetupPluginContext(Plugin* plugin, CWResult request) {
    OSSpec spec;
    shellContextType* sc;
    OSPathSpec cwd;

    sc = static_cast<shellContextType*>(plugin->context->shellContext);
    sc->plugin = plugin;
    sc->userAccessPathsChanged = sc->systemAccessPathsChanged = TRUE;
    plugin->context->request = request;
    plugin->context->apiVersion = 13;
    plugin->context->pluginStorage = NULL;

    OS_MakeFileSpec("Project.mcp", &spec);
    plugin->context->projectFile = spec;
    if (gTarg) {
        OS_MakeSpecWithPath(&gTarg->outputDirectory, NULL, FALSE, &spec);
        plugin->context->outputFileDirectory = spec;
    } else {
        OS_GetCWD(&cwd);
        OS_MakeSpecWithPath(&cwd, NULL, FALSE, &spec);
        plugin->context->outputFileDirectory = spec;
    }

    plugin->context->shellSignature = 'CWIE';
    plugin->context->pluginType = Plugin_GetDropInFlags(plugin)->dropintype;

    if (gTarg) {
        plugin->context->numFiles = Files_Count(&gTarg->files);
    } else {
        plugin->context->numFiles = 0;
    }

    if (gTarg) {
        plugin->context->numOverlayGroups = gTarg->linkage.overlays.grpcnt;
    } else {
        plugin->context->numOverlayGroups = 0;
    }

    plugin->context->callbackOSError = plugin->context->pluginOSError = noErr;
    plugin->context->accessPathList = NULL;
}
}

Boolean SendParserRequest(
    Plugin* plugin,
    Target* target,
    CWCommandLineArgs* args,
    OSType cpu,
    OSType os,
    int numPlugins,
    const CLPluginInfo* pluginInfo,
    int numPanels,
    const char** panelNames,
    CWCommandLineArgs* plugin_args,
    CWCommandLineArgs* panel_args,
    const char* build_date,
    const char* build_time,
    const ToolVersionInfo* build_tool
) {
    int result;
    CWParserContext* pc;

    SetupPluginContext(plugin, 1);

    pc = static_cast<CWParserContext*>(plugin->context);
    pc->build_date = build_date;
    pc->build_time = build_time;
    pc->build_tool = build_tool;
    pc->args = args;
    pc->cpu = cpu;
    pc->os = os;
    pc->numPlugins = numPlugins;
    pc->plugins = pluginInfo;
    pc->numPanels = numPanels;
    pc->panelNames = panelNames;
    pc->plugin_args = plugin_args;
    pc->panel_args = panel_args;

    if (CallPlugin(plugin) != cwNoErr) {
        return FALSE;
    }
    if (!SendTargetInfoRequest(target, target->linker, target->linkerDropinFlags)) {
        return FALSE;
    }

    target->linkmodel = (target->targetinfo->linkType == exelinkageSegmented) ? LinkModel_Segmented
        : (target->targetinfo->linkType == exelinkageOverlay1)                ? LinkModel_Overlay1
                                                                              : LinkModel_Flat;

    if (!Plugin_VerifyPanels(plugin)) {
        return FALSE;
    }

    SetupPluginContext(plugin, 2);
    pc->args = args;
    pc->cpu = cpu;
    pc->os = os;
    pc->numPlugins = numPlugins;
    pc->plugins = pluginInfo;
    pc->numPanels = numPanels;
    pc->panelNames = panelNames;
    pc->plugin_args = plugin_args;
    pc->panel_args = panel_args;

    result = CallPlugin(plugin);
    return (result == cwNoErr);
}

Boolean SendCompilerRequest(Plugin* plugin, File* file, SInt16 stage) {
    CWResult result;
    CWCompilerLinkerContext* cc;
    Boolean precompiling;
    SInt16 type;
    OSType mactype;
    const char* pn;
    int detailed;

    if (stage & CmdLineStageMask_Cg) {
        if (optsCompiler.forcePrecompile == 1) {
            precompiling = TRUE;
        } else if (optsCompiler.forcePrecompile == 2) {
            precompiling = FALSE;
        } else {
            precompiling = (file->mappingflags & kPrecompile) != 0;
        }
    } else {
        precompiling = FALSE;
    }

    if (optsCmdLine.verbose) {
        pn = Plugin_GetDropInName(plugin);
        detailed = optsCmdLine.verbose > 1;
        if (stage & CmdLineStageMask_Pp) {
            CLReport(CLStr34 + detailed, file->srcfilename, pn);
        } else if (stage & CmdLineStageMask_Cg) {
            if (precompiling) {
                CLReport(CLStr32 + detailed, file->srcfilename, pn);
            } else {
                CLReport(CLStr30 + detailed, file->srcfilename, pn);
            }
        } else if (stage & CmdLineStageMask_Dp) {
            CLReport(CLStr36 + detailed, file->srcfilename, pn);
        } else if (stage & CmdLineStageMask_Ds) {
            CLReport(CLStr46 + detailed, file->srcfilename, pn);
        } else if (!stage) {
            CLReport(CLStr48 + detailed, file->srcfilename, pn);
        }
    }

    if (!Plugin_VerifyPanels(plugin)) {
        return FALSE;
    }

    SetupPluginContext(plugin, reqCompile);
    cc = static_cast<CWCompilerLinkerContext*>(plugin->context);
    TargetInfoToIDETargetInfo(plugin, gTarg->targetinfo, cc->targetinfo);

    cc->sourcefile = file->srcfss;
    if (!OS_GetMacFileType(&file->srcfss, &mactype) && mactype == 'TEXT') {
        if (UCBGetFileText(cc, &cc->sourcefile, &cc->sourcetext, &cc->sourcetextsize, &type) !=
            cwNoErr) {
            return FALSE;
        }
    } else {
        cc->sourcetext = NULL;
        cc->sourcetextsize = 0;
    }

    cc->whichfile = file->filenum;
    cc->debuginfo = optsCmdLine.debugInfo;
    cc->recordbrowseinfo = file->recordbrowseinfo;
    cc->fileID = file->browseFileID;
    memcpy(&cc->browseoptions, &file->browseoptions, sizeof(cc->browseoptions));

    if (stage & (CmdLineStageMask_Pp | CmdLineStageMask_Dp)) {
        if (stage & CmdLineStageMask_Pp) {
            cc->preprocess = TRUE;
        } else {
            cc->preprocess = 2;
        }
    } else {
        cc->preprocess = FALSE;
    }

    if ((stage & CmdLineStageMask_Cg) && precompiling) {
        cc->precompile = TRUE;
        cc->autoprecompile = TRUE;
    } else {
        cc->precompile = FALSE;
        cc->autoprecompile = FALSE;
    }

    if (!(stage & (CmdLineStageMask_Pp | CmdLineStageMask_Cg | CmdLineStageMask_Dp))) {
        cc->preprocess = 2;
        cc->precompile = 2;
    }

    result = CallPlugin(plugin);

    if (cc->sourcetext) {
        UCBReleaseFileText(cc, cc->sourcetext);
        cc->sourcetext = NULL;
        cc->sourcetextsize = 0;
    }

    return (result == cwNoErr);
}

Boolean SendTargetInfoRequest(Target* targ, Plugin* linker, SInt32 dropinflags) {
    CWResult result;
    CWCompilerLinkerContext* cc;
    OSSpec spec;

    if (linker && !(dropinflags & isExecutableTool)) {
        if (optsCmdLine.verbose > 1) {
            CLReport(CLStr50, Plugin_GetDropInName(linker));
        }
        OS_ASSERT(298, Plugin_GetPluginType(linker) == CWDROPINLINKERTYPE);

        if (!Plugin_VerifyPanels(linker)) {
            return FALSE;
        }

        SetupPluginContext(linker, reqTargetInfo);
        cc = static_cast<CWCompilerLinkerContext*>(linker->context);
        TargetInfoToIDETargetInfo(linker, targ->targetinfo, cc->targetinfo);
        result = CallPlugin(linker);
        IDETargetInfoToTargetInfo(linker, cc->targetinfo, targ->targetinfo);
    } else {
        OS_MakeFileSpec("(unknown file)", &spec);
        targ->targetinfo->outputType = linkOutputFile;
        targ->targetinfo->outfile = spec;
        targ->targetinfo->symfile = spec;
        targ->targetinfo->runfile = spec;
        targ->targetinfo->linkType = exelinkageFlat;
        targ->targetinfo->canRun = FALSE;
        targ->targetinfo->canDebug = FALSE;
        targ->targetinfo->targetCPU = targ->cpu;
        targ->targetinfo->targetOS = targ->os;
        result = cwNoErr;
    }

    return (result == cwNoErr);
}

Boolean SendLinkerRequest(Plugin* plugin, SInt32 dropinflags, CLTargetInfo* targetInfo) {
    CWResult result;
    CWCompilerLinkerContext* cc;
    const char* nm;
    int detailed;

    if (optsCmdLine.verbose) {
        nm = Plugin_GetDropInName(plugin);
        detailed = optsCmdLine.verbose > 1;
        if (dropinflags & isPostLinker) {
            CLReport(CLStr44 + detailed, nm);
        } else if (dropinflags & isPreLinker) {
            CLReport(CLStr42 + detailed, nm);
        } else {
            CLReport(CLStr40 + detailed, nm);
        }
    }

    if (!Plugin_VerifyPanels(plugin)) {
        return FALSE;
    }

    SetupPluginContext(plugin, reqLink);
    cc = static_cast<CWCompilerLinkerContext*>(plugin->context);
    TargetInfoToIDETargetInfo(plugin, targetInfo, cc->targetinfo);
    result = CallPlugin(plugin);
    IDETargetInfoToTargetInfo(plugin, cc->targetinfo, targetInfo);

    return (result == cwNoErr);
}

Boolean SendDisassemblerRequest(Plugin* linker, File* file) {
    CWResult result;
    Plugin* plugin;

    if (linker == NULL) {
        plugin = file->compiler;
    } else {
        plugin = linker;
    }

    if (optsCmdLine.verbose) {
        CLReport(
            (optsCmdLine.verbose > 1) + CLStr46, file->srcfilename, Plugin_GetDropInName(linker)
        );
    }

    if (!Plugin_VerifyPanels(plugin)) {
        return FALSE;
    }

    if (!linker) {
        SetupPluginContext(plugin, reqCompDisassemble);
        static_cast<CWCompilerLinkerContext*>(plugin->context)->whichfile = file->filenum;
        result = CallPlugin(plugin);
    } else {
        SetupPluginContext(plugin, reqDisassemble);
        static_cast<CWCompilerLinkerContext*>(plugin->context)->whichfile = file->filenum;
        result = CallPlugin(plugin);
    }

    return (result == cwNoErr);
}
