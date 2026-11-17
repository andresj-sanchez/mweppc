#ifndef COMPILER_IROSUBABLE_H
#define COMPILER_IROSUBABLE_H

#include "compiler/common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void IRO_InitializeIsSubableOpArray(void);
extern Boolean IRO_IsSubableExpression(IROLinear* nd);

extern UInt8 register_use_flags_61B910[];

#ifdef __cplusplus
}
#endif

#endif
