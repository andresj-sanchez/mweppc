#include "compiler/IroLoop.h"
#include "compiler/IroLinearForm.h"
#include "compiler/BitVector.h"
#include "compiler/CError.h"
#include "compiler/IroDump.h"
#include "compiler/IroAggregateAssignment.h"
#include "compiler/objects.h"
#include "compiler/types.h"
#include "compiler/IroPointerAnalysisADTs.h"

void IRO_DecomposeAssignments(void) {
    IROLinear* linear;
    void* a;
    void* b;
    void* c;

    for (linear = IRO_FirstLinear; linear; linear = linear->next) {
        if (fn_00591410(linear, &a, &b, &c)) {
            IRO_Dump(IRO_DecomposeMessage, linear->index);
            fn_00591210(linear, a, b, c);
        }
    }
    IRO_UpdateFlagsOnInts(IRO_FirstLinear);
}

void Bv_SetBit(UInt32 bit, BitVector* bv) {
    if ((bit >> 5) < bv->size) {
        bv->data[bit >> 5] |= 1 << (bit & 31);
    } else {
        CError_Internal(IRO_BitVectorFileName, 80);
    }
}

Boolean fn_00592B40(UInt32 bit, BitVector* bv) {
    return Bv_IsBitSet(bit, bv);
}

typedef struct IROAggregateIterator {
    Type* type;
    union {
        StructMember* member;
        ObjMemberVar* ivar;
    } node;
    ClassList* base;
} IROAggregateIterator;

void fn_005953C0(IROAggregateIterator* iter) {
    if (iter->type->type == TYPESTRUCT) {
        if (iter->node.member) {
            iter->node.member = iter->node.member->next;
        }
    } else if (!iter->base) {
        if (iter->node.ivar) {
            iter->node.ivar = iter->node.ivar->next;
        }
    } else {
        iter->base = iter->base->next;
    }
}

void fn_00595400(Type* type, IROAggregateIterator* iter) {
    iter->type = type;
    if (type->type == TYPESTRUCT) {
        iter->node.member = TYPE_STRUCT(type)->members;
    } else {
        iter->base = TYPE_CLASS(type)->bases;
        iter->node.ivar = TYPE_CLASS(type)->ivars;
    }
}
