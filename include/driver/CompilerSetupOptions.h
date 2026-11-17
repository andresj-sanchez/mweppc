#ifndef DRIVER_COMPILERSETUPOPTIONS_H
#define DRIVER_COMPILERSETUPOPTIONS_H

#include "compiler/common.h"
#include "driver/parser.h"

extern int fn_00412800(void);
extern int fn_004128B0(void);
extern int fn_004129E0(void);
extern unsigned char pascalDefinesFilename[];
extern Boolean compilerPS2Includes;
extern Boolean compilerEnvFlag;
extern Pragma compilerPragmas[];
extern Boolean cppPS2Includes;
extern Boolean localCompilerFlag0;
extern Boolean localCompilerFlag1;
extern char emptyCompilerOption[];
extern char simplePrepDump[];
extern char preprocessorOption[];
extern char preprocessorStage[];
extern char cppIncludesEnv[];
extern char cIncludesEnv[];
extern const char* definesVirtualFilename;
extern char outOfMemoryMessage[];
extern char linkerFlag[];
extern Handle compilerAdditionalDefines;
extern Boolean ppDumpFlag;
extern Boolean localCompilerFlag2;
extern Boolean localCompilerFlag3;
extern Boolean localCompilerFlag4;
extern Pragma additionalPragmas[];

#endif
