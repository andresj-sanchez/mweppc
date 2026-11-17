#include <stdio.h>
#include "compiler/common.h"
#include <stdlib.h>
#include <string.h>
#include "msl/file_struc.h"
#include "msl/ansi_files.h"
#include "msl/file_io_helpers.h"
#include "msl/stdio_api.h"
#include "msl/file_io.h"
#include "msl/buffer_io.h"
#include "msl/critical_regions.h"

FILE* __find_unopened_file(void) {
    FILE* p = __files[2].next_file_struct;
    FILE* plast;

    while (p != NULL) {
        if (p->mode.file_kind == __closed_file)
            return p;
        else {
            plast = p;
            p = p->next_file_struct;
        }
    }

    if ((p = malloc(sizeof(FILE))) != NULL) {
        memset(p, 0, sizeof(FILE));
        p->is_dynamically_allocated = 1;
        plast->next_file_struct = p;
        return p;
    }
    return NULL;
}

void __init_file(FILE* file, __file_modes mode, char* buff, size_t size) {
    file->handle = 0;
    file->mode = mode;
    file->state.io_state = __neutral;
    file->state.free_buffer = 0;
    file->state.eof = 0;
    file->state.error = 0;
    file->position = 0;

    if (size)
        setvbuf(file, buff, _IOFBF, size);
    else
        setvbuf(file, NULL, _IONBF, 0);

    file->buffer_ptr = file->buffer;
    file->buffer_len = 0;

    if (file->mode.file_kind == __disk_file) {
        file->position_proc = __position_file;
        file->read_proc = __read_file;
        file->write_proc = __write_file;
        file->close_proc = __close_file;
    }
    file->ref_con = 0;
}

void __close_all(void) {
    FILE* p = &__files[0];
    FILE* plast;

    __begin_critical_region(files_access);
    while (p != NULL) {
        if (p->mode.file_kind != __closed_file)
            fclose(p);

        plast = p;
        p = p->next_file_struct;
        if (plast->is_dynamically_allocated)
            free(plast);
        else {
            plast->mode.file_kind = __unavailable_file;
            if (p != NULL && p->is_dynamically_allocated)
                plast->next_file_struct = NULL;
        }
    }
    __end_critical_region(files_access);
}

int __flush_all(void) {
    int result = 0;
    FILE* p = &__files[0];
    while (p != NULL) {
        if (p->mode.file_kind != __closed_file)
            if (fflush(p))
                result = EOF;
        p = p->next_file_struct;
    }
    return result;
}
