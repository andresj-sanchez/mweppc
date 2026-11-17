#include "compiler/IroCSE.h"
#include "compiler/IroExprs.h"
#include "compiler/IroDump.h"
#include "compiler/IroFlowgraph.h"
#include "compiler/IroLinearForm.h"
#include "compiler/IroMalloc.h"
#include "compiler/IroPointerAnalysis.h"
#include "compiler/IroSubable.h"
#include "compiler/IROUseDef.h"
#include "compiler/IroUtil.h"
#include "compiler/IroVars.h"
#include "compiler/BitVector.h"
#include "compiler/CError.h"
#include "compiler/CExpr.h"
#include "compiler/CInt64.h"
#include "compiler/CompilerTools.h"
#include "compiler/CParser.h"
#include "compiler/enode.h"
#include "compiler/objects.h"
#include <string.h>

#define CSE_POINTSTO(nd) (((UInt8*)(nd))[0x2B])

BitVector* IRO_Depends;
Boolean IRO_NotSubable;
Boolean IRO_IsVolatile;
Boolean IRO_CouldError;
IROExpr* IRO_FirstExpr;
IROExpr* IRO_LastExpr;
SInt32 IRO_NumExprs;
static Boolean HasVectorOperand;

static void IRO_DependsOnForDataFlow(IROLinear* linear, Boolean isIndirect);

static void GetDependsOfIndirect(IROLinear* nd) {
    IROLinear* baseterm;
    SInt32 baseterms;
    SInt32 varterms;
    IROListNode* resultList;
    IROListNode* next;
    IROListNode* scan;
    IROLinear* scannd;
    Object* obj;
    VarRecord* var;
    int index;
    Boolean result;
    Boolean foundObjRef;

    result = FALSE;

    if (nd && copts.opt_pointer_analysis && CSE_POINTSTO(nd) && FunctionName) {
        resultList = NULL;
        PointerAnalysis_LookupLinearNodePointerExpr(FunctionName, nd, &resultList);
        if (resultList) {
            for (scan = resultList; scan; scan = scan->nextList) {
                if (!scan->list.head || !scan->list.tail) {
                    result = TRUE;
                    break;
                }

                foundObjRef = FALSE;
                for (scannd = scan->list.head; scannd != scan->list.tail->next;
                     scannd = scannd->next) {
                    if (scannd->type == IROLinearOperand && scannd->u.node->type == EOBJREF) {
                        foundObjRef = TRUE;
                        break;
                    }
                }

                if (!foundObjRef) {
                    result = TRUE;
                    break;
                }
            }

            if (!result) {
                for (scan = resultList; scan; scan = scan->nextList) {
                    for (scannd = scan->list.head; scannd != scan->list.tail->next;
                         scannd = scannd->next) {
                        if (scannd->type == IROLinearOperand && scannd->u.node->type == EOBJREF) {
                            obj = scannd->u.node->data.objref;
                            CError_ASSERT(119, obj != NULL);
                            var = IRO_FindVar(obj, TRUE, TRUE);
                            CError_ASSERT(121, var != NULL);
                            index = var->index;
                            CError_ASSERT(123, index != 0);

                            if (is_volatile_object(obj)) {
                                IRO_IsVolatile = TRUE;
                                IRO_NotSubable = TRUE;
                            }
                            Bv_SetBit(index, IRO_Depends);
                        }
                    }
                }
            }

            while (resultList) {
                next = resultList->nextList;
                IRO_free(resultList);
                resultList = next;
            }
        } else {
            result = TRUE;
        }
    } else {
        result = TRUE;
    }

    if (result) {
        nd = nd->u.monadic;
        if (nd->type == IROLinearOp1Arg && nd->nodetype == EBITFIELD) {
            nd = nd->u.monadic;
        }

        if (nd->type == IROLinearOp2Arg && nd->nodetype == EADD) {
            IRO_DecomposeAddressExpression_Cheap(nd, &baseterm, &baseterms, &varterms);
            if (baseterms != 1) {
                IRO_CouldError = TRUE;
                Bv_SetBit(0, IRO_Depends);
                Bv_Or(IRO_FuncKills, IRO_Depends);
            }
            if (varterms) {
                IRO_CouldError = TRUE;
            }
        } else {
            IRO_CouldError = TRUE;
            Bv_SetBit(0, IRO_Depends);
            Bv_Or(IRO_FuncKills, IRO_Depends);
        }
    }
}

static void GetDependsOfFunctionCallForDataFlow(IROLinear* nd) {
    IROLinear* innernd;
    IROListNode* resultList;
    IROListNode* scan;
    IROLinear* scannd;
    Object* obj;
    VarRecord* var;
    int index;
    int i;
    IROListNode* next;
    Boolean result;
    Boolean foundObjRef;
    ObjectList* olist;
    ObjectList* depsList;

    result = FALSE;
    innernd = nd->u.funccall.callee;

    if (innernd && copts.opt_pointer_analysis && CSE_POINTSTO(innernd) && FunctionName) {
        resultList = NULL;
        PointerAnalysis_LookupLinearNodePointerExpr(FunctionName, innernd, &resultList);
        if (resultList) {
            for (scan = resultList; scan; scan = scan->nextList) {
                if (!scan->list.head || !scan->list.tail) {
                    result = TRUE;
                    break;
                }

                foundObjRef = FALSE;
                for (scannd = scan->list.head; scannd != scan->list.tail->next;
                     scannd = scannd->next) {
                    if (scannd->type == IROLinearOperand && scannd->u.node->type == EOBJREF) {
                        foundObjRef = TRUE;
                        obj = scannd->u.node->data.objref;
                        CError_ASSERT(234, obj != NULL);

                        depsList = NULL;
                        PointerAnalysis_GetFunctionDependencies(obj, nd, &depsList);

                        for (olist = depsList; olist; olist = olist->next) {
                            if (!olist->object) {
                                result = TRUE;
                                break;
                            }
                        }

                        while (depsList) {
                            olist = depsList->next;
                            IRO_free(depsList);
                            depsList = olist;
                        }

                        if (result) {
                            break;
                        }
                    }
                }

                if (!foundObjRef) {
                    result = TRUE;
                }
                if (result) {
                    break;
                }
            }

            if (!result) {
                for (scan = resultList; scan; scan = scan->nextList) {
                    for (scannd = scan->list.head; scannd != scan->list.tail->next;
                         scannd = scannd->next) {
                        if (scannd->type == IROLinearOperand && scannd->u.node->type == EOBJREF) {
                            obj = scannd->u.node->data.objref;

                            depsList = NULL;
                            PointerAnalysis_GetFunctionDependencies(obj, nd, &depsList);

                            for (olist = depsList; olist; olist = olist->next) {
                                var = IRO_FindVar(olist->object, TRUE, TRUE);
                                CError_ASSERT(285, var != NULL);
                                index = var->index;
                                CError_ASSERT(287, index != 0);

                                if (is_volatile_object(olist->object)) {
                                    IRO_IsVolatile = TRUE;
                                    IRO_NotSubable = TRUE;
                                }
                                Bv_SetBit(index, IRO_Depends);
                            }

                            while (depsList) {
                                olist = depsList->next;
                                IRO_free(depsList);
                                depsList = olist;
                            }
                        }
                    }
                }
            }

            while (resultList) {
                next = resultList->nextList;
                IRO_free(resultList);
                resultList = next;
            }
        } else {
            result = TRUE;
        }
    } else {
        result = TRUE;
    }

    if (result) {
        IRO_DependsOnForDataFlow(nd->u.funccall.callee, FALSE);
        Bv_Or(IRO_FuncKills, IRO_Depends);

        for (i = nd->u.funccall.argCount - 1; i >= 0; i--) {
            IRO_DependsOnForDataFlow(nd->u.funccall.args[i], FALSE);
        }
    }
}

static void IRO_DependsOn(IROLinear* linear, Boolean isIndirect) {
    VarRecord* var;
    IROLinear* inner;

    if (linear->rtype && CParser_IsVolatile(linear->rtype, linear->nodeflags & ENODE_FLAG_QUALS)) {
        IRO_IsVolatile = TRUE;
        IRO_NotSubable = TRUE;
    }

    if (IRO_NotSubable) {
        return;
    }

    switch (linear->type) {
        case IROLinearOperand:
            if (isIndirect && linear->u.node->type == EOBJREF) {
                if ((var = IRO_FindVar(linear->u.node->data.objref, FALSE, TRUE))) {
                    if (is_volatile_object(var->object)) {
                        IRO_IsVolatile = TRUE;
                        IRO_NotSubable = TRUE;
                    }
                    Bv_SetBit(var->index, IRO_Depends);
                } else {
                    IRO_NotSubable = TRUE;
                }
            }
            break;
        case IROLinearOp1Arg:
            if (IRO_IsAssignOp[linear->nodetype]) {
                IRO_NotSubable = TRUE;
                return;
            }
            inner = linear->u.monadic;
            if (linear->nodetype == EINDIRECT) {
                if (inner->type == IROLinearOp1Arg && inner->nodetype == EBITFIELD) {
                    inner = inner->u.monadic;
                }
                if (inner->type != IROLinearOperand || inner->u.node->type != EOBJREF) {
                    GetDependsOfIndirect(linear);
                }
            }

            IRO_DependsOn(inner, linear->nodetype == EINDIRECT);
            break;
        case IROLinearOp2Arg:
            if (IRO_IsAssignOp[linear->nodetype]) {
                IRO_NotSubable = TRUE;
                return;
            }
            if (linear->nodetype == EDIV || linear->nodetype == EMODULO) {
                if (IRO_IsIntConstant(linear->u.diadic.right)) {
                    if (CInt64_Equal(linear->u.diadic.right->u.node->data.intval, cint64_zero)) {
                        IRO_CouldError = TRUE;
                    }
                } else {
                    IRO_CouldError = TRUE;
                }
            }

            IRO_DependsOn(linear->u.diadic.left, isIndirect);
            IRO_DependsOn(linear->u.diadic.right, isIndirect);
            break;
        case IROLinearOp3Arg:
            if (IRO_IsAssignOp[linear->nodetype]) {
                IRO_NotSubable = TRUE;
                return;
            }
            IRO_DependsOn(linear->u.args3.a, isIndirect);
            IRO_DependsOn(linear->u.args3.b, isIndirect);
            IRO_DependsOn(linear->u.args3.c, isIndirect);
            break;
        case IROLinearFunccall:
            IRO_NotSubable = TRUE;
            break;
        default:
            CError_FATAL(479);
    }
}

static void IRO_DependsOnForDataFlow(IROLinear* linear, Boolean isIndirect) {
    VarRecord* var;
    IROLinear* inner;

    if (linear->rtype && CParser_IsVolatile(linear->rtype, linear->nodeflags & ENODE_FLAG_QUALS)) {
        IRO_IsVolatile = TRUE;
        IRO_NotSubable = TRUE;
    }

    switch (linear->type) {
        case IROLinearOperand:
            if (isIndirect && linear->u.node->type == EOBJREF) {
                if ((var = IRO_FindVar(linear->u.node->data.objref, FALSE, TRUE))) {
                    if (is_volatile_object(var->object)) {
                        IRO_IsVolatile = TRUE;
                        IRO_NotSubable = TRUE;
                    }
                    Bv_SetBit(var->index, IRO_Depends);
                } else {
                    IRO_NotSubable = TRUE;
                }
            }
            break;
        case IROLinearOp1Arg:
            if (IRO_IsAssignOp[linear->nodetype]) {
                IRO_NotSubable = TRUE;
            }

            inner = linear->u.monadic;
            if (linear->nodetype == EINDIRECT) {
                if (inner->type == IROLinearOp1Arg && inner->nodetype == EBITFIELD) {
                    inner = inner->u.monadic;
                }
                if (inner->type != IROLinearOperand || inner->u.node->type != EOBJREF) {
                    GetDependsOfIndirect(linear);
                }
            }

            IRO_DependsOnForDataFlow(inner, linear->nodetype == EINDIRECT);
            break;
        case IROLinearOp2Arg:
            if (IRO_IsAssignOp[linear->nodetype]) {
                IRO_NotSubable = TRUE;
            }

            if (linear->nodetype == EDIV || linear->nodetype == EMODULO) {
                if (IRO_IsIntConstant(linear->u.diadic.right)) {
                    if (CInt64_Equal(linear->u.diadic.right->u.node->data.intval, cint64_zero)) {
                        IRO_CouldError = TRUE;
                    }
                } else {
                    IRO_CouldError = TRUE;
                }
            }

            IRO_DependsOnForDataFlow(linear->u.diadic.left, isIndirect);
            IRO_DependsOnForDataFlow(linear->u.diadic.right, isIndirect);
            break;
        case IROLinearOp3Arg:
            if (IRO_IsAssignOp[linear->nodetype]) {
                IRO_NotSubable = TRUE;
            }

            IRO_DependsOnForDataFlow(linear->u.args3.a, isIndirect);
            IRO_DependsOnForDataFlow(linear->u.args3.b, isIndirect);
            IRO_DependsOnForDataFlow(linear->u.args3.c, isIndirect);
            break;
        case IROLinearFunccall:
            IRO_NotSubable = TRUE;
            GetDependsOfFunctionCallForDataFlow(linear);
            break;
        default:
            CError_FATAL(650);
    }
}

void IRO_FindDepends_NoAlloc(IROLinear* linear) {
    Bv_Clear(IRO_Depends);
    IRO_CouldError = FALSE;
    IRO_NotSubable = FALSE;
    IRO_IsVolatile = FALSE;
    IRO_DependsOnForDataFlow(linear, FALSE);
}

void IRO_FindDepends(IROLinear* linear) {
    Bv_AllocVector(&IRO_Depends, IRO_NumVars + 1);
    IRO_CouldError = FALSE;
    IRO_NotSubable = FALSE;
    IRO_DependsOn(linear, FALSE);
}

static void VecAct(IROLinear* linear, Boolean isFirst) {
    if (!isFirst && (linear->flags & IROLF_VecOpBase)) {
        HasVectorOperand = TRUE;
    }
}

static Boolean IRO_DoesNotHaveVectorOperand(IROLinear* linear) {
    HasVectorOperand = FALSE;
    IRO_WalkTree(linear, (IROChildExpressionFunc)VecAct, NULL);
    return !HasVectorOperand;
}
