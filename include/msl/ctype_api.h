#ifndef _MSL_CTYPE_API_H
#define _MSL_CTYPE_API_H

#include "msl/threadlocaldata.h"

#define __msl_cmap_size 256

#define __msl_alpha 0x0001
#define __msl_blank 0x0002
#define __msl_cntrl 0x0004
#define __msl_digit 0x0008
#define __msl_graph 0x0010
#define __msl_lower 0x0020
#define __msl_print 0x0040
#define __msl_punct 0x0080
#define __msl_space 0x0100
#define __msl_upper 0x0200
#define __msl_xdigit 0x0400

#define __msl_alnum (__msl_alpha | __msl_digit)

#define _MSL_CMAP_ACCESS _GetThreadLocalData(1)->_current_locale.ctype_cmpt_ptr->ctype_map_ptr
#define _MSL_CLOWER_ACCESS _GetThreadLocalData(1)->_current_locale.ctype_cmpt_ptr->lower_map_ptr
#define _MSL_CUPPER_ACCESS _GetThreadLocalData(1)->_current_locale.ctype_cmpt_ptr->upper_map_ptr

#if defined(_MSL_USE_INLINE) && _MSL_USE_INLINE

#ifdef __cplusplus
extern "C" {
#endif

inline int isalnum(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? 0 : (int)(_MSL_CMAP_ACCESS[c] & __msl_alnum);
}

inline int isalpha(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? 0 : (int)(_MSL_CMAP_ACCESS[c] & __msl_alpha);
}

inline int isblank(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? 0 : (int)(_MSL_CMAP_ACCESS[c] & __msl_blank);
}

inline int iscntrl(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? 0 : (int)(_MSL_CMAP_ACCESS[c] & __msl_cntrl);
}

inline int isdigit(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? 0 : (int)(_MSL_CMAP_ACCESS[c] & __msl_digit);
}

inline int isgraph(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? 0 : (int)(_MSL_CMAP_ACCESS[c] & __msl_graph);
}

inline int islower(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? 0 : (int)(_MSL_CMAP_ACCESS[c] & __msl_lower);
}

inline int isprint(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? 0 : (int)(_MSL_CMAP_ACCESS[c] & __msl_print);
}

inline int ispunct(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? 0 : (int)(_MSL_CMAP_ACCESS[c] & __msl_punct);
}

inline int isspace(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? 0 : (int)(_MSL_CMAP_ACCESS[c] & __msl_space);
}

inline int isupper(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? 0 : (int)(_MSL_CMAP_ACCESS[c] & __msl_upper);
}

inline int isxdigit(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? 0 : (int)(_MSL_CMAP_ACCESS[c] & __msl_xdigit);
}

inline int tolower(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? c : (int)(_MSL_CLOWER_ACCESS[c]);
}

inline int toupper(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? c : (int)(_MSL_CUPPER_ACCESS[c]);
}

#ifdef __cplusplus
}
#endif

#endif

#endif
