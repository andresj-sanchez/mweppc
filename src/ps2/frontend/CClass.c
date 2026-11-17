#include "compiler/RegListQueries.h"
#include "compiler/CClass.h"
#include "compiler/CDecl.h"
#include "compiler/CTemplateTools.h"
#include "compiler/CExpr.h"
#include "compiler/CError.h"
#include "compiler/CMangler.h"
#include "compiler/CParser.h"
#include "compiler/CScope.h"
#include "compiler/CFunc.h"
#include "compiler/CompilerTools.h"
#include "compiler/common.h"
#include "compiler/objects.h"
#include "compiler/scopes.h"
#include "compiler/types.h"

static struct ThunkList* cclass_thunklist;
static void* cclass_unk63682C;
static void* cclass_unk635A98;
static Boolean cclass_unk61BBC4;

void CClass_Init(void) {
    cclass_thunklist = NULL;
    cclass_unk63682C = NULL;
    cclass_unk635A98 = NULL;
    cclass_unk61BBC4 = FALSE;
}

void CClass_ClassDefaultFuncAction(TypeClass* tclass) {}

Object* CClass_AssignmentOperator(TypeClass* tclass) {
    NameSpaceObjectList* nsol;
    Object* object;
    FuncArg* args;

    for (nsol = CScope_FindName(tclass->nspace, asop_name_node); nsol; nsol = nsol->next) {
        object = nsol->object;
        if (object->otype == OT_OBJECT &&
            IS_TYPE_FUNC(object->type) &&
            (args = TYPE_FUNC(object->type)->args) &&
            args->next &&
            !args->next->next) {
            if (IS_TYPE_CLASS(args->next->type) && TYPE_CLASS(args->next->type) == tclass) {
                return object;
            }

            if (IS_TYPE_REFERENCE(args->next->type) &&
                TPTR_TARGET(args->next->type) == TYPE(tclass)) {
                return object;
            }
        }
    }

    return NULL;
}

NameSpaceObjectList* CClass_MemberObject(TypeClass* tclass, HashNameNode* name) {
    NameSpaceObjectList* nsol;

    nsol = CScope_FindName(tclass->nspace, name);
    if (nsol != NULL && nsol->object->otype == OT_OBJECT) {
        return nsol;
    }

    return NULL;
}

NameSpaceObjectList* CClass_Constructor(TypeClass* tclass) {
    NameSpaceObjectList* nsol;

    if ((nsol = CScope_FindName(tclass->nspace, constructor_name_node)) &&
        nsol->object->otype == OT_OBJECT &&
        IS_TYPE_FUNC(nsol->object->type)) {
        return nsol;
    }

    return NULL;
}

Object* CClass_Destructor(TypeClass* tclass) {
    NameSpaceObjectList* nsol;

    for (nsol = CScope_FindName(tclass->nspace, destructor_name_node); nsol; nsol = nsol->next) {
        if (nsol->object->otype == OT_OBJECT && IS_TYPE_FUNC(nsol->object->type)) {
            return nsol->object;
        }
    }

    return NULL;
}

Type* CClass_CombineClassAccessQualifiers(
    Type* type, UInt32 qual1, UInt32 qual2, UInt32* outflags
) {
    Type* inner;

    qual2 = qual2 & (Q_CONST | Q_VOLATILE);
    if (qual1 & Q_MUTABLE) {
        qual2 &= ~Q_CONST;
    }
    qual1 = qual1 & (Q_CONST | Q_VOLATILE);

    inner = type;
    while (IS_TYPE_ARRAY(inner)) {
        inner = TPTR_TARGET(inner);
    }

    switch ((SInt8)inner->type) {
        case TYPEMEMBERPOINTER:
        case TYPEPOINTER:
            if (qual2) {
                type = CClass_PointerTypeCopy(type);
                inner = type;
                while (IS_TYPE_ARRAY(inner)) {
                    inner = TPTR_TARGET(inner);
                }

                switch ((SInt8)inner->type) {
                    case TYPEPOINTER:
                        TYPE_POINTER(inner)->qual |= qual2;
                        break;
                    case TYPEMEMBERPOINTER:
                        TYPE_MEMBER_POINTER(inner)->qual |= qual2;
                        break;
                    default:
                        CError_FATAL(3883);
                }
            }
            break;
        default:
            qual1 |= qual2;
    }

    *outflags = qual1;
    return type;
}

Boolean CClass_ReferenceArgument(TypeClass* tclass) {
    if ((tclass->flags & (CLASS_COMPLETED | CLASS_IS_TEMPL_INST)) == CLASS_IS_TEMPL_INST) {
        CDecl_CompleteType(TYPE(tclass));
    }

    if (copts.simple_class_byval) {
        return !CClass_IsTrivialCopyClass(tclass);
    }

    return CClass_Destructor(tclass) != NULL || CClass_HasCopyConstructor(tclass) != NULL;
}

Boolean CClass_OverridesBaseMember(TypeClass* tclass, HashNameNode* name, Object* obj) {
    NameSpaceObjectList* nsol;
    ClassList* base;
    Boolean result;
    Object* object;

    result = FALSE;
    for (base = tclass->bases; base; base = base->next) {
        if (base->base->vtable) {
            for (nsol = CScope_FindName(base->base->nspace, name); nsol; nsol = nsol->next) {
                object = OBJECT(nsol->object);
                if (object->otype == OT_OBJECT &&
                    object->datatype == DVFUNC &&
                    CClass_GetOverrideKind(object, obj, TRUE)) {
                    result = TRUE;
                }
            }

            if (CClass_OverridesBaseMember(base->base, name, obj)) {
                result = TRUE;
            }
        }
    }

    return result;
}

ENode* CClass_ClassPointerCast(
    ENode* expr,
    TypeClass* a,
    TypeClass* b,
    Boolean typconflag,
    Boolean ambigerrorflag,
    Boolean pathcheckflag
) {
    BClassList* path;
    Boolean reverse;
    short depth;
    Boolean isambigbase;

    reverse = FALSE;

    if (a != b) {
        do {
            path = CClass_GetBasePath(a, b, &depth, &isambigbase);
            if (path == NULL) {
                if (!typconflag) {
                    CError_Error(ERR_CLASS_CAST_UNKNOWN_BASE, a, 0, b, 0);
                    break;
                }
                if ((path = CClass_GetBasePath(b, a, &depth, &isambigbase)) == NULL) {
                    break;
                }
                if (pathcheckflag) {
                    CClass_NewAccessCheck(0, b, a, 0, 0, 0);
                }
                reverse = TRUE;
            } else {
                if (pathcheckflag) {
                    CClass_NewAccessCheck(0, a, b, 0, 0, 0);
                }
            }
            if (isambigbase && ambigerrorflag) {
                CError_Error(ERR_AMBIGUOUS_ACCESS_CLASS_STRUCT_UNION_MEMBER, a, 0, b, 0);
            }
            if (!(a->flags & CLASS_SINGLE_OBJECT) && !b->sominfo) {
                expr = CClass_AccessPathCast(path, expr, reverse);
            }
        } while (0);
    }

    if (typconflag && ENODE_IS(expr, EINDIRECT) && IS_TYPE_POINTER_ONLY(expr->rtype)) {
        expr = makemonadicnode(expr, ETYPCON);
    }

    return expr;
}

ENode* CClass_AccessPathCast(BClassList* path, ENode* expr, Boolean reverse) {
    ClassList* base;
    SInt32 offset;

    while (path && path->next) {
        base = TYPE_CLASS(path->type)->bases;
        while (1) {
            if (base->base == TYPE_CLASS(path->next->type)) {
                break;
            }
            CError_ASSERT(1251, base = base->next);
        }

        if (base->is_virtual) {
            if (reverse) {
                CError_Error(ERR_ILLEGAL_TYPE_CAST);
                break;
            }

            if (!base->base->sominfo) {
                offset = base->offset;
                if (offset && !canadd(expr, offset)) {
                    expr = makediadicnode(expr, intconstnode(TYPE(&stunsignedint), offset), EADD);
                    optimizecomm(expr);
                }

                expr->rtype = CDecl_NewPointerType(CDecl_NewPointerType(TYPE(base->base)));
                expr = makemonadicnode(expr, EINDIRECT);
                expr->rtype = expr->data.monadic->rtype;
            }
        } else {
            offset = base->offset;
            if (offset) {
                if (reverse) {
                    offset = -offset;
                }
                if (!canadd(expr, offset)) {
                    expr = makediadicnode(expr, intconstnode(TYPE(&stunsignedint), offset), EADD);
                    optimizecomm(expr);
                }
            }
        }

        path = path->next;
    }

    return expr;
}

SInt32 CClass_GetPathOffset(BClassList* path) {
    SInt32 offset;
    ClassList* base;

    offset = 0;

    while (path->next) {
        if (path->type != path->next->type) {
            for (base = TYPE_CLASS(path->type)->bases; base; base = base->next) {
                if (base->base == TYPE_CLASS(path->next->type)) {
                    break;
                }
            }

            if (base == NULL) {
                CError_Error(ERR_ILLEGAL_USE_NON_STATIC_MEMBER);
                return offset;
            }
        }

        if (base->is_virtual) {
            return -1;
        }
        offset += base->offset;

        path = path->next;
    }

    return offset;
}

Boolean CClass_IsPODClass(TypeClass* tclass) {
    NameSpaceObjectList* nsol;
    Object* object;
    ObjMemberVar* ivar;
    Type* type;

    if (tclass->vtable || tclass->bases || CClass_Destructor(tclass)) {
        return FALSE;
    }

    for (nsol = CClass_Constructor(tclass); nsol; nsol = nsol->next) {
        if (nsol->object->otype == OT_OBJECT &&
            IS_TYPE_FUNC(OBJECT(nsol->object)->type) &&
            !(TYPE_FUNC(OBJECT(nsol->object)->type)->flags & FUNC_AUTO_GENERATED)) {
            return FALSE;
        }
    }

    object = CClass_AssignmentOperator(tclass);
    if (object != NULL && !(TYPE_FUNC(object->type)->flags & FUNC_AUTO_GENERATED)) {
        return FALSE;
    }

    for (ivar = tclass->ivars; ivar; ivar = ivar->next) {
        if (ivar->access != ACCESSPUBLIC) {
            return FALSE;
        }

        type = ivar->type;
        while (IS_TYPE_ARRAY(type)) {
            type = TPTR_TARGET(type);
        }

        switch (type->type) {
            case TYPECLASS:
                if (!CClass_IsPODClass(TYPE_CLASS(type))) {
                    return FALSE;
                }
                break;
            case TYPEMEMBERPOINTER:
                return FALSE;
            case TYPEPOINTER:
                if (TPTR_QUAL(type) & Q_REFERENCE) {
                    return FALSE;
                }
                break;
            default:
                break;
        }
    }

    return TRUE;
}

ENode* CClass_CreateThisSelfExpr(void) {
    ObjectList* list;
    Object* object;
    ENode* expr;

    if (cscope_currentfunc && cscope_currentclass && cscope_is_member_func) {
        for (list = ig_reglist_a; list; list = list->next) {
            object = list->object;
            if (object->name == this_name_node) {
                goto haveObject;
            }
        }
    }
    CError_Error(copts.cplusplus ? ERR_ILLEGAL_USE_THIS : ERR_ILLEGAL_USE_SELF);
    object = NULL;

haveObject:
    if (object == NULL) {
        return NULL;
    }

    if (CTemplTool_IsTypeDepType(TYPE(cscope_currentclass))) {
        CError_ASSERT(2951, object->datatype == 1);
        expr = CExpr_NewTemplDepENode(8);
        expr->data.objref = object;
        object->flags |= OBJECT_USED;
    } else {
        expr = create_objectrefnode(object);
        expr->rtype = CDecl_NewPointerType(TYPE(cscope_currentclass));
        expr = makemonadicnode(expr, EINDIRECT);
        expr->data.monadic->rtype = CDecl_NewPointerType(expr->rtype);
    }

    return expr;
}

static TypeClass* cclass_isbase_mostderived;
static SInt32 cclass_isbase_foundoffset;
static Boolean cclass_isambigbase;
static short cclass_founddepth;

Boolean CClass_IsBaseClass(
    TypeClass* a, TypeClass* b, short* founddepth, Boolean pathcheckflag, Boolean ambigerrorflag
) {
    BClassList* path;
    short depth;
    Boolean isambigbase;

    path = CClass_GetBasePath(a, b, &depth, &isambigbase);
    if (path != NULL) {
        if (isambigbase && ambigerrorflag) {
            CError_Error(ERR_AMBIGUOUS_ACCESS_CLASS_STRUCT_UNION_MEMBER, a, 0, b, 0);
        }

        if (pathcheckflag) {
            CClass_NewAccessCheck(0, a, b, 0, 0, 0);
        }

        if (founddepth) {
            *founddepth = depth;
        }

        return TRUE;
    }

    return FALSE;
}

BClassList* CClass_GetBasePath(
    TypeClass* a, TypeClass* b, short* founddepth, Boolean* isambigbase
) {
    BClassList* path;
    BClassList* result;

    if ((a->flags & (CLASS_COMPLETED | CLASS_IS_TEMPL_INST)) == CLASS_IS_TEMPL_INST) {
        CDecl_CompleteType(TYPE(a));
    }
    if ((b->flags & (CLASS_COMPLETED | CLASS_IS_TEMPL_INST)) == CLASS_IS_TEMPL_INST) {
        CDecl_CompleteType(TYPE(b));
    }

    cclass_founddepth = 0;
    cclass_isbase_mostderived = a;
    cclass_isbase_foundoffset = -1;
    cclass_isambigbase = FALSE;

    path = CClass_GetBasePathRec(a, b, 0, 1);
    if (path != NULL) {
        *founddepth = cclass_founddepth;
        *isambigbase = cclass_isambigbase;

        result = lalloc(sizeof(BClassList));
        result->next = path;
        result->type = TYPE(a);
        return result;
    } else {
        *isambigbase = FALSE;
        return NULL;
    }
}

Boolean CClass_IsMoreAccessiblePath(BClassList* path1, BClassList* path2) {
    switch (CClass_GetPathAccess(path2)) {
        case ACCESSPUBLIC:
            return FALSE;
        case ACCESSNONE:
            return TRUE;
        default:
            CError_FATAL(1072);
        case ACCESSPROTECTED:
            switch (CClass_GetPathAccess(path1)) {
                case ACCESSPUBLIC:
                    return TRUE;
                case ACCESSPRIVATE:
                case ACCESSPROTECTED:
                case ACCESSNONE:
                    return FALSE;
                default:
                    CError_FATAL(1081);
            }
        case ACCESSPRIVATE:
            switch (CClass_GetPathAccess(path1)) {
                case ACCESSPUBLIC:
                case ACCESSPROTECTED:
                    return TRUE;
                case ACCESSPRIVATE:
                case ACCESSNONE:
                    return FALSE;
                default:
                    CError_FATAL(1091);
            }
    }

    return FALSE;
}

ENode* CClass_DirectBasePointerCast(ENode* expr, TypeClass* a, TypeClass* b) {
    ClassList* base;
    VClassList* vbase;
    BClassList* path;
    BClassList list1;
    BClassList list2;
    short depth;
    Boolean isambigbase;

    for (base = a->bases; base; base = base->next) {
        if (base->base == b) {
            list1.next = &list2;
            list1.type = TYPE(a);
            list2.next = NULL;
            list2.type = TYPE(b);
            return CClass_AccessPathCast(&list1, expr, FALSE);
        }
    }

    for (vbase = a->vbases; vbase; vbase = vbase->next) {
        if (vbase->base == b) {
            path = CClass_GetBasePath(a, b, &depth, &isambigbase);
            CError_ASSERT(1383, path != NULL);
            return CClass_AccessPathCast(path, expr, FALSE);
        }
    }

    CError_FATAL(1387);
    return expr;
}

Boolean CClass_IsConstructor(Object* obj) {
    return obj && IS_TYPE_FUNC(obj->type) && (TYPE_FUNC(obj->type)->flags & FUNC_IS_CTOR);
}

Boolean CClass_IsDestructor(Object* obj) {
    return obj && IS_TYPE_FUNC(obj->type) && (TYPE_FUNC(obj->type)->flags & FUNC_IS_DTOR);
}
