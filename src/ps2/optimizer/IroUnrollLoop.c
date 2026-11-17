#include "compiler/CParser.h"
#include "compiler/IroUnrollLoop.h"
#include "compiler/InstrSelection.h"
#include "compiler/IroDump.h"
#include "compiler/IroCodePositioning.h"

#include "compiler/BitVector.h"
#include "compiler/CError.h"
#include "compiler/CFunc.h"
#include "compiler/CInt64.h"
#include "compiler/enode.h"
#include "compiler/IrOptimizer.h"
#include "compiler/IroFlowgraph.h"
#include "compiler/IroLinearForm.h"
#include "compiler/IroLoop.h"
#include "compiler/IroType.h"
#include "compiler/IroUtil.h"
#include "compiler/IroVars.h"
#include "compiler/types.h"

static Boolean fn_00573BC0(IROLoop* loop, CInt64* count, void* out);

static Boolean fn_00573FD0(
    IRONode* entry,
    IROLoop* loop,
    IROLoopInd* induction,
    CInt64* count,
    SInt32* matchCount,
    SInt32* bitsCount,
    void* pattern
);

void fn_00570810(IROLinear* first, IROLinear* last, CInt64 val, IROLoop* loop) {
    IROLinear* nd;
    IROLinear* value;
    IROLinear* add;
    Boolean isUnsigned;
    IROLinear* father;
    Boolean typconWidens;
    IROLinear* father2;
    IROLinear* father3;
    Type* fatherType;
    UInt32 foldedIntoAdd;
    Object* varobj;
    IROLinear* next;
    ENode* expr;
    Type* type;
    CInt64 scale;
    CInt64 product;
    IROList list;

    type = IRO_CopyType(loop->induction->nd->rtype);
    isUnsigned = is_unsigned(type);
    for (nd = first; nd; nd = next) {
        next = nd->next;
        varobj = IRO_IsVariable(nd);
        if (varobj && loop->induction->var->object == varobj) {
            add = IRO_NewLinear(IROLinearOp2Arg);
            add->index = ++IRO_NumLinear;
            add->nodetype = EADD;
            add->rtype = type;

            father = IRO_LocateFather(nd);
            typconWidens = TRUE;
            if (father && father->type == IROLinearOp1Arg && father->nodetype == ETYPCON) {
                fatherType = IRO_CopyType(father->rtype);
                father = IRO_LocateFather(father);
                if (fatherType->type != nd->rtype->type || fatherType->size < nd->rtype->size) {
                    typconWidens = FALSE;
                }
            }
            value = IRO_NewLinear(IROLinearOperand);
            value->index = ++IRO_NumLinear;
            value->rtype = type;
            expr = IRO_NewENode(EINTCONST);
            expr->rtype = type;
            expr->data.intval = val;
            value->u.node = expr;

            foldedIntoAdd = 0;
            if (typconWidens &&
                father &&
                father->type == IROLinearOp2Arg &&
                (father->nodetype == ESHL || father->nodetype == EMUL) &&
                IRO_IsIntConstant(father->u.diadic.right) &&
                (father2 = IRO_LocateFather(father)) &&
                father2->type == IROLinearOp2Arg &&
                father2->nodetype == EADD &&
                father2->u.diadic.right == father &&
                (father3 = IRO_LocateFather(father2))) {
                IRO_InitList(&list);
                scale = father->u.diadic.right->u.node->data.intval;
                if (father->nodetype == ESHL) {
                    scale = CInt64_Shl(cint64_one, scale);
                }
                product = value->u.node->data.intval;
                if (isUnsigned) {
                    product = CInt64_MulU(scale, product);
                } else {
                    product = CInt64_Mul(scale, product);
                }
                value->u.node->data.intval = product;
                IRO_AddToList(value, &list);
                IRO_AddToList(add, &list);
                add->u.diadic.right = value;
                IRO_Paste(list.head, list.tail, father3);
                IRO_LocateFather_Cut_And_Paste_Without_Nopping(father2, add);
                add->u.diadic.left = father2;
                add->rtype = father2->rtype;
                foldedIntoAdd = 1;
            }
            if (!foldedIntoAdd) {
                add->u.diadic.right = value;
                add->u.diadic.right->flags |= IROLF_Reffed;
                value->next = add;
                add->u.diadic.left = nd;
                IRO_LocateFather_Cut_And_Paste_Without_Nopping(nd, add);
                add->flags |= IROLF_Reffed;
                nd->next = value;
                add->next = next;
            }
        }
        if (nd == last) {
            break;
        }
    }
}

static void fn_00570690(IROLoop* loop, IROLinear* before, CInt64* count, Boolean adjust);
static void fn_00570510(IROLoop* loop, IRONode* lastnode, CInt64 count) {
    IRONode* newnode;
    IROLinear* linear;
    volatile CInt64 zero;
    CInt64 i;

    newnode = IRO_NewFlowGraphNode();
    linear = IRO_NewLinear(IROLinearNop);
    linear->index = IRO_NumLinear++;
    linear->next = NULL;
    linear->flags |= 1;
    newnode->last = linear;
    newnode->first = newnode->last;
    newnode->nextnode = lastnode->nextnode;
    lastnode->nextnode = newnode;
    linear->next = lastnode->last->next;
    lastnode->last->next = linear;

    zero = cint64_zero;
    i = cint64_zero;
    if (CInt64_Less(*(CInt64*)&zero, count)) {
        do {
            fn_00570690(loop, newnode->last, &i, !CInt64_IsZero(&i));
            i = CInt64_Add(i, cint64_one);
        } while (CInt64_Less(i, count));
    }
    IRO_UpdateInductionIncrement(loop, count, newnode->last);
}

static void fn_00570690(IROLoop* loop, IROLinear* before, CInt64* count, Boolean adjust) {
    IRONode* fnode;
    IRONode* parent;
    IROLinear* scan;
    IROLinear* end;
    IROLinear* head;
    IROLinear* tail;
    IROList copy;
    IROLinear* old;

    head = NULL;
    tail = NULL;
    fnode = loop->unk20;
    while (fnode && fnode != loop->unk10->nextnode) {
        IRO_InitList(&copy);
        end = fnode->last->next;
        scan = fnode->first;
        while (scan != end) {
            if (!IRO_IsLinearNodeInLoopTest(loop, scan)) {
                if (scan->stmt) {
                    scan->stmt->flags |= 0x10;
                }
                if (!IRO_IsLinearNodeInInductionUpdate(loop, scan) &&
                    scan->type != IROLinearLabel &&
                    !(scan->flags & 2)) {
                    IRO_DuplicateExpr(scan, &copy);
                    if (!head) {
                        head = copy.head;
                    }
                    tail = copy.tail;
                }
            }
            scan = scan->next;
        }
        if (copy.head && copy.tail) {
            IRO_Paste(copy.head, copy.tail, before);
        }
        fnode = fnode->nextnode;
    }
    if (adjust) {
        *count = CInt64_Add(*count, IRO_GetAdjustedInductionIncrement(loop->induction));
        fn_00570810(head, tail, *count, loop);
    }
}

typedef struct UnrollPattern {
    IROLinear* indexExpr;
    IROLinear* baseExpr;
    Type* addrType;
    IROLinear* bitExpr;
    IROLinear* valueExpr;
    CInt64 bits;
    CInt64 shift;
} UnrollPattern;

static CInt64 make_unroll_int(SInt32 n) {
    CInt64 value;
    CInt64_SetLong(&value, n);
    return value;
}

void fn_00571950(
    IROLoop* loop,
    SInt32 vectorStride,
    CInt64* unrollFactor,
    SInt32* leftOver,
    UInt32* needOrigLoop,
    UInt32* needUnrollBodyTest,
    UInt32* resetUnrolledFinalValue
) {
    Boolean isUnsigned;
    CInt64 count;
    CInt64 val;
    CInt64 strideRemainder;
    CInt64 mod;
    CInt64 factor;
    CInt64 loopvar3;
    CInt64 loopvar1;
    CInt64 loopvar2;
    CInt64 strideVal;

    count = loop->count;
    if (!loop->countKnown || CInt64_Equal(count, cint64_negone)) {
        *needOrigLoop = TRUE;
        *needUnrollBodyTest = TRUE;
        *resetUnrolledFinalValue = FALSE;
        return;
    }
    isUnsigned = is_unsigned(IRO_CopyType(IRO_PrimaryInductionUpperBound(loop)->rtype));
    CError_ASSERT(1860, vectorStride);
    CInt64_SetLong(&strideVal, vectorStride);
    if (isUnsigned ? CInt64_LessU(count, strideVal) : CInt64_Less(count, strideVal)) {
        *needOrigLoop = TRUE;
        *needUnrollBodyTest = FALSE;
        *unrollFactor = cint64_zero;
        *leftOver = count.lo;
    } else {
        switch (vectorStride) {
            case 1:
                val = count;
                break;
            case 2:
                val = CInt64_ShrU(count, cint64_one);
                break;
            case 4:
                val = CInt64_ShrU(count, make_unroll_int(2));
                break;
            case 8:
                val = CInt64_ShrU(count, make_unroll_int(3));
                break;
            case 16:
                val = CInt64_ShrU(count, make_unroll_int(4));
                break;
            default:
                val = CInt64_Div(count, strideVal);
        }
        if (CInt64_LessU(val, *unrollFactor)) {
            *unrollFactor = val;
        }
        factor = *unrollFactor;
        switch (vectorStride) {
            case 1:
                strideRemainder = cint64_zero;
                break;
            case 2:
                strideRemainder = CInt64_And(count, cint64_one);
                break;
            case 4:
                strideRemainder = CInt64_And(count, make_unroll_int(3));
                break;
            case 8:
                strideRemainder = CInt64_And(count, make_unroll_int(7));
                break;
            case 16:
                strideRemainder = CInt64_And(count, make_unroll_int(15));
                break;
            default:
                strideRemainder = CInt64_Mod(count, strideVal);
        }
        if (CInt64_GreaterEqualU(make_unroll_int(8), val)) {
            *needUnrollBodyTest = vectorStride > 1;
            *unrollFactor = val;
            *leftOver = strideRemainder.lo;
            *needOrigLoop = *leftOver != 0;
            *resetUnrolledFinalValue = !(*needOrigLoop && *needUnrollBodyTest);
        } else {
            loopvar1 = make_unroll_int(0x7FFFFFFF);
            loopvar2 = make_unroll_int(0x7FFFFFFF);
            do {
                mod = CInt64_Mod(val, factor);
                loopvar3 = CInt64_Add(CInt64_Mul(mod, strideVal), strideRemainder);
                if (CInt64_Less(loopvar3, loopvar2)) {
                    loopvar2 = loopvar3;
                    loopvar1 = factor;
                }
                if (vectorStride > 1) {
                    break;
                }
                factor = CInt64_Add(factor, cint64_negone);
            } while (CInt64_GreaterEqualU(CInt64_Mul(factor, factor), val));
            *unrollFactor = loopvar1;
            *leftOver = loopvar2.lo;
            *needOrigLoop = *leftOver != 0;
            *needUnrollBodyTest = CInt64_Less(loopvar1, val) || vectorStride > 1;
            *resetUnrolledFinalValue = !(*needOrigLoop && *needUnrollBodyTest);
        }
    }
    IRO_Dump(
        "---- IterCount = %d, VectorStride = %d, UnrollFactor = %d, LeftOver = %d,\n\tNeedOrigLoop = %d, NeedUnrollBodyTest = %d, ResetUnrolledFinalValue = %d\n",
        count.lo,
        vectorStride,
        unrollFactor->lo,
        *leftOver,
        *needOrigLoop,
        *needUnrollBodyTest,
        *resetUnrolledFinalValue
    );
}

static void fn_00572DA0(IROLoop* loop, UnrollPattern* pattern) {
    IRONode* flow;
    IROLinear* scan;
    IROLinear* endnd;
    IROLinear* indexNd;
    IROLinear* offsetNd;
    IROLinear* baseNd;
    IROLinear* addressNd;
    IROLinear* targetNd;
    IROLinear* assignNd;
    IROLinear* valueNd;
    IROLinear* bitsNd;
    IROLinear* first;
    IROLinear* last;
    CInt64 zero;
    CInt64 scale;
    CInt64 offset;
    IROList list;
    SInt32 pass;
    UInt32 bits;

    CInt64_SetLong(&zero, 0);
    pass = 0;
    do {
        first = NULL;
        last = NULL;
        for (flow = loop->unk20; flow && flow != loop->unk10; flow = flow->nextnode) {
            IRO_InitList(&list);
            endnd = flow->last;
            scan = flow->first;
            while (1) {
                if (scan->stmt) {
                    scan->stmt->flags |= 0x10;
                }
                if (!IRO_IsLinearNodeInInductionUpdate(loop, scan) &&
                    scan->type != IROLinearLabel &&
                    scan->type != IROLinearNop &&
                    !(scan->flags & IROLF_Reffed)) {
                    CError_ASSERT(
                        1220,
                        scan->nodetype == EORASS ||
                            scan->nodetype == EANDASS ||
                            scan->nodetype == EXORASS
                    );
                    IRO_DuplicateExpr(pattern->indexExpr, &list);
                    indexNd = list.tail;
                    scale = CInt64_Shl(cint64_one, pattern->shift);
                    offsetNd = IRO_NewLinear(IROLinearOperand);
                    offsetNd->index = ++IRO_NumLinear;
                    offsetNd->rtype = pattern->indexExpr->rtype;
                    {
                        ENode* expr = IRO_NewENode(EINTCONST);
                        expr->rtype = pattern->indexExpr->rtype;
                        CInt64_SetLong(&expr->data.intval, pass * scale.lo);
                        offsetNd->u.node = expr;
                    }
                    IRO_AddToList(offsetNd, &list);
                    IRO_DuplicateExpr(pattern->baseExpr, &list);

                    baseNd = IRO_NewLinear(IROLinearOp2Arg);
                    baseNd->index = ++IRO_NumLinear;
                    baseNd->nodetype = EADD;
                    baseNd->rtype = IRO_CopyType(pattern->addrType);
                    baseNd->u.diadic.left = list.tail;
                    baseNd->u.diadic.right = offsetNd;
                    IRO_AddToList(baseNd, &list);

                    addressNd = IRO_NewLinear(IROLinearOp2Arg);
                    addressNd->index = ++IRO_NumLinear;
                    addressNd->nodetype = EADD;
                    addressNd->rtype = IRO_CopyType(pattern->addrType);
                    addressNd->u.diadic.left = baseNd;
                    addressNd->u.diadic.right = indexNd;
                    IRO_AddToList(addressNd, &list);

                    targetNd = IRO_NewLinear(IROLinearOp1Arg);
                    targetNd->index = ++IRO_NumLinear;
                    targetNd->nodetype = EINDIRECT;
                    targetNd->rtype = IRO_CopyType(scan->rtype);
                    targetNd->u.monadic = addressNd;
                    IRO_AddToList(targetNd, &list);

                    assignNd = IRO_NewLinear(IROLinearOp2Arg);
                    *assignNd = *scan;
                    assignNd->index = ++IRO_NumLinear;
                    assignNd->u.diadic.left = list.tail;
                    assignNd->next = NULL;

                    if (scan->nodetype == EANDASS && CInt64_Equal(pattern->bits, zero)) {
                        assignNd->nodetype = EASS;
                    } else if (scan->nodetype == EORASS && !pattern->bits.hi) {
                        bits = pattern->bits.lo;
                        if ((scan->rtype->size == 1 && bits == 0xFF) ||
                            (scan->rtype->size == 2 && bits == 0xFFFF) ||
                            (scan->rtype->size == 4 && bits == 0xFFFFFFFF)) {
                            assignNd->nodetype = EASS;
                        }
                    }
                    bitsNd = IRO_NewIntConst(pattern->bits, pattern->bitExpr->rtype);
                    IRO_AddToList(bitsNd, &list);

                    if (pattern->valueExpr->type == IROLinearOp1Arg &&
                        pattern->valueExpr->nodetype == ETYPCON) {
                        valueNd = IRO_NewLinear(IROLinearOp1Arg);
                        *valueNd = *pattern->valueExpr;
                        valueNd->index = ++IRO_NumLinear;
                        valueNd->u.monadic = bitsNd;
                        valueNd->next = NULL;
                        IRO_AddToList(valueNd, &list);
                    } else {
                        valueNd = bitsNd;
                    }
                    assignNd->u.diadic.right = valueNd;
                    IRO_AddToList(assignNd, &list);
                    if (!first) {
                        first = list.head;
                    }
                    last = list.tail;
                }
                if (scan == endnd) {
                    break;
                }
                scan = scan->next;
            }
            if (list.head && list.tail) {
                IRO_Paste(list.head, list.tail, loop->preheader->last);
            }
        }
    } while (++pass < 8);
}

static Boolean fn_00573FD0(
    IRONode* entry,
    IROLoop* loop,
    IROLoopInd* induction,
    CInt64* count,
    SInt32* matchCount,
    SInt32* bitsCount,
    void* output
) {
    UnrollPattern* pattern = (UnrollPattern*)output;
    IROLinear* scan;
    IROLinear* varnode;
    IROLinear* indexNd;
    IROLinear* maskNd;
    IROLinear* left1;
    IROLinear* left2;
    IROLinear* right1;
    IROLinear* right2;
    Object* obj1;
    Object* obj2;
    CInt64 shl;
    CInt64 val;
    CInt64 word;

    *matchCount = 0;
    *bitsCount = 0;
    scan = entry->first;
    while (scan) {
        if (!IRO_IsLinearNodeInInductionUpdate(loop, scan) &&
            !(scan->flags & IROLF_Reffed) &&
            scan->type != IROLinearNop &&
            scan->type != IROLinearLabel) {
            if (scan->type != IROLinearOp2Arg || (UInt8)(scan->nodetype - EANDASS) > 2) {
                return FALSE;
            }
            (*bitsCount)++;
            if (scan->u.diadic.left->type != IROLinearOp1Arg ||
                scan->u.diadic.left->nodetype != EINDIRECT) {
                return FALSE;
            }
            varnode = scan->u.diadic.left->u.monadic;
            if (varnode->type != IROLinearOp2Arg || varnode->nodetype != EADD) {
                return FALSE;
            }
            pattern->baseExpr = varnode->u.diadic.left;
            pattern->addrType = varnode->rtype;
            if (!IRO_IsVariable(varnode->u.diadic.left)) {
                return FALSE;
            }
            pattern->indexExpr = varnode->u.diadic.right;
            if (pattern->indexExpr->type != IROLinearOp2Arg ||
                pattern->indexExpr->nodetype != ESHL ||
                !IRO_IsConstant(pattern->indexExpr->u.diadic.right)) {
                return FALSE;
            }
            pattern->shift = pattern->indexExpr->u.diadic.right->u.node->data.intval;
            indexNd = pattern->indexExpr->u.diadic.left;
            pattern->valueExpr = scan->u.diadic.right;
            if (pattern->valueExpr->type == IROLinearOp1Arg &&
                pattern->valueExpr->nodetype == ETYPCON) {
                if (scan->nodetype == EANDASS) {
                    if (pattern->valueExpr->u.monadic->type != IROLinearOp1Arg ||
                        pattern->valueExpr->u.monadic->nodetype != EBINNOT) {
                        return FALSE;
                    }
                    pattern->bitExpr = pattern->valueExpr->u.monadic->u.monadic;
                } else {
                    pattern->bitExpr = pattern->valueExpr->u.monadic;
                }
                if (pattern->bitExpr->type != IROLinearOp2Arg ||
                    pattern->bitExpr->nodetype != ESHL ||
                    !IRO_IsConstant(pattern->bitExpr->u.diadic.left)) {
                    return FALSE;
                }
                val = pattern->bitExpr->u.diadic.left->u.node->data.intval;
                maskNd = pattern->bitExpr->u.diadic.right;
            } else if (pattern->valueExpr->type == IROLinearOp2Arg &&
                       pattern->valueExpr->nodetype == ESHL &&
                       (scan->nodetype == EORASS || scan->nodetype == EXORASS)) {
                pattern->bitExpr = pattern->valueExpr;
                if (!IRO_IsConstant(pattern->bitExpr->u.diadic.left)) {
                    return FALSE;
                }
                val = pattern->bitExpr->u.diadic.left->u.node->data.intval;
                maskNd = pattern->bitExpr->u.diadic.right;
            } else if (pattern->valueExpr->type == IROLinearOp1Arg &&
                       pattern->valueExpr->nodetype == EBINNOT &&
                       scan->nodetype == EANDASS) {
                pattern->bitExpr = pattern->valueExpr->u.monadic;
                if (pattern->bitExpr->type != IROLinearOp2Arg ||
                    pattern->bitExpr->nodetype != ESHL ||
                    !IRO_IsConstant(pattern->bitExpr->u.diadic.left)) {
                    return FALSE;
                }
                val = pattern->bitExpr->u.diadic.left->u.node->data.intval;
                maskNd = pattern->bitExpr->u.diadic.right;
            } else {
                return FALSE;
            }
            if (maskNd->type != IROLinearOp2Arg ||
                maskNd->nodetype != EAND ||
                indexNd->type != IROLinearOp2Arg ||
                indexNd->nodetype != ESHR) {
                return FALSE;
            }
            left1 = indexNd->u.diadic.left;
            left2 = maskNd->u.diadic.left;
            obj1 = IRO_IsVariable(left1);
            obj2 = IRO_IsVariable(left2);
            if (obj1 != obj2 || obj1 != induction->var->object) {
                return FALSE;
            }
            right1 = indexNd->u.diadic.right;
            right2 = maskNd->u.diadic.right;
            if (!IRO_IsConstant(right1) || !IRO_IsConstant(right2)) {
                return FALSE;
            }
            shl = CInt64_Shl(cint64_one, right1->u.node->data.intval);
            shl = CInt64_Sub(shl, cint64_one);
            if (!CInt64_Equal(shl, right2->u.node->data.intval)) {
                return FALSE;
            }
            CInt64_SetULong(&word, shl.lo);
            if (!CInt64_Equal(shl, word)) {
                return FALSE;
            }
            *count = CInt64_Add(shl, cint64_one);
            if (fn_00574620(CInt64_Add(shl, cint64_one), val, &pattern->bits)) {
                (*matchCount)++;
                if (scan->type == IROLinearOp2Arg && scan->nodetype == EANDASS) {
                    pattern->bits = CInt64_Inv(pattern->bits);
                }
            }
        }
        if (scan == entry->last) {
            break;
        }
        scan = scan->next;
    }
    return TRUE;
}

static Boolean fn_00573BC0(IROLoop* loop, CInt64* count, void* pattern) {
    IRONode* preheader;
    IROLinear* ind;
    IROLoopInd* candidate;
    IRONode* entry;
    SInt32 matches;
    SInt32 operations;
    Type* type;
    UInt32 flags;

    if (loop->unk10 != loop->unk14) {
        IRO_Dump("IsWhileLoopUnrollable:No because the loop form is currently unsupported\n");
        return FALSE;
    }
    preheader = loop->preheader;
    if (!preheader->last || preheader->last->type != IROLinearGoto) {
        IRO_Dump("IsWhileLoopUnrollable:No because the preheader does not end with a goto\n");
        return FALSE;
    }
    if (preheader->nextnode != loop->unk20) {
        IRO_Dump(
            "IsWhileLoopUnrollable:No because the preheader does not precede the InLoopHeaderTarget\n"
        );
        return FALSE;
    }
    if (loop->unk20 == loop->unk10) {
        IRO_Dump("IsWhileLoopUnrollable:No because the InLoopHeaderTarget is the loop header\n");
        return FALSE;
    }
    if (!is_unsigned(loop->cond->u.diadic.left->rtype)) {
        IRO_Dump("IsWhileLoopUnrollable:No because the while loop induction is signed\n");
        return FALSE;
    }
    type = loop->cond->u.diadic.left->rtype;
    if (type->type != TYPEINT || TYPE_INTEGRAL(type)->integral >= IT_17) {
        IRO_Dump("IsWhileLoopUnrollable:No because the while loop induction is not integral\n");
        return FALSE;
    }
    flags = loop->flags;
    if (!(flags & 0x1000)) {
        IRO_Dump(
            "IsWhileLoopUnrollable:No because the while loop operator is not of decrement form\n"
        );
        return FALSE;
    }
    if (flags & 0x20) {
        IRO_Dump("IsWhileLoopUnrollable: No because LP_IF_EXPR_NONCANONICAL \n");
        return FALSE;
    }
    if (flags & 0x400) {
        IRO_Dump("IsWhileLoopUnrollable: No because LP_IND_USED_IN_LOOP \n");
        return FALSE;
    }
    if (flags & 0x800) {
        IRO_Dump("IsWhileLoopUnrollable: No because LP_HAS_MULTIPLE_EXITS \n");
        return FALSE;
    }
    if (!(flags & 0x40)) {
        IRO_Dump("IsWhileLoopUnrollable: No because LP_HAS_MULTIPLE_INDUCTIONS is not set \n");
        return FALSE;
    }
    for (candidate = loop->extraInductions; candidate; candidate = candidate->next) {
        if ((candidate->flags & 1) && (candidate->flags & 2)) {
            break;
        }
    }
    if (!candidate) {
        IRO_Dump(
            "IsWhileLoopUnrollable: No because could not find induction with MOD and DIV operation\n"
        );
        return FALSE;
    }
    if (!is_unsigned(candidate->nd->rtype)) {
        IRO_Dump("IsWhileLoopUnrollable: No because induction is signed\n");
        return FALSE;
    }
    ind = candidate->nd;
    if (ind->type == IROLinearOp2Arg) {
        if (ind->nodetype == EADDASS) {
            if (!IRO_IsConstant(ind->u.diadic.right)) {
                IRO_Dump(
                    "IsWhileLoopUnrollable: No because induction EASS doesn't match pattern\n"
                );
                return FALSE;
            }
            if (!CInt64_IsOne(&candidate->step)) {
                IRO_Dump("IsWhileLoopUnrollable: No because induction step is not one\n");
                return FALSE;
            }
        } else if (ind->nodetype == EASS) {
            IROLinear* child = ind->u.diadic.right;
            if (child->type != IROLinearOp2Arg ||
                child->nodetype != EADD ||
                !IRO_IsConstant(child->u.diadic.right)) {
                IRO_Dump(
                    "IsWhileLoopUnrollable: No because induction EASS doesn't match pattern\n"
                );
                return FALSE;
            }
            if (!CInt64_IsOne(&candidate->step)) {
                IRO_Dump("IsWhileLoopUnrollable: No because induction step is not one\n");
                return FALSE;
            }
        } else {
            IRO_Dump("IsWhileLoopUnrollable: No because induction assignment operator is bad\n");
            return FALSE;
        }
    } else if (ind->type == IROLinearOp1Arg &&
               ind->nodetype != EPOSTINC &&
               ind->nodetype != EPREINC) {
        IRO_Dump("IsWhileLoopUnrollable: No because induction assignment operator is bad\n");
        return FALSE;
    }
    IRO_UnmarkPrimaryInductionAssignmentNodes(loop);
    loop->induction = candidate;
    IRO_MarkPrimaryInductionAssignmentNodes(loop);
    memset(pattern, 0, 0x34);
    entry = IRO_FirstNode;
    for (; entry; entry = entry->nextnode) {
        if (!Bv_IsBitSet(entry->index, loop->bitset) && entry != loop->unk14) {
            if (!fn_00573FD0(entry, loop, candidate, count, &matches, &operations, pattern)) {
                IRO_Dump(
                    "IsWhileLoopUnrollable: No because PatternMatchWhileLoop returned false\n"
                );
                return FALSE;
            }
        }
    }
    if (matches > 1 || operations > 1) {
        IRO_Dump("IsWhileLoopUnrollable: No because too many MOD/DIV operations\n");
        return FALSE;
    }
    return TRUE;
}

static Boolean fn_00573940(IROLoop* loop, CInt64* count, void* out);
static IROLinear* BuildOrigIterationCount_DoWhile(IROList* list, IROLoop* loop);

static void fn_00572360(IROLoop* loop, CInt64 count, void* out) {
    IRONode* preheader;
    IRONode* header;
    IROLinear* loopTest;
    IROLinear* lastLabelNode;
    IROLinear* earlyExit;
    IROLinear* origCount;
    IROLinear* prealign;
    IROLinear* unrolledFinal;
    IROLinear* newFinal;
    IROLinear* entryTest;
    IROLinear* labelNode;
    IROLinear* scan;
    IROLinear* finalNode;
    IROLinear* firstCopy;
    IROLinear* comparison;
    CLabel* earlyLabel;
    CLabel* exitLabel;
    IROList list;

    IRO_Dump("while(n--) loop \n");
    preheader = loop->preheader;
    header = loop->unk10;
    loopTest = preheader->last;
    lastLabelNode = IRO_FindLabelNode((CLabel*)loopTest->u.monadic, loopTest);
    IRO_InitList(&list);
    IRO_DuplicateExprRange(lastLabelNode->next, loop->cond->u.monadic, &list);
    IRO_DuplicateExpr(loop->cond, &list);
    IRO_Paste(list.head, list.tail, preheader->last);
    lastLabelNode = list.tail;

    IRO_InitList(&list);
    earlyExit = IRO_NewLinear(IROLinearNop);
    earlyExit->index = ++IRO_NumLinear;
    if (header->last->type == IROLinearIf) {
        earlyExit->type = IROLinearIfNot;
    } else {
        earlyExit->type = IROLinearIf;
    }
    IRO_AddToList(earlyExit, &list);
    earlyLabel = IRO_NewLabel();
    earlyExit->u.diadic.left = (IROLinear*)earlyLabel;
    earlyExit->u.diadic.right = lastLabelNode;
    lastLabelNode->flags |= 2;
    earlyExit->rtype = header->last->rtype;
    IRO_Paste(list.head, list.tail, preheader->last);

    IRO_InitList(&list);
    origCount = BuildOrigIterationCount_DoWhile(&list, loop);
    IRO_Paste(list.head, list.tail, preheader->last);
    firstCopy = list.head;

    IRO_InitList(&list);
    prealign = fn_00573640(loop->induction, count, &list);
    IRO_Paste(list.head, list.tail, preheader->last);

    IRO_InitList(&list);
    unrolledFinal = fn_00573420(origCount, count, &list, loop);
    IRO_Paste(list.head, list.tail, preheader->last);

    IRO_InitList(&list);
    newFinal = fn_005732A0(origCount, count, &list, loop);
    IRO_Paste(list.head, list.tail, preheader->last);

    IRO_InitList(&list);
    comparison = IRO_NewLinear(IROLinearOp2Arg);
    comparison->index = ++IRO_NumLinear;
    comparison->nodetype = EGREATER;
    comparison->u.diadic.left = IRO_DuplicateExpr(origCount, &list);
    comparison->u.diadic.right = IRO_NewIntConst(count, origCount->rtype);
    IRO_AddToList(comparison->u.diadic.right, &list);
    comparison->u.diadic.right->flags |= 2;
    comparison->rtype = origCount->rtype;
    IRO_AddToList(comparison, &list);
    entryTest = IRO_NewLinear(IROLinearIfNot);
    entryTest->index = ++IRO_NumLinear;
    entryTest->u.diadic.left = (IROLinear*)earlyLabel;
    entryTest->u.diadic.right = comparison;
    entryTest->rtype = origCount->rtype;
    IRO_AddToList(entryTest, &list);
    IRO_Paste(list.head, list.tail, preheader->last);

    IRO_InitList(&list);
    exitLabel = IRO_NewLabel();
    labelNode = IRO_NewLinear(IROLinearLabel);
    labelNode->index = IRO_NumLinear++;
    labelNode->u.monadic = (IROLinear*)exitLabel;
    IRO_AddToList(labelNode, &list);
    IRO_Paste(list.head, list.tail, preheader->last);

    fn_00572DA0(loop, (UnrollPattern*)out);
}

static void RelinearizeExpr(IROLinear* expr, IROList* list); //! @reconstructed

static void fn_0056F6F0(IROLoop* loop, CInt64 count) {
    Boolean hasSpecialHeader;
    Boolean countValid;
    UInt32 needOriginal;
    UInt32 needBodyTest;
    UInt32 resetFinal;
    IRONode* preheader;
    IRONode* inLoop;
    IRONode* newnode;
    IROLinear* origCount;
    IROLinear* newFinal;
    IROLinear* bodyEntryTest;
    IROLinear* entryLabelNode;
    IROLinear* branch;
    IROLinear* test;
    CLabel* exitLabel;
    CInt64 factor;
    CInt64 adjusted;
    CInt64 offset;
    CInt64 leftover;
    SInt32 leftOver;
    IROList list;

    hasSpecialHeader = TRUE;
    if (loop->unk10 == loop->unk14) {
        hasSpecialHeader = loop->unk10 == loop->unk20;
    }
    countValid = loop->countKnown;
    if (countValid) {
        countValid = !CInt64_Equal(loop->count, cint64_negone);
    }
    if (countValid && hasSpecialHeader) {
        adjusted = CInt64_Sub(loop->count, cint64_one);
        loop->count = adjusted;
    }

    if (unroll_count_callback) {
        unroll_count_callback(loop, 1, &factor);
    } else {
        fn_00571950(loop, 1, &factor, &leftOver, &needOriginal, &needBodyTest, &resetFinal);
    }
    if (CInt64_IsZero(&factor)) {
        IRO_Dump("LoopUnroll: zero unroll factor\n");
        return;
    }
    if (!loop->countKnown) {
        factor = CInt64_MulU(loop->count, factor);
    }
    if (loop->countKnown) {
        offset = CInt64_Mod(loop->count, factor);
        needOriginal = !CInt64_IsZero(&offset);
        needBodyTest = CInt64_LessU(factor, loop->count);
    } else {
        offset = cint64_zero;
        needOriginal = TRUE;
        needBodyTest = TRUE;
    }
    resetFinal = !(needOriginal && needBodyTest);
    IRO_Dump(
        "LoopUnroll: unroll factor %d, original %d, body test %d, reset %d\n",
        factor.lo,
        needOriginal,
        needBodyTest,
        resetFinal
    );

    preheader = NULL;
    inLoop = NULL;
    IRO_FindUniqueSuccessorNode(loop, &preheader, &inLoop);
    if (!preheader) {
        IRO_Dump("LoopUnroll: no unique successor\n");
        return;
    }
    CInt64_SetLong(&leftover, 0);
    if (!IRO_CopyLoopTestToLoopPreheaderAndBranchToSuccNode(
            loop, &preheader, inLoop, &leftover, 0
        )) {
        return;
    }
    IRO_InitList(&list);
    origCount = IRO_BuildOrigIterationCount(&list, loop, cint64_zero);
    IRO_Paste(list.head, list.tail, preheader->last);

    IRO_InitList(&list);
    newFinal = fn_00571120(origCount, factor, cint64_one, &list, loop);
    IRO_Paste(list.head, list.tail, preheader->last);

    IRO_InitList(&list);
    exitLabel = IRO_NewLabel();
    IRO_BuildUnrolledBodyEntryTest(loop, &list, origCount, factor, exitLabel);
    IRO_Paste(list.head, list.tail, preheader->last);
    bodyEntryTest = list.tail;

    IRO_InitList(&list);
    entryLabelNode = IRO_NewLabelLinearNode(&list);
    IRO_AddToList(entryLabelNode, &list);
    IRO_Paste(list.head, list.tail, preheader->last);

    if (needBodyTest) {
        test = IRO_DuplicateExpr(loop->cond, &list);
        branch = IRO_NewLinear(IROLinearIfNot);
        branch->index = ++IRO_NumLinear;
        branch->u.diadic.left = (IROLinear*)exitLabel;
        branch->u.diadic.right = test;
        branch->rtype = loop->cond->rtype;
        IRO_AddToList(branch, &list);
        IRO_Paste(list.head, list.tail, preheader->last);
    }
    newnode = IRO_NewFlowGraphNode();
    newnode->first = entryLabelNode;
    newnode->last = bodyEntryTest;
    newnode->nextnode = preheader->nextnode;
    preheader->nextnode = newnode;
    if (factor.lo) {
        fn_00570510(loop, newnode, factor);
    }
    if (resetFinal) {
        IRO_NopOut(newFinal);
    }
}

static Boolean fn_00573940(IROLoop* loop, CInt64* count, void* out) {
    UInt32 flags;
    IROLinear* bound;
    IROLinear* initial;
    IROLinear* header;
    int unique;

    flags = loop->flags;
    if (flags & 0x244016) {
        IRO_Dump("IsLoopUnrollable:No due to prohibited loop flags that are set ");
        IRO_DumpLoopFlags(flags & 0x244016);
        IRO_Dump("\n");
        return FALSE;
    }
    if (!(flags & 0x100)) {
        IRO_Dump("IsLoopUnrollable:No due to required loop flags that are not set ");
        IRO_DumpLoopFlags((flags & 0x100) ^ 0x100);
        IRO_Dump("\n");
        return FALSE;
    }
    unique = (loop->unk10 == loop->unk14 || loop->unk10 == loop->unk20);
    if (!unique) {
        IRO_Dump("IsLoopUnrollable:No because the loop form is currently unsupported\n");
        return FALSE;
    }
    if (!loop->preheader) {
        IRO_Dump("IsLoopUnrollable:No because a unique preheader could not be found\n");
        return FALSE;
    }
    header = loop->unk14->first;
    if (header->flags & 0x10000) {
        IRO_Dump("IsLoopUnrollable:No because the loop has already been vectorized\n");
        return FALSE;
    }
    if (loop->weight > unrollinstrfactor) {
        IRO_Dump("IsLoopUnrollable:No because loop size greater than threshold\n");
        return FALSE;
    }
    if (flags & 0x8000) {
        return fn_00573BC0(loop, count, out);
    }
    bound = IRO_PrimaryInductionUpperBound(loop);
    if (!IRO_IsIntConstant(bound) && !(bound->flags & 0x100)) {
        IRO_Dump("IsLoopUnrollable:No because Loop Upper Bound is Variant in the loop\n");
        return FALSE;
    }
    if (IRO_HasSideEffect(bound)) {
        IRO_Dump("IsLoopUnrollable:No because Loop Upper Bound is has a side effect\n");
        return FALSE;
    }
    if (!loop->induction->init) {
        IRO_Dump(
            "IsLoopUnrollable:No because there is no initialization of loop index in PreHeader\n"
        );
        return FALSE;
    }
    initial = loop->induction->init->u.monadic;
    if (!IRO_IsVariable(initial)) {
        IRO_Dump("IsLoopUnrollable:No because initial value of induction stored through pointer\n");
        return FALSE;
    }
    if (IRO_HasSideEffect(IRO_InductionLowerBound(loop, loop->induction))) {
        IRO_Dump("IsLoopUnrollable:No because Loop lower bound is has a side effect\n");
        return FALSE;
    }
    if (!loop->countKnown) {
        IRO_Dump("IsLoopUnrollable:No because can't figure out an iteration count\n");
        return FALSE;
    }
    if (flags & 0x20) {
        if (loop->cond &&
            loop->cond->type == IROLinearOp2Arg &&
            loop->cond->nodetype == ENOTEQU &&
            loop->countKnown &&
            CInt64_Equal(loop->count, cint64_negone)) {
            return TRUE;
        }
        IRO_Dump("IsLoopUnrollable:No because loop test operator is unsupported\n");
        return FALSE;
    }
    return TRUE;
}

Boolean fn_00574620(CInt64 a, CInt64 b, CInt64* result) {
    CInt64 shifted = cint64_zero;
    CInt64 accumulated = cint64_zero;
    CInt64 overlap = cint64_zero;
    CInt64 i;

    for (i = cint64_zero; CInt64_Less(i, a); i = CInt64_Add(i, cint64_one)) {
        shifted = CInt64_Shl(b, i);
        overlap = CInt64_And(shifted, accumulated);
        if (CInt64_NotEqual(overlap, cint64_zero)) {
            return FALSE;
        } else {
            accumulated = CInt64_Or(shifted, accumulated);
        }
    }
    *result = accumulated;
    return TRUE;
}

IROLinear* fn_005732A0(IROLinear* iterCount, CInt64 factor, IROList* list, IROLoop* loop) {
    Type* type;
    IROLinear* mul;
    IROLinear* step;
    IROLinear* add;
    Object* temp;
    IROLinear* assignment;
    IROLinear* operand;

    type = IRO_CopyType(iterCount->rtype);
    step = IRO_NewIntConst(loop->induction->step, type);
    IRO_AddToList(step, list);
    step->flags |= 2;

    mul = IRO_NewLinear(IROLinearOp2Arg);
    mul->index = ++IRO_NumLinear;
    mul->nodetype = EMUL;
    mul->u.diadic.left = IRO_DuplicateExpr(iterCount, list);
    mul->u.diadic.right = step;
    mul->rtype = type;
    IRO_AddToList(mul, list);
    mul->flags |= 2;
    mul->u.diadic.left->flags &= ~4;
    mul->u.diadic.left->u.monadic->flags &= ~4;

    operand = *(IROLinear* volatile*)&loop->induction->nd->u.monadic;
    IRO_DuplicateExpr(operand, list);
    list->tail->flags &= ~4;
    list->tail->u.monadic->flags &= ~4;

    add = IRO_NewLinear(IROLinearOp2Arg);
    add->index = ++IRO_NumLinear;
    add->nodetype = EADD;
    add->u.diadic.left = mul;
    add->u.diadic.right = list->tail;
    add->rtype = type;
    IRO_AddToList(add, list);
    add->flags |= 2;

    temp = create_temp_object(type);
    IRO_FindVar(temp, TRUE, TRUE);
    assignment = IRO_NewLinear(IROLinearOp2Arg);
    assignment->index = ++IRO_NumLinear;
    assignment->nodetype = EASS;
    assignment->u.diadic.left = IRO_TempReference(temp);
    IRO_AddToList(assignment->u.diadic.left->u.monadic, list);
    assignment->u.diadic.left->flags |= 0x26;
    assignment->u.diadic.left->u.monadic->flags |= 0x24;
    assignment->u.diadic.right = add;
    assignment->u.diadic.right->flags |= 2;
    assignment->rtype = type;
    IRO_AddToList(assignment, list);
    return assignment->u.diadic.left;
}

IROLinear* fn_00573420(IROLinear* iterCount, CInt64 factor, IROList* list, IROLoop* loop) {
    Type* type;
    IROLinear* factorStep;
    IROLinear* step;
    IROLinear* mul;
    IROLinear* sub;
    IROLinear* add;
    Object* temp;
    IROLinear* assignment;
    IROLinear* operand;

    type = IRO_CopyType(iterCount->rtype);
    factorStep = IRO_NewIntConst(CInt64_Mul(loop->induction->step, factor), type);
    IRO_AddToList(factorStep, list);
    factorStep->flags |= 2;
    step = IRO_NewIntConst(loop->induction->step, type);
    IRO_AddToList(step, list);
    step->flags |= 2;

    mul = IRO_NewLinear(IROLinearOp2Arg);
    mul->index = ++IRO_NumLinear;
    mul->nodetype = EMUL;
    mul->u.diadic.left = IRO_DuplicateExpr(iterCount, list);
    mul->u.diadic.right = step;
    mul->rtype = type;
    IRO_AddToList(mul, list);
    mul->flags |= 2;
    mul->u.diadic.left->flags &= ~4;
    mul->u.diadic.left->u.monadic->flags &= ~4;

    sub = IRO_NewLinear(IROLinearOp2Arg);
    sub->index = ++IRO_NumLinear;
    sub->nodetype = ESUB;
    sub->u.diadic.left = mul;
    sub->u.diadic.right = factorStep;
    sub->rtype = type;
    IRO_AddToList(sub, list);
    sub->flags |= 2;

    operand = *(IROLinear* volatile*)&loop->induction->nd->u.monadic;
    IRO_DuplicateExpr(operand, list);
    list->tail->flags &= ~4;
    list->tail->u.monadic->flags &= ~4;

    add = IRO_NewLinear(IROLinearOp2Arg);
    add->index = ++IRO_NumLinear;
    add->nodetype = EADD;
    add->u.diadic.left = sub;
    add->u.diadic.right = list->tail;
    add->rtype = type;
    IRO_AddToList(add, list);
    add->flags |= 2;

    temp = create_temp_object(type);
    IRO_FindVar(temp, TRUE, TRUE);
    assignment = IRO_NewLinear(IROLinearOp2Arg);
    assignment->index = ++IRO_NumLinear;
    assignment->nodetype = EASS;
    assignment->u.diadic.left = IRO_TempReference(temp);
    IRO_AddToList(assignment->u.diadic.left->u.monadic, list);
    assignment->u.diadic.left->flags |= 0x26;
    assignment->u.diadic.left->u.monadic->flags |= 0x24;
    assignment->u.diadic.right = add;
    assignment->u.diadic.right->flags |= 2;
    assignment->rtype = type;
    IRO_AddToList(assignment, list);
    return assignment->u.diadic.left;
}

IROLinear* fn_00573640(IROLoopInd* induction, CInt64 factorValue, IROList* list) {
    Type* type;
    IROLinear* inductionNode;
    IROLinear* operand;
    IROLinear* factor;
    IROLinear* constant;
    IROLinear* division;
    IROLinear* sum;
    IROLinear* product;
    IROLinear* assignment;
    Object* temp;

    inductionNode = induction->nd;
    type = IRO_CopyType(inductionNode->rtype);
    factor = IRO_NewIntConst(factorValue, type);
    IRO_AddToList(factor, list);
    operand = *(IROLinear* volatile*)&inductionNode->u.monadic;
    IRO_DuplicateExpr(operand, list);
    list->tail->flags &= ~4;
    list->tail->u.monadic->flags &= ~4;

    division = IRO_NewLinear(IROLinearOp2Arg);
    division->index = ++IRO_NumLinear;
    division->nodetype = EDIV;
    division->u.diadic.left = list->tail;
    division->u.diadic.right = factor;
    division->rtype = type;
    IRO_AddToList(division, list);
    division->flags |= 2;

    constant = IRO_NewIntConst(cint64_one, type);
    IRO_AddToList(constant, list);
    constant->flags |= 2;

    sum = IRO_NewLinear(IROLinearOp2Arg);
    sum->index = ++IRO_NumLinear;
    sum->nodetype = EADD;
    sum->u.diadic.left = division;
    sum->u.diadic.right = constant;
    sum->rtype = type;
    IRO_AddToList(sum, list);
    sum->flags |= 2;

    IRO_DuplicateExpr(factor, list);
    product = IRO_NewLinear(IROLinearOp2Arg);
    product->index = ++IRO_NumLinear;
    product->nodetype = EMUL;
    product->u.diadic.left = sum;
    product->u.diadic.right = list->tail;
    product->rtype = type;
    IRO_AddToList(product, list);
    product->flags |= 2;

    temp = create_temp_object(type);
    IRO_FindVar(temp, TRUE, TRUE);
    assignment = IRO_NewLinear(IROLinearOp2Arg);
    assignment->index = ++IRO_NumLinear;
    assignment->nodetype = EASS;
    assignment->u.diadic.left = IRO_TempReference(temp);
    IRO_AddToList(assignment->u.diadic.left->u.monadic, list);
    assignment->u.diadic.left->flags |= 0x26;
    assignment->u.diadic.left->u.monadic->flags |= 0x24;
    assignment->u.diadic.right = product;
    assignment->u.diadic.right->flags |= 2;
    assignment->rtype = type;
    IRO_AddToList(assignment, list);
    return assignment->u.diadic.left;
}

static IROLinear* BuildOrigIterationCount_DoWhile(IROList* list, IROLoop* loop) {
    IROLinear* count;
    IROLinear* finalCount;
    IROLinear* assignment;
    Type* type;
    Object* temp;

    type = IRO_CopyType(loop->cond->u.diadic.left->rtype);
    count = IRO_NewIntConst(cint64_one, type);
    IRO_AddToList(count, list);
    count->flags |= IROLF_Reffed;

    finalCount = IRO_NewLinear(IROLinearOp2Arg);
    finalCount->index = ++IRO_NumLinear;
    finalCount->nodetype = EADD;
    finalCount->rtype = type;
    finalCount->u.diadic.left = IRO_DuplicateExpr(loop->cond->u.diadic.left, list);
    finalCount->u.diadic.left->flags |= IROLF_Reffed;
    finalCount->u.diadic.left->flags &= ~4;
    finalCount->u.diadic.left->u.monadic->flags &= ~4;
    finalCount->u.diadic.right = count;
    IRO_AddToList(finalCount, list);

    temp = create_temp_object(type);
    IRO_FindVar(temp, 1, 1);
    assignment = IRO_NewLinear(IROLinearOp2Arg);
    assignment->index = ++IRO_NumLinear;
    assignment->nodetype = EASS;
    assignment->u.diadic.left = IRO_TempReference(temp);
    IRO_AddToList(assignment->u.diadic.left->u.monadic, list);
    assignment->u.diadic.left->flags |= 0x26;
    assignment->u.diadic.left->u.monadic->flags |= 0x24;
    assignment->u.diadic.right = finalCount;
    assignment->rtype = type;
    IRO_AddToList(assignment, list);
    return assignment->u.diadic.left;
}

IROLinear* fn_00571120(
    IROLinear* original, CInt64 firstFactor, CInt64 otherFactor, IROList* list, IROLoop* loop
) {
    Type* type;
    IROLinear* limit;
    IROLinear* constant;
    IROLinear* difference;
    IROLinear* assignment;
    Object* temp;

    type = IRO_CopyType(IRO_PrimaryInductionUpperBound(loop)->rtype);
    if (loop->cond->nodetype != ENOTEQU) {
        if (type->type == TYPEPOINTER) {
            type = IRO_GetPointerType(type);
        }
        constant = IRO_NewIntConst(
            CInt64_Mul(IRO_GetAdjustedInductionIncrement(loop->induction), firstFactor), type
        );
        IRO_AddToList(constant, list);
        limit = IRO_DuplicateExpr(IRO_PrimaryInductionUpperBound(loop), list);
        difference = IRO_NewLinear(IROLinearOp2Arg);
        difference->index = ++IRO_NumLinear;
        difference->nodetype = ESUB;
        difference->u.diadic.left = limit;
        difference->u.diadic.right = constant;
    } else {
        if (type->type == TYPEPOINTER) {
            type = IRO_GetPointerType(type);
        }
        constant = IRO_NewIntConst(
            CInt64_Mul(IRO_GetAdjustedInductionIncrement(loop->induction), otherFactor), type
        );
        IRO_AddToList(constant, list);
        limit = IRO_DuplicateExpr(IRO_PrimaryInductionUpperBound(loop), list);
        difference = IRO_NewLinear(IROLinearOp2Arg);
        difference->index = ++IRO_NumLinear;
        difference->nodetype = ESUB;
        difference->u.diadic.left = limit;
        difference->u.diadic.right = constant;
    }
    difference->rtype = type;
    IRO_AddToList(difference, list);
    IRO_FoldConstantsInExpr(difference);
    temp = create_temp_object(type);
    IRO_FindVar(temp, TRUE, TRUE);
    assignment = IRO_NewLinear(IROLinearOp2Arg);
    assignment->index = ++IRO_NumLinear;
    assignment->nodetype = EASS;
    assignment->u.diadic.left = IRO_TempReference(temp);
    IRO_AddToList(assignment->u.diadic.left->u.monadic, list);
    assignment->u.diadic.left->flags |= 0x26;
    assignment->u.diadic.left->u.monadic->flags |= 0x24;
    assignment->u.diadic.right = difference;
    assignment->u.diadic.right->flags |= 2;
    assignment->rtype = type;
    IRO_AddToList(assignment, list);
    return assignment->u.diadic.left;
}

IROLinear* IRO_BuildOrigIterationCount(IROList* list, IROLoop* loop, CInt64 offset) {
    IROLinear* assignment;
    SInt32 shift;
    Boolean unitStart;
    Boolean isPointer;
    IROLinear* start;
    IROLinear* limit;
    IROLinear* sum;
    IROLinear* step;
    IROLinear* stepValue;
    IROLinear* constant;
    Type* type;
    Object* temp;
    IROLinear* lessSum;
    IROLinear* lessConstant;
    IROLinear* greaterConstant;
    IROLinear* greaterSum;
    IROLinear* countSum;
    IROLinear* count;
    IROLinear* difference;

    start = IRO_InductionLowerBound(loop, loop->induction);
    limit = IRO_PrimaryInductionUpperBound(loop);
    type = IRO_CopyType(limit->rtype);
    isPointer = limit->rtype->type == TYPEPOINTER && start->rtype->type == TYPEPOINTER;

    if (loop->countKnown) {
        if (CInt64_Equal(loop->count, cint64_negone)) {
            IRO_DuplicateExpr(loop->countExpr.tail, list);
            count = list->tail;
        } else {
            if (isPointer) {
                count = IRO_NewIntConst(loop->count, IRO_GetPtrDiffTType());
            } else {
                count = IRO_NewIntConst(loop->count, type);
            }
            IRO_AddToList(count, list);
        }
    } else {
        unitStart = IRO_IsIntConstant(start) &&
            CInt64_Equal(start->u.node->data.intval, CInt64_Neg(offset));
        if (!unitStart) {
            start = IRO_DuplicateExpr(start, list);
            if (!CInt64_IsZero(&offset)) {
                constant = IRO_NewIntConst(
                    offset,
                    start->rtype->type == TYPEPOINTER ? IRO_GetPointerType(start->rtype)
                                                      : start->rtype
                );
                IRO_AddToList(constant, list);
                sum = IRO_NewLinear(IROLinearOp2Arg);
                sum->index = ++IRO_NumLinear;
                sum->nodetype = EADD;
                sum->u.diadic.left = start;
                sum->u.diadic.right = constant;
                sum->rtype = start->rtype;
                IRO_AddToList(sum, list);
                start = sum;
            }
        }

        limit = IRO_DuplicateExpr(limit, list);
        CError_ASSERT(2077, loop->induction != NULL);
        CError_ASSERT(2082, !CInt64_IsZero(&loop->induction->step));

        if (unitStart) {
            difference = limit;
            if (isPointer) {
                limit->rtype = IRO_GetPtrDiffTType();
            }
        } else {
            difference = IRO_NewLinear(IROLinearOp2Arg);
            difference->index = ++IRO_NumLinear;
            difference->nodetype = ESUB;
            difference->u.diadic.left = limit;
            difference->u.diadic.right = start;
            if (isPointer) {
                difference->rtype = IRO_GetPtrDiffTType();
            } else {
                difference->rtype = type;
            }
            IRO_AddToList(difference, list);
        }

        stepValue =
            IRO_NewIntConst(IRO_GetAdjustedInductionIncrement(loop->induction), difference->rtype);
        IRO_AddToList(stepValue, list);
        countSum = IRO_NewLinear(IROLinearOp2Arg);
        countSum->index = ++IRO_NumLinear;
        countSum->nodetype = EADD;
        countSum->u.diadic.left = difference;
        countSum->u.diadic.right = stepValue;
        countSum->rtype = IRO_CopyType(difference->rtype);
        IRO_AddToList(countSum, list);

        if (loop->cond->type == IROLinearOp2Arg && loop->cond->nodetype == ELESS) {
            lessSum = countSum;
            lessConstant = IRO_NewIntConst(cint64_negone, countSum->rtype);
            IRO_AddToList(lessConstant, list);
            countSum = IRO_NewLinear(IROLinearOp2Arg);
            countSum->index = ++IRO_NumLinear;
            countSum->nodetype = EADD;
            countSum->u.diadic.left = lessSum;
            countSum->u.diadic.right = lessConstant;
            countSum->rtype = IRO_CopyType(lessSum->rtype);
            IRO_AddToList(countSum, list);
        } else if (loop->cond->type == IROLinearOp2Arg && loop->cond->nodetype == EGREATER) {
            greaterSum = countSum;
            greaterConstant = IRO_NewIntConst(cint64_one, countSum->rtype);
            IRO_AddToList(greaterConstant, list);
            countSum = IRO_NewLinear(IROLinearOp2Arg);
            countSum->index = ++IRO_NumLinear;
            countSum->nodetype = EADD;
            countSum->u.diadic.left = greaterSum;
            countSum->u.diadic.right = greaterConstant;
            countSum->rtype = IRO_CopyType(greaterSum->rtype);
            IRO_AddToList(countSum, list);
        }

        step = IRO_NewIntConst(IRO_GetAdjustedInductionIncrement(loop->induction), countSum->rtype);
        if (CInt64_Equal(step->u.node->data.intval, cint64_one)) {
            count = countSum;
        } else if (IS_TYPE_INT(step->rtype) &&
                   TYPE_INTEGRAL(step->rtype)->integral < 0x11 &&
                   IRO_IsPowerOfTwo(step, &shift) &&
                   shift > 0) {
            count = IRO_NewLinear(IROLinearOp2Arg);
            count->index = ++IRO_NumLinear;
            count->nodetype = ESHR;
            count->u.diadic.left = countSum;
            count->u.diadic.right = step;
            CInt64_SetLong(&step->u.node->data.intval, shift);
            count->rtype = countSum->rtype;
            IRO_AddToList(step, list);
            IRO_AddToList(count, list);
        } else {
            count = IRO_NewLinear(IROLinearOp2Arg);
            count->index = ++IRO_NumLinear;
            count->nodetype = EDIV;
            count->u.diadic.left = countSum;
            count->u.diadic.right = step;
            count->rtype = countSum->rtype;
            IRO_AddToList(step, list);
            IRO_AddToList(count, list);
        }
    }

    IRO_FoldConstantsInExpr(count);
    temp = create_temp_object(count->rtype);
    IRO_FindVar(temp, 1, 1);
    assignment = IRO_NewLinear(IROLinearOp2Arg);
    assignment->index = ++IRO_NumLinear;
    assignment->nodetype = EASS;
    assignment->u.diadic.left = IRO_TempReference(temp);
    IRO_AddToList(assignment->u.diadic.left->u.monadic, list);
    assignment->u.diadic.left->flags |= 0x26;
    assignment->u.diadic.left->u.monadic->flags |= 0x24;
    assignment->u.diadic.right = count;
    assignment->u.diadic.right->flags |= 2;
    assignment->rtype = count->rtype;
    IRO_AddToList(assignment, list);
    return assignment->u.diadic.left;
}

//! @reconstructed
static void UnrollLoop(IROLoop* loop) {
    CInt64 tmp;
    CInt64 count;
    UInt8 out[0x34];

    fn_0059DCF0(loop);
    if (!(loop->flags & 0x8000)) {
        fn_0059A290(loop, 1);
    }
    IRO_MarkPrimaryInductionAssignmentNodes(loop);
    IRO_MarkLoopTestNodes(loop);
    IRO_FindUniqueInLoopHeaderTarget(loop);
    CInt64_SetLong(&tmp, copts.unrollfactor);
    count = tmp;
    if (fn_00573940(loop, &count, out)) {
        if (loop->flags & 0x8000) {
            fn_00572360(loop, count, out);
        } else {
            fn_0056F6F0(loop, count);
        }
        IRO_BuildNodeTable();
        fn_0058C9B0();
        IRO_ComputeDom();
    }
    IRO_UnmarkPrimaryInductionAssignmentNodes(loop);
    IRO_UnmarkLoopTestNodes(loop);
}

static void RelinearizeExpr(IROLinear* expr, IROList* list) {
    IROLinearIRSave save;
    IROLinear* node;
    IROLinear* last;
    Statement* stmt;

    IRO_SaveLinearIR(&save);
    IRO_InitList(list);
    IRO_AddToList(IRO_NewLinear(IROLinearNop), list);
    IRO_DuplicateExpr(expr, list);
    stmt = IRO_Delinearize(NULL, list->head);
    CError_ASSERT(3613, stmt != NULL);
    IRO_Linearize(IRO_PreLinearize(stmt));
    IRO_InitList(list);
    node = IRO_FirstLinear->next;
    list->head = node;
    for (; node; node = node->next) {
        if (node->type >= 1 && node->type <= 6) {
            last = node;
        }
    }
    list->tail = last;
    list->tail->next = NULL;
    IRO_RestoreLinearIR(&save);
}

IROLinear* IRO_UpdateInductionIncrement(IROLoop* loop, CInt64 count, IROLinear* before) {
    IROList list;
    IROLinear* induction;
    IROLinear* increment;
    IROLinear* duplicate;
    IROLinear* assign;
    Type* type;
    Type* intType;

    IRO_InitList(&list);
    induction = loop->induction->nd;
    type = IRO_CopyType(induction->rtype);
    intType = type;
    if (type->type == TYPEPOINTER) {
        intType = IRO_GetPointerType(type);
    }
    increment = IRO_NewIntConst(CInt64_Mul(count, loop->induction->step), intType);
    IRO_AddToList(increment, &list);
    duplicate = IRO_DuplicateExpr(induction->u.monadic, &list);
    assign = IRO_NewLinear(IROLinearOp2Arg);
    assign->index = ++IRO_NumLinear;
    assign->nodetype = (loop->induction->flags & 4) ? ESUBASS : EADDASS;
    assign->u.diadic.left = duplicate;
    assign->u.diadic.right = increment;
    assign->rtype = type;
    IRO_AddToList(assign, &list);
    IRO_Paste(list.head, list.tail, before);
    return list.tail;
}

void IRO_BuildUnrolledBodyEntryTest(
    IROLoop* loop, IROList* list, IROLinear* iter, CInt64 factor, CLabel* label
) {
    Type* type;
    IROLinear* constant;
    IROLinear* copy;
    IROLinear* greater;
    IROLinear* jump;
    IROList expr;

    type = IRO_CopyType(iter->rtype);
    constant = IRO_NewIntConst(factor, type);
    IRO_AddToList(constant, list);
    copy = IRO_DuplicateExpr(iter, list);
    greater = IRO_NewLinear(IROLinearOp2Arg);
    greater->index = ++IRO_NumLinear;
    greater->nodetype = EGREATER;
    greater->u.diadic.left = copy;
    greater->u.diadic.right = constant;
    constant->flags |= IROLF_Reffed;
    greater->rtype = type;
    IRO_AddToList(greater, list);
    jump = IRO_NewLinear(IROLinearIfNot);
    jump->index = ++IRO_NumLinear;
    jump->u.diadic.right = greater;
    greater->flags |= IROLF_Reffed;
    jump->rtype = type;
    jump->u.monadic = (IROLinear*)label;
    IRO_AddToList(jump, list);
    if (loop->countKnown && loop->unk54) {
        RelinearizeExpr(loop->unk54, &expr);
        IRO_AddToList(expr.head, list);
        jump = IRO_NewLinear(IROLinearIfNot);
        jump->index = ++IRO_NumLinear;
        jump->u.diadic.right = list->tail;
        list->tail->flags |= IROLF_Reffed;
        jump->rtype = type;
        jump->u.monadic = (IROLinear*)label;
        IRO_AddToList(jump, list);
    }
}

void IRO_LoopUnroller(void) {
    IROLoop* first;
    IROLoop* loop;

    first = IRO_FindLoops();
    for (loop = first; loop; loop = loop->next) {
        IRO_FindLoopNodes(loop);
    }
    IRO_MarkInnerLoops(first);
    for (loop = first; loop; loop = loop->next) {
        if (loop->flags & 0x80000) {
            IRO_Dump(IRO_LoopUnroller_Found_5EB28B, loop->unk14->index);
            Bv_Dump((char*)IRO_LoopUnroller_Includes_5EB2B7, loop->bitset);
            UnrollLoop(loop);
            IRO_UpdateFlagsOnInts(IRO_FirstLinear);
        }
    }
    IRO_FreeLoopList(first);
    IRO_BuildFlowgraph(IRO_FirstLinear);
    IRO_CheckForUserBreak();
}
