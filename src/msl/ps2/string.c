#include <string.h>

char* strrchr(const char* str, int chr) {
    const char* p = str;
    const char* q = 0;
    char c = (char)chr;
    char ch;

    while ((ch = *p++)) {
        if (ch == c) {
            q = p - 1;
        }
    }

    if (q) {
        return (char*)q;
    }

    return c ? 0 : (char*)(p - 1);
}

typedef unsigned char char_map[32];

#define set_char_map(map, ch) map[(unsigned char)(ch) >> 3] |= (unsigned char)(1 << ((ch) & 7))
#define tst_char_map(map, ch) (map[(unsigned char)(ch) >> 3] & (unsigned char)(1 << ((ch) & 7)))

char* strpbrk(const char* str, const char* set) {
    const unsigned char* p;
    int c;
    char_map map = {0};

    p = (unsigned char*)set;

    while ((c = *p++)) {
        set_char_map(map, c);
    }

    p = (unsigned char*)str;

    while ((c = *p++)) {
        if (tst_char_map(map, c)) {
            return (char*)(p - 1);
        }
    }

    return 0;
}

char* strstr(const char* str, const char* pat) {
    unsigned char* s1 = (unsigned char*)str;
    unsigned char* p1 = (unsigned char*)pat;
    unsigned char firstc, c1, c2;

    if ((pat == 0) || (!(firstc = *p1++))) {
        return (char*)str;
    }

    while ((c1 = *s1++)) {
        if (c1 == firstc) {
            const unsigned char* s2 = s1;
            const unsigned char* p2 = p1;

            while ((c1 = *s2++) == (c2 = *p2++) && c1)
                ;

            if (!c2) {
                return (char*)(s1 - 1);
            }
        }
    }

    return 0;
}
