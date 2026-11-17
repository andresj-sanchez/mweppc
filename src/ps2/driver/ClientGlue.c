#include "driver/ClientGlue.h"
#include "driver/CLErrors.h"
#include "driver/CLMain.h"
#include "driver/CLPlugins.h"
#include "driver/CWPlugins.h"
#include "driver/MsDos.h"
#include "driver/ResourceStrings.h"
#include "compiler/win32.h"
#include <string.h>
#include "driver/Resources.h"

int __stdcall CmdLine_Terminate(int exitcode) {
    return Main_Terminate(exitcode);
}

int __stdcall CmdLine_Driver(void) {
    return Main_Driver();
}

int __stdcall CmdLine_Initialize(
    int argc, char** argv, const char* builddate, const char* buildtime
) {
    strncpy(cmdline_build_date, builddate, sizeof(cmdline_build_date));
    strncpy(cmdline_build_time, buildtime, sizeof(cmdline_build_time));
    return Main_Initialize(argc, argv);
}

void __stdcall SetPluginType(OSType lang, OSType type) {
    clState.language = lang;
    clState.plugintype = type;
}

void __stdcall SetParserType(OSType plang) {
    clState.parserstyle = plang;
}

void __stdcall SetBuildTarget(OSType cpu, OSType os) {
    clState.cpu = cpu;
    clState.os = os;
}

int __stdcall RegisterStaticParserPlugin(
    const BasePluginCallbacks* callbacks, const ParserPluginCallbacks* pr_callbacks
) {
    return Plugins_Add(Plugin_New(callbacks, NULL, pr_callbacks));
}

int __stdcall RegisterStaticCompilerLinkerPlugin(
    const BasePluginCallbacks* callbacks, const CompilerLinkerPluginCallbacks* cl_callbacks
) {
    return Plugins_Add(Plugin_New(callbacks, cl_callbacks, NULL));
}

int __stdcall RegisterStaticPlugin(const BasePluginCallbacks* callbacks) {
    return Plugins_Add(Plugin_New(callbacks, NULL, NULL));
}

int __stdcall RegisterResource(const char* name, SInt16 rsrcid, const char** list) {
    char buf[256];
    char path[260];
    HINSTANCE inst;
    Ptr* h;

    if (list == NULL) {
        if (GetModuleFileNameA(GetModuleHandleA(NULL), path, sizeof(path) - 1) == 0) {
            OS_SpecToString(&clState.programSpec, path, sizeof(path));
        }

        if ((inst = LoadLibraryA(path)) != NULL) {
            int count = LoadStringA(inst, 101, buf, sizeof(buf));
            FreeLibrary(inst);
            if (count != 0) {
                return TRUE;
            }
        }

        h = GetResource('STR#', rsrcid);
        if (h == NULL) {
            CLFatalError("Resource ('STR#',%d) '%s' not found in executable\n", rsrcid, name);
            return FALSE;
        }
        ReleaseResource(h);
        return TRUE;
    } else {
        return Res_AddResource(name, rsrcid, list);
    }
}
