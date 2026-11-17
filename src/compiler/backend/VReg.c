#include "compiler/IroVars.h"
#include "compiler/common.h"
#include "compiler/PCode.h"
#include "compiler/CError.h"
#include "compiler/IroLoop.h"
#include "compiler/IroUtil.h"
#include "compiler/objects.h"
#include "compiler/CompilerTools.h"
#include "compiler/CParser.h"
#include "compiler/CFunc.h"
#include "compiler/InstrSelection.h"
#include "compiler/VReg.h"
#include "compiler/BitVectors.h"
#include "compiler/IroSubable.h"
#include "compiler/LiveRegisters.h"
#include "driver/Help.h"
#include "compiler/Registers.h"

Boolean fn_004D0330(IROLinear* expr) {
    IROLinear* child;
    ENode* node;
    Object* object;
    Type* type;

    switch (expr->type) {
        case IROLinearOp2Arg:
            if ((UInt8)(expr->nodetype - EADD) <= 1 && IRO_IsConstant(expr->u.diadic.right)) {
                object = IRO_IsVariable(expr->u.diadic.left);
                if (object && object->datatype == DLOCAL) {
                    VarInfo* info = object->u.var.info;
                    if (info && !info->noregister) {
                        return FALSE;
                    }
                } else {
                    child = expr->u.diadic.left;
                    if (child->type == IROLinearOperand && child->u.node->type == EOBJREF) {
                        return FALSE;
                    }
                }
            }
            return register_use_flags_61B910[(UInt8)expr->nodetype] != 0;
        case IROLinearOp1Arg:
            if (register_use_flags_61B910[(UInt8)expr->nodetype]) {
                return TRUE;
            }
            if (expr->nodetype == EINDIRECT) {
                if (!(expr->flags & 4)) {
                    if (expr->flags & 0x20) {
                        child = expr->u.diadic.left;
                        if (child->type == IROLinearOperand) {
                            node = child->u.node;
                            if (node->type == EOBJREF) {
                                object = node->data.objref;
                                if (object->datatype == DLOCAL &&
                                    object->u.var.info &&
                                    !object->u.var.info->noregister) {
                                    return FALSE;
                                }
                            }
                        }
                        return TRUE;
                    }
                    if (IRO_IsVariable(expr)) {
                        child = expr->u.diadic.left;
                        if ((UInt8)IRO_IsRegable(child->u.node->data.objref)) {
                            return FALSE;
                        }
                    }
                    return TRUE;
                }
            }
            if (expr->nodetype == ETYPCON) {
                type = expr->rtype;
                if (type->type == TYPEINT &&
                    TYPE_INTEGRAL(type)->integral < 0x11 &&
                    type->size >= expr->u.diadic.left->rtype->size) {
                    return TRUE;
                }
            }
            return FALSE;
        case IROLinearOperand:
            child = expr->u.monadic;
            if (child && child->type == EOBJREF && iro_reg_callback_6356E8) {
                if (iro_reg_callback_6356E8(child)) {
                    return TRUE;
                }
            }
            break;
        default:
false_result:
            return FALSE;
    }
    if (!copts.opt_pulloutconstants) {
        goto false_result;
    }
    if (IRO_IsIntConstant(expr)) {
        if (fn_004323E0(expr->u.node->data.intval)) {
            return TRUE;
        }
    }
    if (IRO_IsFloatConstant(expr)) {
        return TRUE;
    }
    goto false_result;
}

PCodeArg arg_from_operand(Operand* operand) {
    PCodeArg result;
    PCodeArg temporary;
    switch (operand->kind) {
        case 2:
        case 4:
            result.kind = PCOp_MEMORY;
            result.data.mem.offset = operand->immOffset;
            result.data.mem.obj = operand->object;
            break;
        case 0:
            result.kind = PCOp_REGISTER;
            result.arg = 0;
            result.data.reg.reg = operand->reg;
            result.effect = 1;
            break;
        case 3:
            temporary.kind = PCOp_CINT64;
            temporary.data.cint = operand->immediate;
            result = temporary;
            break;
        case 1:
            result.kind = PCOp_REGISTER;
            result.arg = 1;
            result.data.reg.reg = operand->reg;
            result.effect = 1;
            break;
        case 5:
            result.kind = PCOp_REGISTER;
            result.arg = 5;
            result.data.reg.reg = operand->reg;
            result.effect = 1;
            break;
        default:
            CError_Internal("VReg.c", 122);
            break;
    }
    return result;
}

Operand operand_from_pcode_arg(PCodeArg arg) {
    Operand result = {0};
    switch ((SInt8)arg.arg) {
        case 0:
            result.kind = 0;
            break;
        case 1:
            result.kind = 1;
            break;
        case 5:
            result.kind = 5;
            break;
        default:
            CError_Internal("VReg.c", 165);
            break;
    }
    result.reg = arg.data.reg.reg;
    return result;
}

Operand make_register_operand(int reg, int kind) {
    Operand result = {0};
    result.kind = kind;
    result.reg = reg;
    return result;
}

PCodeArg arg_from_label(CLabel* label) {
    PCodeArg result = {0};
    result.kind = PCOp_LABEL;
    if (!label->pclabel) {
        label->pclabel = makepclabel();
    }
    result.data.label.label = label->pclabel;
    return result;
}

PCodeArg fn_004D02D0(int reg, int kind, int effect) {
    PCodeArg result;
    result.kind = PCOp_REGISTER;
    result.arg = kind;
    result.data.reg.reg = reg;
    result.effect = effect;
    return result;
}

PCodeArg new_register_arg(SInt8 kind) {
    PCodeArg result;
    result.kind = PCOp_REGISTER;
    result.arg = kind;
    result.effect = 2;
    if (kind == 0) {
        result.data.reg.reg = used_virtual_registers[0]++;
    } else if (kind == 1) {
        result.data.reg.reg = used_virtual_registers[1]++;
    } else if (kind == 5) {
        result.data.reg.reg = used_virtual_registers[5]++;
    } else {
        CError_Internal("VReg.c", 60);
    }
    return result;
}

PCodeArg arg_from_int(SInt32 value) {
    PCodeArg result;
    result.kind = PCOp_CINT64;
    CInt64_SetLong(&result.data.cint, value);
    return result;
}

PCodeArg arg_from_cint64(CInt64 value) {
    PCodeArg result;
    result.kind = PCOp_CINT64;
    result.data.cint = value;
    return result;
}

Boolean IRO_IsAddressMultiplyDefault(void) {
    return FALSE;
}
