#include "compiler/common.h"
#include <stdarg.h>
#include <stdio.h>

Boolean IRO_Log;
FILE* DumpFile;

void IRO_Dump(const char* format, ...) {
    va_list va;

    if (!IRO_Log) {
        return;
    }

    va_start(va, format);
    vfprintf(DumpFile, format, va);
    va_end(va);
}

void IRO_CleanupDump(void) {
    if (DumpFile) {
        fclose(DumpFile);
        DumpFile = NULL;
    }
}
