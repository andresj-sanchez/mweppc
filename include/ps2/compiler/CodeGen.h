#ifndef COMPILER_CODEGEN_H
#define COMPILER_CODEGEN_H

#include "compiler/common.h"
#include "compiler/types.h"
#include "compiler/CFunc.h"
#include "compiler/CParser.h"
#include "driver/CWPlugins.h"

typedef struct CodeGenLiveRegister {
    UInt8 cls;
    UInt8 unused;
    SInt16 reg;
} CodeGenLiveRegister;

extern UInt16 move_opcode; //! @reconstructed
extern UInt16 move_opcode_637350;
extern UInt8 fn_00436C70(Type* type);
extern UInt32 cg_option_0063646C;
extern Object* fn_00438B60(Object* object);
extern UInt16 target_register_637328;

struct CodeGenFunction {
    SInt32 wordCount;
    UInt32 gprmask;
    UInt32 fprmask;
};

extern Boolean codegen_flag_637383; //! @reconstructed
extern Boolean codegen_flag_63738A; //! @reconstructed

extern Boolean made_tail_call; //! @reconstructed

extern Statement* current_statement;

extern CLabel* returnlabel; //! @reconstructed
extern Boolean branches_to_return; //! @reconstructed

extern SInt32 stack_alignment_63560C;
extern SInt32 register_size; //! @reconstructed

#define IS_TYPE_FLOAT_HARDWARE(ty) \
    (IS_TYPE_FLOAT(ty) && \
     ((IS_TYPE_FLOAT32(ty) && (copts.fp_library & 1)) || \
      (IS_TYPE_FLOAT64(ty) && (copts.fp_library & 2)))) //! @reconstructed
#define IS_TYPE_FLOAT_SOFTWARE(ty) \
    (IS_TYPE_FLOAT(ty) && \
     ((IS_TYPE_FLOAT32(ty) && !(copts.fp_library & 1)) || \
      (IS_TYPE_FLOAT64(ty) && !(copts.fp_library & 2)))) //! @reconstructed
extern UInt16 mul3_opcode; //! @reconstructed
extern UInt16 dmul3_opcode; //! @reconstructed
extern UInt16 ddiv3_opcode; //! @reconstructed
extern UInt16 div3_opcode; //! @reconstructed

extern Object* rt_dpflt; //! @reconstructed
extern Object* rt_f_flt; //! @reconstructed
extern Object* rt_dpfgt; //! @reconstructed
extern Object* rt_f_fgt; //! @reconstructed
extern Object* rt_dpfle; //! @reconstructed
extern Object* rt_f_fle; //! @reconstructed
extern Object* rt_dpfge; //! @reconstructed
extern Object* rt_f_fge; //! @reconstructed
extern Object* rt_dpfeq; //! @reconstructed
extern Object* rt_f_feq; //! @reconstructed
extern Object* rt_dpfne; //! @reconstructed
extern Object* rt_f_fne; //! @reconstructed

extern Object* rt_l_muls; //! @reconstructed
extern Object* rt_dpmul; //! @reconstructed
extern Object* rt_f_mul; //! @reconstructed
extern Object* rt_f_add;
extern Object* rt_dpadd;
extern Object* rt_f_sub;
extern Object* rt_dpsub;
extern Object* rt_l_mulu; //! @reconstructed

extern Object* rt_moddi3; //! @reconstructed
extern Object* rt_umoddi3; //! @reconstructed
extern Object* rt_l_divs; //! @reconstructed
extern Object* rt_divdi3; //! @reconstructed
extern Object* rt_f_div; //! @reconstructed
extern Object* rt_l_divu; //! @reconstructed
extern Object* rt_dpdiv; //! @reconstructed
extern Object* rt_udivdi3; //! @reconstructed

extern SInt32 fn_00438D70(void);

struct CodeGenEmitRecord {
    struct CodeGenEmitRecord* next;
    Object* key;
    SInt32 value;
    SInt32 reserved;
    SInt16 kind;
    SInt16 padding;
};

extern struct CodeGenEmitRecord* volatile codegen_deferred_list_first_600B88;
extern struct CodeGenEmitRecord* codegen_deferred_list_last_600B8C;

extern void fn_004354E0(void);
extern void fn_00436E50(
    Object* object, PCode* instr, struct CodeGenEmitRecord** head, struct CodeGenEmitRecord** tail
);
extern void fn_00437B50(void);
extern void fn_00437DB0(void);
extern void fn_00437FA0(void);

extern void fn_00435D60(Object* object, SInt32 skip, SInt32 output, SInt32 inverted);
extern void fn_00434540(
    Statement* statements, Object* function, UInt8 mysteryflag, Boolean callOnModuleBind
);
extern void fn_00434160(Object* function, Object* obj, SInt32 offset, SInt32 reserved, SInt32 flag);

extern Object* rt_f_ulltof;
extern Object* rt_fixunsdfdi;
extern Object* rt_l_mod;
extern Object* rt_fixunssfdi;
extern Object* rt_MIPS16_prtglue32;
extern Object* rt_d_ulltod;
extern Object* rt_fp_round;
extern Object* rt_MIPS16_prtglue16;
extern Object* rt_fp_get_fpscr;
extern Object* rt_fp_set_fpscr;
extern Object* rt_l_modu;
extern Object* rt_dpcmp;
extern Object* rt_dpfun;
extern Object* rt_ashldi3;
extern Object* rt_ashrdi3;
extern Object* rt_dpfor;
extern Object* rt_profile_e;
extern Object* rt_profile_s;
extern Object* rt_f_fun;
extern Object* rt_f_cmp;
extern Object* rt_f_for;
extern Object* rt_dptofp;
extern Object* rt_strcpy;
extern Object* rt_dptoli;
extern Object* rt_dptoul;
extern Object* rt_f_itof;
extern Object* rt_f_cmpe;
extern Object* rt_lshrdi3;
extern Object* rt_f_utof;
extern Object* rt_memcpy;
extern Object* rt_dpcmpe;
extern Object* rt_d_utod;
extern Object* rt_fptodp;
extern Object* rt_fptosi;
extern Object* rt_litodp;
extern Object* rt_fptoui;
extern Object* rt_muldi3;
extern Object* rt_fixsfdi;
extern Object* rt_floatdisf;
extern Object* rt_umuldi3;
extern Object* rt_fixdfdi;
extern Object* rt_floatdidf;

extern Operand fn_00438330(Operand* op);
extern Operand fn_004383F0(Operand* op);
extern void fn_004384D0(Operand* result);

extern Boolean codegen_license_active_600BA0;
extern ObjectList* codegen_temp_objects_600B94;
extern ObjectList* codegen_allocated_objects_600B90;
extern Operand codegen_result_636784;
extern Operand codegen_operand_635F6C;
extern Operand codegen_operand_636958;
extern SInt16 cg_arch_register_type_5C9A7E;
extern SInt32 asm_source_offset_6361A0;
extern CLabel* cleanreturnlabel;
extern ObjectList* cg_codegen_tmp_636148;
extern SInt32 cg_codegen_pending_635ABC;
extern SInt32 cg_codegen_reg_635FA0;
extern SInt32 codegen_live_register_count_600740;
extern SInt32 codegen_mode_635B68;
extern SInt32 codegen_operand_max_6363EC;
extern SInt32 codegen_register_6361E4;
extern SInt32 codegen_register_6361E8;
extern SInt32 codegen_register_init_flag_635AB0;
extern SInt32 codegen_register_scan_636020;
extern SInt32 codegen_stack_offset_600B9C;
extern Type* codegen_active_type_600B98;
extern UInt16 cg_arch_register_type_5C9A80;
extern UInt16 cg_option_0063706A;
extern UInt16 mul3_opcode;
extern UInt16 cg_option_0063731C;
extern UInt16 cg_option_00637322;
extern UInt16 cg_option_00637326;
extern UInt16 cg_option_0063734E;
extern UInt16 peephole_opcode_636FCC;
extern UInt16 peephole_opcode_637054;
extern UInt32 cg_option_00636780;
extern UInt8 cg_alias_option_63738E;
extern UInt8 cg_codegen_done_flag_6374AB;
extern UInt8 cg_option_00637387;
extern UInt8 codegen_prefbyte_5C9A82;
extern UInt8 codegen_prefbyte_5C9A91;
extern UInt8 codegen_reserved_registers_622580[16];
extern char codegen_builtin_next_arg_name[];
extern SInt32 codegen_license_context_636D3C;

extern void assign_labels(Statement* stmt);

extern void CodeGen_GetLicense(CWPluginContext context); //! @reconstructed

extern void fn_00433EF0(Object* obj);

extern Boolean CodeGen_IsPublicRuntimeObject(Object* object);
extern Boolean CodeGen_ParseAttribute(void);
extern void fn_00434FF0(Boolean useAlternate);
extern void fn_00436FA0(Object* function, SInt32 emit);
extern void fn_00437300(SInt32 maximum, UInt32 weight);

extern CodeGenLiveRegister codegen_live_registers_600744[129];
extern Boolean cg_pcode_uses_registers_637392;
extern UInt8 codegen_register_option_637385;
extern Object* codegen_current_function_636498;
extern Boolean CodeGen_ReInitRuntimeObjects(Boolean is_precompiler);
extern void CodeGen_SetupRuntimeObjects(void);
extern UInt32 cg_debug_register_mask_636208;
extern SInt32 cg_codegen_reserved_635C08;
extern SInt32 codegen_reg_634814;
extern void CodeGen_GetAsmEffects(Statement* stmt, IAEffects* effects);
extern void CodeGen_PropagateIntoAsm(Statement* stmt, Object* object, ENode* expr);

#endif
