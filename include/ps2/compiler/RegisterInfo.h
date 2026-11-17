#ifndef COMPILER_REGISTERINFO_H
#define COMPILER_REGISTERINFO_H

#include "compiler/common.h"

struct ObjRegInfo {
    UInt8 unk0[0x2E];
    SInt8 rclass;
    UInt8 flags;
    SInt16 unk30;
    SInt16 reg;
    SInt16 unk34;
    SInt16 unk36;
};

struct RegInfo {
    RegInfo* next;
    Object* object;
    HashNameNode* name;
    ObjRegInfo* reginfo;
    NameSpace* nspace;
    Boolean is_static;
};

extern RegInfo* reg_info_list;
extern SInt8 GetRegisterClassName(SInt8 regclass);

extern void* Registers_GetVarInfo(Object* object);

extern Boolean fn_004727A0(Object* object);

extern SInt8 coloring_class;
extern SInt16 stack_pointer_register;
extern char* target_register_class_names_6356C8[];
extern char* target_register_name_patterns_63663C[];
extern char register_name_gpr_5E6078[];
extern char register_pattern_gpr_5E607C[];
extern char register_name_fpr_5E6081[];
extern char register_pattern_fpr_5E6085[];
extern char register_name_special_5E608A[];
extern char register_pattern_special_5E6092[];
extern char register_name_coproc2_i_5E6097[];
extern char register_pattern_coproc2_i_5E60A1[];
extern char register_name_coproc2_ii_5E60A7[];
extern char register_name_coproc2_f_5E60B2[];
extern char register_pattern_coproc2_f_5E60BC[];
extern char register_name_coproc2_special_5E60C2[];
extern char register_pattern_coproc2_special_5E60D2[];
extern UInt8 scheduler_register_colors[7][32];
extern UInt32 reg_counter_gpr_635A6C;
extern UInt32 reg_counter_fpr_636028;
extern UInt32 reg_counter_63602C;
extern UInt32 reg_counter_636030;
extern UInt32 reg_counter_636034;
extern SInt32 register_reservation_limits_636150[];
extern SInt32 starting_available_registers[7];

extern Type* fn_00472850(IGNode* node);
extern PCode* fn_004728B0(SInt32 reg, IGNode* node);
extern PCode* fn_004729B0(SInt32 reg, IGNode* node);
extern SInt32 fn_004727F0(void);
extern void init_target_registers(void);

#define OBJECT_REG(obj) (((obj)->datatype == DLOCAL) ? (obj)->u.var.info->reg : 0)

extern SInt16 used_regs_before_coloring;

extern Boolean fn_00472800(SInt32 first, SInt32 second);

#endif
