#ifndef _MSL_MBSTRING_H
#define _MSL_MBSTRING_H

#include <stddef.h>
#include "msl/locale.h"
#include "msl/threadlocaldata.h"

#define MB_LEN_MAX 3

extern int mbtowc(wchar_t* pwc, const char* s, size_t n);
extern int wctomb(char* s, wchar_t wchar);
extern size_t wcstombs(char* s, const wchar_t* pwcs, size_t n);

#endif
