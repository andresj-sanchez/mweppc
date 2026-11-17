#include <stdlib.h>
#include "runtime/terminate.h"

#pragma use_frame on
void NMWException_dthandler(void) {
    abort();
}

void NMWException_duhandler(void) {
    terminate();
}
#pragma use_frame off
