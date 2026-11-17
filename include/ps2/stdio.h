#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <stdarg.h>

#include "msl/file_struc.h"
extern int setvbuf(FILE* file, char* buff, int mode, size_t size);
extern size_t fwrite(const void* ptr, size_t memb_size, size_t num_memb, FILE* file);
extern void clearerr(FILE* stream);
extern int sprintf(char* s, const char* format, ...);
extern int snprintf(char* s, size_t n, const char* format, ...);
extern int vsnprintf(char* s, size_t n, const char* format, va_list args);
extern int fprintf(FILE* stream, const char* format, ...);
extern int vfprintf(FILE* stream, const char* format, va_list args);
extern int fflush(FILE* stream);
extern int fclose(FILE* stream);
extern FILE* fopen(const char* name, const char* mode);

FILE* freopen(const char* name, const char* mode, FILE* file);
#endif
