#ifndef COMPILER_LIVEREGISTERS_H
#define COMPILER_LIVEREGISTERS_H

#include "compiler/common.h"

typedef struct RegisterLiveBitmaps RegisterLiveBitmaps;
typedef struct VRegLiveSet VRegLiveSet;

struct RegisterLiveBitmaps {
    SInt32 registerCount;
    VRegLiveSet* blockBitmaps;
    UInt32* current;
};

struct VRegLiveSet {
    UInt32* use;
    UInt32* def;
    UInt32* in;
    UInt32* out;
};

extern RegisterLiveBitmaps register_live_bitmaps_6368D0[7];

typedef struct VRegBlockFlags VRegBlockFlags;
struct VRegBlockFlags {
    SInt32 start;
    SInt32 end;
    SInt32 liveIn;
    SInt32 liveOut;
};

typedef struct VRegFinalRegister VRegFinalRegister;
typedef struct VRegFinalRegisters VRegFinalRegisters;
struct VRegFinalRegister {
    SInt8 regClass;
    SInt8 reserved;
    SInt16 reg;
};
struct VRegFinalRegisters {
    SInt32 count;
    VRegFinalRegister regs[0];
};

extern void* live_registers_61B908;

extern void compute_live_registers_of_class(SInt8 cls);
extern SInt32 fn_004CF6D0(PCode* instr);
extern void update_live_registers(PCode* instr);
extern void capture_live_registers(PCodeBlock* block);
extern void compute_live_registers(void);
extern void fn_004CFA10(void);
extern void fn_004CFB80(SInt8 regClass);
extern void fn_004CFD30(SInt8 regClass);
extern void initialize_live_registers(void* data);

#endif
