#include "compiler/common.h"
#include "compiler/cos.h"
#include "driver/CWParserPluginsContext.h"
#include "driver/COSToolsCLT.h"
#include "driver/ELFImportFileIO.h"
#include "driver/ELFImportReader.h"

void fn_00423E90(SInt16 error) {
    char buffer[256];
    COS_GetString(buffer, 0x38f, error);
    CWReportMessage(ELFReader_context_6001FC, 0, buffer, 0, 2, 0);
}

int fn_00423ED0(const CWFileSpec* spec, CWMemHandle* outHandle, SInt32* outSize) {
    CWMemHandle handle;
    SInt32 size;
    HANDLE refNum;
    void* buffer;
    UInt32 err;

    err = COS_FileOpen(spec, &refNum);
    if (err != 0) {
        return 2;
    }
    COS_FileGetSize(refNum, &size);
    if (CWAllocMemHandle(ELFReader_context_6001FC, size, TRUE, &handle) < 0) {
        COS_FileClose(refNum);
        return 1;
    }
    CWLockMemHandle(ELFReader_context_6001FC, handle, FALSE, &buffer);
    err = COS_FileRead(refNum, buffer, size);
    CWUnlockMemHandle(ELFReader_context_6001FC, handle);
    COS_FileClose(refNum);
    if (err == 0) {
        *outHandle = handle;
        *outSize = size;
        return 0;
    }
    CWFreeMemHandle(ELFReader_context_6001FC, handle);
    return 3;
}
