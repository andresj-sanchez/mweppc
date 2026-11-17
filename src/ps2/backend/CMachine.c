#include <string.h>
#include "compiler/CodeGen.h"
#include "compiler/PCodeInfo.h"
#include "compiler/mips.h"
#include "compiler/RegisterInfo.h"
#include "compiler/CMachine.h"
#include "compiler/CDecl.h"
#include "compiler/CMachineState.h"
#include "compiler/objects.h"
#include "compiler/CParser.h"
#include "compiler/ScanFloatX86.h"
#include "compiler/CClass.h"
#include "compiler/CPrep.h"
#include "compiler/CompilerTools.h"
#include "compiler/MIPSError.h"
#include "compiler/PCodeUtilities.h"
#include "compiler/Registers.h"
#include "compiler/InlineAsmMIPS.h"

Boolean CMach_FloatIsZero(double value) {
    return value == 0.0;
}

Boolean CMach_FloatIsOne(double value) {
    return value == 1.0;
}

Boolean CMach_FloatIsNegOne(double value) {
    return value == -1.0;
}

SInt32 CMach_StructLayoutGetCurSize(void) {
    return cmach_structoffset;
}

void CMach_StructLayoutInitOffset(SInt32 offset) {
    cmach_structoffset = offset;
    cmach_curbfsize = 0;
    cmach_structstart = offset;
}

#include "compiler/CError.h"
#include "compiler/CInt64.h"
#include "compiler/types.h"

Float CMach_CalcFloatConvert(Type* type, Float value) {
    switch (type->size) {
        case 4:
            value.value = (float)value.value;
            break;
        case 8:
            value.value = (double)value.value;
            break;
        default:
            CError_Internal("CMachine.c", 818);
    }
    return value;
}

Float CMach_CalcFloatConvertFromInt(Type* type, CInt64 value) {
    Float result;
    if (is_unsigned(type)) {
        result.value = CInt64_ConvertUToLongDouble(&value);
    } else {
        result.value = CInt64_ConvertToLongDouble(&value);
    }
    return result;
}

char* CMach_FloatScan(char* input, Float* result, Boolean* fail) {
    double value;
    char* output = (char*)ScanFloat(input, &value, fail);
    if (*fail) {
        result->value = 0.0;
    } else {
        result->value = value;
    }
    return output;
}

Boolean CMach_IsUnsignedType(Type* type) {
    switch ((SInt8)type->type) {
        case TYPEINT:
            switch (TYPE_INTEGRAL(type)->integral) {
                case IT_BOOL:
                case IT_UCHAR:
                case IT_WCHAR_T:
                case IT_USHORT:
                case 8:
                case 10:
                case 12:
                    return TRUE;
                case IT_CHAR:
                    return copts.unsigned_char;
                default:
                    return FALSE;
            }
        case TYPEPOINTER:
            return TRUE;
        default:
            return FALSE;
    }
}

void CMach_PragmaParams(void) {
    if (copts.warn_illpragma) {
        CPrep_Warning(87);
    }
    while (notendofline()) {
        lex();
    }
}

void CMach_InitFloatMem(Type* type, Float value, void* mem) {
    float f;
    double d;

    if (type->type == TYPEFLOAT) {
        switch (type->size) {
            case 4:
                f = value.value;
                memcpy(mem, &f, 4);
                break;
            case 8:
                d = value.value;
                memcpy(mem, &d, 8);
                break;
            default:
                CError_Internal("CMachine.c", 883);
                break;
        }
    } else {
        CError_Internal("CMachine.c", 888);
        return;
    }
}

Float CMach_CalcFloatMonadic(Type* type, short op, Float value) {
    if (op != '-') {
        CError_Internal("CMachine.c", 741);
    }
    value.value = -value.value;
    return CMach_CalcFloatConvert(type, value);
}

Type* CMach_GetCustomType(SInt16 token) {
    if (token == 0x114) {
        return (Type*)&stint128;
    }
    CError_Error(47);
    return (Type*)&stsignedint;
}

CInt64 CMach_CustomIntConvert(Type* type, Type* oldtype, CInt64 value) {
    return value;
}

void CMach_Configure(void) {
    cmach_type_ptr_5E5E40 = copts.ushort_wchar_t ? cmach_type_ptr_5E5E58 : cmach_type_ptr_5E5E64;
    if (copts.unknown15) {
        cmach_longlong_size_5E5E7C = 8;
        cmach_ulonglong_size_5E5E88 = 8;
    }
    cmach_zero_6363F4 = 0;
}

#include "compiler/CodeGen.h"

void CMach_GetBackEndOptions(void* options) {
    BackEndOptionsSnapshot* destination = (BackEndOptionsSnapshot*)options;
    SInt32 i;

    for (i = 0; i < 0x140 / sizeof(UInt32); i += 4) {
        destination->optionWords[i] = ((const UInt32*)&copts)[i];
        destination->optionWords[i + 1] = ((const UInt32*)&copts)[i + 1];
        destination->optionWords[i + 2] = ((const UInt32*)&copts)[i + 2];
        destination->optionWords[i + 3] = ((const UInt32*)&copts)[i + 3];
    }
    destination->optionWords[0x50] = copts.headtype;
    destination->optionWords[0x51] = copts.texttype;
    destination->optionWords[0x52] = (UInt32)copts.cursegmentname;
    destination->contextFlag = cmach_context_flag_63736A;
}

Boolean CMach_PassResultInHiddenArg(Type* type) {
    if (copts.unknown15 &&
        type &&
        (type->type == TYPEARRAY ||
         type->type == TYPESTRUCT ||
         type->type == TYPECLASS ||
         (type->type == TYPEMEMBERPOINTER && type->size == 12))) {
        if (type->size > register_size) {
            return TRUE;
        }
    }
    if (type->type == TYPECLASS && CClass_ReferenceArgument(TYPE_CLASS(type))) {
        return TRUE;
    }
    return FALSE;
}

SInt16 CMach_MemberAlignValue(Type* type, SInt32 offset) {
    SInt16 align = CMach_GetTypeAlign(type);
    if (align <= 1) {
        return 0;
    }
    return (align - 1) & (align - (offset & (align - 1)));
}

Type* fn_00470080(TypeClass* tclass) {
    ObjMemberVar* info = tclass->ivars;
    ClassList* item;
    ObjMemberVar* record;
    Type* type;
    if (!info) {
        return NULL;
    }
    if (info->next) {
        return NULL;
    }
    for (item = tclass->bases; item; item = item->next) {
        record = item->base->ivars;
        if (record && record->access <= 2) {
            return NULL;
        }
    }
    type = info->type;
    if (type->type == TYPEFLOAT) {
        return type;
    }
    return NULL;
}

Boolean CMach_GetFunctionResultClass(TypeFunc* tfunc) {
    if (fn_00436C70(tfunc->functype) == 1) {
        return TRUE;
    }
    return FALSE;
}

UInt32 fn_00471DA0(UInt32 bits) {
    UInt32 selected = 0;
    UInt32 masked = bits & 0x1E000000;
    if (masked) {
        if (masked == 0x02000000) {
            selected = 0x1;
        } else if (masked == 0x04000000) {
            selected = 0x2;
        } else if (masked == 0x06000000) {
            selected = 0x4;
        } else if (masked == 0x08000000) {
            selected = 0x8;
        } else if (masked == 0x0A000000) {
            selected = 0x10;
        } else if (masked == 0x0C000000) {
            selected = 0x20;
        } else if (masked == 0x10000000) {
            selected = 0x40;
        } else if (masked == 0x12000000) {
            selected = 0x80;
        } else if (masked == 0x14000000) {
            selected = 0x100;
        } else if (masked == 0x16000000) {
            selected = 0x200;
        } else if (masked == 0x18000000) {
            selected = 0x400;
        } else if (masked == 0x1A000000) {
            selected = 0x800;
        } else if (masked == 0x1C000000) {
            selected = 0x1000;
        } else if (masked == 0x1E000000) {
            selected = 0x2000;
        } else {
            CError_Internal("CMachine.c", 340);
        }
    }
    return selected;
}

Boolean fn_00470390(Type* type) {
    if (type->type == TYPECLASS && !CClass_IsPODClass(TYPE_CLASS(type))) {
        return TRUE;
    }
    if (type->type == TYPEMEMBERPOINTER && type->size == 4 && type->size != 4) {
        return TRUE;
    }
    if (type &&
        (type->type == TYPEARRAY ||
         type->type == TYPESTRUCT ||
         type->type == TYPECLASS ||
         (type->type == TYPEMEMBERPOINTER && type->size == 12))) {
        return TRUE;
    }
    return FALSE;
}

Boolean CMach_CalcFloatDiadicBool(Type* type, Float left, SInt16 op, Float right) {
    switch (op) {
        case 0x16A:
            return left.value == right.value;
        case 0x16B:
            return left.value != right.value;
        case 0x16C:
            return left.value <= right.value;
        case 0x16D:
            return left.value >= right.value;
        case '>':
            return left.value > right.value;
        case '<':
            return left.value < right.value;
        default:
            CError_Internal("CMachine.c", 763);
            return FALSE;
    }
}

Float CMach_CalcFloatDiadic(Type* type, Float left, SInt16 op, Float right) {
    switch (op) {
        case '+':
            left.value += right.value;
            break;
        case '-':
            left.value -= right.value;
            break;
        case '*':
            left.value *= right.value;
            break;
        case '/':
            left.value /= right.value;
            break;
        default:
            CError_Internal("CMachine.c", 728);
    }
    return CMach_CalcFloatConvert(type, left);
}

SInt16 CMach_GetObjectAlign(Object* object) {
    Type* type = object->type;
    SInt32 size;
    SInt16 alignment = fn_00471DA0(object->qual);
    SInt32 minimum;
    if (alignment) {
        return alignment;
    }
    alignment = CMach_GetTypeAlign(type);
    if ((type->type == TYPESTRUCT || type->type == TYPECLASS || type->type == TYPEARRAY) &&
        alignment < 4) {
        goto type_four;
    }
after_type:
    size = type->size;
    if (size >= 16 && copts.unknown41 && alignment <= 16) {
        goto big_sixteen;
    }
after_sixteen:
    if (size > 4) {
        minimum = copts.unknown0C >= 3 ? 8 : 4;
        if (alignment <= minimum) {
            goto minimum_limit;
        }
    }
after_minimum:
    if (size == 4 && alignment < 4) {
        goto four_bytes;
    }
    return alignment;
type_four:
    alignment = 4;
    goto after_type;
big_sixteen:
    alignment = 16;
    goto after_sixteen;
minimum_limit:
    if (copts.unknown0C < 3) {
        alignment = 4;
    } else {
        alignment = 8;
    }
    goto after_minimum;
four_bytes:
    alignment = 4;
    return alignment;
}

void CMach_SetBackEndOptions(void* options) {
    const BackEndOptionsSnapshot* state = (const BackEndOptionsSnapshot*)options;
    SInt32 i;
    StringPtr saved1 = copts.oldprefixname;
    HashNameNode* saved2 = copts.cursegmentname;

    for (i = 0; i < 0x140 / sizeof(UInt32); i += 4) {
        ((UInt32*)&copts)[i] = state->optionWords[i];
        ((UInt32*)&copts)[i + 1] = state->optionWords[i + 1];
        ((UInt32*)&copts)[i + 2] = state->optionWords[i + 2];
        ((UInt32*)&copts)[i + 3] = state->optionWords[i + 3];
    }
    copts.headtype = state->optionWords[0x50];
    copts.texttype = state->optionWords[0x51];
    copts.cursegmentname = (HashNameNode*)state->optionWords[0x52];
    cmach_context_flag_63736A = state->contextFlag;
    copts.oldprefixname = saved1;
    copts.cursegmentname = saved2;
}

SInt16 CMach_GetTypeAlign(Type* type) {
    SInt32 alignment;
    while (1) {
        switch ((SInt8)type->type) {
            case 0:
                return 4;
            case TYPEFUNC:
                return 0;
            case TYPEENUM:
                type = TYPE_ENUM(type)->enumtype;
            case TYPEINT:
                switch (TYPE_INTEGRAL(type)->integral) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                    case 10:
                    case 11:
                    case 12:
                    case 17:
                    case 18:
                        alignment = type->size;
                        break;
                    default:
                        CError_Internal("CMachine.c", 957);
                        break;
                }
                break;
            case TYPEFLOAT:
                if (type->size > 4) {
                    alignment = 8;
                } else {
                    alignment = 4;
                }
                break;
            case TYPEMEMBERPOINTER:
            case TYPEPOINTER:
                alignment = 4;
                break;
            case TYPEARRAY:
                type = TYPE_ARRAY(type)->target;
                continue;
            case TYPESTRUCT:
                alignment = TYPE_STRUCT(type)->align;
                break;
            case TYPECLASS:
                alignment = TYPE_CLASS(type)->align;
                break;
            case TYPEBITFIELD:
                type = TYPE_BITFIELD(type)->bitfieldtype;
                continue;
            case 9:
                return 1;
            default:
                CError_Internal("CMachine.c", 997);
                return 0;
        }
        break;
    }
    if (cmach_zero_6363F4 && alignment > cmach_zero_6363F4) {
        alignment = cmach_zero_6363F4;
    }
    return alignment;
}

SInt32 fn_004707B0(TypeStruct* object) {
    SInt32 size = object->size;
    SInt32 alignment = 1;
    StructMember* member;
    if (size <= 1 && copts.mwabi_bitfields) {
        goto one;
    }
    if (size < 1) {
        goto one;
    }
    goto scan_members;
one:
    return 1;
scan_members:
    for (member = object->members; member; member = member->next) {
        SInt32 memberalign;
        if ((UInt8)(member->type->type - TYPEMEMBERPOINTER) > 1) {
            goto use_bits_align;
        }
        goto use_type_align;
use_bits_align:
        memberalign = fn_00471DA0(member->qual);
        if (memberalign) {
            goto compare_member_align;
        }
use_type_align:
        memberalign = CMach_GetTypeAlign(member->type);
compare_member_align:;
        if (memberalign > alignment) {
            alignment = memberalign;
        }
    }
    return alignment;
}

SInt32 fn_00470830(TypeClass* type) {
    ObjMemberVar* member;
    ClassList* first;
    SInt32 alignment = 1;
    Boolean special;
    for (first = type->bases; first; first = first->next) {
        SInt32 current = first->base->align;
        if (current > alignment) {
            alignment = current;
        }
    }
    for (member = type->ivars; member; member = member->next) {
        SInt32 current;
        if ((UInt8)(member->type->type - TYPEMEMBERPOINTER) <= 1) {
            goto get_type_align;
        }
        current = fn_00471DA0(member->qual);
        if (current) {
            goto compare_member_align;
        }
get_type_align:
        current = CMach_GetTypeAlign(member->type);
compare_member_align:
        if (current > alignment) {
            alignment = current;
        }
    }
    if (type->vtable) {
        special = TRUE;
    } else {
        for (first = type->bases; first; first = first->next) {
            if (first->is_virtual) {
                special = TRUE;
                goto flag_found;
            }
        }
        special = FALSE;
    }
flag_found:;
    if (special) {
        if (alignment <= 4) {
            alignment = 4;
        }
    }
    return alignment;
}

SInt32 CMach_StructLayoutGetOffset(Type* type, UInt32 qualifiers) {
    SInt32 alignment;
    UInt32 bits;
    SInt32 padding;
    SInt32 result;
    SInt32 current;
    if ((UInt8)(type->type - TYPEMEMBERPOINTER) <= 1) {
        bits = 0;
    } else {
        bits = CParser_GetTypeQualifiers(type, qualifiers);
    }
    *(volatile UInt16*)&cmach_curbfsize = 0;
    alignment = fn_00471DA0(bits);
    current = cmach_structoffset;
    if (alignment <= 0 && (alignment = CMach_GetTypeAlign(type)) <= 1) {
        padding = 0;
    } else {
        SInt32 mask = alignment - 1;
        SInt32 remainder = current & mask;
        padding = (alignment - remainder) & mask;
    }
    result = (SInt16)padding + cmach_structoffset;
    cmach_structoffset = result + type->size;
    return result;
}

void CMach_InitIntMem(Type* type, CInt64 value, void* memory) {
    UInt32 lg;
    switch ((SInt8)type->type) {
        case TYPEINT:
            switch (type->size) {
                case 1:
                    lg = value.lo;
                    *(UInt8*)memory = *(UInt8*)&lg;
                    break;
                case 2:
                    lg = value.lo;
                    *(UInt16*)memory = (UInt16)lg;
                    break;
                case 4:
                    lg = value.lo;
                    *(UInt32*)memory = lg;
                    break;
                case 8:
                    fn_00446B40(value, memory);
                    break;
                case 16:
                    fn_00455C80(memory, &value, 16);
                    break;
                default:
                    CError_Internal("CMachine.c", 704);
                    break;
            }
            break;
        default:
            CError_Internal("CMachine.c", 709);
            break;
    }
}

CInt64 CMach_CalcIntConvertFromFloat(Type* type, Float value) {
    CInt64 result;
    UInt32 words[2];
    *(Float*)words = value;
    if (is_unsigned(type)) {
        CInt64_ConvertUFromLongDouble(&result, *(double*)words);
        switch (type->size) {
            case 1:
                CInt64_ConvertUInt8(&result);
                break;
            case 2:
                CInt64_ConvertUInt8(&result);
                break;
            case 4:
                CInt64_ConvertUInt32(&result);
                break;
            case 8:
                CInt64_ConvertUInt32(&result);
                break;
            case 16:
                break;
            default:
                CError_Internal("CMachine.c", 623);
                break;
        }
    } else {
        CInt64_ConvertFromLongDouble(&result, *(double*)words);
        switch (type->size) {
            case 1:
                CInt64_ConvertInt8(&result);
                break;
            case 2:
                CInt64_ConvertInt8(&result);
                break;
            case 4:
                CInt64_ConvertInt16(&result);
                break;
            case 8:
                CInt64_ConvertInt32(&result);
                break;
            case 16:
                break;
            default:
                CError_Internal("CMachine.c", 641);
                break;
        }
    }
    return result;
}

CInt64 CMach_CalcIntMonadic(Type* type, SInt16 op, CInt64 val) {
    if (is_unsigned(type)) {
        switch (type->size) {
            case 1:
                CInt64_ConvertUInt8(&val);
                break;
            case 2:
                CInt64_ConvertUInt8(&val);
                break;
            case 4:
                CInt64_ConvertUInt32(&val);
                break;
            case 8:
                CInt64_ConvertUInt32(&val);
                break;
            default:
                CError_Internal("CMachine.c", 529);
                break;
        }
        switch (op) {
            case '-':
                val = CInt64_Neg(val);
                break;
            case '~':
                val = CInt64_Inv(val);
                break;
            case '!':
                val = CInt64_Not(val);
                break;
            default:
                CError_Error(21);
                break;
        }
        switch (type->size) {
            case 1:
                CInt64_ConvertUInt8(&val);
                break;
            case 2:
                CInt64_ConvertUInt8(&val);
                break;
            case 4:
                CInt64_ConvertUInt32(&val);
                break;
            case 8:
                CInt64_ConvertUInt32(&val);
                break;
        }
    } else {
        switch (type->size) {
            case 1:
                CInt64_ConvertInt8(&val);
                break;
            case 2:
                CInt64_ConvertInt8(&val);
                break;
            case 4:
                CInt64_ConvertInt16(&val);
                break;
            case 8:
                CInt64_ConvertInt32(&val);
                break;
            default:
                CError_Internal("CMachine.c", 567);
                break;
        }
        switch (op) {
            case '-':
                val = CInt64_Neg(val);
                break;
            case '~':
                val = CInt64_Inv(val);
                break;
            case '!':
                val = CInt64_Not(val);
                break;
            default:
                CError_Error(21);
                break;
        }
        switch (type->size) {
            case 1:
                CInt64_ConvertInt8(&val);
                break;
            case 2:
                CInt64_ConvertInt8(&val);
                break;
            case 4:
                CInt64_ConvertInt16(&val);
                break;
            case 8:
                CInt64_ConvertInt32(&val);
                break;
        }
    }
    return val;
}

CInt64 CMach_CalcIntDiadic(Type* type, CInt64 left, short op, CInt64 right) {
    if (is_unsigned(type)) {
        switch (type->size) {
            case 1:
                CInt64_ConvertUInt8(&left);
                CInt64_ConvertUInt8(&right);
                break;
            case 2:
                CInt64_ConvertUInt8(&left);
                CInt64_ConvertUInt8(&right);
                break;
            case 4:
                CInt64_ConvertUInt32(&left);
                CInt64_ConvertUInt32(&right);
                break;
            case 8:
                CInt64_ConvertUInt32(&left);
                CInt64_ConvertUInt32(&right);
                break;
            case 16:
                CError_Error(57);
                break;
            default:
                CError_Internal("CMachine.c", 388);
        }

        switch (op) {
            case '*':
                left = CInt64_MulU(left, right);
                break;
            case '/':
                if (CInt64_IsZero(&right)) {
                    CError_Warning(ERR_DIVISION_BY_0);
                } else {
                    left = CInt64_DivU(left, right);
                }
                break;
            case '%':
                if (CInt64_IsZero(&right)) {
                    CError_Warning(ERR_DIVISION_BY_0);
                } else {
                    left = CInt64_ModU(left, right);
                }
                break;
            case '+':
                left = CInt64_Add(left, right);
                break;
            case '-':
                left = CInt64_Sub(left, right);
                break;
            case 366:
                left = CInt64_Shl(left, right);
                break;
            case 367:
                left = CInt64_ShrU(left, right);
                break;
            case '<':
                CInt64_SetLong(&left, CInt64_LessU(left, right));
                break;
            case '>':
                CInt64_SetLong(&left, CInt64_GreaterU(left, right));
                break;
            case 364:
                CInt64_SetLong(&left, CInt64_LessEqualU(left, right));
                break;
            case 365:
                CInt64_SetLong(&left, CInt64_GreaterEqualU(left, right));
                break;
            case 362:
                CInt64_SetLong(&left, CInt64_Equal(left, right));
                break;
            case 363:
                CInt64_SetLong(&left, CInt64_NotEqual(left, right));
                break;
            case '&':
                left = CInt64_And(left, right);
                break;
            case '^':
                left = CInt64_Xor(left, right);
                break;
            case '|':
                left = CInt64_Or(left, right);
                break;
            case 360:
                CInt64_SetLong(&left, (!CInt64_IsZero(&left) && !CInt64_IsZero(&right)));
                break;
            case 361:
                CInt64_SetLong(&left, (!CInt64_IsZero(&left) || !CInt64_IsZero(&right)));
                break;
            default:
                CError_Error(21);
        }

        switch (type->size) {
            case 1:
                CInt64_ConvertUInt8(&left);
                break;
            case 2:
                CInt64_ConvertUInt8(&left);
                break;
            case 4:
                CInt64_ConvertUInt32(&left);
                break;
            case 8:
                CInt64_ConvertUInt32(&left);
                break;
        }
    } else {
        switch (type->size) {
            case 1:
                CInt64_ConvertInt8(&left);
                CInt64_ConvertInt8(&right);
                break;
            case 2:
                CInt64_ConvertInt8(&left);
                CInt64_ConvertInt8(&right);
                break;
            case 4:
                CInt64_ConvertInt16(&left);
                CInt64_ConvertInt16(&right);
                break;
            case 8:
                CInt64_ConvertInt32(&left);
                CInt64_ConvertInt32(&right);
                break;
            case 16:
                CError_Error(57);
                break;
            default:
                CError_Internal("CMachine.c", 462);
        }

        switch (op) {
            case '*':
                left = CInt64_Mul(left, right);
                break;
            case '/':
                if (CInt64_IsZero(&right)) {
                    CError_Warning(ERR_DIVISION_BY_0);
                } else {
                    left = CInt64_Div(left, right);
                }
                break;
            case '%':
                if (CInt64_IsZero(&right)) {
                    CError_Warning(ERR_DIVISION_BY_0);
                } else {
                    left = CInt64_Mod(left, right);
                }
                break;
            case '+':
                left = CInt64_Add(left, right);
                break;
            case '-':
                left = CInt64_Sub(left, right);
                break;
            case 366:
                left = CInt64_Shl(left, right);
                break;
            case 367:
                left = CInt64_Shr(left, right);
                break;
            case '<':
                CInt64_SetLong(&left, CInt64_Less(left, right));
                break;
            case '>':
                CInt64_SetLong(&left, CInt64_Greater(left, right));
                break;
            case 364:
                CInt64_SetLong(&left, CInt64_LessEqual(left, right));
                break;
            case 365:
                CInt64_SetLong(&left, CInt64_GreaterEqual(left, right));
                break;
            case 362:
                CInt64_SetLong(&left, CInt64_Equal(left, right));
                break;
            case 363:
                CInt64_SetLong(&left, CInt64_NotEqual(left, right));
                break;
            case '&':
                left = CInt64_And(left, right);
                break;
            case '^':
                left = CInt64_Xor(left, right);
                break;
            case '|':
                left = CInt64_Or(left, right);
                break;
            case 360:
                CInt64_SetLong(&left, (!CInt64_IsZero(&left) && !CInt64_IsZero(&right)));
                break;
            case 361:
                CInt64_SetLong(&left, (!CInt64_IsZero(&left) || !CInt64_IsZero(&right)));
                break;
            default:
                CError_Error(21);
        }

        switch (type->size) {
            case 1:
                CInt64_ConvertInt8(&left);
                break;
            case 2:
                CInt64_ConvertInt8(&left);
                break;
            case 4:
                CInt64_ConvertInt16(&left);
                break;
            case 8:
                CInt64_ConvertInt32(&left);
                break;
        }
    }

    return left;
}
