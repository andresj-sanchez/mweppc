#ifndef DRIVER_TARGETOPTIMIZERHELPERS_MIPS_H
#define DRIVER_TARGETOPTIMIZERHELPERS_MIPS_H

#include "compiler/common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char inlineIntrinsicsEnabled;
extern int TargetSetOptFlags(short option, unsigned char value);
extern void TargetDisplayOptimizationOptions(Ptr* output);
extern void Utils_Empty(void); //! @reconstructed

#ifdef __cplusplus
}
#endif

#endif
