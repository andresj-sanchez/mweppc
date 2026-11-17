#ifndef _MSL_ANSI_FILES_H
#define _MSL_ANSI_FILES_H

#include "msl/file_struc.h"

extern void __init_file(FILE* file, __file_modes mode, char* buff, size_t buff_size);
extern void __close_all(void);
extern int __flush_all(void);

#define set_eof(file) \
    do { \
        (file)->state.io_state = __neutral; \
        (file)->state.eof = 1; \
        (file)->buffer_len = 0; \
    } while (0)

#define set_error(file) \
    do { \
        (file)->state.error = 1; \
        (file)->buffer_len = 0; \
    } while (0)

#endif
