
#include "runtime/terminate.h"

#pragma use_frame on
void unexpected(void) {
    uhandler();
}
#pragma use_frame off
