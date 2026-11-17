#include <stdio.h>
#include "compiler/common.h"
#include <stdlib.h>
#include "compiler/win32.h"
#include "msl/fcntl.h"
#include "msl/file_io.h"
#include "msl/crtl.h"
#include "msl/startup.win32.h"
#include <errno.h>

static temp_file_info temp_info[FOPEN_MAX];

temp_file_info* find_temp_info(__file_handle inHandle) {
    int count;
    temp_file_info* info;
    for (count = 0, info = temp_info; count < FOPEN_MAX; count++, info++) {
        if (info->mFileHandle == inHandle) {
            return info;
        }
    }
    return NULL;
}

int __msl_GetHandle(void) {
    int i;
    for (i = 0; i < NUM_HANDLES; i++) {
        if (_HandPtr == NUM_HANDLES) {
            _HandPtr = 3;
        }
        if (_HandleTable[_HandPtr] == NULL) {
            return _HandPtr++;
        }
        _HandPtr++;
    }
    return -1;
}

long __msl_lseek(int fildes, long offset, int whence) {
    HANDLE h;
    DWORD method;
    if (fildes >= NUM_HANDLES || !_HandleTable[fildes]) {
        errno = EBADF;
        return -1;
    }
    h = _HandleTable[fildes]->handle;
    switch (whence) {
        case SEEK_SET:
            method = FILE_BEGIN;
            break;
        case SEEK_CUR:
            method = FILE_CURRENT;
            break;
        case SEEK_END:
            method = FILE_END;
            break;
    }
    return SetFilePointer(h, offset, NULL, method);
}

int __msl_write(int fildes, const void* buf, size_t count) {
    DWORD wrote;
    HANDLE h;
    int t;
    char* bp = NULL;
    int status;
    if (fildes >= NUM_HANDLES || !_HandleTable[fildes]) {
        errno = EBADF;
        return -1;
    }
    h = _HandleTable[fildes]->handle;
    t = _HandleTable[fildes]->translate;
    if (t) {
        int i, n;
        char* cp;
        n = 0;
        for (i = 0; i < count; i++) {
            if (((const char*)buf)[i] == '\n') {
                n++;
            }
        }
        bp = (char*)malloc(count + n);
        cp = bp;
        for (i = 0; i < count; i++) {
            if (((const char*)buf)[i] == '\n') {
                *cp++ = '\r';
            }
            *cp++ = ((const char*)buf)[i];
        }
        count += n;
        buf = bp;
    }
    if (_HandleTable[fildes]->append) {
        __msl_lseek(fildes, 0, SEEK_END);
    }
    status = WriteFile(h, buf, (DWORD)count, &wrote, NULL);
    if (bp) {
        free(bp);
    }
    if (status) {
        return wrote;
    }
    __set_errno(GetLastError());
    return 0;
}

int __msl_close(int fildes) {
    HANDLE h;
    if (fildes >= NUM_HANDLES || !_HandleTable[fildes]) {
        return -1;
    }
    h = _HandleTable[fildes]->handle;
    if (CloseHandle(h)) {
        free(_HandleTable[fildes]);
        _HandleTable[fildes] = NULL;
        return 0;
    }
    __set_errno(GetLastError());
    return -1;
}

int __msl_read(int fildes, void* buf, size_t count) {
    DWORD bytesRead;
    HANDLE h;
    int t;
    int readResult;
    if (fildes >= NUM_HANDLES || !_HandleTable[fildes]) {
        errno = EBADF;
        return -1;
    }
    h = _HandleTable[fildes]->handle;
    t = _HandleTable[fildes]->translate;
    readResult = ReadFile(h, buf, (DWORD)count, &bytesRead, NULL);
    if (readResult) {
        if (bytesRead && t) {
            char* cp;
            char* op;
            int i, n;
            cp = op = buf;
            n = 0;
            for (i = 0; i < bytesRead; i++) {
                if (cp[0] == '\r' && i < bytesRead - 1 && cp[1] == '\n') {
                    cp++;
                    n++;
                } else {
                    *op++ = *cp++;
                }
            }
            if (bytesRead == count && bytesRead > 1 && ((char*)buf)[bytesRead - 1] == '\r') {
                __msl_lseek(fildes, -1, SEEK_CUR);
                n++;
            }
            bytesRead -= n;
        }
        return bytesRead;
    }
    __set_errno(GetLastError());
    return 0;
}

int __msl_open(const char* path, int flags) {
    DWORD access;
    DWORD share;
    DWORD create;
    int h;
    if ((((flags & O_RDONLY) != 0) + ((flags & O_RDWR) != 0) + ((flags & O_WRONLY) != 0)) != 1) {
        return -1;
    }
    if ((flags & O_RDONLY) && (flags & O_TRUNC)) {
        return -1;
    }
    if ((flags & O_EXCL) && !(flags & O_CREAT)) {
        return -1;
    }
    h = __msl_GetHandle();
    if (h == -1) {
        return -1;
    }
    _HandleTable[h] = (FileStruct*)malloc(sizeof(FileStruct));
    if (!_HandleTable[h]) {
        return -1;
    }
    _HandleTable[h]->translate = (flags & O_BINARY) == 0;
    _HandleTable[h]->append = 0;
    share = FILE_SHARE_READ;
    if (flags & O_RDONLY) {
        access = GENERIC_READ;
    } else if (flags & O_WRONLY) {
        access = GENERIC_WRITE;
    } else {
        access = GENERIC_READ | GENERIC_WRITE;
    }
    if (flags & O_CREAT) {
        create = OPEN_ALWAYS;
        if (flags & O_TRUNC) {
            create = CREATE_ALWAYS;
        }
        if (flags & O_EXCL) {
            create = CREATE_NEW;
        }
    } else if (flags & O_TRUNC) {
        create = TRUNCATE_EXISTING;
    } else {
        create = OPEN_EXISTING;
    }
    _HandleTable[h]->handle = CreateFileA(path, access, share, NULL, create, 0, NULL);
    if (_HandleTable[h]->handle != INVALID_HANDLE_VALUE) {
        if (flags & O_APPEND) {
            __msl_lseek(h, 0, SEEK_END);
            _HandleTable[h]->append = 1;
        }
        return h;
    }
    free(_HandleTable[h]);
    _HandleTable[h] = NULL;
    __set_errno(GetLastError());
    return -1;
}

#pragma register_coloring off

int __open_file(const char* name, __file_modes mode, __file_handle* handle) {
    int open_mode = mode.open_mode;
    int io_mode = mode.io_mode;
    int h;
    int flag = 0;

    if ((io_mode & 3) == __read) {
        flag |= O_RDONLY;
    }
    if ((io_mode & 3) == __write) {
        flag |= O_WRONLY;
    }
    if ((io_mode & 3) == __read_write) {
        flag |= O_RDWR;
    }
    if (io_mode & __append) {
        flag |= O_APPEND;
    }

    if (mode.binary_io) {
        flag |= O_BINARY;
    }

    if (mode.open_mode == __create_if_necessary) {
        flag |= O_CREAT;
    }
    if (mode.open_mode == __create_or_truncate) {
        flag |= (O_CREAT | O_TRUNC);
    }

    h = __msl_open(name, flag);
    if (h == -1) {
        return __io_error;
    } else {
        *handle = h;
        return __no_io_error;
    }
}

#pragma register_coloring on

int __read_file(__file_handle handle, unsigned char* buffer, size_t* count, __ref_con ref_con) {
    fflush(stdout);
    *count = __msl_read(handle, (char*)buffer, *count);
    return *count ? __no_io_error : __io_EOF;
}

int __write_file(__file_handle handle, unsigned char* buffer, size_t* count, __ref_con ref_con) {
    size_t n;
    n = __msl_write(handle, (char*)buffer, *count);
    if (n >= *count) {
        *count = n;
        return __no_io_error;
    } else {
        *count = n;
        return __io_error;
    }
}

int __position_file(__file_handle handle, fpos_t* position, int mode, __ref_con ref_con) {
    *position = __msl_lseek(handle, *position, mode);
    return *position == -1 ? __io_error : __no_io_error;
}

int __close_file(__file_handle handle) {
    temp_file_info* theTempInfo;

    if (__msl_close(handle) == -1) {
        return __io_error;
    }

    if (handle && (theTempInfo = find_temp_info(handle)) != NULL) {
        int theError = __delete_file((char*)theTempInfo->mFileName);

        free(theTempInfo->mFileName);
        theTempInfo->mFileHandle = 0;

        return theError;
    } else {
        return __no_io_error;
    }
}

int __delete_file(const char* name) {
    return DeleteFileA((LPCSTR)name) ? __no_io_error : __io_error;
}
