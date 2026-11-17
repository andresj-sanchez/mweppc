#ifndef COMPILER_TEMPLATES_H
#define COMPILER_TEMPLATES_H

#include "compiler/common.h"

struct TemplArg {
    TemplArg* next;
    struct TemplParamID pid;
    union {
        struct {
            Type* type;
            UInt32 qual;
        } typeparam;
        struct {
            ENode* expr;
            Boolean is_ref;
        } paramdecl;
        Type* ttargtype;
    } data;
    Boolean is_deduced;
};

struct TemplFuncInstance {
    TemplFuncInstance* next;
    Object* object;
    TemplArg* args;
    Boolean is_instantiated;
    Boolean is_specialized;
    Boolean is_extern;
};

typedef struct TemplClassTail TemplClassTail;
struct TemplClassTail {
    struct TemplClass* templ_parent;
    struct TemplClassInst* inst_parent;
    struct TemplParam* templ__params;
    void* members;
    struct TemplClassInst* instances;
    struct TemplClass* pspec_owner;
    void* pspecs;
    void* actions;
    UInt16 lex_order_count;
    SInt8 align;
    UInt8 flags;
};

struct TemplClass {
    TypeClass theclass; /* 0x00 */
    struct TemplClass* next; /* 0x2C */
    TemplClassTail tail; /* 0x30 */
};

struct TemplClassInst {
    TypeClass theclass; /* 0x00 */
    struct TemplClassInst* next;
    struct TemplClassTail* templ; /* 0x30 */
    TemplArg* unk34;
    TemplArg* inst_args; /* 0x38 */
    Boolean is_instantiated; /* 0x3C */
    Boolean is_specialized; /* 0x3D */
    Boolean is_extern;
    Boolean static_instantiated;
};

#define TEMPL_CLASS(ty) ((TemplClass*)(ty))
#define TEMPL_CLASS_INST(ty) ((TemplClassInst*)(ty))

struct TemplateFunction {
    struct TemplateFunction* next;
    struct TemplateFunction* original;
    HashNameNode* name;
    void* params;
    TokenStream stream;
    TStreamElement deftoken;
    Object* tfunc;
    struct TemplFuncInstance* instances;
    CPrepFileInfo* srcfile;
    SInt32 startoffset;
    SInt32 endoffset;
};

struct TemplParam {
    struct TemplParam* next;
    HashNameNode* name;
    struct TemplParamID pid;
    union {
        struct {
            Type* type;
            UInt32 qual;
        } typeparam;
        struct {
            Type* type;
            UInt32 qual;
            ENode* defaultarg;
        } paramdecl;
        struct {
            struct TemplParam* plist;
            Type* defaultarg;
        } templparam;
    } data;
};

struct TemplStack {
    struct TemplStack* next;
    union {
        Object* func;
        TypeClass* theclass;
    } u;
    Boolean is_func;
    Boolean is_poi;
};

struct DeduceInfo {
    TemplArg* args;
    TemplArg argBuffer[16];
    int maxCount;
    int count;
    UInt8 nindex;
};

extern TemplStack* ctempl_curinstance;
extern SInt16 ctempl_instdepth;

#endif
