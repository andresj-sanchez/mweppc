#include "driver/CLPrefs.h"
#include "driver/CLErrors.h"
#include "driver/CLIO.h"
#include "driver/CLMain.h"
#include "driver/Generic.h"
#include "driver/MemUtils.h"
#include "driver/MsDos.h"
#include "driver/StringExtras.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static PrefPanel* panellist;

Boolean PrefPanel_SetContentsByName(PrefPanelContents* contents) {
    contents->type.perm = 0;
    OS_GetCWD(&contents->spec.path);
    OS_MakeNameSpec("", &contents->spec.name);
    return TRUE;
}

PrefPanel* Prefs_FindPanel(const char* name) {
    PrefPanel* scan;

    for (scan = panellist; scan; scan = scan->next) {
        if (!ustrcmp(scan->name, name)) {
            break;
        }
    }

    return scan;
}

Boolean Prefs_AddPanel(PrefPanel* panel) {
    PrefPanel** scan;

    for (scan = &panellist; *scan; scan = &(*scan)->next) {
        if (!strcmp((*scan)->name, panel->name)) {
            CLReportError(CLStr90, panel->name);
            return FALSE;
        }
    }

    if (clState.pluginDebug) {
        CLPrint("Defining/adding pref panel '%s'\n", panel->name);
    }

    *scan = panel;
    return TRUE;
}

void Prefs_Terminate(void) {
    PrefPanel* scan;
    PrefPanel* next;

    for (scan = panellist; scan; scan = next) {
        xfree(scan->name);
        OS_FreeHandle(&scan->data);
        next = scan->next;
        xfree(scan);
    }
}

void Prefs_Initialize(void) {
    panellist = NULL;
}

int PrefPanel_PutHandle(PrefPanel* panel, OSHandle* data) {
    UInt32 len;

    OS_GetHandleSize(data, &len);
    OS_FreeHandle(&panel->data);
    OS_CopyHandle(data, &panel->data);
    return TRUE;
}

OSHandle* PrefPanel_GetHandle(PrefPanel* panel, OSHandle* out) {
    if (OS_CopyHandle(&panel->data, out)) {
        return NULL;
    }

    return out;
}

PrefPanel* PrefPanel_New(const char* name, void* initdata, SInt32 volatile initdatasize) {
    PrefPanel* pnl;
    SInt32 size;
    char* ptr;

    size = initdatasize;
    pnl = xmalloc(NULL, sizeof(PrefPanel));
    if (!pnl) {
        return pnl;
    }

    pnl->name = xstrdup(name);
    if (OS_NewHandle(size, &pnl->data)) {
        fprintf(stderr, "\n*** Out of memory\n");
        exit(-23);
    }

    ptr = OS_LockHandle(&pnl->data);
    if (initdata) {
        memcpy(ptr, initdata, size);
    } else {
        memset(ptr, 0, size);
    }
    OS_UnlockHandle(&pnl->data);

    pnl->next = NULL;
    return pnl;
}
