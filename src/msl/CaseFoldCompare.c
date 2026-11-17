#define _MSL_USE_INLINE 1
#include "compiler/common.h"
#include <ctype.h>
#include "msl/extras_string.h"

int stricmp(const char* left, const char* right) {
    char c1, c2;

    while (TRUE) {
        c1 = tolower(*left++);
        c2 = tolower(*right++);
        if (c1 < c2) {
            return -1;
        }
        if (c1 > c2) {
            return 1;
        }
        if (c1 == 0) {
            return 0;
        }
    }
}

int fn_004148F0(const char* left, const char* right) {
    return stricmp(left, right);
}
