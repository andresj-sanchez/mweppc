#include "compiler/IroPropagate.h"
#include "compiler/CompilerTools.h"
#include "compiler/objects.h"
#include "compiler/BitVector.h"
#include "compiler/IroLoop.h"
#include "compiler/IroVars.h"
#include "compiler/enode.h"
#include "compiler/IroFlowgraph.h"
#include "compiler/IroNonRegLoopAccesses.h"
#include "compiler/IroCSE.h"
#include "compiler/IroDependsKills.h"
#include "compiler/IroVars.h"

typedef struct NonRegCandidateList {
    NonRegCandidate* head;
    NonRegCandidate* tail;
} NonRegCandidateList;

void fn_005B4C20(
    void* linear, void* address, Object* obj, char flag1, char flag2, NonRegCandidateList* list
) {
    NonRegCandidate* record;
    NonRegElement* element;
    NonRegElement* existing;

    if (!(char)IRO_IsRegable(obj)) {
        if (!list->head) {
            record = IRO_oalloc2(0x18);
            list->head = record;
        } else {
            record = IRO_oalloc2(0x18);
            list->tail->next = record;
            record = list->tail->next;
        }
        list->tail = record;
        list->tail->kind = 2;
        list->tail->linear = linear;
        list->tail->address = address;
        list->tail->object = obj;
        list->tail->next = NULL;
        list->tail->elements = NULL;

        if (obj->varptr) {
            obj->varptr->candidate = list->tail;
        }

        element = IRO_oalloc2(8);
        element->linear = linear;
        element->next = NULL;
        existing = list->tail->elements;
        if (existing) {
            element->next = existing;
            list->tail->elements = element;
        } else {
            list->tail->elements = element;
        }

        if (!flag1 && flag2) {
            list->tail->kind = 1;
        } else if (flag1 || flag2) {
            list->tail->kind = 0;
        }
    }
}

void fn_005B4B30(IROLinear* linear, BitVector* uses, NonRegCandidateList* candidates) {
    VarRecord* var;
    Boolean used;
    UInt32 bit;

    if (((linear->type == IROLinearOp1Arg) && (linear->nodetype == EINDIRECT)) ||
        (linear->type == IROLinearFunccall) ||
        (IRO_NonRegAccessCallback && IRO_NonRegAccessCallback(linear, 0)) ||
        (linear->type == IROLinearAsm)) {
        Bv_Clear(uses);
        IRO_GetUsesInExpr(linear, FALSE, FALSE, FALSE);
        Bv_Copy(IRO_Depends, uses);
        if (!Bv_IsEmpty(uses)) {
            for (var = IRO_FirstVar; var; var = var->next) {
                bit = var->index;
                used = (bit >> 5) < uses->size && (uses->data[bit >> 5] & (1 << (bit & 31)));
                if (used) {
                    if (var->candidate) {
                        var->candidate->kind = 1;
                    } else {
                        fn_005B4C20(linear, NULL, var->object, 0, 1, candidates);
                    }
                }
            }
        }
    }
}
