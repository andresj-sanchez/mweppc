#ifndef COMPILER_CEXPRCONVMATCH_H
#define COMPILER_CEXPRCONVMATCH_H

#include "compiler/common.h"

typedef struct Match {
    struct Match* next;
    Object* object;
    Object* specialfunc;
    Type* type;
    UInt32 qual;
    Type* type2;
    UInt32 qual2;
} Match;

extern ENode* CExpr_Convert(
    ENode* expr, Type* type, UInt32 qual, Boolean isExplicit, Boolean checkAccess
);
extern void fn_0054A9B0(PCode* pcode, UInt32* vec);

extern Match* CExpr_CheckBinaryBuiltin(Match* matches, ENode* left, short token, ENode* right);
extern Match* CExpr_FindBestMatch(
    Match* matches, int count, HashNameNode* name, ObjectList** outList, ENodeList* argExprs
);
extern ENode* fn_00547690(ENode*, Type*, SInt32, SInt32);

#endif
