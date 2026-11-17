#ifndef COMPILER_IROCSE_H
#define COMPILER_IROCSE_H

#include "compiler/common.h"
#include "compiler/BitVector.h"

#ifdef __cplusplus
extern "C" {
#endif

struct IROExpr {
    Boolean flags; /* 0x00 */
    SInt32 index; /* 0x04 */
    IROLinear* linear; /* 0x08 */
    Object* temp; /* 0x0C */
    IRONode* node; /* 0x10 */
    BitVector* depends; /* 0x14 */
    UInt32 unk18;
    IROExpr* use; /* 0x1C */
    UInt8 unk20;
    Boolean couldError; /* 0x21 */
    Boolean notSubable; /* 0x22 */
    IROLinear* factor;
    VarRecord* var;
    IROLinear* indOperand;
    IROExpr* next; /* 0x30 */
};

extern BitVector* IRO_Depends;
extern Boolean IRO_NotSubable;
extern Boolean IRO_IsVolatile;
extern Boolean IRO_CouldError;
extern IROExpr* IRO_FirstExpr;
extern IROExpr* IRO_LastExpr;
extern SInt32 IRO_NumExprs;

extern void IRO_FindDepends_NoAlloc(IROLinear* linear);
extern void IRO_FindDepends(IROLinear* linear);
extern void IRO_FindExpressions(BitVector* bv, Boolean unused);
extern void IRO_ComputeAvail(void);
extern void IRO_CommonSubs(void);

#ifdef __cplusplus
}
#endif
extern void IRO_GenerateTopLevelExprsForSubableOperands(void);

#endif
