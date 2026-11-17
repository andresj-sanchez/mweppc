#ifndef _STDARG_H
#define _STDARG_H

typedef char* va_list;

#define __va_start(parm) \
    ((va_list)((long)&(parm) + ((((long)(&(parm) + 1) - (long)&(parm)) + 3) / 4 * 4)))
#define va_start(ap, parm) ((ap) = __va_start(parm))
#define _Align(type, mask) (sizeof(type) + (mask) & ~(mask))
#define va_arg(ap, type) (*(type*)(((ap) += _Align(type, 3)) - _Align(type, 3)))
#define va_end(ap) ((void)0)

#endif
