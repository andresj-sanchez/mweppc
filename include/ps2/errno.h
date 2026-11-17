#ifndef _ERRNO_H
#define _ERRNO_H

#include "msl/threadlocaldata.h"

#define errno (_GetThreadLocalData(1)->errno)

#define EBADF 9
#define EFPOS 40
#define ESIGPARM 26

#endif
