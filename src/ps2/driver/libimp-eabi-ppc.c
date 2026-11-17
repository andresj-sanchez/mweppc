#include "driver/libimp-eabi-ppc.h"
#include "driver/ClientGlue.h"
#include "driver/ELF_Lib_Import.h"
#include "driver/libimp-ps2.h"

static CWPLUGIN_ENTRY(ELFImport_GetVersionInfo)(const VersionInfo** versionInfo) {
    short result = 0;
    *versionInfo = &elfImportVersionInfo;
    return result;
}

static CWPLUGIN_ENTRY(ELFImport_GetFileTypeMappings)(const OSFileTypeMappingList** mappings) {
    short result = 0;
    *mappings = &elfImportFileTypeMappings;
    return result;
}

static CWPLUGIN_ENTRY(ELFImport_GetObjectFlags)(const CWObjectFlags** objectFlags) {
    short result = 0;
    *objectFlags = &elfImportObjectFlags;
    return result;
}

const BasePluginCallbacks elfImportBaseCallbacks = {
    ELFImport_main,
    ELFImport_GetDropInFlags,
    ELFImport_GetDisplayName,
    ELFImport_GetDropInName,
    ELFImport_GetPanelList,
    NULL,
    NULL,
    ELFImport_GetVersionInfo,
    ELFImport_GetFileTypeMappings
};

const CompilerLinkerPluginCallbacks elfImportCompilerCallbacks = {
    ELFImport_GetTargetList,
    ELFImport_GetDefaultMappingList,
    NULL,
    NULL,
    ELFImport_GetObjectFlags,
    NULL
};

int RegisterStaticLibImporterPlugin(void) {
    return RegisterStaticCompilerLinkerPlugin(&elfImportBaseCallbacks, &elfImportCompilerCallbacks);
}

int RegisterLibImporterResources(void) {
    return RegisterResource(elfImportErrorName, 0x38F, elfImportErrorStrings);
}
