#include "compiler/common.h"

typedef struct LifetimeOperandRef {
    struct LifetimeOperandRef* next;
    void* a;
    void* b;
    void* c;
    void* d;
    UInt8 reserved[8];
    UInt8 kind;
} LifetimeOperandRef;

Boolean fn_005ADC70(LifetimeOperandRef* record, void* needle) {
    for (; record; record = record->next) {
        switch (record->kind) {
            case 1:
                if (record->a == needle) {
                    return TRUE;
                }
                break;
            case 2:
                if (record->a == needle) {
                    return TRUE;
                }
                if (record->b == needle) {
                    return TRUE;
                }
                break;
            case 3:
                if (record->a == needle) {
                    return TRUE;
                }
                break;
            case 4:
                if (record->a == needle) {
                    return TRUE;
                }
                break;
            case 5:
                if (record->a == needle) {
                    return TRUE;
                }
                break;
            case 6:
                if (record->a == needle) {
                    return TRUE;
                }
                if (record->b == needle) {
                    return TRUE;
                }
                if (record->c == needle) {
                    return TRUE;
                }
                if (record->d == needle) {
                    return TRUE;
                }
                break;
            case 17:
                if (record->a == needle) {
                    return TRUE;
                }
                break;
            case 7:
                if (record->a == needle) {
                    return TRUE;
                }
                break;
            case 8:
                if (record->a == needle) {
                    return TRUE;
                }
                if (record->b == needle) {
                    return TRUE;
                }
                break;
            case 9:
                if (record->a == needle) {
                    return TRUE;
                }
                break;
            case 10:
                if (record->a == needle) {
                    return TRUE;
                }
                break;
            case 12:
                if (record->a == needle) {
                    return TRUE;
                }
                if (record->c == needle) {
                    return TRUE;
                }
                break;
        }
    }
    return FALSE;
}
