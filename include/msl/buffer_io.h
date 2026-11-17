#ifndef _MSL_BUFFER_IO_H
#define _MSL_BUFFER_IO_H

#include "msl/file_struc.h"

enum {
    __align_buffer,
    __dont_align_buffer
};

extern void __convert_from_newlines(unsigned char* p, size_t* n);
extern void __convert_to_newlines(unsigned char* p, size_t* n);
extern void __prep_buffer(FILE* file);
extern int __load_buffer(FILE* file, size_t* bytes_loaded, int alignment);
extern int __flush_buffer(FILE* file, size_t* bytes_flushed);

#endif
