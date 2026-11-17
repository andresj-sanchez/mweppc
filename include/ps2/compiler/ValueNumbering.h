#ifndef COMPILER_VALUENUMBERING_H
#define COMPILER_VALUENUMBERING_H

#include "compiler/common.h"
#include "compiler/PCode.h"
#include "compiler/LoopDetection.h"

struct ValueLabel {
    ValueLabel* next;
    PCodeArg op;
};

struct AvailableValue {
    AvailableValue* next;
    ValueLabel* labelled;
    PCode* pcode;
    int killedregister;
    int aliasnumber;
    int opnumbers[0];
};

struct RegValue {
    int number;
    int copyreg;
    AvailableValue* available;
};

struct State {
    void* stackedvalues;
    int valueceiling;
};

struct StackedValue {
    StackedValue* next;
    PCodeArg op;
    RegValue value;
    Alias* alias;
    PCode* valuepcode;
};

extern void killmemory(Alias* alias, PCode* pcode);
extern StackedValue* stackedvalues;
extern RegValue* regvalue[7];

extern void fn_004BC860(PCode* instr);
extern void fn_004BCFC0(PCode* instr);
extern SInt32 fn_004BD400(AvailableValue* node, PCode* instr);
extern SInt32 fn_004BD1D0(PCode* instr, PCodeArg* result);
extern void initializecsedatastructures(void);
extern SInt32 fn_004BDE20(PCode* instr, LoopRegisterNode* context);

extern void fn_004BDAD0(Object* proc);
extern void removecommonsubexpressions(SInt32 moreAggressive);

extern SInt32 cg_codegen_flags_6364C8;
extern SInt32 cg_codegen_setup_6359B0;
extern SInt32 nextvaluenumber;
extern AvailableValue* opvalue[1199];
extern SInt32 valueceiling;
extern SInt32 moreaggressiveoptimization;
extern SInt32* value_register_defs_60B6D0[7];
extern SInt32* value_register_uses_60B6EC[7];

extern void fn_004BBAE0(PCodeBlock* block);
extern void fn_004BBBC0(void);
extern void fn_004BBCE0(PCodeBlock* block);
extern void fn_004BD690(PCodeArg* source, PCodeArg* destination);
extern void fn_004BD7E0(void);
extern void fn_004BDB70(void* context);
extern void fn_004BDF30(void* loop);
extern void fn_004BDD00(PCode* instr, LoopRegisterNode* context);

extern void fn_004BC9B0(PCode*, SInt8);
extern void fn_004BCC20(PCode*, SInt8);

#endif
