#include "compiler/IroTransform.h"
#include "compiler/CError.h"
#include "compiler/CExpr.h"
#include "compiler/CInt64.h"
#include "compiler/CMachine.h"
#include "compiler/CParser.h"
#include "compiler/CPrep.h"
#include "compiler/CodeGen.h"
#include "compiler/CompilerTools.h"
#include "compiler/enode.h"
#include "compiler/IrOptimizer.h"
#include "compiler/IroDump.h"
#include "compiler/IroLinearForm.h"
#include "compiler/IroLoop.h"
#include "compiler/IroMalloc.h"
#include "compiler/IroPointerAnalysis.h"
#include "compiler/IroType.h"
#include "compiler/IroUtil.h"
#include "compiler/types.h"

static ENode* TemplateExpr;

static UInt8 ExprType;
static ENode* IndirectRef;
static Boolean FirstTime;
static CInt64 OperandConst;
static Object* OperandObject;
static ENodeList* LastAddend;
static ENodeList* FirstAddend;

typedef struct ValueNode {
    CInt64 value;
    struct ValueNode* next;
} ValueNode;

typedef struct ValueSet {
    ENode* expr;
    ENode* var;
    CInt64 min;
    CInt64 max;
    ValueNode* values;
    SInt32 count;
    Boolean wrapped;
} ValueSet;

typedef struct FactorNode {
    IROList list;
    Boolean neg;
} FactorNode;

typedef struct FactorCell {
    FactorNode* node;
    struct FactorCell* next;
} FactorCell;

typedef struct FactorPair {
    IROList list;
    struct FactorPair* other;
    FactorCell* pos;
    FactorCell* neg;
} FactorPair;

typedef struct PairCell {
    void* node;
    struct PairCell* next;
} PairCell;

//! @reconstructed
static inline void AppendList(IROList* list, IROList other) {
    if (other.head && other.tail) {
        if (list->head || list->tail) {
            list->tail->next = other.head;
        } else {
            list->head = other.head;
        }
        list->tail = other.tail;
    }
}

static inline Boolean IsSmallIntType(Type* type) {
    return IS_TYPE_INT(type) && TYPE_INTEGRAL(type)->integral < IT_17;
}

static IROLinear* fn_00580DC0(IROLinear* nd, UInt8 kind, IROList* list);
static void fn_005802C0(IROLinear* nd, Boolean isEntry, void* data);
static void fn_00580350(IROLinear* nd, Boolean isEntry, void* data);
static void fn_00580470(IROLinear* nd, Boolean isEntry, void* data);
static void fn_00580540(IROLinear* nd);

static void DoTransformations(IROLinear* nd) {
    IROLinear* left;
    IROLinear* right;
    IROLinear* conv;
    IROLinear* firstConversion;
    IROLinear* logicalNot;
    IROLinear* secondConversion;
    IROLinear* branch;
    IROLinear* cond;
    IROLinear* less;
    IROLinear* zero;
    IROLinear* sum;
    IROLinear* addend;
    IROLinear* p;
    IROList list;
    IROList sumList;
    IROList lessList;
    IROList elseList;
    CInt64 divisor;
    SInt32 shift;

    if (nd->type == IROLinearOp2Arg) {
        switch (nd->nodetype) {
            case EASS:
                right = nd->u.diadic.right;
                if (right->type == IROLinearOp2Arg &&
                    IRO_AssignmentOp[right->nodetype] < IRO_MAXEXPR &&
                    IRO_TypesEqual(nd->rtype, right->rtype)) {
                    left = nd->u.diadic.left;
                    if (!(IRO_IsVariable(left) && (left->flags & 0x80000)) &&
                        !IRO_HasSideEffect(left) &&
                        IRO_ExprsSame(left, right->u.diadic.left)) {
                        nd->nodetype = IRO_AssignmentOp[right->nodetype];
                        nd->u.diadic.right = right->u.diadic.right;
                        IRO_NopOut(right->u.diadic.left);
                        right->type = IROLinearNop;
                        left->flags |= IROLF_Used;
                        left->u.monadic->flags |= IROLF_Used;
                    } else if (!(IRO_IsVariable(left) && (left->flags & 0x80000)) &&
                               !IRO_HasSideEffect(left) &&
                               IRO_ExprsSame(left, right->u.diadic.right) &&
                               IRO_IsCommutativeENodeTypeArray[right->nodetype]) {
                        nd->nodetype = IRO_AssignmentOp[right->nodetype];
                        nd->u.diadic.right = right->u.diadic.left;
                        IRO_NopOut(right->u.diadic.right);
                        right->type = IROLinearNop;
                        left->flags |= IROLF_Used;
                        left->u.monadic->flags |= IROLF_Used;
                    }
                }
                break;
            case EMUL:
            case EMULASS:
                if (IS_TYPE_INT(nd->rtype) &&
                    TYPE_INTEGRAL(nd->rtype)->integral < 0x11 &&
                    IRO_IsPowerOfTwo(nd->u.diadic.right, &shift) &&
                    shift > 0) {
                    if (nd->nodetype == EMUL) {
                        nd->nodetype = ESHL;
                    } else {
                        nd->nodetype = ESHLASS;
                    }
                    CInt64_SetLong(&nd->u.diadic.right->u.node->data.intval, shift);
                }
                break;
            case EDIV:
            case EDIVASS:
                if (IS_TYPE_INT(nd->rtype) &&
                    TYPE_INTEGRAL(nd->rtype)->integral < 0x11 &&
                    IRO_IsPowerOfTwo(nd->u.diadic.right, &shift) &&
                    shift > 0) {
                    if (is_unsigned(nd->rtype)) {
                        if (nd->nodetype == EDIV) {
                            nd->nodetype = ESHR;
                        } else {
                            nd->nodetype = ESHRASS;
                        }
                        CInt64_SetLong(&nd->u.diadic.right->u.node->data.intval, shift);
                    } else if (!is_unsigned(nd->rtype) &&
                               (conv = nd->u.diadic.left)->nodetype == ETYPCON &&
                               IS_TYPE_INT(conv->u.monadic->rtype) &&
                               TYPE_INTEGRAL(conv->u.monadic->rtype)->integral < 0x11 &&
                               is_unsigned(conv->u.monadic->rtype) &&
                               nd->u.diadic.left->u.monadic->rtype->size <
                                   nd->u.diadic.left->rtype->size) {
                        if (nd->nodetype == EDIV) {
                            nd->nodetype = ESHR;
                        } else {
                            nd->nodetype = ESHRASS;
                        }
                        CInt64_SetLong(&nd->u.diadic.right->u.node->data.intval, shift);
                        if (nd->flags & 2) {
                            conv = IRO_AddTypeConversion(nd, nd->rtype, NULL);
                            conv->u.monadic = NULL;
                            IRO_LocateFather_Cut_And_Paste_Without_Nopping(nd, conv);
                            conv->u.monadic = nd;
                            nd->flags |= 2;
                        }
                        nd->rtype = IRO_UnsignedType(nd->rtype);
                        nd->u.diadic.right->rtype = nd->u.diadic.right->u.node->rtype =
                            IRO_UnsignedType(nd->u.diadic.right->rtype);
                        nd->u.diadic.left->rtype = IRO_UnsignedType(nd->u.diadic.left->rtype);
                    } else if (copts.opt_branchlessexpensivethandiv &&
                               copts.opt_rebuildconditionals) {
                        if (nd->nodetype == EDIV) {
                            nd->nodetype = ESHR;
                        } else {
                            nd->nodetype = ESHRASS;
                        }
                        divisor = nd->u.diadic.right->u.node->data.intval;
                        CInt64_SetLong(&nd->u.diadic.right->u.node->data.intval, shift);
                        IRO_InitList(&list);
                        IRO_InitList(&lessList);
                        IRO_InitList(&sumList);
                        IRO_InitList(&elseList);

                        left = nd->u.diadic.left;
                        cond = IRO_NewLinear(IROLinearOp3Arg);
                        nd->u.diadic.left = cond;
                        cond->rtype = left->rtype;
                        cond->nodetype = ECOND;

                        less = IRO_NewLinear(IROLinearOp2Arg);
                        cond->u.args3.a = less;
                        less->rtype = IRO_LogicalType();
                        less->nodetype = ELESS;
                        less->u.diadic.left = IRO_DuplicateExpr(left, &lessList);
                        less->u.diadic.right = zero = IRO_NewIntConst(cint64_zero, left->rtype);

                        sum = IRO_NewLinear(IROLinearOp2Arg);
                        cond->u.args3.b = sum;
                        sum->nodetype = EADD;
                        sum->rtype = left->rtype;
                        sum->u.diadic.left = IRO_DuplicateExpr(left, &sumList);
                        sum->u.diadic.right = addend =
                            IRO_NewIntConst(CInt64_Sub(divisor, cint64_one), left->rtype);

                        cond->u.args3.c = IRO_DuplicateExpr(left, &elseList);

                        AppendList(&list, lessList);
                        IRO_AddToList(zero, &list);
                        IRO_AddToList(less, &list);
                        AppendList(&list, sumList);
                        IRO_AddToList(addend, &list);
                        IRO_AddToList(sum, &list);
                        AppendList(&list, elseList);
                        IRO_AddToList(cond, &list);
                        for (p = list.head; p; p = p->next) {
                            p->index = ++IRO_NumLinear;
                            if (p == list.tail) {
                                break;
                            }
                        }
                        IRO_Paste(list.head, list.tail, nd);
                    }
                }
                break;
            case EMODULO:
            case EMODASS:
                if (IS_TYPE_INT(nd->rtype) &&
                    TYPE_INTEGRAL(nd->rtype)->integral < 0x11 &&
                    is_unsigned(nd->rtype) &&
                    IRO_IsPowerOfTwo(nd->u.diadic.right, &shift) &&
                    shift > 0) {
                    if (nd->nodetype == EMODULO) {
                        nd->nodetype = EAND;
                    } else {
                        nd->nodetype = EANDASS;
                    }
                    nd->u.diadic.right->u.node->data.intval =
                        CInt64_Sub(nd->u.diadic.right->u.node->data.intval, cint64_one);
                }
                break;
            case EEQU:
                if ((firstConversion = IRO_LocateFather(nd)) &&
                    firstConversion->nodetype == ETYPCON &&
                    IS_TYPE_INT(firstConversion->rtype) &&
                    TYPE_INTEGRAL(firstConversion->rtype)->integral < 0x11 &&
                    (logicalNot = IRO_LocateFather(firstConversion)) &&
                    logicalNot->nodetype == ELOGNOT &&
                    (secondConversion = IRO_LocateFather(logicalNot)) &&
                    secondConversion->nodetype == ETYPCON &&
                    IS_TYPE_INT(secondConversion->rtype) &&
                    TYPE_INTEGRAL(secondConversion->rtype)->integral < 0x11 &&
                    (branch = IRO_LocateFather(secondConversion)) &&
                    (branch->type == IROLinearIf || branch->type == IROLinearIfNot)) {
                    IRO_LocateFather_Cut_And_Paste_Without_Nopping(secondConversion, nd);
                    nd->nodetype = ENOTEQU;
                    firstConversion->type = IROLinearNop;
                    logicalNot->type = IROLinearNop;
                    secondConversion->type = IROLinearNop;
                }
                break;
        }
    }
}

void switchFatherLeft(IROLinear* nd, int isleft) {
    IROLinear* child;
    IROLinear* other;

    child = nd->u.diadic.left;
    IRO_LocateFather_Cut_And_Paste_Without_Nopping(nd, child);
    if (isleft) {
        nd->u.diadic.left = child->u.diadic.left;
        child->u.diadic.left = nd;
        other = child->u.diadic.right;
    } else {
        nd->u.diadic.left = child->u.diadic.right;
        child->u.diadic.right = nd;
        other = child->u.diadic.left;
    }
    child->rtype = IRO_CopyType(nd->rtype);
    child->flags = nd->flags;
    child->nodeflags = nd->nodeflags;
    IRO_CutAndPasteAfter(child, child, nd);
    IRO_CutAndPasteAfter(IRO_FindFirst(other), other, nd);
}

static void ReplaceExprWithConst(IROLinear* node, CInt64 value) {
    IRO_NopOut(node);
    node->type = IROLinearOperand;
    if (IS_TYPE_FLOAT(node->rtype)) {
        node->u.node = IRO_NewENodeFloatConst(
            CMach_CalcFloatConvertFromInt(TYPE(&stsignedlong), value), node->rtype
        );
        node->nodetype = EFLOATCONST;
    } else {
        node->u.node = IRO_NewENodeIntConst(value, node->rtype);
        node->nodetype = EINTCONST;
    }
    node->u.node->flags = node->nodeflags;
}

static void ReplaceExprWithLeftChild(IROLinear* nd) {
    IROLinear* left = nd->u.diadic.left;
    IROLinear* right = nd->u.diadic.right;

    if (IRO_TypesEqual(left->rtype, nd->rtype)) {
        IRO_LocateFather_Cut_And_Paste_Without_Nopping(nd, left);
        left->flags = nd->flags;
        left->nodeflags = nd->nodeflags;
        nd->type = IROLinearNop;
        IRO_NopOut(right);
    } else {
        IROLinear* conv = IRO_AddTypeConversion(left, nd->rtype, 0);

        IRO_LocateFather_Cut_And_Paste_Without_Nopping(nd, conv);
        nd->type = IROLinearNop;
        IRO_NopOut(right);
    }
}

static void ReplaceExprWithRightChild(IROLinear* nd) {
    IROLinear* left = nd->u.diadic.left;
    IROLinear* right = nd->u.diadic.right;

    if (IRO_TypesEqual(right->rtype, nd->rtype)) {
        IRO_LocateFather_Cut_And_Paste_Without_Nopping(nd, right);
        right->flags = nd->flags;
        right->nodeflags = nd->nodeflags;
        nd->type = IROLinearNop;
        IRO_NopOut(left);
    } else {
        IROLinear* conv = IRO_AddTypeConversion(right, nd->rtype, 0);

        IRO_LocateFather_Cut_And_Paste_Without_Nopping(nd, conv);
        nd->type = IROLinearNop;
        IRO_NopOut(left);
    }
}

static void ReplaceExprWithMonaminRight(IROLinear* nd) {
    IROLinear* left = nd->u.diadic.left;
    IROLinear* right = nd->u.diadic.right;
    IROLinear* conv;

    if (IRO_TypesEqual(right->rtype, nd->rtype)) {
        nd->type = IROLinearOp1Arg;
        nd->nodetype = EMONMIN;
        nd->u.monadic = right;
        IRO_NopOut(left);
    } else {
        IRO_NopOut(left);
        conv = IRO_AddTypeConversion(right, nd->rtype, 0);
        nd->type = IROLinearOp1Arg;
        nd->nodetype = EMONMIN;
        nd->u.monadic = conv;
    }
}

static void ReplaceExprWithMonaminLeft(IROLinear* nd) {
    IROLinear* left = nd->u.diadic.left;
    IROLinear* right = nd->u.diadic.right;
    IROLinear* conv;

    if (IRO_TypesEqual(left->rtype, nd->rtype)) {
        nd->type = IROLinearOp1Arg;
        nd->nodetype = EMONMIN;
        nd->u.monadic = left;
        IRO_NopOut(right);
    } else {
        IRO_NopOut(right);
        conv = IRO_AddTypeConversion(left, nd->rtype, 0);
        nd->type = IROLinearOp1Arg;
        nd->nodetype = EMONMIN;
        nd->u.monadic = conv;
    }
}

static inline void switchFatherLeftMonadic(IROLinear* nd) {
    IROLinear* inner = nd->u.monadic;

    IRO_LocateFather_Cut_And_Paste_Without_Nopping(nd, inner);
    nd->u.monadic = inner->u.monadic;
    inner->u.monadic = nd;
    inner->rtype = IRO_CopyType(nd->rtype);
    inner->flags = nd->flags;
    inner->nodeflags = nd->nodeflags;
    IRO_CutAndPasteAfter(inner, inner, nd);
}

static inline void PickCommonsubAtLeftLeft(IROLinear* expr) {
    switchFatherLeft(expr, 0);
    ReplaceExprWithRightChild(expr->u.diadic.right);
}

static inline void PickCommonsubAtRightLeft(IROLinear* expr) {
    switchFatherLeft(expr, 1);
    ReplaceExprWithRightChild(expr->u.diadic.right);
}

static inline void PickCommonsubAtLeftRight(IROLinear* expr) {
    switchFatherLeft(expr, 0);
    ReplaceExprWithLeftChild(expr->u.diadic.right);
}

static inline void PickCommonsubAtRightRight(IROLinear* expr) {
    switchFatherLeft(expr, 1);
    ReplaceExprWithLeftChild(expr->u.diadic.right);
}

static void DoTransformations11(IROLinear* nd) {
    IROLinear* left;
    IROLinear* right;
    int changed;
    int compare;
    Type* type;
    SInt32 resultBits;
    SInt32 operandBits;
    CInt64 val;

    if (nd->type == IROLinearOp2Arg) {
        left = nd->u.diadic.left;
        right = nd->u.diadic.right;
        if (IS_TYPE_FLOAT(nd->rtype) || IS_TYPE_FLOAT(left->rtype) || IS_TYPE_FLOAT(right->rtype)) {
            return;
        }

        changed = FALSE;
        if (!IRO_HasSideEffect(left) && !IRO_HasSideEffect(right)) {
            if (IRO_IsIntConstant(right) || IRO_IsFloatConstant(right)) {
                if (IRO_IsConstantZero(right)) {
                    switch (nd->nodetype) {
                        case EADDV:
                        case ESUBV:
                        case EADD:
                        case ESUB:
                        case ESHL:
                        case ESHR:
                        case EXOR:
                        case EOR:
                        case ELOR:
                        case EADDASS:
                        case ESUBASS:
                        case ESHLASS:
                        case ESHRASS:
                        case EXORASS:
                        case EORASS:
                            ReplaceExprWithLeftChild(nd);
                            changed = TRUE;
                            break;
                        case EMUL:
                        case EAND:
                        case ELAND:
                            ReplaceExprWithConst(nd, cint64_zero);
                            changed = TRUE;
                            break;
                        case EMULASS:
                        case EANDASS:
                            nd->nodetype = EASS;
                            nd->u.diadic.right->rtype = nd->rtype;
                            nd->u.diadic.right->u.node->rtype = nd->rtype;
                            changed = TRUE;
                            break;
                        default:
                            break;
                    }
                } else if (nd->nodetype == ELAND) {
                    ReplaceExprWithLeftChild(nd);
                    changed = TRUE;
                } else if (nd->nodetype == ELOR) {
                    ReplaceExprWithConst(nd, cint64_one);
                    changed = TRUE;
                } else if (nd->nodetype == ESHL ||
                           nd->nodetype == ESHR ||
                           nd->nodetype == ESHLASS ||
                           nd->nodetype == ESHRASS) {
                    type = nd->rtype;
                    resultBits = type->size * 8;
                    if (left->type == IROLinearOp1Arg && left->nodetype == ETYPCON) {
                        type = left->u.monadic->rtype;
                        if (left->u.monadic->type == IROLinearOp1Arg &&
                            left->u.monadic->nodetype == EINDIRECT &&
                            left->u.monadic->u.monadic->type == IROLinearOp1Arg &&
                            left->u.monadic->u.monadic->nodetype == EBITFIELD &&
                            IS_TYPE_BITFIELD(left->u.monadic->u.monadic->rtype)) {
                            operandBits =
                                TYPE_BITFIELD(left->u.monadic->u.monadic->rtype)->bitlength;
                        } else {
                            operandBits = type->size * 8;
                        }
                    } else if (left->type == IROLinearOp1Arg &&
                               left->nodetype == EINDIRECT &&
                               left->u.monadic->type == IROLinearOp1Arg &&
                               left->u.monadic->nodetype == EBITFIELD &&
                               IS_TYPE_BITFIELD(left->u.monadic->rtype)) {
                        operandBits = TYPE_BITFIELD(left->u.monadic->rtype)->bitlength;
                    } else {
                        operandBits = resultBits;
                    }

                    switch (nd->nodetype) {
                        case ESHL:
                        case ESHLASS:
                            CInt64_SetLong(&val, resultBits);
                            if (is_unsigned(type)) {
                                compare = CInt64_GreaterEqualU(right->u.node->data.intval, val);
                            } else {
                                compare = CInt64_GreaterEqual(right->u.node->data.intval, val);
                            }
                            break;
                        case ESHR:
                        case ESHRASS:
                            CInt64_SetLong(&val, operandBits);
                            compare = is_unsigned(type) &&
                                CInt64_GreaterEqualU(right->u.node->data.intval, val);
                            break;
                        default:
                            break;
                    }

                    if (compare) {
                        switch (nd->nodetype) {
                            case ESHL:
                            case ESHR:
                                ReplaceExprWithConst(nd, cint64_zero);
                                break;
                            case ESHLASS:
                            case ESHRASS:
                                nd->nodetype = EASS;
                                ReplaceExprWithConst(right, cint64_zero);
                                break;
                            default:
                                break;
                        }
                        changed = TRUE;
                    }
                } else if (IRO_IsConstantOne(right)) {
                    switch (nd->nodetype) {
                        case EMUL:
                        case EMULV:
                        case EDIV:
                        case EMULASS:
                        case EDIVASS:
                            ReplaceExprWithLeftChild(nd);
                            changed = TRUE;
                            break;
                        case EMODULO:
                            ReplaceExprWithConst(nd, cint64_zero);
                            changed = TRUE;
                            break;
                        case EAND:
                            if (IRO_IsLogicalExpr(left)) {
                                ReplaceExprWithLeftChild(nd);
                                changed = TRUE;
                            }
                            break;
                        case EOR:
                            if (IRO_IsLogicalExpr(left)) {
                                ReplaceExprWithConst(nd, cint64_one);
                                changed = TRUE;
                            }
                            break;
                        case EMODASS:
                            nd->nodetype = EASS;
                            ReplaceExprWithConst(right, cint64_zero);
                            nd->u.diadic.right->rtype = nd->rtype;
                            nd->u.diadic.right->u.node->rtype = nd->rtype;
                            changed = TRUE;
                            break;
                        default:
                            break;
                    }
                } else if (IRO_IsConstantNegativeOne(right)) {
                    switch (nd->nodetype) {
                        case EMUL:
                        case EMULV:
                        case EDIV:
                            ReplaceExprWithMonaminLeft(nd);
                            changed = TRUE;
                            break;
                        case EMODULO:
                            ReplaceExprWithConst(nd, cint64_zero);
                            changed = TRUE;
                            break;
                        case EMODASS:
                            nd->nodetype = EASS;
                            ReplaceExprWithConst(right, cint64_zero);
                            nd->u.diadic.right->rtype = nd->rtype;
                            nd->u.diadic.right->u.node->rtype = nd->rtype;
                            changed = TRUE;
                            break;
                        default:
                            break;
                    }
                }
            }

            if (!changed && IRO_IsIntConstant(right)) {
                CInt64 min;

                min = IRO_MinValueOfIntegerType(right->rtype);
                if (CInt64_Equal(min, right->u.node->data.intval)) {
                    switch (nd->nodetype) {
                        case ELESS:
                            ReplaceExprWithConst(nd, cint64_zero);
                            changed = TRUE;
                            break;
                        case EGREATEREQU:
                            ReplaceExprWithConst(nd, cint64_one);
                            changed = TRUE;
                            break;
                        case EMIN:
                            ReplaceExprWithRightChild(nd);
                            changed = TRUE;
                            break;
                        case EMAX:
                            ReplaceExprWithLeftChild(nd);
                            changed = TRUE;
                            break;
                        default:
                            break;
                    }
                } else {
                    CInt64 max;

                    max = IRO_MaxValueOfIntegerType(right->rtype);
                    if (CInt64_Equal(max, right->u.node->data.intval)) {
                        switch (nd->nodetype) {
                            case EGREATER:
                                ReplaceExprWithConst(nd, cint64_zero);
                                changed = TRUE;
                                break;
                            case ELESSEQU:
                                ReplaceExprWithConst(nd, cint64_one);
                                changed = TRUE;
                                break;
                            case EMIN:
                                ReplaceExprWithLeftChild(nd);
                                changed = TRUE;
                                break;
                            case EMAX:
                                ReplaceExprWithRightChild(nd);
                                changed = TRUE;
                                break;
                            default:
                                break;
                        }
                    }
                }
            }

            if (!changed && (IRO_IsIntConstant(left) || IRO_IsFloatConstant(left))) {
                if (IRO_IsConstantZero(left)) {
                    switch (nd->nodetype) {
                        case EADDV:
                        case EADD:
                        case EXOR:
                        case EOR:
                        case ELOR:
                            ReplaceExprWithRightChild(nd);
                            break;
                        case EMUL:
                        case ESHL:
                        case ESHR:
                        case EAND:
                        case ELAND:
                            ReplaceExprWithConst(nd, cint64_zero);
                            break;
                        case ESUBV:
                        case ESUB:
                            ReplaceExprWithMonaminRight(nd);
                            break;
                        default:
                            break;
                    }
                } else if (nd->nodetype == ELAND) {
                    ReplaceExprWithRightChild(nd);
                } else if (nd->nodetype == ELOR) {
                    ReplaceExprWithConst(nd, cint64_one);
                } else if (IRO_IsConstantOne(left)) {
                    switch (nd->nodetype) {
                        case EMUL:
                        case EMULV:
                            ReplaceExprWithRightChild(nd);
                            break;
                        case EAND:
                            if (IRO_IsLogicalExpr(right)) {
                                ReplaceExprWithRightChild(nd);
                            }
                            break;
                        case EOR:
                            if (IRO_IsLogicalExpr(right)) {
                                ReplaceExprWithConst(nd, cint64_one);
                            }
                            break;
                        default:
                            break;
                    }
                } else if (IRO_IsConstantNegativeOne(left)) {
                    switch (nd->nodetype) {
                        case EMUL:
                        case EMULV:
                            ReplaceExprWithMonaminRight(nd);
                            break;
                        default:
                            break;
                    }
                }
            }

            if (!changed && IRO_IsIntConstant(left)) {
                CInt64 min;

                min = IRO_MinValueOfIntegerType(left->rtype);
                if (CInt64_Equal(min, left->u.node->data.intval)) {
                    switch (nd->nodetype) {
                        case EGREATER:
                            ReplaceExprWithConst(nd, cint64_zero);
                            changed = TRUE;
                            break;
                        case ELESSEQU:
                            ReplaceExprWithConst(nd, cint64_one);
                            changed = TRUE;
                            break;
                        case EMIN:
                            ReplaceExprWithLeftChild(nd);
                            changed = TRUE;
                            break;
                        case EMAX:
                            ReplaceExprWithRightChild(nd);
                            changed = TRUE;
                            break;
                        default:
                            break;
                    }
                } else {
                    CInt64 max;

                    max = IRO_MaxValueOfIntegerType(left->rtype);
                    if (CInt64_Equal(max, left->u.node->data.intval)) {
                        switch (nd->nodetype) {
                            case ELESS:
                                ReplaceExprWithConst(nd, cint64_zero);
                                changed = TRUE;
                                break;
                            case EGREATEREQU:
                                ReplaceExprWithConst(nd, cint64_one);
                                changed = TRUE;
                                break;
                            case EMIN:
                                ReplaceExprWithRightChild(nd);
                                changed = TRUE;
                                break;
                            case EMAX:
                                ReplaceExprWithLeftChild(nd);
                                changed = TRUE;
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
        }
    }
}

static void DoTransformations12(IROLinear* nd) {
    IROLinear* left;
    IROLinear* right;

    if (nd->type == IROLinearOp2Arg) {
        left = nd->u.diadic.left;
        right = nd->u.diadic.right;
        if (IS_TYPE_FLOAT(nd->rtype) || IS_TYPE_FLOAT(left->rtype) || IS_TYPE_FLOAT(right->rtype)) {
            return;
        }

        if (!IRO_HasSideEffect(left) && IRO_ExprsSameSemantically(left, right)) {
            switch (nd->nodetype) {
                case ESUBV:
                case ESUB:
                case ELESS:
                case EGREATER:
                case ENOTEQU:
                case EXOR:
                    ReplaceExprWithConst(nd, cint64_zero);
                    break;
                case ELESSEQU:
                case EGREATEREQU:
                case EEQU:
                    ReplaceExprWithConst(nd, cint64_one);
                    break;
                case EAND:
                case EOR:
                case EASS:
                case ESHRASS:
                case EXORASS:
                case ECOMMA:
                case EMIN:
                case EMAX:
                    ReplaceExprWithLeftChild(nd);
                    break;
                case ELAND:
                case ELOR:
                    nd->u.diadic.left = IRO_LogicalizeExpr(left, TRUE);
                    ReplaceExprWithLeftChild(nd);
                    break;
                case EADDASS:
                case EANDASS:
                    nd->nodetype = EASS;
                    ReplaceExprWithConst(right, cint64_zero);
                    break;
                default:
                    break;
            }
        }
    }
}

static void DoTransformations13(IROLinear* nd) {
    IROLinear* left;
    IROLinear* right;
    IROLinear* left2;
    IROLinear* right2;
    IROListNode* next;
    IROListNode* leftlist;
    IROListNode* rightlist;

    if (nd->type == IROLinearOp2Arg) {
        left = nd->u.diadic.left;
        right = nd->u.diadic.right;

        if (!IRO_HasSideEffect(left) &&
            !IRO_HasSideEffect(right) &&
            (nd->nodetype == EEQU || nd->nodetype == ENOTEQU) &&
            PointerAnalysis_IsLinearNodePointerExprDefinite(FunctionName, left) &&
            PointerAnalysis_IsLinearNodePointerExprDefinite(FunctionName, right)) {
            leftlist = NULL;
            PointerAnalysis_LookupLinearNodePointerExpr(FunctionName, left, &leftlist);
            if (leftlist) {
                if (leftlist->list.head && leftlist->list.tail && !leftlist->nextList) {
                    rightlist = NULL;
                    PointerAnalysis_LookupLinearNodePointerExpr(FunctionName, right, &rightlist);
                    if (rightlist) {
                        if (rightlist->list.head && rightlist->list.tail && !rightlist->nextList) {
                            left2 = leftlist->list.tail;
                            right2 = rightlist->list.tail;
                            if (IRO_ExprsSameSemantically(left2, right2)) {
                                ReplaceExprWithConst(
                                    nd, (nd->nodetype == EEQU) ? cint64_one : cint64_zero
                                );
                            } else if (left2->type == right2->type &&
                                       IRO_TypesEqual(left2->rtype, right2->rtype)) {
                                ReplaceExprWithConst(
                                    nd, (nd->nodetype == EEQU) ? cint64_zero : cint64_one
                                );
                            }
                        }

                        while (rightlist) {
                            next = rightlist->nextList;
                            IRO_gfree(rightlist);
                            rightlist = next;
                        }
                    }
                }

                while (leftlist) {
                    next = leftlist->nextList;
                    IRO_gfree(leftlist);
                    leftlist = next;
                }
            }
        }
    }
}

static void DoTransformations21(IROLinear* nd) {
    IROLinear* left;
    IROLinear* right;
    Boolean changed;

    if (nd->type == IROLinearOp2Arg) {
        left = nd->u.diadic.left;
        right = nd->u.diadic.right;

        if (left->type == IROLinearOp1Arg &&
            right->type == IROLinearOp1Arg &&
            left->nodetype == right->nodetype) {
            switch (left->nodetype) {
                case EMONMIN:
                case EBINNOT:
                case ELOGNOT:
                    changed = FALSE;
                    switch (nd->nodetype) {
                        case EXOR:
                            if (left->nodetype == EBINNOT) {
                                goto collapse;
                            }
                            break;
                        case ELESS:
                        case EGREATER:
                        case ELESSEQU:
                        case EGREATEREQU:
                            if (left->nodetype == EMONMIN) {
                                nd->nodetype = IRO_ComplementaryOp[nd->nodetype];
                                goto collapse;
                            }
                            break;
                        case EMUL:
                        case EDIV:
                            if (left->nodetype == EMONMIN) {
                                goto collapse;
                            }
                            break;
                        case EEQU:
                        case ENOTEQU:
                            if (left->nodetype != ELOGNOT) {
collapse:
                                nd->u.diadic.left = left->u.monadic;
                                nd->u.diadic.right = right->u.monadic;
                                left->type = IROLinearNop;
                                right->type = IROLinearNop;
                                changed = TRUE;
                            }
                            break;
                        case ELAND:
                        case ELOR:
                            if (left->nodetype == ELOGNOT) {
                                nd->nodetype = IRO_ComplementaryOp[nd->nodetype];
                                goto switchfather;
                            }
                            break;
                        case EAND:
                        case EOR:
                            if (!IS_TYPE_FLOAT(nd->rtype) &&
                                !IS_TYPE_FLOAT(left->rtype) &&
                                !IS_TYPE_FLOAT(right->rtype) &&
                                left->nodetype != EMONMIN) {
                                nd->nodetype = IRO_ComplementaryOp[nd->nodetype];
                                goto switchfather;
                            }
                            break;
                        case EADD:
                        case ESUB:
                            if (!IS_TYPE_FLOAT(nd->rtype) &&
                                !IS_TYPE_FLOAT(left->rtype) &&
                                !IS_TYPE_FLOAT(right->rtype) &&
                                left->nodetype == EMONMIN) {
switchfather:
                                switchFatherLeftMonadic(nd);
                                nd->u.diadic.right = right->u.monadic;
                                right->type = IROLinearNop;
                                changed = TRUE;
                            }
                            break;
                        default:
                            break;
                    }

                    if (changed) {
                        DoTransformations(nd);
                        DoTransformations11(nd);
                        DoTransformations12(nd);
                        DoTransformations13(nd);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

static void DoTransformations22(IROLinear* nd) {
    IROLinear* left;
    IROLinear* right;
    IROLinear* ndtmp;

    if (nd->type == IROLinearOp2Arg) {
        left = nd->u.diadic.left;
        right = nd->u.diadic.right;
        if (IS_TYPE_FLOAT(nd->rtype) || IS_TYPE_FLOAT(left->rtype) || IS_TYPE_FLOAT(right->rtype)) {
            return;
        }

        if (!IRO_HasSideEffect(left) && !IRO_HasSideEffect(right)) {
            if (left->type == IROLinearOp1Arg && left->nodetype == EMONMIN) {
                switch (nd->nodetype) {
                    case ESUB:
                    case ESUBV:
                        nd->nodetype = IRO_ComplementaryOp[nd->nodetype];
                        switchFatherLeftMonadic(nd);
                        break;

                    case EADD:
                    case EADDV:
                        nd->nodetype = IRO_ComplementaryOp[nd->nodetype];
                        nd->u.diadic.left = right;
                        nd->u.diadic.right = left->u.monadic;
                        left->type = IROLinearNop;
                        DoTransformations(nd);
                        DoTransformations12(nd);
                        DoTransformations21(nd);
                        break;

                    case EDIV:
                        if (IRO_ExprsSameSemantically(left->u.monadic, right)) {
                            ReplaceExprWithConst(nd, cint64_negone);
                        }
                        break;
                    default:
                        break;
                }
            } else {
                ndtmp = NULL;
                if (left->type == IROLinearOp1Arg &&
                    IRO_ExprsSameSemantically(left->u.monadic, right)) {
                    ndtmp = left;
                } else if (right->type == IROLinearOp1Arg &&
                           IRO_ExprsSameSemantically(left, right->u.monadic)) {
                    ndtmp = right;
                }

                if (ndtmp) {
                    switch (nd->nodetype) {
                        case EAND:
                            if (ndtmp->nodetype == EBINNOT || ndtmp->nodetype == ELOGNOT) {
                                ReplaceExprWithConst(nd, cint64_zero);
                            }
                            break;

                        case EANDASS:
                            if (ndtmp->nodetype == EBINNOT || ndtmp->nodetype == ELOGNOT) {
                                nd->nodetype = EASS;
                                ReplaceExprWithConst(right, cint64_zero);
                                nd->u.diadic.right->rtype = IRO_CopyType(nd->rtype);
                                nd->u.diadic.right->u.node->rtype = IRO_CopyType(nd->rtype);
                            }
                            break;

                        case EXOR:
                        case EOR:
                            if (ndtmp->nodetype == EBINNOT) {
                                ReplaceExprWithConst(nd, cint64_zero);
                                nd->u.node->data.intval = CInt64_Inv(nd->u.node->data.intval);
                            }
                            break;

                        case EXORASS:
                        case EORASS:
                            if (ndtmp->nodetype == EBINNOT) {
                                nd->nodetype = EASS;
                                ReplaceExprWithConst(right, cint64_zero);
                                right->u.node->data.intval = CInt64_Inv(right->u.node->data.intval);
                                nd->u.diadic.right->rtype = IRO_CopyType(nd->rtype);
                                nd->u.diadic.right->u.node->rtype = IRO_CopyType(nd->rtype);
                            }
                            break;

                        case ELOR:
                            if (ndtmp->nodetype == EBINNOT || ndtmp->nodetype == ELOGNOT) {
                                ReplaceExprWithConst(nd, cint64_one);
                            }
                            break;

                        case ELAND:
                            if (ndtmp->nodetype == ELOGNOT) {
                                ReplaceExprWithConst(nd, cint64_zero);
                            }
                            break;

                        case EDIV:
                            if (ndtmp->nodetype == EMONMIN) {
                                ReplaceExprWithConst(nd, cint64_negone);
                            }
                            break;

                        case EDIVASS:
                            if (ndtmp->nodetype == EMONMIN) {
                                nd->nodetype = EASS;
                                ReplaceExprWithConst(right, cint64_negone);
                                nd->u.diadic.right->rtype = nd->rtype;
                                nd->u.diadic.right->u.node->rtype = nd->rtype;
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
}

static void SwitchChildren(IROLinear* nd) {
    IROLinear* left = nd->u.diadic.left;
    IROLinear* right = nd->u.diadic.right;

    nd->u.diadic.left = right;
    nd->u.diadic.right = left;
}

static void DoTransformations23(IROLinear* nd);
static void DoTransformations24(IROLinear* nd);
static void DoTransformations25(IROLinear* nd);
static void RemoveRedundantMonadicOp(IROLinear* nd);
static inline void RemoveUnreffed(IROLinear* nd);
static inline void DoDiadic(IROLinear* nd);

static void DoTransformations23(IROLinear* nd) {
    Boolean changed;
    Boolean isCompare;
    UInt8 which;
    IROLinear* left;
    IROLinear* right;
    CInt64 size;
    CInt64 val;

    if (nd->type == IROLinearOp2Arg && !IRO_HasSideEffect(nd)) {
        left = nd->u.diadic.left;
        right = nd->u.diadic.right;
        if (IS_TYPE_FLOAT(nd->rtype) || IS_TYPE_FLOAT(left->rtype) || IS_TYPE_FLOAT(right->rtype)) {
            return;
        }

        changed = isCompare = FALSE;
        which = 0;

        do {
            switch (nd->nodetype) {
                case ELESS:
                case EGREATER:
                case ELESSEQU:
                case EGREATEREQU:
                    isCompare = TRUE;
                case EADDV:
                case EADD:
                case EEQU:
                case ENOTEQU:
                case EAND:
                case EOR:
                    if (left->type == IROLinearOp2Arg) {
                        if (IRO_ExprsSameSemantically(right, left->u.diadic.left)) {
                            which = 1;
                        } else if (IRO_ExprsSameSemantically(right, left->u.diadic.right)) {
                            which = 2;
                        }

                        if (which) {
                            if (isCompare) {
                                nd->nodetype = IRO_ComplementaryOp[nd->nodetype];
                            }
                            SwitchChildren(nd);
                            left = nd->u.diadic.left;
                            right = nd->u.diadic.right;
                            break;
                        }
                    }
                case ESUBV:
                case ESUB:
                case EADDASS:
                case ESUBASS:
                case EANDASS:
                case EORASS:
                    if (right->type == IROLinearOp2Arg) {
                        if (IRO_ExprsSameSemantically(left, right->u.diadic.left)) {
                            which = 1;
                        } else if (IRO_ExprsSameSemantically(left, right->u.diadic.right)) {
                            which = 2;
                        }
                    }
                    break;

                default:
                    continue;
            }

            if (which) {
                switch (right->nodetype) {
                    case EAND:
                    case EOR:
                        if (which == 2) {
                            SwitchChildren(right);
                        }

                        if (nd->nodetype == right->nodetype ||
                            (nd->nodetype == EANDASS && right->nodetype == EAND) ||
                            (nd->nodetype == EORASS && right->nodetype == EOR)) {
                            ReplaceExprWithRightChild(right);
                            changed = TRUE;
                        } else if (nd->nodetype == IRO_ComplementaryOp[right->nodetype] ||
                                   (nd->nodetype == EANDASS && right->nodetype == EOR) ||
                                   (nd->nodetype == EORASS && right->nodetype == EAND)) {
                            ReplaceExprWithLeftChild(nd);
                        }
                        break;

                    case EADD:
                        if (which == 2) {
                            SwitchChildren(right);
                        }

                        switch (nd->nodetype) {
                            case EEQU:
                            case ENOTEQU:
                                ReplaceExprWithConst(left, cint64_zero);
                                ReplaceExprWithRightChild(right);
                                SwitchChildren(nd);
                                if (isCompare) {
                                    nd->nodetype = IRO_ComplementaryOp[nd->nodetype];
                                }
                                changed = TRUE;
                                break;

                            case ESUB:
                            case ESUBV:
                                ReplaceExprWithRightChild(right);
                                ReplaceExprWithMonaminRight(nd);
                                changed = TRUE;
                                break;

                            case ESUBASS:
                                ReplaceExprWithRightChild(right);
                                changed = TRUE;
                                break;
                            default:
                                break;
                        }
                        break;

                    case ESUB:
                        switch (nd->nodetype) {
                            case EEQU:
                            case ENOTEQU:
                                if (which == 1) {
                                    ReplaceExprWithConst(left, cint64_zero);
                                    ReplaceExprWithRightChild(right);
                                    SwitchChildren(nd);
                                }
                                break;
                            case EADD:
                            case EADDV:
                                if (which == 2) {
                                    ReplaceExprWithLeftChild(right);
                                    ReplaceExprWithRightChild(nd);
                                }
                                break;
                            case ESUB:
                            case ESUBV:
                                if (which == 1) {
                                    ReplaceExprWithRightChild(right);
                                    ReplaceExprWithRightChild(nd);
                                }
                                break;
                            case EADDASS:
                                if (which == 2) {
                                    nd->nodetype = EASS;
                                    ReplaceExprWithLeftChild(right);
                                    changed = TRUE;
                                }
                                break;
                            case ESUBASS:
                                if (which == 1) {
                                    nd->nodetype = EASS;
                                    ReplaceExprWithRightChild(right);
                                    changed = TRUE;
                                }
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
            }
        } while (0);

        if (!changed) {
            switch (nd->nodetype) {
                case ESUBV:
                case EADD:
                case ESUB:
                    which = 0;
                    if (left->type == IROLinearOp2Arg) {
                        if (IRO_ExprsSameSemantically(right, left->u.diadic.left)) {
                            which = 1;
                        } else if (IRO_ExprsSameSemantically(right, left->u.diadic.right)) {
                            which = 2;
                        }
                    }

                    if (which == 1) {
                        if (left->nodetype == ESUB) {
                            ReplaceExprWithMonaminRight(left);
                            ReplaceExprWithLeftChild(nd);
                        } else if (left->nodetype == EADD) {
                            ReplaceExprWithRightChild(left);
                            ReplaceExprWithLeftChild(nd);
                        }
                    } else if (which == 2) {
                        if (left->nodetype == EADD) {
                            ReplaceExprWithLeftChild(left);
                            ReplaceExprWithLeftChild(nd);
                        }
                    }
                    break;
                default:
                    break;
            }

            while (left->type == IROLinearOp1Arg &&
                   left->nodetype == ETYPCON &&
                   IRO_IsValuePreservingTypcon(left)) {
                left = left->u.monadic;
            }

            switch (nd->nodetype) {
                case ESHL:
                case ESHR:
                case ESHLASS:
                case ESHRASS:
                    which = 0;
                    if (left->type == IROLinearOp2Arg) {
                        if (left->nodetype == IRO_ComplementaryOp[nd->nodetype] ||
                            left->nodetype == IRO_AssignmentOp[IRO_ComplementaryOp[nd->nodetype]]) {
                            if (IRO_IsIntConstant(right) &&
                                IRO_IsIntConstant(left->u.diadic.right) &&
                                (right->nodeflags & ~0x200) ==
                                    (left->u.diadic.right->nodeflags & ~0x200) &&
                                CInt64_Equal(
                                    right->u.node->data.intval,
                                    left->u.diadic.right->u.node->data.intval
                                )) {
                                which = 2;
                            }
                        }
                    }

                    if (which == 2) {
                        val = right->u.node->data.intval;
                        if (left->nodetype == ESHR || left->nodetype == ESHRASS) {
                            ReplaceExprWithLeftChild(left);
                            nd->nodetype = (nd->nodetype == ESHL) ? EAND : EANDASS;
                            right->u.node->data.intval = CInt64_Shl(cint64_negone, val);
                            if (!IRO_TypesEqual(nd->rtype, right->rtype)) {
                                nd->u.diadic.right = IRO_AddTypeConversion(right, nd->rtype, 0);
                            }
                            changed = TRUE;
                        } else if (is_unsigned(nd->rtype)) {
                            ReplaceExprWithLeftChild(left);
                            nd->nodetype = (nd->nodetype == ESHR) ? EAND : EANDASS;
                            CError_ASSERT(2341, 8 * nd->rtype->size <= 128);
                            CInt64_SetLong(&size, 128 - 8 * nd->rtype->size);
                            val = CInt64_Add(val, size);
                            right->u.node->data.intval = CInt64_ShrU(cint64_negone, val);
                            if (!IRO_TypesEqual(nd->rtype, right->rtype)) {
                                nd->u.diadic.right = IRO_AddTypeConversion(right, nd->rtype, 0);
                            }
                            changed = TRUE;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        if (changed) {
            DoTransformations(nd);
            DoTransformations11(nd);
            DoTransformations12(nd);
            DoTransformations13(nd);
            DoTransformations21(nd);
            DoTransformations22(nd);
            DoTransformations23(nd);
        }
    }
}

static inline Boolean isOrderingOperator(ENodeType op) {
    Boolean result;

    switch (op) {
        case ELAND:
        case ELOR:
        case ECOMMA:
        case ECOND:
        case ECONDASS:
            result = TRUE;
            break;
        default:
            result = FALSE;
            break;
    }

    return result;
}

static inline void RemoveUnreffed(IROLinear* nd) {
    if (!(nd->flags & IROLF_Reffed)) {
        switch (nd->type) {
            case IROLinearOperand:
            case IROLinearOp1Arg:
            case IROLinearOp2Arg:
            case IROLinearOp3Arg:
            case IROLinear5:
            case IROLinearFunccall:
                if (!isOrderingOperator(nd->nodetype)) {
                    IRO_NopNonSideEffects(nd, 0);
                }
                break;
            default:
                break;
        }
    }
}

static inline void DoDiadic(IROLinear* nd) {
    RemoveUnreffed(nd);
    DoTransformations(nd);
    DoTransformations11(nd);
    DoTransformations12(nd);
    DoTransformations13(nd);
    DoTransformations21(nd);
    DoTransformations22(nd);
    DoTransformations23(nd);
    DoTransformations24(nd);
    DoTransformations25(nd);
}

static inline void ReverseOpForMonmin(IROLinear* nd) {
    IROLinear* father;

    if (nd->type == IROLinearOp1Arg &&
        nd->nodetype == EMONMIN &&
        (father = IRO_LocateFather(nd)) &&
        father->type == IROLinearOp2Arg &&
        father->u.diadic.right == nd) {
        switch (father->nodetype) {
            case EADDV:
            case ESUBV:
            case EADD:
            case ESUB:
            case EADDASS:
            case ESUBASS:
                father->nodetype = IRO_ComplementaryOp[father->nodetype];
                nd->type = IROLinearNop;
                father->u.diadic.right = nd->u.monadic;
                break;
            default:
                break;
        }
    }
}

static void DoTransformations24(IROLinear* expr) {
    IROLinear* nd = expr;
    IROLinear* left;
    IROLinear* right;
    UInt8 changed;

    if (nd->type == IROLinearOp2Arg) {
        left = nd->u.diadic.left;
        right = nd->u.diadic.right;
        if (IS_TYPE_FLOAT(nd->rtype) || IS_TYPE_FLOAT(left->rtype) || IS_TYPE_FLOAT(right->rtype)) {
            return;
        }

        if (left->type == IROLinearOp2Arg &&
            right->type == IROLinearOp2Arg &&
            !IRO_HasSideEffect(nd)) {
            changed = 0;

            if (IRO_ExprsSameSemantically(left->u.diadic.left, right->u.diadic.left)) {
                if (left->nodetype == right->nodetype) {
                    switch (left->nodetype) {
                        case EADD:
                            if (nd->nodetype == IRO_ComplementaryOp[left->nodetype]) {
                                ReplaceExprWithRightChild(left);
                                ReplaceExprWithRightChild(right);
                                changed = 1;
                            }
                            break;
                        case ESUB:
                            if (nd->nodetype == ESUB || nd->nodetype == ESUBV) {
                                ReplaceExprWithRightChild(left);
                                ReplaceExprWithRightChild(right);
                                SwitchChildren(nd);
                                changed = 1;
                            }
                            break;
                        case EMUL:
                            break;
                        case EAND:
                            if (nd->nodetype == EXOR) {
                                PickCommonsubAtLeftLeft(nd);
                                changed = 3;
                                break;
                            }
                        case EOR:
                            if (nd->nodetype == left->nodetype) {
                                ReplaceExprWithRightChild(left);
                                changed = 1;
                            } else if (nd->nodetype == IRO_ComplementaryOp[left->nodetype]) {
                                PickCommonsubAtLeftLeft(nd);
                                changed = 3;
                            }
                            break;
                        default:
                            break;
                    }
                } else if (left->nodetype == IRO_ComplementaryOp[right->nodetype]) {
                    switch (nd->nodetype) {
                        case ESUBV:
                        case ESUB:
                            switch (left->nodetype) {
                                case EADD:
                                    nd->nodetype = IRO_ComplementaryOp[nd->nodetype];
                                    ReplaceExprWithRightChild(left);
                                    ReplaceExprWithRightChild(right);
                                    changed = 1;
                                    break;
                                case ESUB:
                                    ReplaceExprWithMonaminRight(left);
                                    ReplaceExprWithRightChild(right);
                                    changed = 1;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case EAND:
                        case EOR:
                            if (left->nodetype == nd->nodetype) {
                                ReplaceExprWithLeftChild(nd);
                            } else if (right->nodetype == nd->nodetype) {
                                ReplaceExprWithRightChild(nd);
                            }
                            break;
                        default:
                            break;
                    }
                }
            } else if (IRO_ExprsSameSemantically(left->u.diadic.right, right->u.diadic.left)) {
                if (left->nodetype == right->nodetype) {
                    switch (left->nodetype) {
                        case EADD:
                            if (nd->nodetype == IRO_ComplementaryOp[left->nodetype]) {
                                ReplaceExprWithLeftChild(left);
                                ReplaceExprWithRightChild(right);
                                changed = 1;
                            }
                            break;
                        case EMUL:
                            break;
                        case EAND:
                            if (nd->nodetype == EXOR) {
                                PickCommonsubAtRightLeft(nd);
                                changed = 3;
                                break;
                            }
                        case EOR:
                            if (nd->nodetype == left->nodetype) {
                                ReplaceExprWithLeftChild(left);
                                changed = 1;
                            } else if (nd->nodetype == IRO_ComplementaryOp[left->nodetype]) {
                                PickCommonsubAtRightLeft(nd);
                                changed = 3;
                            }
                            break;
                        default:
                            break;
                    }
                } else if (left->nodetype == IRO_ComplementaryOp[right->nodetype]) {
                    switch (nd->nodetype) {
                        case ESUBV:
                        case ESUB:
                            if (left->nodetype == EADD) {
                                nd->nodetype = IRO_ComplementaryOp[nd->nodetype];
                                ReplaceExprWithLeftChild(left);
                                ReplaceExprWithRightChild(right);
                                changed = 1;
                            }
                            break;
                        case EADDV:
                        case EADD:
                            if (left->nodetype == ESUB) {
                                ReplaceExprWithLeftChild(left);
                                ReplaceExprWithRightChild(right);
                                changed = 1;
                            }
                            break;
                        case EAND:
                        case EOR:
                            if (left->nodetype == nd->nodetype) {
                                ReplaceExprWithLeftChild(nd);
                            } else if (right->nodetype == nd->nodetype) {
                                ReplaceExprWithRightChild(nd);
                            }
                            break;
                        default:
                            break;
                    }
                }
            } else if (IRO_ExprsSameSemantically(left->u.diadic.left, right->u.diadic.right)) {
                if (left->nodetype == right->nodetype) {
                    switch (left->nodetype) {
                        case EADD:
                            if (nd->nodetype == IRO_ComplementaryOp[left->nodetype]) {
                                ReplaceExprWithRightChild(left);
                                ReplaceExprWithLeftChild(right);
                                changed = 1;
                            }
                            break;
                        case ESUB:
                            switch (nd->nodetype) {
                                case EADDV:
                                case EADD:
                                    nd->nodetype = IRO_ComplementaryOp[nd->nodetype];
                                    ReplaceExprWithRightChild(left);
                                    ReplaceExprWithLeftChild(right);
                                    SwitchChildren(nd);
                                    changed = 1;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case EMUL:
                            break;
                        case EAND:
                            if (nd->nodetype == EXOR) {
                                PickCommonsubAtLeftRight(nd);
                                changed = 2;
                                break;
                            }
                        case EOR:
                            if (nd->nodetype == left->nodetype) {
                                ReplaceExprWithRightChild(left);
                                changed = 1;
                            } else if (nd->nodetype == IRO_ComplementaryOp[left->nodetype]) {
                                PickCommonsubAtLeftRight(nd);
                                changed = 2;
                            }
                            break;
                        default:
                            break;
                    }
                } else if (left->nodetype == IRO_ComplementaryOp[right->nodetype]) {
                    switch (nd->nodetype) {
                        case EADDV:
                        case EADD:
                            if (left->nodetype == EADD) {
                                ReplaceExprWithRightChild(left);
                                ReplaceExprWithLeftChild(right);
                                changed = 1;
                            }
                            break;
                        case ESUBV:
                        case ESUB:
                            if (left->nodetype == ESUB) {
                                ReplaceExprWithMonaminRight(left);
                                ReplaceExprWithRightChild(right);
                                changed = 1;
                            }
                            break;
                        case EAND:
                        case EOR:
                            if (left->nodetype == nd->nodetype) {
                                ReplaceExprWithLeftChild(nd);
                            } else if (right->nodetype == nd->nodetype) {
                                ReplaceExprWithRightChild(nd);
                            }
                            break;
                        default:
                            break;
                    }
                }
            } else if (IRO_ExprsSameSemantically(left->u.diadic.right, right->u.diadic.right)) {
                if (left->nodetype == right->nodetype) {
                    switch (nd->nodetype) {
                        case ESUB:
                            switch (left->nodetype) {
                                case EADD:
                                case ESUB:
                                    ReplaceExprWithLeftChild(left);
                                    ReplaceExprWithLeftChild(right);
                                    changed = 1;
                                default:
                                    break;
                            }
                        case EADD:
                            switch (left->nodetype) {
                                case ESHL:
                                    PickCommonsubAtRightRight(nd);
                                    changed = 2;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case EXOR:
                            switch (left->nodetype) {
                                case ESHL:
                                case ESHR:
                                case EAND:
                                    PickCommonsubAtRightRight(nd);
                                    changed = 2;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case EAND:
                        case EOR:
                            if (left->nodetype == nd->nodetype) {
                                ReplaceExprWithLeftChild(right);
                                changed = 1;
                            } else if (left->nodetype == IRO_ComplementaryOp[nd->nodetype] ||
                                       left->nodetype == ESHR ||
                                       left->nodetype == ESHL) {
                                PickCommonsubAtRightRight(nd);
                                changed = 2;
                            }
                            break;
                        default:
                            break;
                    }
                } else if (left->nodetype == IRO_ComplementaryOp[right->nodetype]) {
                    switch (nd->nodetype) {
                        case EADDV:
                        case EADD:
                            switch (left->nodetype) {
                                case EADD:
                                case ESUB:
                                    ReplaceExprWithLeftChild(left);
                                    ReplaceExprWithLeftChild(right);
                                    changed = 1;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case EAND:
                        case EOR:
                            if (left->nodetype == nd->nodetype) {
                                ReplaceExprWithLeftChild(nd);
                            } else if (right->nodetype == nd->nodetype) {
                                ReplaceExprWithRightChild(nd);
                            }
                            break;
                        default:
                            break;
                    }
                }
            }

            if (changed) {
                DoDiadic(nd);
                if (changed == 2) {
                    DoDiadic(nd->u.diadic.left);
                } else if (changed == 3) {
                    DoDiadic(nd->u.diadic.right);
                }
            }
        }
    }
}

static void DoTransformations25(IROLinear* nd) {
    int changed = FALSE;
    IROLinear* left;
    IROLinear* right;

    if (nd->type == IROLinearOp2Arg) {
        left = nd->u.diadic.left;
        right = nd->u.diadic.right;

        if ((left->type == IROLinearOp2Arg &&
             (IS_TYPE_FLOAT(left->u.diadic.left->rtype) ||
              IS_TYPE_FLOAT(left->u.diadic.right->rtype))) ||
            (right->type == IROLinearOp2Arg &&
             (IS_TYPE_FLOAT(right->u.diadic.left->rtype) ||
              IS_TYPE_FLOAT(right->u.diadic.right->rtype)))) {
            return;
        }

        switch (left->nodetype) {
            case ELESS:
            case EGREATER:
            case ELESSEQU:
            case EGREATEREQU:
            case EEQU:
            case ENOTEQU:
                switch (nd->nodetype) {
                    case EEQU:
                        if (IRO_IsConstantOne(right)) {
                            ReplaceExprWithLeftChild(nd);
                            changed = TRUE;
                        } else if (IRO_IsConstantZero(right)) {
                            left->nodetype = IRO_ComplementaryLogicalComparison[left->nodetype];
                            ReplaceExprWithLeftChild(nd);
                            changed = TRUE;
                        }
                        break;
                    case ENOTEQU:
                        if (IRO_IsConstantOne(right)) {
                            left->nodetype = IRO_ComplementaryLogicalComparison[left->nodetype];
                            ReplaceExprWithLeftChild(nd);
                            changed = TRUE;
                        } else if (IRO_IsConstantZero(right)) {
                            ReplaceExprWithLeftChild(nd);
                            changed = TRUE;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case ELAND:
            case ELOR:
                switch (nd->nodetype) {
                    case EEQU:
                        if (IRO_IsConstantOne(right)) {
                            ReplaceExprWithLeftChild(nd);
                            changed = TRUE;
                        }
                        break;
                    case ENOTEQU:
                        if (IRO_IsConstantZero(right)) {
                            ReplaceExprWithLeftChild(nd);
                            changed = TRUE;
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        if (!changed) {
            switch (right->nodetype) {
                case ELESS:
                case EGREATER:
                case ELESSEQU:
                case EGREATEREQU:
                case EEQU:
                case ENOTEQU:
                    switch (nd->nodetype) {
                        case EEQU:
                            if (IRO_IsConstantOne(left)) {
                                ReplaceExprWithRightChild(nd);
                            } else if (IRO_IsConstantZero(left)) {
                                right->nodetype =
                                    IRO_ComplementaryLogicalComparison[right->nodetype];
                                ReplaceExprWithRightChild(nd);
                            }
                            break;
                        case ENOTEQU:
                            if (IRO_IsConstantOne(left)) {
                                right->nodetype =
                                    IRO_ComplementaryLogicalComparison[right->nodetype];
                                ReplaceExprWithRightChild(nd);
                            } else if (IRO_IsConstantZero(left)) {
                                ReplaceExprWithRightChild(nd);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case ELAND:
                case ELOR:
                    switch (nd->nodetype) {
                        case EEQU:
                            if (IRO_IsConstantOne(left)) {
                                ReplaceExprWithRightChild(nd);
                            }
                            break;
                        case ENOTEQU:
                            if (IRO_IsConstantZero(left)) {
                                ReplaceExprWithRightChild(nd);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

//! @reconstructed
static void DoTransformations26(IROLinear* nd) {
    IROLinear* a = nd->u.args3.a;
    IROLinear* b = nd->u.args3.b;
    IROLinear* c = nd->u.args3.c;

    IRO_CopyType(nd->rtype);

    if (nd &&
        nd->type == IROLinearOp3Arg &&
        nd->nodetype == ECOND &&
        IRO_IsIntConstant(b) &&
        IRO_IsIntConstant(c) &&
        ((IRO_IsConstantOne(b) && IRO_IsConstantZero(c)) ||
         (IRO_IsConstantZero(b) && IRO_IsConstantOne(c)) ||
         (IRO_IsConstantNegativeOne(b) && IRO_IsConstantZero(c)) ||
         (IRO_IsConstantZero(b) && IRO_IsConstantNegativeOne(c)))) {
        if (IRO_IsConstantZero(b)) {
            a = IRO_LogicalNotExpr(a, TRUE);
        }
        a = IRO_LogicalizeExpr(a, TRUE);
        if (!IRO_TypesEqual(a->rtype, nd->rtype)) {
            a = IRO_AddTypeConversion(a, nd->rtype, 0);
        }
        if (IRO_IsConstantNegativeOne(b) || IRO_IsConstantNegativeOne(c)) {
            IROLinear* mon = IRO_NewLinear(IROLinearOp1Arg);

            *mon = *b;
            mon->type = IROLinearOp1Arg;
            mon->nodetype = EMONMIN;
            mon->index = IRO_NumLinear++;
            mon->u.monadic = a;
            IRO_PasteAfter(mon, mon, a);
            a = mon;
        }
        b->type = IROLinearNop;
        b->unk1C = 0;
        c->type = IROLinearNop;
        c->unk1C = 0;
        nd->type = IROLinearNop;
        nd->unk1C = 0;
        IRO_LocateFather_Cut_And_Paste_Without_Nopping(nd, a);
    }
}

//! @reconstructed
static void WarnForDivisionByZero(IROLinear* nd) {
    IROLinear* left;
    IROLinear* right;
    TStreamElement* token;

    left = nd->u.diadic.left;
    right = nd->u.diadic.right;
    if (gWarnForFloatDivByZero &&
        (IS_TYPE_FLOAT(nd->rtype) || IS_TYPE_FLOAT(left->rtype) || IS_TYPE_FLOAT(right->rtype))) {
        return;
    }
    if ((IRO_IsIntConstant(right) || IRO_IsFloatConstant(right)) && IRO_IsConstantZero(right)) {
        switch (nd->nodetype) {
            case EDIV:
            case EMODULO:
            case EDIVASS:
            case EMODASS:
                if (nd->stmt && nd->stmt->sourceoffset) {
                    token = CPrep_CurStreamElement();
                    token->tokenoffset = nd->stmt->sourceoffset;
                    CError_SetErrorToken(token);
                }
                CError_Warning(ERR_DIVISION_BY_0);
                break;
        }
    }
}

void IRO_WarnForDivisionsByZero(IROLinear* first) {
    IROLinear* nd;

    if (gWarnForDivByZero) {
        for (nd = first; nd; nd = nd->next) {
            if (nd->type == IROLinearOp2Arg) {
                WarnForDivisionByZero(nd);
            }
        }
        IRO_CheckForUserBreak();
    }
}

static void AddAddend(ENode* expr) {
    ENodeList* list;

    if (expr->type == EADD) {
        AddAddend(expr->data.diadic.left);
        AddAddend(expr->data.diadic.right);
    } else {
        list = IRO_oalloc(sizeof(ENodeList));
        list->node = expr;
        list->next = NULL;

        if (FirstAddend) {
            LastAddend->next = list;
        } else {
            FirstAddend = list;
        }
        LastAddend = list;
    }
}

#pragma opt_common_subs off
//! @reconstructed
static Boolean IsExprVolatile(ENode* expr) {
    if (expr->type == EADD) {
        if (expr->rtype && CParser_IsVolatile(expr->rtype, expr->flags & 3) ||
            IsExprVolatile(expr->data.diadic.left) ||
            IsExprVolatile(expr->data.diadic.right)) {
            return TRUE;
        }
    }
    return FALSE;
}
#pragma opt_common_subs on

static ENode* CombineConstants(ENode* expr) {
    ENode* addend;
    ENodeList* el;
    ENode* result;
    ENode* var;
    Type* type;
    ENode* tmp;
    ENodeList* prev;
    ENodeList* next;
    CInt64 val;

    if (IsExprVolatile(expr)) {
        return expr;
    }

    FirstAddend = LastAddend = NULL;
    AddAddend(expr->data.diadic.left);
    AddAddend(expr->data.diadic.right);

    el = FirstAddend;
    prev = NULL;
    var = NULL;
    while (el) {
        addend = el->node;
        if (addend->type == EOBJREF) {
            var = addend;
            if (prev) {
                prev->next = el->next;
            } else {
                FirstAddend = el->next;
            }
            break;
        }
        prev = el;
        el = el->next;
    }

    if (!var) {
        el = FirstAddend;
        prev = NULL;
        while (el) {
            addend = el->node;
            if (addend->type == EINDIRECT) {
                var = addend;
                if (prev) {
                    prev->next = el->next;
                } else {
                    FirstAddend = el->next;
                }
                break;
            }
            prev = el;
            el = el->next;
        }
    }

    prev = NULL;
    CInt64_SetLong(&val, 0);
    type = NULL;

    for (el = FirstAddend; el; el = el->next) {
        addend = el->node;
        if (IRO_IsENodeIntConstant(addend)) {
            if (!type || type->size < addend->rtype->size) {
                type = addend->rtype;
            }
            val = CInt64_Add(val, addend->data.intval);
            if (prev) {
                prev->next = el->next;
            } else {
                FirstAddend = el->next;
            }
        } else if (addend->type == EMUL && IRO_IsENodeIntConstant(addend->data.diadic.right)) {
            if (!type || type->size < addend->rtype->size) {
                type = addend->rtype;
            }

            tmp = addend->data.diadic.left;
            if (tmp->type == EADD && IRO_IsENodeIntConstant(tmp->data.diadic.right)) {
                val = CInt64_Add(
                    val,
                    CInt64_MulU(
                        tmp->data.diadic.right->data.intval, addend->data.diadic.right->data.intval
                    )
                );
                addend->data.diadic.left = tmp->data.diadic.left;
            }
            prev = el;
        } else {
            prev = el;
        }
    }

    result = NULL;
    if (var) {
        result = var;
        if (!CInt64_IsZero(&val)) {
            result = IRO_NewENode(EADD);
            result->data.diadic.left = var;
            result->data.diadic.right = IRO_NewENodeIntConst(val, type);
            if (var->rtype->type == TYPEPOINTER) {
                result->rtype = var->rtype;
            } else {
                result->rtype = type;
            }
            result->cost = 1;
            CInt64_SetLong(&val, 0);
        }
    }

    for (el = FirstAddend; el; el = el->next) {
        addend = el->node;
        if (result) {
            tmp = IRO_NewENode(EADD);
            tmp->data.diadic.left = result;
            tmp->data.diadic.right = addend;
            tmp->cost = result->cost + 1;
            if (result->rtype->type == TYPEPOINTER) {
                tmp->rtype = result->rtype;
            } else {
                tmp->rtype = addend->rtype;
            }
            result = tmp;
        } else {
            result = addend;
        }
    }

    if (!CInt64_IsZero(&val)) {
        tmp = IRO_NewENode(EADD);
        tmp->data.diadic.left = result;
        tmp->data.diadic.right = IRO_NewENodeIntConst(val, type);
        tmp->cost = result->cost + 1;
        if (result->rtype->type == TYPEPOINTER) {
            tmp->rtype = result->rtype;
        } else {
            tmp->rtype = type;
        }
        result = tmp;
    }

    result->flags = expr->flags;
    el = FirstAddend;
    while (el) {
        next = el->next;
        IRO_ofree(el);
        el = next;
    }
    return result;
}

static ENode* TransformExprNode(ENode* expr) {
    ENode* left;

    switch (expr->type) {
        case EINDIRECT:
            if (copts.unknown121 && ENODE_IS(expr->data.monadic, EADD)) {
                expr->data.monadic = CombineConstants(expr->data.monadic);
            }
            break;

        case EMUL:
        case EADD:
        case EAND:
        case EXOR:
        case EOR:
            if (IS_TYPE_INT(expr->rtype) &&
                TYPE_INTEGRAL(expr->rtype)->integral < 0x11 &&
                !IRO_IsENodeIntConstant(expr->data.diadic.right) &&
                IRO_IsENodeIntConstant(expr->data.diadic.left)) {
                left = expr->data.diadic.left;
                expr->data.diadic.left = expr->data.diadic.right;
                expr->data.diadic.right = left;
            }
            break;

        case EEQU:
        case ENOTEQU:
            if (IS_TYPE_INT(expr->rtype) &&
                TYPE_INTEGRAL(expr->rtype)->integral < 0x11 &&
                !IRO_IsENodeIntConstant(expr->data.diadic.right) &&
                IRO_IsENodeIntConstant(expr->data.diadic.left)) {
                left = expr->data.diadic.left;
                expr->data.diadic.left = expr->data.diadic.right;
                expr->data.diadic.right = left;
            }

            if (IRO_IsENodeIntConstant(expr->data.diadic.right) &&
                ENODE_IS(expr->data.diadic.left, EBINNOT)) {
                left = expr->data.diadic.left;
                expr->data.diadic.left = left->data.monadic;
                left->data.monadic = expr->data.diadic.right;
                expr->data.diadic.right = left;
            }
            break;
        default:
            break;
    }

    return expr;
}

static ENode* TransformExprTree(ENode* expr) {
    ENodeList* list;

    switch (expr->type) {
ENODE_CASE_MONADIC:
        expr->data.monadic = TransformExprTree(expr->data.monadic);
        break;

ENODE_CASE_DIADIC_ALL:
        expr->data.diadic.left = TransformExprTree(expr->data.diadic.left);
        expr->data.diadic.right = TransformExprTree(expr->data.diadic.right);
        break;

        case EFUNCCALL:
        case EFUNCCALLP:
            TransformExprTree(expr->data.funccall.funcref);
            for (list = expr->data.funccall.args; list; list = list->next) {
                TransformExprTree(list->node);
            }
            break;

        case ECOND:
            TransformExprTree(expr->data.cond.cond);
            TransformExprTree(expr->data.cond.expr1);
            TransformExprTree(expr->data.cond.expr2);
            break;

        case ENULLCHECK:
            TransformExprTree(expr->data.nullcheck.nullcheckexpr);
            TransformExprTree(expr->data.nullcheck.condexpr);
            break;
        default:
            break;
    }

    return TransformExprNode(expr);
}

static void FoldConstantsinAssociativeExprs(ENode* expr) {
    UInt8 nodetype1;
    UInt8 nodetype2;
    UInt8 nodetype3;
    Boolean changed;
    short op;
    CInt64 val1;
    CInt64 val2;
    CInt64 typeBits;
    CInt64 sum;

    if ((expr->type == EADD ||
         expr->type == EMUL ||
         expr->type == EAND ||
         expr->type == EOR ||
         expr->type == EXOR ||
         expr->type == ESHL ||
         expr->type == ESHR) &&
        IS_TYPE_INT(expr->rtype) &&
        TYPE_INTEGRAL(expr->rtype)->integral < 0x11 &&
        IRO_IsENodeIntConstant(expr->data.diadic.right)) {
        do {
            changed = FALSE;

            if (expr->data.diadic.left->type == expr->type &&
                IRO_IsENodeIntConstant(expr->data.diadic.left->data.diadic.right)) {
                val1 = expr->data.diadic.right->data.intval;
                val2 = expr->data.diadic.left->data.diadic.right->data.intval;
                switch (expr->type) {
                    case EADD:
                    case ESHL:
                        op = '+';
                        break;
                    case ESHR:
                        op = '+';
                        if (!is_unsigned(expr->rtype)) {
                            CInt64_SetLong(&typeBits, expr->rtype->size * 8);
                            if (CInt64_GreaterEqualU(val1, typeBits) ||
                                CInt64_GreaterEqualU(val2, typeBits)) {
                                return;
                            }

                            sum = CMach_CalcIntDiadic(expr->rtype, val1, '+', val2);
                            if (CInt64_GreaterEqualU(sum, typeBits)) {
                                val1 = CInt64_Sub(typeBits, cint64_one);
                                val2 = cint64_zero;
                            }
                        }
                        break;
                    case EMUL:
                        op = '*';
                        break;
                    case EAND:
                        op = '&';
                        break;
                    case EOR:
                        op = '|';
                        break;
                    case EXOR:
                        op = '^';
                        break;
                    default:
                        return;
                }

                expr->data.diadic.right->data.intval =
                    CMach_CalcIntDiadic(expr->rtype, val1, op, val2);
                expr->data.diadic.left = expr->data.diadic.left->data.diadic.left;
                changed = TRUE;
            } else if (((nodetype1 = expr->type) == EAND || nodetype1 == EOR) &&
                       ((nodetype2 = expr->data.diadic.left->type) == EAND || nodetype2 == EOR) &&
                       ((nodetype3 = expr->data.diadic.left->data.diadic.left->type) == EAND ||
                        nodetype3 == EOR) &&
                       IRO_IsENodeIntConstant(
                           expr->data.diadic.left->data.diadic.left->data.diadic.right
                       )) {
                val1 = expr->data.diadic.right->data.intval;
                if (CInt64_Equal(
                        val1,
                        expr->data.diadic.left->data.diadic.left->data.diadic.right->data.intval
                    )) {
                    if (nodetype1 == nodetype3) {
                        expr->data.diadic.left->data.diadic.left =
                            expr->data.diadic.left->data.diadic.left->data.diadic.left;
                        changed = TRUE;
                    } else if (nodetype2 == nodetype3) {
                        *expr = *expr->data.diadic.right;
                    } else {
                        expr->data.diadic.left = expr->data.diadic.left->data.diadic.right;
                        changed = TRUE;
                    }
                }
            }
        } while (changed);
    }
}

static void TransformExprTree1(ENode* expr) {
    ENodeList* list;

    switch (expr->type) {
ENODE_CASE_MONADIC:
        TransformExprTree1(expr->data.monadic);
        break;

ENODE_CASE_DIADIC_ALL:
        TransformExprTree1(expr->data.diadic.left);
        TransformExprTree1(expr->data.diadic.right);
        break;

        case EFUNCCALL:
        case EFUNCCALLP:
            TransformExprTree1(expr->data.funccall.funcref);
            for (list = expr->data.funccall.args; list; list = list->next) {
                TransformExprTree1(list->node);
            }
            break;

        case ECOND:
            TransformExprTree1(expr->data.cond.cond);
            TransformExprTree1(expr->data.cond.expr1);
            TransformExprTree1(expr->data.cond.expr2);
            break;

        case ENULLCHECK:
            TransformExprTree1(expr->data.nullcheck.nullcheckexpr);
            TransformExprTree1(expr->data.nullcheck.condexpr);
            break;
        default:
            break;
    }

    FoldConstantsinAssociativeExprs(expr);
}

//! @reconstructed
static Boolean RemoveRedundantBitOperations(ENode* expr) {
    Boolean a;
    Boolean b;

    if (expr->type == ExprType) {
        a = RemoveRedundantBitOperations(expr->data.diadic.left);
        b = RemoveRedundantBitOperations(expr->data.diadic.right);
        return a && b;
    }

    if (expr->type == EINDIRECT) {
        if (expr->data.monadic->type == EOBJREF) {
            if (OperandObject) {
                if (expr->data.monadic->data.objref == OperandObject) {
                    return TRUE;
                }
                return FALSE;
            }
            OperandObject = expr->data.monadic->data.objref;
            IndirectRef = expr;
            return TRUE;
        } else {
            return FALSE;
        }
    }

    if (IRO_IsENodeIntConstant(expr)) {
        if (FirstTime) {
            OperandConst = expr->data.intval;
            FirstTime = FALSE;
        } else if (ExprType == EAND) {
            OperandConst = CInt64_And(expr->data.intval, OperandConst);
        } else if (ExprType == EOR) {
            OperandConst = CInt64_Or(expr->data.intval, OperandConst);
        } else if (ExprType == EXOR) {
            OperandConst = CInt64_Xor(expr->data.intval, OperandConst);
        }
        return TRUE;
    }

    return FALSE;
}

static void TransformExprTree2(ENode* expr) {
    ENodeList* list;

    switch (expr->type) {
ENODE_CASE_MONADIC:
        TransformExprTree2(expr->data.monadic);
        break;

ENODE_CASE_DIADIC_ALL:
        TransformExprTree2(expr->data.diadic.left);
        TransformExprTree2(expr->data.diadic.right);
        break;

        case EFUNCCALL:
        case EFUNCCALLP:
            TransformExprTree2(expr->data.funccall.funcref);
            for (list = expr->data.funccall.args; list; list = list->next) {
                TransformExprTree2(list->node);
            }
            break;

        case ECOND:
            TransformExprTree2(expr->data.cond.cond);
            TransformExprTree2(expr->data.cond.expr1);
            TransformExprTree2(expr->data.cond.expr2);
            break;

        case ENULLCHECK:
            TransformExprTree2(expr->data.nullcheck.nullcheckexpr);
            TransformExprTree2(expr->data.nullcheck.condexpr);
            break;
        default:
            break;
    }

    if (ENODE_IS3(expr, EAND, EOR, EXOR) &&
        (expr->type == expr->data.diadic.left->type ||
         expr->type == expr->data.diadic.right->type)) {
        OperandObject = NULL;
        ExprType = expr->type;
        FirstTime = TRUE;
        IndirectRef = NULL;

        if (RemoveRedundantBitOperations(expr)) {
            expr->data.diadic.left = IndirectRef;
            expr->data.diadic.right->type = EINTCONST;
            expr->data.diadic.right->data.intval = OperandConst;
        }
    }
}

#pragma opt_common_subs off
//! @reconstructed
static void SplitPostIncrement(Statement* stmt, ENode** where) {
    ENode* node;
    ENodeType type;
    ENode* load;
    ENode* objref;
    ENode* copy;
    Statement* newstmt;

    node = *where;
    type = node->type;
    switch (type) {
ENODE_CASE_MONADIC:
        if (type != EFORCELOAD) {
            SplitPostIncrement(stmt, &node->data.monadic);
        }
        node = *where;
        type = node->type;
        if (type == EPOSTINC || type == EPOSTDEC) {
            load = node->data.monadic;
            if (load->type == EINDIRECT &&
                load->rtype &&
                !CParser_IsVolatile(load->rtype, load->flags & 3)) {
                objref = load->data.monadic;
                if (objref->type == EOBJREF && !is_volatile_object(objref->data.objref)) {
                    newstmt = IRO_NewStatement(ST_EXPRESSION, *where);
                    IRO_CopyCommonStatementFields(newstmt, stmt);
                    newstmt->next = stmt->next;
                    stmt->next = newstmt;

                    copy = IRO_NewENode(EINDIRECT);
                    *copy = *load;
                    copy->data.monadic = IRO_NewENode(EOBJREF);
                    *copy->data.monadic = *load->data.monadic;
                    *where = copy;
                }
            }
        }
        break;

ENODE_CASE_DIADIC_ALL:
        if (type != ECOND &&
            type != ECOMMA &&
            type != ELOR &&
            type != ELAND &&
            type != ENULLCHECK) {
            SplitPostIncrement(stmt, &node->data.diadic.left);
            SplitPostIncrement(stmt, &(*where)->data.diadic.right);
        }
        break;
        default:
            break;
    }
}
#pragma opt_common_subs on

void IRO_TransformTree(Statement* statements) {
    Statement* stmt;
    Statement* next;

    TemplateExpr = CExpr_NewENode(ELOR);
    TemplateExpr->data.diadic.left = CExpr_NewENode(ELESS);
    TemplateExpr->data.diadic.left->data.diadic.left = CExpr_NewENode(EINDIRECT);
    TemplateExpr->data.diadic.left->data.diadic.left->data.monadic = CExpr_NewENode(EOBJREF);
    TemplateExpr->data.diadic.left->data.diadic.left->data.monadic->data.objref = NULL;
    TemplateExpr->data.diadic.left->data.diadic.right = intconstnode(TYPE(&stunsignedlong), 0);
    TemplateExpr->data.diadic.right = CExpr_NewENode(EGREATEREQU);
    TemplateExpr->data.diadic.right->data.diadic.left = CExpr_NewENode(EINDIRECT);
    TemplateExpr->data.diadic.right->data.diadic.left->data.monadic = CExpr_NewENode(EOBJREF);
    TemplateExpr->data.diadic.right->data.diadic.right = intconstnode(TYPE(&stunsignedlong), 1);

    for (stmt = statements; stmt; stmt = next) {
        next = stmt->next;
        switch (stmt->type) {
            case ST_EXPRESSION:
            case ST_SWITCH:
            case ST_IFGOTO:
            case ST_IFNGOTO:
            case ST_RETURN:
                if (stmt->expr) {
                    if (copts.opt_movepostops && stmt->type == ST_EXPRESSION) {
                        SplitPostIncrement(stmt, &stmt->expr);
                    }
                    stmt->expr = TransformExprTree(stmt->expr);
                    TransformExprTree2(stmt->expr);
                    TransformExprTree1(stmt->expr);
                }
                break;
            default:
                break;
        }
    }

    IRO_CheckForUserBreak();
}

//! @reconstructed
static Boolean IsLogicalSelectionTest(ENode* expr, ENode* node, ENode** var, CInt64* value) {
    ENode* scan;
    ENode* scan2;

    if (expr->type == ELOR) {
        if (node->type != EEQU) {
            if (node->type == ELOGNOT) {
                scan = node->data.monadic;
                while (scan->type == ETYPCON) {
                    scan = scan->data.monadic;
                }
                if (scan->type == EINDIRECT) {
                    if (var) {
                        *var = node;
                    }
                    if (value) {
                        *value = cint64_zero;
                    }
                    return TRUE;
                }
            }
            return FALSE;
        }
    } else if (expr->type == ELAND) {
        if (node->type != ENOTEQU) {
            scan2 = node;
            while (scan2->type == ETYPCON) {
                scan2 = scan2->data.monadic;
            }
            if (scan2->type == EINDIRECT) {
                if (var) {
                    *var = node->data.monadic;
                }
                if (value) {
                    *value = cint64_zero;
                }
                return TRUE;
            }
            return FALSE;
        }
    } else {
        return FALSE;
    }

    if (IRO_IsENodeIntConstant(node->data.diadic.left)) {
        if (var) {
            *var = node->data.diadic.right;
        }
        if (value) {
            *value = node->data.diadic.left->data.intval;
        }
        return TRUE;
    }
    if (IRO_IsENodeIntConstant(node->data.diadic.right)) {
        if (var) {
            *var = node->data.diadic.left;
        }
        if (value) {
            *value = node->data.diadic.right->data.intval;
        }
        return TRUE;
    }
    return FALSE;
}

//! @reconstructed
static Boolean AddSelectionValue(ValueSet* set, ENode* var, CInt64 value) {
    ValueNode* node;
    ValueNode** p;

    if (!IRO_ENodeExprsSame(var, set->var)) {
        return FALSE;
    }
    if (CInt64_Less(value, set->min)) {
        set->min = value;
    } else if (CInt64_Greater(value, set->max)) {
        set->max = value;
    }
    node = lalloc(sizeof(ValueNode));
    node->value = value;
    p = &set->values;
    while (*p && CInt64_Greater(value, (*p)->value)) {
        p = &(*p)->next;
    }
    if (*p) {
        if (!CInt64_Equal(value, (*p)->value)) {
            node->next = *p;
            *p = node;
        }
    } else {
        *p = node;
        node->next = NULL;
    }
    set->count++;
    return TRUE;
}

//! @reconstructed
static void NormalizeValueSet(ValueSet* set) {
    ValueNode* node;
    ValueNode* head;
    ValueNode* last;
    CInt64 tmp;
    CInt64 span;

    CError_ASSERT(4911, !set->wrapped);
    span = CInt64_Sub(set->max, set->min);
    if (CInt64_Less(span, cint64_zero)) {
        set->wrapped = TRUE;
        tmp = set->min;
        set->min = set->max;
        set->max = tmp;
        last = set->values;
        while (last->next) {
            last = last->next;
        }
        head = set->values;
        while (CInt64_Less(head->value, cint64_zero)) {
            last->next = head;
            last = head;
            head = head->next;
            set->values = head;
            last->next = NULL;
        }
    }
}

//! @reconstructed
static SInt32 CountValueRanges(ValueSet* set, SInt32 bits) {
    ValueNode* node;
    SInt32 count;
    CInt64 runstart;
    CInt64 u;
    CInt64 w;

    count = 0;
    node = set->values;
    while (node) {
        runstart = node->value;
        while (node) {
            u = CInt64_Sub(node->value, runstart);
            CInt64_SetLong(&w, bits);
            if (!CInt64_LessU(u, w)) {
                break;
            }
            node = node->next;
        }
        if (node && CInt64_IsZero(&u)) {
            node = node->next;
        }
        count++;
    }
    return count;
}

//! @reconstructed
static Boolean IsLogicalSelectionBitTestBeneficial(ValueSet* set, Boolean* uselonglong) {
    SInt32 ranges;
    SInt32 llranges;
    SInt32 llbits;

    ranges = CountValueRanges(set, stunsignedint.size * 8);
    llbits = stunsignedlonglong.size * 8;
    if (copts.unknown129) {
        llranges = CountValueRanges(set, llbits);
        *uselonglong = 3 * llranges < 2 * ranges;
        if (*uselonglong) {
            if (2 * llranges >= set->count) {
                IRO_Dump(
                    "IsLogicalSelectionBitTestBeneficial: no (%d vals, %d llranges)\n",
                    set->count,
                    llranges
                );
                return FALSE;
            }
            IRO_Dump(
                "IsLogicalSelectionBitTestBeneficial: yes (%d vals, %d llranges)\n",
                set->count,
                llranges
            );
            return TRUE;
        }
    }
    if (2 * ranges >= set->count) {
        IRO_Dump(
            "IsLogicalSelectionBitTestBeneficial: no (%d vals, %d ranges)\n", set->count, ranges
        );
        return FALSE;
    }
    IRO_Dump("IsLogicalSelectionBitTestBeneficial: yes (%d vals, %d ranges)\n", set->count, ranges);
    return TRUE;
}

//! @reconstructed
static ENode* MakeBitTestNode(
    ENode* result, ValueSet* set, Type* type, Boolean wide, CInt64 runstart, CInt64 v, CInt64 mask
) {
    Type* maskType;
    ENode* conv;
    ENode* range;
    ENode* bitTest;
    ENodeType op;
    CInt64 t;
    CInt64 shift;
    CInt64 sh;
    CInt64 subv;
    CInt64 notzero;
    CInt64 maskvalue;
    SInt32 n;

    maskType = wide ? TYPE(&stunsignedlonglong) : TYPE(&stunsignedint);
    conv = makemonadicnode(IRO_DuplicateENode(set->var), ETYPCON);
    conv->rtype = IRO_CopyType(type);
    if (type->type == TYPEINT) {
        if (TYPE_INTEGRAL(type)->integral == IT_BOOL) {
            CError_FATAL(5095);
        }
        op = set->expr->type == ELOR ? ELESSEQU : EGREATER;
    } else {
        op = set->expr->type == ELOR ? ELESSEQU : EGREATER;
    }
    range = makediadicnode(
        makediadicnode(conv, IRO_NewENodeIntConst(runstart, type), ESUB),
        IRO_NewENodeIntConst(subv = CInt64_Sub(v, runstart), type),
        op
    );
    range->rtype = IRO_LogicalType();
    conv = makemonadicnode(IRO_DuplicateENode(set->var), ETYPCON);
    conv->rtype = type;
    if (type->type == TYPEINT && TYPE_INTEGRAL(type)->integral == IT_BOOL) {
        CError_FATAL(5109);
    }
    t = CInt64_Sub(v, runstart);
    n = t.lo + 1;
    if (n < 64) {
        CInt64_SetLong(&shift, n);
        maskvalue = CInt64_Inv(sh = CInt64_Shl(notzero = CInt64_Inv(cint64_zero), shift));
    } else {
        maskvalue = CInt64_Inv(cint64_zero);
    }
    if (!CInt64_Equal(mask, maskvalue)) {
        bitTest = makediadicnode(
            IRO_NewENodeIntConst(mask, maskType),
            makediadicnode(
                IRO_NewENodeIntConst(cint64_one, maskType),
                makediadicnode(conv, IRO_NewENodeIntConst(runstart, type), ESUB),
                ESHL
            ),
            EAND
        );
        bitTest = makemonadicnode(bitTest, ELOGNOT);
        bitTest->rtype = IRO_LogicalType();
        bitTest = makemonadicnode(bitTest, ELOGNOT);
        bitTest->rtype = IRO_LogicalType();
        if (set->expr->type == ELAND) {
            bitTest = makemonadicnode(bitTest, ELOGNOT);
        }
        range = makediadicnode(range, bitTest, set->expr->type == ELOR ? ELAND : ELOR);
    }
    return range;
}

//! @reconstructed
static ENode* MakeBitTestTree(ValueSet* set, Boolean uselonglong) {
    Type* type;
    SInt32 bits;
    ENode* result;
    ENode* new;
    SInt32 wide;
    ValueNode* node;
    CInt64 v;
    CInt64 runstart;
    CInt64 sh;
    CInt64 mask;
    CInt64 u;
    CInt64 wsub;
    CInt64 w;

    type = IRO_UnsignedType(set->var->rtype);
    bits = uselonglong ? stunsignedlonglong.size * 8 : stunsignedint.size * 8;
    result = NULL;
    node = set->values;
    while (node) {
        v = node->value;
        runstart = v;
        mask = cint64_zero;
        while (node) {
            u = CInt64_Sub(node->value, runstart);
            CInt64_SetLong(&w, bits);
            if (!CInt64_LessU(u, w)) {
                break;
            }
            v = node->value;
            sh = CInt64_Shl(cint64_one, u);
            mask = CInt64_Or(mask, sh);
            node = node->next;
        }
        if (CInt64_Equal(runstart, v)) {
            new = makediadicnode(
                IRO_DuplicateENode(set->var),
                IRO_NewENodeIntConst(runstart, set->var->rtype),
                set->expr->type == ELOR ? EEQU : ENOTEQU
            );
            new->rtype = IRO_LogicalType();
            if (node && CInt64_IsZero(&u)) {
                node = node->next;
            }
        } else {
            if (!uselonglong) {
                wide = 0;
            } else {
                CInt64 t2;
                CInt64_SetLong(&t2, stunsignedint.size * 8);
                wide = CInt64_GreaterEqualU(wsub = CInt64_Sub(v, runstart), t2);
            }
            new = MakeBitTestNode(result, set, type, wide, runstart, v, mask);
        }
        if (result) {
            result = makediadicnode(result, new, set->expr->type);
        } else {
            result = new;
        }
    }
    return result;
}

//! @reconstructed
static ENode* TransformLogicalSelection(ENode* expr, ENode** result, ENode* var, CInt64 value) {
    ValueSet set = {0};
    ENode* cur;
    ENode* scan;
    Boolean uselonglong;
    ENode* new;

    if (IRO_ENodeExprHasSideEffect(var)) {
        return expr;
    }
    set.expr = expr;
    set.min = set.max = value;
    set.var = var;
    set.values = lalloc(sizeof(ValueNode));
    set.values->value = set.min;
    set.values->next = NULL;
    set.count = 1;
    *result = expr;
    scan = expr;
    while (1) {
        cur = scan;
        scan = scan->data.diadic.left;
        if (scan->type == set.expr->type) {
            if (IsLogicalSelectionTest(scan, scan->data.diadic.right, &var, &value) &&
                AddSelectionValue(&set, var, value)) {
                continue;
            }
            *result = scan;
        } else if (IsLogicalSelectionTest(set.expr, scan, &var, &value) &&
                   AddSelectionValue(&set, var, value)) {
            *result = NULL;
        } else {
            *result = scan;
        }
        break;
    }
    NormalizeValueSet(&set);
    if (!IsLogicalSelectionBitTestBeneficial(&set, &uselonglong)) {
        return set.expr;
    }
    new = MakeBitTestTree(&set, uselonglong);
    if (*result == scan) {
        new = makediadicnode(cur->data.diadic.left, new, set.expr->type);
        new->rtype = IRO_LogicalType();
        return new;
    }
    return new;
}

//! @reconstructed
static Boolean TransformLogicalSelectionExpr(ENode** where, SInt32 visit, void* data) {
    ENode* expr;
    ENode* var;
    CInt64 value;
    ENode* out;

    expr = *where;
    if (expr->flags & ENODE_FLAG_VOLATILE) {
        return FALSE;
    }
    if (expr->type == ELAND || expr->type == ELOR) {
        if (IsLogicalSelectionTest(expr, expr->data.diadic.right, &var, &value)) {
            *where = TransformLogicalSelection(expr, &out, var, value);
            CError_ASSERT(5386, *where != NULL);
            return out != NULL;
        }
    }
    return TRUE;
}

void IRO_TransformLogicalSelections(Statement* statements) {
    Statement* stmt;
    Statement* next;

    for (stmt = statements; stmt; stmt = next) {
        next = stmt->next;
        switch (stmt->type) {
            case ST_EXPRESSION:
            case ST_SWITCH:
            case ST_IFGOTO:
            case ST_IFNGOTO:
            case ST_RETURN:
                if (stmt->expr) {
                    IRO_WalkENodeTree(&stmt->expr, TransformLogicalSelectionExpr, 1, 0);
                }
                break;
            default:
                break;
        }
    }

    IRO_CheckForUserBreak();
}

static void fn_00581D50(FactorCell* list, FactorNode* term) {
    FactorCell* next;
    FactorCell* prev;
    FactorCell* cell;
    SInt32 isNull;

    prev = NULL;
    for (;;) {
        isNull = list == NULL;
        if (isNull) {
            goto done;
        }
        if (list->node == NULL) {
            goto done;
        }
        if (list->node->list.tail == term->list.tail) {
            break;
        }
        prev = list;
        list = list->next;
    }

    IRO_TermLinearList(&list->node->list);
    IRO_ofree(list->node);
    if (prev) {
        prev->next = list->next;
        list->node = NULL;
        list->next = NULL;
        while (!isNull && list->node) {
            fn_00581D50(list, list->node);
        }
        IRO_ofree(list);
    } else {
        next = list->next;
        if (next == NULL) {
            list->node = NULL;
            return;
        }
        list->node = next->node;
        list->next = list->next->next;
        next->node = NULL;
        isNull = next == NULL;
        cell = next;
        next->next = NULL;
        while (!isNull && cell->node) {
            fn_00581D50(cell, cell->node);
        }
        IRO_ofree(next);
    }
done:
    return;
}

static void fn_00581C60(FactorCell* list, FactorNode* term) {
    FactorCell* prev;
    FactorCell* next;
    FactorCell* cell;
    SInt32 isNull;

    prev = NULL;
    for (;;) {
        isNull = list == NULL;
        if (isNull) {
            goto done;
        }
        if (list->node == NULL) {
            goto done;
        }
        if (list->node->list.tail == term->list.tail) {
            break;
        }
        prev = list;
        list = list->next;
    }
    if (prev) {
        prev->next = list->next;
        list->node = NULL;
        list->next = NULL;
        while (!isNull && list->node) {
            fn_00581D50(list, list->node);
        }
        IRO_ofree(list);
    } else {
        next = list->next;
        if (next == NULL) {
            list->node = NULL;
            return;
        }
        list->node = next->node;
        list->next = list->next->next;
        next->node = NULL;
        isNull = next == NULL;
        cell = next;
        next->next = NULL;
        while (!isNull && cell->node) {
            fn_00581D50(cell, cell->node);
        }
        IRO_ofree(next);
    }
done:
    return;
}

static void fn_00581AF0(PairCell* factors, FactorPair* pair) {
    PairCell* prev;
    PairCell* cell;
    FactorCell* list;
    FactorPair* node;
    PairCell* next;
    SInt32 isNull;
    SInt32 isEmpty;

    prev = NULL;
    cell = factors;
    for (;;) {
        isNull = cell == NULL;
        if (isNull) {
            goto done;
        }
        node = (FactorPair*)cell->node;
        if (node == NULL) {
            goto done;
        }
        if (node->list.tail == pair->list.tail) {
            break;
        }
        prev = cell;
        cell = cell->next;
    }

    IRO_TermLinearList(&node->list);
    list = node->pos;
    isEmpty = list == NULL;
    while (!isEmpty && list->node) {
        fn_00581C60(list, list->node);
    }
    IRO_ofree(node->pos);
    list = node->neg;
    isEmpty = list == NULL;
    while (!isEmpty && list->node) {
        fn_00581C60(list, list->node);
    }
    IRO_ofree(node->neg);
    IRO_ofree(cell->node);
    if (prev) {
        prev->next = cell->next;
        cell->node = NULL;
        cell->next = NULL;
        while (!isNull && cell->node) {
            fn_00581AF0(cell, cell->node);
        }
        IRO_ofree(cell);
    } else {
        next = cell->next;
        if (next == NULL) {
            cell->node = NULL;
            return;
        }
        cell->node = next->node;
        cell->next = cell->next->next;
        next->node = NULL;
        isNull = next == NULL;
        cell = next;
        next->next = NULL;
        while (!isNull && cell->node) {
            fn_00581AF0(cell, cell->node);
        }
        IRO_ofree(next);
    }
done:
    return;
}

static Boolean fn_00581AA0(IROLinear* nd) {
    return nd->type == IROLinearOp2Arg &&
        (nd->nodetype == EADD || nd->nodetype == ESUB) &&
        IsSmallIntType(nd->rtype);
}

static inline Boolean IsNegatable(IROLinear* nd) {
    Boolean isNeg;

    isNeg = FALSE;
    if (IsSmallIntType(nd->rtype)) {
        isNeg = (nd->type == IROLinearOp1Arg && nd->nodetype == EMONMIN) ||
            (nd->type == IROLinearOp2Arg && nd->nodetype == ESUB) ||
            IRO_IsIntConstant(nd);
    }
    return isNeg;
}

static UInt8 fn_005817B0(IROLinear* nd) {
    Boolean isMul;

    if (IsSmallIntType(nd->rtype)) {
        if (nd->type == IROLinearOp1Arg && nd->nodetype == EMONMIN) {
            return 1;
        }
        if (nd->type == IROLinearOp2Arg && nd->nodetype == ESUB) {
            return 2;
        }
        if (IRO_IsIntConstant(nd)) {
            return 3;
        }
        if (fn_00581AA0(nd)) {
            if (IsNegatable(nd->u.diadic.left)) {
                return 4;
            }
            if (IsNegatable(nd->u.diadic.right)) {
                return 5;
            }
        }
        isMul = nd->type == IROLinearOp2Arg && nd->nodetype == EMUL && IsSmallIntType(nd->rtype);
        if (isMul) {
            if (IsNegatable(nd->u.diadic.left)) {
                return 6;
            }
            if (IsNegatable(nd->u.diadic.right)) {
                return 7;
            }
        }
    }
    return 0;
}

static void fn_00581480(PairCell* factors, FactorNode* term, IROLinear* nd) {
    FactorPair* found;
    FactorPair* other;
    PairCell* cell;
    PairCell* newcell;
    FactorCell* tcell;
    FactorNode* match;
    FactorNode* match2;
    FactorNode* matchP;
    UInt8 kind;
    IROLinear* negdup;
    IROList tmplist;
    Boolean isMul;

    isMul = nd->type == IROLinearOp2Arg && nd->nodetype == EMUL && IsSmallIntType(nd->rtype);
    if (isMul) {
        fn_00581480(factors, term, nd->u.diadic.left);
        fn_00581480(factors, term, nd->u.diadic.right);
        return;
    }

    found = NULL;
    cell = factors;
    while (cell) {
        if (cell->node == NULL) {
            break;
        }
        if (IRO_ExprsSame(((FactorPair*)cell->node)->list.tail, nd)) {
            found = (FactorPair*)cell->node;
            break;
        }
        cell = cell->next;
    }
    if (!found) {
        found = IRO_oalloc(0x14);
        IRO_InitList(&found->list);
        IRO_DuplicateExpr(nd, &found->list);
        found->other = NULL;
        found->pos = IRO_oalloc(8);
        found->pos->node = NULL;
        found->pos->next = NULL;
        found->neg = IRO_oalloc(8);
        found->neg->node = NULL;
        found->neg->next = NULL;
        if (factors->node) {
            newcell = IRO_oalloc(8);
            newcell->node = NULL;
            newcell->next = NULL;
            newcell->node = factors->node;
            newcell->next = factors->next;
            factors->next = newcell;
        }
        factors->node = found;
        kind = fn_005817B0(nd);
        IRO_InitList(&tmplist);
        negdup = fn_00580DC0(nd, kind, &tmplist);
        other = IRO_oalloc(0x14);
        IRO_InitList(&other->list);
        IRO_DuplicateExpr(negdup, &other->list);
        other->other = NULL;
        other->pos = IRO_oalloc(8);
        other->pos->node = NULL;
        other->pos->next = NULL;
        other->neg = IRO_oalloc(8);
        other->neg->node = NULL;
        other->neg->next = NULL;
        if (factors->node) {
            newcell = IRO_oalloc(8);
            newcell->node = NULL;
            newcell->next = NULL;
            newcell->node = factors->node;
            newcell->next = factors->next;
            factors->next = newcell;
        }
        factors->node = other;
        IRO_TermLinearList(&tmplist);
        found->other = other;
        other->other = found;
    }

    other = found->other;
    match = NULL;
    for (tcell = other->pos; tcell; tcell = tcell->next) {
        if (tcell->node == NULL) {
            break;
        }
        if (tcell->node->list.tail == term->list.tail) {
            match = tcell->node;
            break;
        }
    }
    if (!match) {
        if (other->pos->node) {
            tcell = IRO_oalloc(8);
            tcell->node = NULL;
            tcell->next = NULL;
            tcell->node = other->pos->node;
            tcell->next = other->pos->next;
            other->pos->next = tcell;
        }
        other->pos->node = term;
        match2 = NULL;
        if (other->neg->node) {
            for (tcell = other->neg; tcell; tcell = tcell->next) {
                if (tcell->node == NULL) {
                    break;
                }
                if (tcell->node->list.tail == term->list.tail) {
                    match2 = tcell->node;
                    break;
                }
            }
        }
        if (!match2) {
            if (other->neg->node) {
                tcell = IRO_oalloc(8);
                tcell->node = NULL;
                tcell->next = NULL;
                tcell->node = other->neg->node;
                tcell->next = other->neg->next;
                other->neg->next = tcell;
            }
            other->neg->node = term;
        }
    }

    matchP = NULL;
    if (found->pos->node) {
        for (tcell = found->pos; tcell; tcell = tcell->next) {
            if (tcell->node == NULL) {
                break;
            }
            if (tcell->node->list.tail == term->list.tail) {
                matchP = tcell->node;
                break;
            }
        }
    }
    if (!matchP) {
        if (found->pos->node) {
            tcell = IRO_oalloc(8);
            tcell->node = NULL;
            tcell->next = NULL;
            tcell->node = found->pos->node;
            tcell->next = found->pos->next;
            found->pos->next = tcell;
        }
        found->pos->node = term;
    }
    fn_00581C60(found->neg, term);
}

static void fn_005813A0(PairCell* factors, FactorNode* term, IROLinear* nd) {
    PairCell* cell;
    FactorPair* other;
    FactorPair* pair;
    Boolean isMul;

    isMul = nd->type == IROLinearOp2Arg && nd->nodetype == EMUL && IsSmallIntType(nd->rtype);
    if (isMul) {
        fn_005813A0(factors, term, nd->u.diadic.left);
        fn_005813A0(factors, term, nd->u.diadic.right);
        return;
    }

    cell = factors;
    while (cell) {
        if (cell->node == NULL) {
            break;
        }
        if (IRO_ExprsSame(((FactorPair*)cell->node)->list.tail, nd)) {
            break;
        }
        cell = cell->next;
    }
    pair = (FactorPair*)(cell ? cell->node : NULL);
    if (!pair) {
        return;
    }
    fn_00581C60(pair->neg, term);
    fn_00581C60(pair->pos, term);
    other = pair->other;
    fn_00581C60(other->neg, term);
    fn_00581C60(other->pos, term);
    if (pair->pos->node == NULL) {
        fn_00581AF0(factors, pair);
        fn_00581AF0(factors, other);
    }
}

static void fn_00581330(FactorCell* terms, PairCell* factors, IROLinear* nd, Boolean neg) {
    FactorNode* term;
    FactorCell* cell;

    term = IRO_oalloc(0xc);
    IRO_InitList(&term->list);
    IRO_DuplicateExpr(nd, &term->list);
    term->neg = neg;
    if (terms->node != NULL) {
        cell = IRO_oalloc(8);
        cell->node = NULL;
        cell->next = NULL;
        cell->node = terms->node;
        cell->next = terms->next;
        terms->next = cell;
    }
    terms->node = term;
    fn_00581480(factors, term, nd);
}

static void fn_00581080(FactorCell* terms, PairCell* factors, IROLinear* nd, Boolean neg) {
    IROLinear* left;
    IROLinear* right;
    Boolean isSmall;
    Boolean isAddSub;
    Boolean negLeft;
    Boolean negRight;

    left = nd->u.diadic.left;
    right = nd->u.diadic.right;

    isSmall = FALSE;
    isAddSub = left->type == IROLinearOp2Arg && (left->nodetype == EADD || left->nodetype == ESUB);
    if (isAddSub) {
        isSmall = IsSmallIntType(left->rtype);
    }
    if (isSmall) {
        if (fn_00581AA0(left->u.diadic.left)) {
            fn_00581080(terms, factors, left->u.diadic.left, neg);
        } else {
            fn_00581330(terms, factors, left->u.diadic.left, neg);
        }
        negLeft = (left->nodetype == ESUB) ^ neg;
        if (fn_00581AA0(left->u.diadic.right)) {
            fn_00581080(terms, factors, left->u.diadic.right, negLeft);
        } else {
            fn_00581330(terms, factors, left->u.diadic.right, negLeft);
        }
    } else {
        fn_00581330(terms, factors, left, neg);
    }

    negRight = (nd->nodetype == ESUB) ^ neg;

    isSmall = FALSE;
    isAddSub =
        right->type == IROLinearOp2Arg && (right->nodetype == EADD || right->nodetype == ESUB);
    if (isAddSub) {
        isSmall = IsSmallIntType(right->rtype);
    }
    if (isSmall) {
        if (fn_00581AA0(right->u.diadic.left)) {
            fn_00581080(terms, factors, right->u.diadic.left, negRight);
        } else {
            fn_00581330(terms, factors, right->u.diadic.left, negRight);
        }
        negLeft = (right->nodetype == ESUB) ^ negRight;
        if (fn_00581AA0(right->u.diadic.right)) {
            fn_00581080(terms, factors, right->u.diadic.right, negLeft);
        } else {
            fn_00581330(terms, factors, right->u.diadic.right, negLeft);
        }
    } else {
        fn_00581330(terms, factors, right, negRight);
    }
}

static IROLinear* fn_00580DC0(IROLinear* nd, UInt8 kind, IROList* list) {
    IROLinear* dup;
    IROLinear* op;
    IROLinear* result;
    IROLinear* left;
    IROLinear* right;
    CInt64 value;

    switch (kind) {
        case 0:
            dup = IRO_DuplicateExpr(nd, list);
            op = IRO_NewLinear(IROLinearOp1Arg);
            op->index = ++IRO_NumLinear;
            op->nodetype = EMONMIN;
            op->u.monadic = dup;
            op->rtype = IRO_CopyType(dup->rtype);
            IRO_AddToList(op, list);
            break;
        case 1:
            IRO_DuplicateExpr(nd->u.diadic.left, list);
            break;
        case 2:
            right = IRO_DuplicateExpr(nd->u.diadic.right, list);
            dup = IRO_DuplicateExpr(nd, list);
            left = dup->u.diadic.left;
            op = IRO_NewLinear(IROLinearOp2Arg);
            op->index = ++IRO_NumLinear;
            op->nodetype = ESUB;
            op->u.diadic.left = right;
            op->u.diadic.right = left;
            op->rtype = IRO_CopyType(right->rtype);
            IRO_AddToList(op, list);
            break;
        case 3:
            value = CInt64_Neg(nd->u.node->data.intval);
            IRO_TruncateValueToType(&value, nd->rtype);
            dup = IRO_NewIntConst(value, nd->rtype);
            IRO_AddToList(dup, list);
            break;
        case 4:
            kind = fn_005817B0(nd->u.diadic.left);
            left = fn_00580DC0(nd->u.diadic.left, kind, list);
            dup = IRO_DuplicateExpr(nd->u.diadic.right, list);
            result = IRO_NewLinear(IROLinearOp2Arg);
            result->index = ++IRO_NumLinear;
            result->nodetype = ESUB;
            result->u.diadic.left = left;
            result->u.diadic.right = dup;
            result->rtype = IRO_CopyType(left->rtype);
            IRO_AddToList(result, list);
            break;
        case 5:
            kind = fn_005817B0(nd->u.diadic.right);
            left = fn_00580DC0(nd->u.diadic.right, kind, list);
            right = IRO_DuplicateExpr(nd->u.diadic.left, list);
            result = IRO_NewLinear(IROLinearOp2Arg);
            result->index = ++IRO_NumLinear;
            result->nodetype = ESUB;
            result->u.diadic.left = left;
            result->u.diadic.right = right;
            result->rtype = IRO_CopyType(left->rtype);
            IRO_AddToList(result, list);
            break;
        case 6:
            kind = fn_005817B0(nd->u.diadic.left);
            left = fn_00580DC0(nd->u.diadic.left, kind, list);
            right = IRO_DuplicateExpr(nd->u.diadic.right, list);
            op = IRO_NewLinear(IROLinearOp2Arg);
            op->index = ++IRO_NumLinear;
            op->nodetype = EMUL;
            op->u.diadic.left = left;
            op->u.diadic.right = right;
            op->rtype = IRO_CopyType(left->rtype);
            IRO_AddToList(op, list);
            break;
        case 7:
            kind = fn_005817B0(nd->u.diadic.right);
            left = IRO_DuplicateExpr(nd->u.diadic.left, list);
            right = fn_00580DC0(nd->u.diadic.right, kind, list);
            op = IRO_NewLinear(IROLinearOp2Arg);
            op->index = ++IRO_NumLinear;
            op->nodetype = EMUL;
            op->u.diadic.left = left;
            op->u.diadic.right = right;
            op->rtype = IRO_CopyType(left->rtype);
            IRO_AddToList(op, list);
            break;
    }
    return list->tail;
}

static IROLinear* fn_00580B90(FactorCell* terms, IROList* list) {
    FactorNode* first;
    FactorCell* cell;
    FactorNode* term;
    IROLinear* tail;
    IROLinear* right;
    IROLinear* op;
    Boolean isNeg;
    UInt8 kind;

    first = NULL;
    for (cell = terms; cell && cell->node; cell = cell->next) {
        term = cell->node;
        if (term != NULL && term->neg == FALSE) {
            first = term;
            break;
        }
    }
    if (first == NULL) {
        for (cell = terms; cell && cell->node; cell = cell->next) {
            term = cell->node;
            if (term != NULL && fn_005817B0(term->list.tail) != 0) {
                first = term;
                break;
            }
        }
    }
    if (first == NULL) {
        first = terms->node;
    }

    if (first->neg == FALSE) {
        IRO_DuplicateExpr(first->list.tail, list);
        isNeg = FALSE;
    } else {
        kind = fn_005817B0(first->list.tail);
        if (kind != 0) {
            fn_00580DC0(first->list.tail, kind, list);
            isNeg = FALSE;
        } else {
            IRO_DuplicateExpr(first->list.tail, list);
            isNeg = TRUE;
        }
    }
    tail = list->tail;
    if (isNeg) {
        op = IRO_NewLinear(IROLinearOp1Arg);
        op->index = ++IRO_NumLinear;
        op->nodetype = EMONMIN;
        op->u.monadic = tail;
        op->rtype = IRO_CopyType(tail->rtype);
        IRO_AddToList(op, list);
        tail = list->tail;
    }

    for (cell = terms; cell && cell->node; cell = cell->next) {
        term = cell->node;
        if (term != NULL && term != first) {
            if (term->neg == FALSE) {
                IRO_DuplicateExpr(term->list.tail, list);
                isNeg = FALSE;
            } else {
                kind = fn_005817B0(term->list.tail);
                if (kind != 0) {
                    fn_00580DC0(term->list.tail, kind, list);
                    isNeg = FALSE;
                } else {
                    IRO_DuplicateExpr(term->list.tail, list);
                    isNeg = TRUE;
                }
            }
            right = list->tail;
            if (isNeg) {
                op = IRO_NewLinear(IROLinearOp2Arg);
                op->index = ++IRO_NumLinear;
                op->nodetype = ESUB;
            } else {
                op = IRO_NewLinear(IROLinearOp2Arg);
                op->index = ++IRO_NumLinear;
                op->nodetype = EADD;
            }
            op->u.diadic.left = tail;
            op->u.diadic.right = right;
            op->rtype = IRO_CopyType(tail->rtype);
            IRO_AddToList(op, list);
            tail = list->tail;
        }
    }
    return tail;
}

#pragma opt_lifetimes off
static Boolean fn_00580A40(FactorPair* pair, IROLinear* nd, IROList* list) {
    IROLinear* right;
    IROLinear* left;
    IROLinear* op;
    Boolean isMul;

    isMul = nd->type == IROLinearOp2Arg && nd->nodetype == EMUL && IsSmallIntType(nd->rtype);
    if (isMul) {
        left = nd->u.diadic.left;
        right = nd->u.diadic.right;
        if (IRO_ExprsSame(pair->list.tail, left)) {
            IRO_DuplicateExpr(right, list);
            return TRUE;
        }
        if (IRO_ExprsSame(pair->list.tail, right)) {
            IRO_DuplicateExpr(left, list);
            return TRUE;
        }
        if (fn_00580A40(pair, left, list)) {
            left = list->tail;
            IRO_DuplicateExpr(right, list);
            right = list->tail;
            op = IRO_NewLinear(IROLinearOp2Arg);
            op->index = ++IRO_NumLinear;
            op->nodetype = EMUL;
            op->u.diadic.left = left;
            op->u.diadic.right = right;
            op->rtype = IRO_CopyType(left->rtype);
            IRO_AddToList(op, list);
            return TRUE;
        }
        IRO_DuplicateExpr(left, list);
        left = list->tail;
        fn_00580A40(pair, right, list);
        right = list->tail;
        op = IRO_NewLinear(IROLinearOp2Arg);
        op->index = ++IRO_NumLinear;
        op->nodetype = EMUL;
        op->u.diadic.left = left;
        op->u.diadic.right = right;
        op->rtype = IRO_CopyType(left->rtype);
        IRO_AddToList(op, list);
        return TRUE;
    }
    IRO_DuplicateExpr(nd, list);
    return FALSE;
}

#pragma opt_lifetimes on

static FactorPair* fn_005809B0(PairCell* factors) {
    FactorPair* best;
    FactorPair* pair;
    PairCell* cell;
    SInt32 bestCount;
    SInt32 minNeg;
    FactorCell* list;
    SInt32 count1;
    SInt32 count2;

    best = NULL;
    bestCount = 2;
    minNeg = 0x7fffffff;
    for (cell = factors; cell && cell->node; cell = cell->next) {
        pair = (FactorPair*)cell->node;
        if (pair != NULL) {
            for (list = pair->pos, count1 = 0; list && list->node; list = list->next) {
                count1++;
            }
            if (count1 >= bestCount) {
                for (list = pair->neg, count2 = 0; list && list->node; list = list->next) {
                    count2++;
                }
                if (count1 > bestCount || count2 < minNeg) {
                    bestCount = count1;
                    minNeg = count2;
                    best = pair;
                }
            }
        }
    }
    return best;
}

static void fn_00580650(FactorCell* terms, PairCell* factors, FactorPair* pair) {
    FactorCell hdrA;
    FactorCell hdrB;
    IROList tmplist;
    FactorPair* other;
    FactorCell* cell;
    FactorCell* cell2;
    FactorNode* term;
    FactorNode* match;
    FactorNode* newterm;
    IROLinear* built;
    IROLinear* sum;
    IROLinear* mul;
    FactorCell* newcell;
    Boolean isInB;

    hdrB.node = NULL;
    hdrB.next = NULL;
    hdrA.node = NULL;
    hdrA.next = NULL;
    for (cell = pair->pos; cell && cell->node; cell = cell->next) {
        term = cell->node;
        if (term != NULL) {
            IRO_InitList(&tmplist);
            match = NULL;
            for (cell2 = pair->neg; cell2 && cell2->node; cell2 = cell2->next) {
                if (cell2->node->list.tail == term->list.tail) {
                    match = cell2->node;
                    break;
                }
            }
            isInB = match != NULL;
            if (isInB) {
                other = pair->other;
                if (IRO_ExprsSame(other->list.tail, term->list.tail)) {
                    IRO_AddToList(IRO_NewIntConst(cint64_one, term->list.tail->rtype), &tmplist);
                } else {
                    fn_00580A40(other, term->list.tail, &tmplist);
                }
            } else {
                if (IRO_ExprsSame(pair->list.tail, term->list.tail)) {
                    IRO_AddToList(IRO_NewIntConst(cint64_one, term->list.tail->rtype), &tmplist);
                } else {
                    fn_00580A40(pair, term->list.tail, &tmplist);
                }
            }
            built = tmplist.tail;
            newterm = IRO_oalloc(0xc);
            IRO_InitList(&newterm->list);
            IRO_DuplicateExpr(built, &newterm->list);
            newterm->neg = term->neg ^ isInB;
            if (hdrA.node) {
                newcell = IRO_oalloc(8);
                newcell->node = NULL;
                newcell->next = NULL;
                newcell->node = hdrA.node;
                newcell->next = hdrA.next;
                hdrA.next = newcell;
            }
            hdrA.node = newterm;
            if (hdrB.node) {
                newcell = IRO_oalloc(8);
                newcell->node = NULL;
                newcell->next = NULL;
                newcell->node = hdrB.node;
                newcell->next = hdrB.next;
                hdrB.next = newcell;
            }
            hdrB.node = term;
            IRO_TermLinearList(&tmplist);
        }
    }

    IRO_InitList(&tmplist);
    sum = fn_00580B90(&hdrA, &tmplist);
    while (hdrA.node) {
        fn_00581D50(&hdrA, hdrA.node);
    }
    IRO_DuplicateExpr(pair->list.tail, &tmplist);
    mul = IRO_NewLinear(IROLinearOp2Arg);
    mul->index = ++IRO_NumLinear;
    mul->nodetype = EMUL;
    mul->u.diadic.left = tmplist.tail;
    mul->u.diadic.right = sum;
    mul->rtype = IRO_CopyType(tmplist.tail->rtype);
    IRO_AddToList(mul, &tmplist);
    for (cell = &hdrB; cell && cell->node; cell = cell->next) {
        term = cell->node;
        if (term != NULL) {
            fn_005813A0(factors, term, term->list.tail);
            fn_00581D50(terms, term);
        }
    }
    while (hdrB.node) {
        fn_00581C60(&hdrB, hdrB.node);
    }
    newterm = IRO_oalloc(0xc);
    IRO_InitList(&newterm->list);
    IRO_DuplicateExpr(tmplist.tail, &newterm->list);
    newterm->neg = FALSE;
    if (terms->node) {
        newcell = IRO_oalloc(8);
        newcell->node = NULL;
        newcell->next = NULL;
        newcell->node = terms->node;
        newcell->next = terms->next;
        terms->next = newcell;
    }
    terms->node = newterm;
    fn_00581480(factors, newterm, tmplist.tail);
    IRO_TermLinearList(&tmplist);
}

static void fn_00580540(IROLinear* nd) {
    FactorCell terms;
    PairCell factors;
    IROList pastelist;
    FactorPair* pair;
    Boolean didAny;

    terms.node = NULL;
    terms.next = NULL;
    factors.node = NULL;
    factors.next = NULL;
    IRO_InitList(&pastelist);
    fn_00581080(&terms, &factors, nd, FALSE);
    didAny = FALSE;
    do {
        pair = fn_005809B0(&factors);
        if (pair == NULL) {
            continue;
        }
        fn_00580650(&terms, &factors, pair);
        didAny = TRUE;
    } while (pair != NULL);
    if (didAny) {
        IRO_Dump("factoring expression %d\n", nd->index);
        IRO_NopOut(nd);
        fn_00580B90(&terms, &pastelist);
        IRO_PasteAfter(pastelist.head, pastelist.tail, nd);
        IRO_LocateFather_Cut_And_Paste(nd, pastelist.tail);
    }
    IRO_TermLinearList(&pastelist);
    while (factors.node) {
        fn_00581AF0(&factors, factors.node);
    }
    while (terms.node) {
        fn_00581D50(&terms, terms.node);
    }
}

static void fn_00580470(IROLinear* nd, Boolean isEntry, void* data) {
    Boolean isInt;

    if (!isEntry) {
        return;
    }

    isInt = nd->type == IROLinearOp2Arg && nd->nodetype == EMUL && IsSmallIntType(nd->rtype);
    if (isInt) {
        IRO_ActOnChildExpressions(nd, fn_00580470, NULL);
        return;
    }

    isInt = nd->type == IROLinearOp2Arg &&
        (nd->nodetype == EADD || nd->nodetype == ESUB) &&
        IsSmallIntType(nd->rtype);
    if (isInt && !IRO_HasSideEffect(nd)) {
        IRO_ActOnChildExpressions(nd, fn_00580350, NULL);
        fn_00580540(nd);
        return;
    }

    IRO_ActOnChildExpressions(nd, fn_005802C0, NULL);
}

static void fn_00580350(IROLinear* nd, Boolean isEntry, void* data) {
    Boolean isInt;

    if (!isEntry) {
        return;
    }

    isInt = nd->type == IROLinearOp2Arg &&
        (nd->nodetype == EADD || nd->nodetype == ESUB) &&
        IsSmallIntType(nd->rtype);
    if (isInt) {
        IRO_ActOnChildExpressions(nd, fn_00580350, NULL);
        return;
    }

    isInt = nd->type == IROLinearOp2Arg && nd->nodetype == EMUL && IsSmallIntType(nd->rtype);
    if (isInt) {
        IRO_ActOnChildExpressions(nd, fn_00580470, NULL);
        return;
    }

    isInt = nd->type == IROLinearOp2Arg &&
        (nd->nodetype == EADD || nd->nodetype == ESUB) &&
        IsSmallIntType(nd->rtype);
    if (isInt && !IRO_HasSideEffect(nd)) {
        IRO_ActOnChildExpressions(nd, fn_00580350, NULL);
        fn_00580540(nd);
        return;
    }

    IRO_ActOnChildExpressions(nd, fn_005802C0, NULL);
}

static void fn_005802C0(IROLinear* nd, Boolean isEntry, void* data) {
    Boolean isInt;

    if (!isEntry) {
        return;
    }

    isInt = nd->type == IROLinearOp2Arg &&
        (nd->nodetype == EADD || nd->nodetype == ESUB) &&
        IsSmallIntType(nd->rtype);
    if (isInt && !IRO_HasSideEffect(nd)) {
        IRO_ActOnChildExpressions(nd, fn_00580350, NULL);
        fn_00580540(nd);
        return;
    }

    IRO_ActOnChildExpressions(nd, fn_005802C0, NULL);
}

void IRO_DoTransformations(IROLinear* list) {
    IROLinear* nd;
    Boolean isInt;

    for (nd = list; nd; nd = nd->next) {
        switch (nd->type) {
            case IROLinearOp3Arg:
                RemoveUnreffed(nd);
                DoTransformations26(nd);
                break;
            case IROLinearOp2Arg:
                DoDiadic(nd);
                break;
            case IROLinearOp1Arg:
                RemoveUnreffed(nd);
                RemoveRedundantMonadicOp(nd);
                ReverseOpForMonmin(nd);
                break;
            case IROLinearOperand:
                RemoveUnreffed(nd);
                break;
            default:
                break;
        }
    }

    for (nd = list; nd; nd = nd->next) {
        if (!(nd->flags & IROLF_Reffed)) {
            isInt = nd->type == IROLinearOp2Arg &&
                (nd->nodetype == EADD || nd->nodetype == ESUB) &&
                IsSmallIntType(nd->rtype);
            if (isInt && !IRO_HasSideEffect(nd)) {
                IRO_ActOnChildExpressions(nd, fn_00580350, NULL);
                fn_00580540(nd);
            } else {
                IRO_ActOnChildExpressions(nd, fn_005802C0, NULL);
            }
        }
    }

    IRO_UpdateFlagsOnInts(list);
    IRO_CheckForUserBreak();
}

static void RemoveRedundantMonadicOp(IROLinear* nd) {
    IROLinear* father;
    IROLinear* grandfather;

    if (nd->type == IROLinearOp1Arg &&
        (father = IRO_LocateFather(nd)) &&
        father->nodetype == nd->nodetype) {
        switch (nd->nodetype) {
            case ELOGNOT:
                grandfather = IRO_LocateFather(father);
                if (grandfather) {
                    if (grandfather->rtype &&
                        IS_TYPE_INT(grandfather->rtype) &&
                        TYPE_INTEGRAL(grandfather->rtype)->integral == IT_BOOL &&
                        nd->u.monadic->rtype &&
                        IS_TYPE_INT(nd->u.monadic->rtype) &&
                        TYPE_INTEGRAL(nd->u.monadic->rtype)->integral == IT_BOOL) {
                        goto remove;
                    }

                    if (grandfather->type == IROLinearIf) {
                        goto remove;
                    }
                    if (grandfather->type == IROLinearIfNot) {
                        goto remove;
                    }
                    if (grandfather->type == IROLinearOp3Arg && nd == grandfather->u.args3.a) {
                        goto remove;
                    }

                    switch (grandfather->nodetype) {
                        case ELOGNOT:
                        case ELAND:
                        case ELOR:
                            goto remove;
                        default:
                            break;
                    }
                }

                if (nd->u.monadic->type == IROLinearOp1Arg ||
                    nd->u.monadic->type == IROLinearOp2Arg) {
                    switch (nd->u.monadic->nodetype) {
                        case ELOGNOT:
                        case ELESS:
                        case EGREATER:
                        case ELESSEQU:
                        case EGREATEREQU:
                        case EEQU:
                        case ENOTEQU:
                        case ELAND:
                        case ELOR:
                            goto remove;
                        default:
                            break;
                    }
                }
                break;

            case EMONMIN:
            case EBINNOT:
remove:
                IRO_LocateFather_Cut_And_Paste_Without_Nopping(father, nd->u.monadic);
                nd->type = IROLinearNop;
                father->type = IROLinearNop;
                break;

            case ETYPCON:
                if (TYPE_INTEGRAL(nd->rtype)->integral == IT_FLOAT) {
                    switch (TYPE_INTEGRAL(father->rtype)->integral) {
                        case IT_DOUBLE:
                        case IT_LONGDOUBLE:
                            switch (TYPE_INTEGRAL(nd->u.monadic->rtype)->integral) {
                                case IT_WCHAR_T:
                                    if (!copts.ushort_wchar_t) {
                                        break;
                                    }
                                case IT_BOOL:
                                case IT_CHAR:
                                case IT_SCHAR:
                                case IT_UCHAR:
                                case IT_SHORT:
                                case IT_USHORT:
                                    nd->type = IROLinearNop;
                                    father->u.monadic = nd->u.monadic;
                                    break;
                            }
                            break;
                    }
                } else if (IRO_IsValuePreservingTypcon(father) && IRO_IsValuePreservingTypcon(nd)) {
                    nd->type = IROLinearNop;
                    father->u.monadic = nd->u.monadic;
                }
                break;
            default:
                break;
        }
    }
}
