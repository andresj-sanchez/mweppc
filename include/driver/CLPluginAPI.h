#ifndef DRIVER_CLPLUGINAPI_H
#define DRIVER_CLPLUGINAPI_H

#include "driver/oslib.h"

#include "driver/CWPlugins.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void CL_OSTimeToCWFileTime(int pluginapi, const OSTime* ost, CWFileTime* ft);
extern void CL_CWFileTimeToOSTime(int pluginapi, const CWFileTime* ft, OSTime* ost);
extern UInt32 CL_OSSpecToCWFileSpec(int pluginapi, const OSSpec* oss, CWFileSpec* fss);
extern UInt32 CL_CWFileSpecToOSSpec(int pluginapi, const CWFileSpec* fss, OSSpec* oss);
extern int CL_PluginAPI(CWPluginContext context);

#ifdef __cplusplus
}
#endif

#endif
