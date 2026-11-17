#include "driver/CWPlugins.h"
#include "driver/DropInCompilerLinker.h"

#pragma export on

CWPLUGIN_ENTRY(ELFImport_GetDropInFlags)(const DropInFlags** flags, SInt32* flagsSize) {
    short result = 0;
    static DropInFlags sFlags = {
        kCurrentDropInFlagsVersion,
        CWDROPINCOMPILERTYPE,
        DROPINCOMPILERLINKERAPIVERSION_9,
        kGeneratescode | kCompMultiTargAware,
        Lang_MISC,
        DROPINCOMPILERLINKERAPIVERSION_13
    };

    *flags = &sFlags;
    *flagsSize = sizeof(sFlags);
    return result;
}

CWPLUGIN_ENTRY(ELFImport_GetDropInName)(const char** dropinName) {
    short result = 0;
    static const char* sDropInName = "ELF Importer";

    *dropinName = sDropInName;
    return result;
}

CWPLUGIN_ENTRY(ELFImport_GetDisplayName)(const char** displayName) {
    short result = 0;
    static const char* sDisplayName = "ELF Importer";

    *displayName = sDisplayName;
    return result;
}

CWPLUGIN_ENTRY(ELFImport_GetTargetList)(const CWTargetList** targetList) {
    short result = 0;
    static SInt32 sOS[1] = {targetOSAny};
    static SInt32 sCPU[1] = {targetCPUAny};
    static CWTargetList sTargetList = {kCurrentCWTargetListVersion, 1, sCPU, 1, sOS};

    *targetList = &sTargetList;
    return result;
}

CWPLUGIN_ENTRY(ELFImport_GetDefaultMappingList)(const CWExtMapList** defaultMappingList) {
    short result = 0;
    // clang-format off
    static CWExtensionMapping sExtension[6] = {
        {'ELF ', ".o", 0},
        {'ELF ', "", 0},
        {0, ".elf", 0},
        {'MPLF', ".lib", 0},
        {'MPLF', ".a", 0},
        {'MPLF', "", 0}
    };
    // clang-format on
    static CWExtMapList sExtensionMapList = {kCurrentCWExtMapListVersion, 6, sExtension};

    *defaultMappingList = &sExtensionMapList;
    return result;
}

CWPLUGIN_ENTRY(ELFImport_GetPanelList)(const CWPanelList** panelList) {
    short result = 0;
    static CWPanelList sPanelList = {kCurrentCWPanelListVersion, 0, NULL};

    *panelList = &sPanelList;
    return result;
}

#pragma export reset
