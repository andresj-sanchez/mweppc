#ifndef COMPILER_OPERANDS_H
#define COMPILER_OPERANDS_H

#include "compiler/common.h"

extern SInt32 be_macro_adjust_offset_6356A4;

extern SwitchCase* switch_current_6366D0;
extern SInt32 switch_count_6090D4;
extern Operand switch_operand_635BB4;
extern double switch_table_density_5E8F70;

extern Operand get_high_half_operand(Operand* operand); //! @reconstructed
extern void force_operand_register(Operand* operand, Type* type, int reg); //! @reconstructed
extern Operand convert_operand_type(Operand* op, Type* type); //! @reconstructed
extern int move_fpr_register(int dest, int source, char kind, int width); //! @reconstructed
extern int move_operand_register(int dest, int source, char kind, int width); //! @reconstructed

extern void extract_bitfield(Operand operand, TypeBitfield* tbitfield, Operand* result);
extern void fn_004B0B30(Operand source, Operand* target, TypeBitfield* bitfield);
extern void Coerce_to_register(Operand* op, Type* type, SInt16 reg);
extern void indirect(Operand* op, ENode* node); //! @reconstructed

extern short object_base_register(Object* object); //! @reconstructed
extern Operand get_call_result_int(int wide); //! @reconstructed
extern Operand get_call_result(Type* type); //! @reconstructed
extern void pass_int_argument(Operand* op, int wide, int index); //! @reconstructed
extern void pass_argument(Operand* op, Type* type, int index); //! @reconstructed
extern void call_runtime_function(Object* function); //! @reconstructed

extern SInt32 fn_004AFCC0(Operand* op);
extern void assign_local_memory(Object* object);
extern void fn_004AFDE0(void);

extern SInt32 fn_004AFC70(Object* object);

extern UInt32 be_macro_arg_float_mask_609074;
extern UInt32 be_macro_arg_register_mask_609070;

extern void fn_004B1440(Operand source, Operand* target, Type* type);

#endif
