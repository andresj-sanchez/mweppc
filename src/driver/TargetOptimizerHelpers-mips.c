#include "driver/TargetOptimizerHelpers-mips.h"
#include "driver/StringUtils.h"

int TargetSetOptFlags(short option, unsigned char value) {
    switch (option) {
        case 'In':
            inlineIntrinsicsEnabled = value;
            return 1;
        default:
            return 0;
    }
}

void TargetDisplayOptimizationOptions(Ptr* output) {
#pragma optimize_for_size on
    const char* setting;
    unsigned char flag = *(volatile unsigned char*)&inlineIntrinsicsEnabled;
    if (flag) {
        if (flag == 1) {
            setting = "on";
        } else {
            setting = "off";
        }
        HPrintF(output, "\t- inline intrinsics %s\n", setting);
    }
#pragma optimize_for_size off
}

void Utils_Empty(void) {}
