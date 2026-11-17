#ifndef _MSL_FILE_IO_H
#define _MSL_FILE_IO_H

#include "msl/file_struc.h"

extern int __open_file(const char* name, __file_modes mode, __file_handle* handle);
extern int __open_temp_file(__file_handle* handle);
extern int __read_file(
    __file_handle handle, unsigned char* buffer, size_t* count, __ref_con ref_con
);
extern int __write_file(
    __file_handle handle, unsigned char* buffer, size_t* count, __ref_con ref_con
);
extern int __position_file(__file_handle handle, fpos_t* position, int mode, __ref_con ref_con);
extern int __close_file(__file_handle handle);
extern void __temp_file_name(char* name_str, void* fsspec);
extern int __delete_file(const char* name);
extern int __rename_file(const char* old_name, const char* new_name);

typedef struct {
    __file_handle mFileHandle;
    char* mFileName;
} temp_file_info;

extern temp_file_info* find_temp_info(__file_handle inHandle);

#endif
