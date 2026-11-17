#ifndef COMPILER_TYPES_H
#define COMPILER_TYPES_H

#include "compiler/common.h"

enum {
    TYPEVOID = 0,
    TYPEINT = 1,
    TYPEFLOAT = 2,
    TYPEENUM = 3,
    TYPESTRUCT = 4,
    TYPECLASS = 5,
    TYPEFUNC = 6,
    TYPEBITFIELD = 7,
    TYPETEMPLDEP = 8,
    TYPETEMPLATE = 9,
    TYPEMEMBERPOINTER = 10,
    TYPEPOINTER = 11,
    TYPEARRAY = 12
};

enum {
    IT_BOOL = 0,
    IT_CHAR = 1,
    IT_SCHAR = 2,
    IT_UCHAR = 3,
    IT_WCHAR_T = 4,
    IT_SHORT = 5,
    IT_USHORT = 6,
    IT_INT = 7,
    IT_UINT = 8,
    IT_LONG = 9,
    IT_ULONG = 10,
    IT_LONGLONG = 11,
    IT_ULONGLONG = 12,
    IT_FLOAT = 0x0D,
    IT_SHORTDOUBLE = 0x0E,
    IT_DOUBLE = 0x0F,
    IT_LONGDOUBLE = 0x10,
    IT_17 = 0x11,
    IT_18 = 0x12,
    IT_19 = 0x13,
    IT_20 = 0x14,
    IT_21 = 0x15,
    IT_22 = 0x16,
    IT_23 = 0x17,
    IT_24 = 0x18
};

struct Type {
    UInt8 type;
    SInt32 size;
};

struct TypeIntegral {
    UInt8 type;
    SInt32 size;
    UInt8 integral;
};

#define TYPE_INTEGRAL(ty) ((TypeIntegral*)(ty))
#define TYPE_BITFIELD(ty) ((TypeBitfield*)(ty))
#define IS_TYPE_INT(ty) ((ty)->type == TYPEINT)
#define IS_TYPE_ENUM(ty) ((ty)->type == TYPEENUM)
#define IS_TYPE_INT_OR_ENUM(ty) (IS_TYPE_INT(ty) || IS_TYPE_ENUM(ty))
#define IS_TYPE_FLOAT(ty) ((ty)->type == TYPEFLOAT)
#define IS_TYPE_POINTER(ty) ((ty)->type == TYPEPOINTER || (ty)->type == TYPEARRAY)
#define IS_TYPE_POINTER_ONLY(ty) ((ty)->type == TYPEPOINTER)
#define IS_TYPE_REFERENCE(ty) ((ty)->type == TYPEPOINTER && (TYPE_POINTER(ty)->qual & Q_REFERENCE))
#define TPTR_TARGET(ty) (TYPE_POINTER(ty)->target)
#define TPTR_QUAL(ty) (TYPE_POINTER(ty)->qual)
#define IS_TYPEPOINTER_REFERENCE(ty) (TYPE_POINTER(ty)->qual & Q_REFERENCE)
#define IS_TYPE_VOID(ty) ((ty)->type == TYPEVOID)
#define IS_TYPE_STRUCT(ty) ((ty)->type == TYPESTRUCT)
#define IS_TYPE_CLASS(ty) ((ty)->type == TYPECLASS)
#define IS_TYPE_ARRAY(ty) ((ty)->type == TYPEARRAY)
#define IS_TYPE_BITFIELD(ty) ((ty)->type == TYPEBITFIELD)
#define IS_TYPE_4BYTES_MEMBERPOINTER(ty) ((ty)->type == TYPEMEMBERPOINTER && (ty)->size == 4)
#define IS_TYPE_12BYTES_MEMBERPOINTER(ty) ((ty)->type == TYPEMEMBERPOINTER && (ty)->size == 12)
#define IS_TYPE_FLOAT32(ty) \
    (TYPE_INTEGRAL(ty)->integral == IT_FLOAT || \
     (IS_TYPE_FLOAT(ty) && (ty)->size == 4)) //! @reconstructed
#define IS_TYPE_FLOAT64(ty) \
    (TYPE_INTEGRAL(ty)->integral == IT_DOUBLE || \
     (IS_TYPE_FLOAT(ty) && (ty)->size == 8)) //! @reconstructed
#define TYPE_IS_16BYTES(ty) \
    (((ty)->type == TYPEINT || (ty)->type == TYPEENUM) && (ty)->size == 16) //! @reconstructed
#define TYPE_IS_8BYTES(ty) (((ty)->type == TYPEINT || (ty)->type == TYPEENUM) && (ty)->size == 8)

struct TypePointer {
    UInt8 type;
    SInt32 size;
    Type* target;
    UInt32 qual;
};

#define TYPE_POINTER(ty) ((TypePointer*)(ty))

struct TypeArray {
    UInt8 type;
    SInt32 size;
    Type* target;
    UInt32 qual;
    SInt32 nelimit;
};

#define TYPE_ARRAY(ty) ((TypeArray*)(ty))

typedef enum TypeTemplDepType {
    TEMPLDEP_ARGUMENT,
    TEMPLDEP_QUALNAME,
    TEMPLDEP_TEMPLATE,
    TEMPLDEP_ARRAY,
    TEMPLDEP_QUALTEMPL,
    TEMPLDEP_BITFIELD
} TypeTemplDepType;

struct TypeTemplDep {
    UInt8 type;
    SInt32 size;
    UInt8 dtype;
    UInt8 unk9[3];
    union {
        struct TemplParamID* pid;
        struct {
            struct TypeTemplDep* type;
            HashNameNode* name;
        } qual;
        struct {
            struct TemplClass* templ;
            struct TemplArg* args;
        } templ;
        struct {
            Type* type;
            struct ENode* index;
        } array;
    } u;
};

#define TYPE_TEMPLATE(ty) ((TypeTemplDep*)(ty))

struct TypeMemberPointer {
    UInt8 type;
    SInt32 size;
    Type* ty1;
    Type* ty2;
    UInt32 qual;
};

#define TYPE_MEMBER_POINTER(ty) ((TypeMemberPointer*)(ty))

struct TypeEnum {
    UInt8 type;
    SInt32 size;
    NameSpace* nspace;
    ObjEnumConst* enumlist;
    Type* enumtype;
    HashNameNode* enumname;
};

#define TYPE_ENUM(ty) ((TypeEnum*)(ty))

struct StructMember {
    struct StructMember* next;
    Type* type;
    HashNameNode* name;
    SInt32 offset;
    UInt32 qual;
};

struct TypeStruct {
    UInt8 type;
    SInt32 size;
    HashNameNode* name;
    StructMember* members;
    UInt8 stype;
    SInt16 align;
};

#define TYPE_STRUCT(ty) ((TypeStruct*)(ty))

enum {
    STRUCT_TYPE_STRUCT = 0,
    STRUCT_TYPE_UNION = 1,
    STRUCT_TYPE_CLASS = 2
};

struct VTable {
    Object* object;
    TypeClass* owner;
    SInt32 offset;
    SInt32 size;
};

struct TypeClass {
    UInt8 type;
    SInt32 size;
    NameSpace* nspace;
    HashNameNode* classname;
    ClassList* bases;
    VClassList* vbases;
    ObjMemberVar* ivars;
    VTable* vtable;
    SOMInfo* sominfo;
    UInt16 flags;
    SInt8 mode; /* 0x26 */
    SInt8 action; /* 0x27 */
    SInt16 align; /* 0x28 */
    UInt8 eflags; /* 0x2A */
};

#define TYPE_CLASS(ty) ((TypeClass*)(ty))

enum {
    CLASS_MODE_STRUCT = 0,
    CLASS_MODE_UNION = 1,
    CLASS_MODE_CLASS = 2
};

enum {
    CLASS_ACTION_NONE = 0,
    CLASS_ACTION_KEY_FUNCTION = 1,
    CLASS_ACTION_UNUSED = 2,
    CLASS_ACTION_JAVA = 3
};

enum {
    CLASS_EFLAGS_INTERNAL = 1,
    CLASS_EFLAGS_IMPORT = 2,
    CLASS_EFLAGS_EXPORT = 4,
    CLASS_EFLAGS_STRUCT_ALIGN = 0xF0
};

enum {
    CLASS_HANDLEOBJECT = 1,
    CLASS_COMPLETED = 2,
    CLASS_ABSTRACT = 8,
    CLASS_IS_CONVERTIBLE = 0x40,
    CLASS_SINGLE_OBJECT = 0x10,
    CLASS_HAS_VBASES = 0x20,
    CLASS_IS_TEMPL_INST = 0x800,
    CLASS_VBASE_CTOR_OFFSET = 0x8000
};

struct ExceptSpecList {
    ExceptSpecList* next;
    Type* type;
    UInt32 qual;
};

struct TypeFunc {
    UInt8 type;
    SInt32 size;
    FuncArg* args;
    ExceptSpecList* exspecs;
    Type* functype;
    UInt32 qual;
    UInt32 flags;
};

// TypeFunc::flags
enum {
    FUNC_PASCAL = 1,
    FUNC_DEFINED = 2,
    FUNC_KEY_FUNCTION = 4,
    FUNC_PURE = 8,
    FUNC_METHOD = 0x10,
    FUNC_OVERRIDE = 0x20,
    FUNC_CONVERSION = 0x40,
    FUNC_PTM = 0x80,
    FUNC_AUTO_GENERATED = 0x100,
    FUNC_INTRINSIC = 0x200,
    FUNC_NOTHROW = 0x400,
    FUNC_AUTO_INLINE = 0x800,
    FUNC_IS_CTOR = 0x1000,
    FUNC_IS_DTOR = 0x2000,
    FUNC_OBJC_METHOD = 0x4000,
    FUNC_CONST = 0x8000,
    FUNC_VOLATILE = 0x10000,
    FUNC_IS_TEMPL = 0x100000,
    FUNC_TEMPLATE_ACTION = 0x200000,
    FUNC_COVARIANT = 0x400000,
    FUNC_IS_TEMPL_INSTANCE = 0x800000
};

struct BClassList {
    struct BClassList* next;
    Type* type;
};

struct VClassList {
    struct VClassList* next;
    struct TypeClass* base;
    SInt32 offset; // offset within the class instance
    SInt32 voffset; // offset within the vtable
    Boolean has_override;
};

struct ClassList {
    struct ClassList* next; /* 0x00 */
    struct TypeClass* base; /* 0x04 */
    SInt32 offset; /* 0x08 */
    SInt32 voffset; /* 0x0C */
    AccessType access; /* 0x10 */
    SInt8 is_virtual; /* 0x11 */
};

struct TypeMemberFunc {
    UInt8 type;
    SInt32 size;
    FuncArg* args;
    ExceptSpecList* exspecs;
    Type* functype;
    UInt32 qual;
    UInt32 flags;
    TypeClass* theclass;
    SInt32 vtbl_index;
    SInt32 funcid;
    Boolean is_static;
};

#define TYPE_METHOD(ty) ((TypeMemberFunc*)(ty))

struct FuncArg {
    FuncArg* next;
    HashNameNode* name;
    ENode* dexpr;
    Type* type;
    UInt32 qual;
    SInt16 sclass;
    Boolean is_array;
};

#define TYPE_FUNC(ty) ((TypeFunc*)(ty))
#define IS_TYPE_FUNC(ty) ((ty)->type == TYPEFUNC)
#define IS_TEMPL_FUNC(ty) (IS_TYPE_FUNC(ty) && (TYPE_FUNC(ty)->flags & FUNC_IS_TEMPL))

struct TypeBitfield {
    UInt8 type;
    SInt32 size;
    Type* bitfieldtype;
    SInt8 offset;
    SInt8 bitlength;
};

#define TYPE(ty) ((Type*)(ty))

extern TypePointer void_ptr;

#endif
