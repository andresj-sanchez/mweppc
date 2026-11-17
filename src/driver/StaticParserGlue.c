#if MWCCPS2_B198
#include "driver/StaticParserGlue.h"
#include "driver/ClientGlue.h"
#include "driver/CWPlugins.h"
CWPLUGIN_ENTRY(parser_main)(CWPluginContext context);
CWPLUGIN_ENTRY(CWParser_GetDropInFlags)(const DropInFlags** flags, SInt32* flagsSize);
CWPLUGIN_ENTRY(CWParser_GetDropInName)(const char** name);
CWPLUGIN_ENTRY(CWParser_GetDisplayName)(const char** name);
CWPLUGIN_ENTRY(CWParser_GetPanelList)(const CWPanelList** panels);
CWPLUGIN_ENTRY(CWParser_GetVersionInfo)(const VersionInfo** vi);
CWPLUGIN_ENTRY(Parser_SupportsPlugin)(
    const CLPluginInfo* p, CWDataType cpu, CWDataType os, Boolean* supported
);
CWPLUGIN_ENTRY(Parser_SupportsPanels)(int count, const char** names, Boolean* supported);
#else
#include "driver/StaticParserGlue.h"
#include "driver/ClientGlue.h"
#include "driver/CWPlugins.h"
#include "driver/ParserFace.h"

#endif
// clang-format off
static BasePluginCallbacks cb = {
    parser_main,
    CWParser_GetDropInFlags,
    CWParser_GetDisplayName,
    CWParser_GetDropInName,
    CWParser_GetPanelList,
    NULL,
    NULL,
    CWParser_GetVersionInfo,
    NULL
};
// clang-format on

// clang-format off
static ParserPluginCallbacks pr_cb = {
    Parser_SupportsPlugin,
    Parser_SupportsPanels
};
// clang-format on

int RegisterStaticParserPlugins(void) {
    return RegisterStaticParserPlugin(&cb, &pr_cb);
}

#if MWCCPS2_B198
#include "driver/ParserErrors.r"

#else
#include "ParserErrors.r"

#endif
int RegisterStaticParserResources(void) {
    return RegisterResource("Parser Strings", 12010, STR12010);
}
