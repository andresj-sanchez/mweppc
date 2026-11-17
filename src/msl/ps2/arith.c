#include "msl/arith.h"
#include "compiler/common.h"
#include <limits.h>
#include <stdlib.h>

int __msl_add(int* result, int amount) {
    int value = *result;

    if (amount >= 0) {
        if (value > 0 && amount > 0x7fffffff - value) {
            return FALSE;
        }
    } else {
        if (value < 0 && amount < (-2147483647 - 1) - value) {
            return FALSE;
        }
    }
    *result = value + amount;
    return TRUE;
}

int __msl_ladd(long* result, long amount) {
    long value = *result;

    if (amount >= 0) {
        if (value > 0 && amount > 0x7fffffff - value) {
            return FALSE;
        }
    } else {
        if (value < 0 && amount < (-2147483647 - 1) - value) {
            return FALSE;
        }
    }
    *result = value + amount;
    return TRUE;
}

int __msl_lmul(long* result, long factor) {
    long value = *result;
    int sign;

    sign = ((value < 0) ^ (factor < 0)) ? -1 : 1;

    if (value < 0) {
        value = -value;
    }
    if (factor < 0) {
        factor = -factor;
    }

    if (value > LONG_MAX / factor) {
        return FALSE;
    }

    *result = value * factor * sign;
    return TRUE;
}

div_t __msl_div(int lhs, int rhs) {
    int q, r, x_sign, y_sign, q_sign;
    div_t result;

    x_sign = 1;
    y_sign = 1;

    if (lhs < 0) {
        lhs = -lhs;
        x_sign = -1;
    }

    if (rhs < 0) {
        rhs = -rhs;
        y_sign = -1;
    }

    q_sign = x_sign * y_sign;

    q = (lhs / rhs) * q_sign;
    r = (lhs * x_sign) - (q * rhs * y_sign);

    if (r && q_sign < 0) {
        q -= 1;
        r += rhs * y_sign;
    }

    result.quot = q;
    result.rem = r;

    return result;
}

int __msl_mod(int lhs, int rhs) {
    int q, r, x_sign, y_sign, q_sign;

    x_sign = 1;
    y_sign = 1;

    if (lhs < 0) {
        lhs = -lhs;
        x_sign = -1;
    }

    if (rhs < 0) {
        rhs = -rhs;
        y_sign = -1;
    }

    q_sign = x_sign * y_sign;

    q = (lhs / rhs) * q_sign;
    r = (lhs * x_sign) - (q * rhs * y_sign);

    if (r && q_sign < 0) {
        q -= 1;
        r += rhs * y_sign;
    }

    return r;
}
