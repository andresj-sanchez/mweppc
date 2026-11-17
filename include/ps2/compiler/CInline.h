#ifndef COMPILER_CINLINE_H
#define COMPILER_CINLINE_H

#include "compiler/common.h"
#include "compiler/types.h"
#include "compiler/enode.h"
#include "compiler/Exceptions.h"
#include "compiler/Switch.h"
#include "compiler/tokens.h"
#include "compiler/templates.h"
#include "compiler/objects.h"

typedef enum CInlineCopyMode {
    CopyModeLocal,
    CopyModeGlobal,
    CopyModePack,
    CopyModeUnpack,
    CopyModeExpand
} CInlineCopyMode;

struct CI_Var {
    HashNameNode* name;
    Type* type;
    UInt32 qual;
    Object* obj;
    UInt8 sflags;
    SInt8 used;
    UInt8 unk12;
    SInt8 unmodified;
};

typedef struct AObject {
    Object* obj;
    Object* object;
    ENode* useExpr;
    ENode* evalExpr;
} AObject;

typedef struct IDTrans {
    struct IDTrans* next;
    SInt32 from;
    SInt32 to;
} IDTrans;

typedef struct LabelTrans {
    struct LabelTrans* next;
    CLabel** labelptr;
    SInt16 id;
} LabelTrans;

struct CI_SwitchCase {
    SInt16 labelID;
    UInt16 pad2;
    union {
        SwitchBounds bounds;
        struct {
            ENode* expr;
            ENode* expr2;
        } range;
    };
    UInt32 unk20;
};

struct CI_Switch {
    ENode* expr;
    Type* exprtype;
    SInt16 defaultlabelID;
    SInt16 numcases;
    SInt16 numranges;
    CI_SwitchCase cases[1];
};

struct CI_Statement {
    UInt8 type;
    UInt8 flags;
    UInt16 value;
    SInt32 sourceoffset;
    HashNameNode* sourcefilepath;
    struct ExceptionAction* dobjstack;
    union {
        SInt16 statementnum;
        ENode* expr;
        struct {
            ENode* expr;
            SInt16 statementnum;
        } ifgoto;
        CI_Switch* switchdata;
        struct {
            void* data;
            SInt32 size;
        } asmdata;
    } u;
};

enum {
    CI_SFLAGS_NoClass = 0,
    CI_SFLAGS_Register = 1,
    CI_SFLAGS_Auto = 2,
    CI_SFLAGS_Referenced = 0x80
};

typedef enum CI_CanInline {
    CI_CanInlineNone = 0,
    CI_CanInlineStatement = 3,
    CI_CanInlineExpression = 6
} CI_CanInline;

struct CI_FuncData {
    SInt16 numlocals; /* +0x00 */
    SInt16 numarguments; /* +0x02 */
    CI_Var* arguments; /* +0x04 */
    SInt16 numstatements; /* +0x08 */
    CI_Statement* statements; /* +0x0C */
    UInt8 unk10[0x1E4];
    FileOffsetInfo fileoffset;
    SInt32 symdecloffset;
    SInt32 functionbodyoffset;
    void* functionbodypath;
    SInt32 symdeclend;
    UInt8 can_inline;
};

enum {
    CI_ActionInlineFunc = 0,
    CI_ActionMemberFunc = 1,
    CI_ActionTemplateFunc = 2,
    CI_ActionDefaultFunc = 3
};
struct CI_Action {
    CI_Action* next; /* 0x00 */
    Object* obj; /* 0x04 */
    void* unk08; /* 0x08 */
    union {
        struct {
            FileOffsetInfo fileoffset; /* 0x0C */
            /* 0x1C */
            struct {
                SInt32 tokens;
                TStreamElement* firsttoken;
            } stream;
            TypeClass* tclass; /* 0x24 */
        } inlinefunc;
        struct {
            TemplClass* templ; /* 0x0C */
            TemplClassInst* inst; /* 0x10 */
            void* tmemb; /* 0x14 */
        } memberfunc;
        struct {
            void* func; /* 0x0C */
            TemplFuncInstance* inst; /* 0x10 */
        } templatefunc;
    } u;
    UInt8 actiontype; /* 0x28 */
};

typedef struct CI_Export {
    struct CI_Export* next;
    Object* object;
    CI_FuncData* funcdata;
    UInt8 oldStyleParameters;
} CI_Export;

typedef struct CI_StmtLink {
    struct CI_StmtLink* next;
    Statement* stmt;
    CI_Statement* ciStmt;
} CI_StmtLink;

struct TemplDepArg {
    void* data;
    UInt8 kind;
};

struct DepName {
    struct DepName* next;
    struct DepName* child;
    ENode* expr;
    SInt32 unkC;
    SInt32 unk10;
    SInt32 unk14;
    SInt32 unk18;
    SInt32 unk1C;
    SInt32 unk20;
    SInt32 unk24;
    SInt32 unk28;
    SInt32 unk2C;
};

extern void CInline_GenerateInlineFunc(CI_Action* action);
extern Boolean CInline_GenerateDeferredFuncs(void);
extern Boolean CInline_CanFreeLHeap(void);
extern Boolean CInline_ExpressionHasSideEffect(ENode* expr);
extern ENode* CInline_CopyExpression(ENode* expr, CInlineCopyMode mode);
extern void fn_0050DF30(ENode* expr);
extern Object* CInline_GetLocalObj(Object* obj, Boolean inlineCopy);
extern Object* CInline_GetLocalID(Object* obj);
extern void CInline_ObjectAddrRef(Object* object);

extern void* fn_00509850(void* node, void* target);

extern SInt32 cg_codegen_active_line_636A04;
extern SInt32 symdecloffset_0063627C;
extern UInt8 cinline_has_sideeffect;
extern CI_Action* cinline_actionlist;
extern CI_Action* cinline_tactionlist;

extern Boolean CInline_CallGraphInlining(Object* object, Statement* stmt);
extern Statement* CInline_Expand(Object* funcObject, Statement* stmt);
extern void fn_00509A50(ENode* expr);
extern void fn_00509AF0(ENode* node);

extern Boolean CInline_IsSmallFunction(Object* object, Statement* stmt);
extern Boolean fn_00507790(ObjectList* list);
extern Statement* CInline_ExtractInlineFunction(Statement* stmt);
extern Statement* CInline_SeqStringRewrite(Statement* stmt, Boolean* changed);
extern void fn_00507E20(
    Object* funcobj,
    TypeClass* tclass,
    FileOffsetInfo* fileoffset,
    TokenStream* stream,
    Boolean useInlineCache
);
extern void fn_00507D70(
    Object* funcobj, TemplClass* templ, TemplClassInst* inst, void* tmemb, Boolean useInlineCache
);
extern void fn_00507D00(Object* funcobj, void* func, TemplFuncInstance* inst);

#endif
