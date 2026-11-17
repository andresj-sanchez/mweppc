#include "compiler/common.h"
#include "compiler/PCode.h"
#include "compiler/objects.h"
#include "compiler/Registers.h"
#include "compiler/BitVectors.h"
#include "compiler/Alias.h"
#include "compiler/CompilerTools.h"
#include "compiler/CParser.h"

UInt16 empty_compressed_bitvector[2];
CompressedBitVector* compress_buffer;
FreeCompressedBitVector* free_compressed_bitvectors;
SInt32 bvc_bytes_used;
UInt8 bitvector_nbits[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

void bitvectorcopy(UInt32* dst, UInt32* src, SInt32 len) {
    SInt32 i = (len + 31) >> 5;

    while (i != 0) {
        *dst = *src;
        dst++;
        src++;
        i--;
    }
}

int bitvectorchanged(UInt32* dst, UInt32* src, SInt32 len) {
    SInt32 i = (len + 31) >> 5;
    SInt32 changed = FALSE;
    UInt32 v;

    while (i != 0) {
        v = *src;
        if (*dst != v) {
            changed = TRUE;
        }
        *dst = v;
        dst++;
        src++;
        i--;
    }
    return changed;
}

void bitvectorinitialize(UInt32* vec, SInt32 len, UInt32 initval) {
    SInt32 i = (len + 31) >> 5;

    while (i != 0) {
        *vec = initval;
        vec++;
        i--;
    }
}

void bitvectorinitialize_range(BitVectorRange* range, SInt32 first, SInt32 count, UInt32 value) {
    int i, n;
    range->firstword = first >> 5;
    n = count ? ((first + count + 31) >> 5) - (first >> 5) : 0;
    range->nwords = n;
    for (i = 0; i < n; i++) {
        range->words[i] = value;
    }
}

struct BitVectorRange* bitvectorcreate_range(SInt32 first, SInt32 count) {
    struct BitVectorRange* range;
    UInt32 size;

    size = (count ? ((first + count + 31) >> 5) - (first >> 5) : 0) * 4 + 8;
    range = oalloc(size);
    bvc_bytes_used += size;
    memclrw(range, size);
    return range;
}

void bitvectorintersect(UInt32* dst, UInt32* src, SInt32 len) {
    SInt32 i = (len + 31) >> 5;

    while (i != 0) {
        *dst &= *src;
        dst++;
        src++;
        i--;
    }
}

void bitvectorunion(UInt32* dst, UInt32* src, SInt32 len) {
    SInt32 i = (len + 31) >> 5;

    while (i != 0) {
        *dst |= *src;
        dst++;
        src++;
        i--;
    }
}

SInt32 bitvectorcount(UInt32* vec, SInt32 len) {
    SInt32 i = (len + 31) >> 5;
    UInt32 v;
    SInt32 total = 0;

    while (i != 0) {
        if ((v = *vec)) {
            total += bitvector_nbits[v & 0xFF];
            total += bitvector_nbits[(v >> 8) & 0xFF];
            total += bitvector_nbits[(v >> 16) & 0xFF];
            total += bitvector_nbits[(v >> 24) & 0xFF];
        }
        vec++;
        i--;
    }
    return total;
}

int bitvectorintersectionisempty(UInt32* a, UInt32* b, int len) {
    SInt32 i = (len + 31) >> 5;

    while (i != 0) {
        if (*a & *b) {
            return FALSE;
        }
        a++;
        b++;
        i--;
    }
    return TRUE;
}

void init_compressed_bitvectors(int bitCount) {
    SInt32 size;

    free_compressed_bitvectors = NULL;
    size = (((bitCount + 31) >> 5) * 4 + 39) & ~31;
    compress_buffer = oalloc(size);
    bvc_bytes_used = size;
    empty_compressed_bitvector[0] = 1;
    empty_compressed_bitvector[1] = 0;
}

CompressedBitVector* bitvector_empty_compressed(void) {
    return (CompressedBitVector*)empty_compressed_bitvector;
}

#include <string.h>

CompressedBitVector* bitvector_compress(UInt32* vec, int bitCount) {
    struct CompressedBitRun* run;
    SInt32 nwords;
    SInt32 size;
    SInt32 nruns;
    SInt32 count;
    SInt32 zeros;
    UInt32* words;
    UInt32 word;
    SInt32 i;
    struct FreeCompressedBitVector** prev;
    struct FreeCompressedBitVector* scan;

    nwords = (bitCount + 31) >> 5;
    size = 4;
    i = 0;
    nruns = 0;
    count = 0;
    zeros = 0;
    for (; i < nwords; i++) {
        word = vec[i];
        if (word == 0) {
            zeros++;
        } else {
            if (nruns == 0) {
                run = compress_buffer->runs;
                run->firstword = i;
                count = 0;
                words = run->words;
                nruns++;
                zeros = 0;
            } else if (zeros > 2) {
                run->nwords = count;
                size += count * sizeof(UInt32) + sizeof(UInt32);
                run = (struct CompressedBitRun*)&words[count];
                run->firstword = i;
                count = 0;
                words = run->words;
                nruns++;
                zeros = 0;
            } else {
                while (zeros > 0) {
                    words[count++] = 0;
                    zeros--;
                }
            }
            words[count++] = word;
        }
    }
    if (count > 0) {
        run->nwords = count;
        size += count * sizeof(UInt32) + sizeof(UInt32);
    }
    compress_buffer->nwords = ((size + 31) & ~31) / 4;
    compress_buffer->nruns = nruns;
    if (nruns == 0) {
        return (CompressedBitVector*)empty_compressed_bitvector;
    }
    prev = &free_compressed_bitvectors;
    scan = free_compressed_bitvectors;
    while (scan) {
        if (scan->nwords == compress_buffer->nwords) {
            *prev = scan->next;
            memcpy(scan, compress_buffer, compress_buffer->nwords * 4);
            return (struct CompressedBitVector*)scan;
        }
        prev = &scan->next;
    }
    scan = oalloc(compress_buffer->nwords * 4);
    bvc_bytes_used += compress_buffer->nwords * 4;
    memcpy(scan, compress_buffer, compress_buffer->nwords * 4);
    return (struct CompressedBitVector*)scan;
}
