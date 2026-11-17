#ifndef DRIVER_PARSERHELPERS_H
#define DRIVER_PARSERHELPERS_H

#include "compiler/common.h"
#include "driver/oslib.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

extern SInt16 lastStage;
extern Boolean dashIMinusMovesPaths;
extern Boolean usedDashIMinus;
extern Boolean namingSysPaths;

extern char* GetEnvVar(const char* name, Boolean warn, const char** match);
extern int Opt_AddAccessPath(const char* opt, void* var, const char* arg, int flags);
extern int AddAccessPathList(
    const char* list,
    char sep1,
    char sep2,
    int source,
    char* text,
    Boolean system,
    SInt32 position,
    Boolean recursive
);
extern int Opt_FindAndAddFile(const char* opt, void* var, const char* arg, int flags);
extern void AddStringToHandle(Handle* h, const char* str);
extern int Opt_PrintVersion(const char* opt, void* var, const char* arg, int flags);
extern int Opt_SavePrefs(const char* opt, void* var, const char* arg, int flags);
extern int Opt_MaybeMoveAccessPaths(const char* opt, void* var, const char* arg, int flags);
extern int ParseNumber(const char* arg, Boolean emit_error, SInt32* ret, const char** endptr);
extern void GetFirstSourceFilenameBase(char* buffer, char* defaul);
extern void AddStringLenToHandle(Handle* h, const char* str, int len);
extern int Opt_AddUnixLibraryFile(const char* opt, void* var, const char* name);
extern void ListParseMessage(
    void (*errprint)(const char*, va_list), const char* envvar, VarArgShort id, ...
);
extern int FindFileInPath(const char* filename, OSSpec* fss);

#ifdef __cplusplus
}
#endif

#endif
