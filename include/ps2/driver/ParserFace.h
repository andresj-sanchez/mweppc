#ifndef DRIVER_PARSERFACE_H
#define DRIVER_PARSERFACE_H

#include "compiler/common.h"
#include "driver/cmdline.h"
#include "driver/CWPlugins.h"
#include "driver/oslib.h"
#include "driver/parser.h"
#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PrefDataPanel {
    char* name;
    void* ptr;
    SInt32 size;
};

struct ParserTool {
    OSType TYPE;
    OSType LANG;
    OSType CPU;
    OSType OS;
    int numPrefPanels;
    char** prefPanels;
    char* toolInfo;
    char* copyright;
    int numOptionLists;
    struct OptionList** optionLists;
    int numPrefDataPanels;
    PrefDataPanel* prefDataPanels;
    int (*PreParse)(void);
    int (*MidParse)(void);
    int (*PostParse)(void);
};

// 0x44
typedef struct ParseOptsType {
    struct CWPluginPrivateContext* context;
    char helpKey[64];
    SInt32 helpFlags;
    UInt16 ioCols;
    UInt16 ioRows;
    CWCommandLineArgs* args;
    const ToolVersionInfo* toolVersion;
    int numPlugins;
    const CLPluginInfo* plugins;
    int numPanels;
    const char** panelNames;
    OSType cpu;
    OSType os;
    char lastoutputname[256];
    SInt32 currentSegment;
    SInt32 currentOverlayGroup;
    SInt32 currentOverlay;
    int possibleFiles;
    int userSpecifiedFiles;
    int unusedFiles;
    Boolean hadAnyOutput;
    Boolean hadErrors;
    Boolean showHelp;
    Boolean underIDE;
    Boolean alwaysUsePaths;
    Boolean noOptions;
    Boolean printedVersion;
    Boolean passingArgs;
    Boolean disToFile;
    Boolean ppToFile;
    Boolean initBefore;
    Boolean weakImport;
    Boolean mergeIntoOutput;
    Boolean success;
    Boolean ignoreUnknown;
    UInt8 unused[2];
} ParseOptsType;

extern CWMemHandle Parser_FindPrefPanel(const char* name);
extern CWResult Parser_StorePanels(CWPluginContext context);

extern const char* failedCallback;
extern jmp_buf exit_plugin;
extern ParseOptsType parseopts;

#ifdef __cplusplus
}
#endif

#endif
