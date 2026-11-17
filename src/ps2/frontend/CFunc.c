#include "compiler/RegListQueries.h"
#include "compiler/CDecl.h"
#include "compiler/CError.h"
#include "compiler/CExpr.h"
#include "compiler/CFunc.h"
#include "compiler/CParser.h"
#include "compiler/CException.h"
#include "compiler/CompilerTools.h"
#include "compiler/CScope.h"
#include "compiler/common.h"
#include "compiler/objects.h"
#include "compiler/scopes.h"
#include "compiler/types.h"
#include "compiler/CClass.h"
#include "compiler/CBrowse.h"
#include "compiler/CInline.h"
#include "compiler/CSOM.h"

static DeclBlock* currentblock;

DeclBlock* CFunc_NewDeclBlock(void) {
    DeclBlock* block;
    NameSpace* nspace;

    block = lalloc(sizeof(DeclBlock));
    memclrw(block, sizeof(DeclBlock));
    block->next = currentblock;
    block->parent_nspace = cscope_current;
    block->dobjstack = cexcept_dobjstack;

    nspace = CScope_NewListNameSpace(NULL, FALSE);
    nspace->parent = cscope_current;
    cscope_current = nspace;

    currentblock = block;
    return block;
}

void CFunc_SetupLocalVarInfo(Object* obj) {
    obj->u.varptr = CodeGen_GetNewVarInfo();
    ((VarInfo*)obj->u.varptr)->func = cscope_currentfunc;

    if (obj->sclass == TK_REGISTER) {
        if (!copts.optimizesize) {
            ((VarInfo*)obj->u.varptr)->usage = 100;
        } else {
            ((VarInfo*)obj->u.varptr)->usage = 5;
        }
    }

    if (obj->type && is_volatile_object(obj)) {
        ((VarInfo*)obj->u.varptr)->noregister = TRUE;
    }
}

CLabel* newlabel(void) {
    CLabel* label = lalloc(sizeof(CLabel));
    memclrw(label, sizeof(CLabel));

    label->uniquename = CParser_GetUniqueName();
    label->name = label->uniquename;
    return label;
}

Statement* CFunc_AppendStatement(StatementType sttype) {
    Statement* stmt = lalloc(sizeof(Statement));
    memclrw(stmt, sizeof(Statement));

    stmt->type = sttype;
    stmt->value = curstmtvalue;
    stmt->sourceoffset = sourceoffset;
    stmt->sourcefilepath = sourcefilepath;
    stmt->dobjstack = cexcept_dobjstack;

    curstmt->next = stmt;
    curstmt = stmt;
    return stmt;
}

Statement* CFunc_InsertStatement(StatementType sttype, Statement* after) {
    Statement* stmt = lalloc(sizeof(Statement));
    memclrw(stmt, sizeof(Statement));

    stmt->next = after->next;
    after->next = stmt;
    stmt->type = sttype;
    stmt->value = after->value;
    stmt->sourceoffset = after->sourceoffset;
    stmt->sourcefilepath = after->sourcefilepath;
    stmt->dobjstack = after->dobjstack;

    return stmt;
}

Statement* CFunc_InsertBeforeStatement(StatementType sttype, Statement* before) {
    Statement* stmt = lalloc(sizeof(Statement));

    *stmt = *before;
    before->next = stmt;
    before->type = sttype;
    before->flags = 0;

    return before;
}

void CheckCLabels(void) {
    CLabel* scan;

    for (scan = Labels; scan; scan = scan->next) {
        if (!scan->stmt) {
            CError_Error(ERR_UNDEFINED_LABEL, scan->name->name);
        }
    }
}

Object* create_temp_object(Type* type) {
    Object* object = CParser_NewLocalDataObject(NULL, TRUE);
    object->name = CParser_GetUniqueName();
    object->type = type;
    CFunc_SetupLocalVarInfo(object);
    return object;
}

ENode* create_temp_node(Type* type) {
    ENode* node = CExpr_NewETEMPNode(type, FALSE);
    if (IS_TYPE_CLASS(type) && CClass_Destructor(TYPE_CLASS(type))) {
        node->data.temp.needs_dtor = TRUE;
    }
    return node;
}

ENode* create_temp_node2(Type* type) {
    ENode* node = CExpr_NewETEMPNode(type, FALSE);
    node->data.temp.needs_dtor = TRUE;
    return node;
}

void CFunc_GenerateSingleExprFunc(Object* func, ENode* expr) {
    NameSpace* nspace;
    Boolean saveDebugInfo;
    Statement firstStmt;
    Statement* stmt;

    if (!anyerrors) {
        nspace = CFunc_FuncGenSetup(&firstStmt, func);

        saveDebugInfo = copts.unknown134;
        copts.unknown134 = FALSE;

        stmt = CFunc_AppendStatement(ST_EXPRESSION);
        stmt->expr = expr;

        stmt = CFunc_AppendStatement(ST_RETURN);
        stmt->expr = NULL;

        if (cscope_currentclass && cscope_currentclass->sominfo) {
            CSOM_InitSOMSelf(cscope_currentclass, &firstStmt);
        }

        fn_004EC8B0(&firstStmt);
        CInline_CheckInlineability(&firstStmt, func, 0);

        cscope_current = nspace->parent;
        copts.unknown134 = saveDebugInfo;
    }
}

void InitExpr_Register(ENode* expr, Object* object) {
    InitExpr* initexpr;
    InitExpr* scan;

    if (cparamblkptr->precompile == 1 &&
        object->sclass != TK_STATIC &&
        !(object->qual & (Q_IMPLICIT_WEAK | Q_WEAK))) {
        CError_Error(ERR_ILLEGAL_USE_PRECOMPILED_HEADER);
        return;
    }

    if (copts.suppress_init_code) {
        return;
    }

    initexpr = galloc(sizeof(InitExpr));
    initexpr->next = NULL;
    initexpr->object = object;
    initexpr->expr = CInline_CopyExpression(expr, CopyModeGlobal);

    if (init_expressions) {
        scan = init_expressions;
        while (scan->next) {
            scan = scan->next;
        }
        scan->next = initexpr;
    } else {
        init_expressions = initexpr;
    }
}

NameSpace* CFunc_FuncGenSetup(Statement* stmt, Object* func) {
    NameSpace* nspace;
    DeclBlock* block;

    nspace = CScope_NewListNameSpace(NULL, FALSE);
    nspace->parent = cscope_current;
    cscope_current = nspace;

    functionbodyoffset = 0;
    functionbodypath = NULL;
    ig_reglist_a = NULL;
    ig_reglist_b = NULL;
    Labels = NULL;
    codegen_varnumber_637340 = 0;
    cfunc_staticvarcount = 0;
    etempid = 0;
    unk605EC0 = FALSE;

    CExcept_Setup();

    memclrw(stmt, sizeof(Statement));
    curstmt = stmt;
    stmt->type = ST_NOP;
    stmt->value = curstmtvalue = 1;

    block = lalloc(sizeof(DeclBlock));
    memclrw(block, sizeof(DeclBlock));
    block->parent_nspace = cscope_current;
    currentblock = block;

    return nspace;
}

void CFunc_GenerateDummyCtorFunc(Object* func, Object* real_ctor) {
    NameSpace* nspace;
    FuncArg* arg;
    ENodeList* list;
    ENode* expr;
    Boolean saveDebugInfo;
    Statement firstStmt;
    Statement* stmt;

    if (!anyerrors) {
        cscope_currentfunc = func;

        nspace = CFunc_FuncGenSetup(&firstStmt, func);

        saveDebugInfo = copts.unknown134;
        copts.unknown134 = FALSE;

        CFunc_SetupNewFuncArgs(func, TYPE_FUNC(func->type)->args);

        expr = CExpr_NewENode(EFUNCCALL);
        expr->type = EFUNCCALL;
        expr->cost = 200;
        expr->rtype = TYPE(&void_ptr);
        expr->data.funccall.funcref = CExpr_MakeObjRefNode(real_ctor, FALSE);
        expr->data.funccall.functype = TYPE_FUNC(func->type);

        CError_ASSERT(5637, IS_TYPE_FUNC(real_ctor->type));
        CError_ASSERT(5638, TYPE_FUNC(real_ctor->type)->flags & FUNC_METHOD);
        CError_ASSERT(5639, arg = TYPE_FUNC(real_ctor->type)->args);
        CError_ASSERT(5641, arg = arg->next);
        CError_ASSERT(5642, ig_reglist_a);

        list = lalloc(sizeof(ENodeList));
        expr->data.funccall.args = list;
        list->node = create_objectnode(ig_reglist_a->object);

        if (TYPE_METHOD(real_ctor->type)->theclass->flags & CLASS_HAS_VBASES) {
            CError_ASSERT(5648, arg = arg->next);
            CError_ASSERT(5649, ig_reglist_a->next != NULL);
            list->next = lalloc(sizeof(ENodeList));
            list = list->next;
            list->node = create_objectnode(ig_reglist_a->next->object);
        }

        while (arg) {
            CError_ASSERT(5658, arg->dexpr != NULL);
            list->next = lalloc(sizeof(ENodeList));
            list = list->next;
            list->node = CExpr_GetDefaultArgument(expr->data.funccall.funcref, arg);
            arg = arg->next;
        }

        list->next = NULL;

        stmt = CFunc_AppendStatement(ST_RETURN);
        stmt->expr = expr;

        if (cscope_currentclass && cscope_currentclass->sominfo) {
            CSOM_InitSOMSelf(cscope_currentclass, &firstStmt);
        }

        fn_004EC8B0(&firstStmt);
        CInline_CheckInlineability(&firstStmt, func, 0);

        cscope_current = nspace->parent;
        cscope_currentfunc = NULL;
        copts.unknown134 = saveDebugInfo;
    }
}

static inline Boolean CFunc_NeedsCleanup(Statement* stmt, Statement* dest) {
    ExceptionAction *from, *p;

    for (p = dest->dobjstack; p; p = p->prev) {
        if (CExcept_ActionCompare(p, stmt->dobjstack)) {
            return FALSE;
        }
    }
    for (from = stmt->dobjstack, p = dest->dobjstack; from && !CExcept_ActionCompare(from, p);
         from = from->prev) {
        if (CExcept_ActionNeedsDestruction(from)) {
            return TRUE;
        }
    }
    return FALSE;
}

static inline Boolean CFunc_AnyCleanup(ExceptionAction* p) {
    while (p) {
        if (CExcept_ActionNeedsDestruction(p)) {
            return TRUE;
        }
        if (CExcept_ActionCompare(p, NULL)) {
            break;
        }
        p = p->prev;
    }
    return FALSE;
}

static inline ExceptionAction* CFunc_CommonScope(Statement* stmt, ExceptionAction* to) {
    ExceptionAction* p;

    while (to) {
        for (p = stmt->dobjstack; p; p = p->prev) {
            if (CExcept_ActionCompare(p->prev, to)) {
                return p;
            }
        }
        to = to->prev;
    }
    return NULL;
}

static inline Statement* CFunc_EmitCleanups(
    Statement* stmt, ExceptionAction* p, ExceptionAction* stop
) {
    while (p) {
        stmt = CExcept_ActionCleanup(p, stmt);
        if (CExcept_ActionCompare(p, stop)) {
            break;
        }
        p = p->prev;
    }
    return stmt;
}

void CFunc_DestructorCleanup(Statement* first) {
    Statement* volatile next;
    Statement* stmt;

    if (unk605EC0) {
        fn_0047ABD0(first);
    }
    if (!cexcept_hasdobjects) {
        return;
    }

    stmt = first;
    for (;;) {
        if (!(next = stmt->next)) {
            if (stmt->type != ST_RETURN && stmt->dobjstack) {
                CFunc_EmitCleanups(stmt, stmt->dobjstack, NULL);
            }
            break;
        }
        switch (next->type) {
            case ST_GOTO:
                if (!CExcept_ActionCompare(stmt->dobjstack, next->label->stmt->dobjstack) &&
                    CFunc_NeedsCleanup(stmt, next->label->stmt)) {
                    CFunc_EmitCleanups(
                        stmt, stmt->dobjstack, CFunc_CommonScope(stmt, next->label->stmt->dobjstack)
                    );
                }
                stmt = next;
                break;
            case ST_RETURN:
                if (next->expr && CFunc_AnyCleanup(stmt->dobjstack)) {
                    DestructorReturnTransform(stmt, next);
                } else if (stmt->dobjstack) {
                    CFunc_EmitCleanups(stmt, stmt->dobjstack, NULL);
                }
                stmt = next;
                break;
            default:
                switch (stmt->type) {
                    case ST_NOP:
                    case ST_LABEL:
                    case ST_EXPRESSION:
                    case ST_IFGOTO:
                    case ST_IFNGOTO:
                    case ST_BEGINCATCH:
                    case ST_ENDCATCH:
                    case ST_ENDCATCHDTOR:
                    case ST_ASM:
                        if (!CExcept_ActionCompare(stmt->dobjstack, next->dobjstack) &&
                            CFunc_NeedsCleanup(stmt, next)) {
                            CFunc_EmitCleanups(
                                stmt, stmt->dobjstack, CFunc_CommonScope(stmt, next->dobjstack)
                            );
                        }
                        break;
                    case ST_GOTO:
                    case ST_SWITCH:
                    case ST_RETURN:
                        break;
                    default:
                        CError_FATAL(1250);
                        break;
                }
                switch (next->type) {
                    case ST_NOP:
                    case ST_LABEL:
                    case ST_EXPRESSION:
                    case ST_SWITCH:
                    case ST_BEGINCATCH:
                    case ST_ENDCATCH:
                    case ST_ENDCATCHDTOR:
                    case ST_ASM:
                        stmt = next;
                        break;
                    case ST_IFGOTO:
                    case ST_IFNGOTO:
                        if (!CExcept_ActionCompare(next->dobjstack, next->label->stmt->dobjstack) &&
                            CFunc_NeedsCleanup(next, next->label->stmt)) {
                            stmt = DestructorIfTransform(next);
                        } else {
                            stmt = next;
                        }
                        break;
                    default:
                        CError_FATAL(1280);
                        break;
                }
        }
    }
}

static inline Statement* DestructLocals(
    Statement* stmt, ExceptionAction* ea, ExceptionAction* end
) {
    while (ea) {
        stmt = CExcept_ActionCleanup(ea, stmt);
        if (CExcept_ActionCompare(ea, end)) {
            break;
        }
        ea = ea->prev;
    }
    return stmt;
}

static inline ExceptionAction* FindLastNonCommonStackObj(Statement* stmt1, Statement* stmt2) {
    ExceptionAction* ea1;
    ExceptionAction* ea2;

    for (ea2 = stmt2->dobjstack; ea2; ea2 = ea2->prev) {
        for (ea1 = stmt1->dobjstack; ea1; ea1 = ea1->prev) {
            if (CExcept_ActionCompare(ea1->prev, ea2)) {
                return ea1;
            }
        }
    }

    return NULL;
}

Statement* DestructorIfTransform(Statement* stmt) {
    CLabel* label;
    Statement* newStmt;

    if (stmt->type == ST_IFGOTO) {
        stmt->type = ST_IFNGOTO;
    } else {
        stmt->type = ST_IFGOTO;
    }

    label = newlabel();
    newStmt =
        DestructLocals(stmt, stmt->dobjstack, FindLastNonCommonStackObj(stmt, stmt->label->stmt));
    newStmt = CFunc_InsertStatement(ST_GOTO, newStmt);
    newStmt->label = stmt->label;
    newStmt = CFunc_InsertStatement(ST_LABEL, newStmt);
    newStmt->label = label;
    label->stmt = newStmt;
    stmt->label = label;
    newStmt->dobjstack = stmt->dobjstack;
    return newStmt;
}

static inline Boolean NeedsDestruction(Statement* stmt1, Statement* stmt2) {
    ExceptionAction* ea1;
    ExceptionAction* ea2;
    ExceptionAction* scan;

    ea1 = stmt1->dobjstack;
    ea2 = stmt2->dobjstack;

    for (scan = ea2; scan; scan = scan->prev) {
        if (CExcept_ActionCompare(scan, ea1)) {
            return FALSE;
        }
    }

    while (ea1 && !CExcept_ActionCompare(ea1, ea2)) {
        if (CExcept_ActionNeedsDestruction(ea1)) {
            return TRUE;
        }
        ea1 = ea1->prev;
    }

    return FALSE;
}

static inline Boolean DestructorNeeded(ExceptionAction* ea, ExceptionAction* end) {
    while (ea) {
        if (CExcept_ActionNeedsDestruction(ea)) {
            return TRUE;
        }
        if (CExcept_ActionCompare(ea, end)) {
            break;
        }
        ea = ea->prev;
    }

    return FALSE;
}

void DestructorReturnTransform(Statement* stmt1, Statement* stmt2) {
    Object* tempobj;
    Statement* stmt;

    if (stmt1->dobjstack != stmt2->dobjstack && NeedsDestruction(stmt1, stmt2)) {
        stmt1 = DestructLocals(stmt1, stmt1->dobjstack, FindLastNonCommonStackObj(stmt1, stmt2));
    }

    if (DestructorNeeded(stmt2->dobjstack, NULL)) {
        if (CMach_GetFunctionResultClass(TYPE_FUNC(cscope_currentfunc->type)) != 1) {
            tempobj = create_temp_object(stmt2->expr->rtype);
            stmt = CFunc_InsertStatement(ST_EXPRESSION, stmt1);
            stmt->expr = makediadicnode(create_objectnode(tempobj), stmt2->expr, EASS);
            stmt->sourceoffset = stmt2->sourceoffset;
            stmt->sourcefilepath = stmt2->sourcefilepath;
            DestructLocals(stmt, stmt2->dobjstack, NULL);
            stmt2->expr = create_objectnode(tempobj);
            stmt2->dobjstack = NULL;
        } else {
            stmt = CFunc_InsertStatement(ST_EXPRESSION, stmt1);
            stmt->expr = stmt2->expr;
            stmt->sourceoffset = stmt2->sourceoffset;
            stmt->sourcefilepath = stmt2->sourcefilepath;
            DestructLocals(stmt, stmt2->dobjstack, NULL);
            stmt2->expr = nullnode();
            stmt2->dobjstack = NULL;
        }
    }
}
