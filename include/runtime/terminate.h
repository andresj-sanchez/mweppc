#ifndef RUNTIME_TERMINATE_H
#define RUNTIME_TERMINATE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*exception_handler)(void);
exception_handler set_unexpected(exception_handler h);
exception_handler set_terminate(exception_handler h);
void terminate(void);

extern exception_handler thandler;
extern exception_handler uhandler;

#ifdef __cplusplus
}
#endif

#endif
