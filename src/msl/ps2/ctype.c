#include "msl/ctype_api.h"

int isalpha(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? 0 : (int)(_MSL_CMAP_ACCESS[c] & __msl_alpha);
}

int tolower(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? c : (int)(_MSL_CLOWER_ACCESS[c]);
}

int toupper(int c) {
    return ((c < 0) || (c >= __msl_cmap_size)) ? c : (int)(_MSL_CUPPER_ACCESS[c]);
}
