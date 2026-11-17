#include "compiler/CPrep.h"
#include "compiler/MIPSError.h"
#include "compiler/CError.h"
#include "compiler/CParser.h"
#include "compiler/InlineAsm.h"
#include "driver/COSToolsCLT.h"
#include <stdarg.h>
#include <setjmp.h>

static void MIPSError_GetErrorString(char* str, short code) {
    CError_ASSERT(45, code >= 1 && code < MIPSERR_MAX);

    COS_GetString(str, 10001, code + 500);
}

static void MIPSError_VAErrorMessage(
    int code, va_list list, Boolean isTerminal, Boolean isWarning
) {
    char format[256];

    MIPSError_GetErrorString(format, code);
    CError_ErrorMessageVA(code + 36000, format, list, isTerminal, isWarning);
}

void MIPSError_Error(int code, ...) {
    va_list list;

    if (trychain) {
        longjmp(trychain->jmpbuf, 1);
    }

    va_start(list, code);
    MIPSError_VAErrorMessage(code, list, FALSE, FALSE);
    va_end(list);

    if (cprep_in_assembler) {
        AssemblerError();
    }
}

void MIPSError_Warning(int code, ...) {
    va_list list;

    if (trychain || copts.suppress_warnings) {
        return;
    }

    va_start(list, code);
    MIPSError_VAErrorMessage(code, list, FALSE, TRUE);
    va_end(list);
}
