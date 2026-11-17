#ifndef COMPILER_IROEXPRS_H
#define COMPILER_IROEXPRS_H

#include "compiler/common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void IRO_DumpExprs(void);
extern void IRO_RemoveExpr(IROExpr* expr);

extern char IRO_ExprsDependsLabel[];
extern char IRO_ExprsDisplayFormat[];
extern char IRO_ExprsFileName[];
extern char IRO_ExprsHeading[];
extern char IRO_ExprsLineEnd[];

#ifdef __cplusplus
}
#endif

#endif
