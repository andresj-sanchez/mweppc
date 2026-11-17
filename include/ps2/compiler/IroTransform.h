#ifndef COMPILER_IROTRANSFORM_H
#define COMPILER_IROTRANSFORM_H

#include "compiler/common.h"
#include "compiler/CFunc.h"

extern void switchFatherLeft(IROLinear* nd, int isleft);
extern Boolean gWarnForDivByZero;
extern Boolean gWarnForFloatDivByZero;
extern void IRO_WarnForDivisionsByZero(IROLinear* nd);
extern void IRO_DoTransformations(IROLinear* list);
extern void IRO_TransformTree(Statement* statements);
extern void IRO_TransformLogicalSelections(Statement* statements);

extern Boolean gCollapseLogicalSelectionIntoBitTest;

#endif
