#include "compiler/IroLoop.h"

typedef struct IROLoopNodeRef {
    IROLinear* node;
    struct IROLoopNodeRef* next;
} IROLoopNodeRef;

Boolean IRO_IsLinearNodeInLoopTest(IROLoop* loop, IROLinear* node) {
    IROLoopNodeRef* ref = (IROLoopNodeRef*)loop->unk28;

    while (ref) {
        if (ref->node == node) {
            return TRUE;
        }
        ref = ref->next;
    }
    return FALSE;
}

Boolean IRO_IsLinearNodeInInductionUpdate(IROLoop* loop, IROLinear* node) {
    IROLoopNodeRef* ref = (IROLoopNodeRef*)loop->unk30;

    while (ref) {
        if (ref->node == node) {
            return TRUE;
        }
        ref = ref->next;
    }
    return FALSE;
}
