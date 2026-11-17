#include "compiler/common.h"
#include <stdlib.h>
#include "compiler/win32.h"
#include "msl/abort_exit.h"
#include "msl/critical_regions.h"
#include "msl/signal.h"
#include "msl/startup.win32.h"

int __aborting = 0;

void (*__stdio_exit)(void) = 0;
void (*__console_exit)(void) = 0;

void abort(void) {
    raise(SIGABRT);

    __aborting = 1;

    exit(EXIT_FAILURE);
}

int atexit(void (*func)(void)) {
    int result;

    __begin_critical_region(atexit_funcs_access);

    result = __register_atexit(func);

    __end_critical_region(atexit_funcs_access);

    return result;
}

void exit(int status) {
    if (!__aborting) {
        __destroy_global_chain();
    }

    __exit(status);
}

void __exit(int status) {
    _CleanUpMSL();

    if (__console_exit) {
        (*__console_exit)();
        __console_exit = 0;
    }

    ExitProcess(status);
}
