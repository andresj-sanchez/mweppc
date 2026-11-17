#include "compiler/IroLoop.h"

Boolean IRO_IsVectorTempCandidateDefault(IROLinear* linear) {
    int result = FALSE;
    int hasBase = FALSE;

    if (linear->type >= IROLinearOperand &&
        linear->type <= IROLinearFunccall &&
        (linear->flags & 0x100)) {
        hasBase = TRUE;
    }
    if (hasBase && !(linear->flags & 0x20)) {
        result = TRUE;
    }
    return result;
}

Boolean IRO_DoesNotHaveVectorOperand(IROLinear* linear) {
    return TRUE;
}
