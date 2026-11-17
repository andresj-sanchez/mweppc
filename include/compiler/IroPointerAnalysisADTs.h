#ifndef COMPILER_IROPOINTERANALYSISADTS_H
#define COMPILER_IROPOINTERANALYSISADTS_H

#include "compiler/common.h"
#include "compiler/CInt64.h"
#include <setjmp.h>

typedef struct PAMemoryBlock PAMemoryBlock;

typedef struct LocationSet LocationSet;

typedef struct LocationSetSet LocationSetSet;

typedef struct PAHeapBlock PAHeapBlock;

typedef struct PALocalVar PALocalVar;

typedef struct ExtendedParam ExtendedParam;

struct PAHeapBlock {
    IROLinear* nd;
};

struct LocationSet {
    PAMemoryBlock* block;
    Type* rtype;
    union {
        struct {
            CInt64 field;
            UInt32 stride;
        } known;
        struct {
            PAMemoryBlock* restriction;
            LocationSet* bitfieldOf;
        } unknown;
    } u;
};

struct LocationSetSet {
    LocationSet* loc;
    LocationSetSet* otherLocs;
    UInt8 count;
};

typedef struct PointsToEntry PointsToEntry;

typedef struct PointsToFunction PointsToFunction;

typedef struct ParamMapping ParamMapping;

typedef struct ParamMappingFunction ParamMappingFunction;

typedef struct ObjectSet ObjectSet;

struct PointsToEntry {
    LocationSet* loc;
    LocationSetSet* locs;
};

struct PointsToFunction {
    PointsToEntry* pte;
    PointsToFunction* otherPtes;
};

struct ParamMapping {
    IROLinear* actual;
    Object* formal;
    void* extended;
};

struct ParamMappingFunction {
    ParamMapping* mapping;
    ParamMappingFunction* otherMappings;
};

struct ObjectSet {
    Object* obj;
    ObjectSet* otherObjs;
};

enum {
    PAMEMORYBLOCKKIND_INVALID,
    PAMEMORYBLOCKKIND_UNKNOWN,
    PAMEMORYBLOCKKIND_EXTENDEDPARAM,
    PAMEMORYBLOCKKIND_LOCALVAR,
    PAMEMORYBLOCKKIND_HEAPBLOCK,
    PAMEMORYBLOCKKIND_INT,
    PAMEMORYBLOCKKIND_OTHER
};

struct PAMemoryBlock {
    UInt8 kind;
    UInt8 unk01;
    UInt8 unk02;
    union {
        ExtendedParam* ep;
        PALocalVar* localvar;
        PAHeapBlock* heapblock;
        CInt64 intval;
        void* other;
        IROLinear* linear;
    } u;
};

typedef struct PTFList PTFList;

typedef struct ExtendedParamSet ExtendedParamSet;

typedef struct PartialTransferFunction PartialTransferFunction;

struct PartialTransferFunction {
    PointsToFunction* initialPointsToFn;
    PointsToFunction* finalPointsToFn;
    LocationSetSet* funcModifies;
    LocationSet* returnLocation;
    Boolean analyzed;
    struct {
        IROLinear* nd;
        PartialTransferFunction* ptf;
    } context;
};

struct PTFList {
    PartialTransferFunction* ptf;
    PTFList* otherPTFs;
};

struct ExtendedParamSet {
    void* ep;
    ExtendedParamSet* otherEps;
};

struct PALocalVar {
    Object* obj;
    char* name;
};

struct ExtendedParam {
    ObjectSet* objectSet;
    UInt32 num;
};

typedef struct PARegion PARegion;

typedef struct PARegionSet PARegionSet;

struct PARegion {
    UInt16 kind;
    UInt16 unk2;
    BitVector* thing;
};

struct PARegionSet {
    PARegion* region;
    PARegionSet* otherRegions;
};

typedef struct GetFunctionDepsOrKillsActionParams {
    Object* proc;
    PartialTransferFunction* ptf;
    IROLinear* funccall;
    ParamMappingFunction* map;
    ObjectList** list;
} GetFunctionDepsOrKillsActionParams;

typedef struct LookupLinearExprActionParams {
    Object* proc;
    Type* indirectType;
    IROListNode** list;
} LookupLinearExprActionParams;

typedef struct Stack Stack;

struct Stack {
    void* top;
    Stack* next;
};

typedef struct PAEvalCallActionParams {
    Object* proc;
    void* fnode;
    void* nd;
    PartialTransferFunction* ptf;
    void* map;
    UInt32 targetCount;
    UInt8 stop;
    UInt8 result;
    UInt8 visitTarget;
    UInt8 tail;
} PAEvalCallActionParams;

typedef struct PAFlowNode {
    UInt8 pad0[0x14];
    IROLinear* first;
    IROLinear* last;
    UInt8 pad1C[0x18];
    struct PAFlowNode* nextnode;
    ObjectSet* addressed;
} PAFlowNode;

typedef struct PAFuncLinear {
    UInt8 pad0[0x20];
    PointsToFunction* pointsToFunction;
    UInt8 pad24[0x1C];
    struct PAFuncLinear* next;
} PAFuncLinear;

typedef struct PAListSearchNode PAListSearchNode;

struct PAListSearchNode {
    PAListSearchNode* next;
    void* value;
};

typedef struct PA_BitVector {
    UInt32 size;
    UInt32 data[0];
} PA_BitVector;

typedef struct PARegionBitFilter {
    PARegion* skip;
    PARegionSet* regions;
    UInt16* index;
} PARegionBitFilter;

typedef struct PAEqualityContext {
    void* unused0;
    void* value;
    Boolean matched;
} PAEqualityContext;

typedef struct PACallMatchContext {
    void* entries;
    void* other;
    void* params;
    void* extended;
    void* matchedSecondary;
    void* matchedPrimary;
    void* active;
} PACallMatchContext;

typedef struct PASecondaryMatch {
    void* value;
    struct PASecondaryMatch* next;
} PASecondaryMatch;

typedef struct PABlockNode {
    PAMemoryBlock* block;
    struct PABlockNode* next;
} PABlockNode;

typedef struct PABlockContext {
    UInt8 pad[0x38];
    PABlockNode* list;
} PABlockContext;

typedef struct PAIndexValue {
    void* unused;
    UInt32 index;
} PAIndexValue;

typedef struct PAIndexEntry {
    struct PAIndexEntry* lower;
    struct PAIndexEntry* upper;
    PAIndexValue* value;
} PAIndexEntry;

typedef struct PAFunctionRecord {
    PointsToFunction* dependencies;
    PointsToFunction* kills;
    LocationSetSet* locations;
    void* other;
    UInt8 matched;
    UInt8 pad11[3];
    void* first;
    void* second;
} PAFunctionRecord;

typedef struct PAVisitContext {
    void* first;
    void* second;
    void* third;
    PointsToFunction* fourth;
    PointsToFunction* entries;
    LocationSet* loc;
    UInt8 changed;
    UInt8 padding[3];
} PAVisitContext;

extern Boolean fn_004DB110(void* first, void* second);
extern void InvalidatePointsToFunctions(Object* proc);
extern void PointerAnalysis_Term(void);
extern Boolean fn_004E56F0(void* first, void* second);
extern Boolean fn_004E8D70(void* target, void* other, void* entries, void* params, void* extended);
typedef struct MatchPTFActionParams {
    Object* proc;
    PartialTransferFunction* ptfCopy;
    ParamMappingFunction* mapCopy;
} MatchPTFActionParams;

extern void MatchPTFHelper(
    LocationSet* loc,
    LocationSetSet* locs,
    Object* proc,
    ParamMappingFunction* map,
    PartialTransferFunction* ptf
);
extern void MatchPTFAction1(PointsToEntry* tgtPTE, void* refcon);
extern const char PA_modeAddr[];
extern const char PA_modeAnsi[];
extern const char PA_modeType[];
extern UInt32 stMaxPassCount;
extern Object* stCurrentProc;
extern Object stUnknown;
extern jmp_buf stAbortPointerAnalysis;
extern ObjectList* stParamObjs;
extern Stack* stCallingContextStack;
extern UInt32 stExtendedParamNum;
extern UInt8 unk6367C2;
extern void* unk6367CC;
extern UInt16 unk6367D4;

extern void LocationSetSet_RemoveAll(LocationSetSet* lss);
extern void LocationSet_Term(LocationSet* ls);
extern Boolean LocationSets_Equal(LocationSet* first, LocationSet* second);
extern void LocationSetSet_SimpleAddImpl(LocationSetSet* lss, LocationSet* loc);
//! @reconstructed
extern void LocationSet_Copy(LocationSet* dest, LocationSet* src);
extern LocationSet* LocationSet_New(void);
extern void LocationSetSet_Add(LocationSetSet* lss, LocationSet* loc);
extern void LocationSetSet_Init(LocationSetSet* set);
extern LocationSetSet* LocationSetSet_New(void);
extern void LocationSetSet_Remove(LocationSetSet* dest, LocationSet* loc);
extern Boolean PointsToFunction_SimpleAdd(PointsToFunction* ptf, PointsToEntry* pte);
extern void PointsToEntry_Init(PointsToEntry* pte, LocationSet* loc, LocationSetSet* locs);
extern void fn_004D7590(PointsToFunction* ptf, PointsToEntry* pte);
extern void PointsToFunction_RemoveByLocationSet(PointsToFunction*, LocationSet*);
extern Boolean fn_004D84D0(PointsToFunction* ptf, PointsToEntry* pte);
extern int fn_004D8C70(PointsToFunction* ptf, LocationSet* loc);
extern void LocationSetSet_Copy(LocationSetSet* dest, LocationSetSet* src);
extern void DestroyFalseContext(
    Object* proc, PartialTransferFunction* ptf, ParamMappingFunction* map
);
extern void ParamMappingFunction_Term(ParamMappingFunction* pmf);
extern void CreateFalseContext(
    Object* proc, IROLinear* call, PartialTransferFunction** ptf, ParamMappingFunction** map
);
extern void GetFunctionDepsOrKillsAction2(PointsToEntry* pte, void* refcon);
extern void GetFunctionDepsOrKillsAction(LocationSet* loc, void* refcon);
extern void CreateExpressionForLocationSet(
    LocationSet* loc, IROList* list, Type* type, Object* proc
);
extern void fn_004DC4A0(LocationSet* loc, LocationSetSet* dest);
extern void PTFList_Term(PTFList* list);
extern void LocationSetSet_AddUnknown(
    LocationSetSet* set, Type* type, PAMemoryBlock* restriction, LocationSet* bitfieldOf
);
//! @reconstructed
extern void PointsToFunction_Init(PointsToFunction* ptf);
extern PointsToFunction* PointsToFunction_New(void);
extern void PARegionSet_Term(PARegionSet* set);
extern void PointsToFunction_Add(PointsToFunction* ptf, PointsToEntry* pte);
extern void EvalCallAction(Object* proc, PAEvalCallActionParams* params);
extern PAListSearchNode* fn_004E3DD0(void* proc);
extern void fn_004E5090(PointsToEntry* pte, PAVisitContext* context);
extern void fn_004E5150(PointsToEntry* pte, PAVisitContext* context);
extern Boolean fn_004E58D0(LocationSet* loc, void* first, void* second, void* third);
extern void GetActualLocsOfExtendedParam(
    LocationSetSet* dest,
    LocationSet* src,
    void* data,
    Stack** stackPtr,
    ParamMappingFunction* map,
    Boolean flag
);
extern Boolean fn_004E8B30(ObjectSet* objects, void* first, void* third, void* fifth, void* fourth);
extern void EvalVariable(
    LocationSetSet* set,
    Object* proc,
    void* var,
    PointsToFunction* ptf,
    void* first,
    void* second,
    void* third
);
//! @reconstructed
extern void EvalExpr(
    LocationSetSet* set, Object* proc, IROLinear* nd, void* first, void* second, void* third
);
extern Boolean LocationSetRepresentsSingleLocation(LocationSet* loc, void* first, void* second);
extern PAMemoryBlock* stUnknownMb;
extern LocationSet* stUnknownLs;
extern PTFList* stPTFList;
extern ExtendedParamSet* stExtParamSet;
extern PartialTransferFunction* stUnknownPTF;

extern char IRO_BitVectorFileName[];

#endif
