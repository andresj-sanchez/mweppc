#include "compiler/BE_macro.h"
#include "compiler/Operands.h"
#include "compiler/CError.h"
#include "compiler/DIVbyCnst.h"
#include "compiler/CompilerTools.h"
#include "compiler/mips.h"
#include "compiler/VReg.h"
#include "compiler/CParser.h"
#include "compiler/Exceptions.h"

DivisionParameters compute_unsigned_division_parameters(UInt32 divisor) {
    DivisionParameters params;
    UInt32 nc;
    UInt32 delta;
    UInt32 q1;
    UInt32 r1;
    UInt32 q2;
    UInt32 r2;
    int p;

    if (divisor <= 1) {
        CError_Internal("DIVbyCnst.c", 54);
    }

    params.addIndicator = 0;
    p = 31;
    q1 = 0x80000000;
    r1 = 0x80000000;
    nc = 0xFFFFFFFF - (-divisor) % divisor;
    q1 = 0x80000000 / nc;
    r1 = 0x80000000 - q1 * nc;
    q2 = 0x7FFFFFFF / divisor;
    r2 = 0x7FFFFFFF - q2 * divisor;

    do {
        p++;
        if (r1 >= nc - r1) {
            q1 = 2 * q1 + 1;
            r1 = 2 * r1 - nc;
        } else {
            q1 = 2 * q1;
            r1 = 2 * r1;
        }
        if (r2 + 1 >= divisor - r2) {
            if (q2 >= 0x7FFFFFFF) {
                params.addIndicator = 1;
            }
            q2 = 2 * q2 + 1;
            r2 = 2 * r2 + 1 - divisor;
        } else {
            if (q2 >= 0x80000000) {
                params.addIndicator = 1;
            }
            q2 = 2 * q2;
            r2 = 2 * r2 + 1;
        }
        delta = divisor - 1 - r2;
    } while (p < 64 && (q1 < delta || (q1 == delta && r1 == 0)));

    params.multiplier = q2 + 1;
    params.shift = p - 32;
    return params;
}

ConstInfo2 compute_signed_division_multiplier_shift(SInt32 divisor) {
    ConstInfo2 params;
    UInt32 delta;
    UInt32 ad;
    UInt32 anc;
    UInt32 q1, r1, q2, r2;
    SInt32 p;

    ad = __builtin_abs(divisor);
    if (ad <= 2) {
        CError_Internal("DIVbyCnst.c", 14);
    }
    anc = 0x80000000u + ((UInt32)divisor >> 31);
    r1 = anc % ad;
    anc = anc - 1 - r1;
    q1 = 0x80000000u / anc;
    r1 = 0x80000000u - q1 * anc;
    p = 31;
    q2 = 0x80000000u / ad;
    r2 = 0x80000000u - q2 * ad;
    do {
        p++;
        r1 = 2 * r1;
        q1 = 2 * q1;
        if (r1 >= anc) {
            q1++;
            r1 -= anc;
        }
        r2 = 2 * r2;
        q2 = 2 * q2;
        if (r2 >= ad) {
            q2++;
            r2 -= ad;
        }
        delta = ad - r2;
    } while (q1 < delta || (q1 == delta && r1 == 0));

    params.multiplier = q2 + 1;
    if (divisor < 0) {
        params.multiplier = -params.multiplier;
    }
    params.shift = p - 32;
    return params;
}

int fn_0054F3B0(int number, Type* type) {
    if (((type->type == TYPEINT) || (type->type == TYPEENUM)) &&
        (type->size == 8) &&
        copts.unknown25) {
        return 20;
    }
    if (number < 0) {
        if (number <= -65536) {
            return 10;
        }
        return 6;
    }
    if (number >= 65536) {
        return 10;
    }
    return 6;
}

#include "compiler/CMangler.h"
#include "compiler/CodeGen.h"
#include "compiler/RegisterInfo.h"

void fn_00550790(GList* gl) {
    SInt32 mask;
    SInt8 flags = starting_available_registers[0];

    if (codegen_flag_63738A) {
        flags |= 0x20;
    }
    if ((copts.fp_library & 1) && be_macro_debug_register_count_636930) {
        flags |= 0x40;
    }
    AppendGListByte(gl, flags);
    mask = stack_alignment_63560C - 1;
    fn_004AA820(((codegen_operand_max_6363EC + mask) & ~mask) + asm_stack_frame_offset_636E78);
    fn_004AA820(be_macro_adjust_offset_6356A4);
    if (copts.fp_library & 1) {
        SInt32 count = be_macro_debug_register_count_636930;
        if (count) {
            fn_004AA820(count);
        }
    }
}

void fn_0054F190(Operand* input) {
    MulDivCacheEntry* entry;
    MulDivCacheEntry* opposite;
    int i;
    volatile SInt32 one;
    SInt32 bucket;
    for (i = 0; i < 511; i++) {
        division_cache_heads_61E1F4[i] = NULL;
    }
    one = 1;
    bucket = ((SInt32)((one ^ (one >> 31)) - (one >> 31)) % 511);
    for (entry = division_cache_heads_61E1F4[bucket]; entry != NULL && entry->multiplier != 1;
         entry = entry->next) {}
    if (entry == NULL) {
        entry = lalloc(0x40);
        memclrw(entry, sizeof(*entry));
        entry->multiplier = 1;
        entry->ref = NULL;
        entry->next = division_cache_heads_61E1F4[bucket];
        division_cache_heads_61E1F4[bucket] = entry;
    }
    entry->ref = entry;
    entry->initialized = 0;
    entry->cost = 0;
    entry->result = *input;
    one = -1;
    bucket = ((SInt32)((one ^ (one >> 31)) - (one >> 31)) % 511);
    for (opposite = division_cache_heads_61E1F4[bucket];
         opposite != NULL && opposite->multiplier != -1;
         opposite = opposite->next) {}
    if (opposite == NULL) {
        opposite = lalloc(0x40);
        memclrw(opposite, sizeof(*opposite));
        opposite->multiplier = -1;
        opposite->ref = NULL;
        opposite->next = division_cache_heads_61E1F4[bucket];
        division_cache_heads_61E1F4[bucket] = opposite;
    }
    opposite->ref = entry;
    opposite->initialized = 1;
    opposite->cost = 1;
}

#define TRY_MUL_FACTOR(factor, operationCost, operationKind) \
    do { \
        UInt32 limit = cache->cost - (operationCost); \
        MulDivCacheEntry* child = fn_00550360((factor), limit); \
        if (child->ref != NULL && (UInt32)child->cost < limit) { \
            cache->ref = child; \
            cache->initialized = (operationKind); \
            cache->cost = child->cost + (operationCost); \
        } \
    } while (0)

MulDivCacheEntry* fn_00550360(UInt32 multiplier, UInt32 budget) {
    UInt32 quotient;
    UInt32 cost;
    SInt32 hash;
    SInt32 power;
    SInt32 opCost;
    MulDivCacheEntry* cache;
    hash = __builtin_abs((SInt32)multiplier) % 511;
    for (cache = division_cache_heads_61E1F4[hash];
         cache != NULL && cache->multiplier != (SInt32)multiplier;
         cache = cache->next) {}
    if (cache == NULL) {
        UInt32* p;
        int n;
        cache = lalloc(0x40);
        p = (UInt32*)cache;
        for (n = 16; n != 0; n--) {
            *p++ = 0;
        }
        cache->multiplier = multiplier;
        cache->ref = NULL;
        cache->next = division_cache_heads_61E1F4[hash];
        division_cache_heads_61E1F4[hash] = cache;
    }
    power = 4;
    if (cache->ref == NULL && (UInt32)cache->cost < budget) {
        cache->cost = budget;
        if ((SInt32)multiplier <= 0) {
            if (4 < ((SInt32)-multiplier >> 1)) {
                do {
                    opCost = mulcost_negaddshift_5EA358;
                    if ((SInt32)multiplier % (1 - power) == 0) {
                        TRY_MUL_FACTOR((SInt32)multiplier / (1 - power), opCost, 7);
                    }
                    opCost = mulcost_addshift_5EA350;
                    if ((SInt32)multiplier % (power + 1) == 0) {
                        TRY_MUL_FACTOR((SInt32)multiplier / (power + 1), opCost, 5);
                    }
                    power <<= 1;
                } while (power < ((SInt32)-multiplier >> 1));
            }
            opCost = mulcost_negsub_5EA34C;
            quotient = 1 - multiplier;
            do {
                quotient = (SInt32)((quotient + 1) - (quotient < 0x80000000)) >> 1;
            } while ((quotient & 1) == 0);
            TRY_MUL_FACTOR(quotient, opCost, 4);
            opCost = mulcost_sub_5EA348;
            multiplier++;
            do {
                multiplier = (SInt32)((multiplier + 1) - (multiplier < 0x80000000)) >> 1;
            } while ((multiplier & 1) == 0);
            TRY_MUL_FACTOR(multiplier, opCost, 3);
        } else {
            if (4 < ((SInt32)multiplier >> 1)) {
                do {
                    opCost = mulcost_subshift_5EA354;
                    if ((SInt32)multiplier % (power - 1) == 0) {
                        TRY_MUL_FACTOR((SInt32)multiplier / (power - 1), opCost, 6);
                    }
                    opCost = mulcost_addshift_5EA350;
                    if ((SInt32)multiplier % (power + 1) == 0) {
                        TRY_MUL_FACTOR((SInt32)multiplier / (power + 1), opCost, 5);
                    }
                    power <<= 1;
                } while (power < ((SInt32)multiplier >> 1));
            }
            opCost = mulcost_add_5EA344;
            quotient = multiplier - 1;
            do {
                quotient = (SInt32)((quotient + 1) - (quotient < 0x80000000)) >> 1;
            } while ((quotient & 1) == 0);
            TRY_MUL_FACTOR(quotient, opCost, 2);
            opCost = mulcost_sub_5EA348;
            multiplier++;
            do {
                multiplier = (SInt32)((multiplier + 1) - (multiplier < 0x80000000)) >> 1;
            } while ((multiplier & 1) == 0);
            TRY_MUL_FACTOR(multiplier, opCost, 3);
        }
    }
    return cache;
}

#undef TRY_MUL_FACTOR

#define FIND_MUL_ENTRY(value, dest) \
    do { \
        UInt32 key = (value); \
        SInt32 idx = (SInt32)((key ^ ((SInt32)key >> 31)) - ((SInt32)key >> 31)) % 511; \
        for ((dest) = division_cache_heads_61E1F4[idx]; \
             (dest) != NULL && (UInt32)(dest)->multiplier != key; \
             (dest) = (dest)->next) {} \
        if ((dest) == NULL) { \
            UInt32* words; \
            int i; \
            (dest) = lalloc(0x40); \
            words = (UInt32*)(dest); \
            for (i = 16; i != 0; i--) { \
                *words++ = 0; \
            } \
            (dest)->multiplier = key; \
            (dest)->ref = NULL; \
            (dest)->next = division_cache_heads_61E1F4[idx]; \
            division_cache_heads_61E1F4[idx] = (dest); \
        } \
    } while (0)

UInt32 fn_0054F410(MulDivCacheEntry* cache) {
    MulDivCacheEntry* unit;
    MulDivCacheEntry* base;
    MulDivCacheEntry* child;
    Operand res1;
    Operand res2;
    Operand res3;
    Operand res4;
    Operand res5;
    Operand res6;
    Operand res7;
    Operand shifted;
    UInt32 value;
    UInt32 target;
    UInt16 opcode;
    SInt32 count;
    Boolean wide;

    FIND_MUL_ENTRY(1, unit);

    switch (cache->initialized) {
        case 1:
            value = fn_0054F410(cache->ref);
            FIND_MUL_ENTRY(value, child);
            wide =
                ((division_type_61E1F0->type == TYPEINT ||
                  division_type_61E1F0->type == TYPEENUM) &&
                 division_type_61E1F0->size == 8);
            res1 = emit_unary(wide ? 0x83 : 0xB9, &child->result, 0);
            cache->result = res1;
            break;
        case 2:
            value = fn_0054F410(cache->ref);
            FIND_MUL_ENTRY(value, child);
            count = 0;
            do {
                value *= 2;
                count++;
            } while ((UInt32)cache->multiplier - 1 != value);
            shifted = emit_shift_left(&child->result, count, division_type_61E1F0);
            wide =
                ((division_type_61E1F0->type == TYPEINT ||
                  division_type_61E1F0->type == TYPEENUM) &&
                 division_type_61E1F0->size == 8);
            res2 = emit_binary(wide ? 0x79 : 4, &shifted, &unit->result, 0);
            cache->result = res2;
            break;
        case 3:
            value = fn_0054F410(cache->ref);
            FIND_MUL_ENTRY(value, child);
            count = 0;
            do {
                value *= 2;
                count++;
            } while ((UInt32)cache->multiplier + 1 != value);
            shifted = emit_shift_left(&child->result, count, division_type_61E1F0);
            wide =
                ((division_type_61E1F0->type == TYPEINT ||
                  division_type_61E1F0->type == TYPEENUM) &&
                 division_type_61E1F0->size == 8);
            res3 = emit_binary(wide ? 0x8E : 0xDD, &shifted, &unit->result, 0);
            cache->result = res3;
            break;
        case 4:
            value = fn_0054F410(cache->ref);
            FIND_MUL_ENTRY(value, child);
            count = 0;
            do {
                value *= 2;
                count++;
            } while (1 - (UInt32)cache->multiplier != value);
            shifted = emit_shift_left(&child->result, count, division_type_61E1F0);
            wide =
                ((division_type_61E1F0->type == TYPEINT ||
                  division_type_61E1F0->type == TYPEENUM) &&
                 division_type_61E1F0->size == 8);
            res4 = emit_binary(wide ? 0x8E : 0xDD, &unit->result, &shifted, 0);
            cache->result = res4;
            break;
        case 5:
            value = fn_0054F410(cache->ref);
            FIND_MUL_ENTRY(value, child);
            FIND_MUL_ENTRY(value, base);
            target = (UInt32)cache->multiplier - value;
            count = 0;
            do {
                value *= 2;
                count++;
            } while (target != value);
            shifted = emit_shift_left(&base->result, count, division_type_61E1F0);
            wide =
                ((division_type_61E1F0->type == TYPEINT ||
                  division_type_61E1F0->type == TYPEENUM) &&
                 division_type_61E1F0->size == 8);
            res5 = emit_binary(wide ? 0x79 : 4, &child->result, &shifted, 0);
            cache->result = res5;
            break;
        case 6:
            value = fn_0054F410(cache->ref);
            FIND_MUL_ENTRY(value, child);
            FIND_MUL_ENTRY(value, base);
            target = (UInt32)cache->multiplier + value;
            count = 0;
            do {
                value *= 2;
                count++;
            } while (target != value);
            shifted = emit_shift_left(&base->result, count, division_type_61E1F0);
            wide =
                ((division_type_61E1F0->type == TYPEINT ||
                  division_type_61E1F0->type == TYPEENUM) &&
                 division_type_61E1F0->size == 8);
            res6 = emit_binary(wide ? 0x8E : 0xDD, &shifted, &child->result, 0);
            cache->result = res6;
            break;
        case 7:
            value = fn_0054F410(cache->ref);
            FIND_MUL_ENTRY(value, child);
            FIND_MUL_ENTRY(value, base);
            target = value - (UInt32)cache->multiplier;
            count = 0;
            do {
                value *= 2;
                count++;
            } while (target != value);
            shifted = emit_shift_left(&base->result, count, division_type_61E1F0);
            wide =
                ((division_type_61E1F0->type == TYPEINT ||
                  division_type_61E1F0->type == TYPEENUM) &&
                 division_type_61E1F0->size == 8);
            res7 = emit_binary(wide ? 0x8E : 0xDD, &child->result, &shifted, 0);
            cache->result = res7;
            break;
    }
    return cache->multiplier;
}

#undef FIND_MUL_ENTRY
