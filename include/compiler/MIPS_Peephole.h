#ifndef COMPILER_MIPS_PEEPHOLE_H
#define COMPILER_MIPS_PEEPHOLE_H

#include "compiler/common.h"

extern int fn_0055AA00(PCode* pc);
extern int fn_0055AA30(PCode* pc);
extern int fn_0055AA80(PCode* pc);
extern int fn_0055B800(PCode* pc, int opcode, int typeOne, int typeTwo, int mode);
extern int fn_0055BB50(PCode* pc, int opcode, int leftOp, int rightOp, int reg);

extern UInt8 fn_0055BDA0(PCode** first, PCode** second, int change);
extern int fn_0055BE00(PCode* pc);
extern int fn_0055BEE0(PCode* pc);

extern CInt64 peephole_const_5E613C;
extern CInt64 peephole_const_5E614C;
extern UInt32 peephole_flags_634060;

extern int fn_00557DD0(PCode* pc);
extern int fn_00558170(PCode* pc);

extern void fn_00557180(void);

extern void fn_0055D750(void);

extern int fn_00558610(PCode* pc, UInt16 opcode, int which, PCode* other);
extern int fn_00559D80(PCode* pc, UInt16 opcode);
extern int fn_00559FE0(
    PCode* pc,
    UInt16 opcode1,
    UInt16 opcode2,
    UInt16 opcode3,
    UInt16 opcode4,
    UInt16 opcode5,
    SInt32 mode
);
extern int fn_0055AAB0(PCode* pc, UInt16 first, UInt16 second, UInt16 replacement);
extern int fn_0055AED0(PCode* pc, UInt16 first, UInt16 second, UInt16 replacement);
extern int fn_0055B330(PCode* pc, UInt16 opcode1, UInt16 opcode2, UInt16 opcode3);

typedef struct PeepholeRegisterState {
    SInt32* words;
    SInt32 count;
    SInt32 available;
} PeepholeRegisterState;

extern PeepholeRegisterState peephole_register_state_636220[7];

#endif
