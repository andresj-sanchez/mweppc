#ifndef COMPILER_REGLISTQUERIES_H
#define COMPILER_REGLISTQUERIES_H

#include "compiler/common.h"

extern ObjectList* ig_reglist_a;
extern ObjectList* ig_reglist_b;
extern Boolean fn_004D1D10(void* object);
extern Boolean fn_004D1D40(Object* object);
extern Boolean CABI_ExprHasSideEffects(Type* type); //! @reconstructed
extern Boolean CABI_HasSideEffects(Type* type); //! @reconstructed

#endif
