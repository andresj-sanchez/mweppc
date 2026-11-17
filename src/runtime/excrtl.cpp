#include "runtime/excrtl.h"
#include "msl/runinit.h"
#include "msl/threadlocaldata.h"
#include "runtime/ExceptionX86.h"
#include "runtime/terminate.h"

extern "C" {


#pragma use_frame on
void SEH_InitializeCPUInstructionSet(void) {
    SEH_CPUInstructionSet = __detect_cpu_instruction_set();
}
#pragma use_frame off

int SEH_UnwindFilter(ExceptionRecordWin32* record, SEHFrame* frame, ContextWin32* context, SEHFrame** current) {
    if (!(record->ExceptionFlags & 6)) {
        return 1;
    }
    *current = frame;
    return 3;
}

}
