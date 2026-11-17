#ifndef COMPILER_OBJECTS_H
#define COMPILER_OBJECTS_H

#include "compiler/common.h"
#include "compiler/tokens.h"
#include "compiler/CInt64.h"

enum {
    DDATA = 0,
    DLOCAL = 1,
    DABSOLUTE = 2,
    DFUNC = 3,
    DVFUNC = 4,
    DINLINEFUNC = 5,
    DALIAS = 6,
    DLABEL = 8
};

#define OBJECT(obj) ((Object*)(obj))

enum {
    OT_ENUMCONST = 0,
    OT_TYPE = 1,
    OT_TYPETAG = 2,
    OT_NAMESPACE = 3,
    OT_MEMBERVAR = 4,
    OT_OBJECT = 5,
    OT_ILLEGAL = 6
};

enum {
    OBJECT_USED = 1,
    OBJECT_REFERENCED = 2,
    OBJECT_DEFINED = 4
};

struct Object {
    UInt8 otype;
    UInt8 access;
    UInt8 datatype;
    UInt8 section;
    NameSpace* nspace;
    HashNameNode* name;
    Type* type;
    UInt32 qual;
    SInt16 sclass;
    UInt8 flags;
    UInt8 unk17;
    SInt32 unk18;
    CodeGenFunction* unk1C;
    union {
        DwarfNode* entry;
    } debugInfo;
    struct VarRecord* varptr;
    union {
        struct {
            union {
                void* templ;
                struct CI_FuncData* ifuncdata;
                SInt32 intrinsicid;
                ENode* expr;
                void* varptr;
                UInt32* initdata;
            };
            SInt32 unk2C;
            union {
                SInt32 unk30;
                struct {
                    HashNameNode* linkname;
                    TemplFuncInstance* inst;
                    struct PTFList* ptfList;
                    ObjectList* argList;
                } func;
                struct {
                    SInt32 unk30;
                    SInt32 unk34;
                    HashNameNode* linkname;
                    struct ObjRegInfo* reginfo;
                } data;
            };
        };
        struct {
            Object* object;
            HashNameNode* member;
            SInt32 offset;
        } alias;
        struct {
            VarInfo* info;
            Object* realObj; /* 0x2C */
        } var;
        SInt32 tail_to44[7];
    } u;
};

struct VarInfo {
    struct Object* func; /* 0x00 */
    SInt32 usage; /* 0x04 */
    TStreamElement deftoken; /* 0x08 */
    SInt16 varnumber; /* 0x28 */
    Boolean noregister; /* 0x2A */
    Boolean used; /* 0x2B */
    UInt8 unknown2C;
    UInt8 unknown2D; /* 0x2D */
    UInt8 unknown2E;
    UInt8 unknown2F;
    UInt8 flags; /* 0x30 */
    UInt8 rclass; /* 0x31 */
    SInt16 reg; /* 0x32 */
    SInt16 regHi; /* 0x34 */
    UInt16 unknown36;
};

extern VarInfo* CodeGen_GetNewVarInfo(void);

struct ObjectTemplated {
    Object object; /* 0x00 */
    Object* parent; /* 0x44 */
    void* unk48; /* 0x48 */
};
#define OBJECT_TEMPL(obj) ((ObjectTemplated*)(obj))

struct ObjEnumConst {
    UInt8 otype;
    AccessType access;
    ObjEnumConst* next;
    HashNameNode* name;
    Type* type;
    CInt64 val;
};

struct ObjType {
    UInt8 otype;
    AccessType access;
    Type* type;
    UInt32 qual;
};

struct ObjMemberVar {
    UInt8 otype; /* 0x00 */
    AccessType access; /* 0x01 */
    Boolean anonunion;
    Boolean has_path;
    struct ObjMemberVar* next; /* 0x04 */
    HashNameNode* name; /* 0x08 */
    Type* type;
    UInt32 qual;
    UInt32 offset;
};

struct ObjBase {
    UInt8 otype;
};

#define OBJ_BASE(obj) ((ObjBase*)(obj))

struct ObjTypeTag {
    UInt8 otype; /* 0x00 */
    AccessType access; /* 0x01 */
    Type* type; /* 0x04 */
};

struct ObjectList {
    ObjectList* next;
    Object* object;
};

#endif
