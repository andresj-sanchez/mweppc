#ifndef COMPILER_EXPRPREPARATION_H
#define COMPILER_EXPRPREPARATION_H

#include "compiler/common.h"
#include "compiler/types.h"
#include "compiler/CParser.h"

typedef struct ExprPrepObjectLink {
    struct ExprPrepObjectLink* next;
    Object* object;
    CLabel* label;
} ExprPrepObjectLink;

#ifdef __cplusplus
extern "C" {
#endif

static inline SInt32 exprprep_type_width(Type* type) {
    if (type->type == TYPEFLOAT) {
        return type->size * 8;
    }
    if (is_unsigned(type)) {
        return type->size * 2 + 1;
    }
    return type->size * 2;
}

extern void fn_004ADD20(Statement* statements);
extern void fn_004ADF20(ENode* node, Type* context, SInt32 flags);
extern void fn_004AEF80(ENode* expr);
extern void fn_004AEFC0(ENode* expr);
extern SInt32 fn_004AF070(const char* name);
extern void fn_004AF1B0(ENode* expr);
extern void fn_004AF280(ENode* expr, UInt8 opcode);
extern void fn_004AF530(ENode* expr, UInt8 opcode);
extern ENode* fn_004AF7A0(ENode* source);
extern ENode* fn_004AF7F0(ENode* source);
extern ENode* fn_004AF990(ENode* expr);
extern Object* fn_004AFA60(CLabel* label);
extern Object* fn_004AFAE0(Type* type, const UInt32* bits);
extern Boolean fn_004AFBD0(Type* type, double value);

extern Float exprprep_float_zero_609060;
extern SInt32 curstmtvalue;
extern UInt8 cg_alias_option_637389;
extern const char exprprep_abs_5E8F62[];
extern const char exprprep_clear_5E8F47[];
extern const char exprprep_labs_5E8F66[];
extern const char exprprep_memcpy_5E8F5B[];
extern const char exprprep_sqrt_5E8F4F[];
extern const char exprprep_sqrtf_5E8F41[];
extern const char exprprep_strcpy_5E8F54[];

extern ExprPrepObjectLink* exprprep_objects_6361C8;

#ifdef __cplusplus
}
#endif

#endif
