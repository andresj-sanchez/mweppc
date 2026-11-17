#define _MSL_SIZE_T_TYPE unsigned int
#include <stdio.h>
#include <stdlib.h>
#include "msl/buffer_io.h"
#include "msl/critical_regions.h"

void __convert_from_newlines(unsigned char* p, size_t* n) {
#pragma unused(p, n)
}

void __prep_buffer(FILE* file) {
    file->buffer_ptr = file->buffer;
    file->buffer_len = file->buffer_size;
    file->buffer_len -= file->position & file->buffer_alignment;
    file->buffer_pos = file->position;
}

int __flush_buffer(FILE* file, size_t* bytes_flushed) {
    size_t buffer_len;
    int ioresult;

    buffer_len = file->buffer_ptr - file->buffer;

    if (buffer_len) {
        file->buffer_len = buffer_len;

        if (!file->mode.binary_io) {
            __convert_from_newlines(file->buffer, (size_t*)&file->buffer_len);
        }

        ioresult = (*file->write_proc)(
            file->handle, file->buffer, (size_t*)&file->buffer_len, file->ref_con
        );

        if (bytes_flushed) {
            *bytes_flushed = file->buffer_len;
        }

        if (ioresult) {
            return ioresult;
        }

        file->position += file->buffer_len;
    }

    __prep_buffer(file);

    return __no_io_error;
}

#pragma avoid_ebp on

int setvbuf(FILE* restrict file, char* restrict buff, int mode, size_t size) {
    int kind = file->mode.file_kind;
    int crtrgn;

    if (mode == _IONBF) {
        fflush(file);
    }

    if (file->state.io_state != __neutral || kind == __closed_file) {
        return -1;
    }

    if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF) {
        return -1;
    }

    if (file->buffer && file->state.free_buffer) {
        free(file->buffer);
    }

    if (file == stdin) {
        crtrgn = stdin_access;
    } else if (file == stdout) {
        crtrgn = stdout_access;
    } else if (file == stderr) {
        crtrgn = stderr_access;
    } else {
        crtrgn = files_access;
    }

    __begin_critical_region(crtrgn);

    file->mode.buffer_mode = mode;
    file->state.free_buffer = 0;
    file->buffer = &file->char_buffer;
    file->buffer_ptr = &file->char_buffer;
    file->buffer_size = 1;
    file->buffer_len = 0;
    file->buffer_alignment = 0;

    if (mode == _IONBF || size < 1) {
        *(file->buffer_ptr) = '\0';
        __end_critical_region(crtrgn);
        return 0;
    }

    if (!buff) {
        if (!(buff = (char*)malloc(size))) {
            __end_critical_region(crtrgn);
            return -1;
        }

        file->state.free_buffer = 1;
    }

    file->buffer = (unsigned char*)buff;
    file->buffer_ptr = file->buffer;
    file->buffer_size = size;

    file->buffer_alignment = 0;

    __end_critical_region(crtrgn);

    return 0;
}

#pragma avoid_ebp reset
