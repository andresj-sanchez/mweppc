#ifndef COMPILER_CFUNC_H
#define COMPILER_CFUNC_H

#include "compiler/common.h"

struct CLabel {
    struct CLabel* next;
    struct Statement* stmt;
    struct HashNameNode* uniquename;
    struct HashNameNode* name;
    PCodeLabel* pclabel;
};

typedef enum StatementType {
    ST_NOP = 1,
    ST_LABEL,
    ST_GOTO,
    ST_EXPRESSION,
    ST_SWITCH,
    ST_IFGOTO,
    ST_IFNGOTO,
    ST_RETURN,
    ST_OVF,
    ST_EXIT,
    ST_ENTRY,
    ST_BEGINCATCH,
    ST_ENDCATCH,
    ST_ENDCATCHDTOR,
    ST_GOTOEXPR,
    ST_ASM,
    ST_BEGINLOOP,
    ST_ENDLOOP,
    ST_ILLEGAL
} StatementType;

struct Statement {
    struct Statement* next;
    UInt8 type;
    UInt8 marked;
    UInt8 flags;
    UInt16 value;
    ENode* expr;
    CLabel* label;
    struct ExceptionAction* dobjstack;
    SInt32 sourceoffset;
    HashNameNode* sourcefilepath;
};

struct InitExpr {
    struct InitExpr* next;
    ENode* expr;
    Object* object;
};

struct CtorChain {
    struct CtorChain* next;
    ENode* expr;
    void* unk8;
    UInt8 unkC;
    Boolean isexpr;
};

extern void CFunc_SetupNewFuncArgs(Object* func, FuncArg* args);
extern NameSpace* CFunc_FuncGenSetup(Statement* stmt, Object* object);
extern Object* create_temp_object(Type* type);
extern CLabel* newlabel(void);
extern void fn_00473CD0(Object* obj, void* decl, void* tclass, int arg3, int arg4, void* arg5);
extern Statement* CFunc_AppendStatement(StatementType type);
extern Statement* CFunc_InsertStatement(StatementType type, Statement* stmt);
extern Statement* CFunc_InsertBeforeStatement(StatementType type, Statement* stmt);
extern void CheckCLabels(void);
extern void CFunc_SetupLocalVarInfo(Object* obj);

extern UInt16 codegen_varnumber_637340;
extern SInt32 cfunc_staticvarcount;
extern SInt32 etempid;
extern UInt8 unk605EC0;
extern SInt32 functionbodyoffset;
extern void* functionbodypath;
extern InitExpr* init_expressions;
extern Statement* fn_0047ABD0(Statement* first);
extern Statement* DestructorIfTransform(Statement* stmt);
extern void DestructorReturnTransform(Statement* stmt1, Statement* stmt2);
extern CLabel* Labels;
extern Statement* curstmt;
extern SInt32 curstmtvalue;
extern SInt32 sourceoffset;
extern HashNameNode* sourcefilepath;

enum {
    StmtFlag_KeepLabel = 1,
    StmtFlag_EndsConstruction = 2,
    StmtFlag_LoopBranch = 4,
    StmtFlag_AfterNoReturn = 8,
    StmtFlag_Unrolled = 0x10
};

extern FuncArg elipsis;
extern FuncArg oldstyle;
extern ENode* create_temp_node(Type* type);
extern ENode* create_temp_node2(Type* type);

struct DeclBlock {
    struct DeclBlock* next;
    struct ExceptionAction* dobjstack;
    NameSpace* parent_nspace;
    UInt16 index;
};
extern DeclBlock* CFunc_NewDeclBlock(void);

extern void fn_0047BCC0(Object* object);

#endif
