#include "compiler/CError.h"
#include "compiler/CMangler.h"
#include "compiler/CompilerTools.h"
#include "compiler/common.h"
#include "compiler/enode.h"
#include "compiler/objects.h"
#include "compiler/types.h"
#include "compiler/CExprConvMatch.h"

#pragma pack(push, 2)

struct Conversion {
    ENode* expression;
    ENode* left;
    ENode* right;
};
#pragma pack(pop)

Boolean CExpr_CondOperatorMatch(ENode* left, ENode* right, Conversion* conv) {
    Match* match;

    match = CExpr_CheckBinaryBuiltin(NULL, left, ':', right);
    if (match) {
        match = CExpr_FindBestMatch(match, 1, GetHashNameNodeExport("operator?:"), NULL, NULL);
        CError_ASSERT(3026, !match->object);

        conv->expression = NULL;
        conv->left = CExpr_Convert(left, match->type, match->qual, FALSE, TRUE);
        conv->right = CExpr_Convert(right, match->type2, match->qual2, FALSE, TRUE);
        return TRUE;
    }

    return FALSE;
}
