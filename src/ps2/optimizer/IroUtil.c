#include "compiler/IroUtil.h"
#include "compiler/CDecl.h"
#include "compiler/CError.h"
#include "compiler/CInt64.h"
#include "compiler/CMachine.h"
#include "compiler/CFunc.h"
#include "compiler/CParser.h"
#include "compiler/enode.h"
#include "compiler/IroFlowgraph.h"
#include "compiler/IroLinearForm.h"
#include "compiler/IroLoop.h"
#include "compiler/IroType.h"
#include "compiler/objects.h"
#include "compiler/CompilerTools.h"
#include "compiler/IrOptimizer.h"
#include <string.h>
#include <setjmp.h>
#include "compiler/types.h"
#include "compiler/BitVector.h"
#include "compiler/CExpr.h"
#include "compiler/IROUseDef.h"
#include "compiler/InstrSelection.h"
#include "compiler/IroDump.h"
#include "compiler/IroMalloc.h"
#include "compiler/CABI.h"
#include "compiler/CException.h"
#include "compiler/IroPointerAnalysis.h"
#include "driver/CWParserPluginsContext.h"
#include "driver/COSToolsCLT.h"
#include "compiler/IroVars.h"
#include "driver/CCompiler.h"
#include "compiler/IroCSE.h"
#include "msl/setjmp_x86.h"
#include "compiler/IroDependsKills.h"

static UInt32 IRO_LastUserBreakTick;

static IROLinear* ExprStart;
static IROLinear* ExprEnd;

Boolean IRO_IsConstant(IROLinear* linear) {
    if (linear->type == IROLinearOperand && (UInt8)(linear->u.node->type - EINTCONST) <= 1) {
        return TRUE;
    }
    return FALSE;
}

Object* IRO_IsVariable(IROLinear* linear) {
    if (linear->type == IROLinearOp1Arg &&
        linear->nodetype == EINDIRECT &&
        linear->u.monadic->type == IROLinearOperand &&
        linear->u.monadic->u.node->type == EOBJREF) {
        return linear->u.monadic->u.node->data.objref;
    }

    return NULL;
}

Boolean IRO_IsENodeIntConstant(ENode* node) {
    if (node->rtype &&
        node->rtype->type == TYPEINT &&
        TYPE_INTEGRAL(node->rtype)->integral < 0x11 &&
        ENODE_IS(node, EINTCONST)) {
        return TRUE;
    }
    return FALSE;
}

Boolean IRO_IsIntConstant(IROLinear* linear) {
    if (linear->type == IROLinearOperand &&
        linear->rtype->type == TYPEINT &&
        TYPE_INTEGRAL(linear->rtype)->integral < 0x11 &&
        linear->u.node &&
        ENODE_IS(linear->u.node, EINTCONST)) {
        return TRUE;
    }
    return FALSE;
}

Boolean IRO_IsFloatConstant(IROLinear* linear) {
    if (linear->type == IROLinearOperand && ENODE_IS(linear->u.node, EFLOATCONST)) {
        return TRUE;
    }
    return FALSE;
}

Boolean IRO_IsAssignment(IROLinear* linear) {
    if (linear->type == IROLinearOp1Arg || linear->type == IROLinearOp2Arg) {
        if (IRO_IsAssignOp[linear->nodetype]) {
            return TRUE;
        }
    }
    return FALSE;
}

IROLinear* IRO_FindLabelNode(CLabel* label, IROLinear* linear) {
    IROLinear* scan;

    for (scan = linear; scan; scan = scan->next) {
        if (scan->type == IROLinearLabel && scan->u.label == label) {
            break;
        }
    }

    CError_ASSERT(2087, scan);
    return scan;
}

static IROLinear* FindFirst(IROLinear* linear) {
    short i;

    switch (linear->type) {
        case IROLinearOperand:
            return linear;
        case IROLinearOp1Arg:
            return FindFirst(linear->u.monadic);
        case IROLinearOp2Arg:
            if (linear->u.diadic.right->index < linear->u.diadic.left->index) {
                return FindFirst(linear->u.diadic.right);
            } else {
                return FindFirst(linear->u.diadic.left);
            }
        case IROLinearOp3Arg:
            if (linear->u.args3.a->index < linear->u.args3.b->index) {
                if (linear->u.args3.a->index < linear->u.args3.c->index) {
                    return FindFirst(linear->u.args3.a);
                } else {
                    return FindFirst(linear->u.args3.c);
                }
            } else {
                if (linear->u.args3.b->index < linear->u.args3.c->index) {
                    return FindFirst(linear->u.args3.b);
                } else {
                    return FindFirst(linear->u.args3.c);
                }
            }
        case IROLinear5:
            return linear;
        case IROLinearFunccall:
            i = linear->u.funccall.argCount - 1;
            return FindFirst(linear->u.funccall.args[i]);
        default:
            CError_FATAL(2170);
            return NULL;
    }
}

IROLinear* IRO_FindFirst(IROLinear* linear) {
    IRO_RenumberInts();
    return FindFirst(linear);
}

static IROLinear* FindPreced(IROLinear* a) {
    IROLinear* iter;
    IROLinear* prev;

    prev = iter = IRO_FirstLinear;
    while (iter && iter != a) {
        prev = iter;
        iter = iter->next;
    }

    if (iter) {
        return prev;
    }
    return iter;
}

void IRO_CutAndPasteAfter(IROLinear* first, IROLinear* last, IROLinear* after) {
    IROLinear* preced = FindPreced(first);

    CError_ASSERT(2195, preced);
    preced->next = last->next;
    last->next = after->next;
    after->next = first;
}

Boolean IRO_IsConstantZero(IROLinear* linear) {
    return (IRO_IsIntConstant(linear) && CInt64_IsZero(&linear->u.node->data.intval)) ||
        (IRO_IsFloatConstant(linear) && CMach_FloatIsZero(linear->u.node->data.floatval.value));
}

Boolean IRO_IsConstantOne(IROLinear* linear) {
    return (IRO_IsIntConstant(linear) && CInt64_IsOne(&linear->u.node->data.intval)) ||
        (IRO_IsFloatConstant(linear) && CMach_FloatIsOne(linear->u.node->data.floatval.value));
}

Boolean IRO_IsConstantNegativeOne(IROLinear* linear) {
    CInt64 neg = CInt64_Neg(linear->u.node->data.intval);
    return (IRO_IsIntConstant(linear) && CInt64_IsOne(&neg)) ||
        (IRO_IsFloatConstant(linear) && CMach_FloatIsNegOne(linear->u.node->data.floatval.value));
}

static void ActNop(IROLinear* linear, Boolean isEntry, void* data) {
    if (!isEntry) {
        linear->type = IROLinearNop;
        linear->expr = NULL;
    }
}

void IRO_NopOut(IROLinear* linear) {
    IRO_WalkTree(linear, ActNop, NULL);
}

void IRO_Cut(IROLinear* first, IROLinear* last) {
    IROLinear* scan;
    IROLinear* prev;
    IRONode* node;

    scan = first;
    while (1) {
        scan->stmt = NULL;
        if (scan == last) {
            break;
        }
        scan = scan->next;
    }

    prev = NULL;
    for (scan = IRO_FirstLinear; scan && scan != first; scan = scan->next) {
        prev = scan;
    }

    CError_ASSERT(2671, scan);

    for (node = IRO_FirstNode; node; node = node->nextnode) {
        if (node->first == first) {
            if (node->last == last) {
                node->first = node->last = NULL;
                break;
            } else {
                node->first = last->next;
                break;
            }
        } else if (node->last == last) {
            node->last = prev;
            break;
        }
    }

    if (prev) {
        prev->next = last->next;
    } else {
        IRO_FirstLinear = last->next;
    }
}

void IRO_Paste(IROLinear* first, IROLinear* last, IROLinear* before) {
    IRONode* node;
    IROLinear* prev;
    IROLinear* scan;

    prev = NULL;
    for (scan = IRO_FirstLinear; scan && scan != before; scan = scan->next) {
        prev = scan;
    }

    CError_ASSERT(2736, scan);

    for (node = IRO_FirstNode; node; node = node->nextnode) {
        if (node->first == before) {
            node->first = first;
            break;
        }
    }

    last->next = before;
    if (prev) {
        prev->next = first;
    } else {
        IRO_FirstLinear = first;
    }
}

void IRO_PasteAfter(IROLinear* first, IROLinear* last, IROLinear* after) {
    IRONode* node;

    for (node = IRO_FirstNode; node; node = node->nextnode) {
        if (node->last == after) {
            node->last = last;
            break;
        }
    }

    last->next = after->next;
    after->next = first;
}

static void FindStart(IROLinear* linear, Boolean isEntry, void* data) {
    IROLinear* scan;

    if (isEntry) {
        scan = linear;
        do {
            if (scan == ExprStart) {
                ExprStart = linear;
                return;
            }
            if (scan == ExprEnd) {
                return;
            }
        } while ((scan = scan->next));
    }
}

void IRO_InitList(IROList* list) {
    list->head = list->tail = NULL;
}

void IRO_TermLinearList(IROList* list) {
    list->head = list->tail = NULL;
}

void IRO_AddToList(IROLinear* linear, IROList* list) {
    if (list->head) {
        list->tail->next = linear;
    } else {
        list->head = linear;
    }

    list->tail = linear;
    while (list->tail->next) {
        list->tail = list->tail->next;
    }
}

void IRO_DuplicateExprRange(IROLinear* start, IROLinear* end, IROList* list) {
    IROLinear* scan;

    for (scan = start; scan; scan = scan->next) {
        if (scan->type != IROLinearNop &&
            scan->type != IROLinearLabel &&
            !(scan->flags & IROLF_Reffed)) {
            IRO_DuplicateExpr(scan, list);
        }
        if (scan == end) {
            break;
        }
    }
}

IROLinear* IRO_FindStart(IROLinear* linear) {
    ExprStart = ExprEnd = linear;
    IRO_WalkTree(linear, FindStart, NULL);
    return ExprStart;
}

CLabel* IRO_NewLabel(void) {
    CLabel* label = newlabel();
    label->next = Labels;
    Labels = label;
    return label;
}

Boolean IRO_ObjectIsAFunction(Object* obj) {
    return (UInt8)(obj->datatype - DFUNC) <= 2;
}

void IRO_SetupForUserBreakChecking(void) {
    IRO_LastUserBreakTick = 0;
}

Boolean IRO_LinearNodeMightThrowException(IROLinear* linear) {
    if (linear->type == IROLinearFunccall) {
        return IRO_FunctionCallMightThrowException(linear);
    }
    return FALSE;
}

void IRO_ActOnExpressions(IROLinear* linear, IROChildExpressionFunc func, void* data) {
    IROLinear* root = IRO_RootExpressionOfStatement(linear);
    if (root) {
        func(root, TRUE, data);
    }
}

void IRO_ClipExpr(IROExpr* expr) {
    ExprStart = ExprEnd = expr->linear;
    IRO_WalkTree(expr->linear, FindStart, NULL);
    IRO_Cut(ExprStart, ExprEnd);
}

Boolean IRO_StatementsHaveSameExceptionActions(Statement* a, Statement* b) {
    if (a != b && b->dobjstack != a->dobjstack) {
        return CExcept_IsSubList(a->dobjstack, b->dobjstack) == 0;
    }
    return TRUE;
}

void IRO_CopyCommonStatementFields(Statement* dst, Statement* src) {
    dst->dobjstack = src->dobjstack;
    dst->sourcefilepath = src->sourcefilepath;
    dst->sourceoffset = src->sourceoffset;
    dst->value = src->value;
    *(UInt8*)&dst->flags = *(UInt8*)&src->flags;
}

Statement* IRO_NewStatement(StatementType type, ENode* expr) {
    Statement* stmt = lalloc(sizeof(Statement));
    memset(stmt, 0, sizeof(Statement));
    stmt->type = type;
    stmt->expr = expr;
    return stmt;
}

ENode* IRO_NewENode(ENodeType type) {
    ENode* node = lalloc(sizeof(ENode));
    memset(node, 0, sizeof(ENode));
    node->type = type;
    return node;
}

typedef struct IROAddrElm {
    void* value;
    struct IROAddrElm* next;
} IROAddrElm;

void IRO_AddElmToList(void* value, IROAddrElm** list, void* (*allocate)(UInt32)) {
    IROAddrElm* entry = allocate(sizeof(IROAddrElm));
    entry->value = value;
    entry->next = NULL;
    if (*list) {
        entry->next = *list;
        *list = entry;
    } else {
        *list = entry;
    }
}

void IRO_TermAddressRecord(IROAddrRecord* rec, void (*freefn)(void*)) {
    IROElmList* elm;
    IROElmList* next;

    elm = rec->objRefs;
    while (elm) {
        next = elm->next;
        freefn(elm);
        elm = next;
    }
    rec->numObjRefs = 0;
    rec->objRefs = NULL;
    elm = rec->misc;
    while (elm) {
        next = elm->next;
        freefn(elm);
        elm = next;
    }
    rec->numMisc = 0;
    rec->misc = NULL;
    elm = rec->ints;
    while (elm) {
        next = elm->next;
        freefn(elm);
        elm = next;
    }
    rec->numInts = 0;
    rec->ints = NULL;
    rec->unused = 0;
    rec->linear = NULL;
}

void IRO_InitAddressRecord(void* rec, void* value) {
    UInt32* words = rec;
    words[1] = 0;
    words[2] = 0;
    words[3] = 0;
    words[4] = 0;
    words[5] = 0;
    words[6] = 0;
    words[7] = 0;
    words[0] = (UInt32)value;
}

IROLinear* IRO_NewLabelLinearNode(IROList* list) {
    IROLinear* nd = IRO_NewLinear(IROLinearLabel);
    CLabel* label;
    nd->index = IRO_NumLinear++;
    label = newlabel();
    label->next = Labels;
    Labels = label;
    nd->u.label = label;
    nd->flags |= 1;
    return nd;
}

ENode* IRO_NewENodeFloatConst(Float value, Type* type) {
    ENode* node = IRO_NewENode(EFLOATCONST);
    node->data.floatval = value;
    node->rtype = IRO_CopyType(type);
    return node;
}

ENode* IRO_NewENodeIntConst(CInt64 value, Type* type) {
    ENode* node;
    IRO_TruncateValueToType(&value, type);
    node = IRO_NewENode(EINTCONST);
    node->data.intval = value;
    node->rtype = IRO_CopyType(type);
    return node;
}

IROLinear* IRO_NewIntConst(CInt64 value, Type* type) {
    IROLinear* linear = IRO_NewLinear(IROLinearOperand);
    linear->nodetype = EINTCONST;
    linear->index = ++IRO_NumLinear;
    linear->rtype = IRO_CopyType(type);
    linear->u.node = IRO_NewENodeIntConst(value, type);
    linear->flags |= 0x100;
    return linear;
}

void IRO_CheckForUserBreak(void) {
    if (IRO_LastUserBreakTick + 8 < COS_GetTicks()) {
        if (CWUserBreak(cparams.context)) {
            fn_0044F9E0();
        }
        IRO_LastUserBreakTick = COS_GetTicks();
    }
}

typedef struct IROGotoTarget {
    struct IROGotoTarget* next;
    void* label;
    UInt8 padding[0x20];
} IROGotoTarget;

void IRO_FreeGotoTargets(IROGotoTarget* list) {
    IROGotoTarget* next;
    if ((void*)list != IRO_GotoTargetSentinel) {
        while (list) {
            next = list->next;
            IRO_ofree(list);
            list = next;
        }
    }
}

Boolean IRO_ObjectIsRestrictQualified(Object* obj) {
    return (CParser_GetTypeQualifiers(obj->type, obj->qual) & 0x200000) != 0;
}

Boolean IRO_IsIntGenKillCandidate(IROLinear* linear) {
    int kindMatch = 0;
    int matching;
    UInt8 kind = linear->type;
    Boolean result = FALSE;
    kindMatch = kind == IROLinearFunccall;
    if (!kindMatch) {
        kindMatch = kind == IROLinearAsm;
    }
    matching = kindMatch != 0;
    if (matching) {
        return TRUE;
    }
    if ((UInt8)(kind - IROLinearOp1Arg) <= 1) {
        if (IRO_IsAssignOp[linear->nodetype]) {
            result = TRUE;
        }
    }
    return result != FALSE;
}

IROLinear* IRO_RootExpressionOfStatement(IROLinear* linear) {
    switch (linear->type) {
        case IROLinearNop:
        case IROLinearGoto:
        case IROLinearLabel:
        case IROLinearAsm:
        case IROLinearEnd:
            return NULL;
        case IROLinearGotoExpr:
            return linear->u.monadic;
        case IROLinearBeginCatch:
        case IROLinearEndCatch:
        case IROLinearEndCatchDtor:
            return linear->u.monadic;
        case IROLinearOperand:
        case IROLinearOp1Arg:
        case IROLinearOp2Arg:
        case IROLinearOp3Arg:
        case IROLinear5:
        case IROLinearFunccall:
            return linear;
        case IROLinearIf:
        case IROLinearIfNot:
            return linear->u.diadic.right;
        case IROLinearReturn:
            if (linear->u.monadic) {
                return linear->u.monadic;
            }
            break;
        case IROLinearSwitch:
            return linear->u.diadic.right;
        default:
            CError_FATAL(2637);
            break;
    }
    return NULL;
}

IROLinear* IRO_TempReference(Object* obj) {
    IROLinear* op = IRO_NewLinear(IROLinearOperand);
    IROLinear* ind;
    op->nodetype = EOBJREF;
    op->u.node = create_objectrefnode(obj);
    op->rtype = IRO_CopyType(op->u.node->rtype);
    op->index = ++IRO_NumLinear;
    op->flags |= 0x22;
    ind = IRO_NewLinear(IROLinearOp1Arg);
    op->next = ind;
    ind->nodetype = EINDIRECT;
    ind->rtype = IRO_CopyType(obj->type);
    ind->u.monadic = op;
    ind->index = ++IRO_NumLinear;
    ind->next = NULL;
    return ind;
}

//! @reconstructed
static Boolean duplicateenode(ENode** node, SInt32 flags, void* data) {
    if (flags & 1) {
        ENode* copy = IRO_NewENode((*node)->type);
        *copy = **node;
        *node = copy;
    }
    return TRUE;
}

ENode* IRO_DuplicateENode(ENode* node) {
    ENode* copy = node;
    IRO_WalkENodeTree(&copy, duplicateenode, 1, NULL);
    return copy;
}

Object* IRO_IsVariableEquIntConst(IROLinear* linear) {
    int matching = linear != NULL;
    if (matching) {
        matching = linear->nodetype == EEQU;
    }
    {
        Boolean ready = matching != 0;
        if (ready) {
            int isconst = (IRO_IsIntConstant(linear->u.diadic.right) != FALSE);
            if (isconst) {
                return IRO_IsVariable(linear->u.diadic.left);
            }
        }
    }
    return NULL;
}

Boolean IRO_FunctionCallMightThrowException(IROLinear* linear) {
    Object* obj;
    if (linear->type == IROLinearFunccall) {
        obj = NULL;
        if (linear->u.funccall.callee->type == IROLinearOperand &&
            linear->u.funccall.callee->u.node->type == EOBJREF) {
            obj = linear->u.funccall.callee->u.node->data.objref;
        }
        if (!obj ||
            CExcept_CanThrowException(
                obj, obj->datatype == DVFUNC && (linear->nodeflags & 0x80) == 0
            )) {
            return TRUE;
        }
    } else if (linear->type == IROLinearAsm && IRO_GetAsmEffects && linear->u.asm_stmt) {
        IAEffects effects = IRO_EmptyAsmEffects;
        IRO_GetAsmEffects(linear->u.asm_stmt, &effects);
        if (effects.branchWithLink) {
            return TRUE;
        }
    }
    return FALSE;
}

void IRO_WalkInts(IROLinear* first, IROLinear* last, IROChildExpressionFunc func, void* data) {
    IROLinear* scan;

    for (scan = first; scan; scan = scan->next) {
        switch (scan->type) {
            case IROLinearGotoExpr:
                IRO_WalkTree(scan->u.monadic, func, data);
                break;
            case IROLinearBeginCatch:
            case IROLinearEndCatch:
            case IROLinearEndCatchDtor:
                IRO_WalkTree(scan->u.monadic, func, data);
                break;
            case IROLinearOperand:
            case IROLinearOp1Arg:
            case IROLinearOp2Arg:
            case IROLinearOp3Arg:
            case IROLinear5:
            case IROLinearFunccall:
                IRO_WalkTree(scan, func, data);
                break;
            case IROLinearIf:
            case IROLinearIfNot:
                IRO_WalkTree(scan->u.diadic.right, func, data);
                break;
            case IROLinearReturn:
                if (scan->u.monadic) {
                    IRO_WalkTree(scan->u.monadic, func, data);
                }
                break;
            case IROLinearSwitch:
                IRO_WalkTree(scan->u.diadic.right, func, data);
                break;
            case IROLinearAsm:
                func(scan, TRUE, data);
                func(scan, FALSE, data);
                break;
            case IROLinearNop:
            case IROLinearGoto:
            case IROLinearLabel:
            case IROLinearEnd:
                break;
            default:
                CError_FATAL(2507);
                break;
        }
        if (scan == last) {
            return;
        }
    }
}

void IRO_ActOnChildExpressions(IROLinear* linear, IROChildExpressionFunc func, void* data) {
    int i;

    switch (linear->type) {
        case IROLinearGotoExpr:
            func(linear->u.monadic, TRUE, data);
            break;
        case IROLinearOp1Arg:
        case IROLinearBeginCatch:
        case IROLinearEndCatch:
        case IROLinearEndCatchDtor:
            func(linear->u.monadic, TRUE, data);
            break;
        case IROLinearOp2Arg:
            func(linear->u.diadic.left, TRUE, data);
            func(linear->u.diadic.right, TRUE, data);
            break;
        case IROLinearOp3Arg:
            func(linear->u.args3.a, TRUE, data);
            func(linear->u.args3.b, TRUE, data);
            func(linear->u.args3.c, TRUE, data);
            break;
        case IROLinearFunccall:
            func(linear->u.funccall.callee, TRUE, data);
            for (i = 0; i < linear->u.funccall.argCount; i++) {
                func(linear->u.funccall.args[i], TRUE, data);
            }
            break;
        case IROLinearIf:
        case IROLinearIfNot:
            func(linear->u.diadic.right, TRUE, data);
            break;
        case IROLinearReturn:
            if (linear->u.monadic) {
                func(linear->u.monadic, TRUE, data);
            }
            break;
        case IROLinearSwitch:
            func(linear->u.diadic.right, TRUE, data);
            break;
        case IROLinearNop:
        case IROLinearOperand:
        case IROLinear5:
        case IROLinearGoto:
        case IROLinearLabel:
        case IROLinearAsm:
        case IROLinearEnd:
            break;
        default:
            CError_FATAL(2581);
            break;
    }
}

void IRO_WalkTreeToPropagateFlags(IROLinear* linear, IROChildExpressionFunc func, void* data) {
    int i;

    switch (linear->type) {
        case IROLinearOperand:
            break;
        case IROLinearOp1Arg:
            WalkTreeHelper(linear->u.monadic, func, FALSE, data);
            break;
        case IROLinearOp2Arg:
            WalkTreeHelper(linear->u.diadic.left, func, FALSE, data);
            WalkTreeHelper(linear->u.diadic.right, func, FALSE, data);
            break;
        case IROLinearOp3Arg:
            WalkTreeHelper(linear->u.args3.a, func, FALSE, data);
            WalkTreeHelper(linear->u.args3.b, func, FALSE, data);
            WalkTreeHelper(linear->u.args3.c, func, FALSE, data);
            break;
        case IROLinearFunccall:
            WalkTreeHelper(linear->u.funccall.callee, func, FALSE, data);
            for (i = 0; i < linear->u.funccall.argCount; i++) {
                WalkTreeHelper(linear->u.funccall.args[i], func, FALSE, data);
            }
            break;
        default:
            break;
    }
    func(linear, FALSE, data);
}

void IRO_WalkTree(IROLinear* linear, IROChildExpressionFunc func, void* data) {
    int i;

    func(linear, TRUE, data);
    switch (linear->type) {
        case IROLinearOperand:
        case IROLinear5:
            break;
        case IROLinearOp1Arg:
            WalkTreeHelper(linear->u.monadic, func, TRUE, data);
            break;
        case IROLinearOp2Arg:
            WalkTreeHelper(linear->u.diadic.left, func, TRUE, data);
            WalkTreeHelper(linear->u.diadic.right, func, TRUE, data);
            break;
        case IROLinearOp3Arg:
            WalkTreeHelper(linear->u.args3.a, func, TRUE, data);
            WalkTreeHelper(linear->u.args3.b, func, TRUE, data);
            WalkTreeHelper(linear->u.args3.c, func, TRUE, data);
            break;
        case IROLinearFunccall:
            WalkTreeHelper(linear->u.funccall.callee, func, TRUE, data);
            for (i = 0; i < linear->u.funccall.argCount; i++) {
                WalkTreeHelper(linear->u.funccall.args[i], func, TRUE, data);
            }
            break;
        default:
            break;
    }
    func(linear, FALSE, data);
}

static void SetNodeNumInExprList(IROLinear* linear, Boolean isEntry, void* data) {
    IROExpr* expr;
    if (isEntry) {
        expr = linear->expr;
        if (expr) {
            expr->node = data;
        }
    }
}

void IRO_MoveExpression(IROExpr* expr, IROLinear* linear) {
    IRONode* node;
    IROLinear* scan;

    ExprStart = ExprEnd = expr->linear;
    IRO_WalkTree(expr->linear, FindStart, NULL);
    if (ExprStart == linear) {
        return;
    }
    IRO_Cut(ExprStart, ExprEnd);
    IRO_Paste(ExprStart, ExprEnd, linear);
    for (node = IRO_FirstNode; node; node = node->nextnode) {
        for (scan = node->first; scan; scan = scan->next) {
            if (scan == expr->linear) {
                expr->node = node;
                break;
            }
            if (scan == node->last) {
                break;
            }
        }
    }
    IRO_WalkTree(expr->linear, SetNodeNumInExprList, expr->node);
}

int IRO_HasSideEffect(IROLinear* linear) {
    int effect;

    if (!linear) {
        return (int)linear;
    }
    if (((volatile IROLinear*)linear)->rtype) {
        if (CParser_IsVolatile(((volatile IROLinear*)linear)->rtype, linear->nodeflags & 3)) {
            return (int)linear;
        }
    }
    switch (linear->type) {
        case IROLinearAsm:
            return (int)linear;
        case IROLinearOperand:
            if (linear->u.node->type == EOBJREF &&
                is_volatile_object(linear->u.node->data.objref)) {
                return (int)linear;
            }
            return 0;
        case IROLinearOp1Arg:
            if (IRO_IsAssignOp[linear->nodetype]) {
                return (int)linear;
            }
            return IRO_HasSideEffect(linear->u.monadic);
        case IROLinearOp2Arg:
            if (IRO_IsAssignOp[linear->nodetype]) {
                return (int)linear;
            }
            effect = IRO_HasSideEffect(linear->u.diadic.left);
            if (!effect) {
                effect = IRO_HasSideEffect(linear->u.diadic.right);
            }
            return effect;
        case IROLinearOp3Arg:
            if ((effect = IRO_HasSideEffect(linear->u.args3.a))) {
                return effect;
            } else if ((effect = IRO_HasSideEffect(linear->u.args3.b))) {
                return effect;
            }
            return IRO_HasSideEffect(linear->u.args3.c);
        case IROLinear5:
            return (int)linear;
        case IROLinearFunccall:
            return (int)linear;
        default:
            return (int)linear;
    }
}

typedef struct IROStatementJmp {
    jmp_buf buf;
    Boolean found;
} IROStatementJmp;

//! @reconstructed
static void StatementSideEffectCallback(IROLinear* linear, Boolean entry, void* data) {
    IROStatementJmp* jmp = data;
    if (entry && ((UInt8)(linear->type - 1) <= 5 || linear->type == IROLinearAsm)) {
        if (IRO_HasSideEffect(linear)) {
            jmp->found = TRUE;
            longjmp(jmp->buf, 1);
        }
    }
}

Boolean IRO_StatementContainsSideEffect(IROLinear* linear) {
    IROStatementJmp jmp;
    memset(&jmp, 0, sizeof(jmp));
    jmp.found = FALSE;
    if (!setjmp(jmp.buf)) {
        IRO_WalkInts(linear, linear, StatementSideEffectCallback, &jmp);
    }
    return jmp.found;
}

typedef struct IROStatementUseJmp {
    jmp_buf buf;
    IROLinear* target;
    Boolean found;
} IROStatementUseJmp;

//! @reconstructed
static void StatementUseCallback(IROLinear* linear, Boolean entry, void* data) {
    IROStatementUseJmp* jmp = data;
    if (entry && linear == jmp->target) {
        jmp->found = TRUE;
        longjmp(jmp->buf, 1);
    }
}

Boolean IRO_StatementContainsUse(IROLinear* linear, IROLinear* target) {
    IROStatementUseJmp jmp;
    memset(&jmp, 0, sizeof(jmp));
    jmp.target = target;
    jmp.found = FALSE;
    if (!setjmp(jmp.buf)) {
        IRO_WalkInts(linear, linear, StatementUseCallback, &jmp);
    }
    return jmp.found;
}

IROLinear* IRO_CheckSideEffect(IROLinear* linear) {
    IROLinear* result;
    IROLinear* effect1;
    IROLinear* effect2;
    IROLinear* effect3;

    if (!linear) {
        return linear;
    }
    if (((volatile IROLinear*)linear)->rtype &&
        CParser_IsVolatile(linear->rtype, linear->nodeflags & 3)) {
        linear->flags &= ~IROLF_Reffed;
        return linear;
    }
    result = NULL;
    switch (linear->type) {
        case IROLinearAsm:
            linear->flags &= ~IROLF_Reffed;
            return linear;
        case IROLinearOperand:
            if (linear->u.node->type == EOBJREF &&
                is_volatile_object(linear->u.node->data.objref)) {
                linear->flags &= ~IROLF_Reffed;
                return linear;
            }
            break;
        case IROLinearOp1Arg:
            if (IRO_IsAssignOp[linear->nodetype]) {
                linear->flags &= ~IROLF_Reffed;
                return linear;
            }
            if ((result = IRO_CheckSideEffect(linear->u.monadic))) {
                if (linear->nodetype == EINDIRECT && linear->u.monadic->type == IROLinearOperand) {
                    linear->flags &= ~IROLF_Reffed;
                    return linear;
                }
            }
            break;
        case IROLinearOp2Arg:
            if (IRO_IsAssignOp[linear->nodetype]) {
                linear->flags &= ~IROLF_Reffed;
                return linear;
            }
            effect1 = IRO_CheckSideEffect(linear->u.diadic.left);
            effect2 = IRO_CheckSideEffect(linear->u.diadic.right);
            result = effect1 ? effect1 : effect2;
            break;
        case IROLinearOp3Arg:
            effect1 = IRO_CheckSideEffect(linear->u.args3.a);
            effect2 = IRO_CheckSideEffect(linear->u.args3.b);
            effect3 = IRO_CheckSideEffect(linear->u.args3.c);
            if (!effect1) {
                if (!effect2) {
                    effect2 = effect3;
                }
                effect1 = effect2;
            }
            result = effect1;
            break;
        case IROLinear5:
            linear->flags &= ~IROLF_Reffed;
            return linear;
        case IROLinearFunccall:
            linear->flags &= ~IROLF_Reffed;
            return linear;
        default:
            return linear;
    }
    linear->type = IROLinearNop;
    IRO_Dump("Nop out with side-effects checking at: %d\n", linear->index);
    return result;
}

Boolean IRO_IsPowerOfTwo(IROLinear* linear, SInt32* bit) {
    CInt64 value;
    SInt32 index;

    *bit = -1;
    if (IRO_IsIntConstant(linear)) {
        value = linear->u.node->data.intval;
        index = 0;
        if (!CInt64_IsZero(&value)) {
            do {
                if (CInt64_Equal(CInt64_And(value, cint64_one), cint64_one)) {
                    if (CInt64_IsOne(&value)) {
                        *bit = index;
                        return TRUE;
                    }
                    break;
                }
                value = CInt64_ShrU(value, cint64_one);
                index++;
            } while (!CInt64_IsZero(&value));
        }
    }
    *bit = 0;
    return FALSE;
}

static Boolean NotNoppable(IROLinear* linear) {
    return linear->type == IROLinearFunccall ||
        IRO_IsAssignment(linear) ||
        (linear->type == IROLinearOperand && linear->u.node->type == EINSTRUCTION) ||
        (linear->rtype &&
         CParser_IsVolatile(((volatile IROLinear*)linear)->rtype, linear->nodeflags & 3)) ||
        (linear->type == IROLinearOperand &&
         linear->u.node->type == EOBJREF &&
         is_volatile_object(linear->u.node->data.objref));
}

static void ActNopNonSideEffects(IROLinear* linear, Boolean isEntry, void* data) {
    SInt32* level = data;
    if (isEntry) {
        if (NotNoppable(linear)) {
            if (!*level) {
                linear->flags &= ~IROLF_Reffed;
            }
            ++*level;
        }
    } else {
        if (NotNoppable(linear)) {
            --*level;
        } else if (!*level) {
            linear->type = IROLinearNop;
        }
    }
}

void IRO_NopNonSideEffects(IROLinear* linear, SInt32 level) {
    IRO_WalkTree(linear, ActNopNonSideEffects, &level);
}

//! @reconstructed
static Boolean ENodeSideEffectCallback(ENode** expr, SInt32 flags, void* data) {
    ENode* node = *expr;
    if ((node->flags & ENODE_FLAG_VOLATILE) ||
        (node->type == EOBJREF && is_volatile_object(node->data.objref)) ||
        IRO_IsAssignOp[node->type] ||
        (UInt8)(node->type + 0xC8) <= 1) {
        *(Boolean*)data = TRUE;
        return FALSE;
    }
    return TRUE;
}

Boolean IRO_ENodeExprHasSideEffect(ENode* expr) {
    Boolean result = FALSE;
    if (expr) {
        IRO_WalkENodeTree(&expr, ENodeSideEffectCallback, 1, &result);
    }
    return result;
}

static inline IROLinear* LocateFatherHelper(IROLinear* linear, Boolean wrap, IROLinear*** link) {
    IROLinear* scan;
    SInt32 index;
    SInt32 i;

    for (scan = linear->next, index = 0; index < (wrap ? 2 : 1); index++) {
        while (scan) {
            switch (scan->type) {
                case IROLinearGotoExpr:
                    if (scan->u.monadic == linear) {
                        if (link) {
                            *link = &scan->u.monadic;
                        }
                        return scan;
                    }
                    break;
                case IROLinearIf:
                case IROLinearIfNot:
                    if (scan->u.diadic.right == linear) {
                        if (link) {
                            *link = &scan->u.diadic.right;
                        }
                        return scan;
                    }
                    break;
                case IROLinearReturn:
                    if (scan->u.monadic == linear) {
                        if (link) {
                            *link = &scan->u.monadic;
                        }
                        return scan;
                    }
                    break;
                case IROLinearOp1Arg:
                    if (scan->u.monadic == linear) {
                        if (link) {
                            *link = &scan->u.monadic;
                        }
                        return scan;
                    }
                    break;
                case IROLinearSwitch:
                    if (scan->u.diadic.right == linear) {
                        if (link) {
                            *link = &scan->u.diadic.right;
                        }
                        return scan;
                    }
                    break;
                case IROLinearOp2Arg:
                    if (scan->u.diadic.left == linear) {
                        if (link) {
                            *link = &scan->u.diadic.left;
                        }
                        return scan;
                    }
                    if (scan->u.diadic.right == linear) {
                        if (link) {
                            *link = &scan->u.diadic.right;
                        }
                        return scan;
                    }
                    break;
                case IROLinearOp3Arg:
                    if (scan->u.args3.a == linear) {
                        if (link) {
                            *link = &scan->u.args3.a;
                        }
                        return scan;
                    }
                    if (scan->u.args3.b == linear) {
                        if (link) {
                            *link = &scan->u.args3.b;
                        }
                        return scan;
                    }
                    if (scan->u.args3.c == linear) {
                        if (link) {
                            *link = &scan->u.args3.c;
                        }
                        return scan;
                    }
                    break;
                case IROLinearFunccall:
                    if (scan->u.funccall.callee == linear) {
                        if (link) {
                            *link = &scan->u.funccall.callee;
                        }
                        return scan;
                    }
                    for (i = 0; i < scan->u.funccall.argCount; i++) {
                        if (scan->u.funccall.args[i] == linear) {
                            if (link) {
                                *link = &scan->u.funccall.args[i];
                            }
                            return scan;
                        }
                    }
                    break;
                case IROLinearNop:
                case IROLinearOperand:
                case IROLinear5:
                case IROLinearGoto:
                case IROLinearLabel:
                case 14:
                case 15:
                case IROLinearBeginCatch:
                case IROLinearEndCatch:
                case IROLinearEndCatchDtor:
                case IROLinearAsm:
                case IROLinearEnd:
                    break;
                default:
                    CError_FATAL(3288);
                    break;
            }
            scan = scan->next;
        }
        scan = IRO_FirstLinear;
    }

    if (link) {
        *link = NULL;
    }
    return NULL;
}

IROLinear* IRO_LocateFather(IROLinear* linear) {
    return LocateFatherHelper(linear, FALSE, NULL);
}

IROLinear* IRO_LocateFather_Cut_And_Paste_Without_Nopping(IROLinear* nd, IROLinear* replacement) {
    IROLinear* scan;
    SInt16 i;
    IROLinear** link;
    volatile SInt32 index;

    for (scan = nd->next, index = 0; index < 1; index++) {
        while (scan) {
            switch (scan->type) {
                case IROLinearGotoExpr:
                    if (scan->u.monadic != nd) {
                        break;
                    }
                    link = &scan->u.monadic;
                    goto done;
                case IROLinearIf:
                case IROLinearIfNot:
                    if (scan->u.diadic.right != nd) {
                        break;
                    }
                    link = &scan->u.diadic.right;
                    goto done;
                case IROLinearReturn:
                    if (scan->u.monadic != nd) {
                        break;
                    }
                    link = &scan->u.monadic;
                    goto done;
                case IROLinearOp1Arg:
                    if (scan->u.monadic != nd) {
                        break;
                    }
                    link = &scan->u.monadic;
                    goto done;
                case IROLinearSwitch:
                    if (scan->u.diadic.right != nd) {
                        break;
                    }
                    link = &scan->u.diadic.right;
                    goto done;
                case IROLinearOp2Arg:
                    if (scan->u.diadic.left == nd) {
                        link = &scan->u.diadic.left;
                        goto done;
                    }
                    if (scan->u.diadic.right != nd) {
                        break;
                    }
                    link = &scan->u.diadic.right;
                    goto done;
                case IROLinearOp3Arg:
                    if (scan->u.args3.a == nd) {
                        link = &scan->u.args3.a;
                        goto done;
                    }
                    if (scan->u.args3.b == nd) {
                        link = &scan->u.args3.b;
                        goto done;
                    }
                    if (scan->u.args3.c != nd) {
                        break;
                    }
                    link = &scan->u.args3.c;
                    goto done;
                case IROLinearFunccall:
                    if (scan->u.funccall.callee == nd) {
                        link = &scan->u.funccall.callee;
                        goto done;
                    }
                    for (i = 0; i < scan->u.funccall.argCount; i++) {
                        if (scan->u.funccall.args[i] == nd) {
                            link = &scan->u.funccall.args[i];
                            goto done;
                        }
                    }
                    break;
                case IROLinearNop:
                case IROLinearOperand:
                case IROLinear5:
                case IROLinearGoto:
                case IROLinearLabel:
                case 14:
                case 15:
                case IROLinearBeginCatch:
                case IROLinearEndCatch:
                case IROLinearEndCatchDtor:
                case IROLinearAsm:
                case IROLinearEnd:
                    break;
                default:
                    CError_FATAL(3288);
                    break;
            }
            scan = scan->next;
        }
        scan = IRO_FirstLinear;
    }
    link = NULL;
    scan = NULL;
done:
    if (scan) {
        CError_ASSERT(3335, link && *link);
        *link = replacement;
    }
    return scan;
}

IROLinear* IRO_LocateFather_Cut_And_Paste(IROLinear* nd, IROLinear* replacement) {
    IROLinear* scan;
    SInt32 i;
    IROLinear** link;
    volatile SInt32 index;

    for (scan = nd->next, index = 0; index < 1; index++) {
        while (scan) {
            switch (scan->type) {
                case IROLinearGotoExpr:
                    if (scan->u.monadic != nd) {
                        break;
                    }
                    link = &scan->u.monadic;
                    goto done;
                case IROLinearIf:
                case IROLinearIfNot:
                    if (scan->u.diadic.right != nd) {
                        break;
                    }
                    link = &scan->u.diadic.right;
                    goto done;
                case IROLinearReturn:
                    if (scan->u.monadic != nd) {
                        break;
                    }
                    link = &scan->u.monadic;
                    goto done;
                case IROLinearOp1Arg:
                    if (scan->u.monadic != nd) {
                        break;
                    }
                    link = &scan->u.monadic;
                    goto done;
                case IROLinearSwitch:
                    if (scan->u.diadic.right != nd) {
                        break;
                    }
                    link = &scan->u.diadic.right;
                    goto done;
                case IROLinearOp2Arg:
                    if (scan->u.diadic.left == nd) {
                        link = &scan->u.diadic.left;
                        goto done;
                    }
                    if (scan->u.diadic.right != nd) {
                        break;
                    }
                    link = &scan->u.diadic.right;
                    goto done;
                case IROLinearOp3Arg:
                    if (scan->u.args3.a == nd) {
                        link = &scan->u.args3.a;
                        goto done;
                    }
                    if (scan->u.args3.b == nd) {
                        link = &scan->u.args3.b;
                        goto done;
                    }
                    if (scan->u.args3.c != nd) {
                        break;
                    }
                    link = &scan->u.args3.c;
                    goto done;
                case IROLinearFunccall:
                    if (scan->u.funccall.callee == nd) {
                        link = &scan->u.funccall.callee;
                        goto done;
                    }
                    for (i = 0; i < scan->u.funccall.argCount; i++) {
                        if (scan->u.funccall.args[i] == nd) {
                            link = &scan->u.funccall.args[i];
                            goto done;
                        }
                    }
                    break;
                case IROLinearNop:
                case IROLinearOperand:
                case IROLinear5:
                case IROLinearGoto:
                case IROLinearLabel:
                case 14:
                case 15:
                case IROLinearBeginCatch:
                case IROLinearEndCatch:
                case IROLinearEndCatchDtor:
                case IROLinearAsm:
                case IROLinearEnd:
                    break;
                default:
                    CError_FATAL(3288);
                    break;
            }
            scan = scan->next;
        }
        scan = IRO_FirstLinear;
    }
    link = NULL;
    scan = NULL;
done:
    if (scan) {
        CError_ASSERT(3319, link && *link);
        IRO_NopOut(nd);
        *link = replacement;
    }
    return scan;
}

CInt64 IRO_GetSelfAssignmentVal(IROLinear* linear) {
    CInt64 result;

    if (linear->type == IROLinearOp1Arg) {
        switch (linear->nodetype) {
            case EPOSTINC:
            case EPREINC:
                CInt64_SetLong(&result, 1);
                break;
            case EPOSTDEC:
            case EPREDEC:
                CInt64_SetLong(&result, -1);
                break;
            default:
                CError_FATAL(4076);
                break;
        }
        if (IS_TYPE_POINTER_ONLY(linear->rtype)) {
            CInt64 mul;
            CInt64_SetLong(&mul, ((TypePointer*)linear->rtype)->target->size);
            result = CInt64_Mul(result, mul);
        }
    } else if (linear->type == IROLinearOp2Arg) {
        switch (linear->nodetype) {
            case EMULASS:
            case EDIVASS:
            case EMODASS:
            case EADDASS:
            case ESHLASS:
            case ESHRASS:
            case EANDASS:
            case EXORASS:
            case EORASS:
                result = linear->u.diadic.right->u.node->data.intval;
                break;
            case ESUBASS:
                result = linear->u.diadic.right->u.node->data.intval;
                result = CInt64_Neg(result);
                break;
            default:
                CError_FATAL(4119);
                break;
        }
    } else {
        CError_FATAL(4124);
    }
    return result;
}

//! @reconstructed
static void IRO_DecomposeAddressExpression_Cheap_Recursive(
    IROLinear* linear, IROLinear** obj, SInt32* objCount, SInt32* otherCount
) {
    IROLinear* left = linear->u.diadic.left;

    if (left->type == IROLinearOp2Arg && left->nodetype == EADD) {
        IRO_DecomposeAddressExpression_Cheap_Recursive(left, obj, objCount, otherCount);
    } else if (!IRO_IsIntConstant(left)) {
        if (left->type == IROLinearOperand && left->u.node->type == EOBJREF) {
            (*objCount)++;
            *obj = left;
        } else {
            (*otherCount)++;
        }
    }

    if (linear->u.diadic.right->type == IROLinearOp2Arg &&
        linear->u.diadic.right->nodetype == EADD) {
        IRO_DecomposeAddressExpression_Cheap_Recursive(
            linear->u.diadic.right, obj, objCount, otherCount
        );
    } else if (!IRO_IsIntConstant(linear->u.diadic.right)) {
        if (linear->u.diadic.right->type == IROLinearOperand &&
            linear->u.diadic.right->u.node->type == EOBJREF) {
            (*objCount)++;
            *obj = linear->u.diadic.right;
        } else {
            (*otherCount)++;
        }
    }
}

void IRO_DecomposeAddressExpression_Cheap(
    IROLinear* linear, IROLinear** obj, SInt32* objCount, SInt32* otherCount
) {
    *obj = NULL;
    *objCount = 0;
    *otherCount = 0;
    IRO_DecomposeAddressExpression_Cheap_Recursive(linear, obj, objCount, otherCount);
}

typedef struct IRODecomposedAddress {
    IROLinear* original;
    SInt32 numObjRefs;
    IROAddrElm* objRefs;
    SInt32 numMisc;
    IROAddrElm* misc;
    SInt32 numInts;
    IROAddrElm* ints;
    SInt32 unused;
} IRODecomposedAddress;

void IRO_DecomposeAddressExpression(
    IROLinear* linear, IRODecomposedAddress* rec, void* (*allocate)(UInt32)
) {
    IROLinear* left = linear->u.diadic.left;
    IROLinear* right;

    if (left->type == IROLinearOp2Arg && left->nodetype == EADD) {
        IRO_DecomposeAddressExpression(left, rec, allocate);
    } else if (IRO_IsIntConstant(left)) {
        rec->numInts++;
        IRO_AddElmToList(left, &rec->ints, allocate);
    } else if (left->type == IROLinearOperand && left->u.node->type == EOBJREF) {
        rec->numObjRefs++;
        IRO_AddElmToList(left, &rec->objRefs, allocate);
    } else {
        rec->numMisc++;
        IRO_AddElmToList(left, &rec->misc, allocate);
    }

    right = linear->u.diadic.right;
    if (right->type == IROLinearOp2Arg && right->nodetype == EADD) {
        IRO_DecomposeAddressExpression(right, rec, allocate);
    } else if (IRO_IsIntConstant(right)) {
        rec->numInts++;
        IRO_AddElmToList(right, &rec->ints, allocate);
    } else if (right->type == IROLinearOperand && right->u.node->type == EOBJREF) {
        rec->numObjRefs++;
        IRO_AddElmToList(right, &rec->objRefs, allocate);
    } else {
        rec->numMisc++;
        IRO_AddElmToList(right, &rec->misc, allocate);
    }
}

IROLinear* IRO_DuplicateExpr(IROLinear* linear, IROList* list) {
    IROLinear* copy;
    ENode* copynode;
    SInt16 i;

    copy = IRO_NewLinear(linear->type);
    *copy = *linear;
    copy->index = ++IRO_NumLinear;
    copy->next = NULL;
    copy->expr = NULL;

    switch (copy->type) {
        case IROLinearNop:
            break;
        case IROLinearOperand:
            copynode = IRO_NewENode(linear->u.node->type);
            *copynode = *linear->u.node;
            copy->u.node = copynode;
            break;
        case IROLinearOp1Arg:
            copy->u.monadic = IRO_DuplicateExpr(copy->u.monadic, list);
            break;
        case IROLinearOp2Arg:
            if (linear->flags & 0x8000) {
                copy->u.diadic.right = IRO_DuplicateExpr(copy->u.diadic.right, list);
                copy->u.diadic.left = IRO_DuplicateExpr(copy->u.diadic.left, list);
            } else {
                copy->u.diadic.left = IRO_DuplicateExpr(copy->u.diadic.left, list);
                copy->u.diadic.right = IRO_DuplicateExpr(copy->u.diadic.right, list);
            }
            break;
        case IROLinearOp3Arg:
            copy->u.args3.a = IRO_DuplicateExpr(copy->u.args3.a, list);
            copy->u.args3.b = IRO_DuplicateExpr(copy->u.args3.b, list);
            copy->u.args3.c = IRO_DuplicateExpr(copy->u.args3.c, list);
            break;
        case IROLinear5:
            copynode = IRO_NewENode(linear->u.node->type);
            *copynode = *linear->u.node;
            copy->u.node = copynode;
            break;
        case IROLinearFunccall:
            copy->u.funccall.callee = IRO_DuplicateExpr(copy->u.funccall.callee, list);
            copy->u.funccall.args = IRO_oalloc2(sizeof(IROLinear*) * copy->u.funccall.argCount);
            for (i = 0; i < copy->u.funccall.argCount; i++) {
                copy->u.funccall.args[i] = IRO_DuplicateExpr(linear->u.funccall.args[i], list);
            }
            break;
        case IROLinearAsm:
            if (IRO_CopyAsmStat && linear->u.monadic) {
                copy->u.monadic = IRO_CopyAsmStat(linear->u.monadic);
            }
            break;
        default:
            break;
    }
    IRO_AddToList(copy, list);
    return copy;
}

void IRO_ReplaceReference(IROLinear* linear, Object* obj, IROLinear* after) {
    SInt32 index;
    IROLinear* scan;
    SInt32 i;
    IROLinear** link;

    for (scan = linear->next, index = 0; index < 2; index++) {
        while (scan) {
            switch (scan->type) {
                case IROLinearGotoExpr:
                    if (scan->u.monadic != linear) {
                        break;
                    }
                    link = &scan->u.monadic;
                    goto found;
                case IROLinearIf:
                case IROLinearIfNot:
                    if (scan->u.diadic.right != linear) {
                        break;
                    }
                    link = &scan->u.diadic.right;
                    goto found;
                case IROLinearReturn:
                    if (scan->u.monadic != linear) {
                        break;
                    }
                    link = &scan->u.monadic;
                    goto found;
                case IROLinearOp1Arg:
                    if (scan->u.monadic != linear) {
                        break;
                    }
                    link = &scan->u.monadic;
                    goto found;
                case IROLinearSwitch:
                    if (scan->u.diadic.right != linear) {
                        break;
                    }
                    link = &scan->u.diadic.right;
                    goto found;
                case IROLinearOp2Arg:
                    if (scan->u.diadic.left == linear) {
                        link = &scan->u.diadic.left;
                        goto found;
                    }
                    if (scan->u.diadic.right != linear) {
                        break;
                    }
                    link = &scan->u.diadic.right;
                    goto found;
                case IROLinearOp3Arg:
                    if (scan->u.args3.a == linear) {
                        link = &scan->u.args3.a;
                        goto found;
                    }
                    if (scan->u.args3.b == linear) {
                        link = &scan->u.args3.b;
                        goto found;
                    }
                    if (scan->u.args3.c != linear) {
                        break;
                    }
                    link = &scan->u.args3.c;
                    goto found;
                case IROLinearFunccall:
                    if (scan->u.funccall.callee == linear) {
                        link = &scan->u.funccall.callee;
                        goto found;
                    }
                    for (i = 0; i < scan->u.funccall.argCount; i++) {
                        if (scan->u.funccall.args[i] == linear) {
                            link = &scan->u.funccall.args[i];
                            goto found;
                        }
                    }
                    break;
                case IROLinearNop:
                case IROLinearOperand:
                case IROLinear5:
                case IROLinearGoto:
                case IROLinearLabel:
                case 14:
                case 15:
                case IROLinearBeginCatch:
                case IROLinearEndCatch:
                case IROLinearEndCatchDtor:
                case IROLinearAsm:
                case IROLinearEnd:
                    break;
                default:
                    CError_FATAL(3288);
                    break;
            }
            scan = scan->next;
        }
        scan = IRO_FirstLinear;
    }
    link = NULL;
    scan = NULL;
found:
    if (scan) {
        IROLinear* operand;
        IROLinear* indirect;
        IROLinear* tail;
        IRONode* node;
        CError_ASSERT(3355, link && *link);
        operand = IRO_NewLinear(IROLinearOperand);
        operand->nodetype = EOBJREF;
        operand->u.node = create_objectrefnode(obj);
        operand->rtype = IRO_CopyType(operand->u.node->rtype);
        operand->index = ++IRO_NumLinear;
        operand->flags |= 0x22;
        indirect = IRO_NewLinear(IROLinearOp1Arg);
        operand->next = indirect;
        indirect->nodetype = EINDIRECT;
        indirect->rtype = IRO_CopyType(obj->type);
        indirect->u.monadic = operand;
        indirect->index = ++IRO_NumLinear;
        indirect->next = NULL;
        *link = indirect;
        (*link)->rtype = linear->rtype;
        tail = (*link)->u.monadic;
        while (tail->next) {
            tail = tail->next;
        }
        for (node = IRO_FirstNode; node; node = node->nextnode) {
            if (node->last == after) {
                node->last = tail;
                break;
            }
        }
        tail->next = after->next;
        after->next = indirect;
        tail->flags |= IROLF_Reffed;
        if (linear->flags & 0x100) {
            tail->flags |= 0x100;
        }
    } else {
        IRO_Dump("Oh, oh, did not find reference to replace\n");
    }
}

void IRO_InitializeIRO_IsCommutativeENodeTypeArray(void) {
    SInt32 i;
    for (i = 0; i < IRO_MAXEXPR; i++) {
        IRO_IsCommutativeENodeTypeArray[i] = 0;
    }
    IRO_IsCommutativeENodeTypeArray[0] = 0;
    IRO_IsCommutativeENodeTypeArray[1] = 0;
    IRO_IsCommutativeENodeTypeArray[2] = 0;
    IRO_IsCommutativeENodeTypeArray[3] = 0;
    IRO_IsCommutativeENodeTypeArray[4] = 0;
    IRO_IsCommutativeENodeTypeArray[5] = 0;
    IRO_IsCommutativeENodeTypeArray[6] = 0;
    IRO_IsCommutativeENodeTypeArray[7] = 0;
    IRO_IsCommutativeENodeTypeArray[8] = 0;
    IRO_IsCommutativeENodeTypeArray[9] = 1;
    IRO_IsCommutativeENodeTypeArray[10] = 1;
    IRO_IsCommutativeENodeTypeArray[11] = 0;
    IRO_IsCommutativeENodeTypeArray[12] = 0;
    IRO_IsCommutativeENodeTypeArray[13] = 1;
    IRO_IsCommutativeENodeTypeArray[14] = 0;
    IRO_IsCommutativeENodeTypeArray[15] = 1;
    IRO_IsCommutativeENodeTypeArray[16] = 0;
    IRO_IsCommutativeENodeTypeArray[17] = 0;
    IRO_IsCommutativeENodeTypeArray[18] = 0;
    IRO_IsCommutativeENodeTypeArray[19] = 0;
    IRO_IsCommutativeENodeTypeArray[20] = 0;
    IRO_IsCommutativeENodeTypeArray[21] = 0;
    IRO_IsCommutativeENodeTypeArray[22] = 0;
    IRO_IsCommutativeENodeTypeArray[23] = 1;
    IRO_IsCommutativeENodeTypeArray[24] = 1;
    IRO_IsCommutativeENodeTypeArray[25] = 1;
    IRO_IsCommutativeENodeTypeArray[26] = 1;
    IRO_IsCommutativeENodeTypeArray[27] = 1;
    IRO_IsCommutativeENodeTypeArray[28] = 0;
    IRO_IsCommutativeENodeTypeArray[29] = 0;
    IRO_IsCommutativeENodeTypeArray[30] = 0;
    IRO_IsCommutativeENodeTypeArray[31] = 0;
    IRO_IsCommutativeENodeTypeArray[32] = 0;
    IRO_IsCommutativeENodeTypeArray[33] = 0;
    IRO_IsCommutativeENodeTypeArray[34] = 0;
    IRO_IsCommutativeENodeTypeArray[35] = 0;
    IRO_IsCommutativeENodeTypeArray[36] = 0;
    IRO_IsCommutativeENodeTypeArray[37] = 0;
    IRO_IsCommutativeENodeTypeArray[38] = 0;
    IRO_IsCommutativeENodeTypeArray[39] = 0;
    IRO_IsCommutativeENodeTypeArray[40] = 0;
    IRO_IsCommutativeENodeTypeArray[41] = 0;
    IRO_IsCommutativeENodeTypeArray[42] = 1;
    IRO_IsCommutativeENodeTypeArray[43] = 1;
    IRO_IsCommutativeENodeTypeArray[44] = 0;
    IRO_IsCommutativeENodeTypeArray[45] = 0;
    IRO_IsCommutativeENodeTypeArray[46] = 0;
    IRO_IsCommutativeENodeTypeArray[47] = 0;
    IRO_IsCommutativeENodeTypeArray[48] = 0;
    IRO_IsCommutativeENodeTypeArray[49] = 0;
    IRO_IsCommutativeENodeTypeArray[50] = 0;
    IRO_IsCommutativeENodeTypeArray[51] = 0;
    IRO_IsCommutativeENodeTypeArray[52] = 0;
    IRO_IsCommutativeENodeTypeArray[53] = 0;
    IRO_IsCommutativeENodeTypeArray[54] = 0;
    IRO_IsCommutativeENodeTypeArray[55] = 0;
    IRO_IsCommutativeENodeTypeArray[56] = 0;
    IRO_IsCommutativeENodeTypeArray[57] = 0;
    IRO_IsCommutativeENodeTypeArray[58] = 0;
    IRO_IsCommutativeENodeTypeArray[59] = 0;
    IRO_IsCommutativeENodeTypeArray[60] = 0;
    IRO_IsCommutativeENodeTypeArray[61] = 0;
    IRO_IsCommutativeENodeTypeArray[62] = 0;
    IRO_IsCommutativeENodeTypeArray[63] = 0;
    IRO_IsCommutativeENodeTypeArray[64] = 0;
    IRO_IsCommutativeENodeTypeArray[65] = 0;
    IRO_IsCommutativeENodeTypeArray[66] = 0;
    IRO_IsCommutativeENodeTypeArray[67] = 0;
    IRO_IsCommutativeENodeTypeArray[68] = 0;
    IRO_IsCommutativeENodeTypeArray[69] = 0;
    IRO_IsCommutativeENodeTypeArray[70] = 0;
    IRO_IsCommutativeENodeTypeArray[71] = 0;
    IRO_IsCommutativeENodeTypeArray[72] = 0;
    IRO_IsCommutativeENodeTypeArray[73] = 0;
    IRO_IsCommutativeENodeTypeArray[74] = 0;
    IRO_IsCommutativeENodeTypeArray[75] = 0;
    IRO_IsCommutativeENodeTypeArray[76] = 0;
    IRO_IsCommutativeENodeTypeArray[77] = 0;
    IRO_IsCommutativeENodeTypeArray[78] = 0;
    IRO_IsCommutativeENodeTypeArray[79] = 0;
    IRO_IsCommutativeENodeTypeArray[80] = 0;
    IRO_IsCommutativeENodeTypeArray[81] = 0;
    IRO_IsCommutativeENodeTypeArray[82] = 0;
}

void IRO_InitializeIRO_IsAssociativeENodeTypeArray(void) {
    SInt32 i;
    for (i = 0; i < IRO_MAXEXPR; i++) {
        IRO_IsAssociativeENodeTypeArray[i] = 0;
    }
    IRO_IsAssociativeENodeTypeArray[0] = 0;
    IRO_IsAssociativeENodeTypeArray[1] = 0;
    IRO_IsAssociativeENodeTypeArray[2] = 0;
    IRO_IsAssociativeENodeTypeArray[3] = 0;
    IRO_IsAssociativeENodeTypeArray[4] = 0;
    IRO_IsAssociativeENodeTypeArray[5] = 0;
    IRO_IsAssociativeENodeTypeArray[6] = 0;
    IRO_IsAssociativeENodeTypeArray[7] = 0;
    IRO_IsAssociativeENodeTypeArray[8] = 0;
    IRO_IsAssociativeENodeTypeArray[9] = 1;
    IRO_IsAssociativeENodeTypeArray[10] = 1;
    IRO_IsAssociativeENodeTypeArray[11] = 0;
    IRO_IsAssociativeENodeTypeArray[12] = 0;
    IRO_IsAssociativeENodeTypeArray[13] = 1;
    IRO_IsAssociativeENodeTypeArray[14] = 0;
    IRO_IsAssociativeENodeTypeArray[15] = 1;
    IRO_IsAssociativeENodeTypeArray[16] = 0;
    IRO_IsAssociativeENodeTypeArray[17] = 0;
    IRO_IsAssociativeENodeTypeArray[18] = 0;
    IRO_IsAssociativeENodeTypeArray[19] = 0;
    IRO_IsAssociativeENodeTypeArray[20] = 0;
    IRO_IsAssociativeENodeTypeArray[21] = 0;
    IRO_IsAssociativeENodeTypeArray[22] = 0;
    IRO_IsAssociativeENodeTypeArray[23] = 0;
    IRO_IsAssociativeENodeTypeArray[24] = 0;
    IRO_IsAssociativeENodeTypeArray[25] = 1;
    IRO_IsAssociativeENodeTypeArray[26] = 1;
    IRO_IsAssociativeENodeTypeArray[27] = 1;
    IRO_IsAssociativeENodeTypeArray[28] = 0;
    IRO_IsAssociativeENodeTypeArray[29] = 0;
    IRO_IsAssociativeENodeTypeArray[30] = 0;
    IRO_IsAssociativeENodeTypeArray[31] = 0;
    IRO_IsAssociativeENodeTypeArray[32] = 0;
    IRO_IsAssociativeENodeTypeArray[33] = 0;
    IRO_IsAssociativeENodeTypeArray[34] = 0;
    IRO_IsAssociativeENodeTypeArray[35] = 0;
    IRO_IsAssociativeENodeTypeArray[36] = 0;
    IRO_IsAssociativeENodeTypeArray[37] = 0;
    IRO_IsAssociativeENodeTypeArray[38] = 0;
    IRO_IsAssociativeENodeTypeArray[39] = 0;
    IRO_IsAssociativeENodeTypeArray[40] = 0;
    IRO_IsAssociativeENodeTypeArray[41] = 0;
    IRO_IsAssociativeENodeTypeArray[42] = 1;
    IRO_IsAssociativeENodeTypeArray[43] = 1;
    IRO_IsAssociativeENodeTypeArray[44] = 0;
    IRO_IsAssociativeENodeTypeArray[45] = 0;
    IRO_IsAssociativeENodeTypeArray[46] = 0;
    IRO_IsAssociativeENodeTypeArray[47] = 0;
    IRO_IsAssociativeENodeTypeArray[48] = 0;
    IRO_IsAssociativeENodeTypeArray[49] = 0;
    IRO_IsAssociativeENodeTypeArray[50] = 0;
    IRO_IsAssociativeENodeTypeArray[51] = 0;
    IRO_IsAssociativeENodeTypeArray[52] = 0;
    IRO_IsAssociativeENodeTypeArray[53] = 0;
    IRO_IsAssociativeENodeTypeArray[54] = 0;
    IRO_IsAssociativeENodeTypeArray[55] = 0;
    IRO_IsAssociativeENodeTypeArray[56] = 0;
    IRO_IsAssociativeENodeTypeArray[57] = 0;
    IRO_IsAssociativeENodeTypeArray[58] = 0;
    IRO_IsAssociativeENodeTypeArray[59] = 0;
    IRO_IsAssociativeENodeTypeArray[60] = 0;
    IRO_IsAssociativeENodeTypeArray[61] = 0;
    IRO_IsAssociativeENodeTypeArray[62] = 0;
    IRO_IsAssociativeENodeTypeArray[63] = 0;
    IRO_IsAssociativeENodeTypeArray[64] = 0;
    IRO_IsAssociativeENodeTypeArray[65] = 0;
    IRO_IsAssociativeENodeTypeArray[66] = 0;
    IRO_IsAssociativeENodeTypeArray[67] = 0;
    IRO_IsAssociativeENodeTypeArray[68] = 0;
    IRO_IsAssociativeENodeTypeArray[69] = 0;
    IRO_IsAssociativeENodeTypeArray[70] = 0;
    IRO_IsAssociativeENodeTypeArray[71] = 0;
    IRO_IsAssociativeENodeTypeArray[72] = 0;
    IRO_IsAssociativeENodeTypeArray[73] = 0;
    IRO_IsAssociativeENodeTypeArray[74] = 0;
    IRO_IsAssociativeENodeTypeArray[75] = 0;
    IRO_IsAssociativeENodeTypeArray[76] = 0;
    IRO_IsAssociativeENodeTypeArray[77] = 0;
    IRO_IsAssociativeENodeTypeArray[78] = 0;
    IRO_IsAssociativeENodeTypeArray[79] = 0;
    IRO_IsAssociativeENodeTypeArray[80] = 0;
    IRO_IsAssociativeENodeTypeArray[81] = 0;
    IRO_IsAssociativeENodeTypeArray[82] = 0;
}

void IRO_InitializeIRO_ComplementaryLogicalComparisonArray(void) {
    SInt32 i;
    for (i = 0; i < IRO_MAXEXPR; i++) {
        IRO_ComplementaryLogicalComparison[i] = IRO_MAXEXPR;
    }
    IRO_ComplementaryLogicalComparison[0] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[1] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[2] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[3] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[4] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[5] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[6] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[7] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[8] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[9] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[10] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[11] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[12] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[13] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[14] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[15] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[16] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[17] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[18] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[19] = 22;
    IRO_ComplementaryLogicalComparison[20] = 21;
    IRO_ComplementaryLogicalComparison[21] = 20;
    IRO_ComplementaryLogicalComparison[22] = 19;
    IRO_ComplementaryLogicalComparison[23] = 24;
    IRO_ComplementaryLogicalComparison[24] = 23;
    IRO_ComplementaryLogicalComparison[25] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[26] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[27] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[28] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[29] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[30] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[31] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[32] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[33] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[34] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[35] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[36] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[37] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[38] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[39] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[40] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[41] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[42] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[43] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[44] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[45] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[46] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[47] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[48] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[49] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[50] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[51] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[52] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[53] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[54] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[55] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[56] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[57] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[58] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[59] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[60] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[61] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[62] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[63] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[64] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[65] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[66] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[67] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[68] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[69] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[70] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[71] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[72] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[73] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[74] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[75] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[76] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[77] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[78] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[79] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[80] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[81] = IRO_MAXEXPR;
    IRO_ComplementaryLogicalComparison[82] = IRO_MAXEXPR;
}

void IRO_InitializeIRO_ComplementaryOpArray(void) {
    SInt32 i;
    for (i = 0; i < IRO_MAXEXPR; i++) {
        IRO_ComplementaryOp[i] = IRO_MAXEXPR;
    }
    IRO_ComplementaryOp[0] = 1;
    IRO_ComplementaryOp[1] = 0;
    IRO_ComplementaryOp[2] = 3;
    IRO_ComplementaryOp[3] = 2;
    IRO_ComplementaryOp[4] = IRO_MAXEXPR;
    IRO_ComplementaryOp[5] = 5;
    IRO_ComplementaryOp[6] = 6;
    IRO_ComplementaryOp[7] = 7;
    IRO_ComplementaryOp[8] = IRO_MAXEXPR;
    IRO_ComplementaryOp[9] = 11;
    IRO_ComplementaryOp[10] = IRO_MAXEXPR;
    IRO_ComplementaryOp[11] = 9;
    IRO_ComplementaryOp[12] = IRO_MAXEXPR;
    IRO_ComplementaryOp[13] = 14;
    IRO_ComplementaryOp[14] = 13;
    IRO_ComplementaryOp[15] = 16;
    IRO_ComplementaryOp[16] = 15;
    IRO_ComplementaryOp[17] = 18;
    IRO_ComplementaryOp[18] = 17;
    IRO_ComplementaryOp[19] = 20;
    IRO_ComplementaryOp[20] = 19;
    IRO_ComplementaryOp[21] = 22;
    IRO_ComplementaryOp[22] = 21;
    IRO_ComplementaryOp[23] = 24;
    IRO_ComplementaryOp[24] = 23;
    IRO_ComplementaryOp[25] = 27;
    IRO_ComplementaryOp[26] = IRO_MAXEXPR;
    IRO_ComplementaryOp[27] = 25;
    IRO_ComplementaryOp[28] = 29;
    IRO_ComplementaryOp[29] = 28;
    IRO_ComplementaryOp[30] = IRO_MAXEXPR;
    IRO_ComplementaryOp[31] = 32;
    IRO_ComplementaryOp[32] = 31;
    IRO_ComplementaryOp[33] = IRO_MAXEXPR;
    IRO_ComplementaryOp[34] = 35;
    IRO_ComplementaryOp[35] = 34;
    IRO_ComplementaryOp[36] = 37;
    IRO_ComplementaryOp[37] = 36;
    IRO_ComplementaryOp[38] = 40;
    IRO_ComplementaryOp[39] = IRO_MAXEXPR;
    IRO_ComplementaryOp[40] = 38;
    IRO_ComplementaryOp[41] = IRO_MAXEXPR;
    IRO_ComplementaryOp[42] = IRO_MAXEXPR;
    IRO_ComplementaryOp[43] = IRO_MAXEXPR;
    IRO_ComplementaryOp[44] = IRO_MAXEXPR;
    IRO_ComplementaryOp[45] = 46;
    IRO_ComplementaryOp[46] = 45;
    IRO_ComplementaryOp[47] = IRO_MAXEXPR;
    IRO_ComplementaryOp[48] = IRO_MAXEXPR;
    IRO_ComplementaryOp[49] = IRO_MAXEXPR;
    IRO_ComplementaryOp[50] = IRO_MAXEXPR;
    IRO_ComplementaryOp[51] = IRO_MAXEXPR;
    IRO_ComplementaryOp[52] = IRO_MAXEXPR;
    IRO_ComplementaryOp[53] = IRO_MAXEXPR;
    IRO_ComplementaryOp[54] = IRO_MAXEXPR;
    IRO_ComplementaryOp[55] = IRO_MAXEXPR;
    IRO_ComplementaryOp[56] = IRO_MAXEXPR;
    IRO_ComplementaryOp[57] = IRO_MAXEXPR;
    IRO_ComplementaryOp[58] = IRO_MAXEXPR;
    IRO_ComplementaryOp[59] = IRO_MAXEXPR;
    IRO_ComplementaryOp[60] = IRO_MAXEXPR;
    IRO_ComplementaryOp[61] = IRO_MAXEXPR;
    IRO_ComplementaryOp[62] = IRO_MAXEXPR;
    IRO_ComplementaryOp[63] = IRO_MAXEXPR;
    IRO_ComplementaryOp[64] = IRO_MAXEXPR;
    IRO_ComplementaryOp[65] = IRO_MAXEXPR;
    IRO_ComplementaryOp[66] = IRO_MAXEXPR;
    IRO_ComplementaryOp[67] = IRO_MAXEXPR;
    IRO_ComplementaryOp[68] = IRO_MAXEXPR;
    IRO_ComplementaryOp[69] = IRO_MAXEXPR;
    IRO_ComplementaryOp[70] = IRO_MAXEXPR;
    IRO_ComplementaryOp[71] = IRO_MAXEXPR;
    IRO_ComplementaryOp[72] = IRO_MAXEXPR;
    IRO_ComplementaryOp[73] = IRO_MAXEXPR;
    IRO_ComplementaryOp[74] = IRO_MAXEXPR;
    IRO_ComplementaryOp[75] = IRO_MAXEXPR;
    IRO_ComplementaryOp[76] = IRO_MAXEXPR;
    IRO_ComplementaryOp[77] = IRO_MAXEXPR;
    IRO_ComplementaryOp[78] = IRO_MAXEXPR;
    IRO_ComplementaryOp[79] = IRO_MAXEXPR;
    IRO_ComplementaryOp[80] = IRO_MAXEXPR;
    IRO_ComplementaryOp[81] = IRO_MAXEXPR;
    IRO_ComplementaryOp[82] = IRO_MAXEXPR;
}

void IRO_InitializeIRO_AssignmentOpArray(void) {
    SInt32 i;
    for (i = 0; i < IRO_MAXEXPR; i++) {
        IRO_AssignmentOp[i] = IRO_MAXEXPR;
    }
    IRO_AssignmentOp[0] = IRO_MAXEXPR;
    IRO_AssignmentOp[1] = IRO_MAXEXPR;
    IRO_AssignmentOp[2] = IRO_MAXEXPR;
    IRO_AssignmentOp[3] = IRO_MAXEXPR;
    IRO_AssignmentOp[4] = IRO_MAXEXPR;
    IRO_AssignmentOp[5] = IRO_MAXEXPR;
    IRO_AssignmentOp[6] = IRO_MAXEXPR;
    IRO_AssignmentOp[7] = IRO_MAXEXPR;
    IRO_AssignmentOp[8] = IRO_MAXEXPR;
    IRO_AssignmentOp[9] = 31;
    IRO_AssignmentOp[10] = IRO_MAXEXPR;
    IRO_AssignmentOp[11] = 32;
    IRO_AssignmentOp[12] = 33;
    IRO_AssignmentOp[13] = IRO_MAXEXPR;
    IRO_AssignmentOp[14] = IRO_MAXEXPR;
    IRO_AssignmentOp[15] = 34;
    IRO_AssignmentOp[16] = 35;
    IRO_AssignmentOp[17] = 36;
    IRO_AssignmentOp[18] = 37;
    IRO_AssignmentOp[19] = IRO_MAXEXPR;
    IRO_AssignmentOp[20] = IRO_MAXEXPR;
    IRO_AssignmentOp[21] = IRO_MAXEXPR;
    IRO_AssignmentOp[22] = IRO_MAXEXPR;
    IRO_AssignmentOp[23] = IRO_MAXEXPR;
    IRO_AssignmentOp[24] = IRO_MAXEXPR;
    IRO_AssignmentOp[25] = 38;
    IRO_AssignmentOp[26] = 39;
    IRO_AssignmentOp[27] = 40;
    IRO_AssignmentOp[28] = IRO_MAXEXPR;
    IRO_AssignmentOp[29] = IRO_MAXEXPR;
    IRO_AssignmentOp[30] = IRO_MAXEXPR;
    IRO_AssignmentOp[31] = IRO_MAXEXPR;
    IRO_AssignmentOp[32] = IRO_MAXEXPR;
    IRO_AssignmentOp[33] = IRO_MAXEXPR;
    IRO_AssignmentOp[34] = IRO_MAXEXPR;
    IRO_AssignmentOp[35] = IRO_MAXEXPR;
    IRO_AssignmentOp[36] = IRO_MAXEXPR;
    IRO_AssignmentOp[37] = IRO_MAXEXPR;
    IRO_AssignmentOp[38] = IRO_MAXEXPR;
    IRO_AssignmentOp[39] = IRO_MAXEXPR;
    IRO_AssignmentOp[40] = IRO_MAXEXPR;
    IRO_AssignmentOp[41] = IRO_MAXEXPR;
    IRO_AssignmentOp[42] = IRO_MAXEXPR;
    IRO_AssignmentOp[43] = IRO_MAXEXPR;
    IRO_AssignmentOp[44] = IRO_MAXEXPR;
    IRO_AssignmentOp[45] = IRO_MAXEXPR;
    IRO_AssignmentOp[46] = IRO_MAXEXPR;
    IRO_AssignmentOp[47] = IRO_MAXEXPR;
    IRO_AssignmentOp[48] = IRO_MAXEXPR;
    IRO_AssignmentOp[49] = IRO_MAXEXPR;
    IRO_AssignmentOp[50] = IRO_MAXEXPR;
    IRO_AssignmentOp[51] = IRO_MAXEXPR;
    IRO_AssignmentOp[52] = IRO_MAXEXPR;
    IRO_AssignmentOp[53] = IRO_MAXEXPR;
    IRO_AssignmentOp[54] = IRO_MAXEXPR;
    IRO_AssignmentOp[55] = IRO_MAXEXPR;
    IRO_AssignmentOp[56] = IRO_MAXEXPR;
    IRO_AssignmentOp[57] = IRO_MAXEXPR;
    IRO_AssignmentOp[58] = IRO_MAXEXPR;
    IRO_AssignmentOp[59] = IRO_MAXEXPR;
    IRO_AssignmentOp[60] = IRO_MAXEXPR;
    IRO_AssignmentOp[61] = IRO_MAXEXPR;
    IRO_AssignmentOp[62] = IRO_MAXEXPR;
    IRO_AssignmentOp[63] = IRO_MAXEXPR;
    IRO_AssignmentOp[64] = IRO_MAXEXPR;
    IRO_AssignmentOp[65] = IRO_MAXEXPR;
    IRO_AssignmentOp[66] = IRO_MAXEXPR;
    IRO_AssignmentOp[67] = IRO_MAXEXPR;
    IRO_AssignmentOp[68] = IRO_MAXEXPR;
    IRO_AssignmentOp[69] = IRO_MAXEXPR;
    IRO_AssignmentOp[70] = IRO_MAXEXPR;
    IRO_AssignmentOp[71] = IRO_MAXEXPR;
    IRO_AssignmentOp[72] = IRO_MAXEXPR;
    IRO_AssignmentOp[73] = IRO_MAXEXPR;
    IRO_AssignmentOp[74] = IRO_MAXEXPR;
    IRO_AssignmentOp[75] = IRO_MAXEXPR;
    IRO_AssignmentOp[76] = IRO_MAXEXPR;
    IRO_AssignmentOp[77] = IRO_MAXEXPR;
    IRO_AssignmentOp[78] = IRO_MAXEXPR;
    IRO_AssignmentOp[79] = IRO_MAXEXPR;
    IRO_AssignmentOp[80] = IRO_MAXEXPR;
    IRO_AssignmentOp[81] = IRO_MAXEXPR;
    IRO_AssignmentOp[82] = IRO_MAXEXPR;
}

void IRO_InitializeIRO_NonAssignmentOpArray(void) {
    SInt32 i;
    for (i = 0; i < IRO_MAXEXPR; i++) {
        IRO_NonAssignmentOp[i] = IRO_MAXEXPR;
    }
    IRO_NonAssignmentOp[0] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[1] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[2] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[3] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[4] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[5] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[6] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[7] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[8] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[9] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[10] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[11] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[12] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[13] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[14] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[15] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[16] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[17] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[18] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[19] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[20] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[21] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[22] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[23] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[24] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[25] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[26] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[27] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[28] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[29] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[30] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[31] = 9;
    IRO_NonAssignmentOp[32] = 11;
    IRO_NonAssignmentOp[33] = 12;
    IRO_NonAssignmentOp[34] = 15;
    IRO_NonAssignmentOp[35] = 16;
    IRO_NonAssignmentOp[36] = 17;
    IRO_NonAssignmentOp[37] = 18;
    IRO_NonAssignmentOp[38] = 25;
    IRO_NonAssignmentOp[39] = 26;
    IRO_NonAssignmentOp[40] = 27;
    IRO_NonAssignmentOp[41] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[42] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[43] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[44] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[45] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[46] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[47] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[48] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[49] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[50] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[51] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[52] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[53] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[54] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[55] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[56] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[57] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[58] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[59] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[60] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[61] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[62] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[63] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[64] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[65] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[66] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[67] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[68] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[69] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[70] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[71] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[72] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[73] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[74] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[75] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[76] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[77] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[78] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[79] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[80] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[81] = IRO_MAXEXPR;
    IRO_NonAssignmentOp[82] = IRO_MAXEXPR;
}

void IRO_InitializeIRO_IsModifyOpArray(void) {
    SInt32 i;
    for (i = 0; i < IRO_MAXEXPR; i++) {
        IRO_IsModifyOp[i] = 0;
    }
    IRO_IsModifyOp[0] = 1;
    IRO_IsModifyOp[1] = 1;
    IRO_IsModifyOp[2] = 1;
    IRO_IsModifyOp[3] = 1;
    IRO_IsModifyOp[4] = 0;
    IRO_IsModifyOp[5] = 0;
    IRO_IsModifyOp[6] = 0;
    IRO_IsModifyOp[7] = 0;
    IRO_IsModifyOp[8] = 0;
    IRO_IsModifyOp[9] = 0;
    IRO_IsModifyOp[10] = 0;
    IRO_IsModifyOp[11] = 0;
    IRO_IsModifyOp[12] = 0;
    IRO_IsModifyOp[13] = 0;
    IRO_IsModifyOp[14] = 0;
    IRO_IsModifyOp[15] = 0;
    IRO_IsModifyOp[16] = 0;
    IRO_IsModifyOp[17] = 0;
    IRO_IsModifyOp[18] = 0;
    IRO_IsModifyOp[19] = 0;
    IRO_IsModifyOp[20] = 0;
    IRO_IsModifyOp[21] = 0;
    IRO_IsModifyOp[22] = 0;
    IRO_IsModifyOp[23] = 0;
    IRO_IsModifyOp[24] = 0;
    IRO_IsModifyOp[25] = 0;
    IRO_IsModifyOp[26] = 0;
    IRO_IsModifyOp[27] = 0;
    IRO_IsModifyOp[28] = 0;
    IRO_IsModifyOp[29] = 0;
    IRO_IsModifyOp[30] = 0;
    IRO_IsModifyOp[31] = 1;
    IRO_IsModifyOp[32] = 1;
    IRO_IsModifyOp[33] = 1;
    IRO_IsModifyOp[34] = 1;
    IRO_IsModifyOp[35] = 1;
    IRO_IsModifyOp[36] = 1;
    IRO_IsModifyOp[37] = 1;
    IRO_IsModifyOp[38] = 1;
    IRO_IsModifyOp[39] = 1;
    IRO_IsModifyOp[40] = 1;
    IRO_IsModifyOp[41] = 0;
    IRO_IsModifyOp[42] = 0;
    IRO_IsModifyOp[43] = 0;
    IRO_IsModifyOp[44] = 1;
    IRO_IsModifyOp[45] = 1;
    IRO_IsModifyOp[46] = 1;
    IRO_IsModifyOp[47] = 1;
    IRO_IsModifyOp[48] = 1;
    IRO_IsModifyOp[49] = 1;
    IRO_IsModifyOp[50] = 0;
    IRO_IsModifyOp[51] = 0;
    IRO_IsModifyOp[52] = 0;
    IRO_IsModifyOp[53] = 0;
    IRO_IsModifyOp[54] = 0;
    IRO_IsModifyOp[55] = 0;
    IRO_IsModifyOp[56] = 0;
    IRO_IsModifyOp[57] = 0;
    IRO_IsModifyOp[58] = 0;
    IRO_IsModifyOp[59] = 0;
    IRO_IsModifyOp[60] = 0;
    IRO_IsModifyOp[61] = 0;
    IRO_IsModifyOp[62] = 0;
    IRO_IsModifyOp[63] = 0;
    IRO_IsModifyOp[64] = 0;
    IRO_IsModifyOp[65] = 0;
    IRO_IsModifyOp[66] = 0;
    IRO_IsModifyOp[67] = 0;
    IRO_IsModifyOp[68] = 0;
    IRO_IsModifyOp[69] = 0;
    IRO_IsModifyOp[70] = 0;
    IRO_IsModifyOp[71] = 0;
    IRO_IsModifyOp[72] = 0;
    IRO_IsModifyOp[73] = 0;
    IRO_IsModifyOp[74] = 0;
    IRO_IsModifyOp[75] = 0;
    IRO_IsModifyOp[76] = 0;
    IRO_IsModifyOp[77] = 0;
    IRO_IsModifyOp[78] = 0;
    IRO_IsModifyOp[79] = 0;
    IRO_IsModifyOp[80] = 0;
    IRO_IsModifyOp[81] = 0;
    IRO_IsModifyOp[82] = 1;
}

void IRO_InitializeIRO_IsAssignOpArray(void) {
    SInt32 i;
    for (i = 0; i < IRO_MAXEXPR; i++) {
        IRO_IsAssignOp[i] = 0;
    }
    IRO_IsAssignOp[0] = 1;
    IRO_IsAssignOp[1] = 1;
    IRO_IsAssignOp[2] = 1;
    IRO_IsAssignOp[3] = 1;
    IRO_IsAssignOp[4] = 0;
    IRO_IsAssignOp[5] = 0;
    IRO_IsAssignOp[6] = 0;
    IRO_IsAssignOp[7] = 0;
    IRO_IsAssignOp[8] = 0;
    IRO_IsAssignOp[9] = 0;
    IRO_IsAssignOp[10] = 0;
    IRO_IsAssignOp[11] = 0;
    IRO_IsAssignOp[12] = 0;
    IRO_IsAssignOp[13] = 0;
    IRO_IsAssignOp[14] = 0;
    IRO_IsAssignOp[15] = 0;
    IRO_IsAssignOp[16] = 0;
    IRO_IsAssignOp[17] = 0;
    IRO_IsAssignOp[18] = 0;
    IRO_IsAssignOp[19] = 0;
    IRO_IsAssignOp[20] = 0;
    IRO_IsAssignOp[21] = 0;
    IRO_IsAssignOp[22] = 0;
    IRO_IsAssignOp[23] = 0;
    IRO_IsAssignOp[24] = 0;
    IRO_IsAssignOp[25] = 0;
    IRO_IsAssignOp[26] = 0;
    IRO_IsAssignOp[27] = 0;
    IRO_IsAssignOp[28] = 0;
    IRO_IsAssignOp[29] = 0;
    IRO_IsAssignOp[30] = 1;
    IRO_IsAssignOp[31] = 1;
    IRO_IsAssignOp[32] = 1;
    IRO_IsAssignOp[33] = 1;
    IRO_IsAssignOp[34] = 1;
    IRO_IsAssignOp[35] = 1;
    IRO_IsAssignOp[36] = 1;
    IRO_IsAssignOp[37] = 1;
    IRO_IsAssignOp[38] = 1;
    IRO_IsAssignOp[39] = 1;
    IRO_IsAssignOp[40] = 1;
    IRO_IsAssignOp[41] = 0;
    IRO_IsAssignOp[42] = 0;
    IRO_IsAssignOp[43] = 0;
    IRO_IsAssignOp[44] = 1;
    IRO_IsAssignOp[45] = 0;
    IRO_IsAssignOp[46] = 0;
    IRO_IsAssignOp[47] = 1;
    IRO_IsAssignOp[48] = 1;
    IRO_IsAssignOp[49] = 1;
    IRO_IsAssignOp[50] = 0;
    IRO_IsAssignOp[51] = 0;
    IRO_IsAssignOp[52] = 0;
    IRO_IsAssignOp[53] = 0;
    IRO_IsAssignOp[54] = 0;
    IRO_IsAssignOp[55] = 0;
    IRO_IsAssignOp[56] = 0;
    IRO_IsAssignOp[57] = 0;
    IRO_IsAssignOp[58] = 0;
    IRO_IsAssignOp[59] = 0;
    IRO_IsAssignOp[60] = 0;
    IRO_IsAssignOp[61] = 0;
    IRO_IsAssignOp[62] = 0;
    IRO_IsAssignOp[63] = 0;
    IRO_IsAssignOp[64] = 0;
    IRO_IsAssignOp[65] = 0;
    IRO_IsAssignOp[66] = 0;
    IRO_IsAssignOp[67] = 0;
    IRO_IsAssignOp[68] = 0;
    IRO_IsAssignOp[69] = 0;
    IRO_IsAssignOp[70] = 0;
    IRO_IsAssignOp[71] = 0;
    IRO_IsAssignOp[72] = 0;
    IRO_IsAssignOp[73] = 0;
    IRO_IsAssignOp[74] = 0;
    IRO_IsAssignOp[75] = 0;
    IRO_IsAssignOp[76] = 0;
    IRO_IsAssignOp[77] = 0;
    IRO_IsAssignOp[78] = 0;
    IRO_IsAssignOp[79] = 0;
    IRO_IsAssignOp[80] = 0;
    IRO_IsAssignOp[81] = 0;
    IRO_IsAssignOp[82] = 1;
}

IROLinear* IRO_AssignToTemp(Object* obj, IROLinear* expression, IROList* list) {
    IROLinear* operand;
    IROLinear* indirect;
    IROLinear* assign;

    operand = IRO_NewLinear(IROLinearOperand);
    operand->nodetype = EOBJREF;
    operand->u.node = create_objectrefnode(obj);
    operand->rtype = IRO_CopyType(operand->u.node->rtype);
    operand->index = ++IRO_NumLinear;
    operand->flags |= 0x26;

    indirect = IRO_NewLinear(IROLinearOp1Arg);
    indirect->nodetype = EINDIRECT;
    indirect->rtype = IRO_CopyType(expression->rtype);
    indirect->u.monadic = operand;
    indirect->index = ++IRO_NumLinear;
    indirect->flags |= 6;

    assign = IRO_NewLinear(IROLinearOp2Arg);
    assign->nodetype = EASS;
    assign->u.diadic.left = indirect;
    assign->u.diadic.right = expression;
    assign->rtype = IRO_CopyType(expression->rtype);
    assign->index = ++IRO_NumLinear;
    operand->next = indirect;
    indirect->next = assign;

    IRO_AddToList(operand, list);
    return assign;
}

void IRO_ReplaceReferenceWithNode(IROLinear* linear, IROLinear* replacement) {
    IROLinear** link;
    SInt32 i;
    volatile SInt32 index;
    IROLinear* scan;

    for (scan = linear->next, index = 0; index < 2; index++) {
        while (scan) {
            switch (scan->type) {
                case IROLinearGotoExpr:
                    if (scan->u.monadic != linear) {
                        break;
                    }
                    link = &scan->u.monadic;
                    goto found;
                case IROLinearIf:
                case IROLinearIfNot:
                    if (scan->u.diadic.right != linear) {
                        break;
                    }
                    link = &scan->u.diadic.right;
                    goto found;
                case IROLinearReturn:
                    if (scan->u.monadic != linear) {
                        break;
                    }
                    link = &scan->u.monadic;
                    goto found;
                case IROLinearOp1Arg:
                    if (scan->u.monadic != linear) {
                        break;
                    }
                    link = &scan->u.monadic;
                    goto found;
                case IROLinearSwitch:
                    if (scan->u.diadic.right != linear) {
                        break;
                    }
                    link = &scan->u.diadic.right;
                    goto found;
                case IROLinearOp2Arg:
                    if (scan->u.diadic.left == linear) {
                        link = &scan->u.diadic.left;
                        goto found;
                    }
                    if (scan->u.diadic.right != linear) {
                        break;
                    }
                    link = &scan->u.diadic.right;
                    goto found;
                case IROLinearOp3Arg:
                    if (scan->u.args3.a == linear) {
                        link = &scan->u.args3.a;
                        goto found;
                    }
                    if (scan->u.args3.b == linear) {
                        link = &scan->u.args3.b;
                        goto found;
                    }
                    if (scan->u.args3.c != linear) {
                        break;
                    }
                    link = &scan->u.args3.c;
                    goto found;
                case IROLinearFunccall:
                    if (scan->u.funccall.callee == linear) {
                        link = &scan->u.funccall.callee;
                        goto found;
                    }
                    for (i = 0; i < scan->u.funccall.argCount; i++) {
                        if (scan->u.funccall.args[i] == linear) {
                            link = &scan->u.funccall.args[i];
                            goto found;
                        }
                    }
                    break;
                case IROLinearNop:
                case IROLinearOperand:
                case IROLinear5:
                case IROLinearGoto:
                case IROLinearLabel:
                case 14:
                case 15:
                case IROLinearBeginCatch:
                case IROLinearEndCatch:
                case IROLinearEndCatchDtor:
                case IROLinearAsm:
                case IROLinearEnd:
                    break;
                default:
                    CError_FATAL(3288);
                    break;
            }
            scan = scan->next;
        }
        scan = IRO_FirstLinear;
    }
    link = NULL;
    scan = NULL;
found:
    if (scan) {
        CError_ASSERT(3419, link && *link);
        *link = replacement;
        replacement->flags |= IROLF_Reffed;
    } else {
        IRO_Dump("Oh, oh, did not find reference to replace\n");
    }
}

Boolean IRO_ExprsSame(IROLinear* a, IROLinear* b) {
    UInt16 mask;
    ENode* lhs;
    ENode* rhs;

    if (a->type == IROLinearOp1Arg &&
        b->type == IROLinearOp1Arg &&
        a->nodetype == 0x32 &&
        b->nodetype == 0x32) {
        mask = 0xFD7F;
    } else {
        mask = 0xFDFF;
    }
    if (a->type != b->type ||
        (a->nodeflags & mask) != (b->nodeflags & mask) ||
        !IRO_TypesEqual(a->rtype, b->rtype)) {
        return FALSE;
    }
    switch (a->type) {
        case IROLinearOperand:
            lhs = a->u.node;
            rhs = b->u.node;
            if (lhs->type != rhs->type) {
                return FALSE;
            }
            switch (lhs->type) {
                case EINTCONST:
                    return CInt64_Equal(lhs->data.intval, rhs->data.intval);
                case EFLOATCONST:
                    return !CMach_FloatIsZero(lhs->data.floatval.value) &&
                        CMach_CalcFloatDiadicBool(
                            lhs->rtype, lhs->data.floatval, 0x16A, rhs->data.floatval
                        );
                case ESTRINGCONST:
                    if (((UInt32*)&lhs->data)[0] != ((UInt32*)&rhs->data)[0]) {
                        return FALSE;
                    }
                    return memcmp(
                               ((void**)&lhs->data)[1],
                               ((void**)&rhs->data)[1],
                               ((UInt32*)&lhs->data)[0]
                           ) == 0;
                case EOBJREF:
                    return lhs->data.objref == rhs->data.objref;
                default:
                    return FALSE;
            }
        case IROLinearOp1Arg:
            if (a->nodetype == b->nodetype) {
                return IRO_ExprsSame(a->u.monadic, b->u.monadic);
            }
            return FALSE;
        case IROLinearOp2Arg:
            if (a->nodetype == b->nodetype) {
                return IRO_ExprsSame(a->u.diadic.left, b->u.diadic.left) &&
                    IRO_ExprsSame(a->u.diadic.right, b->u.diadic.right);
            }
            return FALSE;
        case IROLinearOp3Arg:
            if (a->nodetype == b->nodetype) {
                return IRO_ExprsSame(a->u.args3.a, b->u.args3.a) &&
                    IRO_ExprsSame(a->u.args3.b, b->u.args3.b) &&
                    IRO_ExprsSame(a->u.args3.c, b->u.args3.c);
            }
            return FALSE;
        default:
            return FALSE;
    }
}

void IRO_FindReffedLabels(IROLinear* first, IROLinear* last) {
    IROLinear* scan;
    CLabel* label;
    IROGotoTarget* entry;

    IRO_GotoTargetCount = 0;
    IRO_GotoTargetSentinel = NULL;
    for (scan = first; scan; scan = scan->next) {
        if (scan->type == IROLinearOperand &&
            scan->u.node &&
            scan->u.node->type == ELABEL &&
            scan->u.node->data.objref) {
            entry = IRO_oalloc2(sizeof(IROGotoTarget));
            memset(entry, 0, sizeof(IROGotoTarget));
            entry->label = scan->u.node->data.objref;
            entry->next = IRO_GotoTargetSentinel;
            IRO_GotoTargetCount++;
            IRO_GotoTargetSentinel = entry;
        } else if (scan->type == IROLinearOperand &&
                   scan->u.node &&
                   scan->u.node->type == EOBJREF) {
            Object* obj = scan->u.node->data.objref;
            if (obj->datatype == 8) {
                for (label = Labels; label; label = label->next) {
                    if ((void*)obj->u.expr == (void*)FunctionName && label->name == obj->name) {
                        entry = IRO_oalloc2(sizeof(IROGotoTarget));
                        memset(entry, 0, sizeof(IROGotoTarget));
                        entry->label = label;
                        entry->next = IRO_GotoTargetSentinel;
                        IRO_GotoTargetCount++;
                        IRO_GotoTargetSentinel = entry;
                    }
                }
                CError_ASSERT(5493, label != NULL);
            }
        } else if (scan->type == IROLinearLabel &&
                   scan->u.node &&
                   scan->u.node->type == 2 &&
                   scan->u.node->data.cond.cond &&
                   scan->u.node->data.cond.cond->type != IROLinearOperand) {
            for (label = Labels; label; label = label->next) {
                if (label->stmt && label->stmt->marked != 0x40) {
                    entry = IRO_oalloc2(sizeof(IROGotoTarget));
                    memset(entry, 0, sizeof(IROGotoTarget));
                    entry->label = label;
                    entry->next = IRO_GotoTargetSentinel;
                    IRO_GotoTargetCount++;
                    IRO_GotoTargetSentinel = entry;
                }
            }
        }
        if (scan == last) {
            break;
        }
    }
}

IROLinear* IRO_LogicalizeExpr(IROLinear* linear, Boolean convert) {
    IROLinear* first;
    IROLinear* second;
    IRONode* node;
    Type* logicalType = IRO_LogicalType();

    if (IRO_IsLogicalExpr(linear)) {
        first = linear;
        if (convert && first->rtype != logicalType && !IRO_TypesEqual(first->rtype, logicalType)) {
            first = IRO_AddTypeConversion(first, logicalType, 0);
        }
        return first;
    }

    first = IRO_NewLinear(IROLinearOp1Arg);
    *first = *linear;
    first->type = IROLinearOp1Arg;
    first->nodetype = ELOGNOT;
    first->index = IRO_NumLinear++;
    first->u.monadic = linear;
    first->rtype = logicalType;

    second = IRO_NewLinear(IROLinearOp1Arg);
    *second = *first;
    second->type = IROLinearOp1Arg;
    second->nodetype = ELOGNOT;
    second->index = IRO_NumLinear++;
    second->rtype = logicalType;
    second->u.monadic = first;
    for (node = IRO_FirstNode; node; node = node->nextnode) {
        if (node->last == linear) {
            node->last = second;
            break;
        }
    }
    second->next = linear->next;
    linear->next = first;
    first->next = second;
    if (convert && linear->rtype != logicalType && !IRO_TypesEqual(linear->rtype, logicalType)) {
        second = IRO_AddTypeConversion(second, logicalType, 0);
    }
    return second;
}

IROLinear* IRO_LogicalNotExpr(IROLinear* linear, Boolean convert) {
    IROLinear* scan;
    IROLinear* result;
    IRONode* node;
    Type* logicalType = IRO_LogicalType();

    if (linear->type == IROLinearOp1Arg && linear->nodetype == ELOGNOT) {
        scan = linear->u.monadic;
        if (scan->rtype == logicalType || IRO_TypesEqual(scan->rtype, linear->rtype)) {
            linear->type = IROLinearNop;
            linear->unk1C = 0;
            result = scan;
            if (convert &&
                result->rtype != logicalType &&
                !IRO_TypesEqual(result->rtype, logicalType)) {
                result = IRO_AddTypeConversion(result, logicalType, 0);
            }
            return result;
        }
    }
    for (scan = linear; scan->type == IROLinearOp1Arg && scan->nodetype == ETYPCON;
         scan = scan->u.monadic) {}
    if (IRO_IsLogicalExpr(linear) && scan->type == IROLinearOp2Arg) {
        UInt8 op = scan->nodetype;
        if (op == ELESS ||
            op == EGREATER ||
            (IRO_ComplementaryLogicalComparison[op] != IRO_MAXEXPR &&
             scan->u.diadic.left->rtype->type == TYPEINT &&
             TYPE_INTEGRAL(scan->u.diadic.left->rtype)->integral < 0x11)) {
            scan->nodetype = IRO_ComplementaryLogicalComparison[op];
            result = linear;
            goto done;
        }
    }

    result = IRO_NewLinear(IROLinearOp1Arg);
    *result = *linear;
    result->type = IROLinearOp1Arg;
    result->nodetype = ELOGNOT;
    result->index = IRO_NumLinear++;
    result->rtype = logicalType;
    result->u.monadic = linear;
    for (node = IRO_FirstNode; node; node = node->nextnode) {
        if (node->last == linear) {
            node->last = result;
            break;
        }
    }
    result->next = linear->next;
    linear->next = result;
done:
    if (convert && result->rtype != logicalType && !IRO_TypesEqual(result->rtype, logicalType)) {
        result = IRO_AddTypeConversion(result, logicalType, 0);
    }
    return result;
}

IROLinear* fn_00565FF0(IROLinear* linear) {
    switch (linear->type) {
        case IROLinearOp1Arg:
            return fn_00565FF0(linear->u.monadic);
        case IROLinearOp2Arg:
            return fn_00565FF0(linear->u.diadic.left);
        case IROLinearOperand:
            return linear;
        default:
            return NULL;
    }
}

void IRO_WalkENodeTree(
    ENode** expr, Boolean (*callback)(ENode**, SInt32, void*), SInt32 flags, void* data
) {
    ENode* node;

    if (flags & 1) {
        if (!callback(expr, 1, data)) {
            return;
        }
    }

    node = *expr;
    CError_ASSERT(5820, node);
    switch (node->type) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 50:
        case 51:
            IRO_WalkENodeTree(&node->data.monadic, callback, flags, data);
            break;
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 74:
        case 75:
            IRO_WalkENodeTree(&node->data.diadic.left, callback, flags, data);
            IRO_WalkENodeTree(&node->data.diadic.right, callback, flags, data);
            break;
        case 55:
        case 82:
            IRO_WalkENodeTree(&node->data.cond.cond, callback, flags, data);
            IRO_WalkENodeTree(&node->data.cond.expr1, callback, flags, data);
            IRO_WalkENodeTree(&node->data.cond.expr2, callback, flags, data);
            break;
        case 59:
            IRO_WalkENodeTree(&node->data.monadic, callback, flags, data);
            break;
        case 56:
        case 57:
            IRO_WalkENodeTree(&node->data.funccall.funcref, callback, flags, data);
            {
                ENodeList* list;
                for (list = node->data.funccall.args; list; list = list->next) {
                    IRO_WalkENodeTree(&list->node, callback, flags, data);
                }
            }
            break;
        case 52:
        case 53:
        case 54:
        case 58:
        case 60:
        case 61:
        case 62:
        case 63:
        case 78:
            break;
        default:
            CError_FATAL(5880);
            break;
    }
    if (flags & 2) {
        callback(expr, 2, data);
    }
}

Boolean IRO_ENodeExprsSame(ENode* a, ENode* b) {
    UInt16 mask;
    if (a->type == 50 && b->type == 50) {
        mask = 0xFD7F;
    } else {
        mask = 0xFDFF;
    }

    if (a->type != b->type || (a->flags & mask) != (b->flags & mask)) {
        return FALSE;
    }
    if (!IRO_TypesEqual(a->rtype, b->rtype)) {
        return FALSE;
    }
    switch (a->type) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 50:
        case 51:
            return IRO_ENodeExprsSame(a->data.monadic, b->data.monadic);
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 74:
        case 75:
            return IRO_ENodeExprsSame(a->data.diadic.left, b->data.diadic.left) &&
                IRO_ENodeExprsSame(a->data.diadic.right, b->data.diadic.right);
        case ECOND:
            return IRO_ENodeExprsSame(a->data.cond.cond, b->data.cond.cond) ||
                IRO_ENodeExprsSame(a->data.cond.expr1, b->data.cond.expr1) ||
                IRO_ENodeExprsSame(a->data.cond.expr2, b->data.cond.expr2);
        case ENULLCHECK:
            return IRO_ENodeExprsSame(
                       a->data.nullcheck.nullcheckexpr, b->data.nullcheck.nullcheckexpr
                   ) ||
                IRO_ENodeExprsSame(a->data.nullcheck.condexpr, b->data.nullcheck.condexpr);
        case EINTCONST:
        case EFLOATCONST:
        case ESTRINGCONST:
        case EOBJREF:
            if (a->type != b->type) {
                return FALSE;
            }
            switch (a->type) {
                case EINTCONST:
                    return CInt64_Equal(a->data.intval, b->data.intval);
                case EFLOATCONST:
                    return !CMach_FloatIsZero(a->data.floatval.value) &&
                        CMach_CalcFloatDiadicBool(
                            a->rtype, a->data.floatval, 0x16A, b->data.floatval
                        );
                case ESTRINGCONST:
                    return *(UInt32*)&a->data == *(UInt32*)&b->data &&
                        memcmp(((void**)&a->data)[1], ((void**)&b->data)[1], *(UInt32*)&a->data) ==
                        0;
                case EOBJREF:
                    return a->data.objref == b->data.objref;
                default:
                    return FALSE;
            }
        case EPRECOMP:
        case ELABEL:
        case ESCOPEBEGIN:
        case ESCOPEEND:
            return a->data.objref == b->data.objref;
        case EFUNCCALL:
        case EFUNCCALLP: {
            ENodeList* left;
            ENodeList* right;
            if (!IRO_ENodeExprsSame(a->data.funccall.funcref, b->data.funccall.funcref)) {
                return FALSE;
            }
            left = a->data.funccall.args;
            right = b->data.funccall.args;
            while (left) {
                if (!IRO_ENodeExprsSame(left->node, right->node)) {
                    return FALSE;
                }
                left = left->next;
                right = right->next;
            }
            return TRUE;
        }
        default:
            return FALSE;
    }
}

typedef struct IROExcWalkAction {
    struct IROExcWalkAction* prev;
    Object* arg1;
    Object* arg2;
    Object* arg3;
    Object* arg4;
    UInt8 unk14[8];
    UInt8 type;
} IROExcWalkAction;

void IRO_WalkExcActions(IROExcWalkAction* action, void (*callback)(Object*, void*), void* data) {
    while (action) {
        switch (action->type) {
            case 1:
                callback(action->arg1, data);
                break;
            case 2:
                callback(action->arg1, data);
                callback(action->arg2, data);
                break;
            case 3:
                callback(action->arg1, data);
                break;
            case 4:
                callback(action->arg1, data);
                break;
            case 5:
                callback(action->arg1, data);
                break;
            case 6:
                callback(action->arg1, data);
                callback(action->arg2, data);
                callback(action->arg3, data);
                callback(action->arg4, data);
                break;
            case 17:
                callback(action->arg1, data);
                break;
            case 7:
                callback(action->arg1, data);
                break;
            case 8:
                callback(action->arg1, data);
                callback(action->arg2, data);
                break;
            case 9:
                callback(action->arg1, data);
                break;
            case 10:
                callback(action->arg1, data);
                break;
            case 12:
                callback(action->arg1, data);
                callback(action->arg3, data);
                break;
            case 11:
                callback(action->arg1, data);
                break;
            case 13:
                if (action->arg1) {
                    callback(action->arg1, data);
                }
                callback(action->arg2, data);
                break;
            case 14:
                callback(action->arg1, data);
                break;
            case 18:
                callback(action->arg1, data);
                break;
            default:
                break;
        }
        action = action->prev;
    }
}

void IRO_CheckUnorderedRegionsForDefs(
    IROLinear* linear, IROLinear* excluded, void* kills, void* uses, Boolean* found
) {
    int i;
    switch (linear->type) {
        case IROLinearOp2Arg:
            if (linear->nodetype == ELAND) {
                break;
            }
            if (linear->nodetype == ELOR) {
                break;
            }
            if (linear->nodetype == ECOMMA) {
                break;
            }
            if (linear->u.diadic.left != excluded) {
                IRO_CheckUnorderedRegionForDefs(linear->u.diadic.left, kills, uses, found);
            }
            if (linear->u.diadic.right != excluded) {
                IRO_CheckUnorderedRegionForDefs(linear->u.diadic.right, kills, uses, found);
            }
            break;
        case IROLinearFunccall:
            if (linear->u.funccall.callee != excluded) {
                IRO_CheckUnorderedRegionForDefs(linear->u.funccall.callee, kills, uses, found);
            }
            if (!copts.opt_defineargorder && IRO_CallArgumentsFlag) {
                for (i = 0; !*found && i < linear->u.funccall.argCount; i++) {
                    if (linear->u.funccall.args[i] != excluded) {
                        IRO_CheckUnorderedRegionForDefs(
                            linear->u.funccall.args[i], kills, uses, found
                        );
                    }
                }
            }
            break;
        default:
            break;
    }
}

void IRO_CheckUnorderedRegionForDefs(
    IROLinear* linear, BitVector* kills, BitVector* uses, Boolean* found
) {
    Bv_Clear(kills);
    Bv_Clear(IRO_VarKills);
    IRO_WalkTree(linear, IRO_GetKillsInExpr, NULL);
    Bv_Or(IRO_VarKills, kills);
    if (Bv_BitsInCommon(kills, uses)) {
        *found = TRUE;
    }
}

Boolean IRO_ExprsSameSemantically(IROLinear* a, IROLinear* b) {
    return IRO_ExprsSameSemantically_Recursive(a, b, FALSE);
}

Boolean IRO_ExprsSameSemantically_Recursive(IROLinear* a, IROLinear* b, Boolean allowSwaps) {
    UInt16 mask;
    ENode* lhs;
    ENode* rhs;
    Boolean swappedSame = FALSE;
    int i;

    if (a->type == IROLinearOp1Arg &&
        b->type == IROLinearOp1Arg &&
        a->nodetype == 0x32 &&
        b->nodetype == 0x32) {
        mask = 0xFD7F;
    } else {
        mask = 0xFDFF;
    }
    if (a->type != b->type ||
        (a->nodeflags & mask) != (b->nodeflags & mask) ||
        !IRO_TypesEqual(a->rtype, b->rtype)) {
        return FALSE;
    }
    switch (a->type) {
        case IROLinearOperand:
            lhs = a->u.node;
            rhs = b->u.node;
            if (lhs->type != rhs->type) {
                return FALSE;
            }
            switch (lhs->type) {
                case EINTCONST:
                    return CInt64_Equal(lhs->data.intval, rhs->data.intval);
                case EFLOATCONST:
                    return !CMach_FloatIsZero(lhs->data.floatval.value) &&
                        CMach_CalcFloatDiadicBool(
                            lhs->rtype, lhs->data.floatval, 0x16A, rhs->data.floatval
                        );
                case ESTRINGCONST:
                    if (((UInt32*)&lhs->data)[0] != ((UInt32*)&rhs->data)[0]) {
                        return FALSE;
                    }
                    return memcmp(
                               ((void**)&lhs->data)[1],
                               ((void**)&rhs->data)[1],
                               ((UInt32*)&lhs->data)[0]
                           ) == 0;
                case EOBJREF:
                    return lhs->data.objref == rhs->data.objref;
                default:
                    return FALSE;
            }
        case IROLinearOp1Arg:
            if (a->nodetype == b->nodetype) {
                return IRO_ExprsSameSemantically_Recursive(a->u.monadic, b->u.monadic, allowSwaps);
            }
            return FALSE;
        case IROLinearOp2Arg:
            if (a->nodetype == b->nodetype) {
                if (IRO_IsCommutativeENodeTypeArray[a->nodetype] && !IRO_HasSideEffect(a)) {
                    swappedSame = IRO_ExprsSameSemantically_Recursive(
                                      a->u.diadic.left, b->u.diadic.right, allowSwaps
                                  ) &&
                        IRO_ExprsSameSemantically_Recursive(
                                      a->u.diadic.right, b->u.diadic.left, allowSwaps
                        );
                }
                return swappedSame ||
                    (IRO_ExprsSameSemantically_Recursive(
                         a->u.diadic.left, b->u.diadic.left, allowSwaps
                     ) &&
                     IRO_ExprsSameSemantically_Recursive(
                         a->u.diadic.right, b->u.diadic.right, allowSwaps
                     ));
            }
            return FALSE;
        case IROLinearOp3Arg:
            if (a->nodetype == b->nodetype) {
                return IRO_ExprsSameSemantically_Recursive(
                           a->u.args3.a, b->u.args3.a, allowSwaps
                       ) &&
                    IRO_ExprsSameSemantically_Recursive(a->u.args3.b, b->u.args3.b, allowSwaps) &&
                    IRO_ExprsSameSemantically_Recursive(a->u.args3.c, b->u.args3.c, allowSwaps);
            }
            return FALSE;
        case IROLinearFunccall:
            if (!IRO_ExprsSameSemantically_Recursive(
                    a->u.funccall.callee, b->u.funccall.callee, allowSwaps
                ) ||
                a->u.funccall.argCount != b->u.funccall.argCount) {
                return FALSE;
            }
            for (i = 0; i < a->u.funccall.argCount; i++) {
                if (!IRO_ExprsSameSemantically_Recursive(
                        a->u.funccall.args[i], b->u.funccall.args[i], allowSwaps
                    )) {
                    return FALSE;
                }
            }
            return TRUE;
        default:
            return FALSE;
    }
}

void WalkTreeHelper(IROLinear* linear, IROChildExpressionFunc func, Boolean propagate, void* data) {
    int i;
#define WALK_CHILD(expr) \
    do { \
        IROLinear* child = (expr); \
        int j; \
        if (propagate) { \
            func(child, TRUE, data); \
        } \
        switch (child->type) { \
            case IROLinearOperand: \
            case IROLinear5: \
                break; \
            case IROLinearOp1Arg: \
                WalkTreeHelper(child->u.monadic, func, propagate, data); \
                break; \
            case IROLinearOp2Arg: \
                WalkTreeHelper(child->u.diadic.left, func, propagate, data); \
                WalkTreeHelper(child->u.diadic.right, func, propagate, data); \
                break; \
            case IROLinearOp3Arg: \
                WalkTreeHelper(child->u.args3.a, func, propagate, data); \
                WalkTreeHelper(child->u.args3.b, func, propagate, data); \
                WalkTreeHelper(child->u.args3.c, func, propagate, data); \
                break; \
            case IROLinearFunccall: \
                WalkTreeHelper(child->u.funccall.callee, func, propagate, data); \
                for (j = 0; j < child->u.funccall.argCount; j++) { \
                    WalkTreeHelper(child->u.funccall.args[j], func, propagate, data); \
                } \
                break; \
            default: \
                break; \
        } \
        func(child, FALSE, data); \
    } while (0)

    if (propagate) {
        func(linear, TRUE, data);
    }
    switch (linear->type) {
        case IROLinearOperand:
        case IROLinear5:
            break;
        case IROLinearOp1Arg:
            WALK_CHILD(linear->u.monadic);
            break;
        case IROLinearOp2Arg:
            WALK_CHILD(linear->u.diadic.left);
            WALK_CHILD(linear->u.diadic.right);
            break;
        case IROLinearOp3Arg:
            WALK_CHILD(linear->u.args3.a);
            WALK_CHILD(linear->u.args3.b);
            WALK_CHILD(linear->u.args3.c);
            break;
        case IROLinearFunccall:
            WALK_CHILD(linear->u.funccall.callee);
            for (i = 0; i < linear->u.funccall.argCount; i++) {
                WALK_CHILD(linear->u.funccall.args[i]);
            }
            break;
        default:
            break;
    }
    func(linear, FALSE, data);
#undef WALK_CHILD
}

IROLinear* fn_00562650(IROLinear* expr) {
    IROLinearIRSave save;
    IROList list;
    Statement* stmt;
    IROLinear* node;
    IROLinear* result;

    IRO_SaveLinearIR(&save);
    IRO_InitList(&list);
    IRO_DuplicateExpr(expr, &list);
    stmt = IRO_Delinearize(NULL, list.head);
    CError_ASSERT(4826, stmt != NULL);
    stmt->expr = fn_0052DC40(stmt->expr);
    CError_ASSERT(4828, stmt->expr != NULL);
    if (copts.opt_prelinearize) {
        stmt = IRO_PreLinearize(stmt);
    }
    IRO_Linearize(stmt);
    IRO_InitList(&list);
    list.head = IRO_FirstLinear;
    list.tail = IRO_LastLinear;
    IRO_RestoreLinearIR(&save);
    result = NULL;
    for (node = list.head; node; node = node->next) {
        if (!(node->flags & 2) && IRO_IsAssignment(node)) {
            result = node;
        }
    }
    return result;
}

void fn_00563AE0(
    IROLinear* linear, IROLinear* excluded, BitVector* kills, BitVector* uses, Boolean* found
) {
    IROLinear* node;
    UInt32 n;
    UInt32 i;
    Boolean common;

#define CHECK_REGION(expr) \
    do { \
        node = (expr); \
        if (node != excluded) { \
            if (IRO_HasSideEffect(node)) { \
                *found = TRUE; \
            } else { \
                Bv_Clear(uses); \
                IRO_GetUsesInExpr(node, FALSE, FALSE, TRUE); \
                Bv_Or(IRO_Depends, uses); \
                n = (kills->size < uses->size) ? kills->size : uses->size; \
                common = FALSE; \
                for (i = 0; i < n; i++) { \
                    if (kills->data[i] & uses->data[i]) { \
                        common = TRUE; \
                        break; \
                    } \
                } \
                if (common) { \
                    *found = TRUE; \
                } \
            } \
        } \
    } while (0)

    switch (linear->type) {
        case IROLinearOp2Arg:
            if (linear->nodetype == ELAND ||
                linear->nodetype == ELOR ||
                linear->nodetype == ECOMMA) {
                break;
            }
            CHECK_REGION(linear->u.diadic.left);
            CHECK_REGION(linear->u.diadic.right);
            break;
        case IROLinearFunccall:
            CHECK_REGION(linear->u.funccall.callee);
            for (i = 0; !*found && i < linear->u.funccall.argCount; i++) {
                CHECK_REGION(linear->u.funccall.args[i]);
            }
            break;
        default:
            break;
    }
#undef CHECK_REGION
}

Boolean fn_00562780(IROLinear* a, IROLinear* b, IROList* output) {
    IROLinear* source;
    IROLinear* target;
    IROLinear* copy;
    int i;

    if (b->type == IROLinearOp1Arg && b->nodetype == 0x32) {
        if (a->type != IROLinearOp1Arg || a->nodetype != 0x32) {
            if (fn_00562780(a, b->u.monadic, output)) {
                return TRUE;
            }
        }
        if (b->u.monadic->type == IROLinearOp1Arg &&
            b->u.monadic->nodetype == 7 &&
            b->u.monadic->u.monadic->type == IROLinearOp1Arg &&
            b->u.monadic->u.monadic->nodetype == 7 &&
            fn_00562780(a, b->u.monadic->u.monadic, output)) {
            return TRUE;
        }
    }
    if (a->type == IROLinearOp1Arg && a->nodetype == 0x32) {
        if (fn_00562780(a->u.monadic, b, output)) {
            return TRUE;
        }
    }
    if (a->type != b->type) {
        return FALSE;
    }
    if (a->type == IROLinearOperand) {
        if (a->u.node && b->u.node && a->u.node->type == b->u.node->type) {
            return IRO_ExprsSame(a, b);
        }
        return FALSE;
    }
    if (a->nodetype != b->nodetype) {
        return FALSE;
    }
    switch (a->type) {
        case IROLinearOp1Arg:
            if (fn_00562780(a->u.monadic, b->u.monadic, output)) {
                copy = IRO_NewLinear(IROLinearOp1Arg);
                *copy = *b;
                copy->index = IRO_NumLinear++;
                copy->rtype = IRO_CopyType(a->rtype);
                copy->next = NULL;
                copy->u.monadic = output->tail;
                IRO_AddToList(copy, output);
                return TRUE;
            }
            return FALSE;
        case IROLinearOp2Arg:
            if (fn_00562780(a->u.diadic.left, b->u.diadic.left, output) &&
                fn_00562780(a->u.diadic.right, b->u.diadic.right, output)) {
                copy = IRO_NewLinear(IROLinearOp2Arg);
                *copy = *b;
                copy->index = IRO_NumLinear++;
                copy->rtype = IRO_CopyType(a->rtype);
                copy->next = NULL;
                copy->u.diadic.left = output->tail;
                IRO_AddToList(copy, output);
                return TRUE;
            }
            return FALSE;
        case IROLinearOp3Arg:
            return fn_00562780(a->u.args3.a, b->u.args3.a, output) &&
                fn_00562780(a->u.args3.b, b->u.args3.b, output) &&
                fn_00562780(a->u.args3.c, b->u.args3.c, output);
        case IROLinearFunccall:
            if (a->u.funccall.argCount != b->u.funccall.argCount) {
                return FALSE;
            }
            if (!fn_00562780(a->u.funccall.callee, b->u.funccall.callee, output)) {
                return FALSE;
            }
            for (i = 0; i < a->u.funccall.argCount; i++) {
                if (!fn_00562780(a->u.funccall.args[i], b->u.funccall.args[i], output)) {
                    return FALSE;
                }
            }
            return TRUE;
        default:
            return FALSE;
    }
}

Boolean fn_00561E60(IROLinear* linear) {
    IROLinear* lhs;
    IROLinear* rhs;
    IROLinear* leftCopy;
    IROLinear* rightCopy;
    IROLinear* result;
    IROLinear* selected;
    IROList list;
    ENodeType op;
    Boolean convert = FALSE;

    if (IRO_IsAssignment(linear) &&
        IRO_IsModifyOp[linear->nodetype] &&
        linear->type == IROLinearOp1Arg &&
        fn_005621D0(linear)) {
        op = linear->nodetype;
    } else {
        op = linear->nodetype;
    }

    if (!IRO_IsAssignment(linear) ||
        !IRO_IsModifyOp[op] ||
        linear->type != IROLinearOp2Arg ||
        IRO_NonAssignmentOp[op] == IRO_MAXEXPR) {
        return FALSE;
    }

    op = IRO_NonAssignmentOp[op];
    lhs = linear->u.diadic.left;
    rhs = linear->u.diadic.right;
    if (op == 0x10 &&
        rhs->type == IROLinearOperand &&
        rhs->rtype &&
        rhs->rtype->type == TYPEINT &&
        TYPE_INTEGRAL(rhs->rtype)->integral < 0x11 &&
        rhs->u.node &&
        rhs->u.node->type == EINTCONST &&
        !is_unsigned(rhs->rtype)) {
        op = 0xf;
        convert = TRUE;
    }
    if (!lhs ||
        !rhs ||
        op == IRO_MAXEXPR ||
        !lhs->u.monadic ||
        lhs->u.monadic->type != IROLinearOperand ||
        !lhs->u.monadic->u.node ||
        lhs->u.monadic->u.node->type != EOBJREF ||
        IRO_HasSideEffect(lhs)) {
        return FALSE;
    }

    IRO_InitList(&list);
    leftCopy = IRO_DuplicateExpr(rhs, &list);
    rightCopy = IRO_DuplicateExpr(lhs, &list);
    if (!leftCopy || !rightCopy) {
        return FALSE;
    }
    if (!IRO_TypesEqual(leftCopy->rtype, lhs->rtype)) {
        leftCopy = IRO_AddTypeConversion(leftCopy, lhs->rtype, 0);
    }
    result = IRO_NewLinear(IROLinearOp2Arg);
    result->nodetype = op;
    result->flags |= 2;
    result->rtype = IRO_CopyType(rightCopy->rtype);
    result->index = ++IRO_NumLinear;
    result->u.diadic.left = rightCopy;
    result->u.diadic.right = leftCopy;
    IRO_AddToList(result, &list);

    if (!IRO_TypesEqual(result->rtype, lhs->rtype)) {
        result = IRO_AddTypeConversion(result, lhs->rtype, 0);
    }
    selected = fn_00562650(linear);
    if (selected &&
        selected->type == IROLinearOp2Arg &&
        fn_00562780(selected->u.diadic.right, lhs, &list)) {
        IRO_NopOut(linear->u.diadic.right);
        linear->nodetype = EASS;
        linear->u.diadic.right = list.tail;
        IRO_Paste(list.head, list.tail, linear);
        return TRUE;
    }
    return FALSE;
}

Boolean fn_005621D0(IROLinear* linear) {
    IROLinear* lhs;
    IROLinear* rhs;
    IROLinear* copy;
    IROLinear* constant;
    Type* rtype;
    ENodeType op;
    IROList list;

    if (!IRO_IsAssignment(linear) || !IRO_IsModifyOp[linear->nodetype]) {
        return FALSE;
    }
    op = linear->nodetype;
    if (linear->type == IROLinearOp1Arg && op <= 3) {
        lhs = linear->u.monadic;
        rtype = linear->rtype;
        if (!(linear->flags & 2) &&
            rtype &&
            ((rtype->type == TYPEINT && TYPE_INTEGRAL(rtype)->integral < 0x11) ||
             rtype->type == TYPEPOINTER)) {
            IRO_InitList(&list);
            constant = IRO_NewLinear(IROLinearOperand);
            constant->nodetype = EINTCONST;
            constant->rtype = IRO_CopyType(rtype);
            constant->index = ++IRO_NumLinear;
            IRO_AddToList(constant, &list);
            rhs = IRO_NewLinear(IROLinearOp2Arg);
            rhs->nodetype = EADD;
            rhs->rtype = IRO_CopyType(rtype);
            rhs->index = ++IRO_NumLinear;
            rhs->u.diadic.left = lhs;
            rhs->u.diadic.right = constant;
            IRO_AddToList(rhs, &list);
            lhs->flags |= 2;
            linear->type = IROLinearOp2Arg;
            linear->nodetype = EASS;
            linear->u.diadic.left = lhs;
            linear->u.diadic.right = rhs;
            IRO_Paste(list.head, list.tail, linear);
            return TRUE;
        }
    }

    if (linear->type == IROLinearOp2Arg) {
        lhs = linear->u.diadic.left;
        rhs = linear->u.diadic.right;
        if (!lhs || !rhs || IRO_HasSideEffect(lhs) || !rhs->rtype || !lhs->rtype) {
            return FALSE;
        }
        if (!IRO_TypesEqual(lhs->rtype, rhs->rtype)) {
            copy = IRO_AddTypeConversion(rhs, lhs->rtype, 0);
        } else {
            copy = rhs;
        }
        lhs->flags |= 2;
        linear->type = IROLinearOp2Arg;
        linear->nodetype = op;
        linear->u.diadic.right = copy;
        return TRUE;
    }
    return FALSE;
}
