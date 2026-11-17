#ifndef _MSL_TIME_API_H
#define _MSL_TIME_API_H

#include "compiler/win32.h"
#include <time.h>

extern clock_t __get_clock(void);
extern int __isdst(void);
extern time_t __get_time(void);
extern int __to_gm_time(time_t* time);
extern time_t __systemtime_to_time_t(SYSTEMTIME* time);

#endif
