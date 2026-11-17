#include <stdio.h>
#include "msl/ansi_files.h"
#include "msl/misc_io.h"
#include "msl/abort_exit.h"

void clearerr(FILE* file) {
    file->state.eof = 0;
    file->state.error = 0;
}

void __stdio_atexit(void) {
    __stdio_exit = __close_all;
}
