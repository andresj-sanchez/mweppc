#ifndef _MSL_FCNTL_H
#define _MSL_FCNTL_H

#include <stddef.h>

#define O_TEXT 0x0
#define O_RDWR 0x1
#define O_RDONLY 0x2
#define O_WRONLY 0x4
#define O_APPEND 0x0100
#define O_CREAT 0x0200
#define O_EXCL 0x0400
#define O_TRUNC 0x0800
#define O_BINARY 0x8000

extern int __msl_open(const char* name, int flags);
extern int __msl_close(int handle);
extern long __msl_lseek(int handle, long offset, int mode);
extern int __msl_read(int handle, void* buffer, size_t count);
extern int __msl_write(int handle, const void* buffer, size_t count);

#endif
