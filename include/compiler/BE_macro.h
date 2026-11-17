#ifndef COMPILER_BE_MACRO_H
#define COMPILER_BE_MACRO_H

#include "compiler/common.h"
#include "compiler/DWARF.h"

extern void fn_00495D60(void* object);
extern void fn_00495DA0(PCode* pc);
extern void fn_00496520(PCode** current, UInt8 direction);
extern void fn_0049A4B0(PCode* pc);
extern void fn_0049A620(UInt8 save, PCode* pc);
extern void fn_0049A8F0(PCodeBlock* last);
extern void fn_0049ACB0(PCode* pc);
extern void fn_0049AD60(PCode* pc);
extern void fn_0049AE00(PCode* pc);
extern void fn_0049AE70(UInt8 save, PCode* pc, UInt32* registers);
extern SInt32 fn_0049B080(PCodeBlock* block, UInt32* mask);

extern CInt64 be_macro_split_immediate;
extern PCodeBlock* be_macro_current_block_636058;
extern SInt16 be_macro_expanded_opcode2_637082[];
extern SInt16 be_macro_expanded_opcode_6371D0[];
extern SInt32 asm_stack_frame_offset_636E78;
extern DwarfLocationOperand be_macro_save_header_625300;
extern DwarfRegisterSet be_macro_gp_save_records_625AA8;
extern DwarfRegisterSet be_macro_fp_single_save_records_621280;
extern DwarfRegisterSet be_macro_fp_save_records_621080;
extern SInt32 be_macro_accumulated_stack_6361CC;
extern SInt32 be_macro_debug_register_count_636930;
extern SInt32 be_macro_fp_first_635F4C;
extern SInt32 be_macro_frame_header_608DDC;
extern SInt32 be_macro_frame_padding2_6364CC;
extern SInt32 be_macro_frame_padding_636438;
extern SInt32 be_macro_gp_first_6361D4;
extern SInt32 be_macro_gp_last_63618C;
extern SInt32 be_macro_immediate_63613C;
extern SInt32 be_macro_reserved_offset_608DD8;
extern SInt32 be_macro_reserved_reg_608DE0;
extern SInt32 be_macro_special_registers_6253B8[];
extern SInt32 be_macro_stack_base_635A40;
extern UInt16 be_macro_saved_fp_reg_637338;
extern UInt16 be_macro_saved_gp_reg_63732E;
extern UInt32 be_macro_mask_buffer_608DE4[];
extern UInt8 be_macro_reserved_reg_used_608E04;
extern void* be_macro_active_object_608E00;

#endif
