#ifndef COMPILER_CERROR_H
#define COMPILER_CERROR_H

#include "compiler/common.h"
#include "compiler/tokens.h"
#include <stdarg.h>
#include <setjmp.h>
typedef struct CErrorBuffer {
    char* start;
    char* end;
    UInt32 size;
    UInt32 remaining;
} CErrorBuffer;

extern void CError_AppendObjectName(CErrorBuffer* eb, Object* obj);
extern void CError_AppendFunctionName(
    CErrorBuffer* eb, struct NameSpace* nspace, HashNameNode* name, int addAll, TypeFunc* tfunc
);
extern void CError_BufferAppendNameSpace(CErrorBuffer* eb, struct NameSpace* nspace);

enum {
    ERR_ILLEGAL_USE_PRECOMPILED_HEADER = 81,
    ERR_LPAREN_EXPECTED = 15,
    ERR_RPAREN_EXPECTED = 16,
    ERR_DECLARATION_SYNTAX_ERROR = 22,
    ERR_ILLEGAL_CONSTANT_EXPRESSION = 25,
    ERR_RBRACKET_EXPECTED = 26,
    ERR_ILLEGAL_USE_VOID = 27,
    ERR_ILLEGAL_USE_INCOMPLETE_STRUCT_UNION_CLASS = 37,
    ERR_DIVISION_BY_0 = 40,
    ERR_DATA_TYPE_INCOMPLETE = 46,
    ERR_ILLEGAL_TYPE = 47,
    ERR_ILLEGAL_USE_HANDLEOBJECT = 92,
    ERR_ILLEGAL_OPERATOR_DECLARATION = 94,
    ERR_ILLEGAL_AMPERSAND_REFERENCE = 97,
    ERR_ILLEGAL_EXCEPTION_SPECIFICATION = 165,
    ERR_EXCEPTION_SPECIFICATION_LIST_MISMATCH = 166,
    ERR_FUNCTIONS_CANNOT_RETURN_SOM_CLASSES = 184,
    ERR_SOM_CLASS_ARRAYS_NOT_SUPPORTED = 190,
    ERR_POINTER_TO_MEMBER_NOT_SUPPORTED_SOM = 191,
    ERR_UNDEFINED_LABEL = 60,
    ERR_ILLEGAL_USE = 214,
    ERR_CANNOT_PASS_VOID_FUNCTION_PARAMETER = 254,
    ERR_ASSIGNING_NON_INT_NUMERIC_VALUE_UNPROTOTYPED = 217,
    ERR_AMBIGUOUS_ACCESS_CLASS_STRUCT_UNION_MEMBER = 322,
    ERR_ILLEGAL_USE_NON_STATIC_MEMBER = 122,
    ERR_ILLEGAL_USE_SELF = 90,
    ERR_ILLEGAL_TYPE_CAST = 65,
    ERR_CLASS_CAST_UNKNOWN_BASE = 110,
    ERR_ILLEGAL_USE_THIS = 202,
    ERR_TYPE_SIZE_TOO_LARGE = 334,
};

extern void CError_Internal(char* file, int line);
extern void CError_NoMem(void);
extern char* CError_GetTypeName(Type* type, Boolean qualified, Boolean usetypedef);
extern char* CError_GetQualifiedName(struct NameSpace* nspace, HashNameNode* name);
extern char* CError_GetObjectName(Object* obj);

extern void CError_ErrorMessageVA(
    int code, const char* format, va_list list, Boolean isTerminal, Boolean isWarning
);

extern void CError_Warning(int code, ...);
extern void CError_Error(int code, ...);
extern void CError_ErrorSkipExpr(int code);
extern void CError_ErrorSemantic(int code);
extern void CError_GetNameString(int code);
extern void CError_AbstractClassError(TypeClass* tclass);
extern void CError_SetErrorToken(TStreamElement* token);
extern void CError_ResetErrorSkip(void);
extern char* CError_GetFunctionName(NameSpace* nspace, HashNameNode* name, TypeFunc* tfunc);

extern void fn_0044F9E0(void);
extern void fn_00455C40(void);

#endif
