#include "driver/Arguments.h"
#include "driver/IO.h"
#include "driver/Memory.h"
#include "driver/ParserFace.h"
#include "driver/ParserHelpers-cc.h"
#include "driver/StdTargetWarningHelpers-cc.h"
#include "driver/StringUtils.h"
#include "driver/OptimizerHelpers.h"
#include "driver/ToolHelpers-cc.h"
#include "driver/pref_structs.h"
#include "driver/ParserHelpers.h"
#include "driver/Projects.h"
#include "driver/ToolHelpers.h"
#include "driver/ParserErrors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/CompilerSetupOptions.h"

int fn_00412770(int unused1, int unused2, char* options) {
    char buffer[1024];
    char* comma;
    int length;

    while (*options) {
        comma = strchr(options, ',');
        if (comma) {
            length = comma - options;
            strncpy(buffer, options, length);
            buffer[length] = 0;
            options = comma + 1;
        } else {
            strcpy(buffer, options);
            options = emptyCompilerOption;
        }
        Arg_AddToToolArgs(&linkargs, 1, NULL);
        Arg_AddToToolArgs(&linkargs, 2, buffer);
    }
    return 1;
}

int fn_00412800(void) {
    parseopts.ignoreUnknown = 1;
    setLinkerOutputFilename = 0;
    linkerOutputFilename[0] = 0;
    definesHandle = NULL;
    compilerAdditionalDefines = NULL;
    compilerPS2Includes = 1;
    cppPS2Includes = 1;
    compilerEnvFlag = 1;
    localCompilerFlag0 = 0;
    localCompilerFlag1 = 0;
    ppDumpFlag = 0;
    namingSysPaths = 1;
    dashIMinusMovesPaths = 1;
    pCmdLineCompiler.includeSearch = 3;
    localCompilerFlag2 = 0;
    localCompilerFlag3 = 1;
    localCompilerFlag4 = 0;
    pCmdLine.stderr2stdout = 0;
    *(volatile SInt16*)&pCmdLine.msgStyle = 1;
    Opt_SavePrefs(NULL, NULL, NULL, 0);
    return 1;
}

int fn_004128B0(void) {
    const char* match1;
    const char* match2;
    char* value;

    if (ppDumpFlag) {
        if (!(pCmdLine.stages & 1)) {
            pCmdLine.state = 2;
            pCmdLine.toDisk |= 1;
            if (!Opt_SetStage(preprocessorOption, preprocessorStage, NULL, 0)) {
                return 0;
            }
        } else {
            if (!Opt_AddStringToDefines(preprocessorOption, simplePrepDump, NULL, 0)) {
                return 0;
            }
        }
    }

    if ((int)pCmdLine.state == 0) {
        pCmdLine.state = 3;
    }

    if (cppPS2Includes) {
        value = GetEnvVar(cppIncludesEnv, 0, &match1);
        if (value && !AddAccessPathList(value, ';', ':', 0, NULL, 1, -1, 0)) {
            return 0;
        }
    }

    if (compilerPS2Includes) {
        value = GetEnvVar(cIncludesEnv, 0, &match2);
        if (value && !AddAccessPathList(value, ';', ':', 0, NULL, 1, -1, 0)) {
            return 0;
        }
    }
    return 1;
}

int fn_004129E0(void) {
    if (!SetupPragmas(warningPragmas)) {
        return 0;
    }
    if (!SetupPragmas(compilerPragmas)) {
        return 0;
    }
    if (!SetupPragmas(additionalPragmas)) {
        return 0;
    }
    if (!SetupPragmas(irPragmas)) {
        return 0;
    }

    if (!definesHandle && !compilerAdditionalDefines) {
        pascalDefinesFilename[0] = 0;
    } else {
        if (definesHandle && compilerAdditionalDefines) {
            SetHandleSize(definesHandle, GetHandleSize(definesHandle) - 1);
            if (MemError()) {
                fprintf(stderr, outOfMemoryMessage);
                exit(-23);
            }
        }
        if (compilerAdditionalDefines && HandAndHand(compilerAdditionalDefines, definesHandle)) {
            fprintf(stderr, outOfMemoryMessage);
            exit(-23);
        }
        if (pCmdLine.verbose) {
            ShowTextHandle(definesVirtualFilename, definesHandle);
        }
        AddVirtualFile(definesVirtualFilename, &definesHandle);
        c2pstrcpy(pascalDefinesFilename, definesVirtualFilename);
    }
    FinishCompilerTool();
    if (!ValidateToolState(1)) {
        return 0;
    }
    if (pCmdLine.state == 3 && setLinkerOutputFilename) {
        Arg_AddToToolArgs(&linkargs, 1, NULL);
        Arg_AddToToolArgs(&linkargs, 2, linkerFlag);
        Arg_AddToToolArgs(&linkargs, 1, NULL);
        Arg_AddToToolArgs(&linkargs, 2, linkerOutputFilename);
    }
    return 1;
}
