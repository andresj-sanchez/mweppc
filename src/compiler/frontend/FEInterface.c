#include "compiler/RegisterInfo.h"
#include "compiler/RegListQueries.h"
#include "compiler/common.h"
#include "compiler/IroLoop.h"
#include "compiler/enode.h"
#include "compiler/CError.h"
#include "compiler/CParser.h"
#include "compiler/CPrep.h"
#include "compiler/IrOptimizer.h"
#include "compiler/VReg.h"
#include "compiler/FEInterface.h"
#include "compiler/IroLoopVector.h"
#include "compiler/CodeGen.h"

Boolean fn_00439CA0(IROLinear* expr) {
    IROLinear* inner;
    Type* type;
    ENode* node;
    Object* object;
    ObjRegInfo* info;

    if (!fn_004D0330(expr)) {
        return FALSE;
    }
    if (!copts.cse_vector_type &&
        ((type = expr->rtype) == (Type*)&stint128 || CABI_HasSideEffects(type))) {
        return FALSE;
    }
    if (expr->nodetype != EINDIRECT) {
        return TRUE;
    }
    inner = expr->u.monadic;
    if (!inner) {
        CError_Internal(FEInterface_CFile_5E26C3, 567);
        return FALSE;
    }
    if (inner->nodetype != EOBJREF || !(node = inner->u.node)) {
        return TRUE;
    }
    if (node->type != EOBJREF || (object = node->data.objref) == NULL) {
        return TRUE;
    }
    if (!fn_004727A0(object)) {
        return TRUE;
    }
    info = Registers_GetVarInfo(object);
    if (info) {
        switch (info->rclass) {
            case 0:
                return copts.cse_hard_reg_gpr;
            case 1:
                return copts.cse_hard_reg_fpr;
            case 5:
            case 6:
                return copts.cse_hard_reg_vu0;
            default:
                if (!copts.cse_hard_reg_special) {
                    return FALSE;
                }
                return TRUE;
        }
    }
    return TRUE;
}

void fn_00439C70(void) {
    fn_004D0C80();
    copts.unknown130 = (SInt8)copts.optimizationlevel >= 3;
    copts.opt_movepostops = TRUE;
    copts.opt_pulloutconstants = TRUE;
    copts.unknown12F = FALSE;
}

void fn_00439BF0(void) {
    void (*callbacks[21])(void) = {0};

    callbacks[14] = fn_00439D90;
    callbacks[3] = fn_00439DC0;
    callbacks[12] = fn_00439DA0;
    callbacks[0] = (void (*)(void))fn_00439CA0;
    callbacks[1] = (void (*)(void))IRO_IsAddressMultiplyDefault;
    callbacks[4] = (void (*)(void))CodeGen_GetAsmEffects;
    callbacks[5] = (void (*)(void))CodeGen_PropagateIntoAsm;
    callbacks[6] = fn_0043A3C0;
    callbacks[10] = IRO_IsAssignmentReductionCandidateDefault;
    callbacks[11] = (void (*)(void))CodeGen_ParseAttribute;
    callbacks[16] = fn_00439DB0;
    IRO_Setup(callbacks);
}
