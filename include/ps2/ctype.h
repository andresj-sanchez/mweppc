#ifndef _CTYPE_H
#define _CTYPE_H

#if defined(_MSL_USE_INLINE) && _MSL_USE_INLINE
#include "msl/ctype_api.h"
#else
extern int isalpha(int c);
extern int tolower(int c);
extern int toupper(int c);
#endif

#endif
