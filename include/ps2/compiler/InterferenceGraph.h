#ifndef COMPILER_INTERFERENCEGRAPH_H
#define COMPILER_INTERFERENCEGRAPH_H

#include "compiler/common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct IGEdge {
    IGEdge* next;
    IGEdge* nextLow;
    IGEdge* nextHigh;
    SInt16 low;
    SInt16 high;
};

struct IGNode {
    IGNode* next;
    Object* spillTemporary;
    PCode* instr;
    SInt32 spillCost;
    SInt16 reg;
    SInt16 color;
    UInt16 flags;
    SInt16 degree;
    UInt16 liveEdges;
    UInt16 unknown1A;
    IGEdge* edges;
};

extern IGNode* interferencegraph;
extern IGEdge* interference_hash[16384];
extern SInt16* coalesced;

extern void buildinterferencegraph(Object* proc);
extern SInt32 interferes(UInt32 lhs, UInt32 rhs);
extern void makeinterfere(UInt32 lhs, UInt32 rhs);

extern void fn_004BE090(void);
extern void fn_004BE240(void);
extern void fn_004BE350(void);
extern void fn_004BE4B0(void);
extern SInt32 fn_004BE5C0(void);
extern void fn_004BE9B0(void);
extern void fn_004BEE10(void);

extern UInt32 ig_graph_count;
extern UInt32 ig_graph_size;
extern UInt32* ig_graph_ids;
extern UInt32* ig_graph_positions;
extern char ig_source_file[];

#ifdef __cplusplus
}
#endif

#endif
