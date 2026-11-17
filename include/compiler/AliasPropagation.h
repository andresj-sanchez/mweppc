#ifndef COMPILER_ALIASPROPAGATION_H
#define COMPILER_ALIASPROPAGATION_H

#include "compiler/common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void fn_0054AC50(PCode* first, PCode* second);
extern void propagatealiasinfo(void* function);
extern void finishpropagatealiases(void);
extern SInt32 addresspropagatestouse(SInt32 candidateID, SInt32 useID);
extern void fn_0054AEE0(PCode* use_pcode, SInt32 reg, Alias* reference);
extern Alias* fn_0054B2E0(PCode* pcode);
extern SInt32 is_address_load(PCode* pcode);
extern Alias* fn_0054B360(Operand* operand, PCode* instr);

extern SInt32 alias_graph_dirty;
extern SInt32 alias_propagation_enabled;
extern char alias_propagation_name[];
extern char alias_source_file[];

#ifdef __cplusplus
}
#endif

#endif
