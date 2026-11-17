#include "compiler/MIPSError.h"
#include "compiler/PCodeUtilities.h"
#include "compiler/PCodeInfo.h"
#include "compiler/mips.h"
#include "compiler/RegisterInfo.h"
#include "compiler/CError.h"
#include "compiler/Alias.h"
#include "compiler/Registers.h"
#include "compiler/CMachine.h"
#include "compiler/CodeGen.h"
#include "compiler/objects.h"
#include "compiler/InterferenceGraph.h"
#include "compiler/CParser.h"

SInt8 GetRegisterClassName(SInt8 regclass) {
    switch (regclass) {
        case 5:
        case 6:
            return 'v';
        case 0:
            return 'r';
        case 1:
            return 'f';
        default:
            CError_Internal("RegisterInfo.c", 45);
            return '?';
    }
}

void init_target_registers(void) {
    int specialreg = -1;
    int i;
    target_register_class_names_6356C8[0] = register_name_gpr_5E6078;
    target_register_name_patterns_63663C[0] = register_pattern_gpr_5E607C;
    target_register_class_names_6356C8[1] = register_name_fpr_5E6081;
    target_register_name_patterns_63663C[1] = register_pattern_fpr_5E6085;
    target_register_class_names_6356C8[2] = register_name_special_5E608A;
    target_register_name_patterns_63663C[2] = register_pattern_special_5E6092;
    target_register_class_names_6356C8[3] = register_name_coproc2_i_5E6097;
    target_register_name_patterns_63663C[3] = register_pattern_coproc2_i_5E60A1;
    target_register_class_names_6356C8[4] = register_name_coproc2_ii_5E60A7;
    target_register_name_patterns_63663C[4] = register_pattern_coproc2_i_5E60A1;
    target_register_class_names_6356C8[5] = register_name_coproc2_f_5E60B2;
    target_register_name_patterns_63663C[5] = register_pattern_coproc2_f_5E60BC;
    target_register_class_names_6356C8[6] = register_name_coproc2_special_5E60C2;
    target_register_name_patterns_63663C[6] = register_pattern_coproc2_special_5E60D2;
    n_real_registers[0] = 32;
    n_real_registers[1] = 32;
    n_real_registers[2] = 32;
    n_real_registers[3] = 16;
    n_real_registers[4] = 32;
    n_real_registers[5] = 32;
    n_real_registers[6] = 32;
    if (copts.unknown47) {
        specialreg = copts.unknown48;
        if (specialreg < 0 || specialreg > 25) {
            specialreg = 25;
        }
    }
    scheduler_register_colors[0][0] = 2;
    scheduler_register_colors[0][1] = 2;
    scheduler_register_colors[0][26] = 2;
    scheduler_register_colors[0][27] = 2;
    scheduler_register_colors[0][28] = 2;
    scheduler_register_colors[0][29] = 2;
    scheduler_register_colors[0][31] = 2;
    if (codegen_flag_63738A) {
        scheduler_register_colors[0][30] = 2;
        stack_pointer_register = 30;
    } else {
        stack_pointer_register = 29;
    }
    if (copts.unknown08) {
        scheduler_register_colors[0][23] = 2;
    }
    scheduler_register_colors[3][0] = 2;
    scheduler_register_colors[4][0] = 2;
    scheduler_register_colors[5][0] = 2;
    for (i = 1; i < n_real_registers[3]; ++i) {
        scheduler_register_colors[3][i] = 0;
    }
    reg_counter_gpr_635A6C = 0;
    reg_counter_fpr_636028 = 0;
    for (i = 1; i < n_real_registers[4]; ++i) {
        scheduler_register_colors[4][i] = 0;
    }
    n_nonvolatile_registers[4] = 0;
    reg_counter_63602C = 0;
    for (i = 1; i < n_real_registers[5]; ++i) {
        scheduler_register_colors[5][i] = 0;
    }
    n_nonvolatile_registers[5] = 0;
    reg_counter_636030 = 0;
    for (i = 0; i < n_real_registers[6]; ++i) {
        scheduler_register_colors[6][i] = 2;
    }
    n_nonvolatile_registers[6] = 0;
    reg_counter_636034 = 0;
    for (i = 0; i < n_real_registers[1]; ++i) {
        scheduler_register_colors[1][i] = 0;
        if (!copts.floatsp32 && (i & 1)) {
            scheduler_register_colors[1][i] = 2;
        }
    }
    for (i = 0; i < n_real_registers[2]; ++i) {
        scheduler_register_colors[2][i] = 2;
    }
    n_nonvolatile_registers[0] = 0;
    for (i = 16; i <= 22; ++i) {
        nonvolatile_registers[0][n_nonvolatile_registers[0]++] = i;
    }
    if (!copts.unknown08) {
        nonvolatile_registers[0][n_nonvolatile_registers[0]++] = 23;
    }
    if (!codegen_flag_63738A) {
        nonvolatile_registers[0][n_nonvolatile_registers[0]++] = 30;
    }
    assignable_registers[0] = n_nonvolatile_registers[0];
    n_nonvolatile_registers[1] = 0;
    for (i = 20; i <= 31; ++i) {
        if (!scheduler_register_colors[1][i]) {
            nonvolatile_registers[1][n_nonvolatile_registers[1]++] = i;
        }
    }
    assignable_registers[1] = n_nonvolatile_registers[1];
    if (copts.unknown47) {
        n_nonvolatile_registers[5] = 0;
        register_reservation_limits_636150[5] = 17;
        for (i = 9; i < n_real_registers[5]; ++i) {
            if (!scheduler_register_colors[5][i]) {
                nonvolatile_registers[5][n_nonvolatile_registers[5]++] = i;
            }
        }
        reg_counter_636030 = n_nonvolatile_registers[5];
        scheduler_register_colors[5][1] = 0;
        scheduler_register_colors[5][2] = 0;
        scheduler_register_colors[5][3] = 0;
        scheduler_register_colors[5][4] = 0;
        n_nonvolatile_registers[6] = 0;
        register_reservation_limits_636150[6] = n_real_registers[6] * 30 / 100;
        for (i = 0; i < register_reservation_limits_636150[6]; ++i) {
            if (!scheduler_register_colors[6][i]) {
                nonvolatile_registers[6][n_nonvolatile_registers[6]++] = i;
            }
        }
        reg_counter_636034 = n_nonvolatile_registers[6];
        register_reservation_limits_636150[0] =
            n_nonvolatile_registers[0] ? n_nonvolatile_registers[0] * 30 / 100 : 0;
        register_reservation_limits_636150[1] =
            n_nonvolatile_registers[1] ? n_nonvolatile_registers[1] * 30 / 100 : 0;
        register_reservation_limits_636150[3] = reg_counter_gpr_635A6C;
        register_reservation_limits_636150[4] = n_nonvolatile_registers[4];
    }
    {
        SInt8 regclass;
        for (regclass = 0; regclass < 7; ++regclass) {
            int reg;
            n_scratch_registers[regclass] = 0;
            for (reg = 0; reg < n_real_registers[regclass]; ++reg) {
                int index;
                if (scheduler_register_colors[regclass][reg]) {
                    continue;
                }
                for (index = 0; index < (SInt32)n_nonvolatile_registers[regclass]; ++index) {
                    if (reg == nonvolatile_registers[regclass][index]) {
                        break;
                    }
                }
                if (index == (SInt32)n_nonvolatile_registers[regclass]) {
                    scratch_registers[regclass][n_scratch_registers[regclass]++] = reg;
                }
            }
        }
    }
    if (copts.unknown47) {
        RegInfo* record;
        int extra = 0;
        for (record = reg_info_list; record; record = record->next) {
            int index;
            int changed;
            if ((record->reginfo->rclass != 5 || record->reginfo->reg) &&
                (record->reginfo->rclass != 0 || record->reginfo->reg)) {
                if (!scheduler_register_colors[record->reginfo->rclass][record->reginfo->reg]) {
                    changed = FALSE;
                    if (record->reginfo->rclass == 5) {
                        if (specialreg < record->reginfo->reg) {
                            MIPSError_Error(27);
                        } else {
                            int counted = FALSE;
                            scheduler_register_colors[record->reginfo->rclass]
                                                     [record->reginfo->reg] = 2;
                            changed = TRUE;
                            for (index = 0;
                                 index < (SInt32)n_nonvolatile_registers[record->reginfo->rclass];
                                 ++index) {
                                if (record->reginfo->reg ==
                                    nonvolatile_registers[record->reginfo->rclass][index]) {
                                    starting_available_registers[record->reginfo->rclass]++;
                                    assignable_registers[record->reginfo->rclass]--;
                                    counted = TRUE;
                                    if (assignable_registers[record->reginfo->rclass] < 0) {
                                        assignable_registers[record->reginfo->rclass] = 0;
                                    }
                                    break;
                                }
                            }
                            if (!counted) {
                                extra++;
                            }
                        }
                    } else {
                        for (index = 0;
                             index < (SInt32)n_nonvolatile_registers[record->reginfo->rclass];
                             ++index) {
                            if (record->reginfo->reg ==
                                nonvolatile_registers[record->reginfo->rclass][index]) {
                                if (starting_available_registers[record->reginfo->rclass] <
                                    register_reservation_limits_636150[record->reginfo->rclass]) {
                                    scheduler_register_colors[record->reginfo->rclass]
                                                             [record->reginfo->reg] = 2;
                                    starting_available_registers[record->reginfo->rclass]++;
                                    assignable_registers[record->reginfo->rclass]--;
                                    changed = TRUE;
                                    if (assignable_registers[record->reginfo->rclass] < 0) {
                                        assignable_registers[record->reginfo->rclass] = 0;
                                    }
                                } else {
                                    char regname[2];
                                    regname[0] = GetRegisterClassName(record->reginfo->rclass);
                                    regname[1] = 0;
                                    MIPSError_Error(
                                        29,
                                        regname,
                                        register_reservation_limits_636150[record->reginfo->rclass]
                                    );
                                }
                                break;
                            }
                        }
                    }
                    if (!changed) {
                        MIPSError_Error(25);
                    }
                } else {
                    char regname[2];
                    regname[0] = GetRegisterClassName(record->reginfo->rclass);
                    regname[1] = 0;
                    MIPSError_Error(24, regname, record->reginfo->reg);
                }
            }
        }
        if (extra > 6) {
            MIPSError_Warning(26);
        }
    }
}

PCode* fn_004729B0(SInt32 reg, IGNode* node) {
    Object* operand = node->spillTemporary;
    Type* type = operand->type;
    UInt16 flags = node->flags;
    PCode* instr;
    SInt32 displacement = 0;
    int isFloat;
    UInt16 opcode;
    if (flags & 0x10) {
        displacement = copts.littleendian ? 4 : 0;
    } else if (flags & 0x20) {
        displacement = copts.littleendian ? 0 : 4;
    }
    opcode = fn_0048E9D0(type, &isFloat);
    if (opcode == 0x467 && coloring_class == 5) {
        opcode = 0x468;
    }
    instr = make_instruction(opcode, 3);
    instr->args[0].kind = PCOp_REGISTER;
    instr->args[0].arg = coloring_class;
    instr->args[0].data.reg.reg = reg;
    instr->args[0].effect = 2;
    if (instr->op == 0x467) {
        instr->args[0].effect |= 0x2000;
    }
    if (copts.unknown0C >= 2 && (copts.fp_library & 2)) {
        opcode = 0x455;
    } else {
        opcode = 0x452;
    }
    if (instr->op == opcode) {
        instr->args[0].effect |= 0x8000;
        if (copts.unknown1A) {
            instr->args[0].effect |= 0x20;
        }
    }
    instr->args[1].kind = PCOp_MEMORY;
    instr->args[1].data.mem.obj = node->spillTemporary;
    instr->args[1].data.mem.offset = node->spillTemporary->u.unk30 + displacement;
    instr->alias = make_alias(
        instr->args[1].data.mem.obj,
        instr->args[1].data.mem.offset,
        nbytes_loaded_or_stored_by(instr->op)
    );
    instr->args[2].kind = PCOp_REGISTER;
    instr->args[2].arg = 0;
    instr->args[2].data.reg.reg = stack_pointer_register;
    instr->args[2].effect = 1;
    return instr;
}

PCode* fn_004728B0(SInt32 reg, IGNode* node) {
    Object* operand = node->spillTemporary;
    Type* type = operand->type;
    UInt16 flags = node->flags;
    PCode* instr;
    SInt32 displacement = 0;
    UInt16 opcode;
    if (flags & 0x10) {
        displacement = copts.littleendian ? 4 : 0;
    } else if (flags & 0x20) {
        displacement = copts.littleendian ? 0 : 4;
    }
    opcode = fn_0048ECB0(type);
    if (coloring_class == 5 && opcode == 0x489) {
        opcode = 0x48A;
    }
    instr = make_instruction(opcode, 3);
    instr->args[0].kind = PCOp_REGISTER;
    instr->args[0].arg = coloring_class;
    instr->args[0].data.reg.reg = reg;
    instr->args[0].effect = 1;
    if (instr->op == 0x489) {
        instr->args[0].effect |= 0x2000;
    }
    instr->args[1].kind = PCOp_MEMORY;
    instr->args[1].data.mem.obj = operand;
    instr->args[1].data.mem.offset = operand->u.unk30 + displacement;
    instr->alias = make_alias(
        instr->args[1].data.mem.obj,
        instr->args[1].data.mem.offset,
        nbytes_loaded_or_stored_by(instr->op)
    );
    instr->args[2].kind = PCOp_REGISTER;
    instr->args[2].arg = 0;
    instr->args[2].data.reg.reg = stack_pointer_register;
    instr->args[2].effect = 1;
    return instr;
}

Type* fn_00472850(IGNode* node) {
    UInt32 allow;
    if (!coloring_class) {
        return (Type*)&stint128;
    }
    allow = copts.enable_vu0_registers;
    if ((UInt8)allow) {
        if (coloring_class == 5) {
            return (Type*)&stint128;
        }
    }
    if (!(UInt8)allow && coloring_class == 5) {
        return (Type*)&stint128;
    }
    if (copts.floatsp32) {
        return (Type*)&stfloat;
    }
    return (Type*)&stlongdouble;
}

Boolean fn_00472800(SInt32 first, SInt32 second) {
    if (!coloring_class) {
        return !first || !second;
    }
    if (!copts.enable_vu0_registers || coloring_class != 5) {
        return FALSE;
    }
    return !first || !second;
}

SInt32 fn_004727F0(void) {
    return cmach_unk6355E4;
}

Boolean fn_004727A0(Object* object) {
    struct ObjRegInfo* info;
    UInt8 mode;
    if (copts.unknown47 && object && (info = object->u.data.reginfo) && object->datatype == DDATA) {
        if ((UInt16)info->reg || (mode = (UInt8)info->rclass) == 5 || (info->flags && !mode)) {
            return TRUE;
        }
    }
    return FALSE;
}

void* Registers_GetVarInfo(Object* object) {
    switch (object->datatype) {
        case DDATA:
            if (!object->u.data.reginfo) {
                CError_Internal("RegisterInfo.c", 821);
            }
            return object->u.data.reginfo;
        case DLOCAL:
            if (!object->u.var.info) {
                CError_Internal("RegisterInfo.c", 834);
            }
            return object->u.var.info;
        default:
            CError_Internal("RegisterInfo.c", 839);
            return NULL;
    }
}

char register_name_gpr_5E6078[] = "GPR";
char register_pattern_gpr_5E607C[] = "r%ld";
char register_name_fpr_5E6081[] = "FPR";
char register_pattern_fpr_5E6085[] = "f%ld";
char register_name_special_5E608A[] = "SPECIAL";
char register_pattern_special_5E6092[] = "#%ld";
char register_name_coproc2_i_5E6097[] = "COPROC2_i";
char register_pattern_coproc2_i_5E60A1[] = "vi%ld";
char register_name_coproc2_ii_5E60A7[] = "COPROC2_ii";
char register_name_coproc2_f_5E60B2[] = "COPROC2_f";
char register_pattern_coproc2_f_5E60BC[] = "vf%ld";
char register_name_coproc2_special_5E60C2[] = "COPROC2_special";
char register_pattern_coproc2_special_5E60D2[] = "COPROC2_%ld";
