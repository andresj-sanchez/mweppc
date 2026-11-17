#include "compiler/CABI.h"
#include "compiler/CError.h"
#include "compiler/CExpr.h"
#include "compiler/CMachine.h"
#include "compiler/CMangler.h"
#include "compiler/CDecl.h"
#include "compiler/CParser.h"
#include "compiler/CompilerTools.h"
#include "compiler/common.h"
#include "compiler/enode.h"
#include "compiler/objects.h"
#include "compiler/types.h"
#include "compiler/scopes.h"
#include "compiler/CScope.h"
#include "compiler/CSOM.h"
#include "compiler/RegListQueries.h"

short CABI_GetStructResultArgumentIndex(TypeFunc* tfunc) {
    return 0;
}

Type* CABI_GetSizeTType(void) {
    return TYPE(&stunsignedint);
}

Type* CABI_GetPtrDiffTType(void) {
    return TYPE(&stsignedint);
}

SInt16 CABI_StructSizeAlignValue(Type* type, SInt32 size) {
    int align;

    align = CMach_GetTypeAlign(type);
    if (align <= 1) {
        return 0;
    } else {
        return (align - 1) & (align - ((size & (align - 1))));
    }
}

void CABI_ReverseBitField(TypeBitfield* tbitfield) {
    int bits;
    UInt8 offset;
    UInt8 length;

    switch (tbitfield->bitfieldtype->size) {
        case 1:
            bits = 8;
            break;
        case 2:
            bits = 16;
            break;
        case 4:
            bits = 32;
            break;
        case 8:
            bits = 64;
            break;
        default:
            CError_FATAL(175);
    }

    length = tbitfield->bitlength;
    offset = tbitfield->offset;
    tbitfield->offset = (bits - offset) - length;
}

SInt32 CABI_GetVTableOffset(TypeClass* tclass) {
    return 0;
}

static Object* CABI_ThisArg(void) {
    CError_ASSERT(1110, ig_reglist_a && IS_TYPE_POINTER_ONLY(ig_reglist_a->object->type));
    return ig_reglist_a->object;
}

ENode* CABI_MakeThisExpr(TypeClass* tclass, SInt32 offset) {
    ENode* expr;

    if (tclass) {
        if (!tclass->sominfo) {
            expr = create_objectnode(CABI_ThisArg());
            if (tclass->flags & CLASS_HANDLEOBJECT) {
                expr = makemonadicnode(expr, EINDIRECT);
            }
        } else {
            expr = CSOM_SOMSelfObjectExpr(tclass);
        }
    } else {
        expr = create_objectnode(CABI_ThisArg());
        expr->rtype = TYPE(&void_ptr);
    }

    if (offset) {
        expr = makediadicnode(expr, intconstnode(TYPE(&stunsignedint), offset), EADD);
    }

    return expr;
}

static SInt32 CABI_FindNVBase(TypeClass* tclass, TypeClass* baseclass, SInt32 offset) {
    ClassList* base;
    SInt32 baseOffset;

    if (tclass == baseclass) {
        return offset;
    }

    for (base = tclass->bases; base; base = base->next) {
        if (!base->is_virtual &&
            (baseOffset = CABI_FindNVBase(base->base, baseclass, offset + base->offset)) >= 0) {
            return baseOffset;
        }
    }

    return -1;
}

static SInt32 CABI_GetBaseSize(TypeClass* tclass) {
    SInt32 size = tclass->size;

    if (copts.vbase_abi_v2 && tclass->vbases) {
        return tclass->vbases->offset;
    }

    return size;
}

SInt32 CABI_GetCtorOffsetOffset(TypeClass* tclass, TypeClass* baseclass) {
    SInt32 baseSize;
    SInt32 size;
    char saveAlignMode;

    size = CABI_GetBaseSize(tclass);
    if (baseclass) {
        baseSize = CABI_FindNVBase(tclass, baseclass, 0);
        CError_ASSERT(1357, baseSize >= 0);
        size -= baseSize;
    }

    saveAlignMode = copts.structalignment;
    {
        int alignBits = tclass->eflags & CLASS_EFLAGS_STRUCT_ALIGN;
        if (alignBits) {
            copts.structalignment = (alignBits >> 4) - 1;
        }
    }
    size += CMach_MemberAlignValue(TYPE(&stunsignedint), size);
    copts.structalignment = saveAlignMode;

    return size;
}

static Boolean CABI_IsOperatorNew(Object* obj) {
    return obj->otype == OT_OBJECT &&
        IS_TYPE_FUNC(obj->type) &&
        TYPE_FUNC(obj->type)->args &&
        TYPE_FUNC(obj->type)->args->type == CABI_GetSizeTType() &&
        TYPE_FUNC(obj->type)->args->next == NULL;
}

Object* CABI_ConstructorCallsNew(TypeClass* tclass) {
    NameSpaceObjectList* nsol;
    NameResult pr;

    if (tclass->sominfo == NULL && (tclass->flags & CLASS_HANDLEOBJECT)) {
        if (CScope_FindClassMemberObject(tclass, &pr, CMangler_OperatorName(TK_NEW))) {
            if (pr.object) {
                if (CABI_IsOperatorNew(OBJECT(pr.object))) {
                    return OBJECT(pr.object);
                }
            } else {
                for (nsol = pr.objects; nsol; nsol = nsol->next) {
                    if (CABI_IsOperatorNew(OBJECT(nsol->object))) {
                        return OBJECT(nsol->object);
                    }
                }
            }
        }

        return newh_func;
    }

    return NULL;
}

Object* CABI_GetDestructorObject(Object* obj, CABIDestroyMode mode) {
    return obj;
}

ENode* CABI_DestroyObject(
    Object* dtor, ENode* objexpr, CABIDestroyMode mode, Boolean nonvirtual, Boolean dealloc
) {
    ENode* expr;
    short varg;
    ENodeList* list;

    switch (mode) {
        case CABIDestroyDelete:
        case CABIDestroySDelete:
            if (dealloc) {
                varg = 1;
            } else {
                varg = -1;
            }
            break;
        case CABIDestroyComplete:
            varg = -1;
            break;
        case CABIDestroyBase:
            varg = 0;
            break;
        default:
            CError_FATAL(2977);
    }

    expr = CExpr_NewENode(EFUNCCALL);
    expr->cost = 200;
    expr->rtype = &stvoid;
    expr->data.funccall.funcref = create_objectrefnode(dtor);
    if (nonvirtual) {
        expr->data.funccall.funcref->flags |= ENODE_FLAG_NONVIRTUAL;
    }
    expr->data.funccall.functype = TYPE_FUNC(dtor->type);
    dtor->flags |= OBJECT_USED;

    list = lalloc(sizeof(ENodeList));
    list->node = objexpr;
    expr->data.funccall.args = list;
    list->next = lalloc(sizeof(ENodeList));
    list = list->next;
    list->next = NULL;
    list->node = intconstnode(TYPE(&stsignedshort), varg);
    return expr;
}
