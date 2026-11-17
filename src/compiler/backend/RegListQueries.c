#include "compiler/RegListQueries.h"
#include "compiler/objects.h"
#include "compiler/types.h"

Boolean fn_004D1D10(void* object) {
    ObjectList* scan;
    for (scan = ig_reglist_a; scan; scan = scan->next) {
        if (scan->object == object && scan->object == object) {
            return TRUE;
        }
    }
    return FALSE;
}

Boolean fn_004D1D40(Object* object) {
    ObjectList* scan;
    if (!object) {
        return FALSE;
    }
    if (object->datatype != 1) {
        return FALSE;
    }
    for (scan = ig_reglist_b; scan; scan = scan->next) {
        if (scan->object == object) {
            return CABI_HasSideEffects(object->type);
        }
    }
    return FALSE;
}

Boolean CABI_ExprHasSideEffects(Type* expr) {
    if (expr && expr->type == TYPEPOINTER) {
        if (CABI_HasSideEffects(TYPE_POINTER(expr)->target)) {
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

#pragma opt_repositioncode off
Boolean CABI_HasSideEffects(Type* input) {
    TypeClass* object;
    ObjMemberVar* ptr;
    ClassList* ref;

    object = (TypeClass*)input;
    if (!object) {
        goto null_false;
    }
    if (object->type != TYPECLASS || object->vtable) {
        goto flags_false;
    }
    ptr = object->ivars;
    if (!ptr) {
        goto check_ref;
    }
    if (ptr->next == 0) {
        goto check_ptr;
    }
    return FALSE;
null_false:
    return FALSE;
check_ref:
    ref = object->bases;
    if (!ref) {
        goto empty_false;
    }
    if (ref->base && !ref->next) {
        return CABI_HasSideEffects((Type*)ref->base);
    }
    return FALSE;
empty_false:
    return FALSE;
check_ptr:
    if (ptr && (ptr->type->type != TYPEINT || ptr->type->size != 16)) {
        return FALSE;
    }
    return TRUE;
flags_false:
    return FALSE;
}

#pragma opt_repositioncode on
