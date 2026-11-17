#include "compiler/CodeGen.h"
#include "compiler/ObjGen.h"
#include "compiler/common.h"
#include "compiler/CompilerTools.h"
#include "compiler/types.h"
#include "compiler/CInt64.h"
#include "compiler/CMachine.h"
#include "compiler/CParser.h"
#include "compiler/enode.h"
#include "compiler/objects.h"
#include "compiler/CFunc.h"
#include "compiler/Exceptions.h"
#include "compiler/ExprPreparation.h"
#include "compiler/InlineAsmMIPS.h"
#include "compiler/CInit.h"
#include "compiler/Intrinsics.h"
#include "compiler/CExprConvMatch.h"

#pragma dont_inline on
ENode* fn_004AF7A0(ENode* source) {
    ENode* result;

    result = lalloc(sizeof(ENode));
    memclrw(result, sizeof(ENode));
    *result = *source;
    return result;
}

#pragma dont_inline reset

Object* fn_004AFA60(CLabel* label) {
    ExprPrepObjectLink* entry;
    Object* object;

    for (entry = exprprep_objects_6361C8; entry; entry = entry->next) {
        if (entry->label == label) {
            return entry->object;
        }
    }
    object = galloc(sizeof(Object));
    object->type = TYPE(&void_ptr);
    object->name = label->uniquename;
    object->sclass = 0x102;
    object->qual = 1;
    object->datatype = DDATA;
    object->flags |= 2;
    entry = galloc(sizeof(ExprPrepObjectLink));
    entry->object = object;
    entry->label = label;
    entry->next = exprprep_objects_6361C8;
    exprprep_objects_6361C8 = entry;
    return object;
}

Boolean fn_004AFBD0(Type* type, double value) {
    float narrow;
    UInt32 word;
    FloatStorage wide;
    UInt32 lo;
    UInt32 hi;
    Boolean result;
    if (!copts.optimizesize) {
        return TRUE;
    }
    if (TYPE_INTEGRAL(type)->integral == IT_FLOAT || (type->type == TYPEFLOAT && type->size == 4)) {
        narrow = (float)value;
        word = *(UInt32*)&narrow;
        if ((word & 0xFFFF) == 0) {
            result = TRUE;
        } else {
            result = FALSE;
        }
        return result;
    }
    wide = *(FloatStorage*)&value;
    lo = wide.words[0];
    hi = wide.words[1];
    if (lo == 0) {
        if ((hi & 0xFFFF) == 0) {
            result = TRUE;
        } else {
            result = FALSE;
        }
        return result;
    }
    if (hi == 0) {
        if ((lo & 0xFFFF) == 0) {
            result = TRUE;
        } else {
            result = FALSE;
        }
        return result;
    }
    return FALSE;
}

void fn_004AEF80(ENode* expr) {
    ENodeList* args;
    UInt8 kind;
    args = expr->data.funccall.args;
    if (args->node->type == EOBJREF &&
        (args = args->next, kind = args->node->type, kind == EOBJREF && kind == EINTCONST)) {
        fn_00466380(expr, 9);
    } else {
        expr->hascall = 1;
        codegen_register_option_637385 = 0;
    }
}

ENode* fn_004AF990(ENode* expr) {
    ENode* result;
    ENode* value;
    SInt32 isSingle;
    Type* type;
    result = lalloc(sizeof(ENode));
    value = lalloc(sizeof(ENode));
    type = expr->rtype;
    if (type->type == TYPEFLOAT) {
        value->type = EFLOATCONST;
        value->cost = 0;
        type = *(Type* volatile*)&expr->rtype;
        isSingle = 1;
        if (TYPE_INTEGRAL(type)->integral != IT_FLOAT &&
            (type->type != TYPEFLOAT || type->size != 4)) {
            isSingle = 0;
        }
        value->rtype = isSingle ? TYPE(&stfloat) : TYPE(&stdouble);
        value->data.floatval = exprprep_float_zero_609060;
    } else {
        value->type = EINTCONST;
        value->cost = 0;
        value->rtype = TYPE(&stsignedint);
        value->data.intval = cint64_zero;
    }
    result->type = ENOTEQU;
    result->cost = expr->cost;
    result->rtype = TYPE(&stsignedint);
    result->data.diadic.left = expr;
    result->data.diadic.right = value;
    return result;
}

void fn_004AF1B0(ENode* expr) {
    ENode* saved;
    ENode* value;
    Type* type;
    saved = expr->data.monadic;
    value = lalloc(sizeof(ENode));
    type = expr->rtype;
    if (type->type == TYPEFLOAT) {
        value->type = EFLOATCONST;
        value->cost = 0;
        value->rtype = type;
        value->data.floatval = CMach_CalcFloatConvertFromInt(TYPE(&stsignedint), cint64_one);
    } else if (type->type == TYPEPOINTER) {
        value->type = EINTCONST;
        value->cost = 0;
        value->rtype = TYPE(&stunsignedint);
        CInt64_SetULong(&value->data.intval, TPTR_TARGET(type)->size);
    } else {
        value->type = EINTCONST;
        value->cost = 0;
        value->rtype = type;
        value->data.intval = cint64_one;
    }
    expr->type = expr->type == EPREDEC ? ESUBASS : EADDASS;
    expr->data.diadic.left = saved;
    expr->data.diadic.right = value;
}

void fn_004AEFC0(ENode* expr) {
    ENodeList* args;
    ENode* inner;
    ENode* value;
    char* str;

    if (!copts.poolstrings &&
        copts.readonly_strings &&
        (args = expr->data.funccall.args)->node->type == EOBJREF &&
        (args = args->next, inner = args->node, inner->type == ESTRINGCONST)) {
        args->next = lalloc(sizeof(ENodeList));
        memclrw(args->next, sizeof(ENodeList));
        args = args->next;
        args->node = lalloc(sizeof(ENode));
        memclrw(args->node, sizeof(ENode));
        value = args->node;
        value->type = EINTCONST;
        value->cost = 0;
        value->rtype = TYPE(&stsignedint);
        str = inner->data.string.data;
        CInt64_SetLong(&value->data.intval, (__builtin_strlen(str) + 1));
        fn_00466380(expr, 8);
    } else {
        expr->hascall = 1;
        codegen_register_option_637385 = 0;
    }
}

Object* fn_004AFAE0(Type* type, const UInt32* bits) {
    ObjectList* entry;
    Object* object;
    UInt32* data;
    UInt32 low, high;
    UInt8 regclass;
    entry = objgen_float_constants_636E6C;
    if (entry) {
        goto scan;
    }
create:
    object = galloc(sizeof(Object));
    memclrw(object, sizeof(Object));
    object->type = type;
    object->name = CParser_GetUniqueName();
    object->sclass = 0x102;
    object->qual = 0x10001;
    object->datatype = DDATA;
    object->flags |= 2;
    entry = galloc(sizeof(ObjectList));
    entry->object = object;
    data = galloc(8);
    object->u.initdata = data;
    data = *(UInt32* volatile*)&object->u.initdata;
    low = bits[0];
    high = bits[1];
    data[0] = low;
    data[1] = high;
    if (copts.unknown17) {
        if (TYPE_INTEGRAL(type)->integral == IT_FLOAT ||
            (type->type == TYPEFLOAT && type->size == 4)) {
            regclass = (UInt8)objgen_special_scope_636190;
        } else {
            regclass = (UInt8)objgen_special_scope_63617C;
        }
        object->section = regclass;
    }
    entry->next = objgen_float_constants_636E6C;
    objgen_float_constants_636E6C = entry;
    return object;
scan:
    object = entry->object;
    data = object->u.initdata;
    if (object->type == type && data[0] == bits[0] && data[1] == bits[1]) {
        return object;
    }
    entry = entry->next;
    if (entry) {
        goto scan;
    }
    goto create;
}

SInt32 fn_004AF070(const char* name) {
    UInt8 option = *(volatile UInt8*)&copts.fastmath;
    if (option && !__builtin_memcmp(name, exprprep_sqrt_5E8F4F, 5)) {
        return 16;
    }
    if (option && !__builtin_memcmp(name, exprprep_sqrtf_5E8F41, 6)) {
        return 16;
    }
    if (!__builtin_memcmp(name, exprprep_clear_5E8F47, 8)) {
        return 18;
    }
    if (!copts.inline_intrinsics) {
        return -1;
    }
    if (!__builtin_memcmp(name, exprprep_strcpy_5E8F54, 7)) {
        return 8;
    }
    if (!__builtin_memcmp(name, exprprep_memcpy_5E8F5B, 7)) {
        return 9;
    }
    if (!__builtin_memcmp(name, exprprep_abs_5E8F62, 4)) {
        return 10;
    }
    if (!__builtin_memcmp(name, exprprep_labs_5E8F66, 5)) {
        return 11;
    }
    return -1;
}

#define EXPRPREP_IS_MONADIC(kind) ((kind) <= EFORCELOAD || (UInt32)((kind) - ETYPCON) <= 1)
#define EXPRPREP_IS_DIADIC(kind) \
    ((UInt32)((kind) - EMUL) <= 0x28 || (UInt32)((kind) - EPOINTERSTAR) <= 1)

ENode* fn_004AF7F0(ENode* source) {
    ENode* result;
    ENode* child;
    ENode* cloned;
    UInt32 kind;

    result = lalloc(sizeof(ENode));
    memclrw(result, sizeof(ENode));
    *result = *source;

    kind = source->type;
    if (EXPRPREP_IS_MONADIC(kind)) {
        child = source->data.monadic;
        cloned = fn_004AF7A0(child);
        kind = child->type;
        if (EXPRPREP_IS_MONADIC(kind)) {
            cloned->data.monadic = fn_004AF7F0(child->data.monadic);
        } else if (EXPRPREP_IS_DIADIC(kind)) {
            cloned->data.diadic.left = fn_004AF7F0(child->data.diadic.left);
            cloned->data.diadic.right = fn_004AF7F0(child->data.diadic.right);
        }
        result->data.monadic = cloned;
        return result;
    } else if (EXPRPREP_IS_DIADIC(kind)) {
        child = source->data.diadic.left;
        cloned = fn_004AF7A0(child);
        kind = child->type;
        if (EXPRPREP_IS_MONADIC(kind)) {
            cloned->data.monadic = fn_004AF7F0(child->data.monadic);
        } else if (EXPRPREP_IS_DIADIC(kind)) {
            cloned->data.diadic.left = fn_004AF7F0(child->data.diadic.left);
            cloned->data.diadic.right = fn_004AF7F0(child->data.diadic.right);
        }
        result->data.diadic.left = cloned;
        child = source->data.diadic.right;
        cloned = fn_004AF7A0(child);
        kind = child->type;
        if (EXPRPREP_IS_MONADIC(kind)) {
            cloned->data.monadic = fn_004AF7F0(child->data.monadic);
        } else if (EXPRPREP_IS_DIADIC(kind)) {
            cloned->data.diadic.left = fn_004AF7F0(child->data.diadic.left);
            cloned->data.diadic.right = fn_004AF7F0(child->data.diadic.right);
        }
        result->data.diadic.right = cloned;
    }
    return result;
}

void fn_004AF280(ENode* expr, UInt8 opcode) {
    ENode* left;
    ENode* volatile right;
    ENode* origin;
    ENode* pair;
    ENode* address;
    ENode* clone;
    ENode* cast;
    Type* leftType;
    Type* rightType;
    SInt32 width1;
    SInt32 width2;

    left = expr->data.diadic.left;
    origin = left->data.monadic;
    right = expr->data.diadic.right;
    leftType = left->rtype;
    rightType = expr->data.diadic.right->rtype;
    if (leftType == rightType && right->type == ETYPCON) {
        width1 = exprprep_type_width(rightType);
        width2 = exprprep_type_width(leftType);
        if (width2 < width1) {
            expr->data.diadic.right = right->data.monadic;
            right = expr->data.diadic.right;
        }
    }

    pair = lalloc(sizeof(ENode));
    pair->type = opcode;
    pair->cost = 0;
    if (opcode == ESHL || opcode == ESHR) {
        pair->rtype = leftType;
    } else {
        pair->rtype = rightType;
    }
    pair->data.diadic.left = expr->data.diadic.left;
    pair->data.diadic.right = right;
    expr->type = EASS;
    expr->data.diadic.left = origin;
    expr->data.diadic.right = pair;
    if (origin->type != EOBJREF) {
        address = lalloc(sizeof(ENode));
        address->type = EDEFINE;
        address->cost = 0;
        address->rtype = leftType;
        address->data.diadic.right = NULL;
        right = lalloc(sizeof(ENode));
        right->type = EREUSE;
        right->cost = 0;
        right->rtype = leftType;
        right->data.monadic = address;
        if (origin->type == EBITFIELD) {
            address->data.monadic = origin->data.monadic;
            origin->data.monadic = address;
            clone = lalloc(sizeof(ENode));
            *clone = *origin;
            clone->data.monadic = right;
            pair->data.diadic.left->data.monadic = clone;
        } else {
            address->data.monadic = expr->data.diadic.left;
            expr->data.diadic.left = address;
            pair->data.diadic.left->data.monadic = right;
        }
    }
    if (leftType != rightType && leftType->type != TYPEPOINTER) {
        width1 = exprprep_type_width(leftType);
        width2 = exprprep_type_width(rightType);
        if (width1 < width2) {
            cast = lalloc(sizeof(ENode));
            cast->type = ETYPCON;
            cast->cost = 0;
            cast->rtype = leftType;
            cast->data.monadic = expr->data.diadic.right;
            expr->data.diadic.right = cast;
            cast = lalloc(sizeof(ENode));
            cast->type = ETYPCON;
            cast->cost = 0;
            cast->rtype = rightType;
            cast->data.monadic = pair->data.diadic.left;
            pair->data.diadic.left = cast;
        }
    }
}

void fn_004AF530(ENode* expr, UInt8 opcode) {
    ENode* left = expr->data.diadic.left;
    ENode* right = expr->data.diadic.right;
    Type* leftType = left->rtype;
    Type* rightType = right->rtype;
    ENode* volatile leftChild = left->data.monadic;
    ENode* rightChild = right->data.monadic;
    ENode* volatile node;
    ENode* address;
    ENode* reference;
    ENode* rightAddress;
    ENode* rightReference;
    ENode* clone;
    ENode* leftCursor;
    ENode* rightCursor;
    ENode* scratch;

    expr->type = ECOND;
    node = lalloc(sizeof(ENode));
    memclrw(node, sizeof(ENode));
    node->type = opcode;
    node->rtype = TYPE(&stsignedint);
    node->data.diadic.left = left;
    node->data.diadic.right = right;
    expr->data.cond.cond = node;
    if (leftChild->type == EOBJREF) {
        expr->data.cond.expr1 = fn_004AF7F0(left);
    }
    if (rightChild->type == EOBJREF) {
        expr->data.cond.expr2 = fn_004AF7F0(right);
    }
    if (leftChild->type != EOBJREF) {
        address = lalloc(sizeof(ENode));
        memclrw(address, sizeof(ENode));
        address->type = EDEFINE;
        address->rtype = leftType;
        reference = lalloc(sizeof(ENode));
        memclrw(reference, sizeof(ENode));
        reference->type = EREUSE;
        reference->rtype = leftType;
        reference->data.monadic = address;
        if (leftChild->type == EBITFIELD) {
            expr->data.cond.expr1 = fn_004AF7F0(left);
            address->data.monadic = leftChild->data.monadic;
            leftChild->data.monadic = address;
            scratch = lalloc(sizeof(ENode));
            memclrw(scratch, sizeof(ENode));
            clone = fn_004AF7F0(leftChild);
            clone->data.monadic = reference;
            leftCursor = expr->data.cond.expr1;
            if (leftCursor->type != EBITFIELD) {
                while (leftCursor->data.monadic->type != EBITFIELD) {
                    leftCursor = leftCursor->data.monadic;
                }
            }
            leftCursor->data.monadic = clone;
        } else {
            address->data.monadic = node->data.diadic.left;
            node->data.diadic.left = address;
            expr->data.cond.expr1 = reference;
        }
    }
    if (rightChild->type != EOBJREF) {
        rightAddress = lalloc(sizeof(ENode));
        memclrw(rightAddress, sizeof(ENode));
        rightAddress->type = EDEFINE;
        rightAddress->rtype = rightType;
        rightReference = lalloc(sizeof(ENode));
        memclrw(rightReference, sizeof(ENode));
        rightReference->type = EREUSE;
        rightReference->rtype = rightType;
        rightReference->data.monadic = rightAddress;
        if (rightChild->type == EBITFIELD) {
            expr->data.cond.expr2 = fn_004AF7F0(right);
            rightAddress->data.monadic = rightChild->data.monadic;
            rightChild->data.monadic = rightAddress;
            scratch = lalloc(sizeof(ENode));
            memclrw(scratch, sizeof(ENode));
            clone = fn_004AF7F0(rightChild);
            clone->data.monadic = rightReference;
            rightCursor = expr->data.cond.expr2;
            if (rightCursor->type != EBITFIELD) {
                while (rightCursor->data.monadic->type != EBITFIELD) {
                    rightCursor = leftCursor->data.monadic;
                }
            }
            rightCursor->data.monadic = clone;
        } else {
            rightAddress->data.monadic = node->data.diadic.right;
            node->data.diadic.right = rightAddress;
            expr->data.cond.expr2 = rightReference;
        }
    }
}

#include "compiler/CMangler.h"
#include "compiler/CDecl.h"
