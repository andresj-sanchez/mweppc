#ifndef COMPILER_CPREC_H
#define COMPILER_CPREC_H

#include "compiler/common.h"
#if !MWCCPS2_B52
#if MWCCPSP
#include "driver/COSToolsCLT.h"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern char* precomp_target_str;

extern void SetupPrecompiler(Boolean isPrecompiling);
extern void CleanupPrecompiler(void);
#if MWCCPS2_B52
extern void PreComp_StaticData(Object* obj, const void* data, OLinkList* links, SInt32 size);
extern void PrecompilerWrite(void);
extern void PrecompilerRead(short refnum, void* buffer);
#else
extern void PreComp_StaticData(Object* obj, const void* data, OLinkList* links, SInt32 size);
extern void PrecompilerWrite(void);
#if MWCCPSP
extern void PrecompilerRead(COSFileRef refnum, void* buffer);
#else
extern void PrecompilerRead(short refnum, void* buffer);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
