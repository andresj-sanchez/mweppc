#ifndef _MSL_FILE_IO_HELPERS_H
#define _MSL_FILE_IO_HELPERS_H

#include <stddef.h>
#include "msl/file_struc.h"

extern FILE* __find_unopened_file(void);
extern int __get_file_modes(const char* mode, __file_modes* modes);
extern long ftell(FILE* file);
extern int fseek(FILE* file, long offset, int mode);

#endif
