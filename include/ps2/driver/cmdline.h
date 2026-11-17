#ifndef DRIVER_CMDLINE_H
#define DRIVER_CMDLINE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IDEAccessPathList IDEAccessPathList;
typedef struct Path Path;
typedef struct Paths Paths;
typedef struct File File;
typedef struct Target Target;
typedef struct IDEAccessPath IDEAccessPath;
typedef struct VFile VFile;
typedef struct InclFile InclFile;
typedef struct Deps Deps;
typedef struct Incls Incls;
typedef struct Files Files;
typedef struct CWCommandLineArgs CWCommandLineArgs;
struct CWCommandLineArgs {
    int argc;
    char** argv;
    char** envp;
};
typedef struct CLPluginInfo CLPluginInfo;
typedef struct Plugin Plugin;
typedef struct ToolVersionInfo ToolVersionInfo;

#ifdef __cplusplus
}
#endif

#endif
