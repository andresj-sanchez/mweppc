#ifndef COMPILER_IROFLOWGRAPH_H
#define COMPILER_IROFLOWGRAPH_H

#include "compiler/common.h"

struct IRONode {
    UInt16 index;
    UInt16 numsucc;
    UInt16* succ;
    UInt16 numpred;
    UInt16* pred;
    UInt16 loopdepth; // 0x10
    IROLinear* first; // 0x14
    IROLinear* last; // 0x18
    BitVector* in; // 0x1C
    BitVector* out;
    BitVector* gen;
    BitVector* kill;
    BitVector* copyOut; // 0x2C
    BitVector* dom; // 0x30
    IRONode* nextnode; // 0x34
    struct ObjectSet* addressed; // 0x38
    Boolean reachable; // 0x3C
    Boolean visited;
    Boolean mustreach;
    Boolean smartMustReach; // 0x3F
    Boolean marked; // 0x40
};

extern IRONode* IRO_FirstNode;

extern void IRO_BuildNodeTable(void);
extern void IRO_ComputeDom(void);
extern void fn_0058C9B0(void);
extern void IRO_ComputeSuccPred(void);
extern void IRO_BuildFlowgraph(IROLinear* linear);
extern IRONode* IRO_NewFlowGraphNode(void);
extern IRONode* IRO_MergeFlowGraphNodes(IRONode* a, IRONode* b);

extern IRONode* IRO_FunctionFirstNode(Object* proc);

extern BitVector* IRO_VarKills;
extern BitVector* IRO_ExprKills;
extern BitVector* IRO_FuncKills;
extern BitVector* IRO_Avail;
extern IRONode** IRO_NodeTable;
extern UInt16 IRO_NumNodes;
extern IRONode* IRO_LastNode;

#endif
