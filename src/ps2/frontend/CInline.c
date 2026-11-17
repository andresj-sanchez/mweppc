#define CERROR_FILE "CInline.c"
#include "compiler/common.h"
#include "compiler/CExpr.h"
#include "compiler/CFunc.h"
#include "compiler/CClass.h"
#include "compiler/CABI.h"
#include "compiler/CMangler.h"
#include "compiler/CMachine.h"
#include "compiler/CInit.h"
#include "compiler/CInline.h"
#include "compiler/CBrowse.h"
#include "compiler/COptimizer.h"
#include "compiler/CParser.h"
#include "compiler/CPrep.h"
#include "compiler/CScope.h"
#include "compiler/scopes.h"
#include "compiler/CTemplateTools.h"
#include "compiler/CompilerTools.h"
#include "compiler/CError.h"
#include "compiler/Switch.h"
#include "compiler/enode.h"
#include "compiler/objects.h"
#include "compiler/CExpr2.h"
#include "compiler/CTemplateFuncInst.h"
#include "compiler/InlineAsm.h"
#include "compiler/RegListQueries.h"
#include "compiler/Registers.h"
#include "driver/DropInCompilerLinker.h"
#include "compiler/CodeGen.h"
#include "compiler/templates.h"

static ObjectList* cinline_freflist;
static Statement* cinline_first_stmt;
static LabelTrans* cinline_label_trans;
static SInt32 cinline_localcount;
static CI_Var* cinline_localvars;
static AObject* local_aobjects;
static IDTrans* enode_idtrans;
static Boolean enode_globalcopy;
static CInlineCopyMode enode_copymode;

static ENode* CInline_CopyNodes(ENode* node);
static void CInline_CheckUsage(ENode* expr, Boolean isModified);
static void CInline_AddFRefList_InlineFunc(CI_FuncData* funcdata);

static SInt32 CInline_TranslateID(SInt32 id) {
    IDTrans* trans;

    for (trans = enode_idtrans; trans; trans = trans->next) {
        if (trans->from == id) {
            return trans->to;
        }
    }

    trans = lalloc(sizeof(IDTrans));
    trans->next = enode_idtrans;
    enode_idtrans = trans;
    trans->from = id;
    trans->to = CParser_GetUniqueID();

    return trans->to;
}

static SInt16 CInline_GetLabelStatementNumber(HashNameNode* name) {
    Statement* stmt;
    SInt16 i;

    for (stmt = cinline_first_stmt, i = 0; stmt; stmt = stmt->next, i++) {
        if (stmt->type == ST_LABEL && stmt->label->uniquename == name) {
            return i;
        }
    }

    CError_FATAL(775);
    return 0;
}

static ENode* CInline_CopyExpressionSave(ENode* expr) {
    CInlineCopyMode save_copymode;
    Boolean save_globalcopy;
    IDTrans* save_idtrans;

    save_globalcopy = enode_globalcopy;
    enode_globalcopy = TRUE;

    save_copymode = enode_copymode;
    enode_copymode = CopyModeExpand;

    save_idtrans = enode_idtrans;
    enode_idtrans = NULL;

    expr = CInline_CopyNodes(expr);

    enode_globalcopy = save_globalcopy;
    enode_copymode = save_copymode;
    enode_idtrans = save_idtrans;

    return expr;
}

static Boolean CInline_IsTrivialExpression(ENode* expr) {
    while (1) {
        switch (expr->type) {
            case EINTCONST:
            case EFLOATCONST:
            case EOBJREF:
                return FALSE;

            case ESCOPEBEGIN:
            case ESCOPEEND:
                return TRUE;

            case ESTRINGCONST:
                return copts.dont_reuse_strings;

            case EOBJACCESS:
                if (expr->data.objaccess.unk19 && expr->data.objaccess.expr) {
                    return CInline_IsTrivialExpression(expr->data.objaccess.expr);
                }
                return FALSE;

            case EINDIRECT:
                if (expr->data.monadic->type == EOBJREF) {
                    Object* object = expr->data.monadic->data.objref;
                    Boolean result;

                    if (object->datatype == DLOCAL && !(object->flags & OBJECT_REFERENCED)) {
                        result = FALSE;
                    } else if (is_const_object(object)) {
                        result = FALSE;
                    } else {
                        result = TRUE;
                    }
                    return result;
                }

                return TRUE;

            case EPOSTINC:
            case EPOSTDEC:
            case EPREINC:
            case EPREDEC:
            case EFORCELOAD:
            case EASS:
            case EMULASS:
            case EDIVASS:
            case EMODASS:
            case EADDASS:
            case ESUBASS:
            case ESHLASS:
            case ESHRASS:
            case EANDASS:
            case EXORASS:
            case EORASS:
            case EFUNCCALL:
            case EFUNCCALLP:
            case ENULLCHECK:
            case EPRECOMP:
            case ELABEL:
            case EMFPOINTER:
            case ETEMP:
            case EINSTRUCTION:
                return TRUE;

            case EMONMIN:
            case EBINNOT:
            case ELOGNOT:
            case ETYPCON:
            case EBITFIELD:
                expr = expr->data.monadic;
                continue;

            case EMUL:
            case EDIV:
            case EMODULO:
            case EADD:
            case ESUB:
            case ESHL:
            case ESHR:
            case ELESS:
            case EGREATER:
            case ELESSEQU:
            case EGREATEREQU:
            case EEQU:
            case ENOTEQU:
            case EAND:
            case EXOR:
            case EOR:
            case ELAND:
            case ELOR:
            case ECOMMA:
            case EMIN:
            case EMAX:
            case EROTL:
            case EROTR:
                if (CInline_IsTrivialExpression(expr->data.diadic.left)) {
                    return TRUE;
                }
                expr = expr->data.diadic.right;
                continue;

            case ECOND:
                if (CInline_IsTrivialExpression(expr->data.cond.cond)) {
                    return TRUE;
                }
                if (CInline_IsTrivialExpression(expr->data.cond.expr1)) {
                    return TRUE;
                }
                expr = expr->data.cond.expr2;
                continue;

            default:
                CError_FATAL(614);
        }
    }
}

Object* CInline_GetLocalID(Object* obj) {
    CI_Var* var;
    SInt32 i;

    if (obj) {
        CError_ASSERT(418, obj->datatype == DLOCAL);

        for (var = cinline_localvars, i = 0; i < cinline_localcount; i++, var++) {
            if (var->obj == (Object*)obj->u.unk2C) {
                var->used = TRUE;
                var->unmodified = FALSE;
                return var->obj;
            }
        }

        CError_FATAL(429);
    }

    return NULL;
}

//! @reconstructed
static inline AObject* CInline_GetLocalAObj(Object* obj) {
    SInt32 i;

    for (i = 0; i < cinline_localcount; i++) {
        if (obj == local_aobjects[i].obj) {
            return &local_aobjects[i];
        }
    }

    CError_FATAL(447);
    return NULL;
}

Object* CInline_GetLocalObj(Object* obj, Boolean inlineCopy) {
    ObjectList* list;
    AObject* aobj;

    if (obj) {
        if (inlineCopy) {
            for (list = ig_reglist_b; list; list = list->next) {
                if (list->object->datatype == DLOCAL && (Object*)list->object->u.unk2C == obj) {
                    return list->object;
                }
            }

            for (list = ig_reglist_a; list; list = list->next) {
                if ((Object*)list->object->u.unk2C == obj) {
                    return list->object;
                }
            }

            CError_FATAL(492);
        }

        aobj = CInline_GetLocalAObj(obj);
        CError_ASSERT(495, aobj->object);
        return (Object*)aobj->object;
    }

    return NULL;
}

Boolean CInline_ExpressionHasSideEffect(ENode* expr) {
    while (1) {
        switch (expr->type) {
            case EINTCONST:
            case EFLOATCONST:
            case ESTRINGCONST:
            case EOBJREF:
            case EPRECOMP:
            case ELABEL:
            case ESCOPEBEGIN:
            case ESCOPEEND:
            case ETEMP:
                return FALSE;

            case EPOSTINC:
            case EPOSTDEC:
            case EPREINC:
            case EPREDEC:
            case EASS:
            case EMULASS:
            case EDIVASS:
            case EMODASS:
            case EADDASS:
            case ESUBASS:
            case ESHLASS:
            case ESHRASS:
            case EANDASS:
            case EXORASS:
            case EORASS:
            case EFUNCCALL:
            case EFUNCCALLP:
            case EINSTRUCTION:
                return TRUE;

            case EINDIRECT:
            case EMONMIN:
            case EBINNOT:
            case ELOGNOT:
            case EFORCELOAD:
            case ETYPCON:
            case EBITFIELD:
                expr = expr->data.monadic;
                continue;

            case EMUL:
            case EDIV:
            case EMODULO:
            case EADD:
            case ESUB:
            case ESHL:
            case ESHR:
            case ELESS:
            case EGREATER:
            case ELESSEQU:
            case EGREATEREQU:
            case EEQU:
            case ENOTEQU:
            case EAND:
            case EXOR:
            case EOR:
            case ELAND:
            case ELOR:
            case ECOMMA:
            case EMIN:
            case EMAX:
            case EROTL:
            case EROTR:
                if (CInline_ExpressionHasSideEffect(expr->data.diadic.left)) {
                    return TRUE;
                }
                expr = expr->data.diadic.right;
                continue;

            case EOBJACCESS:
                if (expr->data.objaccess.unk19 && expr->data.objaccess.expr) {
                    return CInline_ExpressionHasSideEffect(expr->data.objaccess.expr);
                }
                return FALSE;

            case EMFPOINTER:
                if (CInline_ExpressionHasSideEffect(expr->data.mfpointer.accessnode)) {
                    return TRUE;
                }
                expr = expr->data.mfpointer.mfpointer;
                continue;

            case ENULLCHECK:
                if (CInline_ExpressionHasSideEffect(expr->data.nullcheck.nullcheckexpr)) {
                    return TRUE;
                }
                expr = expr->data.nullcheck.condexpr;
                continue;

            case ECOND:
                if (CInline_ExpressionHasSideEffect(expr->data.cond.cond)) {
                    return TRUE;
                }
                if (CInline_ExpressionHasSideEffect(expr->data.cond.expr1)) {
                    return TRUE;
                }
                expr = expr->data.cond.expr2;
                continue;

            default:
                CError_FATAL(722);
        }
    }
}

static ENodeList* CInline_CopyNodeList(ENodeList* list) {
    ENodeList* first;
    ENodeList* last;
    ENodeList* copy;

    first = NULL;
    while (list) {
        if (enode_globalcopy) {
            copy = galloc(sizeof(ENodeList));
        } else {
            copy = lalloc(sizeof(ENodeList));
        }

        copy->node = CInline_CopyNodes(list->node);
        copy->next = NULL;

        if (first) {
            last->next = copy;
            last = copy;
        } else {
            first = last = copy;
        }

        list = list->next;
    }

    return first;
}

//! @reconstructed
static DepName* CInline_CopyDepNameTree(DepName* name) {
    DepName* first;
    DepName* last;

    first = NULL;
    while (name) {
        if (first) {
            last->next = enode_globalcopy ? galloc(sizeof(DepName)) : lalloc(sizeof(DepName));
            last = last->next;
        } else {
            first = last = enode_globalcopy ? galloc(sizeof(DepName)) : lalloc(sizeof(DepName));
        }
        *last = *name;
        if (last->expr) {
            last->expr = CInline_CopyNodes(last->expr);
        }
        if (*(DepName* volatile*)&last->child) {
            last->child = CInline_CopyDepNameTree(last->child);
        }
        name = name->next;
    }

    return first;
}

//! @reconstructed
static TemplDepArg* CInline_CopyTemplDepArg(TemplDepArg* arg) {
    TemplDepArg* copy;

    if (!arg) {
        return NULL;
    }

    if (enode_globalcopy) {
        copy = galloc(sizeof(TemplDepArg));
    } else {
        copy = lalloc(sizeof(TemplDepArg));
    }

    *copy = *arg;

    switch (copy->kind) {
        case 0:
            copy->data = CInline_CopyNodes((ENode*)copy->data);
            break;
        case 1:
            copy->data = CInline_CopyNodeList((ENodeList*)copy->data);
            break;
        case 2:
            copy->data = CInline_CopyDepNameTree((DepName*)copy->data);
            break;
        default:
            CError_FATAL(879);
    }

    return copy;
}

//! @reconstructed
static CtorChain* CInline_CopyCtorChain(CtorChain* chain) {
    CtorChain* copy;
    CtorChain* next;

    if (!chain) {
        return NULL;
    }

    if (enode_globalcopy) {
        copy = galloc(sizeof(CtorChain));
    } else {
        copy = lalloc(sizeof(CtorChain));
    }

    *copy = *chain;
    next = chain->next;
    copy->next = CInline_CopyCtorChain(next);
    if (copy->isexpr) {
        if (copy->expr) {
            copy->expr = CInline_CopyNodes(copy->expr);
        }
    } else {
        copy->expr = (ENode*)CInline_CopyNodeList((ENodeList*)copy->expr);
    }

    return copy;
}

static ENode* CInline_CopyNodes(ENode* node) {
    ENode* copy;
    SInt32 i;

    if (enode_globalcopy) {
        copy = galloc(sizeof(ENode));
    } else {
        copy = lalloc(sizeof(ENode));
    }

    while (1) {
        *copy = *node;
        switch (copy->type) {
            case ESTRINGCONST:
                if (copy->data.string.unk8 &&
                    enode_copymode == CopyModePack &&
                    cscope_currentfunc) {
                    CInit_RewriteStringConst(copy);
                }
                break;

            case ETEMPLDEP:
                switch (copy->data.templdep.subtype) {
                    case TDE_UNK5:
                        if (copy->data.templdep.u0.nsoList) {
                            copy->data.templdep.u0.nsoList = CScope_CopyNameSpaceObjectList(
                                copy->data.templdep.u0.nsoList, enode_globalcopy
                            );
                        }
                        if (copy->data.templdep.expr14) {
                            copy->data.templdep.expr14 =
                                CInline_CopyNodes(copy->data.templdep.expr14);
                        }
                        if (copy->data.templdep.uC.templargs) {
                            copy->data.templdep.uC.templargs =
                                CTemplTool_MakeGlobalTemplArgCopy(copy->data.templdep.uC.templargs);
                        }
                        break;
                    case TDE_UNK1:
                        if (copy->data.templdep.u4.b4.flag) {
                            copy->data.templdep.u0.expr =
                                CInline_CopyNodes(copy->data.templdep.u0.expr);
                        }
                        break;
                    case TDE_UNK2:
                        copy->data.templdep.u0.exprlist =
                            CInline_CopyNodeList(copy->data.templdep.u0.exprlist);
                        break;
                    case TDE_UNK6:
                        copy->data.templdep.u0.expr =
                            CInline_CopyNodes(copy->data.templdep.u0.expr);
                        break;
                    case TDE_UNK9:
                    case TDE_UNK10:
                    case TDE_UNK11:
                    case TDE_UNK21:
                        copy->data.templdep.u0.expr =
                            CInline_CopyNodes(copy->data.templdep.u0.expr);
                        break;
                    case TDE_UNK7:
                        copy->data.templdep.u0.expr =
                            CInline_CopyNodes(copy->data.templdep.u0.expr);
                        copy->data.templdep.u4.args =
                            CInline_CopyNodeList(copy->data.templdep.u4.args);
                        break;
                    case TDE_UNK12:
                        if (copy->data.templdep.expr8) {
                            copy->data.templdep.expr8 =
                                CInline_CopyNodes(copy->data.templdep.expr8);
                        }
                        copy->data.templdep.uC.listC =
                            CInline_CopyNodeList(copy->data.templdep.uC.listC);
                        copy->data.templdep.list10 =
                            CInline_CopyNodeList(copy->data.templdep.list10);
                        break;
                    case TDE_UNK13:
                        copy->data.templdep.u0.expr =
                            CInline_CopyNodes(copy->data.templdep.u0.expr);
                        break;
                    case TDE_UNK14:
                        copy->data.templdep.u0.expr =
                            CInline_CopyNodes(copy->data.templdep.u0.expr);
                        copy->data.templdep.u4.expr =
                            CInline_CopyNodes(copy->data.templdep.u4.expr);
                        break;
                    case TDE_UNK15:
                    case TDE_UNK16:
                    case TDE_UNK17:
                    case TDE_UNK18:
                    case TDE_UNK19:
                        if (copy->data.templdep.u0.expr) {
                            copy->data.templdep.u0.expr =
                                CInline_CopyNodes(copy->data.templdep.u0.expr);
                        }
                        break;
                    case TDE_UNK20:
                        copy->data.templdep.u0.expr =
                            CInline_CopyNodes(copy->data.templdep.u0.expr);
                        copy->data.templdep.u4.name =
                            CExpr_CopyDepName(copy->data.templdep.u4.name);
                        break;
                    case TDE_UNK23:
                    case TDE_UNK24:
                        copy->data.templdep.u4.arg =
                            CInline_CopyTemplDepArg(copy->data.templdep.u4.arg);
                        break;
                    case TDE_PARAM:
                    case TDE_UNK3:
                    case TDE_UNK4:
                    case TDE_UNK8:
                    case TDE_UNK22:
                    case TDE_UNK25:
                        break;
                    default:
                        CError_FATAL(1109);
                }
                break;

            case ETEMP:
                if (enode_copymode == CopyModeUnpack && copy->data.temp.uniqueid) {
                    copy->data.temp.uniqueid = CInline_TranslateID(copy->data.temp.uniqueid);
                }
                break;

            case ELABEL:
                switch (enode_copymode) {
                    case CopyModePack:
                        copy->data.precompid =
                            CInline_GetLabelStatementNumber(copy->data.label->uniquename);
                        return copy;
                    case CopyModeUnpack:
                    case CopyModeExpand: {
                        LabelTrans* trans = lalloc(sizeof(LabelTrans));
                        trans->next = cinline_label_trans;
                        cinline_label_trans = trans;
                        trans->id = copy->data.precompid;
                        trans->labelptr = &copy->data.label;
                        return copy;
                    }
                    default:
                        break;
                }
                break;

            case EINTCONST:
            case EFLOATCONST:
            case EINSTRUCTION:
                break;

            case EPOSTINC:
            case EPOSTDEC:
            case EPREINC:
            case EPREDEC:
            case EMONMIN:
            case EBINNOT:
            case ELOGNOT:
            case EFORCELOAD:
            case ETYPCON:
            case EBITFIELD:
                copy->data.monadic = CInline_CopyNodes(copy->data.monadic);
                break;

ENODE_CASE_DIADIC_ALL:
                copy->data.diadic.left = CInline_CopyNodes(copy->data.diadic.left);
                copy->data.diadic.right = CInline_CopyNodes(copy->data.diadic.right);
                break;

            case ECOND:
                copy->data.cond.cond = CInline_CopyNodes(copy->data.cond.cond);
                copy->data.cond.expr1 = CInline_CopyNodes(copy->data.cond.expr1);
                copy->data.cond.expr2 = CInline_CopyNodes(copy->data.cond.expr2);
                break;

            case EMFPOINTER:
                copy->data.mfpointer.accessnode =
                    CInline_CopyNodes(copy->data.mfpointer.accessnode);
                copy->data.mfpointer.mfpointer = CInline_CopyNodes(copy->data.mfpointer.mfpointer);
                break;

            case EFUNCCALL:
            case EFUNCCALLP:
                copy->data.funccall.funcref = CInline_CopyNodes(copy->data.funccall.funcref);
                copy->data.funccall.args = CInline_CopyNodeList(copy->data.funccall.args);
                break;

            case ENULLCHECK:
                copy->data.nullcheck.precompid =
                    CInline_TranslateID(copy->data.nullcheck.precompid);
                copy->data.nullcheck.nullcheckexpr =
                    CInline_CopyNodes(copy->data.nullcheck.nullcheckexpr);
                copy->data.nullcheck.condexpr = CInline_CopyNodes(copy->data.nullcheck.condexpr);
                break;

            case EPRECOMP:
                copy->data.precompid = CInline_TranslateID(copy->data.precompid);
                break;

            case EINDIRECT:
                if (enode_copymode == CopyModeExpand &&
                    copy->data.monadic->type == EOBJREF &&
                    (copy->data.monadic->flags & ENODE_FLAG_LOCALID)) {
                    for (i = 0; i < cinline_localcount; i++) {
                        if (copy->data.monadic->data.objref == local_aobjects[i].obj) {
                            break;
                        }
                    }
                    CError_ASSERT(447, i < cinline_localcount);
                    if (!local_aobjects[i].object) {
                        CError_ASSERT(1183, local_aobjects[i].useExpr);
                        copy = CInline_CopyExpressionSave(local_aobjects[i].useExpr);
                        if (copy->rtype != node->rtype) {
                            if (IS_TYPE_INT(copy->rtype) && IS_TYPE_INT(node->rtype)) {
                                copy = makemonadicnode(copy, ETYPCON);
                            }
                            copy->rtype = node->rtype;
                        }
                        return copy;
                    }
                }
                copy->data.monadic = CInline_CopyNodes(copy->data.monadic);
                break;

            case EOBJREF:
            case ESCOPEBEGIN:
            case ESCOPEEND:
                if (copy->flags & ENODE_FLAG_LOCALID) {
                    switch (enode_copymode) {
                        case CopyModeUnpack:
                            copy->data.objref = CInline_GetLocalObj(copy->data.objref, TRUE);
                            break;
                        case CopyModeExpand:
                            copy->data.objref = CInline_GetLocalObj(copy->data.objref, FALSE);
                            break;
                        default:
                            CError_FATAL(1216);
                    }
                    CError_ASSERT(1218, copy->data.objref);
                    copy->flags &= ~ENODE_FLAG_LOCALID;
                } else if (enode_copymode == CopyModePack) {
                    if (node->data.objref->datatype == DALIAS) {
                        CExpr_AliasTransform(node);
                        continue;
                    }
                    if (node->data.objref->datatype == DLOCAL) {
                        copy->data.objref = (Object*)node->data.objref->u.unk2C;
                        copy->flags |= ENODE_FLAG_LOCALID;
                    }
                }
                break;

            case ENEWEXCEPTION:
            case ENEWEXCEPTIONARRAY:
                copy->data.newexception.initexpr =
                    CInline_CopyNodes(copy->data.newexception.initexpr);
                copy->data.newexception.tryexpr =
                    CInline_CopyNodes(copy->data.newexception.tryexpr);
                break;

            case EINITTRYCATCH:
                copy->data.itc.initexpr = CInline_CopyNodes(copy->data.itc.initexpr);
                copy->data.itc.tryexpr = CInline_CopyNodes(copy->data.itc.tryexpr);
                copy->data.itc.catchexpr = CInline_CopyNodes(copy->data.itc.catchexpr);
                copy->data.itc.result = CInline_CopyNodes(copy->data.itc.result);
                break;

            case EOBJACCESS:
                if (copy->data.objaccess.expression) {
                    copy->data.objaccess.expression = (ENode*)CScope_CopyNameSpaceObjectList(
                        (NameSpaceObjectList*)copy->data.objaccess.expression, enode_globalcopy
                    );
                }
                if (copy->data.objaccess.expr) {
                    copy->data.objaccess.expr = CInline_CopyNodes(copy->data.objaccess.expr);
                }
                if (copy->data.objaccess.unkC) {
                    copy->data.objaccess.unkC = (SInt32)CTemplTool_MakeGlobalTemplArgCopy(
                        (TemplArg*)copy->data.objaccess.unkC
                    );
                }
                break;

            case ECTORINIT:
                copy->data.ctorinit = CInline_CopyCtorChain(copy->data.ctorinit);
                break;

            default:
                CError_FATAL(1272);
        }

        return copy;
    }
}

//! @reconstructed
static void CInline_CheckDepNameUsage(DepName* name) {
    DepName* child;

    while (name) {
        if (name->expr) {
            CInline_CheckUsage(name->expr, FALSE);
        }
        child = name->child;
        if (child) {
            CInline_CheckDepNameUsage(child);
        }
        name = name->next;
    }
}

static CI_Var* CInline_FindLocalVar(Object* obj) {
    SInt32 i;

    for (i = 0; i < cinline_localcount; i++) {
        if (obj == cinline_localvars[i].obj) {
            return &cinline_localvars[i];
        }
    }

    CError_FATAL(464);
    return NULL;
}

static void CInline_CheckUsage(ENode* expr, Boolean isModified) {
    ENodeList* list;
    ENode* inner;
    CtorChain* chain;
    CI_Var* var;

    while (1) {
        switch (expr->type) {
            case EOBJREF:
                if (expr->flags & ENODE_FLAG_LOCALID) {
                    var = CInline_FindLocalVar(expr->data.objref);
                    var->used = TRUE;
                    var->unmodified = FALSE;
                }
                return;

            case ESCOPEBEGIN:
            case ESCOPEEND:
                if (expr->flags & ENODE_FLAG_LOCALID) {
                    var = CInline_FindLocalVar(expr->data.objref);
                    var->used = TRUE;
                }
                return;

            case EINDIRECT:
                inner = expr->data.monadic;
                if (inner->type == EOBJREF && (inner->flags & ENODE_FLAG_LOCALID)) {
                    var = CInline_FindLocalVar(inner->data.objref);
                    var->used = TRUE;
                    if (isModified) {
                        var->unmodified = FALSE;
                    }
                    return;
                }
                isModified = FALSE;
                expr = inner;
                continue;

            case EMONMIN:
            case EBINNOT:
            case ELOGNOT:
            case EFORCELOAD:
            case ETYPCON:
            case EBITFIELD:
                isModified = FALSE;
                expr = expr->data.monadic;
                continue;

            case EPOSTINC:
            case EPOSTDEC:
            case EPREINC:
            case EPREDEC:
                expr = expr->data.monadic;
                isModified = TRUE;
                continue;

            case EASS:
            case EMULASS:
            case EDIVASS:
            case EMODASS:
            case EADDASS:
            case ESUBASS:
            case ESHLASS:
            case ESHRASS:
            case EANDASS:
            case EXORASS:
            case EORASS:
                CInline_CheckUsage(expr->data.diadic.left, TRUE);
                isModified = FALSE;
                expr = expr->data.diadic.right;
                continue;

            case EMUL:
            case EDIV:
            case EMODULO:
            case EADD:
            case ESUB:
            case ESHL:
            case ESHR:
            case ELESS:
            case EGREATER:
            case ELESSEQU:
            case EGREATEREQU:
            case EEQU:
            case ENOTEQU:
            case EAND:
            case EXOR:
            case EOR:
            case ELAND:
            case ELOR:
            case ECOMMA:
            case EMIN:
            case EMAX:
            case EROTL:
            case EROTR:
            case EPOINTERSTAR:
            case EDOTSTAR:
                CInline_CheckUsage(expr->data.diadic.left, FALSE);
                isModified = FALSE;
                expr = expr->data.diadic.right;
                continue;

            case EINTCONST:
            case EFLOATCONST:
            case ESTRINGCONST:
            case EPRECOMP:
            case ELABEL:
            case ETEMP:
                return;

            case EOBJACCESS:
                if (expr->data.objaccess.unk19 && expr->data.objaccess.expr) {
                    CInline_CheckUsage(expr->data.objaccess.expr, FALSE);
                }
                return;

            case ECTORINIT:
                for (chain = expr->data.ctorinit; chain; chain = chain->next) {
                    if (chain->isexpr) {
                        if (chain->expr) {
                            CInline_CheckUsage(chain->expr, FALSE);
                        }
                    } else {
                        for (list = (ENodeList*)chain->expr; list; list = list->next) {
                            CInline_CheckUsage(list->node, FALSE);
                        }
                    }
                }
                return;

            case EFUNCCALL:
            case EFUNCCALLP:
                CInline_CheckUsage(expr->data.funccall.funcref, FALSE);
                for (list = expr->data.funccall.args; list; list = list->next) {
                    CInline_CheckUsage(list->node, FALSE);
                }
                return;

            case ENULLCHECK:
                CInline_CheckUsage(expr->data.nullcheck.nullcheckexpr, FALSE);
                isModified = FALSE;
                expr = expr->data.nullcheck.condexpr;
                continue;

            case EMFPOINTER:
                CInline_CheckUsage(expr->data.mfpointer.accessnode, FALSE);
                isModified = FALSE;
                expr = expr->data.mfpointer.mfpointer;
                continue;

            case ECOND:
                CInline_CheckUsage(expr->data.cond.cond, FALSE);
                CInline_CheckUsage(expr->data.cond.expr1, FALSE);
                isModified = FALSE;
                expr = expr->data.cond.expr2;
                continue;

            case EINSTRUCTION:
                return;

            case ENEWEXCEPTION:
            case ENEWEXCEPTIONARRAY:
                CInline_CheckUsage(expr->data.newexception.initexpr, FALSE);
                CInline_CheckUsage(expr->data.newexception.tryexpr, FALSE);
                return;

            case EINITTRYCATCH:
                CInline_CheckUsage(expr->data.itc.initexpr, FALSE);
                CInline_CheckUsage(expr->data.itc.tryexpr, FALSE);
                CInline_CheckUsage(expr->data.itc.catchexpr, FALSE);
                CInline_CheckUsage(expr->data.itc.result, FALSE);
                return;

            case ETEMPLDEP:
                switch (expr->data.templdep.subtype) {
                    case TDE_UNK5:
                        if (expr->data.templdep.expr14) {
                            CInline_CheckUsage(expr->data.templdep.expr14, FALSE);
                        }
                        break;
                    case TDE_UNK1:
                        if (expr->data.templdep.u4.b4.flag) {
                            CInline_CheckUsage(expr->data.templdep.u0.expr, FALSE);
                        }
                        break;
                    case TDE_UNK2:
                        for (list = expr->data.templdep.u0.exprlist; list; list = list->next) {
                            CInline_CheckUsage(list->node, FALSE);
                        }
                        break;
                    case TDE_UNK6:
                        CInline_CheckUsage(expr->data.templdep.u0.expr, FALSE);
                        break;
                    case TDE_UNK9:
                    case TDE_UNK10:
                    case TDE_UNK11:
                    case TDE_UNK21:
                        CInline_CheckUsage(expr->data.templdep.u0.expr, FALSE);
                        break;
                    case TDE_UNK7:
                        CInline_CheckUsage(expr->data.templdep.u0.expr, FALSE);
                        for (list = expr->data.templdep.u4.args; list; list = list->next) {
                            CInline_CheckUsage(list->node, FALSE);
                        }
                        break;
                    case TDE_UNK12:
                        if (expr->data.templdep.expr8) {
                            CInline_CheckUsage(expr->data.templdep.expr8, FALSE);
                        }
                        for (list = expr->data.templdep.uC.listC; list; list = list->next) {
                            CInline_CheckUsage(list->node, FALSE);
                        }
                        for (list = expr->data.templdep.list10; list; list = list->next) {
                            CInline_CheckUsage(list->node, FALSE);
                        }
                        break;
                    case TDE_UNK13:
                        CInline_CheckUsage(expr->data.templdep.u0.expr, FALSE);
                        break;
                    case TDE_UNK14:
                        CInline_CheckUsage(expr->data.templdep.u0.expr, FALSE);
                        CInline_CheckUsage(expr->data.templdep.u4.expr, FALSE);
                        break;
                    case TDE_UNK15:
                    case TDE_UNK16:
                    case TDE_UNK17:
                    case TDE_UNK18:
                    case TDE_UNK19:
                        if (expr->data.templdep.u0.expr) {
                            CInline_CheckUsage(expr->data.templdep.u0.expr, FALSE);
                        }
                        break;
                    case TDE_UNK20:
                        CInline_CheckUsage(expr->data.templdep.u0.expr, FALSE);
                        break;
                    case TDE_UNK23:
                    case TDE_UNK24:
                        if (expr->data.templdep.u4.arg) {
                            switch (expr->data.templdep.u4.arg->kind) {
                                case 0:
                                    CInline_CheckUsage(
                                        (ENode*)expr->data.templdep.u4.arg->data, FALSE
                                    );
                                    break;
                                case 1:
                                    for (list = (ENodeList*)expr->data.templdep.u4.arg->data; list;
                                         list = list->next) {
                                        CInline_CheckUsage(list->node, FALSE);
                                    }
                                    break;
                                case 2:
                                    CInline_CheckDepNameUsage(
                                        (DepName*)expr->data.templdep.u4.arg->data
                                    );
                                    break;
                                default:
                                    CError_FATAL(1561);
                            }
                        }
                        break;
                    case TDE_PARAM:
                    case TDE_UNK3:
                    case TDE_UNK4:
                    case TDE_UNK8:
                    case TDE_UNK22:
                    case TDE_UNK25:
                        break;
                    default:
                        CError_FATAL(1567);
                }
                return;

            default:
                CError_FATAL(1589);
        }
    }
}

ENode* CInline_CopyExpression(ENode* expr, CInlineCopyMode mode) {
    enode_copymode = mode;

    switch (mode) {
        case CopyModeLocal:
        case CopyModeExpand:
            enode_idtrans = NULL;
            enode_globalcopy = FALSE;
            expr = CInline_CopyNodes(expr);
            break;
        case CopyModeUnpack:
            enode_globalcopy = FALSE;
            expr = CInline_CopyNodes(expr);
            break;
        case CopyModeGlobal:
            enode_idtrans = NULL;
            enode_globalcopy = TRUE;
            expr = CInline_CopyNodes(expr);
            break;
        case CopyModePack:
            enode_idtrans = NULL;
            enode_globalcopy = TRUE;
            expr = CInline_CopyNodes(expr);
            CInline_CheckUsage(expr, FALSE);
            break;
    }

    return expr;
}

static void CInline_AddFRefList_Object(Object* object) {
    ObjectList* list;

    if (object->datatype != DFUNC && object->datatype != DVFUNC ||
        (object->flags & OBJECT_DEFINED) ||
        IS_TEMPL_FUNC(object->type)) {
        return;
    }

    for (list = cinline_freflist; list; list = list->next) {
        if (list->object == object) {
            return;
        }
    }

    list = lalloc(sizeof(ObjectList));
    list->object = object;
    list->next = cinline_freflist;
    cinline_freflist = list;

    if ((object->qual & Q_INLINE) && *(CI_FuncData* volatile*)&object->u.ifuncdata) {
        CInline_AddFRefList_InlineFunc(object->u.ifuncdata);
    }
}

static void CInline_AddFRefList_ExAction(ExceptionAction* exc) {
    while (exc) {
        switch (exc->type) {
            case EAT_DESTROYLOCAL:
                CInline_AddFRefList_Object(exc->data.destroy_local.dtor);
                break;
            case EAT_DESTROYLOCALCOND:
                CInline_AddFRefList_Object(exc->data.destroy_local_cond.dtor);
                break;
            case EAT_DESTROYLOCALOFFSET:
                CInline_AddFRefList_Object(exc->data.destroy_local_offset.dtor);
                break;
            case EAT_DESTROYLOCALPOINTER:
                CInline_AddFRefList_Object(exc->data.destroy_local_pointer.dtor);
                break;
            case EAT_DESTROYLOCALARRAY:
                CInline_AddFRefList_Object(exc->data.destroy_local_array.dtor);
                break;
            case EAT_DESTROYPARTIALARRAY:
                CInline_AddFRefList_Object(exc->data.destroy_partial_array.dtor);
                break;
            case EAT_DESTROYMEMBER:
            case EAT_DESTROYBASE:
                CInline_AddFRefList_Object(exc->data.destroy_member.dtor);
                break;
            case EAT_DESTROYMEMBERCOND:
                CInline_AddFRefList_Object(exc->data.destroy_member_cond.dtor);
                break;
            case EAT_DESTROYMEMBERARRAY:
                CInline_AddFRefList_Object(exc->data.destroy_member_array.dtor);
                break;
            case EAT_DELETEPOINTER:
            case EAT_DELETELOCALPOINTER:
                CInline_AddFRefList_Object(exc->data.delete_pointer.deletefunc);
                break;
            case EAT_DELETEPOINTERCOND:
                CInline_AddFRefList_Object(exc->data.delete_pointer_cond.deletefunc);
                break;
            case EAT_CATCHBLOCK:
            case EAT_ACTIVECATCHBLOCK:
            case EAT_SPECIFICATION:
            case EAT_TERMINATE:
            case EAT_DELETEVLA:
                break;
            default:
                CError_FATAL(4840);
        }

        exc = exc->prev;
    }
}

static void CInline_AddFRefList_ExprCB(ENode* expr) {
    if (expr->type == EOBJREF && !(expr->flags & ENODE_FLAG_LOCALID)) {
        CInline_AddFRefList_Object(expr->data.objref);
    }
}

static void CInline_AddFRefList_InlineFunc(CI_FuncData* funcdata) {
    SInt16 i;
    ExceptionAction* exc;

    for (i = 0; i < funcdata->numstatements; i++) {
        switch (funcdata->statements[i].type) {
            case ST_EXPRESSION:
            case ST_BEGINCATCH:
            case ST_ENDCATCH:
            case ST_ENDCATCHDTOR:
                CExpr_SearchExprTree(funcdata->statements[i].u.expr, CInline_AddFRefList_ExprCB);
                break;
            case ST_RETURN:
                if (funcdata->statements[i].u.expr) {
                    CExpr_SearchExprTree(
                        funcdata->statements[i].u.expr, CInline_AddFRefList_ExprCB
                    );
                }
                break;
            case ST_IFGOTO:
            case ST_IFNGOTO:
                CExpr_SearchExprTree(
                    funcdata->statements[i].u.ifgoto.expr, CInline_AddFRefList_ExprCB
                );
                break;
            case ST_SWITCH:
                CExpr_SearchExprTree(
                    funcdata->statements[i].u.switchdata->expr, CInline_AddFRefList_ExprCB
                );
                break;
            case ST_OVF:
            case ST_EXIT:
            case ST_ENTRY:
            case ST_GOTOEXPR:
            default:
                CError_FATAL(4954);
            case ST_NOP:
            case ST_LABEL:
            case ST_GOTO:
            case ST_ASM:
                break;
        }

        for (exc = funcdata->statements[i].dobjstack; exc; exc = exc->prev) {
            switch (exc->type) {
                case EAT_DESTROYLOCAL:
                    CInline_AddFRefList_Object(exc->data.destroy_local.dtor);
                    break;
                case EAT_DESTROYLOCALCOND:
                    CInline_AddFRefList_Object(exc->data.destroy_local_cond.dtor);
                    break;
                case EAT_DESTROYLOCALOFFSET:
                    CInline_AddFRefList_Object(exc->data.destroy_local_offset.dtor);
                    break;
                case EAT_DESTROYLOCALPOINTER:
                    CInline_AddFRefList_Object(exc->data.destroy_local_pointer.dtor);
                    break;
                case EAT_DESTROYLOCALARRAY:
                    CInline_AddFRefList_Object(exc->data.destroy_local_array.dtor);
                    break;
                case EAT_DESTROYPARTIALARRAY:
                    break;
                case EAT_DESTROYMEMBER:
                case EAT_DESTROYBASE:
                    CInline_AddFRefList_Object(exc->data.destroy_member.dtor);
                    break;
                case EAT_DESTROYMEMBERCOND:
                    CInline_AddFRefList_Object(exc->data.destroy_member_cond.dtor);
                    break;
                case EAT_DESTROYMEMBERARRAY:
                    CInline_AddFRefList_Object(exc->data.destroy_member_array.dtor);
                    break;
                case EAT_DELETEPOINTER:
                case EAT_DELETELOCALPOINTER:
                    CInline_AddFRefList_Object(exc->data.delete_pointer.deletefunc);
                    break;
                case EAT_DELETEPOINTERCOND:
                    CInline_AddFRefList_Object(exc->data.delete_pointer_cond.deletefunc);
                    break;
                case EAT_CATCHBLOCK:
                case EAT_ACTIVECATCHBLOCK:
                case EAT_SPECIFICATION:
                case EAT_TERMINATE:
                case EAT_DELETEVLA:
                    break;
                default:
                    CError_FATAL(5005);
            }
        }
    }
}

static Statement* cinline_serial_stmt;
static Statement* cinline_cur_serial_stmt;
typedef struct UIDTemp {
    struct UIDTemp* next;
    Object* object;
    SInt32 uid;
} UIDTemp;

static UIDTemp* cinline_uid_temps;

static Statement* CInline_NewStatement(StatementType sttype) {
    Statement* stmt = lalloc(sizeof(Statement));
    memclrw(stmt, sizeof(Statement));

    stmt->type = sttype;
    if (cinline_serial_stmt) {
        cinline_cur_serial_stmt->next = stmt;
    } else {
        cinline_serial_stmt = stmt;
    }

    return cinline_cur_serial_stmt = stmt;
}

static void fn_0050C330(ENode* expr) {
    switch (expr->type) {
        case EINDIRECT:
        case EFUNCCALL:
        case EFUNCCALLP:
            cinline_has_sideeffect = 1;
            break;
        default:
            break;
    }
}

static Boolean recursive_inline;
static Object* expanding_function;
static SInt32 inline_max_size;
static SInt16 cinline_level;
static Boolean cinline_unconditionalpart;

static Boolean CInline_CanExpand(ENode* expr) {
    TypeFunc* tfunc;
    Object* object;

    object = expr->data.objref;
    tfunc = TYPE_FUNC(object->type);

    if (IS_TYPE_FUNC(tfunc) &&
        ((object->qual & Q_INLINE) || (tfunc->flags & FUNC_AUTO_INLINE)) &&
        (object->datatype == DFUNC ||
         (object->datatype == DVFUNC && (expr->flags & ENODE_FLAG_NONVIRTUAL)))) {
        return TRUE;
    }

    return FALSE;
}

static SInt32 CInline_EstimateSizeOfFunc(CI_FuncData* funcdata, SInt32 size, SInt32 level);

static SInt32 CInline_EstimateSizeOfExpr(ENode* expr, SInt32 size, SInt32 level) {
    ENodeList* list;

    switch (expr->type) {
        case EPOSTINC:
        case EPOSTDEC:
        case EPREINC:
        case EPREDEC:
        case EINDIRECT:
        case EMONMIN:
        case EBINNOT:
        case ELOGNOT:
        case EFORCELOAD:
        case ETYPCON:
        case EBITFIELD:
            size = CInline_EstimateSizeOfExpr(expr->data.monadic, size, level) + 1;
            break;

        case EMUL:
        case EMULV:
        case EDIV:
        case EMODULO:
        case EADDV:
        case ESUBV:
        case EADD:
        case ESUB:
        case ESHL:
        case ESHR:
        case ELESS:
        case EGREATER:
        case ELESSEQU:
        case EGREATEREQU:
        case EEQU:
        case ENOTEQU:
        case EAND:
        case EXOR:
        case EOR:
        case ELAND:
        case ELOR:
        case EASS:
        case EMULASS:
        case EDIVASS:
        case EMODASS:
        case EADDASS:
        case ESUBASS:
        case ESHLASS:
        case ESHRASS:
        case EANDASS:
        case EXORASS:
        case EORASS:
        case ECOMMA:
        case EMIN:
        case EMAX:
        case EPMODULO:
        case EROTL:
        case EROTR:
        case EBCLR:
        case EBTST:
        case EBSET:
        case EPOINTERSTAR:
        case EDOTSTAR:
            size = CInline_EstimateSizeOfExpr(expr->data.diadic.left, size, level);
            if (size <= inline_max_size) {
                size = CInline_EstimateSizeOfExpr(expr->data.diadic.right, size, level) + 1;
            }
            break;

        case EFUNCCALL:
        case EFUNCCALLP: {
            ENode* funcref = expr->data.funccall.funcref;
            if (funcref->type == EOBJREF &&
                funcref->data.objref->u.ifuncdata &&
                CInline_CanExpand(funcref)) {
                recursive_inline |= funcref->data.objref == expanding_function;
                if (level == 0) {
                    if (!recursive_inline) {
                        size = inline_max_size + 1;
                    }
                } else {
                    size = CInline_EstimateSizeOfFunc(
                        funcref->data.objref->u.ifuncdata, size, level - 1
                    );
                }
            } else {
                size++;
            }

            for (list = expr->data.funccall.args; list; list = list->next) {
                if (size > inline_max_size) {
                    break;
                }

                size = CInline_EstimateSizeOfExpr(list->node, size, level);
            }
            break;
        }

        case ECOND:
            size = CInline_EstimateSizeOfExpr(expr->data.cond.cond, size, level);
            if (size <= inline_max_size) {
                size = CInline_EstimateSizeOfExpr(expr->data.cond.expr1, size, level) + 1;
            }
            if (size <= inline_max_size) {
                size = CInline_EstimateSizeOfExpr(expr->data.cond.expr2, size, level) + 1;
            }
            break;

        case ENULLCHECK:
            size = CInline_EstimateSizeOfExpr(expr->data.nullcheck.nullcheckexpr, size, level);
            if (size <= inline_max_size) {
                size = CInline_EstimateSizeOfExpr(expr->data.nullcheck.condexpr, size, level) + 1;
            }
            break;

        case EMFPOINTER:
            size = CInline_EstimateSizeOfExpr(expr->data.mfpointer.accessnode, size, level);
            if (size <= inline_max_size) {
                size = CInline_EstimateSizeOfExpr(expr->data.mfpointer.mfpointer, size, level) + 1;
            }
            break;

        case EOBJACCESS:
            if (expr->data.objaccess.unk19 && expr->data.objaccess.expr) {
                size = CInline_EstimateSizeOfExpr(expr->data.templdep.expr14, size, level) + 1;
            }
            break;

        default:
            size++;
    }

    return size;
}

static SInt32 CInline_EstimateSizeOfFunc(CI_FuncData* funcdata, SInt32 size, SInt32 level) {
    CI_Statement* stmt;
    SInt32 i;

    size += funcdata->numstatements;
    if (size > inline_max_size) {
        return size;
    }

    i = 0;
    stmt = funcdata->statements;
    if (funcdata->numstatements > 0) {
        do {
            switch (stmt->type) {
                case ST_NOP:
                case ST_LABEL:
                case ST_GOTO:
                case ST_ASM:
                    break;
                case ST_EXPRESSION:
                case ST_IFGOTO:
                case ST_IFNGOTO:
                case ST_BEGINCATCH:
                case ST_ENDCATCH:
                case ST_ENDCATCHDTOR:
                    size = CInline_EstimateSizeOfExpr(stmt->u.expr, size, level);
                    break;
                case ST_SWITCH:
                    size = CInline_EstimateSizeOfExpr(stmt->u.switchdata->expr, size, level);
                    break;
                case ST_RETURN:
                    if (stmt->u.expr) {
                        size = CInline_EstimateSizeOfExpr(stmt->u.expr, size, level);
                    }
                    break;
                default:
                    CError_FATAL(2276);
            }

            if (size > inline_max_size) {
                break;
            }
            i++;
            stmt++;
        } while (i < funcdata->numstatements);
    }

    return size;
}

static SInt32 EstimateExpandedSizeOfExpr(ENode* expr, SInt32 level) {
    ENodeList* list;
    SInt32 size;

    size = 0;

    switch (expr->type) {
        case EPOSTINC:
        case EPOSTDEC:
        case EPREINC:
        case EPREDEC:
        case EINDIRECT:
        case EMONMIN:
        case EBINNOT:
        case ELOGNOT:
        case EFORCELOAD:
        case ETYPCON:
        case EBITFIELD:
            size++;
            size += EstimateExpandedSizeOfExpr(expr->data.monadic, level);
            break;

        case EMUL:
        case EMULV:
        case EDIV:
        case EMODULO:
        case EADDV:
        case ESUBV:
        case EADD:
        case ESUB:
        case ESHL:
        case ESHR:
        case ELESS:
        case EGREATER:
        case ELESSEQU:
        case EGREATEREQU:
        case EEQU:
        case ENOTEQU:
        case EAND:
        case EXOR:
        case EOR:
        case ELAND:
        case ELOR:
        case EASS:
        case EMULASS:
        case EDIVASS:
        case EMODASS:
        case EADDASS:
        case ESUBASS:
        case ESHLASS:
        case ESHRASS:
        case EANDASS:
        case EXORASS:
        case EORASS:
        case ECOMMA:
        case EMIN:
        case EMAX:
        case EPMODULO:
        case EROTL:
        case EROTR:
        case EBCLR:
        case EBTST:
        case EBSET:
        case EPOINTERSTAR:
        case EDOTSTAR:
            size++;
            size += EstimateExpandedSizeOfExpr(expr->data.diadic.left, level);
            size += EstimateExpandedSizeOfExpr(expr->data.diadic.right, level);
            break;

        case EFUNCCALL:
        case EFUNCCALLP: {
            ENode* funcref = expr->data.funccall.funcref;
            if (funcref->type == EOBJREF &&
                funcref->data.objref->u.ifuncdata &&
                CInline_CanExpand(funcref)) {
                if (level) {
                    SInt32 est =
                        CInline_EstimateSizeOfFunc(funcref->data.objref->u.ifuncdata, 0, level - 1);
                    if (est > inline_max_size) {
                        size++;
                    } else {
                        size += est;
                    }
                } else {
                    size++;
                }
            } else {
                size++;
            }

            for (list = expr->data.funccall.args; list; list = list->next) {
                size += EstimateExpandedSizeOfExpr(list->node, level);
            }
            break;
        }

        case ECOND:
            size++;
            size += EstimateExpandedSizeOfExpr(expr->data.cond.cond, level);
            size += EstimateExpandedSizeOfExpr(expr->data.cond.expr1, level);
            size += EstimateExpandedSizeOfExpr(expr->data.cond.expr2, level);
            break;

        case ENULLCHECK:
            size++;
            size += EstimateExpandedSizeOfExpr(expr->data.nullcheck.nullcheckexpr, level);
            size += EstimateExpandedSizeOfExpr(expr->data.nullcheck.condexpr, level);
            break;

        case EMFPOINTER:
            size++;
            size += EstimateExpandedSizeOfExpr(expr->data.mfpointer.accessnode, level);
            size += EstimateExpandedSizeOfExpr(expr->data.mfpointer.mfpointer, level);
            break;

        case EOBJACCESS:
            if (expr->data.objaccess.unk19 && expr->data.objaccess.expr) {
                size += EstimateExpandedSizeOfExpr(expr->data.templdep.expr14, level);
            }
            break;

        case EINTCONST:
        case EFLOATCONST:
        case EOBJREF:
        case ESCOPEBEGIN:
        case ESCOPEEND:
            break;

        default:
            size++;
    }

    return size;
}

static SInt32 EstimateExpandedSizeOfFunction(Statement* stmt, Boolean zeroLevel) {
    SInt32 size;
    SInt32 level;

    if (zeroLevel) {
        level = 0;
    } else {
        level = copts.inlinelevel;
        if (!level) {
            level = 8;
        }
    }

    size = 0;

    while (stmt) {
        switch (stmt->type) {
            case ST_NOP:
                break;
            case ST_EXPRESSION:
                if (stmt->expr->type == ESCOPEBEGIN || stmt->expr->type == ESCOPEEND) {
                    break;
                }
            case ST_SWITCH:
            case ST_IFGOTO:
            case ST_IFNGOTO:
            case ST_BEGINCATCH:
            case ST_ENDCATCH:
            case ST_ENDCATCHDTOR:
                size++;
                size += EstimateExpandedSizeOfExpr(stmt->expr, level);
                break;
            case ST_RETURN:
                size++;
                if (stmt->expr) {
                    size += EstimateExpandedSizeOfExpr(stmt->expr, level);
                }
                break;
            case ST_LABEL:
            case ST_GOTO:
            case ST_ASM:
                size++;
                break;
            default:
                CError_FATAL(2477);
        }

        stmt = stmt->next;
    }

    return size;
}

static Boolean CInline_InlineFunctionCheck(ENode* expr);

static Boolean any_inline_expanded;
static Boolean inline_expanded;
static Boolean cinline_serialize_stmt;
static SInt16 cinline_stmtlevelexprs;
static ENode* cinline_stmtlevelexpr[16];

static ENode* CInline_RefArgTransform(ENode* expr, Boolean convertArg);

static ENode* CInline_SetupArgsExpression(Object* object, CI_FuncData* data, ENodeList* list) {
    ENode* commaNodes;
    ENode* expr;
    CI_Var* var;
    ENodeList* scan;
    Object* obj;
    SInt32 i;
    Boolean is_oldstyle;
    Type* vtype;
    UInt32 vqual;
    UInt8 vsflags;
    Boolean makeLocal;

    is_oldstyle = FALSE;
    if (TYPE_FUNC(object->type)->args == &oldstyle) {
        is_oldstyle = TRUE;
    }

    cinline_localcount = data->numlocals;
    local_aobjects = lalloc(sizeof(AObject) * cinline_localcount);

    for (i = 0, var = data->arguments, scan = list; i < data->numlocals; i++, var++) {
        local_aobjects[i].obj = NULL;
        local_aobjects[i].object = NULL;
        local_aobjects[i].useExpr = NULL;
        local_aobjects[i].evalExpr = NULL;

        if (var->used) {
            local_aobjects[i].obj = var->obj;
            makeLocal = TRUE;

            if (var->sflags & 0x40) {
                if (scan &&
                    var->unmodified &&
                    !CInline_IsTrivialExpression(scan->node) &&
                    (!is_oldstyle || scan->node->rtype->size == var->type->size)) {
                    local_aobjects[i].useExpr = scan->node;
                    makeLocal = FALSE;
                } else if (scan && var->type->type == TYPEPOINTER && (var->qual & 0x20)) {
                    expr = CInline_RefArgTransform(scan->node, var->unmodified);
                    if (expr) {
                        local_aobjects[i].useExpr = expr;
                        local_aobjects[i].evalExpr = scan->node;
                        makeLocal = FALSE;
                    }
                }
            }

            if (makeLocal) {
                vtype = var->type;
                vqual = var->qual;
                vsflags = var->sflags;
                obj = CParser_NewLocalDataObject(NULL, TRUE);
                obj->name = CParser_GetUniqueName();
                obj->type = vtype;
                obj->qual = vqual;
                if (vsflags & CI_SFLAGS_Referenced) {
                    obj->flags |= OBJECT_REFERENCED;
                }
                switch (vsflags & 3) {
                    case CI_SFLAGS_NoClass:
                        obj->sclass = 0;
                        break;
                    case CI_SFLAGS_Register:
                        obj->sclass = TK_REGISTER;
                        break;
                    case CI_SFLAGS_Auto:
                        obj->sclass = TK_AUTO;
                        break;
                    default:
                        CError_FATAL(1699);
                }
                CFunc_SetupLocalVarInfo(obj);
                local_aobjects[i].object = obj;
                obj->u.var.info->unknown2D = var->unk12;
            }
        } else {
            vtype = var->type;
            vqual = var->qual;
            vsflags = var->sflags;
            obj = CParser_NewLocalDataObject(NULL, TRUE);
            obj->name = CParser_GetUniqueName();
            obj->type = vtype;
            obj->qual = vqual;
            if (vsflags & CI_SFLAGS_Referenced) {
                obj->flags |= OBJECT_REFERENCED;
            }
            switch (vsflags & 3) {
                case CI_SFLAGS_NoClass:
                    obj->sclass = 0;
                    break;
                case CI_SFLAGS_Register:
                    obj->sclass = TK_REGISTER;
                    break;
                case CI_SFLAGS_Auto:
                    obj->sclass = TK_AUTO;
                    break;
                default:
                    CError_FATAL(1699);
            }
            CFunc_SetupLocalVarInfo(obj);
            local_aobjects[i].object = obj;
            obj->u.var.info->unknown2D = var->unk12;
            if (!var->unmodified) {
                obj->flags |= OBJECT_REFERENCED;
            }
        }

        if (scan) {
            scan = scan->next;
        }
    }

    commaNodes = NULL;

    for (i = 0, scan = list; scan; scan = scan->next, i++) {
        if (i >= data->numarguments) {
            if (!commaNodes) {
                commaNodes = scan->node;
            } else {
                commaNodes = makecommaexpression(scan->node, commaNodes);
            }
        } else if (!local_aobjects[i].object || local_aobjects[i].evalExpr) {
            if (local_aobjects[i].evalExpr) {
                if (!commaNodes) {
                    commaNodes = local_aobjects[i].evalExpr;
                } else {
                    commaNodes = makecommaexpression(local_aobjects[i].evalExpr, commaNodes);
                }
            } else if (!local_aobjects[i].useExpr && CInline_IsTrivialExpression(scan->node)) {
                if (!commaNodes) {
                    commaNodes = scan->node;
                } else {
                    commaNodes = makecommaexpression(scan->node, commaNodes);
                }
                CError_ASSERT(1470, scan->node->type != EPRECOMP);
            }
        } else {
            if (is_oldstyle && scan->node->rtype->size != local_aobjects[i].object->type->size) {
                scan->node = makemonadicnode(scan->node, ETYPCON);
                scan->node->rtype = local_aobjects[i].object->type;
            }

            expr = makediadicnode(create_objectnode2(local_aobjects[i].object), scan->node, EASS);
            if (!commaNodes) {
                commaNodes = expr;
            } else {
                commaNodes = makecommaexpression(expr, commaNodes);
            }
        }
    }

    return commaNodes;
}

static ENode* CInline_ReturnCheck(ENode* expr) {
    ENode* copy;

    if (expr->type == EFORCELOAD) {
        return expr;
    }

    cinline_has_sideeffect = FALSE;
    CExpr_SearchExprTree(expr, fn_0050C330);

    if (!cinline_has_sideeffect) {
        return expr;
    }

    copy = CExpr_CopyENode(expr);
    copy->type = EFORCELOAD;
    copy->data.monadic = expr;

    return copy;
}

static ENode* CInline_ReturnMemResult(Object* object) {
    SInt16 index;

    index = CABI_GetStructResultArgumentIndex(TYPE_FUNC(object->type));
    if (local_aobjects[index].object == NULL) {
        return CInline_CopyExpressionSave(local_aobjects[index].useExpr);
    } else {
        return create_objectnode(local_aobjects[index].object);
    }
}

static ENode* CInline_RefArgTransform(ENode* expr, Boolean convertArg) {
    ENodeList* arg;

    while (expr->type == ECOMMA) {
        expr = expr->data.diadic.right;
    }

    switch (expr->type) {
        case EOBJREF:
        case ETEMP:
            if (convertArg) {
                switch (expr->type) {
                    case EOBJREF:
                        return CExpr_CopyENode(expr);
                    default:
                        return NULL;
                    case ETEMP:
                        CError_FATAL(1738);
                }
            }
            break;

        case EFUNCCALL:
            if (IS_TYPE_POINTER_ONLY(expr->rtype) && IS_TYPE_CLASS(TPTR_TARGET(expr->rtype))) {
                if (expr->data.funccall.funcref->type == EOBJREF &&
                    CClass_IsConstructor(expr->data.funccall.funcref->data.objref) &&
                    (arg = expr->data.funccall.args)) {
                    switch (arg->node->type) {
                        case EOBJREF:
                            return CExpr_CopyENode(arg->node);
                        default:
                            return NULL;
                        case ETEMP:
                            CError_FATAL(1738);
                    }
                }

                if (TPTR_TARGET(expr->rtype) == expr->data.funccall.functype->functype &&
                    CMach_GetFunctionResultClass(expr->data.funccall.functype) == 1 &&
                    (arg = expr->data.funccall.args)) {
                    SInt16 index;

                    index = CABI_GetStructResultArgumentIndex(expr->data.funccall.functype);
                    switch (index) {
                        case 0:
                            break;
                        case 1:
                            if ((arg = arg->next)) {
                                break;
                            }
                            CError_FATAL(1779);
                        default:
                            CError_FATAL(1780);
                    }

                    switch (arg->node->type) {
                        case EOBJREF:
                            return CExpr_CopyENode(arg->node);
                        default:
                            return NULL;
                        case ETEMP:
                            CError_FATAL(1738);
                    }
                }
            }
            break;

        default:
            break;
    }

    return NULL;
}

static ENode* CInline_InlineFunctionExpression(ENode* expr) {
    Object* object;
    CI_FuncData* funcdata;
    short i;
    Boolean resultInMemory;
    ENode* argsExpr;

    object = expr->data.funccall.funcref->data.objref;
    if (object->datatype == DALIAS) {
        object = object->u.alias.object;
    }

    funcdata = object->u.ifuncdata;
    if (!funcdata) {
        return expr;
    }

    if (funcdata->can_inline < CI_CanInlineExpression) {
        if (funcdata->can_inline == CI_CanInlineStatement) {
            if (cinline_unconditionalpart && cinline_stmtlevelexprs < 16) {
                cinline_stmtlevelexpr[cinline_stmtlevelexprs++] = expr;
            }
            cinline_serialize_stmt = TRUE;
        }
        return expr;
    }

    resultInMemory = CMach_GetFunctionResultClass(TYPE_FUNC(object->type)) == 1;
    argsExpr = CInline_SetupArgsExpression(object, funcdata, expr->data.funccall.args);

    for (i = 0; i < funcdata->numstatements; i++) {
        switch (funcdata->statements[i].type) {
            case ST_RETURN:
                if (funcdata->statements[i].u.expr) {
                    ENode* copy =
                        CInline_CopyExpression(funcdata->statements[i].u.expr, CopyModeExpand);
                    if (resultInMemory) {
                        if (argsExpr) {
                            argsExpr = makecommaexpression(argsExpr, copy);
                        } else {
                            argsExpr = copy;
                        }

                        argsExpr = makecommaexpression(argsExpr, CInline_ReturnMemResult(object));
                    } else {
                        if (argsExpr) {
                            argsExpr = makecommaexpression(argsExpr, CInline_ReturnCheck(copy));
                        } else {
                            argsExpr = CInline_ReturnCheck(copy);
                        }
                    }
                }
                break;
            case ST_EXPRESSION:
                if (funcdata->statements[i].u.expr->type == ESCOPEBEGIN ||
                    funcdata->statements[i].u.expr->type == ESCOPEEND) {
                    break;
                }
                if (argsExpr) {
                    argsExpr = makecommaexpression(
                        argsExpr,
                        CInline_CopyExpression(funcdata->statements[i].u.expr, CopyModeExpand)
                    );
                } else {
                    argsExpr =
                        CInline_CopyExpression(funcdata->statements[i].u.expr, CopyModeExpand);
                }
                break;
            default:
                CError_FATAL(2066);
        }
    }

    if (!argsExpr) {
        argsExpr = nullnode();
    }
    if (!IS_TYPE_VOID(expr->rtype)) {
        argsExpr->rtype = expr->rtype;
    }

    inline_expanded = TRUE;
    return fn_00539A30(argsExpr, fn_0050DF30);
}

static SInt32 CInline_GetStatementNumber(Statement* first, Statement* stmt) {
    SInt32 index;

    index = 0;
    while (first) {
        if (first == stmt) {
            return index;
        }
        first = first->next;
        index++;
    }

    CError_FATAL(3986);
    return 0;
}

static SInt32 CInline_GetLocalID2(Object* object) {
    SInt32 i;

    if (object) {
        CError_ASSERT(418, object->datatype == DLOCAL);
        for (i = 0; i < cinline_localcount; i++) {
            if (cinline_localvars[i].obj == object->u.var.realObj) {
                cinline_localvars[i].used = TRUE;
                cinline_localvars[i].unmodified = FALSE;
                return i;
            }
        }
        CError_FATAL(429);
    }

    return 0;
}

static ExceptionAction* fn_005089C0(Statement* start, Statement* stmt) {
    ExceptionAction* exc;
    ExceptionAction* last;
    ExceptionAction* packexc;

    exc = stmt->dobjstack;
    last = NULL;

    while (exc) {
        packexc = galloc(sizeof(ExceptionAction));
        packexc->prev = last;
        last = packexc;

        packexc->type = exc->type;

        switch (exc->type) {
            case EAT_DESTROYLOCAL:
                packexc->data.destroy_local.local =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_local.local);
                packexc->data.destroy_local.dtor = exc->data.destroy_local.dtor;
                break;
            case EAT_DESTROYLOCALCOND:
                packexc->data.destroy_local_cond.local =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_local_cond.local);
                packexc->data.destroy_local_cond.dtor = exc->data.destroy_local_cond.dtor;
                packexc->data.destroy_local_cond.cond =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_local_cond.cond);
                break;
            case EAT_DESTROYLOCALOFFSET:
                packexc->data.destroy_local_offset.local =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_local_offset.local);
                packexc->data.destroy_local_offset.dtor = exc->data.destroy_local_offset.dtor;
                packexc->data.destroy_local_offset.offset = exc->data.destroy_local_offset.offset;
                break;
            case EAT_DESTROYLOCALPOINTER:
                packexc->data.destroy_local_pointer.pointer =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_local_pointer.pointer);
                packexc->data.destroy_local_pointer.dtor = exc->data.destroy_local_pointer.dtor;
                break;
            case EAT_DESTROYLOCALARRAY:
                packexc->data.destroy_local_array.localarray =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_local_array.localarray);
                packexc->data.destroy_local_array.dtor = exc->data.destroy_local_array.dtor;
                packexc->data.destroy_local_array.elements = exc->data.destroy_local_array.elements;
                packexc->data.destroy_local_array.element_size =
                    exc->data.destroy_local_array.element_size;
                break;
            case EAT_DESTROYPARTIALARRAY:
                packexc->data.destroy_partial_array.arraypointer =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_partial_array.arraypointer);
                packexc->data.destroy_partial_array.arraycounter =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_partial_array.arraycounter);
                packexc->data.destroy_partial_array.dtor =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_partial_array.dtor);
                packexc->data.destroy_partial_array.element_size =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_partial_array.element_size);
                break;
            case EAT_DESTROYMEMBER:
            case EAT_DESTROYBASE:
                packexc->data.destroy_member.objectptr =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_member.objectptr);
                packexc->data.destroy_member.dtor = exc->data.destroy_member.dtor;
                packexc->data.destroy_member.offset = exc->data.destroy_member.offset;
                break;
            case EAT_DESTROYMEMBERCOND:
                packexc->data.destroy_member_cond.objectptr =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_member_cond.objectptr);
                packexc->data.destroy_member_cond.cond =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_member_cond.cond);
                packexc->data.destroy_member_cond.dtor = exc->data.destroy_member_cond.dtor;
                packexc->data.destroy_member_cond.offset = exc->data.destroy_member_cond.offset;
                break;
            case EAT_DESTROYMEMBERARRAY:
                packexc->data.destroy_member_array.objectptr =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_member_array.objectptr);
                packexc->data.destroy_member_array.dtor = exc->data.destroy_member_array.dtor;
                packexc->data.destroy_member_array.offset = exc->data.destroy_member_array.offset;
                packexc->data.destroy_member_array.elements =
                    exc->data.destroy_member_array.elements;
                packexc->data.destroy_member_array.element_size =
                    exc->data.destroy_member_array.element_size;
                break;
            case EAT_DELETEPOINTER:
            case EAT_DELETELOCALPOINTER:
                packexc->data.delete_pointer.pointerobject =
                    (Object*)CInline_GetLocalID2(exc->data.delete_pointer.pointerobject);
                packexc->data.delete_pointer.deletefunc = exc->data.delete_pointer.deletefunc;
                break;
            case EAT_DELETEPOINTERCOND:
                packexc->data.delete_pointer_cond.pointerobject =
                    (Object*)CInline_GetLocalID2(exc->data.delete_pointer_cond.pointerobject);
                packexc->data.delete_pointer_cond.deletefunc =
                    exc->data.delete_pointer_cond.deletefunc;
                packexc->data.delete_pointer_cond.cond =
                    (Object*)CInline_GetLocalID2(exc->data.delete_pointer_cond.cond);
                break;
            case EAT_CATCHBLOCK:
                packexc->data.catch_block.catch_object =
                    (Object*)CInline_GetLocalID2(exc->data.catch_block.catch_object);
                packexc->data.catch_block.catch_info_object =
                    (Object*)CInline_GetLocalID2(exc->data.catch_block.catch_info_object);
                packexc->data.catch_block.catch_label = (CLabel*)CInline_GetStatementNumber(
                    start->next, exc->data.catch_block.catch_label->stmt
                );
                packexc->data.catch_block.catch_typeid = exc->data.catch_block.catch_typeid;
                packexc->data.catch_block.catch_type = exc->data.catch_block.catch_type;
                packexc->data.catch_block.catch_qual = exc->data.catch_block.catch_qual;
                break;
            case EAT_ACTIVECATCHBLOCK:
                packexc->data.active_catch_block.catch_info_object =
                    (Object*)CInline_GetLocalID2(exc->data.active_catch_block.catch_info_object);
                packexc->data.active_catch_block.call_dtor = exc->data.active_catch_block.call_dtor;
                break;
            case EAT_SPECIFICATION:
                packexc->data.specification.unexp_ids = exc->data.specification.unexp_ids;
                packexc->data.specification.unexp_id = exc->data.specification.unexp_id;
                packexc->data.specification.unexp_label = (CLabel*)CInline_GetStatementNumber(
                    start->next, exc->data.specification.unexp_label->stmt
                );
                packexc->data.specification.unexp_info_object =
                    (Object*)CInline_GetLocalID2(exc->data.specification.unexp_info_object);
                break;
            case EAT_DESTROYLOCALOBJECT:
                packexc->data.destroy_local_object.local =
                    (Object*)CInline_GetLocalID2(exc->data.destroy_local_object.local);
                packexc->data.destroy_local_object.call_dtor =
                    exc->data.destroy_local_object.call_dtor;
                break;
            case EAT_DELETEVLA:
                packexc->data.delete_vla.vlaobject = exc->data.delete_vla.vlaobject;
                packexc->data.delete_vla.deletefunc = exc->data.delete_vla.deletefunc;
                break;
            case EAT_TERMINATE:
                break;
            default:
                CError_FATAL(4222);
        }

        exc = exc->prev;
    }

    return last;
}

static CI_Switch* CInline_PackSwitch(Statement* start, Statement* stmt) {
    SwitchInfo* info;
    SwitchCase* swcase;
    SwitchRange* swrange;
    CI_Switch* packed;
    SInt32 i;
    SInt32 ncases;
    SInt32 total;

    info = (SwitchInfo*)stmt->label;
    ncases = 0;
    total = 0;
    for (swcase = info->cases; swcase; swcase = swcase->next) {
        ncases++;
        total++;
    }
    for (swrange = info->ranges; swrange; swrange = swrange->next) {
        total++;
    }

    packed = galloc(sizeof(CI_Switch) - sizeof(CI_SwitchCase) + total * sizeof(CI_SwitchCase));
    packed->numcases = ncases;
    packed->numranges = total - ncases;
    packed->expr = CInline_CopyExpression(stmt->expr, CopyModePack);
    packed->defaultlabelID = CInline_GetStatementNumber(start, info->defaultlabel->stmt);
    packed->exprtype = info->exprtype;

    i = 0;
    for (swcase = info->cases; swcase; swcase = swcase->next, i++) {
        packed->cases[i].labelID = CInline_GetStatementNumber(start, swcase->label->stmt);
        packed->cases[i].bounds = swcase->bounds;
    }

    for (swrange = info->ranges; swrange; swrange = swrange->next, i++) {
        packed->cases[i].labelID = CInline_GetStatementNumber(start, swrange->label->stmt);
        packed->cases[i].range.expr = swrange->expr;
        if (swrange->expr2) {
            packed->cases[i].range.expr2 = swrange->expr2;
        } else {
            packed->cases[i].range.expr2 = NULL;
        }
    }

    return packed;
}

static ENode* CInline_ExpandExpression(ENode* expr) {
    ENodeList* list;
    Boolean save;

    switch (expr->type) {
        case EPOSTINC:
        case EPOSTDEC:
        case EPREINC:
        case EPREDEC:
        case EINDIRECT:
        case EMONMIN:
        case EBINNOT:
        case ELOGNOT:
        case ETYPCON:
        case EBITFIELD:
            expr->data.monadic = CInline_ExpandExpression(expr->data.monadic);
            break;

        case EFORCELOAD:
            expr->data.monadic = CInline_ExpandExpression(expr->data.monadic);
            if (expr->data.monadic->type == EFORCELOAD) {
                expr->data.monadic = expr->data.monadic->data.monadic;
            }
            break;

        case EMUL:
        case EDIV:
        case EMODULO:
        case EADD:
        case ESUB:
        case ESHL:
        case ESHR:
        case ELESS:
        case EGREATER:
        case ELESSEQU:
        case EGREATEREQU:
        case EEQU:
        case ENOTEQU:
        case EAND:
        case EXOR:
        case EOR:
        case EASS:
        case EMULASS:
        case EDIVASS:
        case EMODASS:
        case EADDASS:
        case ESUBASS:
        case ESHLASS:
        case ESHRASS:
        case EANDASS:
        case EXORASS:
        case EORASS:
        case EMIN:
        case EMAX:
        case EPMODULO:
        case EROTL:
        case EROTR:
            expr->data.diadic.left = CInline_ExpandExpression(expr->data.diadic.left);
            expr->data.diadic.right = CInline_ExpandExpression(expr->data.diadic.right);
            break;

        case ELAND:
        case ELOR:
        case ECOMMA:
            expr->data.diadic.left = CInline_ExpandExpression(expr->data.diadic.left);
            save = cinline_unconditionalpart;
            cinline_unconditionalpart = FALSE;
            expr->data.diadic.right = CInline_ExpandExpression(expr->data.diadic.right);
            cinline_unconditionalpart = save;
            break;

        case EFUNCCALL:
        case EFUNCCALLP:
            expr->data.funccall.funcref = CInline_ExpandExpression(expr->data.funccall.funcref);
            for (list = expr->data.funccall.args; list; list = list->next) {
                list->node = CInline_ExpandExpression(list->node);
            }

            if (expr->data.funccall.funcref->type == EOBJREF &&
                CInline_InlineFunctionCheck(expr->data.funccall.funcref)) {
                expr = CInline_InlineFunctionExpression(expr);
            }
            break;

        case ENULLCHECK:
            expr->data.nullcheck.nullcheckexpr =
                CInline_ExpandExpression(expr->data.nullcheck.nullcheckexpr);
            save = cinline_unconditionalpart;
            cinline_unconditionalpart = FALSE;
            expr->data.nullcheck.condexpr = CInline_ExpandExpression(expr->data.nullcheck.condexpr);
            cinline_unconditionalpart = save;
            break;

        case EMFPOINTER:
            expr->data.mfpointer.accessnode =
                CInline_ExpandExpression(expr->data.mfpointer.accessnode);
            expr->data.mfpointer.mfpointer =
                CInline_ExpandExpression(expr->data.mfpointer.mfpointer);
            break;

        case ECOND:
            expr->data.cond.cond = CInline_ExpandExpression(expr->data.cond.cond);
            save = cinline_unconditionalpart;
            cinline_unconditionalpart = FALSE;
            expr->data.cond.expr1 = CInline_ExpandExpression(expr->data.cond.expr1);
            expr->data.cond.expr2 = CInline_ExpandExpression(expr->data.cond.expr2);
            cinline_unconditionalpart = save;
            break;

        case EOBJACCESS:
            if (expr->data.objaccess.unk19 && expr->data.objaccess.expr) {
                expr = CInline_ExpandExpression(expr->data.templdep.expr14);
            } else {
                expr = nullnode();
            }
            break;

        case EINTCONST:
        case EFLOATCONST:
        case ESTRINGCONST:
        case EOBJREF:
        case EPRECOMP:
        case ELABEL:
        case ESCOPEBEGIN:
        case ESCOPEEND:
        case EINSTRUCTION:
            break;

        default:
            CError_FATAL(2725);
    }

    return expr;
}

static void CInline_SerializeStatement(Statement* stmt);

static Statement* CInline_ExpandStatement(Statement* stmt) {
    ENode* expr;
    Boolean changed;

    do {
        changed = FALSE;

        if (stmt->type == ST_EXPRESSION &&
            stmt->expr->type == EINDIRECT &&
            !CParser_IsVolatile(stmt->expr->rtype, stmt->expr->flags & 3)) {
            stmt->expr = stmt->expr->data.monadic;
            changed = TRUE;
            if (stmt->expr->type == EOBJREF || stmt->expr->type == EBITFIELD) {
                stmt->expr = nullnode();
            }
        }

        if (stmt->expr->type == ECOMMA) {
            Statement* newStmt = lalloc(sizeof(Statement));
            *newStmt = *stmt;

            stmt->next = newStmt;
            stmt->type = ST_EXPRESSION;
            stmt->expr = stmt->expr->data.diadic.left;
            newStmt->expr = newStmt->expr->data.diadic.right;

            changed = TRUE;
        }
    } while (changed);

    if ((expr = stmt->expr)->type == EFUNCCALL || expr->type == EFUNCCALLP) {
        if (expr->data.funccall.funcref->type == EOBJREF &&
            CInline_InlineFunctionCheck(expr->data.funccall.funcref)) {
            stmt = CInline_SeqStringRewrite(stmt, &changed);
            if (changed) {
                any_inline_expanded = TRUE;
                return stmt;
            }
        }
    }

    inline_expanded = FALSE;
    cinline_unconditionalpart = TRUE;
    cinline_serialize_stmt = FALSE;
    cinline_stmtlevelexprs = 0;
    stmt->expr = CInline_ExpandExpression(stmt->expr);

    if (cinline_serialize_stmt) {
        cinline_unconditionalpart = TRUE;
        cinline_serialize_stmt = FALSE;
        cinline_stmtlevelexprs = 0;
        CInline_SerializeStatement(stmt);
        stmt->expr = CInline_ExpandExpression(stmt->expr);
    }

    if (inline_expanded) {
        stmt->expr = fn_00539A30(stmt->expr, fn_0050DF30);
        any_inline_expanded = TRUE;
    }

    if (cinline_stmtlevelexprs) {
        stmt = CInline_ExtractInlineFunction(stmt);
        any_inline_expanded = TRUE;
    }

    return stmt;
}

static void CInline_SetObjectSFlags(Object* object, UInt8 sflags) {
    if (sflags & CI_SFLAGS_Referenced) {
        object->flags |= OBJECT_REFERENCED;
        sflags &= ~CI_SFLAGS_Referenced;
    }

    switch (sflags) {
        case CI_SFLAGS_NoClass:
            object->sclass = 0;
            break;
        case CI_SFLAGS_Register:
            object->sclass = TK_REGISTER;
            break;
        case CI_SFLAGS_Auto:
            object->sclass = TK_AUTO;
            break;
        default:
            CError_FATAL(1229);
    }
}

static Object* CInline_NewLocalObject(Type* type, SInt16 qual, UInt8 sflags, SInt32 unused) {
    Object* object = CParser_NewLocalDataObject(NULL, TRUE);
    object->name = CParser_GetUniqueName();
    object->type = type;
    object->qual = qual;
    CInline_SetObjectSFlags(object, sflags);
    CFunc_SetupLocalVarInfo(object);
    return object;
}

static ExceptionAction* CInline_UnpackActions(CI_Statement* packstmt, Boolean inlineCopy);
static void CInline_UnpackSwitch(Statement* stmt, CI_Statement* packstmt, CLabel** labels);

static ExceptionAction* CInline_UnpackActions(CI_Statement* packstmt, Boolean inlineCopy);
static void CInline_UnpackSwitch(Statement* stmt, CI_Statement* packstmt, CLabel** labels);

static Statement* CInline_ExpandStatements(
    Object* funcobj,
    Statement* stmt,
    CI_FuncData* funcdata,
    ENode* funccall,
    CLabel* label,
    Object* resultobj,
    Boolean appendOriginal
) {
    CLabel** labels;
    CI_Statement* packstmt;
    short i;
    CI_StmtLink* stmtLinks;
    CI_StmtLink* link;
    ENode* setupArgs;
    Boolean resultInMemory;
    Statement origStmt;

    origStmt = *stmt;
    resultInMemory = CMach_GetFunctionResultClass(TYPE_FUNC(funcobj->type)) == 1;

    if ((setupArgs =
             CInline_SetupArgsExpression(funcobj, funcdata, funccall->data.funccall.args)) ==
        NULL) {
        stmt->type = ST_NOP;
    } else {
        stmt->type = ST_EXPRESSION;
        stmt->expr = fn_00539A30(setupArgs, fn_0050DF30);
    }

    stmtLinks = NULL;
    cinline_label_trans = NULL;

    labels = lalloc(sizeof(CLabel*) * funcdata->numstatements);
    memclrw(labels, sizeof(CLabel*) * funcdata->numstatements);

    for (i = 0, packstmt = funcdata->statements; i < funcdata->numstatements; i++, packstmt++) {
        stmt->next = lalloc(sizeof(Statement));
        stmt = stmt->next;
        *stmt = origStmt;

        stmt->type = packstmt->type;
        stmt->flags = packstmt->flags;
        stmt->value += packstmt->value;

        if (packstmt->dobjstack) {
            ExceptionAction* unpacked = CInline_UnpackActions(packstmt, FALSE);
            if (stmt->dobjstack) {
                ExceptionAction* scan = unpacked;
                while (scan->prev) {
                    scan = scan->prev;
                }
                scan->prev = stmt->dobjstack;
            }
            stmt->dobjstack = unpacked;
        }

        switch (stmt->type) {
            case ST_NOP:
                break;

            case ST_EXPRESSION:
            case ST_BEGINCATCH:
            case ST_ENDCATCH:
            case ST_ENDCATCHDTOR:
            case ST_GOTOEXPR:
                setupArgs = CInline_CopyExpression(packstmt->u.expr, CopyModeExpand);
                stmt->expr = fn_00539A30(setupArgs, fn_0050DF30);
                break;

            case ST_RETURN:
                if (packstmt->u.expr) {
                    setupArgs = CInline_CopyExpression(packstmt->u.expr, CopyModeExpand);
                    stmt->expr = fn_00539A30(setupArgs, fn_0050DF30);
                    if (resultInMemory) {
                        stmt->expr =
                            makecommaexpression(stmt->expr, CInline_ReturnMemResult(funcobj));
                    }

                    if (resultobj) {
                        stmt->type = ST_EXPRESSION;
                        stmt->expr =
                            makediadicnode(create_objectnode2(resultobj), stmt->expr, EASS);
                    } else {
                        stmt->type = origStmt.type;
                        if (stmt->type == ST_EXPRESSION &&
                            !CInline_ExpressionHasSideEffect(stmt->expr)) {
                            stmt->type = ST_NOP;
                        }
                    }

                    if (label) {
                        stmt->next = lalloc(sizeof(Statement));
                        stmt = stmt->next;
                        *stmt = origStmt;
                        stmt->type = ST_GOTO;
                        stmt->label = label;
                    }
                } else {
                    if (label) {
                        stmt->type = ST_GOTO;
                        stmt->label = label;
                    } else {
                        stmt->type = ST_NOP;
                    }
                }
                break;

            case ST_LABEL:
                labels[i] = stmt->label = newlabel();
                stmt->label->stmt = stmt;
                break;

            case ST_IFGOTO:
            case ST_IFNGOTO:
                setupArgs = CInline_CopyExpression(packstmt->u.ifgoto.expr, CopyModeExpand);
                stmt->expr = fn_00539A30(setupArgs, fn_0050DF30);
            case ST_GOTO:
                link = lalloc(sizeof(CI_StmtLink));
                link->next = stmtLinks;
                stmtLinks = link;

                link->stmt = stmt;
                link->ciStmt = packstmt;
                break;

            case ST_SWITCH:
                setupArgs = CInline_CopyExpression(packstmt->u.switchdata->expr, CopyModeExpand);
                stmt->expr = fn_00539A30(setupArgs, fn_0050DF30);
            case ST_ASM:
                link = lalloc(sizeof(CI_StmtLink));
                link->next = stmtLinks;
                stmtLinks = link;

                link->stmt = stmt;
                link->ciStmt = packstmt;
                break;

            default:
                CError_FATAL(3535);
        }
    }

    if (label) {
        stmt->next = lalloc(sizeof(Statement));
        stmt = stmt->next;
        *stmt = origStmt;

        stmt->type = ST_LABEL;
        stmt->label = label;
        label->stmt = stmt;

        if (appendOriginal) {
            stmt->next = lalloc(sizeof(Statement));
            stmt = stmt->next;
            *stmt = origStmt;
        }
    }

    while (stmtLinks) {
        Statement* linkstmt = stmtLinks->stmt;
        packstmt = stmtLinks->ciStmt;

        switch (linkstmt->type) {
            case ST_GOTO:
                CError_ASSERT(3555, linkstmt->label = labels[packstmt->u.statementnum]);
                break;
            case ST_IFGOTO:
            case ST_IFNGOTO:
                CError_ASSERT(3560, linkstmt->label = labels[packstmt->u.ifgoto.statementnum]);
                break;
            case ST_SWITCH:
                CInline_UnpackSwitch(linkstmt, packstmt, labels);
                break;
            case ST_ASM:
                InlineAsm_UnpackAsmStatement(
                    linkstmt, labels, FALSE, packstmt->u.asmdata.data, packstmt->u.asmdata.size
                );
                break;
            default:
                CError_FATAL(3571);
        }

        stmtLinks = stmtLinks->next;
    }

    while (cinline_label_trans) {
        CError_ASSERT(3578, *cinline_label_trans->labelptr = labels[cinline_label_trans->id]);
        cinline_label_trans = cinline_label_trans->next;
    }

    return stmt;
}

static Statement* CInline_ExtractInlineFunction(Statement* stmt) {
    TypeFunc* tfunc;
    CI_FuncData* funcdata;
    Object* funcObject;
    Object* resultObject;
    short i;
    ENode* expr;

    for (i = 0; i < cinline_stmtlevelexprs; i++) {
        expr = cinline_stmtlevelexpr[i];

        funcObject = expr->data.funccall.funcref->data.objref;
        if (funcObject->datatype == DALIAS) {
            funcObject = funcObject->u.alias.object;
        }

        if ((funcdata = funcObject->u.ifuncdata)) {
            tfunc = TYPE_FUNC(funcObject->type);
            CError_ASSERT(3636, IS_TYPE_FUNC(tfunc));

            if (!IS_TYPE_VOID(tfunc->functype)) {
                if (CMach_GetFunctionResultClass(TYPE_FUNC(funcObject->type)) == 1) {
                    resultObject =
                        CInline_NewLocalObject(CDecl_NewPointerType(tfunc->functype), 0, 0, 0);
                } else {
                    resultObject = CInline_NewLocalObject(tfunc->functype, 0, 0, 0);
                }
            } else {
                resultObject = NULL;
            }

            stmt = CInline_ExpandStatements(
                funcObject, stmt, funcdata, expr, newlabel(), resultObject, TRUE
            );

            {
                ENode* node;

                if (resultObject) {
                    node = create_objectnode2(resultObject);
                } else {
                    node = nullnode();
                }

                *expr = *node;
            }
        }
    }

    return stmt;
}

Statement* CInline_Expand(Object* funcObject, Statement* stmt) {
    Statement* scan;

    funcObject = funcObject;
    fn_00539780(stmt, fn_00509A50);
    if (!copts.dontinline && copts.inlinelevel >= 0) {
        if (copts.inline_bottom_up) {
            inline_max_size = copts.inlinemaxsize;
            while (inline_max_size > 1 &&
                   EstimateExpandedSizeOfFunction(stmt, 0) > copts.inlinemaxtotalsize) {
                inline_max_size >>= 1;
            }
        }

        cinline_level = 0;
        while (1) {
            any_inline_expanded = FALSE;
            for (scan = stmt; scan; scan = scan->next) {
                switch (scan->type) {
                    case ST_NOP:
                    case ST_LABEL:
                    case ST_GOTO:
                    case ST_BEGINCATCH:
                    case ST_ENDCATCH:
                    case ST_ENDCATCHDTOR:
                    case ST_ASM:
                        break;
                    case ST_RETURN:
                        if (!scan->expr) {
                            break;
                        }
                    case ST_EXPRESSION:
                    case ST_SWITCH:
                    case ST_IFGOTO:
                    case ST_IFNGOTO:
                        scan = CInline_ExpandStatement(scan);
                        break;
                    default:
                        CError_FATAL(3911);
                }
            }

            if (!any_inline_expanded && !copts.inline_bottom_up) {
                break;
            }

            if (!copts.alwaysinline || copts.inline_bottom_up) {
                if (copts.inlinelevel == 0) {
                    if (copts.inline_bottom_up) {
                        if ((cinline_level + 1) >= 8) {
                            break;
                        }
                    } else {
                        if (cinline_level >= 3) {
                            break;
                        }
                    }
                } else {
                    if ((cinline_level + 1) >= copts.inlinelevel) {
                        break;
                    }
                }
            }

            if (CWDisplayLines(cparamblkptr->context, lines) != cwNoErr) {
                fn_0044F9E0();
            }

            cinline_level++;
        }
    }

    fn_00539780(stmt, fn_00509AF0);
    for (scan = stmt; scan; scan = scan->next) {
        if (scan->dobjstack) {
            CExcept_CheckStackRefs(scan->dobjstack);
        }
    }

    return stmt;
}

static Statement* CInline_SeqStringRewrite(Statement* stmt, Boolean* changed) {
    CI_FuncData* funcdata;
    CLabel* label;
    Object* object;

    *changed = FALSE;

    object = stmt->expr->data.funccall.funcref->data.objref;
    if (object->datatype == DALIAS) {
        object = object->u.alias.object;
    }

    funcdata = object->u.ifuncdata;
    if (!funcdata || funcdata->can_inline < CI_CanInlineStatement) {
        return stmt;
    }

    if (stmt->type != ST_EXPRESSION) {
        short i;
        for (i = 0; i < (funcdata->numstatements - 1); i++) {
            if (funcdata->statements[i].type == ST_RETURN) {
                return stmt;
            }
        }

        if (funcdata->statements[funcdata->numstatements - 1].type != ST_RETURN) {
            return stmt;
        }

        label = NULL;
    } else {
        label = newlabel();
    }

    *changed = TRUE;
    return CInline_ExpandStatements(object, stmt, funcdata, stmt->expr, label, NULL, FALSE);
}

static Boolean CInline_InlineFunctionCheck(ENode* expr) {
    TypeFunc* tfunc;
    Object* object;
    SInt32 level;

    object = expr->data.objref;
    if (object->datatype == DALIAS) {
        object = object->u.alias.object;
    }

    tfunc = TYPE_FUNC(object->type);

    if (IS_TYPE_FUNC(tfunc) &&
        ((object->qual & Q_INLINE) || (tfunc->flags & FUNC_AUTO_INLINE)) &&
        (object->datatype == DFUNC ||
         (object->datatype == DVFUNC && (expr->flags & ENODE_FLAG_NONVIRTUAL)))) {
        if (copts.alwaysinline) {
            return TRUE;
        }

        if (copts.inline_bottom_up) {
            CI_FuncData* funcdata;

            if (!(funcdata = object->u.ifuncdata)) {
                return FALSE;
            }

            if (copts.inlinelevel == 0) {
                level = 7 - cinline_level;
            } else {
                level = copts.inlinelevel - cinline_level - 1;
            }

            if (copts.inline_bottom_up_once && (object->qual & Q_INLINE) && level == 0) {
                return TRUE;
            }

            if (CInline_EstimateSizeOfFunc(funcdata, 0, level) > inline_max_size) {
                return FALSE;
            }
        } else if (cinline_level > 0 && copts.inlinelevel == 0) {
            CI_FuncData* funcdata;

            if (!(funcdata = object->u.ifuncdata)) {
                return FALSE;
            }

            if (funcdata->numstatements > 10) {
                return FALSE;
            }
            if (cinline_level > 1 && funcdata->numstatements > 7) {
                return FALSE;
            }
            if (cinline_level > 2 && funcdata->numstatements > 3) {
                return FALSE;
            }
        }

        return TRUE;
    }

    return FALSE;
}

void fn_00509A50(ENode* expr) {
    switch (expr->type) {
        case EOBJREF:
            if (expr->data.objref->datatype == DALIAS) {
                CExpr_AliasTransform(expr);
            }
            break;
        case EOBJACCESS:
            if (expr->data.objaccess.unk19 && expr->data.objaccess.expr) {
                ENode* source = expr->data.templdep.expr14;
                SInt32 i;
                for (i = 0; i < 0x40; i += 0x10) {
                    *(UInt32*)((UInt8*)expr + i) = *(UInt32*)((UInt8*)source + i);
                    *(UInt32*)((UInt8*)expr + i + 4) = *(UInt32*)((UInt8*)source + i + 4);
                    *(UInt32*)((UInt8*)expr + i + 8) = *(UInt8*)((UInt8*)source + i + 8);
                    *(UInt32*)((UInt8*)expr + i + 0xC) = *(UInt32*)((UInt8*)source + i + 0xC);
                }
            }
            break;
        case EMFPOINTER:
            expr->type = EINTCONST;
            expr->flags = 0;
            expr->rtype = TYPE(&stsignedlong);
            expr->data.intval = cint64_zero;
            break;
        default:
            break;
    }
}

void fn_00509AF0(ENode* node) {
    TypeClass* tclass;
    SInt32 index;

    switch (node->type) {
        case 0x38:
        case 0x39:
            if (copts.warn_notinlined &&
                !copts.dontinline &&
                node->data.monadic->type == 0x3A &&
                (node->data.monadic->data.objref->qual & 0x10) &&
                node->data.monadic->data.objref->datatype != DINLINEFUNC) {
                if (!CParser_IsVirtualFunction(node->data.monadic->data.objref, &tclass, &index)) {
                    CError_Warning(0xF3, node->data.monadic->data.objref);
                }
            }
            break;
        case 0x3A:
            CInline_ObjectAddrRef(node->data.objref);
            break;
        case 0x3D:
            if (node->data.label->stmt) {
                node->data.label->stmt->flags |= 1;
            }
            break;
        default:
            break;
    }
}

static void CInline_UnpackSwitch(Statement* stmt, CI_Statement* packstmt, CLabel** labels) {
    SwitchInfo* info;
    SwitchCase* swcase;
    SwitchRange* swrange;
    short i;

    info = lalloc(sizeof(SwitchInfo));
    info->cases = NULL;
    info->ranges = NULL;
    stmt->label = (CLabel*)info;
    CError_ASSERT(3224, info->defaultlabel = labels[packstmt->u.switchdata->defaultlabelID]);
    info->exprtype = packstmt->u.switchdata->exprtype;

    for (i = 0; i < packstmt->u.switchdata->numcases; i++) {
        if (!info->cases) {
            swcase = lalloc(sizeof(SwitchCase));
            info->cases = swcase;
        } else {
            swcase->next = lalloc(sizeof(SwitchCase));
            swcase = swcase->next;
        }

        swcase->next = NULL;
        swcase->bounds = packstmt->u.switchdata->cases[i].bounds;
        CError_ASSERT(3236, swcase->label = labels[packstmt->u.switchdata->cases[i].labelID]);
    }

    for (; i < (packstmt->u.switchdata->numcases + packstmt->u.switchdata->numranges); i++) {
        if (!info->ranges) {
            swrange = lalloc(sizeof(SwitchRange));
            info->ranges = swrange;
        } else {
            swrange->next = lalloc(sizeof(SwitchRange));
            swrange = swrange->next;
        }

        swrange->next = NULL;
        swrange->expr =
            CInline_CopyExpression(packstmt->u.switchdata->cases[i].range.expr, CopyModeExpand);
        if (packstmt->u.switchdata->cases[i].range.expr2) {
            swrange->expr2 = CInline_CopyExpression(
                packstmt->u.switchdata->cases[i].range.expr2, CopyModeExpand
            );
        } else {
            swrange->expr2 = NULL;
        }
        CError_ASSERT(3251, swrange->label = labels[packstmt->u.switchdata->cases[i].labelID]);
    }
}
static ExceptionAction* CInline_UnpackActions(CI_Statement* packstmt, Boolean inlineCopy) {
    ExceptionAction* packexc;
    ExceptionAction* last;
    ExceptionAction* exc;

    packexc = packstmt->dobjstack;
    last = NULL;

    while (packexc) {
        exc = galloc(sizeof(ExceptionAction));
        exc->prev = last;
        last = exc;

        exc->type = packexc->type;

        switch (packexc->type) {
            case EAT_DESTROYLOCAL:
                exc->data.destroy_local.local =
                    CInline_GetLocalObj(packexc->data.destroy_local.local, inlineCopy);
                exc->data.destroy_local.dtor = packexc->data.destroy_local.dtor;
                break;
            case EAT_DESTROYLOCALCOND:
                exc->data.destroy_local_cond.local =
                    CInline_GetLocalObj(packexc->data.destroy_local_cond.local, inlineCopy);
                exc->data.destroy_local_cond.dtor = packexc->data.destroy_local_cond.dtor;
                exc->data.destroy_local_cond.cond =
                    CInline_GetLocalObj(packexc->data.destroy_local_cond.cond, inlineCopy);
                break;
            case EAT_DESTROYLOCALOFFSET:
                exc->data.destroy_local_offset.local =
                    CInline_GetLocalObj(packexc->data.destroy_local_offset.local, inlineCopy);
                exc->data.destroy_local_offset.dtor = packexc->data.destroy_local_offset.dtor;
                exc->data.destroy_local_offset.offset = packexc->data.destroy_local_offset.offset;
                break;
            case EAT_DESTROYLOCALPOINTER:
                exc->data.destroy_local_pointer.pointer =
                    CInline_GetLocalObj(packexc->data.destroy_local_pointer.pointer, inlineCopy);
                exc->data.destroy_local_pointer.dtor = packexc->data.destroy_local_pointer.dtor;
                break;
            case EAT_DESTROYLOCALARRAY:
                exc->data.destroy_local_array.localarray =
                    CInline_GetLocalObj(packexc->data.destroy_local_array.localarray, inlineCopy);
                exc->data.destroy_local_array.dtor = packexc->data.destroy_local_array.dtor;
                exc->data.destroy_local_array.elements = packexc->data.destroy_local_array.elements;
                exc->data.destroy_local_array.element_size =
                    packexc->data.destroy_local_array.element_size;
                break;
            case EAT_DESTROYPARTIALARRAY:
                exc->data.destroy_partial_array.arraypointer = CInline_GetLocalObj(
                    packexc->data.destroy_partial_array.arraypointer, inlineCopy
                );
                exc->data.destroy_partial_array.arraycounter = CInline_GetLocalObj(
                    packexc->data.destroy_partial_array.arraycounter, inlineCopy
                );
                exc->data.destroy_partial_array.dtor =
                    CInline_GetLocalObj(packexc->data.destroy_partial_array.dtor, inlineCopy);
                exc->data.destroy_partial_array.element_size = CInline_GetLocalObj(
                    packexc->data.destroy_partial_array.element_size, inlineCopy
                );
                break;
            case EAT_DESTROYMEMBER:
            case EAT_DESTROYBASE:
                exc->data.destroy_member.objectptr =
                    CInline_GetLocalObj(packexc->data.destroy_member.objectptr, inlineCopy);
                exc->data.destroy_member.dtor = packexc->data.destroy_member.dtor;
                exc->data.destroy_member.offset = packexc->data.destroy_member.offset;
                break;
            case EAT_DESTROYMEMBERCOND:
                exc->data.destroy_member_cond.objectptr =
                    CInline_GetLocalObj(packexc->data.destroy_member_cond.objectptr, inlineCopy);
                exc->data.destroy_member_cond.cond =
                    CInline_GetLocalObj(packexc->data.destroy_member_cond.cond, inlineCopy);
                exc->data.destroy_member_cond.dtor = packexc->data.destroy_member_cond.dtor;
                exc->data.destroy_member_cond.offset = packexc->data.destroy_member_cond.offset;
                break;
            case EAT_DESTROYMEMBERARRAY:
                exc->data.destroy_member_array.objectptr =
                    CInline_GetLocalObj(packexc->data.destroy_member_array.objectptr, inlineCopy);
                exc->data.destroy_member_array.dtor = packexc->data.destroy_member_array.dtor;
                exc->data.destroy_member_array.offset = packexc->data.destroy_member_array.offset;
                exc->data.destroy_member_array.elements =
                    packexc->data.destroy_member_array.elements;
                exc->data.destroy_member_array.element_size =
                    packexc->data.destroy_member_array.element_size;
                break;
            case EAT_DELETEPOINTER:
            case EAT_DELETELOCALPOINTER:
                exc->data.delete_pointer.pointerobject =
                    CInline_GetLocalObj(packexc->data.delete_pointer.pointerobject, inlineCopy);
                exc->data.delete_pointer.deletefunc = packexc->data.delete_pointer.deletefunc;
                break;
            case EAT_DELETEPOINTERCOND:
                exc->data.delete_pointer_cond.pointerobject = CInline_GetLocalObj(
                    packexc->data.delete_pointer_cond.pointerobject, inlineCopy
                );
                exc->data.delete_pointer_cond.deletefunc =
                    packexc->data.delete_pointer_cond.deletefunc;
                exc->data.delete_pointer_cond.cond =
                    CInline_GetLocalObj(packexc->data.delete_pointer_cond.cond, inlineCopy);
                break;
            case EAT_CATCHBLOCK: {
                LabelTrans* trans;
                exc->data.catch_block.catch_object =
                    CInline_GetLocalObj(packexc->data.catch_block.catch_object, inlineCopy);
                exc->data.catch_block.catch_info_object =
                    CInline_GetLocalObj(packexc->data.catch_block.catch_info_object, inlineCopy);

                trans = lalloc(sizeof(LabelTrans));
                trans->next = cinline_label_trans;
                cinline_label_trans = trans;

                trans->id = (SInt32)packexc->data.catch_block.catch_label;
                trans->labelptr = &exc->data.catch_block.catch_label;

                exc->data.catch_block.catch_typeid = packexc->data.catch_block.catch_typeid;
                exc->data.catch_block.catch_type = packexc->data.catch_block.catch_type;
                exc->data.catch_block.catch_qual = packexc->data.catch_block.catch_qual;
                break;
            }
            case EAT_ACTIVECATCHBLOCK:
                exc->data.active_catch_block.catch_info_object = CInline_GetLocalObj(
                    packexc->data.active_catch_block.catch_info_object, inlineCopy
                );
                exc->data.active_catch_block.call_dtor = packexc->data.active_catch_block.call_dtor;
                break;
            case EAT_SPECIFICATION: {
                LabelTrans* trans;
                exc->data.specification.unexp_ids = packexc->data.specification.unexp_ids;
                exc->data.specification.unexp_id = packexc->data.specification.unexp_id;

                trans = lalloc(sizeof(LabelTrans));
                trans->next = cinline_label_trans;
                cinline_label_trans = trans;

                trans->id = (SInt32)packexc->data.specification.unexp_label;
                trans->labelptr = &exc->data.specification.unexp_label;

                exc->data.specification.unexp_info_object =
                    CInline_GetLocalObj(packexc->data.specification.unexp_info_object, inlineCopy);
                break;
            }
            case EAT_DESTROYLOCALOBJECT:
                exc->data.destroy_local_object.local =
                    CInline_GetLocalObj(packexc->data.destroy_local_object.local, inlineCopy);
                exc->data.destroy_local_object.call_dtor =
                    packexc->data.destroy_local_object.call_dtor;
                break;
            case EAT_DELETEVLA:
                exc->data.delete_vla.vlaobject = packexc->data.delete_vla.vlaobject;
                break;
            case EAT_TERMINATE:
                break;
            default:
                CError_FATAL(3399);
        }

        packexc = packexc->prev;
    }

    return last;
}

void CInline_UnpackIFunctionData(Object* object, CI_FuncData* funcdata, Statement* firstStmt) {
    Object* obj;
    CLabel** labels;
    ObjectList* last;
    Statement* stmt;
    CI_Statement* packstmt;
    CI_Var* var;
    SInt32 i;

    cparser_fileoffset = funcdata->fileoffset;
    symdecloffset_0063627C = funcdata->symdecloffset;
    functionbodyoffset = funcdata->functionbodyoffset;
    functionbodypath = funcdata->functionbodypath;
    cg_codegen_active_line_636A04 = funcdata->symdeclend;

    ig_reglist_a = NULL;
    for (i = 0, var = funcdata->arguments; i < funcdata->numarguments; i++, var++) {
        if (i == 0) {
            last = lalloc(sizeof(ObjectList));
            ig_reglist_a = last;
        } else {
            last->next = lalloc(sizeof(ObjectList));
            last = last->next;
        }

        obj = galloc(sizeof(Object));
        memclrw(obj, sizeof(Object));
        last->object = obj;
        last->next = NULL;

        obj->otype = OT_OBJECT;
        obj->access = 0;
        obj->datatype = DLOCAL;
        obj->name = var->name;
        obj->type = var->type;
        obj->qual = var->qual;
        obj->u.unk2C = (SInt32)var->obj;
        if (var->sflags & CI_SFLAGS_Referenced) {
            obj->flags |= OBJECT_REFERENCED;
        }
        switch (var->sflags & 3) {
            case CI_SFLAGS_NoClass:
                obj->sclass = 0;
                break;
            case CI_SFLAGS_Register:
                obj->sclass = TK_REGISTER;
                break;
            case CI_SFLAGS_Auto:
                obj->sclass = TK_AUTO;
                break;
            default:
                CError_FATAL(1699);
        }
        CFunc_SetupLocalVarInfo(obj);
        obj->u.var.info->unknown2D = var->unk12;
        if (funcdata->fileoffset.file) {
            obj->u.var.info->deftoken.tokenfile = funcdata->fileoffset.file;
            obj->u.var.info->deftoken.tokenoffset = funcdata->fileoffset.tokenoffset;
        }
    }

    ig_reglist_b = NULL;
    for (; i < funcdata->numlocals; i++, var++) {
        if (!ig_reglist_b) {
            last = lalloc(sizeof(ObjectList));
            ig_reglist_b = last;
        } else {
            last->next = lalloc(sizeof(ObjectList));
            last = last->next;
        }

        obj = galloc(sizeof(Object));
        memclrw(obj, sizeof(Object));
        last->object = obj;
        last->next = NULL;

        obj->otype = OT_OBJECT;
        obj->access = 0;
        obj->datatype = DLOCAL;
        obj->name = var->name;
        obj->type = var->type;
        obj->qual = var->qual;
        obj->u.unk2C = (SInt32)var->obj;
        if (var->sflags & CI_SFLAGS_Referenced) {
            obj->flags |= OBJECT_REFERENCED;
        }
        switch (var->sflags & 3) {
            case CI_SFLAGS_NoClass:
                obj->sclass = 0;
                break;
            case CI_SFLAGS_Register:
                obj->sclass = TK_REGISTER;
                break;
            case CI_SFLAGS_Auto:
                obj->sclass = TK_AUTO;
                break;
            default:
                CError_FATAL(1699);
        }
        CFunc_SetupLocalVarInfo(obj);
        obj->u.var.info->unknown2D = var->unk12;
        if (funcdata->fileoffset.file) {
            obj->u.var.info->deftoken.tokenfile = funcdata->fileoffset.file;
            obj->u.var.info->deftoken.tokenoffset = funcdata->fileoffset.tokenoffset;
        }
    }

    enode_idtrans = NULL;
    cinline_label_trans = NULL;
    labels = lalloc(sizeof(CLabel*) * funcdata->numstatements);
    memclrw(labels, sizeof(CLabel*) * funcdata->numstatements);

    for (i = 0, stmt = firstStmt, packstmt = funcdata->statements; i < funcdata->numstatements;
         i++, packstmt++) {
        stmt->next = lalloc(sizeof(Statement));
        stmt = stmt->next;

        stmt->type = packstmt->type;
        stmt->value = packstmt->value;
        stmt->flags = packstmt->flags;
        stmt->sourceoffset = packstmt->sourceoffset;
        stmt->sourcefilepath = packstmt->sourcefilepath;
        stmt->dobjstack = CInline_UnpackActions(packstmt, TRUE);
        stmt->next = NULL;

        switch (stmt->type) {
            case ST_NOP:
            case ST_GOTO:
            case ST_ASM:
                break;
            case ST_EXPRESSION:
            case ST_BEGINCATCH:
            case ST_ENDCATCH:
            case ST_ENDCATCHDTOR:
            case ST_GOTOEXPR:
                stmt->expr = CInline_CopyExpression(packstmt->u.expr, CopyModeUnpack);
                break;
            case ST_RETURN:
                if (packstmt->u.expr) {
                    stmt->expr = CInline_CopyExpression(packstmt->u.expr, CopyModeUnpack);
                } else {
                    stmt->expr = NULL;
                }
                break;
            case ST_LABEL:
                labels[i] = stmt->label = newlabel();
                stmt->label->stmt = stmt;
                break;
            case ST_IFGOTO:
            case ST_IFNGOTO:
                stmt->expr = CInline_CopyExpression(packstmt->u.ifgoto.expr, CopyModeUnpack);
                break;
            case ST_SWITCH:
                stmt->expr = CInline_CopyExpression(packstmt->u.switchdata->expr, CopyModeUnpack);
                break;
            default:
                CError_FATAL(4542);
        }
    }

    for (stmt = firstStmt->next, packstmt = funcdata->statements; stmt;
         stmt = stmt->next, packstmt++) {
        switch (stmt->type) {
            case ST_GOTO:
                CError_ASSERT(4549, stmt->label = labels[packstmt->u.statementnum]);
                break;
            case ST_IFGOTO:
            case ST_IFNGOTO:
                CError_ASSERT(4554, stmt->label = labels[packstmt->u.ifgoto.statementnum]);
                break;
            case ST_SWITCH:
                CInline_UnpackSwitch(stmt, packstmt, labels);
                break;
            case ST_ASM:
                InlineAsm_UnpackAsmStatement(
                    stmt, labels, TRUE, packstmt->u.asmdata.data, packstmt->u.asmdata.size
                );
                break;
            default:
                break;
        }
    }

    cinline_first_stmt = firstStmt->next;

    while (cinline_label_trans) {
        CError_ASSERT(4570, *cinline_label_trans->labelptr = labels[cinline_label_trans->id]);
        cinline_label_trans = cinline_label_trans->next;
    }
}

void CInline_GenIFunctionCode(Object* object, CI_FuncData* func, UInt8 oldStyleParameters) {
    Boolean saveDebugInfo;
    Statement firstStmt;
    CScopeSave saveScope;
    Statement* stmt;

    if (cparamblkptr->precompile == 1) {
        return;
    }
    if (!func) {
        return;
    }

    fn_0046D7F0();
    CScope_SetFunctionScope(object, &saveScope);
    CFunc_FuncGenSetup(&firstStmt, object);
    CInline_UnpackIFunctionData(object, func, &firstStmt);

    saveDebugInfo = copts.unknown134;
    if (copts.nosyminline || (!symdecloffset_0063627C && !cg_codegen_active_line_636A04)) {
        copts.unknown134 = FALSE;
    }

    expanding_function = object;
    recursive_inline = FALSE;
    stmt = CInline_Expand(object, &firstStmt);

    if (!anyerrors) {
        if (copts.unknown134) {
            fn_00443380(&cparser_fileoffset);
        }
        fn_00434540(stmt, object, oldStyleParameters, FALSE);
    }

    CScope_RestoreScope(&saveScope);
    copts.unknown134 = saveDebugInfo;
}

static SInt32 CInline_GetStatementNumber(Statement* first, Statement* stmt);

static void fn_00508540(CI_FuncData* funcdata, Statement* stmt, Object* object) {
    ObjectList* list;
    CI_Var* var;
    Statement* scan;
    CI_Statement* packstmt;
    SInt32 i;
    SInt32 n;

    cinline_first_stmt = stmt->next;
    memclrw(funcdata, sizeof(CI_FuncData));
    CMach_GetBackEndOptions(&funcdata->statements);

    funcdata->can_inline = CInline_CallGraphInlining(object, stmt->next);

    if (copts.unknown134) {
        funcdata->fileoffset = cparser_fileoffset;
        funcdata->fileoffset.is_inline = TRUE;
        funcdata->symdecloffset = symdecloffset_0063627C;
        funcdata->functionbodyoffset = functionbodyoffset;
        funcdata->functionbodypath = functionbodypath;
        funcdata->symdeclend = cg_codegen_active_line_636A04;
    }

    list = ig_reglist_a;
    i = 0;
    while (list) {
        i++;
        list = list->next;
    }

    funcdata->numarguments = i;
    list = ig_reglist_b;
    while (list) {
        if (list->object->datatype == DLOCAL) {
            i++;
        }
        list = list->next;
    }

    funcdata->numlocals = i;
    cinline_localcount = funcdata->numlocals;

    if (i > 0) {
        var = funcdata->arguments = galloc(sizeof(CI_Var) * i);
        memclrw(funcdata->arguments, sizeof(CI_Var) * i);

        for (list = ig_reglist_a; list; list = list->next, var++) {
            var->name = list->object->name;
            var->type = list->object->type;
            var->qual = list->object->qual;
            var->obj = (Object*)list->object->u.unk2C;
            switch (list->object->sclass) {
                case 0:
                    var->sflags = 0;
                    break;
                case TK_REGISTER:
                    var->sflags = 1;
                    break;
                case TK_AUTO:
                    var->sflags = 2;
                    break;
                default:
                    CError_FATAL(1678);
            }
            if (list->object->flags & OBJECT_REFERENCED) {
                var->sflags |= CI_SFLAGS_Referenced;
            }
            var->sflags |= 0x40;
            var->used = 0;
            var->unmodified = 1;
        }

        for (list = ig_reglist_b; list; list = list->next) {
            if (list->object->datatype == DLOCAL) {
                var->name = list->object->name;
                var->type = list->object->type;
                var->qual = list->object->qual;
                var->obj = (Object*)list->object->u.unk2C;
                switch (list->object->sclass) {
                    case 0:
                        var->sflags = 0;
                        break;
                    case TK_REGISTER:
                        var->sflags = 1;
                        break;
                    case TK_AUTO:
                        var->sflags = 2;
                        break;
                    default:
                        CError_FATAL(1678);
                }
                if (list->object->flags & OBJECT_REFERENCED) {
                    var->sflags |= CI_SFLAGS_Referenced;
                }
                var->used = 0;
                var->unk12 = list->object->u.var.info->unknown2D;
                var->unmodified = 0;
                var++;
            }
        }
    }

    scan = stmt->next;
    i = 0;
    while (scan) {
        i++;
        scan = scan->next;
    }

    funcdata->numstatements = i;
    funcdata->statements = galloc(sizeof(CI_Statement) * i);

    for (scan = stmt->next, packstmt = funcdata->statements; scan; scan = scan->next, packstmt++) {
        packstmt->type = scan->type;
        packstmt->flags = scan->flags;
        packstmt->value = scan->value;
        packstmt->dobjstack = fn_005089C0(stmt, scan);
        packstmt->sourceoffset = scan->sourceoffset;
        packstmt->sourcefilepath = scan->sourcefilepath;

        switch (scan->type) {
            case ST_NOP:
            case ST_LABEL:
                break;
            case ST_EXPRESSION:
            case ST_BEGINCATCH:
            case ST_ENDCATCH:
            case ST_ENDCATCHDTOR:
            case ST_GOTOEXPR:
                packstmt->u.expr = CInline_CopyExpression(scan->expr, CopyModePack);
                break;
            case ST_RETURN:
                if (scan->expr) {
                    packstmt->u.expr = CInline_CopyExpression(scan->expr, CopyModePack);
                } else {
                    packstmt->u.expr = NULL;
                }
                break;
            case ST_GOTO:
                packstmt->u.statementnum =
                    CInline_GetStatementNumber(stmt->next, scan->label->stmt);
                break;
            case ST_IFGOTO:
            case ST_IFNGOTO:
                packstmt->u.ifgoto.expr = CInline_CopyExpression(scan->expr, CopyModePack);
                packstmt->u.ifgoto.statementnum =
                    CInline_GetStatementNumber(stmt->next, scan->label->stmt);
                break;
            case ST_SWITCH:
                packstmt->u.switchdata = CInline_PackSwitch(stmt->next, scan);
                break;
            case ST_ASM:
                InlineAsm_PackAsmStatement(
                    scan, stmt->next, &packstmt->u.asmdata.data, &packstmt->u.asmdata.size
                );
                break;
            default:
                CError_FATAL(3862);
        }
    }
}
static Boolean cinline_gendeps;
static CI_Export* cinline_exportlist;

static void CInline_AddFRefList_Statement(Statement* stmt) {
    while (stmt) {
        if (stmt->dobjstack) {
            CInline_AddFRefList_ExAction(stmt->dobjstack);
        }

        switch (stmt->type) {
            case ST_NOP:
            case ST_LABEL:
            case ST_GOTO:
            case ST_BEGINCATCH:
            case ST_ENDCATCH:
            case ST_ENDCATCHDTOR:
            case ST_ASM:
                break;
            case ST_RETURN:
                if (!stmt->expr) {
                    break;
                }
            case ST_EXPRESSION:
            case ST_SWITCH:
            case ST_IFGOTO:
            case ST_IFNGOTO:
            case ST_GOTOEXPR:
                CExpr_SearchExprTree(stmt->expr, CInline_AddFRefList_ExprCB);
                break;
            default:
                CError_FATAL(4902);
        }

        stmt = stmt->next;
    }
}

void CInline_CheckInlineability(Statement* stmt, Object* object, UInt8 oldStyleParameters) {
    CI_FuncData* funcdata;
    CI_Export* export;
    Boolean isAutoInline;
    Boolean isInline;

    TYPE_FUNC(object->type)->flags |= FUNC_DEFINED;

    isInline = isAutoInline = FALSE;
    if (!(object->qual & Q_INLINE)) {
        if (copts.auto_inline &&
            !copts.dontinline &&
            CInline_CallGraphInlining(object, stmt->next) &&
            CInline_IsSmallFunction(object, stmt->next)) {
            isInline = isAutoInline = TRUE;
            TYPE_FUNC(object->type)->flags |= FUNC_AUTO_INLINE;
        }
    } else {
        isInline = TRUE;
    }

    if (isInline) {
        COpt_SimpleOptimizer(object, stmt);

        funcdata = galloc(sizeof(CI_FuncData));
        fn_00508540(funcdata, stmt, object);

        object->u.ifuncdata = funcdata;

        if (!isAutoInline && !(object->flags & OBJECT_REFERENCED)) {
            if (cinline_gendeps) {
                cinline_freflist = NULL;
                CInline_AddFRefList_Statement(stmt);
                fn_00507790(cinline_freflist);
            }
            return;
        }
    }

    object->flags |= OBJECT_DEFINED;

    if (cparamblkptr->precompile == 1) {
        CError_Error(ERR_ILLEGAL_USE_PRECOMPILED_HEADER);
        return;
    }

    cinline_freflist = NULL;
    CInline_AddFRefList_Statement(stmt);

    if (fn_00507790(cinline_freflist) || copts.defer_codegen) {
        if (!isInline) {
            funcdata = galloc(sizeof(CI_FuncData));
            fn_00508540(funcdata, stmt, object);
        } else {
            funcdata = object->u.ifuncdata;
        }

        export = galloc(sizeof(CI_Export));
        export->object = object;
        export->funcdata = funcdata;
        export->oldStyleParameters = oldStyleParameters;

        export->next = cinline_exportlist;
        cinline_exportlist = export;
    } else {
        expanding_function = object;
        recursive_inline = FALSE;
        stmt = CInline_Expand(object, stmt);

        if (copts.unknown134) {
            fn_00443380(&cparser_fileoffset);
        }

        if (!anyerrors) {
            fn_00434540(stmt, object, oldStyleParameters, FALSE);
        }
    }
}

void CInline_ObjectAddrRef(Object* object) {
    CI_FuncData* funcdata;
    UInt32 qual;
    UInt32 flags;
    CI_Export* export;
    CI_Action* scan;
    CI_Action* action;
    CI_Action** prev;

    object->flags |= OBJECT_REFERENCED;

    switch (object->datatype) {
        case DFUNC:
        case DVFUNC:
            qual = object->qual;
            if (qual & Q_INLINE) {
                funcdata = object->u.ifuncdata;
                if (funcdata &&
                    !(object->flags & OBJECT_DEFINED) &&
                    !(TYPE_FUNC(object->type)->flags & FUNC_IS_TEMPL)) {
                    export = galloc(sizeof(CI_Export));
                    export->object = object;
                    export->funcdata = funcdata;
                    export->oldStyleParameters = 0;
                    export->next = cinline_exportlist;
                    cinline_exportlist = export;
                    object->flags |= OBJECT_DEFINED;
                    return;
                }
            }

            if ((qual & Q_IS_TEMPLATED) && !OBJECT_TEMPL(object)->unk48) {
                OBJECT_TEMPL(object)->unk48 = fn_004BADA0();
            }

            flags = TYPE_FUNC(object->type)->flags;
            if ((flags & FUNC_AUTO_GENERATED) && !(flags & 2)) {
                for (scan = cinline_actionlist; scan; scan = scan->next) {
                    if (scan->obj == object) {
                        goto ensured;
                    }
                }
                action = galloc(sizeof(CI_Action));
                memclrw(action, sizeof(CI_Action));
                action->actiontype = CI_ActionDefaultFunc;
                action->obj = object;
                action->next = cinline_actionlist;
                cinline_actionlist = action;
ensured:
                return;
            } else if (flags & FUNC_TEMPLATE_ACTION) {
                prev = &cinline_tactionlist;
                for (action = cinline_tactionlist; action; action = action->next) {
                    if (action->obj == object) {
                        break;
                    }
                    prev = &action->next;
                }
                if (!action) {
                    CError_FATAL(5034);
                }
                *prev = action->next;
                action->next = cinline_actionlist;
                cinline_actionlist = action;
                TYPE_FUNC(object->type)->flags &= ~FUNC_TEMPLATE_ACTION;
            }
            return;

        case DALIAS:
            CInline_ObjectAddrRef(object->u.alias.object);
            return;

        case DDATA:
            if (object->qual & 0x10000) {
                CInit_ExportConst(object);
            }

            if (object->flags & 8) {
                object->flags &= ~8;
                fn_004483B0(object);
            }
            return;

        default:
            return;
    }
}

Boolean fn_00507790(ObjectList* list) {
    Object* obj;
    UInt32 flags;
    CI_Action* scan;
    CI_Action* action;
    CI_Action** prev;
    Boolean result;

    result = FALSE;
    for (; list; list = list->next) {
        obj = list->object;
        flags = TYPE_FUNC(obj->type)->flags;
        if ((flags & FUNC_AUTO_GENERATED) && !(flags & 2)) {
            for (scan = cinline_actionlist; scan; scan = scan->next) {
                if (scan->obj == obj) {
                    goto ensured;
                }
            }
            action = galloc(sizeof(CI_Action));
            memclrw(action, sizeof(CI_Action));
            action->actiontype = CI_ActionDefaultFunc;
            action->obj = obj;
            action->next = cinline_actionlist;
            cinline_actionlist = action;
ensured:
            result = TRUE;
        } else {
            if ((obj->qual & Q_IS_TEMPLATED) && fn_00519660(obj)) {
                result = TRUE;
            } else {
                for (scan = cinline_actionlist; scan; scan = scan->next) {
                    if (scan->obj == obj) {
                        result = TRUE;
                        break;
                    }
                }
                if (TYPE_FUNC(obj->type)->flags & FUNC_TEMPLATE_ACTION) {
                    prev = &cinline_tactionlist;
                    for (action = cinline_tactionlist; action; action = action->next) {
                        if (action->obj == obj) {
                            break;
                        }
                        prev = &action->next;
                    }
                    if (!action) {
                        CError_FATAL(5034);
                    }
                    *prev = action->next;
                    action->next = cinline_actionlist;
                    cinline_actionlist = action;
                    TYPE_FUNC(obj->type)->flags &= ~FUNC_TEMPLATE_ACTION;
                    result = TRUE;
                }
            }
        }
    }

    return result;
}

void CInline_GenerateDefaultFunc(Object* object) {
    TypeClass* tclass;

    CError_ASSERT(5421, TYPE_FUNC(object->type)->flags & FUNC_AUTO_GENERATED);
    CError_ASSERT(5422, TYPE_FUNC(object->type)->flags & FUNC_METHOD);

    tclass = TYPE_METHOD(object->type)->theclass;

    if (object == CClass_DefaultConstructor(tclass)) {
        if (object->u.unk2C) {
            CABI_MakeDefaultArgConstructor(tclass, object);
        } else {
            CABI_MakeDefaultConstructor(tclass, object);
        }
        return;
    }
    if (object == CClass_HasCopyConstructor(tclass)) {
        CABI_MakeDefaultCopyConstructor(tclass, object);
        return;
    }
    if (object == CClass_AssignmentOperator(tclass)) {
        CABI_MakeDefaultAssignmentOperator(tclass, object);
        return;
    }
    if (object == CClass_Destructor(tclass)) {
        CABI_MakeDefaultDestructor(tclass, object);
        return;
    }
    CError_FATAL(5456);
}

Boolean CInline_IsSmallFunction(Object* object, Statement* stmt) {
    ObjectList* list;
    SInt32 statementCount;
    SInt32 localSize;

    statementCount = 0;
    while (stmt) {
        if (stmt->type != ST_NOP &&
            stmt->type != ST_LABEL &&
            (stmt->type != ST_EXPRESSION ||
             (stmt->expr->type != ESCOPEBEGIN && stmt->expr->type != ESCOPEEND))) {
            statementCount++;
        }
        if (statementCount > copts.unknown5A) {
            return FALSE;
        }
        stmt = stmt->next;
    }

    for (list = ig_reglist_b, localSize = 0; list; list = list->next) {
        localSize += list->object->type->size;
    }

    if (localSize > 0x400) {
        return FALSE;
    }

    return TRUE;
}

void fn_00507D70(
    Object* funcobj, TemplClass* templ, TemplClassInst* inst, void* tmemb, Boolean useInlineCache
) {
    CI_Action* scan;
    CI_Action* head;
    CI_Action* action;

    if (useInlineCache) {
        head = cinline_tactionlist;
    } else {
        head = cinline_actionlist;
    }
    scan = head;
    while (scan) {
        if (scan->obj == funcobj) {
            return;
        }
        scan = scan->next;
    }

    action = galloc(sizeof(CI_Action));
    memclrw(action, sizeof(CI_Action));
    action->actiontype = CI_ActionMemberFunc;
    action->obj = funcobj;
    action->unk08 = fn_004BADA0();
    action->u.memberfunc.templ = templ;
    action->u.memberfunc.inst = inst;
    action->u.memberfunc.tmemb = tmemb;
    if (useInlineCache) {
        action->next = cinline_tactionlist;
        cinline_tactionlist = action;
        TYPE_FUNC(funcobj->type)->flags |= FUNC_TEMPLATE_ACTION;
    } else {
        action->next = cinline_actionlist;
        cinline_actionlist = action;
    }
}

void fn_00507D00(Object* funcobj, void* func, TemplFuncInstance* inst) {
    CI_Action* scan;
    CI_Action* action;

    for (scan = cinline_tactionlist; scan; scan = scan->next) {
        if (scan->obj == funcobj) {
            return;
        }
    }

    action = galloc(sizeof(CI_Action));
    memclrw(action, sizeof(CI_Action));
    action->actiontype = CI_ActionTemplateFunc;
    action->obj = funcobj;
    action->u.templatefunc.func = func;
    action->u.templatefunc.inst = inst;
    action->next = cinline_tactionlist;
    cinline_tactionlist = action;
    TYPE_FUNC(funcobj->type)->flags |= FUNC_TEMPLATE_ACTION;
}

void* fn_00509850(void* node, void* target) {
    SInt32 index;

    index = 0;
    while (node) {
        if (node == target) {
            return (void*)index;
        }
        node = *(void**)node;
        index++;
    }

    CError_FATAL(3986);
    return (void*)0;
}

ENode* CInline_SerializeExpr(ENode* expr);

static ENode* fn_0050B380(ENode* expr);
static ENode* fn_0050B590(ENode* expr);
static ENode* fn_0050B170(ENode* expr);
static ENode* fn_0050AE60(ENode* expr);

static ENode* CInline_LoadToTemp(ENode* expr, Object** objectptr) {
    Object* object;

    if (!(object = *objectptr)) {
        switch (expr->rtype->type) {
            case TYPEVOID:
                return expr;
            case TYPEINT:
            case TYPEFLOAT:
            case TYPEENUM:
            case TYPESTRUCT:
            case TYPECLASS:
            case TYPEMEMBERPOINTER:
            case TYPEPOINTER:
                object = create_temp_object(expr->rtype);
                *objectptr = object;
                break;
            default:
                CError_FATAL(2778);
        }
    }

    return makediadicnode(create_objectnode(object), expr, EASS);
}

static ENode* fn_0050B380(ENode* expr) {
    ENode* n;
    Statement* stmt;
    CLabel* label;
    Object* temp = NULL;

    label = newlabel();

    n = CInline_SerializeExpr(expr->data.diadic.left);
    n = makemonadicnode(n, ELOGNOT);
    n->rtype = expr->rtype;
    n = makemonadicnode(n, ELOGNOT);
    n = CInline_LoadToTemp(n, &temp);
    stmt = CInline_NewStatement(ST_IFNGOTO);
    stmt->expr = n;
    stmt->label = label;

    n = CInline_SerializeExpr(expr->data.diadic.right);
    n = makemonadicnode(n, ELOGNOT);
    n->rtype = expr->rtype;
    n = makemonadicnode(n, ELOGNOT);
    n = CInline_LoadToTemp(n, &temp);
    stmt = CInline_NewStatement(ST_EXPRESSION);
    stmt->expr = n;

    stmt = CInline_NewStatement(ST_LABEL);
    stmt->label = label;
    label->stmt = stmt;

    return create_objectnode(temp);
}

static ENode* fn_0050B590(ENode* expr) {
    ENode* n;
    Statement* stmt;
    CLabel* label;
    Object* temp = NULL;

    label = newlabel();

    n = CInline_SerializeExpr(expr->data.diadic.left);
    n = makemonadicnode(n, ELOGNOT);
    n->rtype = expr->rtype;
    n = makemonadicnode(n, ELOGNOT);
    n = CInline_LoadToTemp(n, &temp);
    stmt = CInline_NewStatement(ST_IFGOTO);
    stmt->expr = n;
    stmt->label = label;

    n = CInline_SerializeExpr(expr->data.diadic.right);
    n = makemonadicnode(n, ELOGNOT);
    n->rtype = expr->rtype;
    n = makemonadicnode(n, ELOGNOT);
    n = CInline_LoadToTemp(n, &temp);
    stmt = CInline_NewStatement(ST_EXPRESSION);
    stmt->expr = n;

    stmt = CInline_NewStatement(ST_LABEL);
    stmt->label = label;
    label->stmt = stmt;

    return create_objectnode(temp);
}

static ENode* fn_0050B170(ENode* expr) {
    Statement* stmt;
    CLabel* label;
    ENode* n;
    Object* temp = NULL;
    UIDTemp uidtemp;

    label = newlabel();

    n = CInline_SerializeExpr(expr->data.nullcheck.nullcheckexpr);
    stmt = CInline_NewStatement(ST_IFNGOTO);
    stmt->expr = CInline_LoadToTemp(n, &temp);
    stmt->label = label;

    uidtemp.next = cinline_uid_temps;
    uidtemp.object = temp;
    uidtemp.uid = expr->data.nullcheck.precompid;
    cinline_uid_temps = &uidtemp;

    n = CInline_SerializeExpr(expr->data.nullcheck.condexpr);
    stmt = CInline_NewStatement(ST_EXPRESSION);
    stmt->expr = CInline_LoadToTemp(n, &temp);

    cinline_uid_temps = uidtemp.next;

    stmt = CInline_NewStatement(ST_LABEL);
    stmt->label = label;
    label->stmt = stmt;

    return create_objectnode(temp);
}

static ENode* fn_0050AE60(ENode* expr) {
    ENode* n;
    Statement* stmt;
    CLabel* label1;
    CLabel* label2;
    Object* temp = NULL;

    label1 = newlabel();
    label2 = newlabel();

    n = CInline_SerializeExpr(expr->data.cond.cond);
    stmt = CInline_NewStatement(ST_IFNGOTO);
    stmt->expr = n;
    stmt->label = label1;

    n = CInline_SerializeExpr(expr->data.cond.expr1);
    n = CInline_LoadToTemp(n, &temp);
    stmt = CInline_NewStatement(ST_EXPRESSION);
    stmt->expr = n;

    stmt = CInline_NewStatement(ST_GOTO);
    stmt->label = label2;

    stmt = CInline_NewStatement(ST_LABEL);
    stmt->label = label1;
    label1->stmt = stmt;

    n = CInline_SerializeExpr(expr->data.cond.expr2);
    n = CInline_LoadToTemp(n, &temp);
    stmt = CInline_NewStatement(ST_EXPRESSION);
    stmt->expr = n;

    stmt = CInline_NewStatement(ST_LABEL);
    stmt->label = label2;
    label2->stmt = stmt;

    if (!temp) {
        n = nullnode();
        n->rtype = &stvoid;
        return n;
    }

    return create_objectnode(temp);
}

void fn_0050DF30(ENode* expr) {
    ENode* left;
    ENode* right;
    CInt64 val;

    switch (expr->type) {
        case EMONMIN:
        case EBINNOT:
        case ELOGNOT: {
            ENode* inner = expr->data.monadic;
            switch (inner->type) {
                case EINTCONST:
                    if (expr->type != ELOGNOT) {
                        inner->data.intval = CMach_CalcIntMonadic(
                            expr->rtype, CParser_GetOperator(expr->type), inner->data.intval
                        );
                    } else {
                        inner->data.intval = CInt64_Not(inner->data.intval);
                    }
                    break;
                case EFLOATCONST:
                    if (expr->type == ELOGNOT) {
                        inner->type = EINTCONST;
                        CInt64_SetLong(
                            &inner->data.intval, CMach_FloatIsZero(inner->data.floatval.value)
                        );
                    } else {
                        inner->data.floatval = CMach_CalcFloatMonadic(
                            expr->rtype, CParser_GetOperator(expr->type), inner->data.floatval
                        );
                    }
                    break;
                default:
                    return;
            }
            inner->rtype = expr->rtype;
            expr = inner;
            break;
        }

        case ETYPCON: {
            ENode* inner = expr->data.monadic;
            switch (inner->type) {
                case EINTCONST:
                    switch (expr->rtype->type) {
                        case TYPEFLOAT:
                            expr->type = EFLOATCONST;
                            expr->data.floatval =
                                CMach_CalcFloatConvertFromInt(inner->rtype, inner->data.intval);
                            return;
                        case TYPEINT:
                            break;
                        default:
                            return;
                    }
                    break;
                case EFLOATCONST:
                    switch (expr->rtype->type) {
                        case TYPEFLOAT:
                            expr->type = EFLOATCONST;
                            expr->data.floatval =
                                CMach_CalcFloatConvert(expr->rtype, inner->data.floatval);
                            return;
                        case TYPEINT:
                            expr->type = EINTCONST;
                            expr->data.intval =
                                CMach_CalcIntConvertFromFloat(expr->rtype, inner->data.floatval);
                            return;
                        default:
                            return;
                    }
                    break;
                default:
                    return;
            }
            break;
        }

        case EPOSTINC:
        case EPOSTDEC: {
            ENode* inner = expr->data.monadic;
            switch (inner->type) {
                case EINTCONST:
                case EFLOATCONST:
                    inner->rtype = expr->rtype;
                    expr = inner;
                    break;
                default:
                    return;
            }
            break;
        }

        case EMUL:
        case EADD:
        case ESUB:
        case ESHL:
        case ESHR:
        case EAND:
        case EXOR:
        case EOR:
        case EMIN:
        case EMAX:
        case EROTL:
        case EROTR:
            left = expr->data.diadic.left;
            right = expr->data.diadic.right;
            if (left->type == right->type) {
                switch (left->type) {
                    case EINTCONST:
                        left->data.intval = CMach_CalcIntDiadic(
                            expr->rtype,
                            left->data.intval,
                            CParser_GetOperator(expr->type),
                            right->data.intval
                        );
                        left->rtype = expr->rtype;
                        expr = left;
                        break;
                    case EFLOATCONST:
                        left->data.floatval = CMach_CalcFloatDiadic(
                            expr->rtype,
                            left->data.floatval,
                            CParser_GetOperator(expr->type),
                            right->data.floatval
                        );
                        left->rtype = expr->rtype;
                        expr = left;
                        break;
                    default:
                        break;
                }
            }
            break;

        case EDIV:
        case EMODULO:
            left = expr->data.diadic.left;
            right = expr->data.diadic.right;
            if (left->type == right->type) {
                switch (left->type) {
                    case EINTCONST:
                        if (CInt64_IsZero(&right->data.intval)) {
                            return;
                        }
                        left->data.intval = CMach_CalcIntDiadic(
                            expr->rtype,
                            left->data.intval,
                            CParser_GetOperator(expr->type),
                            right->data.intval
                        );
                        left->rtype = expr->rtype;
                        expr = left;
                        break;
                    case EFLOATCONST:
                        left->data.floatval = CMach_CalcFloatDiadic(
                            expr->rtype,
                            left->data.floatval,
                            CParser_GetOperator(expr->type),
                            right->data.floatval
                        );
                        left->rtype = expr->rtype;
                        expr = left;
                        break;
                    default:
                        break;
                }
            }
            break;

        case ELESS:
        case EGREATER:
        case ELESSEQU:
        case EGREATEREQU:
        case EEQU:
        case ENOTEQU:
            left = expr->data.diadic.left;
            right = expr->data.diadic.right;
            if (left->type == right->type) {
                switch (left->type) {
                    case EINTCONST:
                        left->data.intval = CMach_CalcIntDiadic(
                            left->rtype,
                            left->data.intval,
                            CParser_GetOperator(expr->type),
                            right->data.intval
                        );
                        left->rtype = expr->rtype;
                        expr = left;
                        break;
                    case EFLOATCONST:
                        CInt64_SetLong(
                            &left->data.intval,
                            CMach_CalcFloatDiadicBool(
                                left->rtype,
                                left->data.floatval,
                                CParser_GetOperator(expr->type),
                                right->data.floatval
                            )
                        );
                        left->type = EINTCONST;
                        left->rtype = CParser_GetBoolType();
                        expr = left;
                        break;
                    default:
                        break;
                }
            }
            break;

        case ELAND:
            if (iszero(expr->data.diadic.left)) {
                expr = expr->data.diadic.left;
                break;
            }
            if (fn_005390E0(expr->data.diadic.left)) {
                expr = makemonadicnode(expr->data.diadic.right, ELOGNOT);
                expr->rtype = expr->rtype;
                expr = makemonadicnode(expr, ELOGNOT);
                break;
            }
            break;

        case ELOR:
            if (fn_005390E0(expr->data.diadic.left)) {
                expr = expr->data.diadic.left;
                break;
            }
            if (iszero(expr->data.diadic.left)) {
                expr = makemonadicnode(expr->data.diadic.right, ELOGNOT);
                expr->rtype = expr->rtype;
                expr = makemonadicnode(expr, ELOGNOT);
                break;
            }
            break;

        case ECOND:
            if (fn_005390E0(expr->data.cond.cond)) {
                expr = expr->data.cond.expr1;
                break;
            }
            if (iszero(expr->data.cond.cond)) {
                expr = expr->data.cond.expr2;
                break;
            }
            break;

        default:
            break;
    }
}

ENode* CInline_SerializeExpr(ENode* expr) {
    ENodeList* list;
    Statement* stmt;
    Object* temp;
    UIDTemp* uidtemp;
    ENode* n;

    switch (expr->type) {
        case EPOSTINC:
        case EPOSTDEC:
        case EPREINC:
        case EPREDEC:
        case EINDIRECT:
        case EMONMIN:
        case EBINNOT:
        case ELOGNOT:
        case ETYPCON:
        case EBITFIELD:
            expr->data.monadic = CInline_SerializeExpr(expr->data.monadic);
            return expr;

        case EMUL:
        case EMULV:
        case EDIV:
        case EMODULO:
        case EADDV:
        case ESUBV:
        case EADD:
        case ESUB:
        case ESHL:
        case ESHR:
        case ELESS:
        case EGREATER:
        case ELESSEQU:
        case EGREATEREQU:
        case EEQU:
        case ENOTEQU:
        case EAND:
        case EXOR:
        case EOR:
        case EASS:
        case EMULASS:
        case EDIVASS:
        case EMODASS:
        case EADDASS:
        case ESUBASS:
        case ESHLASS:
        case ESHRASS:
        case EANDASS:
        case EXORASS:
        case EORASS:
        case ECOMMA:
        case EMIN:
        case EMAX:
        case EPMODULO:
        case EROTL:
        case EROTR:
            expr->data.diadic.left = CInline_SerializeExpr(expr->data.diadic.left);
            expr->data.diadic.right = CInline_SerializeExpr(expr->data.diadic.right);
            return expr;

        case ELAND:
            return fn_0050B380(expr);

        case ELOR:
            return fn_0050B590(expr);

        case ECOND:
            return fn_0050AE60(expr);

        case ENULLCHECK:
            return fn_0050B170(expr);

        case EOBJACCESS:
            CError_FATAL(3171);

        default:
            CError_FATAL(3174);

        case EFORCELOAD:
            while (expr->data.monadic->type == EFORCELOAD) {
                expr->data.monadic = expr->data.monadic->data.monadic;
            }

            expr->data.monadic = CInline_SerializeExpr(expr->data.monadic);
            stmt = CInline_NewStatement(ST_EXPRESSION);
            temp = NULL;
            n = expr->data.monadic;
            if (!temp) {
                switch (n->rtype->type) {
                    case TYPEVOID:
                        break;
                    case TYPEINT:
                    case TYPEFLOAT:
                    case TYPEENUM:
                    case TYPESTRUCT:
                    case TYPECLASS:
                    case TYPEMEMBERPOINTER:
                    case TYPEPOINTER:
                        temp = create_temp_object(n->rtype);
                        break;
                    default:
                        CError_FATAL(2778);
                }
            }
            if (temp) {
                n = makediadicnode(create_objectnode(temp), n, EASS);
            }
            stmt->expr = n;
            return create_objectnode(temp);

            expr->data.diadic.left = CInline_SerializeExpr(expr->data.diadic.left);
            stmt = CInline_NewStatement(ST_EXPRESSION);
            stmt->expr = expr->data.diadic.left;
            return CInline_SerializeExpr(expr->data.diadic.right);

        case EPRECOMP:
            uidtemp = cinline_uid_temps;
            while (1) {
                if (!uidtemp) {
                    CError_FATAL(2939);
                }
                if (uidtemp->uid == expr->data.precompid) {
                    return create_objectnode(uidtemp->object);
                }
                uidtemp = uidtemp->next;
            }

        case EFUNCCALL:
        case EFUNCCALLP:
            expr->data.funccall.funcref = CInline_SerializeExpr(expr->data.funccall.funcref);
            for (list = expr->data.funccall.args; list; list = list->next) {
                list->node = CInline_SerializeExpr(list->node);
            }
            return expr;

        case EMFPOINTER:
            expr->data.mfpointer.accessnode =
                CInline_SerializeExpr(expr->data.mfpointer.accessnode);
            expr->data.mfpointer.mfpointer = CInline_SerializeExpr(expr->data.mfpointer.mfpointer);
            return expr;

        case EINITTRYCATCH:
            expr->data.itc.initexpr = CInline_SerializeExpr(expr->data.itc.initexpr);
            expr->data.itc.tryexpr = CInline_SerializeExpr(expr->data.itc.tryexpr);
            expr->data.itc.catchexpr = CInline_SerializeExpr(expr->data.itc.catchexpr);
            expr->data.itc.result = CInline_SerializeExpr(expr->data.itc.result);
            return expr;

        case EINTCONST:
        case EFLOATCONST:
        case ESTRINGCONST:
        case EOBJREF:
        case ELABEL:
        case EINSTRUCTION:
            return expr;
    }
}

void CInline_SerializeStatement(Statement* stmt) {
    Statement* scan;
    Statement* copy;

    cinline_serial_stmt = NULL;
    cinline_uid_temps = NULL;
    stmt->expr = CInline_SerializeExpr(stmt->expr);

    if (cinline_serial_stmt) {
        for (scan = cinline_serial_stmt; scan; scan = scan->next) {
            scan->value = stmt->value;
            scan->dobjstack = stmt->dobjstack;
            scan->sourceoffset = stmt->sourceoffset;
            scan->sourcefilepath = stmt->sourcefilepath;
        }

        copy = CInline_NewStatement(ST_EXPRESSION);
        *copy = *stmt;

        *stmt = *cinline_serial_stmt;
    }
}

Boolean CInline_CanFreeLHeap(void) {
    CI_Action* action;

    if (!anyerrors) {
        for (action = cinline_actionlist; action; action = action->next) {
            if (action->actiontype == CI_ActionInlineFunc) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

InitExpr* CInline_InitTemplateData(InitExpr* init) {
    Object* obj;
    Statement* stmt;
    CLabel* label;
    Object* guard;

    obj = init->object;
    CMangler_GetLinkName(obj);
    guard = CABI_NewGuardVariable(obj);
    stmt = CFunc_AppendStatement(ST_IFGOTO);
    stmt->expr = CABI_AcquireGuardVariable(guard, 0);
    label = newlabel();
    stmt->label = label;
    do {
        stmt = CFunc_AppendStatement(ST_EXPRESSION);
        stmt->expr = CInline_CopyExpression(init->expr, CopyModeLocal);
        init = init->next;
    } while (init && init->object == obj);
    stmt = CFunc_AppendStatement(ST_EXPRESSION);
    stmt->expr = CABI_ReleaseGuardVariable(guard, 0);
    stmt = CFunc_AppendStatement(ST_LABEL);
    stmt->label = label;
    label->stmt = stmt;
    return init;
}

Boolean CInline_GenerateDeferredFuncs(void) {
    CI_Action* action;
    CI_Export* export;
    UInt8 options[0x1E4];

    action = cinline_actionlist;
    export = cinline_exportlist;
    if (!anyerrors) {
        if (action) {
            cinline_actionlist = action->next;
            cinline_gendeps = TRUE;
            switch (action->actiontype) {
                case CI_ActionInlineFunc:
                    if (!(action->obj->flags & OBJECT_DEFINED)) {
                        CInline_GenerateInlineFunc(action);
                    }
                    break;
                case CI_ActionMemberFunc:
                    if (!(TYPE_FUNC(action->obj->type)->flags & FUNC_DEFINED)) {
                        fn_00519A00(
                            &action->u.memberfunc.templ->tail,
                            action->u.memberfunc.inst,
                            action->u.memberfunc.tmemb,
                            action->unk08,
                            action->obj,
                            0
                        );
                    }
                    break;
                case CI_ActionTemplateFunc:
                    if (!(TYPE_FUNC(action->obj->type)->flags & FUNC_DEFINED) &&
                        !action->u.templatefunc.inst->is_specialized) {
                        CTempl_GenFuncInstance(
                            action->u.templatefunc.func, action->u.templatefunc.inst, 0
                        );
                    }
                    break;
                case CI_ActionDefaultFunc:
                    CInline_GenerateDefaultFunc(action->obj);
                    break;
                default:
                    CError_FATAL(5663);
                    break;
            }
            cinline_gendeps = FALSE;
            return TRUE;
        }
        if (export && !copts.defer_codegen) {
            cinline_exportlist = export->next;
            CMach_GetBackEndOptions(options);
            CMach_SetBackEndOptions(&export->funcdata->unk10);
            CInline_GenIFunctionCode(export->object, export->funcdata, export->oldStyleParameters);
            CMach_SetBackEndOptions(options);
            copts.defer_codegen = FALSE;
            return TRUE;
        }
    }
    return FALSE;
}

static inline TypeClass* CInline_FindNestedTemplInst(TypeClass* tclass) {
    NameSpace* nspace;
    while (tclass) {
        if (tclass->flags & CLASS_IS_TEMPL_INST) {
            return tclass;
        }
        if (!copts.template_patch) {
            break;
        }
        nspace = tclass->nspace->parent;
        tclass = NULL;
        while (nspace) {
            if (nspace->theclass) {
                tclass = nspace->theclass;
                break;
            }
            nspace = nspace->parent;
        }
    }
    return NULL;
}

void CInline_GenerateInlineFunc(CI_Action* action) {
    Object* object;
    TypeFunc* type;
    TypeClass* tclass;
    TypeClass* inst;
    UInt32 state;
    UInt8 decl[0x80];

    object = action->obj;
    fn_00443E10(&action->u.inlinefunc.stream, &state);
    cparser_fileoffset = action->u.inlinefunc.fileoffset;
    symdecloffset_0063627C = cparser_fileoffset.tokenline;
    switch ((tk = lex())) {
        case ':':
        case '{':
        case 0x151:
            break;
        default:
            CError_FATAL(5511);
    }
    symdecltoken_00623E50 = *CPrep_CurStreamElement();
    type = TYPE_FUNC(object->type);
    if ((type->flags & FUNC_METHOD) && (TYPE_METHOD(object->type)->theclass->flags & 0x100)) {
        fn_0055E3D0(CTemplTool_Class2Template(TYPE_METHOD(object->type)->theclass), object, 1);
    } else {
        type->flags &= ~FUNC_DEFINED;
        if (object->type->type == TYPEFUNC &&
            (type->flags & FUNC_METHOD) &&
            (inst = CInline_FindNestedTemplInst(TYPE_METHOD(object->type)->theclass))) {
            CTempl_ParseInstanceScopeFunction(
                object, CTemplTool_Class2TemplClassInst(inst), action->unk08, NULL
            );
        } else {
            memclrw(decl, sizeof(decl));
            tclass = action->u.inlinefunc.tclass;
            if (tclass && (inst = CInline_FindNestedTemplInst(tclass))) {
                CTempl_ParseInstanceScopeFunction(
                    object, CTemplTool_Class2TemplClassInst(inst), action->unk08, tclass
                );
            } else {
                fn_00473CD0(object, decl, tclass, 0, 0, NULL);
            }
        }
    }
    fn_00443DA0(&action->u.inlinefunc.stream, &state);
}
