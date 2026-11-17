#ifndef COMPILER_CMACHINESTATE_H
#define COMPILER_CMACHINESTATE_H

#include "compiler/common.h"

typedef struct BackEndOptionsSnapshot BackEndOptionsSnapshot;
struct BackEndOptionsSnapshot {
    UInt32 optionWords[0x53];
    UInt8 reserved[0x94];
    UInt8 contextFlag;
};

#endif
