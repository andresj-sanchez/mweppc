#ifndef COMPILER_IROTRANSFORM_H
#define COMPILER_IROTRANSFORM_H

#include "compiler/common.h"
#if MWCCPS2_B52
#pragma pack(push, 2)
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void IRO_InitializeAssignmentOpArray(void);
extern void IRO_InitializeComplementaryOpArray(void);
extern void IRO_InitializeComplementaryOpLogicalArray(void);
extern void IRO_DoTransformations(void);
extern Boolean IRO_TransformSelfAssignmentToAssignment(IROLinear* nd);
extern void IRO_TransformTree(Statement* statements);

#ifdef __cplusplus
}
#endif

#if MWCCPS2_B52
#pragma pack(pop)
#endif
#endif
