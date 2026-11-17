#include "msl/alloc.h"
#include <stdlib.h>
#include "compiler/common.h"
#include "compiler/win32.h"
#include "msl/abort_exit.h"
#include "msl/critical_regions.h"
#include "msl/crtl.h"
#include "msl/file_struc.h"
#include "msl/startup.win32.h"
#include "msl/threadlocaldata.h"

FileStruct* _HandleTable[NUM_HANDLES];

int _HandPtr;

int* __get_MSL_init_count(void) {
    static int __MSL_init_count = 0;

    return &__MSL_init_count;
}

static void _InitStdHandles(void) {
    HANDLE handle;
    HANDLE msl_handle;

    _HandleTable[0] = (FileStruct*)malloc(sizeof(FileStruct));
    if (_HandleTable[0]) {
        handle = GetStdHandle(STD_INPUT_HANDLE);

        if (__dup_core(handle, &msl_handle) != NO_ERROR) {
            _HandleTable[0]->handle = INVALID_HANDLE_VALUE;
            stdin->mode.file_kind = __closed_file;
        } else {
            _HandleTable[0]->handle = msl_handle;
        }

        _HandleTable[0]->translate = 1;
        _HandleTable[0]->append = 0;
    }

    _HandleTable[1] = (FileStruct*)malloc(sizeof(FileStruct));
    if (_HandleTable[1]) {
        handle = GetStdHandle(STD_OUTPUT_HANDLE);

        if (__dup_core(handle, &msl_handle) != NO_ERROR) {
            _HandleTable[1]->handle = INVALID_HANDLE_VALUE;
            stdout->mode.file_kind = __closed_file;
        } else {
            _HandleTable[1]->handle = msl_handle;
        }

        _HandleTable[1]->translate = 1;
        _HandleTable[1]->append = 0;
    }

    _HandleTable[2] = (FileStruct*)malloc(sizeof(FileStruct));
    if (_HandleTable[2]) {
        handle = GetStdHandle(STD_ERROR_HANDLE);

        if (__dup_core(handle, &msl_handle) != NO_ERROR) {
            _HandleTable[2]->handle = INVALID_HANDLE_VALUE;
            stderr->mode.file_kind = __closed_file;
        } else {
            _HandleTable[2]->handle = msl_handle;
        }

        _HandleTable[2]->translate = 1;
        _HandleTable[2]->append = 0;
    }

    _HandPtr = 3;
}

#pragma avoid_ebp on

int _CRTStartup(void) {
    if ((*__get_MSL_init_count())++) {
        return 1;
    }

    __init_critical_regions();

    if (!_InitializeMainThreadData()) {
        return 0;
    }

    _InitStdHandles();

    return 1;
}

#pragma avoid_ebp reset
#pragma register_coloring off

void _CleanUpMSL(void) {
    if (--(*__get_MSL_init_count()) > 0) {
        return;
    }

    __destroy_global_chain();

    if (__stdio_exit) {
        (*__stdio_exit)();
        __stdio_exit = 0;
    }

    if (__console_exit) {
        (*__console_exit)();
        __console_exit = 0;
    }

    _DisposeAllThreadData();

    __kill_critical_regions();

    __malloc_free_all();

    _DisposeThreadDataIndex();
}

#pragma register_coloring on
