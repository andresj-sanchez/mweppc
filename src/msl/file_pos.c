#include "compiler/common.h"
#include "msl/ansi_files.h"
#include "msl/buffer_io.h"
#include "msl/critical_regions.h"
#include <errno.h>

fpos_t _ftell(FILE* file) {
    int charsInUndoBuffer = 0;
    fpos_t position;

    unsigned char tmp_kind = file->mode.file_kind;
    if (!(tmp_kind == __disk_file || tmp_kind == __console_file) || file->state.error) {
        errno = EFPOS;
        return -1L;
    }

    if (file->state.io_state == __neutral) {
        return file->position;
    }

    position = file->buffer_pos + (file->buffer_ptr - file->buffer);

    if (file->state.io_state >= __rereading) {
        charsInUndoBuffer = file->state.io_state - __rereading + 1;
        position -= charsInUndoBuffer;
    }

    if (!file->mode.binary_io) {
        ptrdiff_t n = file->buffer_ptr - file->buffer - charsInUndoBuffer;
        unsigned char* p = file->buffer;

        while (n--) {
            if (*p++ == '\n') {
                position++;
            }
        }
    }

    return position;
}

long ftell(FILE* file) {
    long retval;
    int crtrgn;

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

    retval = (long)_ftell(file);

    __end_critical_region(crtrgn);

    return retval;
}

int _fseek(FILE* file, fpos_t offset, int mode) {
    fpos_t position;
    __pos_proc pos_proc;

    unsigned char tmp_kind = file->mode.file_kind;
    if (!(tmp_kind == __disk_file) || file->state.error) {
        errno = EFPOS;
        return -1;
    }

    if (file->state.io_state == __writing) {
        if (__flush_buffer(file, NULL) != __no_io_error) {
            set_error(file);
            errno = EFPOS;
            return -1;
        }
    }

    if (mode == SEEK_CUR) {
        mode = SEEK_SET;
        offset += _ftell(file);
    }

    if ((pos_proc = file->position_proc) != 0 &&
        (*pos_proc)(file->handle, &offset, mode, file->ref_con)) {
        set_error(file);
        errno = EFPOS;
        return -1;
    }

    file->state.io_state = __neutral;
    file->state.eof = 0;
    file->position = offset;
    file->buffer_len = 0;

    return 0;
}

int fseek(FILE* file, long offset, int mode) {
    fpos_t real_offset = (fpos_t)offset;
    int retval;

    int crtrgn;

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

    retval = _fseek(file, real_offset, mode);

    __end_critical_region(crtrgn);

    return retval;
}
