#define _MSL_SIZE_T_TYPE unsigned int
#include "msl/file_struc.h"
#include "msl/critical_regions.h"
#include "msl/buffer_io.h"
#include "msl/misc_io.h"
#include <string.h>
#include "msl/file_io_helpers.h"
#include "msl/wchar_io.h"

#include "msl/stdio_api.h"

size_t fwrite(const void* ptr, size_t size, size_t count, FILE* file) {
    int region;
    CRITICAL_SECTION* critical;
    size_t result;

    if (file == stdout) {
        region = stdout_access;
    } else if (file == stderr) {
        region = stderr_access;
    } else {
        region = files_access;
    }

    critical = &__cs[region];
    EnterCriticalSection(critical);
    result = __fwrite(ptr, size, count, file);
    LeaveCriticalSection(critical);
    return result;
}

size_t __fwrite(const void* ptr, size_t size, size_t count, FILE* file) {
    const unsigned char* buffer;
    size_t bytes_to_go;
    size_t bytes_written;
    size_t bytes_this_time;
    int always_buffer;
    int ioresult;
    unsigned char* saved_buffer;
    size_t saved_size;
    void* newline;

    if (fwide(file, 0) == 0) {
        fwide(file, -1);
    }
    bytes_to_go = size * count;
    if (!bytes_to_go || file->state.error || file->mode.file_kind == __closed_file) {
        return 0;
    }
    if (file->mode.file_kind == __console_file) {
        __stdio_atexit();
    }
    always_buffer = !file->mode.binary_io ||
        file->mode.buffer_mode == _IOFBF ||
        file->mode.buffer_mode == _IOLBF;
    if (file->state.io_state == __neutral) {
        if (file->mode.io_mode & __write) {
            if (file->mode.io_mode & __append) {
                if (fseek(file, 0, SEEK_END)) {
                    return 0;
                }
            }
            file->state.io_state = __writing;
            __prep_buffer(file);
        }
    }
    if (file->state.io_state != __writing) {
        file->state.error = 1;
        file->buffer_len = 0;
        return 0;
    }
    buffer = (const unsigned char*)ptr;
    bytes_written = 0;
    if (bytes_to_go != 0 && (file->buffer_ptr != file->buffer || always_buffer)) {
        file->buffer_len = file->buffer_size - (file->buffer_ptr - file->buffer);
        do {
            newline = 0;
            bytes_this_time = file->buffer_len;
            if (bytes_this_time > bytes_to_go) {
                bytes_this_time = bytes_to_go;
            }
            if (file->mode.buffer_mode == _IOLBF && bytes_this_time != 0) {
                newline = __memrchr(buffer, '\n', bytes_this_time);
                if (newline) {
                    bytes_this_time = (unsigned char*)newline + 1 - buffer;
                }
            }
            if (bytes_this_time != 0) {
                memcpy(file->buffer_ptr, buffer, bytes_this_time);
                buffer += bytes_this_time;
                bytes_written += bytes_this_time;
                bytes_to_go -= bytes_this_time;
                file->buffer_ptr += bytes_this_time;
                file->buffer_len -= bytes_this_time;
            }
            if (file->buffer_len == 0 || newline || file->mode.buffer_mode == _IONBF) {
                if (__flush_buffer(file, 0) != __no_io_error) {
                    file->state.error = 1;
                    bytes_to_go = 0;
                    file->buffer_len = 0;
                    break;
                }
            }
        } while (bytes_to_go && always_buffer);
    }
    if (bytes_to_go && !always_buffer) {
        saved_buffer = file->buffer;
        saved_size = file->buffer_size;
        file->buffer = (unsigned char*)buffer;
        file->buffer_size = bytes_to_go;
        file->buffer_ptr = (unsigned char*)buffer + bytes_to_go;
        ioresult = __flush_buffer(file, &bytes_this_time);
        if (ioresult == __no_io_error) {
            bytes_written += bytes_this_time;
        } else {
            file->state.error = 1;
            file->buffer_len = 0;
        }
        file->buffer = saved_buffer;
        file->buffer_size = saved_size;
        __prep_buffer(file);
        file->buffer_len = 0;
    }
    if (file->mode.buffer_mode != _IOFBF) {
        file->buffer_len = 0;
    }
    return bytes_written / size;
}
