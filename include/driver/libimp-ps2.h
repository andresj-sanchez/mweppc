#ifndef DRIVER_LIBIMP_PS2_H
#define DRIVER_LIBIMP_PS2_H

#include "driver/CWPlugins.h"

#ifdef __cplusplus
extern "C" {
#endif

extern CWPLUGIN_ENTRY(ELFImport_GetDropInFlags)(const DropInFlags** flags, SInt32* flagsSize);
extern CWPLUGIN_ENTRY(ELFImport_GetDropInName)(const char** dropinName);
extern CWPLUGIN_ENTRY(ELFImport_GetDisplayName)(const char** displayName);
extern CWPLUGIN_ENTRY(ELFImport_GetTargetList)(const CWTargetList** targetList);
extern CWPLUGIN_ENTRY(ELFImport_GetDefaultMappingList)(const CWExtMapList** defaultMappingList);
extern CWPLUGIN_ENTRY(ELFImport_GetPanelList)(const CWPanelList** panelList);

#ifdef __cplusplus
}
#endif

#endif
