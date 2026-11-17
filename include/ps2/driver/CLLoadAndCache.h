#ifndef DRIVER_CLLOADANDCACHE_H
#define DRIVER_CLLOADANDCACHE_H

#include "compiler/common.h"
#include "driver/oslib.h"

#ifdef __cplusplus
extern "C" {
#endif

extern short FixTextHandle(OSHandle* txt);
extern int LoadAndCacheFile(OSSpec* spec, OSHandle** texthandle, Boolean* precomp);
extern void CopyFileText(OSHandle* src, char** text, SInt32* textsize);

#ifdef __cplusplus
}
#endif

#endif
