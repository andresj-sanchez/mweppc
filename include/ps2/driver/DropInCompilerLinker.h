#ifndef DRIVER_DROPINCOMPILERLINKER_H
#define DRIVER_DROPINCOMPILERLINKER_H

#include "compiler/common.h"
#include "driver/CWPlugins.h"
#include "driver/MacSpecs.h"
#include "driver/oslib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DROPINCOMPILERLINKERAPIVERSION_8 8
#define DROPINCOMPILERLINKERAPIVERSION_9 9
#define DROPINCOMPILERLINKERAPIVERSION_12 12
#define DROPINCOMPILERLINKERAPIVERSION_13 13

enum {
    reqCompile = 0,
    reqMakeParse,
    reqCompDisassemble,
    reqCheckSyntax,
    reqPreprocessForDebugger
};

enum {
    reqLink = 0,
    reqDisassemble,
    reqTargetInfo,
    reqPreRun
};

enum {
    exelinkageFlat,
    exelinkageSegmented,
    exelinkageOverlay1
};

enum {
    linkOutputNone,
    linkOutputFile,
    linkOutputDirectory
};

#pragma pack(push, 2)
typedef struct CWBrowseOptions {
    UInt8 recordClasses;
    UInt8 recordEnums;
    UInt8 recordMacros;
    UInt8 recordTypedefs;
    UInt8 recordConstants;
    UInt8 recordTemplates;
    UInt8 recordUndefinedFuncs;
    SInt32 reserved1;
    SInt32 reserved2;
} CWBrowseOptions;
#pragma pack(pop)
#pragma pack(push, 2)
typedef UInt32 CWDependencyTag;

typedef struct CWDependencyInfo {
    SInt32 fileIndex; // 0x000
    CWFileSpec fileSpec; // 0x004
    short fileSpecAccessType; // 0x108
    short dependencyType; // 0x10A
} CWDependencyInfo;
typedef struct CWObjectData {
    CWMemHandle objectdata;
    CWMemHandle browsedata;
    SInt32 reserved1;
    SInt32 codesize;
    SInt32 udatasize;
    SInt32 idatasize;
    SInt32 compiledlines;
    Boolean interfaceChanged;
    SInt32 reserved2;
    void* compilecontext;
    struct CWDependencyInfo* dependencies;
    short dependencyCount;
    CWFileSpec* objectfile;
} CWObjectData;
typedef struct CWTargetInfo {
    short outputType; // 0x000
    CWWinFileSpec outfile; // 0x002
    CWWinFileSpec symfile; // 0x106
    CWWinFileSpec runfile; // 0x20A
    SInt16 linkType; // 0x30E
    Boolean canRun; // 0x310
    Boolean canDebug; // 0x311
    CWDataType targetCPU; // 0x312
    CWDataType targetOS; // 0x316
    Boolean runHelperIsRegKey; // 0x31A
    Boolean debugHelperIsRegKey; // 0x31B
    char args[512]; // 0x31C
    char runHelperName[512]; // 0x51C
    Boolean runHelperRequiresURL; // 0x71C
    char reserved2; // 0x71D
    char debugHelperName[512]; // 0x71E
    CWWinFileSpec linkAgainstFile; // 0x91E
} CWTargetInfo;
typedef struct CWMacTargetInfo {
    short outputType; // 0x000
    FSSpec outfile; // 0x002
    FSSpec symfile; // 0x048
    FSSpec runfile; // 0x08E
    SInt16 linkType; // 0x0D4
    Boolean canRun; // 0x0D6
    Boolean canDebug; // 0x0D7
    CWDataType targetCPU; // 0x0D8
    CWDataType targetOS; // 0x0DC
    Boolean runHelperIsRegKey; // 0x0E0
    Boolean debugHelperIsRegKey; // 0x0E1
    char args[512]; // 0x0E2
    char runHelperName[512]; // 0x2E2
    Boolean runHelperRequiresURL; // 0x4E2
    char reserved2; // 0x4E3
    char debugHelperName[512]; // 0x4E4
    FSSpec linkAgainstFile; // 0x6E4
} CWMacTargetInfo;
typedef struct CWTargetInfoV7 {
    CWFileSpec outfile;
    CWFileSpec symfile;
    short linkType;
    Boolean canRun;
    Boolean canDebug;
    Boolean useRunHelperApp;
    char reserved1;
    CWDataType debuggerCreator;
    CWDataType runHelperCreator;
    SInt32 reserved2[2];
} CWTargetInfoV7;
typedef struct CWEnvVarInfo {
    char* name;
    char* value;
} CWEnvVarInfo;
#pragma pack(pop)

typedef struct CWCompilerLinkerCallbacks {
    CW_CALLBACK_PTR (*cbCachePrecompiledHeader)(CWPluginContext, const OSSpec*, CWMemHandle);
    CW_CALLBACK_PTR (*cbLoadObjectData)(CWPluginContext, SInt32, CWMemHandle*);
    CW_CALLBACK_PTR (*cbStoreObjectData)(CWPluginContext, SInt32, struct IDEObjectData*);
    CW_CALLBACK_PTR (*cbFreeObjectData)(CWPluginContext, SInt32, CWMemHandle);
    CW_CALLBACK_PTR (*cbDisplayLines)(CWPluginContext, SInt32);
    CW_CALLBACK_PTR (*cbBeginSubCompile)(CWPluginContext, SInt32, CWPluginContext*);
    CW_CALLBACK_PTR (*cbEndSubCompile)(CWPluginContext);
    CW_CALLBACK_PTR (*cbGetPrecompiledHeaderSpec)(CWPluginContext, OSSpec*, const char*);
    CW_CALLBACK_PTR (*cbGetResourceFile)(CWPluginContext, OSSpec*);
    CW_CALLBACK_PTR (*cbPutResourceFile)(CWPluginContext, const char*, const char*, OSSpec*);
    CW_CALLBACK_PTR (*cbLookUpUnit)(CWPluginContext, const char*, Boolean, const void**, SInt32*);
    CW_CALLBACK_PTR (*cbSBMfiles)(CWPluginContext, short);
    CW_CALLBACK_PTR (*cbStoreUnit)(CWPluginContext, const char*, CWMemHandle, CWDependencyTag);
    CW_CALLBACK_PTR (*cbReleaseUnit)(CWPluginContext, void*);
    CW_CALLBACK_PTR (*cbUnitNameToFileName)(CWPluginContext, const char*, char*);
    CW_CALLBACK_PTR (*cbOSErrorMessage)(CWPluginContext, const char*, OSErr);
    CW_CALLBACK_PTR (*cbOSAlert)(CWPluginContext, const char*, OSErr);
    CW_CALLBACK_PTR (*cbGetModifiedFiles)(CWPluginContext, SInt32*, const SInt32**);
    CW_CALLBACK_PTR (*cbGetSuggestedObjectFileSpec)(CWPluginContext, SInt32, OSSpec*);
    CW_CALLBACK_PTR (*cbGetStoredObjectFileSpec)(CWPluginContext, SInt32, OSSpec*);
    CW_CALLBACK_PTR (*cbGetRuntimeSettings)(CWPluginContext);
} CWCompilerLinkerCallbacks;

extern CW_CALLBACK CWGetTargetInfo(CWPluginContext context, CWTargetInfo* targetInfo);
extern CW_CALLBACK CWStoreObjectData(
    CWPluginContext context, SInt32 whichfile, CWObjectData* object
);
extern CW_CALLBACK CWDisplayLines(CWPluginContext context, SInt32 nlines);
extern CW_CALLBACK CWCachePrecompiledHeader(
    CWPluginContext context, const CWFileSpec* filespec, CWMemHandle pchhandle
);
extern CW_CALLBACK CWIsPrecompiling(CWPluginContext context, Boolean* isPrecompiling);
extern CW_CALLBACK CWIsAutoPrecompiling(CWPluginContext context, Boolean* isAutoPrecompiling);
extern CW_CALLBACK CWIsPreprocessing(CWPluginContext context, Boolean* isPreprocessing);
extern CW_CALLBACK CWIsGeneratingDebugInfo(CWPluginContext context, Boolean* isGenerating);
extern CW_CALLBACK CWIsCachingPrecompiledHeaders(CWPluginContext context, Boolean* isCaching);
extern CW_CALLBACK CWGetBrowseOptions(CWPluginContext context, CWBrowseOptions* browseOptions);
extern CW_CALLBACK CWGetMainFileNumber(CWPluginContext context, SInt32* fileNumber);
extern CW_CALLBACK CWGetMainFileID(CWPluginContext context, short* fileID);
extern CW_CALLBACK CWGetMainFileSpec(CWPluginContext context, CWFileSpec* fileSpec);
extern CW_CALLBACK CWGetMainFileText(
    CWPluginContext context, const char** text, SInt32* textLength
);
extern CW_CALLBACK CWGetPrecompiledHeaderSpec(
    CWPluginContext context, CWFileSpec* pchspec, const char* target
);

#ifdef __cplusplus
}
#endif

#endif
