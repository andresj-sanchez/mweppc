#ifndef COMPILER_REGISTERS_H
#define COMPILER_REGISTERS_H

#include "compiler/common.h"
typedef int RegClass;
enum {
    RegClass_GPR = 0,
    RegClass_FPR = 1,
    RegClassMax = 7
};

#ifdef __cplusplus
extern "C" {
#endif

enum {
    RegisterInvalid = -1,
    Register0 = 0,
    Register2 = 2,
    Register3 = 3,
    Register4 = 4,
    RegisterMax = 32
};

enum {
    RegState_Free = 0,
    RegState_Retained = 1,
    RegState_Reserved = 2
};

extern SInt32 used_virtual_registers[RegClassMax];
extern SInt32 used_nonvolatile_registers[RegClassMax];
extern SInt32 assignable_registers[RegClassMax];
extern SInt32 n_real_registers[RegClassMax];
extern SInt32 n_scratch_registers[RegClassMax];
extern SInt32 scratch_registers[RegClassMax][RegisterMax];
extern SInt32 n_nonvolatile_registers[RegClassMax];
extern SInt32 nonvolatile_registers[RegClassMax][RegisterMax];
extern UInt8 reg_state[RegClassMax][RegisterMax];
extern SInt32 last_temporary_register[RegClassMax];
extern SInt32 first_temporary_register[RegClassMax];
extern void* register_code_context;
extern char register_assert_source[];
extern UInt8 register_character_class[256];
extern UInt8 register_newline_seen;
extern char* register_class_name[RegClassMax];
extern char* register_class_format[RegClassMax];
extern int coloring;
extern int optimizing;

struct RegisterMaskRecord {
    RegisterMaskRecord* next;
    Object* object;
    UInt32 mask[RegClassMax];
};

extern RegisterMaskRecord* register_mask_heads[128];
extern void fn_0046D200(Object* obj, UInt32 mask[RegClassMax]);
extern void fn_0046D290(Object* obj);
extern void fn_0046D2E0(UInt32 mask[RegClassMax]);

struct RegisterDebugSymbolRecord {
    UInt32 address;
    UInt32 size;
    UInt32 encoding;
};
extern char register_debug_name[];
extern char register_debug_label[];
extern char register_debug_format[];
extern SInt32 register_debug_type_index;
extern SInt32 register_debug_scope_index;
extern char register_dbg_prefix_a[];
extern char register_dbg_prefix_b[];

struct RegisterDebugShortRecord {
    UInt8 format;
    UInt8 count;
    UInt16 reg;
    UInt32 address;
    UInt16 locations[0];
};

struct RegisterDebugLongRecord {
    UInt8 format;
    UInt8 reserved;
    UInt16 count;
    UInt32 reg;
    UInt32 address;
    UInt32 locations[0];
};
extern void fn_0046D800(Object* obj, UInt32 register_id);

extern SInt32 fn_0046D3F0(void);
extern void init_registers(void);
extern void assign_register_to_variable(Object* obj, SInt8 cls);
extern void retain_register(Object* obj, SInt8 cls, SInt16 reg);
extern SInt32 available_registers(SInt8 cls);
extern UInt32 volatile_registers(SInt8 cls);
extern SInt32 obtain_nonvolatile_register(SInt8 cls);
extern void open_temp_registers(void);
extern void check_temp_registers(void);
extern void close_temp_registers(void);

extern void fn_0046DD50(Object* object, void* data, void* refs, UInt32 size);
extern void fn_0046E410(Object* obj, void* context, void* buffer, void* unknown4);

extern void fn_0046D380(void);
extern SInt16 fn_0046C840(void);

extern void fn_0046D9E0(Object* object, void* data, void* bytes, SInt32 size, void* refs);

extern void fn_0046D7F0(void);

extern char* fn_0046C430(char* input);

extern SInt32 fn_0046C560(void);
extern SInt32 fn_0046C6C0(UInt32 unused, UInt8 line, Boolean consume_newlines);

#ifdef __cplusplus
}
#endif

#endif
