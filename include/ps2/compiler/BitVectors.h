#ifndef COMPILER_BITVECTORS_H
#define COMPILER_BITVECTORS_H

#include "compiler/common.h"

#ifdef __cplusplus
extern "C" {
#endif

//! @reconstructed
struct BitVectorRange {
    SInt32 firstword;
    SInt32 nwords;
    UInt32 words[1];
};

//! @reconstructed
struct CompressedBitRun {
    UInt16 firstword;
    UInt16 nwords;
    UInt32 words[1];
};
//! @reconstructed
struct CompressedBitVector {
    UInt16 nwords;
    UInt16 nruns;
    CompressedBitRun runs[1];
};

struct FreeCompressedBitVector {
    UInt32 nwords;
    FreeCompressedBitVector* next;
};

extern SInt32 bvc_bytes_used;
extern FreeCompressedBitVector* free_compressed_bitvectors;
extern CompressedBitVector* compress_buffer;
extern UInt16 empty_compressed_bitvector[2];
extern UInt8 bitvector_nbits[256];

extern void bitvectorcopy(UInt32* dst, UInt32* src, SInt32 len);
extern int bitvectorchanged(UInt32* dst, UInt32* src, SInt32 len);
extern void bitvectorinitialize(UInt32* vec, SInt32 len, UInt32 initval);
extern void bitvectorintersect(UInt32* dst, UInt32* src, SInt32 len);
extern void bitvectorunion(UInt32* dst, UInt32* src, SInt32 len);
extern int bitvectorintersectionisempty(UInt32* a, UInt32* b, int len);

extern SInt32 bitvectorcount(UInt32* vec, SInt32 len); //! @reconstructed

extern BitVectorRange* bitvectorcreate_range(SInt32 first, SInt32 count);
extern void bitvectorinitialize_range(
    BitVectorRange* range, SInt32 first, SInt32 count, UInt32 value
);
extern CompressedBitVector* bitvector_compress(UInt32* vec, int bitCount);
extern CompressedBitVector* bitvector_empty_compressed(void);
extern void init_compressed_bitvectors(int bitCount);

#define bitvectorgetbit(_bit, _vec) ((1 << ((_bit) & 31)) & (_vec)[(_bit) >> 5])
#define bitvectorsetbit(_bit, _vec) ((_vec)[(_bit) >> 5] |= 1 << ((_bit) & 31))
#define bitvectorclearbit(_bit, _vec) ((_vec)[(_bit) >> 5] &= ~(1 << ((_bit) & 31)))

//! @reconstructed
static inline char bitvectorgetbit_range(SInt32 bit, BitVectorRange* range) {
    int word = (bit >> 5) - range->firstword;
    if (word < 0 || word >= range->nwords) {
        return 0;
    }
    return (range->words[word] >> (bit & 31)) & 1;
}

//! @reconstructed
static inline void bitvectorsetbit_range(SInt32 bit, BitVectorRange* range) {
    int word = (bit >> 5) - range->firstword;
    if (word >= 0 && word < range->nwords) {
        range->words[word] |= 1 << (bit & 31);
    }
}

//! @reconstructed
static inline void bitvectorclearbit_range(SInt32 bit, BitVectorRange* range) {
    int word = (bit >> 5) - range->firstword;
    if (word >= 0 && word < range->nwords) {
        range->words[word] &= ~(1 << (bit & 31));
    }
}

#ifdef __cplusplus
}
#endif

#endif
