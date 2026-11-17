#ifndef COMPILER_BITVECTOR_H
#define COMPILER_BITVECTOR_H

#include "compiler/common.h"

struct BitVector {
    UInt32 size;
    UInt32 data[0];
};

extern void Bv_AllocVector(BitVector** bv, UInt32 size);
extern void Bv_AllocVectorLocal(BitVector** bv, UInt32 size);
extern void Bv_ClearBit(UInt32 bit, BitVector* bv);
extern void Bv_And(const BitVector* a, BitVector* b);
extern void Bv_Or(const BitVector* a, BitVector* b);
extern Boolean Bv_Compare(const BitVector* a, const BitVector* b);
extern void Bv_Minus(const BitVector* a, BitVector* b);
extern void Bv_Copy(const BitVector* src, BitVector* dst);
extern void Bv_Clear(BitVector* bv);
extern void Bv_Set(BitVector* bv);
extern Boolean Bv_IsSubset(const BitVector* a, const BitVector* b);
extern Boolean Bv_IsEmpty(const BitVector* bv);
extern UInt32 Bv_CountBits(BitVector* bv);
extern UInt32 Bv_CountBitsInBitRange(BitVector* bv, UInt32 first, UInt32 last);
extern void Bv_Dump(char* name, BitVector* bv);

static inline Boolean Bv_BitsInCommon(BitVector* a, BitVector* b) {
    UInt32 size;
    UInt32 i;

    size = a->size;
    if (b->size < size) {
        size = b->size;
    }
    for (i = 0; i < size; i++) {
        if (a->data[i] & b->data[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

static Boolean Bv_IsBitSet(UInt32 bit, BitVector* bv) {
    return ((UInt32)(bit >> 5) < bv->size) && (bv->data[bit >> 5] & (1 << (bit & 31)));
}

#endif
