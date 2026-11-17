#include "compiler/common.h"
#include "msl/strtold.h"

double __double_min = DBL_MIN;
double __double_max = DBL_MAX;

double strtod(const char* str, char** end) {
    double value, abs_value;
    int count, overflow;

    __InStrCtrl isc;
    isc.NextChar = (char*)str;
    isc.NullCharDetected = 0;

    value = __strtold(INT_MAX, &__StringRead, (void*)&isc, &count, &overflow);

    if (end) {
        *end = (char*)str + count;
    }

    abs_value = fabs(value);

    if (overflow || (value != 0.0 && (abs_value < __double_min || abs_value > __double_max))) {
        errno = ERANGE;
    }

    return value;
}
