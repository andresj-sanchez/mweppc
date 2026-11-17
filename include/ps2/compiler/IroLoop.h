#ifndef COMPILER_IROLOOP_H
#define COMPILER_IROLOOP_H

#include "compiler/common.h"
#include "compiler/CFunc.h"
#include "compiler/CInt64.h"
#include "compiler/IroVars.h"

typedef enum IROLinearType {
    IROLinearNop = 0,
    IROLinearOperand = 1,
    IROLinearOp1Arg = 2,
    IROLinearOp2Arg = 3,
    IROLinearOp3Arg = 4,
    IROLinear5 = 5,
    IROLinearFunccall = 6,
    IROLinearGoto = 7,
    IROLinearGotoExpr = 8,
    IROLinearIf = 9,
    IROLinearIfNot = 10,
    IROLinearReturn = 11,
    IROLinearLabel = 12,
    IROLinearSwitch = 13,
    IROLinearBeginCatch = 16,
    IROLinearEndCatch = 17,
    IROLinearEndCatchDtor = 18,
    IROLinearAsm = 19,
    IROLinearEnd = 22
} IROLinearType;

enum {
    IROLF_StartsNode = 0x1,
    IROLF_Reffed = 0x2,
    IROLF_Used = 0x10,
    IROLF_VecOpBase = 0x10000
};

struct IROLinear {
    UInt8 type;
    UInt8 nodetype;
    UInt32 flags;
    UInt16 nodeflags;
    UInt16 unkA;
    SInt32 index;
    Statement* stmt;
    Type* rtype;
    Type* unk18;
    union {
        SInt32 unk1C;
        IROExpr* expr;
    };
    struct PointsToFunction* pointsToFunction;
    SInt32 unk24;
    SInt32 unk28;
    union {
        IROLinear* monadic;
        ENode* node;
        struct {
            IROLinear* left;
            IROLinear* right;
        } diadic;
        struct {
            IROLinear* a;
            IROLinear* b;
            IROLinear* c;
        } args3;
        CLabel* label;
        struct {
            struct SwitchInfo* info;
        } swtch;
        struct {
            CLabel* label;
            SInt32 unk30;
            struct IROCounterLoop* counterLoop;
        } branch;
        Statement* asm_stmt;
        struct {
            char ispascal;
            short argCount;
            IROLinear** args;
            IROLinear* callee;
        } funccall;
    } u;
    SInt32 unk38;
    SInt32 unk3C;
    IROLinear* next;
};

struct IROList {
    IROLinear* head;
    IROLinear* tail;
};

struct IROListNode {
    IROList list;
    struct IROListNode* nextList;
};

struct IROLoopInd {
    UInt8 flags;
    UInt8 unk1[3];
    VarRecord* var;
    UInt8 unk8[4];
    IROLinear* nd;
    IROLinear* init;
    UInt8 unk14[0x24];
    void* unk38;
    CInt64 step;
    UInt32 unk4C;
    IROLoopInd* next;
};

struct IROLoop {
    UInt32 flags;
    UInt32 unk4;
    BitVector* bitset;
    UInt32 unkC;
    IRONode* unk10;
    IRONode* unk14;
    void* unk18;
    IRONode* preheader;
    IRONode* unk20;
    IROLinear* cond;
    SInt32 unk28;
    IROLoopInd* induction;
    SInt32 unk30;
    Boolean countKnown;
    CInt64 count;
    IROList countExpr;
    UInt32 unk50;
    IROLinear* unk54;
    SInt32 weight;
    UInt8 unk5C[0x14];
    IROLoopInd* extraInductions; /* 0x70 */
    struct IROLoop* next; /* 0x74 */
};

extern IROLinear* IRO_InductionLowerBound(IROLoop* loop, IROLoopInd* induction);
//! @reconstructed
extern IROLinear* IRO_PrimaryInductionUpperBound(IROLoop* loop); //! @reconstructed
extern CInt64 IRO_GetAdjustedInductionIncrement(IROLoopInd* induction);
extern void IRO_FoldConstantsInExpr(IROLinear* count);

extern void IRO_FindUniqueInLoopHeaderTarget(IROLoop* loop);
extern void IRO_MarkLoopTestNodes(IROLoop* loop);
extern void IRO_UnmarkLoopTestNodes(IROLoop* loop);
extern void IRO_MarkPrimaryInductionAssignmentNodes(IROLoop* loop);
extern void IRO_UnmarkPrimaryInductionAssignmentNodes(IROLoop* loop);
extern void fn_0059DCF0(IROLoop* loop);
extern void fn_0059A290(IROLoop* loop, int flag);
extern Boolean IRO_IsLinearNodeInLoopTest(IROLoop* loop, IROLinear* node);
extern Boolean IRO_IsLinearNodeInInductionUpdate(IROLoop* loop, IROLinear* node);
extern void IRO_SortLoopsByIncreasingNest(void);
extern IROLoop* IRO_FindLoops(void);
extern void IRO_FindLoopNodes(IROLoop* loop);
extern void IRO_MarkInnerLoops(IROLoop* loop);
extern void IRO_FreeLoopList(IROLoop* loop);
extern void IRO_FindUniqueSuccessorNode(IROLoop* loop, IRONode** first, IRONode** second);
extern Boolean IRO_CopyLoopTestToLoopPreheaderAndBranchToSuccNode(
    IROLoop* loop, IRONode** first, IRONode* second, CInt64* count, SInt32 control
);

extern IRONode* IRO_CreatePreHeader(IROLoop* loop, IRONode* node, IRONode* before);

#endif
