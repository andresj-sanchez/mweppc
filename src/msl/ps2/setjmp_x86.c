#include <setjmp.h>

int setjmp(jmp_buf env) {
    env[0] = 0;
    env[1] = 0;
    env[2] = 0;
    env[3] = (int)env;
    env[4] = 0;
    env[5] = 0;
    return 0;
}
