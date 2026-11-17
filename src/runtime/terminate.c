
#include "runtime/terminate.h"

exception_handler set_unexpected(exception_handler h)
{
    exception_handler old = uhandler;
    uhandler = h;
    return old;
}

void terminate(void)
{
    thandler();
}

exception_handler set_terminate(exception_handler h)
{
    exception_handler old = thandler;
    thandler = h;
    return old;
}
