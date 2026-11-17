#include <stdlib.h>
#include "compiler/common.h"
#include "compiler/win32.h"
#include <errno.h>
#include "msl/signal.h"
#include "msl/startup.win32.h"

int __stdcall __handler(unsigned long CtrlType) {
    __signal_func_ptr ctrl_action;
    __signal_func_ptr* pctrl_action;

    int __signal;

    if (CtrlType == CTRL_C_EVENT) {
        ctrl_action = *(pctrl_action = &ctrlc);
        __signal = SIGINT;
    } else {
        ctrl_action = *(pctrl_action = &ctrlbreak);
        __signal = SIGBREAK;
    }

    if (ctrl_action == SIG_DFL) {
        return 0;
    }

    if (ctrl_action != SIG_IGN) {
        *pctrl_action = SIG_DFL;
        (*ctrl_action)(__signal);
    }

    return 1;
}

void __check_console_handler(void) {
    if (!ConsoleCtrlHandler) {
        if (SetConsoleCtrlHandler(__handler, TRUE)) {
            ConsoleCtrlHandler = 1;
        } else {
            __set_errno(GetLastError());
        }
    }
}

__signal_func_ptr signal(int __signaltype, __signal_func_ptr signal_func) {
    __signal_func_ptr old_signal_func;

    if (__signaltype < 1 || __signaltype > __msl_signal_max) {
        errno = ESIGPARM;
        return SIG_ERR;
    }

    old_signal_func = signal_funcs[__signaltype - 1];

    if (__signaltype == SIGINT) {
        __check_console_handler();
        ctrlc = signal_func;
    } else if (__signaltype == SIGBREAK) {
        __check_console_handler();
        ctrlbreak = signal_func;
    }

    signal_funcs[__signaltype - 1] = signal_func;

    return old_signal_func;
}
