#ifndef DRIVER_LIBIMP_EABI_PPC_H
#define DRIVER_LIBIMP_EABI_PPC_H

#include "driver/CWPlugins.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const VersionInfo elfImportVersionInfo;
extern const OSFileTypeMappingList elfImportFileTypeMappings;
extern const CWObjectFlags elfImportObjectFlags;
extern const BasePluginCallbacks elfImportBaseCallbacks;
extern const CompilerLinkerPluginCallbacks elfImportCompilerCallbacks;
extern const char* elfImportErrorStrings[];
extern const char elfImportErrorName[];
extern int RegisterStaticLibImporterPlugin(void);
extern int RegisterLibImporterResources(void);

#ifdef __cplusplus
}
#endif

#endif
