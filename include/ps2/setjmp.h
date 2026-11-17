#ifndef _SETJMP_H
#define _SETJMP_H

typedef int jmp_buf[10];

void longjmp(jmp_buf env, int val);

#endif
