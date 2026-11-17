#include <stdlib.h>
#include "compiler/common.h"
#include <errno.h>
#include "msl/critical_regions.h"
#include "msl/signal.h"

int raise(int signal) {
    int index;
    CRITICAL_SECTION* cs;
    long signaltype = signal;
    __signal_func_ptr signal_func;

    if (signaltype < 1 || signaltype > __msl_signal_max) {
        return -1;
    }

    cs = &__cs[signal_funcs_access];
    EnterCriticalSection(cs);

    index = signaltype - 1;
    signal_func = signal_funcs[index];

    if (signal_func != SIG_IGN) {
        signal_funcs[index] = SIG_DFL;
    }

    LeaveCriticalSection(cs);

    if (signal_func == SIG_IGN || (signal_func == SIG_DFL && signaltype == SIGABRT)) {
        return 0;
    }

    if (signal_func == SIG_DFL) {
        exit(0);
    }

    (*signal_func)(signaltype);

    return 0;
}
