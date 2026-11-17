#ifndef COMPILER_CRTTI_H
#define COMPILER_CRTTI_H

#include "compiler/common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern Object* Rgtid_func;
extern OLinkList* CRTTI_ConstructVTableHeaders(TypeClass* tclass, void* data, OLinkList* links);
extern ENode* CRTTI_ParseTypeID(void);
extern ENode* CRTTI_Parse_dynamic_cast(void);
extern ENode* CRTTI_Parse_static_cast(void);
extern ENode* CRTTI_Parse_reinterpret_cast(void);
extern ENode* CRTTI_Parse_const_cast(void);

extern void fn_00555180(Object* object, int mode);

#ifdef __cplusplus
}
#endif
extern TemplClass* CRTTI_Class2TemplClass(TemplClass* tmclass); //! @reconstructed

#endif
