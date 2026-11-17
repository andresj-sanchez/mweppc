#include "compiler/BitVector.h"
#include "compiler/CError.h"
#include "compiler/CompilerTools.h"
#include <string.h>
#include "compiler/IroDump.h"

static SInt8 bv_popcount[256];
static Boolean bv_popcount_ready;

static void Bv_InitPopCountTable(void) {
    int i;
    int mask;
    int count;

    for (i = 0; i < 256; i++) {
        for (mask = 1, count = 0; mask < 256; mask += mask) {
            count += (i & mask) != 0;
        }
        bv_popcount[i] = count;
    }
    bv_popcount_ready = TRUE;
}

void Bv_AllocVector(BitVector** bv, UInt32 size) {
    size = (size / 32) + 1;
    *bv = IRO_oalloc(sizeof(BitVector) + sizeof(UInt32) * size);
    (*bv)->size = size;
    Bv_Clear(*bv);
}

void Bv_AllocVectorLocal(BitVector** bv, UInt32 size) {
    size = (size / 32) + 1;
    *bv = lalloc(sizeof(BitVector) + sizeof(UInt32) * size);
    (*bv)->size = size;
    Bv_Clear(*bv);
}

void Bv_ClearBit(UInt32 bit, BitVector* bv) {
    if ((bit / 32) < bv->size) {
        bv->data[bit / 32] &= ~(1 << (bit & 31));
    } else {
        CError_FATAL(56);
    }
}

void Bv_And(const BitVector* a, BitVector* b) {
    UInt32 i;
    UInt32 len;

    len = a->size;
    if (b->size < len) {
        len = b->size;
    }

    for (i = 0; i < len; i++) {
        b->data[i] &= a->data[i];
    }
}

void Bv_Or(const BitVector* a, BitVector* b) {
    UInt32 i;
    UInt32 len;

    len = a->size;
    if (b->size < len) {
        len = b->size;
    }

    for (i = 0; i < len; i++) {
        b->data[i] |= a->data[i];
    }
}

Boolean Bv_Compare(const BitVector* a, const BitVector* b) {
    UInt32 i;
    UInt32 sizeA;
    UInt32 sizeB;
    UInt32 len;
    UInt32 len2;

    sizeB = b->size;
    sizeA = a->size;
    if (sizeA < sizeB) {
        len = sizeA;
    } else {
        len = sizeB;
    }

    len2 = len;
    for (i = 0; i < len2; i++) {
        if (a->data[i] != b->data[i]) {
            return FALSE;
        }
    }

    if (sizeA < sizeB) {
        for (i = sizeA; i < sizeB; i++) {
            if (b->data[i]) {
                return FALSE;
            }
        }
    } else if (sizeB < sizeA) {
        for (i = sizeB; i < sizeA; i++) {
            if (a->data[i]) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

void Bv_Minus(const BitVector* a, BitVector* b) {
    UInt32 len;
    UInt32 i;

    len = a->size;
    if (b->size < len) {
        len = b->size;
    }

    for (i = 0; i < len; i++) {
        b->data[i] &= ~a->data[i];
    }
}

void Bv_Copy(const BitVector* src, BitVector* dst) {
    if (dst->size <= src->size) {
        memcpy(dst->data, src->data, sizeof(UInt32) * dst->size);
    } else {
        memcpy(dst->data, src->data, sizeof(UInt32) * src->size);
        memset(dst->data + src->size, 0, sizeof(UInt32) * (dst->size - src->size));
    }
}

void Bv_Clear(BitVector* bv) {
    memset(bv->data, 0, sizeof(UInt32) * bv->size);
}

void Bv_Set(BitVector* bv) {
    memset(bv->data, 0xFF, sizeof(UInt32) * bv->size);
}

Boolean Bv_IsSubset(const BitVector* a, const BitVector* b) {
    int i;

    for (i = 0; (UInt32)i < a->size; i++) {
        if (b->size < (UInt32)i) {
            if (a->data[i]) {
                return FALSE;
            }
        } else {
            if (a->data[i] & ~(b->data[i])) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

Boolean Bv_IsEmpty(const BitVector* bv) {
    UInt32 i;

    for (i = 0; i < bv->size; i++) {
        if (bv->data[i]) {
            return FALSE;
        }
    }

    return TRUE;
}

//! @reconstructed
static UInt32 Bv_CountBitsInWordRange(BitVector* bv, UInt32 first, UInt32 last) {
    UInt32 vector = (UInt32)bv;
    UInt32 i;
    UInt32 p;
    UInt32 count;

    count = 0;
    CError_ASSERT(257, first <= last && last <= ((BitVector*)vector)->size);
    i = first;
    if (i < last) {
        p = (UInt32) & ((BitVector*)vector)->data[first];
        do {
            if (((BitVector*)vector)->data[i] != 0) {
                if (((BitVector*)vector)->data[i] == 0xFFFFFFFF) {
                    count += 0x20;
                } else {
                    UInt8* bytes = (UInt8*)p;
                    count += bv_popcount[bytes[0]] + bv_popcount[bytes[1]] + bv_popcount[bytes[2]] +
                        bv_popcount[bytes[3]];
                }
            }
            i++;
            p += 4;
        } while (i < last);
    }

    return count;
}

UInt32 Bv_CountBitsInBitRange(BitVector* bv, UInt32 first, UInt32 last) {
    UInt32 i;
    UInt32 count;
    UInt32 t;
    UInt32 n;
    UInt32 j;

    count = 0;
    if (!bv_popcount_ready) {
        Bv_InitPopCountTable();
    }
    CError_ASSERT(347, first <= last && last <= bv->size * 32);

    for (i = first; i < last && (i & 7) != 0; i++) {
        if (Bv_IsBitSet(i, bv)) {
            count++;
        }
    }

    if (i + 8 < last) {
        t = 0;
        n = last >> 3;
        i >>= 3;
        CError_ASSERT(312, i <= n && n <= bv->size * 4);
        j = i;
        while (j < n) {
            if ((j & 3) == 0 && (j >> 2) != (n >> 2)) {
                t += Bv_CountBitsInWordRange(bv, j >> 2, n >> 2);
                j = n & ~3;
            } else {
                t += bv_popcount[((UInt8*)bv->data)[j]];
                j++;
            }
        }
        count += t;
        i = last & ~7;
    }

    for (; i < last; i++) {
        if (Bv_IsBitSet(i, bv)) {
            count++;
        }
    }

    return count;
}

UInt32 Bv_CountBits(BitVector* bv) {
    if (!bv_popcount_ready) {
        Bv_InitPopCountTable();
    }
    return Bv_CountBitsInBitRange(bv, 0, bv->size * 32);
}

void Bv_Dump(char* name, BitVector* bv) {
    UInt32 i;
    UInt32 rangeStart;
    Boolean inRange = FALSE;
    Boolean isFirst = TRUE;

    if (!IRO_Dumping()) {
        return;
    }

    IRO_Dump(name);
    if (!bv) {
        IRO_Dump("NULL");
    } else {
        for (i = 0; i < bv->size * 32; i++) {
            if (Bv_IsBitSet((SInt32)i, bv)) {
                if (!inRange) {
                    if (!isFirst) {
                        IRO_Dump(",");
                    }
                    isFirst = FALSE;
                    IRO_Dump("%d", i);
                    inRange = TRUE;
                    rangeStart = i;
                }
            } else {
                if (inRange) {
                    inRange = FALSE;
                    if (i != rangeStart + 1) {
                        IRO_Dump("-%d", i - 1);
                    }
                }
            }
        }

        if (inRange && i != rangeStart + 1) {
            IRO_Dump("-%d", i - 1);
        }
    }

    IRO_Dump("\n");
}
