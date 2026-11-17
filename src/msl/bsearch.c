#include <stddef.h>

#pragma register_coloring off
void* bsearch(
    const void* volatile key,
    const void* volatile base,
    size_t count,
    volatile size_t width,
    int (*volatile compare)(const void*, const void*)
) {
    volatile size_t first;
    size_t last;
    size_t middle;
    const char* element;
    int comparison;

    if (!key || !base || !count || !width || !compare) {
        return 0;
    }

    element = (const char*)base;
    comparison = compare(key, element);
    if (comparison == 0) {
        return (void*)element;
    }
    if (comparison < 0) {
        return 0;
    }

    first = 1;
    last = count - 1;
    if (last >= 1) {
        do {
            middle = (first + last) >> 1;
            element = (const char*)base + width * middle;
            comparison = compare(key, element);
            if (comparison == 0) {
                return (void*)element;
            }
            if (comparison < 0) {
                last = middle - 1;
            } else {
                first = middle;
                ++first;
            }
        } while (first <= last);
    }

    return 0;
}

#pragma register_coloring on
