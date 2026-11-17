#ifndef DRIVER_CWPLUGINSPRIVATE_H
#define DRIVER_CWPLUGINSPRIVATE_H

#include "compiler/common.h"
#include "driver/cmdline.h"
#include "driver/CWPlugins.h"
#include "driver/DropInCompilerLinker.h"
#include "driver/oslib.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 2)

typedef struct IDEAccessPath {
    OSSpec pathSpec; // 0x000
    Boolean recursive; // 0x204
    SInt32 subdirectoryCount; // 0x206
    OSSpec* subdirectories; // 0x20A
} IDEAccessPath;

typedef struct IDEAccessPathList {
    SInt32 userPathCount; // 0x00
    IDEAccessPath* userPaths; // 0x04
    SInt32 systemPathCount; // 0x08
    IDEAccessPath* systemPaths; // 0x0C
    Boolean alwaysSearchUserPaths; // 0x10
    Boolean convertPaths; // 0x11
    Boolean sourceRelativeIncludes; // 0x12
} IDEAccessPathList;

typedef struct IDEProjectFileInfo {
    OSSpec filespec; // 0x000
    OSTime moddate; // 0x204
    short segment; // 0x20C
    Boolean hasobjectcode; // 0x20E
    Boolean hasresources; // 0x20F
    Boolean isresourcefile; // 0x210
    Boolean weakimport; // 0x211
    Boolean initbefore; // 0x212
    Boolean gendebug; // 0x213
    OSTime objmoddate; // 0x214
    OSNameSpec dropinname; // 0x21C
    short fileID; // 0x31C
    Boolean recordbrowseinfo; // 0x31E
    Boolean reserved; // 0x31F
    OSType filetype; // 0x320
    OSType filecreator; // 0x324
    Boolean hasunitdata; // 0x328
    Boolean mergeintooutput; // 0x329
    UInt32 unitdatadependencytag; // 0x32A
} IDEProjectFileInfo;

typedef struct IDEFileInfo {
    Boolean fullsearch; // 0x000
    char dependencyType; // 0x001
    SInt32 isdependentoffile; // 0x002
    Boolean suppressload; // 0x006
    Boolean padding; // 0x007
    const char* filedata; // 0x008
    SInt32 filedatalength; // 0x00C
    short filedatatype; // 0x010
    short fileID; // 0x012
    OSSpec filespec; // 0x014
    Boolean alreadyincluded; // 0x218
    Boolean recordbrowseinfo; // 0x219
} IDEFileInfo;

typedef struct IDEMessageRef {
    OSSpec sourcefile; // 0x000
    SInt32 linenumber; // 0x204
    short tokenoffset; // 0x208
    short tokenlength; // 0x20A
    SInt32 selectionoffset; // 0x20C
    SInt32 selectionlength; // 0x210
} IDEMessageRef;

typedef struct IDEDependencyInfo {
    SInt32 fileIndex; // 0x000
    OSSpec fileSpec; // 0x004
    short fileSpecAccessType; // 0x208
    short dependencyType; // 0x20A
} IDEDependencyInfo;

typedef struct IDEObjectData {
    CWMemHandle objectdata; // 0x00
    CWMemHandle browsedata; // 0x04
    SInt32 reserved1; // 0x08
    SInt32 codesize; // 0x0C
    SInt32 udatasize; // 0x10
    SInt32 idatasize; // 0x14
    SInt32 compiledlines; // 0x18
    Boolean interfaceChanged; // 0x1C
    SInt32 reserved2; // 0x1E
    void* compilecontext; // 0x22
    IDEDependencyInfo* dependencies; // 0x26
    short dependencyCount; // 0x2A
    OSSpec* objectfile; // 0x2C
} IDEObjectData;

typedef struct IDENewAccessPathInfo {
    OSSpec pathSpec; // 0x000
    SInt32 position; // 0x204
    Boolean recursive; // 0x208
    CWAccessPathType type; // 0x209
} IDENewAccessPathInfo;

typedef struct IDETargetInfo {
    short outputType; // 0x000
    OSSpec outfile; // 0x002
    OSSpec symfile; // 0x206
    OSSpec runfile; // 0x40A
    SInt16 linkType; // 0x60E
    Boolean canRun; // 0x610
    Boolean canDebug; // 0x611
    CWDataType targetCPU; // 0x612
    CWDataType targetOS; // 0x616
    Boolean runHelperIsRegKey; // 0x61A
    Boolean debugHelperIsRegKey; // 0x61B
    char args[512]; // 0x61C
    char runHelperName[512]; // 0x81C
    Boolean runHelperRequiresURL; // 0xA1C
    char reserved2; // 0xA1D
    char debugHelperName[512]; // 0xA1E
    OSSpec linkAgainstFile; // 0xC1E
} IDETargetInfo;

typedef struct CWTargetInfo_V7 {
    OSSpec outfile; // 0x000
    OSSpec symfile; // 0x204
    short linkType; // 0x408
    Boolean canRun; // 0x40A
    Boolean canDebug; // 0x40B
    Boolean useRunHelperApp; // 0x40C
    char reserved1; // 0x40D
    CWDataType debuggerCreator; // 0x40E
    CWDataType runHelperCreator; // 0x412
    unsigned char args[256]; // 0x416
    char runHelperName[512]; // 0x516
    Boolean runHelperRequiresURL; // 0x716
    SInt32 reserved2[2]; // 0x718
} CWTargetInfo_V7;

typedef struct CW_BasePluginCallbacks {
    CW_CALLBACK_PTR (*cbGetFileInfo)(CWPluginContext, SInt32, Boolean, IDEProjectFileInfo*);
    CW_CALLBACK_PTR (*cbFindAndLoadFile)(CWPluginContext, const char*, IDEFileInfo*);
    CW_CALLBACK_PTR (*cbGetFileText)(CWPluginContext, const OSSpec*, const char**, SInt32*, short*);
    CW_CALLBACK_PTR (*cbReleaseFileText)(CWPluginContext, const char*);
    CW_CALLBACK_PTR (*cbGetSegmentInfo)(CWPluginContext, SInt32, CWProjectSegmentInfo*);
    CW_CALLBACK_PTR (*cbGetOverlay1GroupInfo)(CWPluginContext, SInt32, CWOverlay1GroupInfo*);
    CW_CALLBACK_PTR (*cbGetOverlay1Info)(CWPluginContext, SInt32, SInt32, CWOverlay1Info*);
    CW_CALLBACK_PTR (*cbGetOverlay1FileInfo)(
        CWPluginContext, SInt32, SInt32, SInt32, CWOverlay1FileInfo*
    );
    CW_CALLBACK_PTR (*cbReportMessage)(
        CWPluginContext, const IDEMessageRef*, const char*, const char*, short, SInt32
    );
    CW_CALLBACK_PTR (*cbAlert)(CWPluginContext, const char*, const char*, const char*, const char*);
    CW_CALLBACK_PTR (*cbShowStatus)(CWPluginContext, const char*, const char*);
    CW_CALLBACK_PTR (*cbUserBreak)(CWPluginContext);
    CW_CALLBACK_PTR (*cbGetNamedPreferences)(CWPluginContext, const char*, CWMemHandle*);
    CW_CALLBACK_PTR (*cbStorePluginData)(CWPluginContext, SInt32, CWDataType, CWMemHandle);
    CW_CALLBACK_PTR (*cbGetPluginData)(CWPluginContext, SInt32, CWDataType, CWMemHandle*);
    CW_CALLBACK_PTR (*cbSetModDate)(CWPluginContext, const OSSpec*, OSTime*, Boolean);
    CW_CALLBACK_PTR (*cbAddProjectEntry)(
        CWPluginContext, const OSSpec*, Boolean, const CWNewProjectEntryInfo*, SInt32*
    );
    CW_CALLBACK_PTR (*cbCreateNewTextDocument)(CWPluginContext, const CWNewTextDocumentInfo*);
    CW_CALLBACK_PTR (*cbAllocateMemory)(CWPluginContext, SInt32, Boolean, void**);
    CW_CALLBACK_PTR (*cbFreeMemory)(CWPluginContext, void*, Boolean);
    CW_CALLBACK_PTR (*cbAllocMemHandle)(CWPluginContext, SInt32, Boolean, CWMemHandle*);
    CW_CALLBACK_PTR (*cbFreeMemHandle)(CWPluginContext, CWMemHandle);
    CW_CALLBACK_PTR (*cbGetMemHandleSize)(CWPluginContext, CWMemHandle, SInt32*);
    CW_CALLBACK_PTR (*cbResizeMemHandle)(CWPluginContext, CWMemHandle, SInt32);
    CW_CALLBACK_PTR (*cbLockMemHandle)(CWPluginContext, CWMemHandle, Boolean, void**);
    CW_CALLBACK_PTR (*cbUnlockMemHandle)(CWPluginContext, CWMemHandle);
    void* cbInternal[8]; // 0x68
    CW_CALLBACK_PTR (*cbGetTargetName)(CWPluginContext, char*, short);
    CW_CALLBACK_PTR (*cbCacheAccessPathList)(CWPluginContext);
    CW_CALLBACK_PTR (*cbPreDialog)(CWPluginContext);
    CW_CALLBACK_PTR (*cbPostDialog)(CWPluginContext);
    CW_CALLBACK_PTR (*cbPreFileAction)(CWPluginContext, const OSSpec*);
    CW_CALLBACK_PTR (*cbPostFileAction)(CWPluginContext, const OSSpec*);
    CW_CALLBACK_PTR (*cbCheckoutLicense)(
        CWPluginContext, const char*, const char*, UInt32, void*, SInt32*
    );
    CW_CALLBACK_PTR (*cbCheckinLicense)(CWPluginContext, SInt32);
    CW_CALLBACK_PTR (*cbResolveRelativePath)(
        CWPluginContext, const CWRelativePath*, OSSpec*, Boolean
    );
} CW_BasePluginCallbacks;

struct CWPluginPrivateContext {
#ifdef __cplusplus
    CWPluginPrivateContext(SInt32 thePluginType, SInt32 totalSize = 0);
    ~CWPluginPrivateContext();
#endif

    SInt32 request; // 0x000
    SInt32 apiVersion; // 0x004
    void* shellContext; // 0x008
    void* pluginStorage; // 0x00C
    OSSpec projectFile; // 0x010
    OSSpec outputFileDirectory; // 0x214
    SInt32 shellSignature; // 0x418
    SInt32 pluginType; // 0x41C
    SInt32 numFiles; // 0x420
    SInt32 numOverlayGroups; // 0x424
    UInt32 callbackOSError; // 0x428
    UInt32 pluginOSError; // 0x42C
    CWIDEInfo* shellInfo; // 0x430
    IDEAccessPathList* accessPathList; // 0x434
    SInt32 dontEatEvents; // 0x438
    OSSpec* targetDataDirectorySpec; // 0x43C
    struct SourceRelativeIncludesSupport* srcRelativeIncludes; // 0x440
    Boolean isRemote; // 0x444
    char reserved0[3]; // 0x445
    SInt32 reserved[15]; // 0x448
    CW_BasePluginCallbacks* callbacks; // 0x484
};

#ifdef __cplusplus
struct CWCompilerLinkerContext : CWPluginPrivateContext {
    CWCompilerLinkerContext();
    ~CWCompilerLinkerContext();

    SInt32 whichfile; // 0x488
    OSSpec sourcefile; // 0x48C
    const char* sourcetext; // 0x690
    SInt32 sourcetextsize; // 0x694
    Boolean precompile; // 0x698
    Boolean autoprecompile; // 0x699
    Boolean preprocess; // 0x69A
    Boolean cachingPCH; // 0x69B
    Boolean debuginfo; // 0x69C
    SInt16 fileID; // 0x69E
    CWBrowseOptions browseoptions; // 0x6A0
    Boolean recordbrowseinfo; // 0x6B0
    void* reserved; // 0x6B2
    SInt32 sequenceID; // 0x6B6
    CWCompilerLinkerContext* parentPB; // 0x6BA
    void* targetStorage; // 0x6BE
    Boolean sourceRelativeSearch; // 0x6C2
    Boolean checkingSyntax; // 0x6C3
    Boolean phasedBuild; // 0x6C4
    Boolean concurrentBuild; // 0x6C5
    SInt32 reservedcompiler[6]; // 0x6C6
    CWMemHandle texthandle; // 0x6DE
    CWTargetInfo_V7 targetinfo_V7; // 0x6E2
    IDETargetInfo* targetinfo; // 0xE02
    const char* commandLineArgs; // 0xE06
    OSSpec* workingDirectorySpec; // 0xE0A
    SInt32 numEnvironmentVariables; // 0xE0E
    CWEnvVarInfo* environmentVariables; // 0xE12
    SInt32 workingDirectoryError; // 0xE16
    SInt32 reservedlinker[2]; // 0xE1A
    int m_queryVersion; // 0xE22
    int m_queryCode; // 0xE26
    CWCompilerLinkerCallbacks* callbacks; // 0xE2A
};

struct CWParserContext : CWPluginPrivateContext {
    CWParserContext();
    ~CWParserContext();

    CWCommandLineArgs* args; // 0x488
    CWDataType cpu; // 0x48C
    CWDataType os; // 0x490
    const char* build_date; // 0x494
    const char* build_time; // 0x498
    const struct ToolVersionInfo* build_tool; // 0x49C
    int numPlugins; // 0x4A0
    const CLPluginInfo* plugins; // 0x4A4
    int numPanels; // 0x4A8
    const char** panelNames; // 0x4AC
    CWCommandLineArgs* plugin_args; // 0x4B0
    CWCommandLineArgs* panel_args; // 0x4B4
    const char* panelName; // 0x4B8
    void* panelData; // 0x4BC
    SInt32 panelSize; // 0x4C0
    struct CWParserCallbacks* callbacks; // 0x4C4
};
#endif

#pragma pack(pop)

extern void ConvertFSSpecToNativePath(CWPluginContext context, const OSSpec* oss, CWFileSpec* fss);
extern void ConvertNativePathToFSSpec(CWPluginContext context, const CWFileSpec* fss, OSSpec* oss);
extern void ConvertMacTimeToNativeTime(CWPluginContext context, OSTime time, CWFileTime* ft);
extern void ConvertNativeTimeToMacTime(CWPluginContext context, const CWFileTime* ft, OSTime* time);

extern CWResult CWSetPluginOSError(OSErr err);

extern CW_CALLBACK CWGetNamedPreferences(
    CWPluginContext context, const char* name, CWMemHandle* mem
);

extern CWResult __stdcall CWResizeMemHandle(
    CWPluginContext context, CWMemHandle handle, SInt32 size
);

#ifdef __cplusplus
}
#endif

#endif
