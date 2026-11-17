#ifndef DRIVER_CLPREFS_H
#define DRIVER_CLPREFS_H

#include "compiler/common.h"
#include "driver/oslib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PrefPanel {
    char* name;
    OSHandle data;
    struct PrefPanel* next;
} PrefPanel;

typedef struct PrefPanelContents {
    uOSTypePair type;
    OSSpec spec;
} PrefPanelContents;

extern PrefPanel* PrefPanel_New(const char* name, void* initdata, SInt32 initdatasize);
extern OSHandle* PrefPanel_GetHandle(PrefPanel* panel, OSHandle* out);
extern int PrefPanel_PutHandle(PrefPanel* panel, OSHandle* data);
extern void Prefs_Initialize(void);
extern void Prefs_Terminate(void);
extern Boolean Prefs_AddPanel(PrefPanel* panel);
extern PrefPanel* Prefs_FindPanel(const char* name);
extern Boolean PrefPanel_SetContentsByName(PrefPanelContents* contents);

#ifdef __cplusplus
}
#endif

#endif
