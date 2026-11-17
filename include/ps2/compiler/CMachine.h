#ifndef COMPILER_CMACHINE_H
#define COMPILER_CMACHINE_H

#include "compiler/common.h"
#include "compiler/enode.h"
#include "compiler/types.h"

extern void CMach_GetBackEndOptions(void* options);
extern void CMach_SetBackEndOptions(void* options);
extern CInt64 CMach_CustomIntConvert(Type* type, Type* oldtype, CInt64 value);
extern Type* CMach_GetCustomType(SInt16 token);
extern void CMach_Configure(void);
extern void CMach_ParseGlobalRegisterVariable(DeclInfo* declaration);
extern Boolean CMach_PassResultInHiddenArg(Type* type);
extern Type* fn_00470080(TypeClass* tclass);
extern Boolean CMach_GetFunctionResultClass(TypeFunc* tfunc);
extern SInt32 cmach_structoffset;
extern SInt32 cmach_structstart;
extern UInt16 cmach_curbfsize;
extern UInt32 cmach_type_ptr_5E5E58;
extern UInt32 cmach_type_ptr_5E5E64;
extern UInt32 cmach_type_ptr_5E5E40;
extern UInt32 cmach_longlong_size_5E5E7C;
extern UInt32 cmach_ulonglong_size_5E5E88;
extern SInt32 cmach_zero_6363F4;
extern UInt8 cmach_context_flag_63736A;
extern SInt16 CMach_GetObjectAlign(Object* object); //! @reconstructed
extern SInt16 CMach_MemberAlignValue(Type* type, SInt32 offset); //! @reconstructed
extern SInt32 fn_004707B0(TypeStruct* type);
extern SInt32 fn_00470830(TypeClass* type);
extern SInt32 CMach_StructLayoutGetCurSize(void);
extern void CMach_StructLayoutInitOffset(SInt32 offset);
extern SInt16 CMach_GetTypeAlign(Type* type);
extern Boolean CMach_FloatIsNegOne(double value);
extern Boolean CMach_FloatIsOne(double value);
extern Boolean CMach_FloatIsZero(double value);
extern Float CMach_CalcFloatConvertFromInt(Type* type, CInt64 val);
extern CInt64 CMach_CalcIntDiadic(Type* type, CInt64 left, short op, CInt64 right);

extern TypeIntegral stbool;
extern TypeIntegral stchar;
extern TypeIntegral stsignedchar;
extern TypeIntegral stunsignedchar;
extern TypeIntegral stwchar;
extern TypeIntegral stsignedshort;
extern TypeIntegral stunsignedshort;
extern TypeIntegral stsignedint;
extern TypeIntegral stunsignedint;
extern TypeIntegral stsignedlong;
extern TypeIntegral stunsignedlong;
extern TypeIntegral stsignedlonglong;
extern TypeIntegral stunsignedlonglong;
extern TypeIntegral stfloat;
extern TypeIntegral stshortdouble;
extern TypeIntegral stdouble;
extern TypeIntegral stlongdouble;
extern TypePointer char_ptr;
extern TypeIntegral stcustomtype113;
extern TypeIntegral stint128;

extern SInt32 cmach_unk6355E4;

extern CInt64 CMach_CalcIntConvertFromFloat(Type* type, Float value);
extern Float CMach_CalcFloatConvert(Type* type, Float value);
extern Float CMach_CalcFloatMonadic(Type* type, short op, Float value);

extern Boolean CMach_IsUnsignedType(Type* type);
extern Boolean fn_00470390(Type* type);
extern CInt64 CMach_CalcIntMonadic(Type* rtype, SInt16 op, CInt64 val);
extern Float CMach_CalcFloatDiadic(Type* type, Float left, SInt16 op, Float right);
extern Boolean CMach_CalcFloatDiadicBool(Type* type, Float left, SInt16 op, Float right);

#endif
