#include "driver/ParserFace.h"
#include "driver/Targets.h"
#include "driver/ToolHelpers.h"
#include "driver/ParserHelpers.h"
#include "driver/Projects.h"
#include "driver/pref_structs.h"
#include "driver/StdTargetWarningHelpers-cc.h"
#include "driver/Arguments.h"
#include "driver/IO.h"
#include "driver/ParserHelpers-cc.h"
#include "driver/StringUtils.h"
#include "driver/ToolHelpers-cc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "driver/CompilerSetupOptions.h"
#include "driver/ParserGlue-ps2-cc.h"
#include "msl/CaseFoldCompare.h"

int RegisterStaticParserToolInfo(void) {
    return SetParserToolInfo(&parserPS2ToolInfo);
}

int fn_00406E30(void) {
    return 0;
}

int fn_00406E20(void) {
    return 0;
}

int PostParse(void) {
    if (parserGCCCompatibilityMode) {
        return fn_004129E0();
    }
    if (!SetupPragmas(compilerPragmas) || !SetupPragmas(warningPragmas)) {
        return FALSE;
    }
    if (definesHandle) {
        if (pCmdLine.verbose) {
            ShowTextHandle(definesVirtualFilename, definesHandle);
        }
        AddVirtualFile(definesVirtualFilename, &definesHandle);
        c2pstrcpy(pascalDefinesFilename, definesVirtualFilename);
    } else {
        pascalDefinesFilename[0] = 0;
    }
    FinishCompilerTool();
    if (!ValidateToolState(TRUE)) {
        return FALSE;
    }
    if (linkerOutputFilename[0]) {
        Arg_AddToToolArgs(&linkargs, 1, NULL);
        Arg_AddToToolArgs(&linkargs, 2, linkerFlag);
        Arg_AddToToolArgs(&linkargs, 1, NULL);
        Arg_AddToToolArgs(&linkargs, 2, linkerOutputFilename);
        Arg_AddToToolArgs(&linkargs, 1, NULL);
    }
    return TRUE;
}

int MidParse(void) {
    const char* matched;
    char* paths;

    if (parserGCCCompatibilityMode) {
        return fn_004128B0();
    }
    if (!pCmdLine.state) {
        pCmdLine.state = 3;
    }
    if (parseopts.possibleFiles > 0 && compilerPS2Includes) {
        paths = GetEnvVar(ps2CIncludesEnv, TRUE, &matched);
        if (paths && !AddAccessPathList(paths, ';', ':', 1, (char*)matched, TRUE, -1, FALSE)) {
            return FALSE;
        }
    }
    return TRUE;
}

int PreParse(void) {
    if (parserGCCCompatibilityMode) {
        return fn_00412800();
    }
    compilerPS2Includes = TRUE;
    setLinkerOutputFilename = FALSE;
    linkerOutputFilename[0] = 0;
    definesHandle = NULL;
    return TRUE;
}
