#ifndef MSL_SETUPARGS_H
#define MSL_SETUPARGS_H

#ifdef __cplusplus
extern "C" {
#endif

extern char** __argv;
extern int __argc;
extern int __argv1_offs;
extern char* _acmdln;
extern void _SetupArgs(void);

#ifdef __cplusplus
}
#endif

#endif
