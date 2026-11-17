#ifndef DRIVER_PARSERGLUE_PS2_CC_H
#define DRIVER_PARSERGLUE_PS2_CC_H

#include "compiler/common.h"
#include "driver/ParserFace.h"

typedef struct ParserInputArguments {
    int argc;
    char** argv;
} ParserInputArguments;

extern int RegisterStaticParserToolInfo(void);
extern char ps2CIncludesEnv[];
extern SInt32 parserGCCCompatibilityMode;
extern ParserTool parserTool_5D6A1C;
extern ParserTool parserTool_5D6AB0;
extern ParserTool parserPS2ToolInfo;
extern ParserInputArguments* parserInputArgs_625934;

#endif
