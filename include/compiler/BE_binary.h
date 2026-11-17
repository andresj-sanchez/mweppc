#ifndef COMPILER_BE_BINARY_H
#define COMPILER_BE_BINARY_H

#include "compiler/common.h"

extern SInt32 binary_alignment;
extern volatile SInt32 binary_word_index;
extern SInt32 binary_byte_offset;
extern ObjectList* binary_fixup_list;
extern CodeGenFunction* binary_word_count_destination;

extern void pad_short_loop(PCodeBlock* block); //! @reconstructed

extern void fn_004C2800(Object* section);
extern void fn_004C2B60(PCode* pcode);
extern void fn_004C2C40(PCode* pcode, int pass);
extern void fn_004C2BE0(void);

#endif
