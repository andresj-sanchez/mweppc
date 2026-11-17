#ifndef COMPILER_IROTYPE_H
#define COMPILER_IROTYPE_H

#include "compiler/common.h"
#include "compiler/CInt64.h"

extern Type* IRO_GetPointerType(Type* type); //! @reconstructed
extern Type* IRO_GetPtrDiffTType(void); //! @reconstructed

extern Type* IRO_LogicalType(void); //! @reconstructed
extern CInt64 IRO_MaxValueOfIntegerType(Type* type);
extern CInt64 IRO_MinValueOfIntegerType(Type* type);
extern Type* IRO_UnsignedType(Type* type); //! @reconstructed
extern Type* IRO_CopyType(Type* type); //! @reconstructed
extern void IRO_TruncateValueToType(CInt64* val, Type* type);
extern Boolean IRO_TypesEqual(Type* a, Type* b);
extern Boolean IRO_is_CPtypeequal(Type* a, Type* b);
extern Type* IRO_SignedType(Type* type);
extern void IRO_TruncateBitfieldValueToType(CInt64* val, Type* type, TypeBitfield* bitfieldType);
extern void IRO_SpellType(Type* type, char* buf);
extern void IRO_MarkUnpromotableTypesLinear(void);
extern void IRO_UnpromoteTypesLinear(IROLinear* list, Boolean mark, IROLinear* end);
extern void IRO_DumpType(Type* type);
extern Type* IRO_PromotedIntegralType(Type* type);
extern IROLinear* IRO_AddTypeConversion(IROLinear* linear, Type* type, IROList* list);
extern Boolean IRO_IsValuePreservingTypcon(IROLinear* linear);

#endif
