#define _MSL_SIZE_T_TYPE unsigned int
#include <string.h>
#include <stddef.h>
#include "msl/locale.h"
#include "msl/mbstring.h"

int mbtowc(wchar_t* pwc, const char* s, size_t n) {
    return _GetThreadLocalData(1)->_current_locale.ctype_cmpt_ptr->decode_mb(pwc, s, n);
}

int __mbtowc_noconv(wchar_t* pwc, const char* s, size_t n) {
    if (!s) {
        return 0;
    }

    if (!n) {
        return -1;
    }

    if (pwc) {
        *pwc = (unsigned char)*s;
    }

    if (!*s) {
        return 0;
    }

    return 1;
}

int __wctomb_noconv(char* s, wchar_t wchar) {
    if (!s) {
        return 0;
    }

    *s = wchar;

    return 1;
}

int wctomb(char* s, wchar_t wchar) {
    return _GetThreadLocalData(1)->_current_locale.ctype_cmpt_ptr->encode_wc(s, wchar);
}

#pragma avoid_ebp on
size_t wcstombs(char* restrict s, const wchar_t* restrict pwcs, size_t n) {
    int chars_written = 0;
    int result;
    char temp[MB_LEN_MAX];
    wchar_t* source;

    if (!s || !pwcs) {
        return 0;
    }

    source = (wchar_t*)pwcs;
    while (chars_written <= n) {
        if (!*source) {
            *(s + chars_written) = '\0';
            break;
        } else {
            result = wctomb(temp, *source++);
            if ((chars_written + result) <= n) {
                strncpy(s + chars_written, temp, result);
                chars_written += result;
            } else {
                break;
            }
        }
    }

    return chars_written;
}
