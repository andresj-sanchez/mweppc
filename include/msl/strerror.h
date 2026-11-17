#ifndef MSL_STRERROR_H
#define MSL_STRERROR_H

extern char __strerror_buffer[256];
extern char* __strerror(int errnum, char* buffer);

#endif
