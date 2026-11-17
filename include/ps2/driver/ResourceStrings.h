#ifndef DRIVER_RESOURCESTRINGS_H
#define DRIVER_RESOURCESTRINGS_H

#include "compiler/common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* Res_GetResource(SInt16 rsrcid, SInt16 index);
extern int Res_AddResource(const char* name, SInt16 rsrcid, const char** strings);

#ifdef __cplusplus
}
#endif

#endif
