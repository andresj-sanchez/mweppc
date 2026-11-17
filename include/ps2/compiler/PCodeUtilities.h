#ifndef COMPILER_PCODEUTILITIES_H
#define COMPILER_PCODEUTILITIES_H

#include "compiler/common.h"
#include "compiler/PCode.h"

extern void note_object_alignment(int align, Object* object); //! @reconstructed
extern PCode* make_instruction(int opcode, short argCount); //! @reconstructed
extern void branch_always(PCodeLabel* target);
extern void fn_0048F530(int opcode, short dstReg, short srcReg, SInt32 value);
extern PCode* fn_0048F5D0(short opcode, short destReg, short baseReg, Object* object, int offset);
extern PCode* fn_0048F710(short opcode, short destReg, short baseReg, Object* object, int offset);

extern void fn_0048F850(short opcode, short destReg, short baseReg, Object* object, int offset);
extern void fn_0048F990(short opcode, short destReg, short baseReg, Object* object, int offset);

#endif
