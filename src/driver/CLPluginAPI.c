#include "driver/CLPluginAPI.h"
#include "driver/CLDropinCallbacks_V10.h"
#include "driver/CWPluginsPrivate.h"
#include "compiler/common.h"
#include "driver/CLErrors.h"
#include "driver/MacSpecs.h"
#include "driver/MsDos.h"
#include <string.h>

void CL_OSTimeToCWFileTime(int pluginapi, const OSTime* ost, CWFileTime* ft) {
    switch (pluginapi) {
        case 1:
            OS_TimeToMac(*ost, (UInt32*)ft);
            return;
        case 2:
            ft->dwLowDateTime = ost->dwLowDateTime;
            ft->dwHighDateTime = ost->dwHighDateTime;
            return;
        default:
            CLFatalError("Unsupported CWPLUGIN_API %d\n", pluginapi);
            return;
    }
}

void CL_CWFileTimeToOSTime(int pluginapi, const CWFileTime* ft, OSTime* ost) {
    switch (pluginapi) {
        case 1:
            if (!ft || !memcmp(ft, "\0\0\0\0\0\0\0\0", 8)) {
                OS_GetTime(ost);
            } else {
                OS_MacToTime(*(const UInt32*)ft, ost);
            }
            return;
        case 2:
            if (!ft || !memcmp(ft, "\0\0\0\0\0\0\0\0", 8)) {
                OS_GetTime(ost);
            } else {
                ost->dwLowDateTime = ft->dwLowDateTime;
                ost->dwHighDateTime = ft->dwHighDateTime;
            }
            return;
        default:
            CLFatalError("Unsupported CWPLUGIN_API %d\n", pluginapi);
            return;
    }
}

UInt32 CL_OSSpecToCWFileSpec(int pluginapi, const OSSpec* oss, CWFileSpec* fss) {
    switch (pluginapi) {
        case 1:
            return OS_OSSpec_To_FSSpec(oss, (FSSpec*)fss);
        case 2:
            if (OS_SpecToString(oss, fss->path, 260)) {
                return 0;
            }
            return 111;
        default:
            CLFatalError("Unsupported CWPLUGIN_API %d\n", pluginapi);
            return -1;
    }
}

UInt32 CL_CWFileSpecToOSSpec(int pluginapi, const CWFileSpec* fss, OSSpec* oss) {
    switch (pluginapi) {
        case 1:
            return OS_FSSpec_To_OSSpec((const FSSpec*)fss, oss);
        case 2:
            return OS_MakeSpec(fss->path, oss, NULL);
        default:
            CLFatalError("Unsupported CWPLUGIN_API %d\n", pluginapi);
            return -1;
    }
}

int CL_PluginAPI(CWPluginContext context) {
    return ((shellContextType*)context->shellContext)->cwplugin_api;
}
