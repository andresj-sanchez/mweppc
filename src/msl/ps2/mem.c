#include <string.h>

#undef memset

typedef unsigned int word;

void* memset(void* dest, int c, size_t count) {
    unsigned char* q = (unsigned char*)dest;
    unsigned int value = (unsigned char)c;
    if (count > 16) {
        unsigned int half = value | (value << 8);
        value = half | (half << 16);
        if (count > 4) {
            size_t leading = -(unsigned long)q & 3;
            while (leading--) {
                *q++ = (unsigned char)value;
                count--;
            }
            while (count > 3) {
                *(word*)q = value;
                q += 4;
                count -= 4;
            }
        }
    }
    while (count--) {
        *q++ = (unsigned char)value;
    }
    return dest;
}
