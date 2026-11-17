#include "compiler/win32.h"
#include <stdio.h>
#include <stdlib.h>

char assert_format_with_function_5DF6B4[] =
    "Assertion (%s) failed in \"%s\", function \"%s\", line %d\n\n"
    "Choose 'Abort' to end the program, 'Retry' to debug, or 'Ignore' to continue.\n";
char assert_title_5DF73C[] = "Assertion Failure";
char assert_format_without_function_5DF750[] =
    "Assertion (%s) failed in \"%s\", line %d\n\n"
    "Choose 'Abort' to end the program, 'Retry' to debug, or 'Ignore' to continue.\n";

extern void __stdcall DebugBreak(void);

int fn_0041E940(const char* title, const char* message) {
    int response = MessageBoxA(0, message, title, 0x10032);

    if (response == 4) {
        DebugBreak();
        return 0;
    }
    if (response == 5) {
        return 0;
    }
    return 1;
}

void __msl_assertion_failed(const char* condition, const char* filename, const char* function, int line) {
    char message[4096];

    if (function) {
        snprintf(message, sizeof(message), assert_format_with_function_5DF6B4,
                 condition, filename, function, line);
    } else {
        snprintf(message, sizeof(message), assert_format_without_function_5DF750,
                 condition, filename, line);
    }
    if (fn_0041E940(assert_title_5DF73C, message)) {
        abort();
    }
}
