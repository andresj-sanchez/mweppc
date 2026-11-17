#ifndef COMPILER_DIVBYCNST_H
#define COMPILER_DIVBYCNST_H

#include "compiler/InstrSelection.h"

extern DivisionParameters compute_unsigned_division_parameters(UInt32 divisor);
//! @reconstructed
extern ConstInfo2 compute_signed_division_multiplier_shift(SInt32 divisor);
//! @reconstructed

extern Operand multiply_by_constant(Operand* x, SInt32 multiplier, Type* type);
//! @reconstructed

struct MulDivCacheEntry {
    struct MulDivCacheEntry* ref;
    SInt32 multiplier;
    SInt32 cost;
    UInt8 initialized;
    UInt8 padding[3];
    struct MulDivCacheEntry* next;
    Operand result;
};

extern MulDivCacheEntry* division_cache_heads_61E1F4[511];

extern void fn_0054F190(Operand* input);
extern MulDivCacheEntry* fn_00550360(UInt32 multiplier, UInt32 budget);
extern UInt32 fn_0054F410(MulDivCacheEntry* cache);

extern void fn_00550790(GList* list);

extern SInt32 mulcost_add_5EA344;
extern SInt32 mulcost_addshift_5EA350;
extern SInt32 mulcost_negaddshift_5EA358;
extern SInt32 mulcost_negsub_5EA34C;
extern SInt32 mulcost_sub_5EA348;
extern SInt32 mulcost_subshift_5EA354;
extern Type* division_type_61E1F0;

#endif
