#ifndef _STDDEF_H
#define _STDDEF_H

#ifndef _MSL_SIZE_T_TYPE
#define _MSL_SIZE_T_TYPE unsigned long
#endif
typedef _MSL_SIZE_T_TYPE size_t;
typedef int ptrdiff_t;

#ifndef _WCHAR_T
#define _WCHAR_T
#if !defined(__cplusplus)
typedef unsigned short wchar_t;
#endif
#endif

#endif
