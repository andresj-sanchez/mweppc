#define CINT64_NEG_IN_PLACE(v) \
    (v).hi = ~(v).hi; \
    (v).unk4 = ~(v).unk4; \
    (v).unk8 = ~(v).unk8; \
    (v).lo = ~(v).lo; \
    carry = 0; \
    for (i = 3; i >= 0; i--) { \
        lw = ((UInt32*)&(v))[i]; \
        rw = ((UInt32*)&cint64_one)[i]; \
        if (lw & 0x80000000) { \
            if (rw & 0x80000000) { \
                lw = lw + (rw + carry); \
                carry = 1; \
            } else { \
                lw = lw + rw; \
                if (carry) { \
                    carry = 0; \
                    if (!(lw & 0x80000000)) { \
                        carry = 1; \
                    } \
                    lw = lw + 1; \
                    if (!(lw & 0x80000000)) { \
                        carry = 1; \
                    } \
                } else { \
                    if (lw & 0x80000000) { \
                        carry = 0; \
                    } else { \
                        carry = 1; \
                    } \
                } \
            } \
        } else { \
            if (rw & 0x80000000) { \
                lw = lw + rw; \
                if (carry) { \
                    carry = 0; \
                    if (!(lw & 0x80000000)) { \
                        carry = 1; \
                    } \
                    lw = lw + 1; \
                    if (!(lw & 0x80000000)) { \
                        carry = 1; \
                    } \
                } else { \
                    if (lw & 0x80000000) { \
                        carry = 0; \
                    } else { \
                        carry = 1; \
                    } \
                } \
            } else { \
                lw = lw + (rw + carry); \
                carry = 0; \
            } \
        } \
        ((UInt32*)&(v))[i] = lw; \
    }

#include "compiler/CInt64.h"

CInt64 cint64_zero;
CInt64 cint64_negone;
CInt64 cint64_one;
CInt64 cint64_max;
CInt64 cint64_min;

void CInt64_Init(void) {
    cint64_one.lo = 1;
    cint64_negone.hi = 0xFFFFFFFF;
    cint64_max.hi = 0xFFFFFFFF;
    cint64_negone.unk4 = 0xFFFFFFFF;
    cint64_max.unk4 = 0xFFFFFFFF;
    cint64_negone.unk8 = 0xFFFFFFFF;
    cint64_max.unk8 = 0xFFFFFFFF;
    cint64_negone.lo = 0xFFFFFFFF;
    cint64_max.lo = 0xFFFFFFFF;
    cint64_max.hi = 0x7FFFFFFF;
    cint64_min.hi = 0x80000000;
}

void CInt64_SetLong(CInt64* pN, SInt32 n) {
    UInt32 ext;

    ext = (n < 0) ? 0xFFFFFFFF : 0;
    pN->hi = ext;
    pN->unk4 = ext;
    pN->unk8 = ext;
    pN->lo = n;
}

void CInt64_SetULong(CInt64* pN, UInt32 n) {
    pN->hi = 0;
    pN->unk4 = 0;
    pN->unk8 = 0;
    pN->lo = n;
}

Boolean CInt64_IsZero(CInt64* pN) {
    int i;

    for (i = 0; i < 4; i++) {
        if (((UInt32*)pN)[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

Boolean CInt64_IsOne(CInt64* pN) {
    int i;

    if (pN->lo != 1) {
        return FALSE;
    }
    for (i = 0; i < 3; i++) {
        if (((UInt32*)pN)[i] != 0) {
            return FALSE;
        }
    }
    return TRUE;
}

CInt64 CInt64_Not(CInt64 input) {
    CInt64 output;

    CInt64_SetLong(&output, CInt64_IsZero(&input));

    return output;
}

CInt64 CInt64_Inv(CInt64 input) {
    input.hi = ~input.hi;
    input.unk4 = ~input.unk4;
    input.unk8 = ~input.unk8;
    input.lo = ~input.lo;
    return input;
}

CInt64 CInt64_Neg(CInt64 input) {
    SInt32 carry;
    int i;
    UInt32 lw;
    UInt32 rw;

    input.hi = ~input.hi;
    input.unk4 = ~input.unk4;
    input.unk8 = ~input.unk8;
    input.lo = ~input.lo;
    carry = 0;
    for (i = 3; i >= 0; i--) {
        lw = ((UInt32*)&(input))[i];
        rw = ((UInt32*)&(cint64_one))[i];
        if (lw & 0x80000000) {
            if (rw & 0x80000000) {
                lw = lw + (rw + carry);
                carry = 1;
            } else {
                lw = lw + rw;
                if (carry) {
                    carry = 0;
                    if (!(lw & 0x80000000)) {
                        carry = 1;
                    }
                    lw = lw + 1;
                    if (!(lw & 0x80000000)) {
                        carry = 1;
                    }
                } else {
                    if (lw & 0x80000000) {
                        carry = 0;
                    } else {
                        carry = 1;
                    }
                }
            }
        } else {
            if (rw & 0x80000000) {
                lw = lw + rw;
                if (carry) {
                    carry = 0;
                    if (!(lw & 0x80000000)) {
                        carry = 1;
                    }
                    lw = lw + 1;
                    if (!(lw & 0x80000000)) {
                        carry = 1;
                    }
                } else {
                    if (lw & 0x80000000) {
                        carry = 0;
                    } else {
                        carry = 1;
                    }
                }
            } else {
                lw = lw + (rw + carry);
                carry = 0;
            }
        }
        ((UInt32*)&(input))[i] = lw;
    }
    return input;
}

CInt64 CInt64_Sub(CInt64 lhs, CInt64 rhs) {
    SInt32 carry;
    int i;
    UInt32 lw;
    UInt32 rw;

    rhs.hi = ~rhs.hi;
    rhs.unk4 = ~rhs.unk4;
    rhs.unk8 = ~rhs.unk8;
    rhs.lo = ~rhs.lo;
    carry = 1;
    for (i = 3; i >= 0; i--) {
        lw = ((UInt32*)&(lhs))[i];
        rw = ((UInt32*)&(rhs))[i];
        if (lw & 0x80000000) {
            if (rw & 0x80000000) {
                lw = lw + (rw + carry);
                carry = 1;
            } else {
                lw = lw + rw;
                if (carry) {
                    carry = 0;
                    if (!(lw & 0x80000000)) {
                        carry = 1;
                    }
                    lw = lw + 1;
                    if (!(lw & 0x80000000)) {
                        carry = 1;
                    }
                } else {
                    if (lw & 0x80000000) {
                        carry = 0;
                    } else {
                        carry = 1;
                    }
                }
            }
        } else {
            if (rw & 0x80000000) {
                lw = lw + rw;
                if (carry) {
                    carry = 0;
                    if (!(lw & 0x80000000)) {
                        carry = 1;
                    }
                    lw = lw + 1;
                    if (!(lw & 0x80000000)) {
                        carry = 1;
                    }
                } else {
                    if (lw & 0x80000000) {
                        carry = 0;
                    } else {
                        carry = 1;
                    }
                }
            } else {
                lw = lw + (rw + carry);
                carry = 0;
            }
        }
        ((UInt32*)&(lhs))[i] = lw;
    }
    return lhs;
}

CInt64 CInt64_Mul(CInt64 lhs, CInt64 rhs) {
    CInt64 a;
    CInt64 b;
    SInt32 carry;
    int i;
    UInt32 lw;
    UInt32 rw;

    if (((UInt32*)&rhs)[0] & 0x80000000) {
        if (((UInt32*)&lhs)[0] & 0x80000000) {
            a = lhs;
            CINT64_NEG_IN_PLACE(a)
            b = rhs;
            CINT64_NEG_IN_PLACE(b)
            return CInt64_MulU(a, b);
        }
        b = rhs;
        CINT64_NEG_IN_PLACE(b)
        a = CInt64_MulU(lhs, b);
        CINT64_NEG_IN_PLACE(a)
        return a;
    }
    if (((UInt32*)&lhs)[0] & 0x80000000) {
        a = lhs;
        CINT64_NEG_IN_PLACE(a)
        b = CInt64_MulU(a, rhs);
        CINT64_NEG_IN_PLACE(b)
        return b;
    }
    return CInt64_MulU(lhs, rhs);
}

CInt64 CInt64_MulU(CInt64 lhs, CInt64 rhs) {
    CInt64 result;
    CInt64 work;
    UInt32 word;
    int j;
    int i;
    SInt32 carry;
    UInt32 lw;
    UInt32 rw;

    for (i = 0; i < 3; i++) {
        if (((UInt32*)&lhs)[i] != 0 || ((UInt32*)&rhs)[i] != 0) {
            break;
        }
    }
    if (i == 3 && ((lhs.lo | rhs.lo) & 0xFFFF0000) == 0) {
        CInt64_SetLong(&result, lhs.lo * rhs.lo);
        return result;
    }

    result.hi = 0;
    result.unk4 = 0;
    result.unk8 = 0;
    result.lo = 0;

    for (j = 0; j < 4; j++) {
        word = ((UInt32*)&rhs)[3 - j];
        if (word == 0) {
            continue;
        }
        for (i = 0; i < 4; i++) {
            if (i + j < 4) {
                ((UInt32*)&work)[i] = ((UInt32*)&lhs)[i + j];
            } else {
                ((UInt32*)&work)[i] = 0;
            }
        }
        for (;;) {
            if (word & 1) {
                carry = 0;
                for (i = 3; i >= 0; i--) {
                    lw = ((UInt32*)&result)[i];
                    rw = ((UInt32*)&work)[i];
                    if (lw & 0x80000000) {
                        if (rw & 0x80000000) {
                            lw = lw + (rw + carry);
                            carry = 1;
                        } else {
                            lw = lw + rw;
                            if (carry) {
                                carry = 0;
                                if (!(lw & 0x80000000)) {
                                    carry = 1;
                                }
                                lw = lw + 1;
                                if (!(lw & 0x80000000)) {
                                    carry = 1;
                                }
                            } else {
                                if (lw & 0x80000000) {
                                    carry = 0;
                                } else {
                                    carry = 1;
                                }
                            }
                        }
                    } else {
                        if (rw & 0x80000000) {
                            lw = lw + rw;
                            if (carry) {
                                carry = 0;
                                if (!(lw & 0x80000000)) {
                                    carry = 1;
                                }
                                lw = lw + 1;
                                if (!(lw & 0x80000000)) {
                                    carry = 1;
                                }
                            } else {
                                if (lw & 0x80000000) {
                                    carry = 0;
                                } else {
                                    carry = 1;
                                }
                            }
                        } else {
                            lw = lw + (rw + carry);
                            carry = 0;
                        }
                    }
                    ((UInt32*)&result)[i] = lw;
                }
            }
            word >>= 1;
            if (word == 0) {
                break;
            }
            for (i = 0; i < 3; i++) {
                ((UInt32*)&work)[i] <<= 1;
                if (((UInt32*)&work)[i + 1] & 0x80000000) {
                    ((UInt32*)&work)[i] |= 1;
                }
            }
            ((UInt32*)&work)[3] <<= 1;
        }
    }
    return result;
}

static void CInt64_DivMod(const CInt64* lhs, const CInt64* rhs, CInt64* pDiv, CInt64* pMod) {
    Boolean bad;
    CInt64 right;
    CInt64 work;
    CInt64 negRight;
    CInt64 left;
    CInt64 out;
    int counter;
    int i;
    SInt32 carry;
    UInt32 lw;
    UInt32 rw;

    bad = TRUE;
    for (i = 0; i < 4; i++) {
        if (((const UInt32*)rhs)[i] != 0) {
            bad = FALSE;
            break;
        }
    }
    if (bad) {
        return;
    }

    for (i = 0; i < 3; i++) {
        if (((const UInt32*)lhs)[i] != 0 || ((const UInt32*)rhs)[i] != 0) {
            break;
        }
    }
    if (i == 3) {
        if (pDiv) {
            pDiv->hi = 0;
            pDiv->unk4 = 0;
            pDiv->unk8 = 0;
            pDiv->lo = lhs->lo / rhs->lo;
        }
        if (pMod) {
            pMod->hi = 0;
            pMod->unk4 = 0;
            pMod->unk8 = 0;
            pMod->lo = lhs->lo % rhs->lo;
        }
        return;
    }

    work = cint64_zero;
    left = *lhs;
    out = cint64_zero;
    right = *rhs;
    right.hi = ~right.hi;
    right.unk4 = ~right.unk4;
    right.unk8 = ~right.unk8;
    carry = 0;
    for (i = 3; i >= 0; i--) {
        lw = ((UInt32*)&(right))[i];
        rw = ((UInt32*)&(cint64_one))[i];
        if (lw & 0x80000000) {
            if (rw & 0x80000000) {
                lw = lw + (rw + carry);
                carry = 1;
            } else {
                lw = lw + rw;
                if (carry) {
                    carry = 0;
                    if (!(lw & 0x80000000)) {
                        carry = 1;
                    }
                    lw = lw + 1;
                    if (!(lw & 0x80000000)) {
                        carry = 1;
                    }
                } else {
                    if (lw & 0x80000000) {
                        carry = 0;
                    } else {
                        carry = 1;
                    }
                }
            }
        } else {
            if (rw & 0x80000000) {
                lw = lw + rw;
                if (carry) {
                    carry = 0;
                    if (!(lw & 0x80000000)) {
                        carry = 1;
                    }
                    lw = lw + 1;
                    if (!(lw & 0x80000000)) {
                        carry = 1;
                    }
                } else {
                    if (lw & 0x80000000) {
                        carry = 0;
                    } else {
                        carry = 1;
                    }
                }
            } else {
                lw = lw + (rw + carry);
                carry = 0;
            }
        }
        ((UInt32*)&(right))[i] = lw;
    }
    negRight = right;

    for (counter = 0; counter < 128; counter++) {
        for (i = 0; i < 3; i++) {
            ((UInt32*)&work)[i] <<= 1;
            if (((UInt32*)&work)[i + 1] & 0x80000000) {
                ((UInt32*)&work)[i] |= 1;
            }
        }
        ((UInt32*)&work)[3] <<= 1;
        if (((UInt32*)&left)[0] & 0x80000000) {
            ((UInt32*)&work)[3] |= 1;
        }
        for (i = 0; i < 3; i++) {
            ((UInt32*)&left)[i] <<= 1;
            if (((UInt32*)&left)[i + 1] & 0x80000000) {
                ((UInt32*)&left)[i] |= 1;
            }
        }
        ((UInt32*)&left)[3] <<= 1;
        for (i = 0; i < 3; i++) {
            ((UInt32*)&out)[i] <<= 1;
            if (((UInt32*)&out)[i + 1] & 0x80000000) {
                ((UInt32*)&out)[i] |= 1;
            }
        }
        ((UInt32*)&out)[3] <<= 1;
        if (CInt64_UnsignedCompare(&work, rhs) >= 0) {
            ((UInt32*)&out)[3] |= 1;
            carry = 0;
            for (i = 3; i >= 0; i--) {
                lw = ((UInt32*)&(work))[i];
                rw = ((UInt32*)&(negRight))[i];
                if (lw & 0x80000000) {
                    if (rw & 0x80000000) {
                        lw = lw + (rw + carry);
                        carry = 1;
                    } else {
                        lw = lw + rw;
                        if (carry) {
                            carry = 0;
                            if (!(lw & 0x80000000)) {
                                carry = 1;
                            }
                            lw = lw + 1;
                            if (!(lw & 0x80000000)) {
                                carry = 1;
                            }
                        } else {
                            if (lw & 0x80000000) {
                                carry = 0;
                            } else {
                                carry = 1;
                            }
                        }
                    }
                } else {
                    if (rw & 0x80000000) {
                        lw = lw + rw;
                        if (carry) {
                            carry = 0;
                            if (!(lw & 0x80000000)) {
                                carry = 1;
                            }
                            lw = lw + 1;
                            if (!(lw & 0x80000000)) {
                                carry = 1;
                            }
                        } else {
                            if (lw & 0x80000000) {
                                carry = 0;
                            } else {
                                carry = 1;
                            }
                        }
                    } else {
                        lw = lw + (rw + carry);
                        carry = 0;
                    }
                }
                ((UInt32*)&(work))[i] = lw;
            }
        }
    }

    if (pDiv) {
        *pDiv = out;
    }
    if (pMod) {
        *pMod = work;
    }
}

CInt64 CInt64_DivU(CInt64 lhs, CInt64 rhs) {
    CInt64 result;

    CInt64_DivMod(&lhs, &rhs, &result, 0);
    return result;
}

CInt64 CInt64_ModU(CInt64 lhs, CInt64 rhs) {
    CInt64 result;

    CInt64_DivMod(&lhs, &rhs, 0, &result);
    return result;
}

CInt64 CInt64_Shl(CInt64 lhs, CInt64 rhs) {
    int i;
    int counter;
    int j;

    for (i = 0; i < 3; i++) {
        if (((UInt32*)&rhs)[i] != 0) {
            return cint64_zero;
        }
    }
    if (((UInt32*)&rhs)[i] >= 0x80) {
        return cint64_zero;
    }
    for (counter = ((UInt32*)&rhs)[i]; counter > 0; counter--) {
        for (j = 0; j < 3; j++) {
            ((UInt32*)&lhs)[j] <<= 1;
            if (((UInt32*)&lhs)[j + 1] & 0x80000000) {
                ((UInt32*)&lhs)[j] |= 1;
            }
        }
        lhs.lo <<= 1;
    }
    return lhs;
}

CInt64 CInt64_ShrU(CInt64 lhs, CInt64 rhs) {
    int i;
    int counter;

    for (i = 0; i < 3; i++) {
        if (((UInt32*)&rhs)[i] != 0) {
            return cint64_zero;
        }
    }
    if (((UInt32*)&rhs)[i] >= 0x80) {
        return cint64_zero;
    }
    for (counter = ((UInt32*)&rhs)[i]; counter > 0; counter--) {
        for (i = 3; i > 0; i--) {
            ((UInt32*)&lhs)[i] >>= 1;
            if (((UInt32*)&lhs)[i - 1] & 1) {
                ((UInt32*)&lhs)[i] |= 0x80000000;
            }
        }
        ((UInt32*)&lhs)[0] >>= 1;
    }
    return lhs;
}

static int CInt64_UnsignedCompare(const CInt64* lhs, const CInt64* rhs) {
    int i;

    for (i = 0; i < 4; i++) {
        if (((const UInt32*)lhs)[i] != ((const UInt32*)rhs)[i]) {
            if ((UInt32)((const UInt32*)lhs)[i] < (UInt32)((const UInt32*)rhs)[i]) {
                return -1;
            } else {
                return 1;
            }
        }
    }
    return 0;
}

static int CInt64_SignedCompare(const CInt64* lhs, const CInt64* rhs) {
    CInt64 lhs_;
    CInt64 rhs_;

    lhs_ = *lhs;
    lhs_.hi ^= 0x80000000;
    rhs_ = *rhs;
    rhs_.hi ^= 0x80000000;
    return CInt64_UnsignedCompare(&lhs_, &rhs_);
}

Boolean CInt64_Less(CInt64 lhs, CInt64 rhs) {
    return CInt64_SignedCompare(&lhs, &rhs) < 0;
}

Boolean CInt64_LessU(CInt64 lhs, CInt64 rhs) {
    return CInt64_UnsignedCompare(&lhs, &rhs) < 0;
}

Boolean CInt64_Greater(CInt64 lhs, CInt64 rhs) {
    return CInt64_SignedCompare(&lhs, &rhs) > 0;
}

Boolean CInt64_GreaterU(CInt64 lhs, CInt64 rhs) {
    return CInt64_UnsignedCompare(&lhs, &rhs) > 0;
}

Boolean CInt64_LessEqual(CInt64 lhs, CInt64 rhs) {
    return CInt64_SignedCompare(&lhs, &rhs) <= 0;
}

Boolean CInt64_LessEqualU(CInt64 lhs, CInt64 rhs) {
    return CInt64_UnsignedCompare(&lhs, &rhs) <= 0;
}

Boolean CInt64_GreaterEqual(CInt64 lhs, CInt64 rhs) {
    return CInt64_SignedCompare(&lhs, &rhs) >= 0;
}

Boolean CInt64_GreaterEqualU(CInt64 lhs, CInt64 rhs) {
    return CInt64_UnsignedCompare(&lhs, &rhs) >= 0;
}

Boolean CInt64_NotEqual(CInt64 lhs, CInt64 rhs) {
    int i;

    for (i = 0; i < 4; i++) {
        if (((UInt32*)&lhs)[i] != ((UInt32*)&rhs)[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

Boolean CInt64_Equal(CInt64 lhs, CInt64 rhs) {
    int i;

    for (i = 0; i < 4; i++) {
        if (((UInt32*)&lhs)[i] != ((UInt32*)&rhs)[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

Boolean CInt64_IsInRange(CInt64 value, short len) {
    int i;
    int last;

    if (len >= 16) {
        return TRUE;
    }
    if (((UInt32*)&value)[0] & 0x80000000) {
        value = CInt64_Inv(value);
    }
    last = 3 - ((len - 1) >> 2);
    for (i = 0; i < last; i++) {
        if (((UInt32*)&value)[i] != 0) {
            return FALSE;
        }
    }
    switch (len & 3) {
        case 1:
        default:
            return (((UInt32*)&value)[i] & 0xFFFFFF80) == 0;
        case 2:
            return (((UInt32*)&value)[i] & 0xFFFF8000) == 0;
        case 3:
            return (((UInt32*)&value)[i] & 0xFF800000) == 0;
        case 0:
            return (((UInt32*)&value)[i] & 0x80000000) == 0;
    }
}

Boolean CInt64_IsInURange(CInt64 value, short len) {
    int i;
    int last;

    if (len >= 16) {
        return TRUE;
    }
    last = 3 - ((len - 1) >> 2);
    for (i = 0; i < last; i++) {
        if (((UInt32*)&value)[i] != 0) {
            return FALSE;
        }
    }
    switch (len & 3) {
        case 0:
        default:
            return TRUE;
        case 1:
            return (((UInt32*)&value)[i] & 0xFFFFFF00) == 0;
        case 2:
            return (((UInt32*)&value)[i] & 0xFFFF0000) == 0;
        case 3:
            return (((UInt32*)&value)[i] & 0xFF000000) == 0;
    }
}

CInt64 CInt64_And(CInt64 lhs, CInt64 rhs) {
    UInt32 w0;
    UInt32 w1;
    UInt32 w2;
    UInt32 w3;

    w3 = rhs.lo;
    w2 = rhs.unk8;
    w1 = rhs.unk4;
    w0 = rhs.hi;
    lhs.hi &= w0;
    lhs.unk4 &= w1;
    lhs.unk8 &= w2;
    lhs.lo &= w3;
    return lhs;
}

CInt64 CInt64_Xor(CInt64 lhs, CInt64 rhs) {
    UInt32 w0;
    UInt32 w1;
    UInt32 w2;
    UInt32 w3;

    w3 = rhs.lo;
    w2 = rhs.unk8;
    w1 = rhs.unk4;
    w0 = rhs.hi;
    lhs.hi ^= w0;
    lhs.unk4 ^= w1;
    lhs.unk8 ^= w2;
    lhs.lo ^= w3;
    return lhs;
}

CInt64 CInt64_Or(CInt64 lhs, CInt64 rhs) {
    UInt32 w0;
    UInt32 w1;
    UInt32 w2;
    UInt32 w3;

    w3 = rhs.lo;
    w2 = rhs.unk8;
    w1 = rhs.unk4;
    w0 = rhs.hi;
    lhs.hi |= w0;
    lhs.unk4 |= w1;
    lhs.unk8 |= w2;
    lhs.lo |= w3;
    return lhs;
}

void CInt64_ConvertInt64(CInt64* i) {
    UInt32 ext;

    ext = (i->unk8 & 0x80000000) ? 0xFFFFFFFF : 0;
    i->hi = ext;
    i->unk4 = ext;
}

void CInt64_ConvertUInt64(CInt64* i) {
    i->hi = 0;
    i->unk4 = 0;
}

void CInt64_ConvertInt32(CInt64* i) {
    UInt32 ext;

    ext = (i->lo & 0x80000000) ? 0xFFFFFFFF : 0;
    i->hi = ext;
    i->unk4 = ext;
    i->unk8 = ext;
}

void CInt64_ConvertUInt32(CInt64* i) {
    i->hi = 0;
    i->unk4 = 0;
    i->unk8 = 0;
}

void CInt64_ConvertInt16(CInt64* i) {
    UInt32 ext;

    i->lo = (SInt16)i->lo;
    ext = (i->lo & 0x80000000) ? 0xFFFFFFFF : 0;
    i->hi = ext;
    i->unk4 = ext;
    i->unk8 = ext;
}

void CInt64_ConvertUInt16(CInt64* i) {
    i->lo = (UInt16)i->lo;
    i->hi = 0;
    i->unk4 = 0;
    i->unk8 = 0;
}

void CInt64_ConvertInt8(CInt64* i) {
    UInt32 ext;

    i->lo = (SInt8)i->lo;
    ext = (i->lo & 0x80000000) ? 0xFFFFFFFF : 0;
    i->hi = ext;
    i->unk4 = ext;
    i->unk8 = ext;
}

void CInt64_ConvertUInt8(CInt64* i) {
    i->lo = (UInt8)i->lo;
    i->hi = 0;
    i->unk4 = 0;
    i->unk8 = 0;
}
#define CINT64_ADD_CARRY(l, r, initial) \
    carry = (initial); \
    i = 3; \
    while (i >= 0) { \
        lw = ((UInt32*)&(l))[i]; \
        rw = ((UInt32*)&(r))[i]; \
        if (lw & 0x80000000) { \
            if (rw & 0x80000000) { \
                lw = lw + (rw + carry); \
                carry = 1; \
            } else { \
                lw = lw + rw; \
                if (carry) { \
                    carry = 0; \
                    if (!(lw & 0x80000000)) { \
                        carry = 1; \
                    } \
                    lw = lw + 1; \
                    if (!(lw & 0x80000000)) { \
                        carry = 1; \
                    } \
                } else { \
                    if (lw & 0x80000000) { \
                        carry = 0; \
                    } else { \
                        carry = 1; \
                    } \
                } \
            } \
        } else { \
            if (rw & 0x80000000) { \
                lw = lw + rw; \
                if (carry) { \
                    carry = 0; \
                    if (!(lw & 0x80000000)) { \
                        carry = 1; \
                    } \
                    lw = lw + 1; \
                    if (!(lw & 0x80000000)) { \
                        carry = 1; \
                    } \
                } else { \
                    if (lw & 0x80000000) { \
                        carry = 0; \
                    } else { \
                        carry = 1; \
                    } \
                } \
            } else { \
                lw = lw + (rw + carry); \
                carry = 0; \
            } \
        } \
        ((UInt32*)&(l))[i] = lw; \
        i = i - 1; \
    }

static Boolean powers_built;
static double next_power;
static double powers_of_two[129];

static Boolean powers_built;
static double next_power;
static double powers_of_two[129];

static double power_of_two(short n) {
    int i;

    if (!powers_built) {
        i = 0;
        next_power = 1.0;
        do {
            powers_of_two[i] = next_power;
            i = i + 1;
            next_power = next_power + next_power;
        } while (i < 129);
        powers_built = TRUE;
    }
    return powers_of_two[n];
}

void CInt64_ConvertUFromLongDouble(CInt64* pResult, double value) {
    int i;
    int bit;
    UInt32 mask;

    if (value <= 0.0) {
        if (value == 0.0) {
            *pResult = cint64_zero;
            return;
        }
    }
    if (value >= power_of_two(128)) {
        *pResult = cint64_negone;
        return;
    }

    *pResult = cint64_zero;
    i = 0;
    mask = 0x80000000;
    for (bit = 127; bit >= 0; bit--) {
        if (mask == 0) {
            mask = 0x80000000;
            i = i + 1;
        }
        if (power_of_two(bit) <= value) {
            ((UInt32*)pResult)[i] |= mask;
            value = value - power_of_two(bit);
        }
        mask >>= 1;
    }
}

void CInt64_ConvertFromLongDouble(CInt64* pResult, double value) {
    CInt64 work;
    SInt32 carry;
    int i;
    UInt32 lw;
    UInt32 rw;

    if (value < 0.0) {
        CInt64_ConvertUFromLongDouble(pResult, -value);
        work = *pResult;
        CINT64_NEG_IN_PLACE(work)
        *pResult = work;
    } else {
        CInt64_ConvertUFromLongDouble(pResult, value);
    }
}

double CInt64_ConvertUToLongDouble(const CInt64* value) {
    double result;
    Boolean zero;
    UInt32 word;
    int i;
    int j;

    result = 0.0;
    zero = TRUE;
    for (i = 0; i < 4; i++) {
        if (((const UInt32*)value)[i] != 0) {
            zero = FALSE;
            break;
        }
    }
    if (zero) {
        return result;
    }

    for (i = 0; i < 4; i++) {
        word = ((const UInt32*)value)[i];
        for (j = 0; j < 32; j++) {
            result += result;
            if (word & 0x80000000) {
                result = result + 1.0;
            }
            word <<= 1;
        }
    }
    return result;
}

double CInt64_ConvertToLongDouble(const CInt64* value) {
    CInt64 work;
    CInt64 magnitude;
    SInt32 carry;
    int i;
    UInt32 lw;
    UInt32 rw;

    if (!(value->hi & 0x80000000)) {
        return CInt64_ConvertUToLongDouble(value);
    }

    work = *value;
    CINT64_NEG_IN_PLACE(work)
    magnitude = work;
    return -CInt64_ConvertUToLongDouble(&magnitude);
}

char* CInt64_ScanOctString(CInt64* pResult, char* str, Boolean* pFail) {
    char ch;
    CInt64 value;
    CInt64 addend;
    CInt64 rhs;
    SInt32 carry;
    int j;
    int i;
    UInt32 lw;
    UInt32 rw;

    *pFail = FALSE;
    pResult->hi = 0;
    pResult->unk4 = 0;
    pResult->unk8 = 0;
    pResult->lo = 0;

    while ((ch = *str) >= '0' && ch <= '7') {
        if (pResult->hi & 0xE0000000) {
            *pFail = TRUE;
        }

        for (j = 0; j < 3; j++) {
            for (i = 0; i < 3; i++) {
                ((UInt32*)pResult)[i] <<= 1;
                if (((UInt32*)pResult)[i + 1] & 0x80000000) {
                    ((UInt32*)pResult)[i] |= 1;
                }
            }
            pResult->lo <<= 1;
        }

        CInt64_SetLong(&addend, ch - '0');
        rhs = addend;
        value = *pResult;
        CINT64_ADD_CARRY(value, rhs, 0)
        *pResult = value;
        ++str;
    }

    return str;
}

char* CInt64_ScanDecString(CInt64* pResult, char* str, Boolean* pFail) {
    char ch;
    CInt64 rhs4;
    CInt64 addend2;
    CInt64 work;
    CInt64 rhs1;
    CInt64 value1;
    CInt64 rhs3;
    CInt64 value4;
    CInt64 value3;
    CInt64 value2;
    SInt32 carry;
    int i;
    int j;
    UInt32 lw;
    UInt32 rw;

    *pFail = FALSE;
    pResult->hi = 0;
    pResult->unk4 = 0;
    pResult->unk8 = 0;
    pResult->lo = 0;

    while ((ch = *str) >= '0' && ch <= '9') {
        if (pResult->hi & 0xE0000000) {
            *pFail = TRUE;
        }

        work = *pResult;
        for (i = 0; i < 3; i++) {
            ((UInt32*)&work)[i] <<= 1;
            if (((UInt32*)&work)[i + 1] & 0x80000000) {
                ((UInt32*)&work)[i] |= 1;
            }
        }
        work.lo <<= 1;
        *pResult = work;

        for (j = 0; j < 2; j++) {
            for (i = 0; i < 3; i++) {
                ((UInt32*)pResult)[i] <<= 1;
                if (((UInt32*)pResult)[i + 1] & 0x80000000) {
                    ((UInt32*)pResult)[i] |= 1;
                }
            }
            pResult->lo <<= 1;
        }

        if (pResult->hi & 0x80000000) {
            rhs1 = work;
            value1 = *pResult;
            CINT64_ADD_CARRY(value1, rhs1, 0)
            *pResult = value1;
            if (!(pResult->hi & 0x80000000)) {
                *pFail = TRUE;
            }
        } else {
            addend2 = work;
            value2 = *pResult;
            CINT64_ADD_CARRY(value2, addend2, 0)
            *pResult = value2;
        }

        CInt64_SetLong(&work, ch - '0');
        if (pResult->hi & 0x80000000) {
            rhs3 = work;
            value3 = *pResult;
            CINT64_ADD_CARRY(value3, rhs3, 0)
            *pResult = value3;
            if (!(pResult->hi & 0x80000000)) {
                *pFail = TRUE;
            }
        } else {
            rhs4 = work;
            value4 = *pResult;
            CINT64_ADD_CARRY(value4, rhs4, 0)
            *pResult = value4;
        }

        ++str;
    }

    return str;
}

char* CInt64_ScanHexString(CInt64* pResult, char* str, Boolean* pFail) {
    char digit;
    CInt64 value;
    CInt64 addend;
    CInt64 rhs;
    SInt32 carry;
    int i;
    int j;
    UInt32 lw;
    UInt32 rw;

    *pFail = FALSE;
    pResult->hi = 0;
    pResult->unk4 = 0;
    pResult->unk8 = 0;
    pResult->lo = 0;

    for (;;) {
        digit = str[0];
        if (digit >= '0' && digit <= '9') {
            digit -= '0';
        } else if (digit >= 'A' && digit <= 'F') {
            digit -= 'A' - 10;
        } else if (digit >= 'a' && digit <= 'f') {
            digit -= 'a' - 10;
        } else {
            break;
        }

        ++str;
        if (pResult->hi & 0xF0000000) {
            *pFail = TRUE;
        }

        for (j = 0; j < 4; j++) {
            for (i = 0; i < 3; i++) {
                ((UInt32*)pResult)[i] <<= 1;
                if (((UInt32*)pResult)[i + 1] & 0x80000000) {
                    ((UInt32*)pResult)[i] |= 1;
                }
            }
            pResult->lo <<= 1;
        }

        CInt64_SetLong(&addend, digit);
        rhs = addend;
        value = *pResult;
        CINT64_ADD_CARRY(value, rhs, 0)
        *pResult = value;
    }

    return str;
}

char* CInt64_ScanBinString(CInt64* pResult, char* str, Boolean* pFail) {
    char digit;
    CInt64 value;
    CInt64 one;
    SInt32 carry;
    int i;
    UInt32 lw;
    UInt32 rw;

    *pFail = FALSE;
    pResult->hi = 0;
    pResult->unk4 = 0;
    pResult->unk8 = 0;
    pResult->lo = 0;

    for (;;) {
        if (*str == '0') {
            digit = 0;
        } else if (*str == '1') {
            digit = 1;
        } else {
            break;
        }
        ++str;
        if (pResult->hi & 0x80000000) {
            *pFail = TRUE;
        }
        for (i = 0; i < 3; i++) {
            ((UInt32*)pResult)[i] <<= 1;
            if (((UInt32*)pResult)[i + 1] & 0x80000000) {
                ((UInt32*)pResult)[i] |= 1;
            }
        }
        pResult->lo <<= 1;
        if (digit == 1) {
            one = cint64_one;
            value = *pResult;
            CINT64_ADD_CARRY(value, one, 0)
            *pResult = value;
        }
    }

    return str;
}

int CInt64_PrintDec(char* output, CInt64 value) {
    int length;
    CInt64 work;
    char buf[32];
    CInt64 rem;
    CInt64 rhs2;
    CInt64 lhs2;
    CInt64 divisor;
    char* bufp;
    SInt32 carry;
    int i;
    UInt32 lw;
    UInt32 rw;

    length = 0;
    if (value.hi & 0x80000000) {
        work = value;
        CINT64_NEG_IN_PLACE(work)
        value = work;
        *output = '-';
        output++;
        length++;
    }

    if (!CInt64_IsZero(&value)) {
        divisor.hi = 0;
        divisor.unk4 = 0;
        divisor.unk8 = 0;
        divisor.lo = 10;

        bufp = buf;
        for (;;) {
            rhs2 = divisor;
            lhs2 = value;
            CInt64_DivMod(&lhs2, &rhs2, 0, &rem);
            *(bufp++) = rem.lo + '0';
            rhs2 = divisor;
            lhs2 = value;
            CInt64_DivMod(&lhs2, &rhs2, &value, 0);
            if (CInt64_IsZero(&value)) {
                break;
            }
        }

        while (--bufp >= buf) {
            *(output++) = *bufp;
            length++;
        }
    } else {
        *(output++) = '0';
        length++;
    }

    *output = 0;
    return length;
}

void fn_00455C80(UInt8* dst, const CInt64* number, SInt32 size) {
    SInt32 i;
    SInt32 byteIndex;
    SInt32 wordIndex;
    UInt32 word;

    i = 0;
    byteIndex = 0;
    wordIndex = 4;
    if (size > 0) {
        do {
            byteIndex--;
            if (byteIndex < 0) {
                wordIndex--;
                word = ((const UInt32*)number)[wordIndex];
                byteIndex = 3;
            }
            *dst = word;
            i++;
            dst++;
            word >>= 8;
        } while (i < size);
    }
}
