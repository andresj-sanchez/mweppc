#ifndef COMPILER_SWITCH_H
#define COMPILER_SWITCH_H

#include "compiler/common.h"
#include "compiler/enode.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SwitchCase SwitchCase;
typedef struct SwitchRange SwitchRange;
typedef struct SwitchInfo SwitchInfo;

typedef struct SwitchBounds {
    CInt64 low;
    CInt64 high; /* 0x10 */
    UInt32 unk18[3];
} SwitchBounds;

struct SwitchCase {
    struct SwitchCase* next;
    struct CLabel* label;
    union {
        CInt64 min;
        UInt32 unk8[8];
        SwitchBounds bounds;
    };
};

struct SwitchRange {
    struct SwitchRange* next;
    struct CLabel* label;
    ENode* expr;
    ENode* expr2;
};

struct SwitchInfo {
    struct SwitchCase* cases;
    struct SwitchRange* ranges;
    struct CLabel* defaultlabel;
    Type* exprtype;
};

extern ObjectList* switchtables;

extern void switchstatement(ENode* expr, struct SwitchInfo* info);
extern void dumpswitchtables(Object* funcobj);

#ifdef __cplusplus
}
#endif

#endif
