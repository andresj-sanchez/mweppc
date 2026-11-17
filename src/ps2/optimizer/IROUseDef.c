#include "compiler/IROUseDef.h"
#include "compiler/IroDump.h"
#include "compiler/IroFlowgraph.h"
#include "compiler/IroLinearForm.h"
#include "compiler/IroMalloc.h"
#include "compiler/IroPointerAnalysis.h"
#include "compiler/IroTransform.h"
#include "compiler/IroUtil.h"
#include "compiler/IroVars.h"
#include "compiler/BitVector.h"
#include "compiler/CError.h"
#include "compiler/CFunc.h"
#include "compiler/CInt64.h"
#include "compiler/CMachine.h"
#include "compiler/CompilerTools.h"
#include "compiler/CParser.h"
#include "compiler/enode.h"
#include "compiler/InlineAsm.h"
#include "compiler/objects.h"
#include "compiler/types.h"
#include <string.h>
#include "compiler/CDecl.h"

ENodeType IRO_NonAssignmentOp[IRO_MAXEXPR];
typedef CInt64 (*AssignmentFoldingFunc)(CInt64, CInt64);
static AssignmentFoldingFunc AssignmentFoldingFunction[IRO_MAXEXPR];
static SInt32 NumDefs;
static IRODef* FirstDef;
static IRODef* LastDef;
static SInt32 NumUses;
static BitVector* Reaches;
static BitVector* MightReachAnyUse;
static BitVector* alldefs;
static BitVector* alluses;
static BitVector* defset;
static BitVector* useset;
IROUse* IRO_FirstVarUse;
IROUse* IRO_LastVarUse;

static IROLinear* IRO_AddTypeConversion(IROLinear* node, Type* type, IROList* list) {
    IROLinear* first;
    IROLinear* last;
    int isBool;

    isBool = type->type == TYPEINT && TYPE_INTEGRAL(type)->integral == IT_BOOL;

    if (isBool) {
        first = IRO_NewLinear(IROLinearOp1Arg);
        memcpy(first, node, sizeof(IROLinear));
        first->type = IROLinearOp1Arg;
        first->nodetype = ELOGNOT;
        first->index = IRO_NumLinear++;
        first->rtype = type;
        first->u.monadic = node;
        first->next = last = IRO_NewLinear(IROLinearOp1Arg);
        memcpy(last, node, sizeof(IROLinear));
        last->type = IROLinearOp1Arg;
        last->nodetype = ELOGNOT;
        last->index = IRO_NumLinear++;
        last->rtype = type;
        last->u.monadic = first;
        last->next = NULL;
        if (!list) {
            IRO_PasteAfter(first, last, node);
        } else {
            IRO_AddToList(first, list);
        }
    } else {
        first = IRO_NewLinear(IROLinearOp1Arg);
        memcpy(first, node, sizeof(IROLinear));
        first->type = IROLinearOp1Arg;
        first->nodetype = ETYPCON;
        first->index = IRO_NumLinear++;
        first->rtype = type;
        first->u.monadic = node;
        first->next = NULL;
        if (!list) {
            IRO_PasteAfter(first, first, node);
        } else {
            IRO_AddToList(first, list);
        }
        last = first;
    }
    return last;
}

static void UpdateUse(IROLinear* linear, CInt64 val, Type* type) {
    Type* promoted;
    Boolean isInt;
    IROLinear* father;
    IROLinear* newdiadic;
    IROLinear* operand;
    IROList list;
    Type* constType;
    IROLinear* repl;

    if ((father = IRO_LocateFather(linear))) {
        switch (father->type) {
            case IROLinearOp1Arg:
                switch (father->nodetype) {
                    case EPOSTINC:
                    case EPOSTDEC:
                    case EPREINC:
                    case EPREDEC:
                        val = CInt64_Add(val, IRO_GetSelfAssignmentVal(father));
                        father->nodetype = EADDASS;
                        father->type = IROLinearOp2Arg;
                        repl = IRO_NewIntConst(val, type);
                        father->u.diadic.right = repl;
                        IRO_PasteAfter(repl, repl, linear);
                        return;
                    default:
                        break;
                }
                break;
            case IROLinearOp2Arg:
                if (IRO_IsIntConstant(father->u.diadic.right)) {
                    switch (father->nodetype) {
                        case EADD:
                        case EADDASS:
                            father->u.diadic.right->u.node->data.intval =
                                CInt64_Add(father->u.diadic.right->u.node->data.intval, val);
                            return;
                        case ESUB:
                        case ESUBASS:
                            father->u.diadic.right->u.node->data.intval =
                                CInt64_Sub(father->u.diadic.right->u.node->data.intval, val);
                            return;
                        default:
                            break;
                    }
                }
                break;
            default:
                break;
        }
    }

    IRO_InitList(&list);
    promoted = type;
    isInt = type->type == TYPEINT && TYPE_INTEGRAL(type)->integral < IT_17;
    if (isInt || type->type == TYPEENUM) {
        promoted = type;
        if (type->type == TYPEENUM) {
            promoted = TYPE_ENUM(type)->enumtype;
        }
        if (TYPE_INTEGRAL(promoted)->integral < IT_INT) {
            if (is_unsigned(promoted)) {
                promoted = TYPE(&stunsignedint);
            } else {
                promoted = TYPE(&stsignedint);
            }
        }
    }
    constType = type;
    if (type->type == TYPEPOINTER) {
        constType = TYPE(&stunsignedlong);
    }
    operand = linear;
    if (promoted != type) {
        operand = IRO_AddTypeConversion(linear, promoted, &list);
    }
    if (constType == type) {
        constType = promoted;
    }
    repl = IRO_NewIntConst(val, constType);
    IRO_AddToList(repl, &list);
    newdiadic = IRO_NewLinear(IROLinearOp2Arg);
    newdiadic->index = ++IRO_NumLinear;
    newdiadic->nodetype = EADD;
    newdiadic->u.diadic.left = operand;
    newdiadic->u.diadic.right = repl;
    newdiadic->rtype = promoted;
    IRO_AddToList(newdiadic, &list);
    if (promoted != type) {
        newdiadic = IRO_AddTypeConversion(newdiadic, type, &list);
    }
    IRO_LocateFather_Cut_And_Paste_Without_Nopping(linear, newdiadic);
    IRO_PasteAfter(list.head, list.tail, linear);
}

static void UpdateUseForOtherSelfAssignment(IROLinear* a, IROLinear* b, Type* type) {
    CInt64 val;
    IROLinear* father;
    IROLinear* repl;
    IROLinear* newdiadic;
    IROLinear* operand;
    IROList list;
    Type* promoted;
    Boolean isInt;

    val = IRO_GetSelfAssignmentVal(b);
    if ((father = IRO_LocateFather(a)) &&
        father->type == IROLinearOp2Arg &&
        IRO_IsIntConstant(father->u.diadic.right)) {
        CInt64 fatherval = father->u.diadic.right->u.node->data.intval;
        if (AssignmentFoldingFunction[b->nodetype] &&
            ((father->nodetype == b->nodetype) ||
             (father->nodetype == IRO_NonAssignmentOp[b->nodetype]))) {
            CInt64 v;
            CInt64 folded;
            CInt64_SetLong(&v, b->rtype->size * 8);
            folded = AssignmentFoldingFunction[b->nodetype](fatherval, val);
            if (b->nodetype == ESHRASS && !is_unsigned(b->rtype)) {
                if (CInt64_LessU(fatherval, v) && CInt64_LessU(val, v)) {
                    if (CInt64_GreaterEqualU(folded, v)) {
                        folded = CInt64_Sub(v, cint64_one);
                    }
                    father->u.diadic.right->u.node->data.intval = folded;
                    return;
                }
            } else {
                father->u.diadic.right->u.node->data.intval = folded;
                return;
            }
        } else {
            switch (b->nodetype) {
                case EMULASS:
                    if (father->nodetype == ESHL || father->nodetype == ESHLASS) {
                        CInt64 v;
                        int powvalue;
                        if (IRO_IsPow2(b->u.diadic.right, &powvalue)) {
                            if (powvalue > 0) {
                                CInt64_SetLong(&v, powvalue);
                                father->u.diadic.right->u.node->data.intval =
                                    CInt64_Add(v, fatherval);
                            }
                        } else {
                            father->nodetype = (father->nodetype == ESHL) ? EMUL : EMULASS;
                            fatherval = CInt64_Shl(cint64_one, fatherval);
                            father->u.diadic.right->u.node->data.intval =
                                CInt64_Mul(fatherval, val);
                        }
                        return;
                    }
                    break;
                case EDIVASS:
                    if ((father->nodetype == ESHR || father->nodetype == ESHRASS) &&
                        is_unsigned(father->rtype)) {
                        CInt64 v;
                        int powvalue;
                        if (IRO_IsPow2(b->u.diadic.right, &powvalue)) {
                            if (powvalue > 0) {
                                CInt64_SetLong(&v, powvalue);
                                father->u.diadic.right->u.node->data.intval =
                                    CInt64_Add(v, fatherval);
                            }
                        } else {
                            father->nodetype = (father->nodetype == ESHR) ? EDIV : EDIVASS;
                            fatherval = CInt64_Shl(cint64_one, fatherval);
                            father->u.diadic.right->u.node->data.intval =
                                CInt64_Mul(fatherval, val);
                        }
                        return;
                    }
                    break;
                case ESHLASS:
                    if (father->nodetype == EMUL || father->nodetype == EMULASS) {
                        CInt64 v;
                        int powvalue;
                        if (IRO_IsPow2(father->u.diadic.right, &powvalue)) {
                            if (powvalue > 0) {
                                father->nodetype = (father->nodetype == EMUL) ? ESHL : ESHLASS;
                                CInt64_SetLong(&v, powvalue);
                                father->u.diadic.right->u.node->data.intval = CInt64_Add(v, val);
                            }
                        } else {
                            val = CInt64_Shl(cint64_one, val);
                            father->u.diadic.right->u.node->data.intval =
                                CInt64_Mul(fatherval, val);
                        }
                        return;
                    } else if (father->nodetype == ESHR || father->nodetype == ESHRASS) {
                        if (CInt64_Equal(fatherval, val) && is_unsigned(father->rtype)) {
                            father->nodetype = (father->nodetype == ESHR) ? EAND : EANDASS;
                            if (father->rtype->size < 8) {
                                CInt64 v;
                                CInt64_SetLong(&v, 64 - (father->rtype->size * 8));
                                val = CInt64_Add(val, v);
                            }
                            father->u.diadic.right->u.node->data.intval =
                                CInt64_ShrU(cint64_negone, val);
                            return;
                        }
                    }
                    break;
                case ESHRASS:
                    if ((father->nodetype == EDIV || father->nodetype == EDIVASS) &&
                        is_unsigned(father->rtype)) {
                        CInt64 v;
                        int powvalue;
                        if (IRO_IsPow2(father->u.diadic.right, &powvalue)) {
                            if (powvalue > 0) {
                                father->nodetype = (father->nodetype == EDIV) ? ESHR : ESHRASS;
                                CInt64_SetLong(&v, powvalue);
                                father->u.diadic.right->u.node->data.intval = CInt64_Add(v, val);
                            }
                        } else {
                            val = CInt64_Shl(cint64_one, val);
                            father->u.diadic.right->u.node->data.intval =
                                CInt64_Mul(fatherval, val);
                        }
                        return;
                    } else if (father->nodetype == ESHL || father->nodetype == ESHLASS) {
                        if (CInt64_Equal(fatherval, val)) {
                            father->nodetype = (father->nodetype == ESHL) ? EAND : EANDASS;
                            father->u.diadic.right->u.node->data.intval =
                                CInt64_Shl(cint64_negone, val);
                            return;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    IRO_InitList(&list);
    promoted = type;
    isInt = type->type == TYPEINT && TYPE_INTEGRAL(type)->integral < IT_17;
    if (isInt || type->type == TYPEENUM) {
        promoted = type;
        if (type->type == TYPEENUM) {
            promoted = TYPE_ENUM(type)->enumtype;
        }
        if (TYPE_INTEGRAL(promoted)->integral < IT_INT) {
            if (is_unsigned(promoted)) {
                promoted = TYPE(&stunsignedint);
            } else {
                promoted = TYPE(&stsignedint);
            }
        }
    }
    operand = a;
    if (promoted != type) {
        operand = IRO_AddTypeConversion(a, promoted, &list);
    }
    repl = IRO_NewIntConst(val, promoted);
    IRO_AddToList(repl, &list);
    newdiadic = IRO_NewLinear(IROLinearOp2Arg);
    newdiadic->index = ++IRO_NumLinear;
    newdiadic->nodetype = IRO_NonAssignmentOp[b->nodetype];
    newdiadic->u.diadic.left = operand;
    newdiadic->u.diadic.right = repl;
    newdiadic->rtype = promoted;
    IRO_AddToList(newdiadic, &list);
    if (promoted != type) {
        newdiadic = IRO_AddTypeConversion(newdiadic, type, &list);
    }
    IRO_LocateFather_Cut_And_Paste_Without_Nopping(a, newdiadic);
    IRO_PasteAfter(list.head, list.tail, a);
}
