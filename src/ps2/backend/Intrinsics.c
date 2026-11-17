#include "compiler/common.h"
#include "compiler/Intrinsics.h"
#include "compiler/CMangler.h"
#include "compiler/PCodeInfo.h"
#include "compiler/PCode.h"
#include "compiler/CompilerTools.h"
#include "compiler/CScope.h"
#include "compiler/InstrSelection.h"
#include "compiler/Operands.h"
#include "compiler/VReg.h"
#include "compiler/enode.h"
#include "compiler/CError.h"
#include <string.h>
#include "compiler/CParser.h"
#include "compiler/objects.h"
#include "compiler/CMachine.h"
#include "compiler/CodeGen.h"
#include "compiler/CFunc.h"
#include "compiler/mips.h"
#include "compiler/StructMoves.h"
#include "compiler/MIPSError.h"
#include "compiler/Registers.h"
#include "compiler/RegisterInfo.h"
#include "compiler/CABI.h"
#include "compiler/CDecl.h"

void fn_00466380(ENode* node, SInt16 number) {
    ENode* target = *(ENode* volatile*)&node->data.funccall.funcref;
    Object* value = intrinsics_601CD8[number];
    target->data.objref = value;
}

int is_intrinsic_function_call(ENode* funccall) {
    ENode* funcref = funccall->data.funccall.funcref;
    return ENODE_IS(funcref, EOBJREF) &&
        funcref->data.objref->datatype == DFUNC &&
        (TYPE_FUNC(funcref->data.objref->type)->flags & FUNC_INTRINSIC);
}

ObjRegInfo* fn_004662F0(Operand* op, SInt8 klass, SInt32 reg) {
    Object* object;
    ObjRegInfo* info;

    if (!op) {
        return NULL;
    }
    object = op->object;
    if (!object) {
        return NULL;
    }
    if (fn_004727A0(object)) {
        info = Registers_GetVarInfo(object);
        if (!info) {
            return NULL;
        }
        if (klass == -1) {
            return info;
        }
        if (info->rclass != klass) {
            return NULL;
        }
        if (reg >= 0 && info->reg != reg) {
            return NULL;
        }
        return info;
    }
    return NULL;
}

UInt8 fn_00459690(Boolean exporting) {
    SInt16 i;
    if (exporting) {
        for (i = 0; (UInt16)i < 0x3E5; i++) {
            char* name = (char*)opcode_info[(UInt16)i].unk20;
            if (name) {
                HashNameNode* hash = GetHashNameNodeExport(name);
                intrinsics_601CD8[opcode_info[(UInt16)i].unk24]->name = hash;
            }
        }
    }
    return 1;
}

void fn_00463AF0(ENodeList* args, SInt16 opcode, Operand* result) {
    IntrinsicOpRecord values[10];
    UInt32 count = -1;
    UInt8 flags = 0;
    UInt8 mode = 0;
    int i, target, fpreg;
    SInt16 physreg;
    ENode* node;
    ENodeList* cursor;
    ObjRegInfo* info;
    Operand* op;
    Operand temp;
    Operand scratch1, scratch2, scratch3, scratch4;
    Operand scratch5, scratch6, scratch7, scratch8;
    Operand scratch9, scratch10, scratch11, scratch12;
    Operand scratch13, scratch14, scratch15, scratch16;
    Operand scratch17, scratch18, scratch19, scratch20, scratch21, scratch22;

    memset(values, 0, sizeof(values));
    for (i = 0; i < 10; i++) {
        values[i].category = 0;
    }
    switch (opcode) {
        case 0x269:
        case 0x27d:
        case 0x27e:
        case 0x27f:
        case 0x280:
        case 0x28e:
        case 0x28f:
        case 0x290:
        case 0x291:
        case 0x2af:
        case 0x2b0:
            count = 3;
            values[0].category = 1;
            values[1].category = 8;
            values[2].category = 1;
            mode = 2;
            break;
        case 0x26a:
        case 0x2a2:
        case 0x2a9:
        case 0x2c0:
        case 0x2d9:
            count = 4;
            values[0].category = 1;
            values[1].category = 8;
            values[2].category = 1;
            values[3].category = 1;
            mode = 4;
            break;
        case 0x26c:
        case 0x2c2:
        case 0x2db:
            count = 4;
            values[0].category = 1;
            values[1].category = 8;
            values[2].category = 1;
            values[3].category = 6;
            mode = 5;
            break;
        case 0x26e:
        case 0x2a5:
        case 0x2ac:
        case 0x2c4:
        case 0x2dd:
            count = 5;
            values[0].category = 1;
            values[1].category = 8;
            values[2].category = 1;
            values[3].category = 1;
            values[4].category = 10;
            mode = 6;
            break;
        case 0x27b:
            count = 2;
            values[0].category = 1;
            values[1].category = 1;
            mode = 1;
            break;
        case 0x27c:
        case 0x2d4:
            count = 5;
            values[0].category = 7;
            values[1].category = 1;
            values[2].category = 8;
            values[3].category = 1;
            values[4].category = 9;
            mode = 0xe;
            break;
        case 0x294:
        case 0x2b1:
            count = 5;
            values[0].category = 1;
            values[1].category = 8;
            values[2].category = 2;
            values[3].category = 1;
            values[4].category = 1;
            mode = 8;
            break;
        case 0x296:
        case 0x2b3:
            count = 5;
            values[0].category = 1;
            values[1].category = 8;
            values[2].category = 2;
            values[3].category = 1;
            values[4].category = 6;
            mode = 9;
            break;
        case 0x298:
        case 0x2b5:
            count = 6;
            values[0].category = 1;
            values[1].category = 8;
            values[2].category = 2;
            values[3].category = 1;
            values[4].category = 1;
            values[5].category = 10;
            mode = 10;
            break;
        case 0x2a3:
        case 0x2aa:
            count = 4;
            values[0].category = 1;
            values[1].category = 8;
            values[2].category = 1;
            values[3].category = 3;
            mode = 4;
            break;
        case 0x2cf:
            count = 3;
            values[0].category = 2;
            values[1].category = 1;
            values[2].category = 1;
            mode = 0x10;
            break;
        case 0x2d0:
            count = 4;
            values[0].category = 1;
            values[1].category = 2;
            values[2].category = 1;
            values[3].category = 1;
            mode = 0x11;
            break;
        case 0x2d1:
        case 0x2d3:
            count = 3;
            values[0].category = 1;
            values[1].category = 8;
            values[2].category = 4;
            mode = 3;
            break;
        case 0x2d2:
        case 0x2d5:
            count = 3;
            values[0].category = 4;
            values[1].category = 1;
            values[2].category = 8;
            mode = 0xf;
            break;
        case 0x2d8:
            count = 3;
            values[0].category = 7;
            values[1].category = 1;
            values[2].category = 8;
            mode = 0xd;
            break;
        case 0x2e7:
            count = 1;
            values[0].category = 7;
            mode = 0xc;
            break;
        default:
            CError_Internal("Intrinsics.c", 0x5ae);
    }
    cursor = args;
    i = 0;
    if (count) {
        do {
            if (!cursor) {
                CError_Internal("Intrinsics.c", 0x5b5);
            }
            node = cursor->node;
            values[i].node = node;
            op = &values[i].operand;
            switch (values[i].category) {
                case 1:
                    gen_func_table[node->type](node, 0, op);
                    info = fn_004662F0(op, -1, -1);
                    target = used_virtual_registers[5];
                    if (!info) {
                        if (op->kind == 5) {
                            values[i].reg = op->reg;
                        } else {
                            if (op->kind) {
                                Coerce_to_register(op, (Type*)&stint128, 0);
                            }
                            target = used_virtual_registers[5];
                            if (op->kind == 5) {
                                values[i].reg = op->reg;
                            } else {
                                if (op->kind) {
                                    CError_Internal("Intrinsics.c", 0x5da);
                                }
                                used_virtual_registers[5]++;
                                values[i].reg = target;
                                emitpcode(0x266, op->reg, values[i].reg);
                            }
                        }
                    } else if (info->rclass == 5) {
                        values[i].reg = info->reg;
                    } else if (info->rclass == 6 && info->reg == 3) {
                        values[i].reg = used_virtual_registers[5]++;
                        emitpcode(0x266, values[i].reg, 0);
                        emitpcode(0x294, 0xf, values[i].reg, values[i].reg, values[i].reg);
                    } else {
                        if (info->rclass) {
                            CError_Internal("Intrinsics.c", 0x5ca);
                        }
                        used_virtual_registers[5]++;
                        values[i].reg = target;
                        emitpcode(0x266, op->reg, values[i].reg);
                    }
                    break;
                case 2:
                    values[i].reg = 3;
                    gen_func_table[node->type](node, 0, op);
                    info = fn_004662F0(op, 6, 3);
                    if (!info) {
                        if (op->kind != 5 && op->kind != 0) {
                            Coerce_to_register(op, (Type*)&stint128, 0);
                        }
                        if (op->kind == 5) {
                            emitpcode(0x275, 0xf, op->reg, 0);
                        } else if (op->kind == 0) {
                            target = used_virtual_registers[5]++;
                            emitpcode(0x266, op->reg, target);
                            emitpcode(0x275, 0xf, target, 0);
                        } else {
                            CError_Internal("Intrinsics.c", 0x5f2);
                        }
                    }
                    break;
                case 3:
                case 4:
                case 5:
                case 6:
                    if (values[i].category != 6) {
                        values[i].reg = values[i].category == 3 ? 1
                            : values[i].category == 4           ? 4
                                                                : 2;
                    }
                    gen_func_table[node->type](node, 0, op);
                    info = fn_004662F0(op, 6, -1);
                    fpreg = used_virtual_registers[0];
                    if (!info) {
                        if (op->kind) {
                            Coerce_to_register(op, (Type*)&stfloat, 0);
                        }
                        fpreg = used_virtual_registers[0];
                        if (op->kind == 0) {
                            emitpcode(
                                0x1d0,
                                op->reg,
                                values[i].category == 3       ? 0x15
                                    : values[i].category == 4 ? 0x14
                                                              : 0x16
                            );
                        } else {
                            if (op->kind != 1) {
                                CError_Internal(
                                    "Intrinsics.c",
                                    values[i].category == 3       ? 0x61d
                                        : values[i].category == 4 ? 0x647
                                        : values[i].category == 5 ? 0x674
                                                                  : 0x6ab
                                );
                            }
                            used_virtual_registers[0]++;
                            emitpcode(0xa5, fpreg, op->reg);
                            emitpcode(
                                0x1d0,
                                fpreg,
                                values[i].category == 3       ? 0x15
                                    : values[i].category == 4 ? 0x14
                                                              : 0x16
                            );
                        }
                        if (values[i].category >= 5) {
                            emitpcode(0x2e7);
                        }
                    } else {
                        physreg = info->reg;
                        if (values[i].category == 6 && physreg == 1) {
                            values[i].reg = op->reg;
                            switch (opcode) {
                                case 0x26c:
                                    opcode = 0x26b;
                                    break;
                                case 0x296:
                                    opcode = 0x295;
                                    break;
                                case 0x2b3:
                                    opcode = 0x2b2;
                                    break;
                                case 0x2c2:
                                    opcode = 0x2c1;
                                    break;
                                case 0x2db:
                                    opcode = 0x2da;
                                    break;
                                default:
                                    CError_Internal("Intrinsics.c", 0x68d);
                            }
                        } else if (physreg ==
                                   (values[i].category == 3       ? 1
                                        : values[i].category == 4 ? 4
                                                                  : 2)) {
                            if (values[i].category == 6) {
                                values[i].reg = 2;
                            }
                        } else {
                            if (physreg != 1 && physreg != 2 && physreg != 4) {
                                CError_Internal(
                                    "Intrinsics.c",
                                    values[i].category == 3       ? 0x60d
                                        : values[i].category == 4 ? 0x637
                                        : values[i].category == 5 ? 0x662
                                                                  : 0x699
                                );
                            }
                            used_virtual_registers[0]++;
                            if (physreg == 2) {
                                emitpcode(0x2e7);
                            }
                            emitpcode(
                                0x1cf,
                                fpreg,
                                physreg == 1       ? 0x15
                                    : physreg == 4 ? 0x14
                                                   : 0x16
                            );
                            emitpcode(
                                0x1d0,
                                fpreg,
                                values[i].category == 3       ? 0x15
                                    : values[i].category == 4 ? 0x14
                                                              : 0x16
                            );
                            if (values[i].category >= 5) {
                                emitpcode(0x2e7);
                            }
                            if (values[i].category == 6) {
                                values[i].reg = 2;
                            }
                        }
                    }
                    break;
                case 7:
                    values[i].reg = -1;
                    break;
                case 8:
                    flags = (UInt8)node->data.intval.lo;
                    values[i].reg = flags;
                    break;
                case 9:
                    values[i].reg = (UInt8)node->data.intval.lo;
                    break;
                case 10:
                    values[i].reg = (UInt8)node->data.intval.lo;
                    switch (values[i].reg) {
                        case 4:
                            switch (opcode) {
                                case 0x26e:
                                    opcode = 0x26f;
                                    break;
                                case 0x298:
                                    opcode = 0x299;
                                    break;
                                case 0x2a5:
                                    opcode = 0x2a6;
                                    break;
                                case 0x2ac:
                                    opcode = 0x2ad;
                                    break;
                                case 0x2b5:
                                    opcode = 0x2b6;
                                    break;
                                case 0x2c4:
                                    opcode = 0x2c5;
                                    break;
                                case 0x2dd:
                                    opcode = 0x2de;
                                    break;
                                default:
                                    CError_Internal("Intrinsics.c", 0x6cd);
                            }
                            break;
                        case 2:
                            switch (opcode) {
                                case 0x26e:
                                    opcode = 0x270;
                                    break;
                                case 0x298:
                                    opcode = 0x29a;
                                    break;
                                case 0x2a5:
                                    opcode = 0x2a7;
                                    break;
                                case 0x2ac:
                                    opcode = 0x2ae;
                                    break;
                                case 0x2b5:
                                    opcode = 0x2b7;
                                    break;
                                case 0x2c4:
                                    opcode = 0x2c6;
                                    break;
                                case 0x2dd:
                                    opcode = 0x2df;
                                    break;
                                default:
                                    CError_Internal("Intrinsics.c", 0x6db);
                            }
                            break;
                        case 1:
                            switch (opcode) {
                                case 0x26e:
                                    opcode = 0x26d;
                                    break;
                                case 0x298:
                                    opcode = 0x297;
                                    break;
                                case 0x2a5:
                                    opcode = 0x2a4;
                                    break;
                                case 0x2ac:
                                    opcode = 0x2ab;
                                    break;
                                case 0x2b5:
                                    opcode = 0x2b4;
                                    break;
                                case 0x2c4:
                                    opcode = 0x2c3;
                                    break;
                                case 0x2dd:
                                    opcode = 0x2dc;
                                    break;
                                default:
                                    CError_Internal("Intrinsics.c", 0x6e9);
                            }
                            break;
                        case 8:
                            break;
                        default:
                            CError_Error(78);
                            break;
                    }
                    break;
                default:
                    CError_Internal("Intrinsics.c", 0x6f4);
            }
            cursor = cursor->next;
        } while (++i < (int)count);
    }
    if (cursor) {
        CError_Internal("Intrinsics.c", 0x6fc);
    }
    switch (mode) {
        case 1:
            emitpcode(opcode, values[0].reg, values[1].reg);
            break;
        case 2:
        case 3:
            emitpcode(opcode, (UInt16)flags, values[0].reg, values[2].reg);
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            emitpcode(opcode, (UInt16)flags, (SInt16)values[0].reg, values[2].reg, values[3].reg);
            break;
        case 8:
        case 9:
        case 10:
        case 11:
            emitpcode(opcode, (UInt16)flags, (SInt16)values[0].reg, values[3].reg, values[4].reg);
            break;
        case 12:
            break;
        case 13:
            emitpcode(opcode, (UInt16)flags, values[1].reg);
            break;
        case 14:
            emitpcode(opcode, (UInt16)flags, (SInt16)values[1].reg, 0, values[3].reg);
            break;
        case 15:
            emitpcode(opcode, (UInt16)flags, values[2].reg);
            break;
        case 16:
            emitpcode(opcode, values[1].reg, values[2].reg);
            break;
        case 17:
            emitpcode(opcode, (SInt16)values[0].reg, values[2].reg, values[3].reg);
            break;
        default:
            CError_Internal("Intrinsics.c", 0x732);
    }
    switch (mode) {
        case 1:
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 17:
            node = values[0].node;
            gen_func_table[node->type](node, 0, &values[0].operand);
            info = fn_004662F0(&values[0].operand, -1, -1);
            target = used_virtual_registers[0];
            if (!info) {
                if (values[0].operand.kind == 0) {
                    emitpcode(0x1cf, values[0].operand.reg, 0x16);
                    scratch1 = make_register_operand(values[0].operand.reg, 0);
                    *result = scratch1;
                } else {
                    used_virtual_registers[0]++;
                    emitpcode(0x264, target, values[0].reg);
                    scratch2 = make_register_operand(target, 0);
                    *result = scratch2;
                    fn_004B1440(*result, &values[0].operand, (Type*)&stint128);
                }
            } else if (info->rclass == 5) {
                scratch3 = make_register_operand(values[0].reg, 5);
                *result = scratch3;
            } else if (info->rclass == 0) {
                emitpcode(0x264, values[0].operand.reg, values[0].reg);
                scratch4 = make_register_operand(values[0].operand.reg, 0);
                *result = scratch4;
            } else {
                if (info->rclass != 6 || info->reg != 3) {
                    CError_Internal("Intrinsics.c", 0x7ae);
                }
                emitpcode(0x298, 0xf, (SInt16)values[0].reg, 0, 0);
                scratch5 = make_register_operand(values[0].reg, 5);
                *result = scratch5;
            }
            break;
        case 12:
        case 13:
        case 14:
            emitpcode(0x2e7);
            node = values[0].node;
            gen_func_table[node->type](node, 0, &values[0].operand);
            info = fn_004662F0(&values[0].operand, -1, -1);
            fpreg = used_virtual_registers[1];
            target = used_virtual_registers[0];
            if (!info) {
                if (values[0].operand.kind == 0) {
                    used_virtual_registers[1]++;
                    emitpcode(0x1cf, values[0].operand.reg, 0x16);
                    emitpcode(0xad, values[0].operand.reg, fpreg);
                    scratch6 = make_register_operand(fpreg, 1);
                    *result = scratch6;
                } else if (values[0].operand.kind == 1) {
                    used_virtual_registers[0]++;
                    emitpcode(0x1cf, target, 0x16);
                    emitpcode(0xad, target, values[0].operand.reg);
                    scratch7 = make_register_operand(values[0].operand.reg, 1);
                    *result = scratch7;
                } else {
                    used_virtual_registers[0]++;
                    used_virtual_registers[1]++;
                    emitpcode(0x1cf, target, 0x16);
                    emitpcode(0xad, target, fpreg);
                    scratch8 = make_register_operand(fpreg, 1);
                    *result = scratch8;
                    fn_004B1440(*result, &values[0].operand, (Type*)&stfloat);
                }
            } else if (info->rclass == 6) {
                physreg = info->reg;
                if (physreg == 2) {
                    used_virtual_registers[0]++;
                    used_virtual_registers[1]++;
                    emitpcode(0x1cf, target, 0x16);
                    emitpcode(0xad, target, fpreg);
                    scratch9 = make_register_operand(fpreg, 1);
                    *result = scratch9;
                } else {
                    if (physreg != 1) {
                        CError_Internal("Intrinsics.c", 0x75d);
                    }
                    used_virtual_registers[0]++;
                    used_virtual_registers[1]++;
                    emitpcode(0x1cf, target, 0x16);
                    emitpcode(0x1d0, target, 0x15);
                    emitpcode(0xad, target, fpreg);
                    scratch22 = make_register_operand(fpreg, 1);
                    *result = scratch22;
                }
            } else if (info->rclass == 0) {
                used_virtual_registers[1]++;
                emitpcode(0x1cf, info->reg, 0x16);
                emitpcode(0xad, info->reg, fpreg);
                scratch10 = make_register_operand(fpreg, 1);
                *result = scratch10;
            } else if (info->rclass == 1) {
                used_virtual_registers[0]++;
                emitpcode(0x1cf, target, 0x16);
                emitpcode(0xad, target, info->reg);
                scratch11 = make_register_operand(info->reg, 1);
                *result = scratch11;
            } else {
                CError_Internal("Intrinsics.c", 0x76f);
            }
            break;
        case 15:
            node = values[0].node;
            gen_func_table[node->type](node, 0, &values[0].operand);
            info = fn_004662F0(&values[0].operand, -1, -1);
            fpreg = used_virtual_registers[RegClass_FPR];
            target = used_virtual_registers[RegClass_GPR];
            if (!info) {
                if (values[0].operand.kind == 0) {
                    used_virtual_registers[RegClass_FPR]++;
                    emitpcode(0x1cf, values[0].operand.reg, 0x16);
                    emitpcode(0xad, values[0].operand.reg, fpreg);
                    scratch16 = make_register_operand(fpreg, 1);
                    *result = scratch16;
                } else if (values[0].operand.kind == 1) {
                    used_virtual_registers[RegClass_GPR]++;
                    emitpcode(0x1cf, target, 0x16);
                    emitpcode(0xad, target, values[0].operand.reg);
                    scratch17 = make_register_operand(values[0].operand.reg, 1);
                    *result = scratch17;
                } else {
                    used_virtual_registers[RegClass_GPR]++;
                    used_virtual_registers[RegClass_FPR]++;
                    emitpcode(0x1cf, target, 0x14);
                    emitpcode(0xad, target, fpreg);
                    scratch18 = make_register_operand(fpreg, 1);
                    *result = scratch18;
                    fn_004B1440(*result, &values[0].operand, TYPE(&stfloat));
                }
            } else if (info->rclass == RegClass_GPR) {
                emitpcode(0x1cf, values[0].operand.reg, 0x14);
                scratch19 = make_register_operand(values[0].operand.reg, 0);
                *result = scratch19;
            } else if (info->rclass == RegClass_FPR) {
                used_virtual_registers[RegClass_GPR]++;
                emitpcode(0x1cf, target, 0x14);
                emitpcode(0xad, target, values[0].operand.reg);
                scratch20 = make_register_operand(values[0].operand.reg, 1);
                *result = scratch20;
            } else {
                if (info->rclass != 6 || info->reg != 4) {
                    CError_Internal("Intrinsics.c", 0x7da);
                }
                used_virtual_registers[RegClass_GPR]++;
                used_virtual_registers[RegClass_FPR]++;
                emitpcode(0x1cf, target, 0x14);
                emitpcode(0xad, target, fpreg);
                scratch21 = make_register_operand(fpreg, 1);
                *result = scratch21;
            }
            break;
        case 16:
            node = values[0].node;
            gen_func_table[node->type](node, 0, &values[0].operand);
            info = fn_004662F0(&values[0].operand, -1, -1);
            target = used_virtual_registers[5];
            fpreg = used_virtual_registers[0];
            if (!info) {
                used_virtual_registers[5]++;
                used_virtual_registers[0]++;
                emitpcode(0x298, 0xf, (SInt16)target, 0, 0);
                emitpcode(0x264, fpreg, target);
                scratch12 = make_register_operand(fpreg, 0);
                *result = scratch12;
                fn_004B1440(*result, &values[0].operand, (Type*)&stint128);
            } else if (info->rclass == 5) {
                emitpcode(0x298, 0xf, (SInt16)values[0].reg, 0, 0);
                scratch13 = make_register_operand(values[0].reg, 5);
                *result = scratch13;
            } else if (info->rclass == 0) {
                used_virtual_registers[5]++;
                emitpcode(0x298, 0xf, (SInt16)target, 0, 0);
                emitpcode(0x264, values[0].operand.reg, target);
                scratch14 = make_register_operand(values[0].operand.reg, 0);
                *result = scratch14;
            } else {
                if (info->rclass != 6 || info->reg != 3) {
                    CError_Internal("Intrinsics.c", 0x80d);
                }
                used_virtual_registers[5]++;
                emitpcode(0x298, 0xf, (SInt16)target, 0, 0);
                scratch15 = make_register_operand(target, 5);
                *result = scratch15;
            }
            break;
        default:
            CError_Internal("Intrinsics.c", 0x81b);
    }
}

//! @reconstructed
void Intrinsics_GenerateIntrinsicCall(ENode* node, short reg, Operand* result) {
    ENode* callee = node->data.funccall.funcref;
    ENodeList* args = node->data.funccall.args;
    SInt32 id = callee->data.objref->u.intrinsicid;
    Operand operand1, operand2;
    SInt32 target;
    Operand defaultValue = make_register_operand(0, 0);
    *result = defaultValue;
    if (!copts.unknown47) {
        if (callee->data.objref->u.intrinsicid >= 0x26 &&
            callee->data.objref->u.intrinsicid <= 0x75) {
            MIPSError_Error(28);
        }
        if (callee->data.objref->u.intrinsicid >= 0x85 &&
            callee->data.objref->u.intrinsicid <= 0xb1) {
            MIPSError_Error(28);
        }
    }
    switch (id) {
        case 0x0:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            result->kind = 0;
            result->immOffset = 0;
            result->object = NULL;
            result->reg = fn_004AFCC0(&operand1);
            break;
        case 0x1: {
            SInt16 dest = 4;
            SInt32 index = 1;
            if (intrinsic_sequence_count >= 1) {
                do {
                    emitpcode(move_opcode, dest, intrinsic_sequence_operands[index].reg);
                    intrinsic_sequence_operands[index].reg = dest;
                    index++;
                    dest++;
                } while (index <= intrinsic_sequence_count);
            }
            break;
        }
        case 0x2:
            intrinsic_sequence_pending = FALSE;
            intrinsic_sequence_count = 0;
            break;
        case 0x3:
            intrinsic_sequence_count = 0;
            if (intrinsic_sequence_pending) {
                new_basic_block();
            }
            break;
        case 0x4: {
            SInt32 index;
            gen_func_table[args->node->type](args->node, 0, &operand1);
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand1.kind != 3) {
                CError_Error(0xA7, "__evaluate");
            }
            index = -operand1.immediate.lo;
            if (index <= 0 || index > 4) {
                CError_Error(0xA7, "__evaluate");
            }
            intrinsic_sequence_count++;
            if (intrinsic_sequence_count > 4) {
                CError_Error(0xA7, "__evaluate");
            }
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, args->node->rtype, 0);
            }
            intrinsic_sequence_operands[index] = operand2;
            break;
        }
        case 0x5:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            fn_004384D0(&operand1);
            break;
        case 0x6:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            *result = fn_004383F0(&operand1);
            break;
        case 0x7:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            *result = fn_00438330(&operand1);
            break;
        case 0x8: {
            Operand operand3;
            SInt32 align;
            SInt32 sourceAlign;
            gen_func_table[args->node->type](args->node, 0, &operand1);
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (args->next->next) {
                gen_func_table[args->next->next->node->type](args->next->next->node, 0, &operand3);
            }
            align = CMach_GetObjectAlign(operand1.object);
            sourceAlign = CMach_GetObjectAlign(operand2.object);
            if (sourceAlign < align) {
                align = sourceAlign;
            }
            fn_0053CC40(&operand1, &operand2, (SInt32)operand3.object, align);
            *result = operand1;
            break;
        }
        case 0x9: {
            Operand operand3;
            SInt32 align;
            SInt32 sourceAlign;
            gen_func_table[args->node->type](args->node, 0, &operand1);
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            gen_func_table[args->next->next->node->type](args->next->next->node, 0, &operand3);
            if (operand3.kind == 3) {
                align = CMach_GetObjectAlign(operand1.object);
                sourceAlign = CMach_GetObjectAlign(operand2.object);
                if (sourceAlign < align) {
                    align = sourceAlign;
                }
                fn_0053CC40(&operand1, &operand2, (SInt32)operand3.object, align);
            } else {
                if (operand1.kind != 0) {
                    Coerce_to_register(&operand1, TYPE(&void_ptr), 0);
                }
                if (operand2.kind != 0) {
                    Coerce_to_register(&operand2, TYPE(&void_ptr), 0);
                }
                if (operand3.kind != 0) {
                    Coerce_to_register(&operand3, TYPE(&stunsignedint), 0);
                }
                pass_int_argument(&operand1, 0, 1);
                pass_int_argument(&operand2, 0, 2);
                pass_int_argument(&operand3, 0, 2);
                call_runtime_function(rt_memcpy);
            }
            *result = operand1;
            break;
        }
        case 0xa:
        case 0xb: {
            SInt32 scratch;
            CLabel* branchLabel;
            CLabel* afterLabel;
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind == 3) {
                if (operand1.immediate.hi < 0) {
                    operand1.immediate = CInt64_Neg(operand1.immediate);
                }
                *result = operand1;
                break;
            }
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(id == 0xa ? &stsignedint : &stsignedlong), 1);
            }
            result->kind = 0;
            if (copts.conditional_move) {
                scratch = used_virtual_registers[RegClass_GPR]++;
                emitpcode(0xd4, scratch, operand1.reg, 0);
                result->reg = used_virtual_registers[RegClass_GPR]++;
                emitpcode(0xb8, result->reg, operand1.reg);
                emitpcode(0x111, result->reg, operand1.reg, scratch);
            } else {
                branchLabel = newlabel();
                afterLabel = newlabel();
                branchLabel->pclabel = makepclabel();
                afterLabel->pclabel = makepclabel();
                result->reg = used_virtual_registers[RegClass_GPR]++;
                emitpcode(move_opcode, result->reg, operand1.reg);
                emit_branch(0x1c, &operand1, branchLabel);
                emitpcode(0xb8, result->reg, operand1.reg);
                emit_label(branchLabel->pclabel);
            }
            break;
        }
        case 0xc:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stfloat), 1);
            }
            if (copts.fp_library) {
                result->reg = used_virtual_registers[RegClass_FPR]++;
                emitpcode(0xeb, result->reg, operand1.reg);
                result->kind = 1;
            } else {
                target = used_virtual_registers[RegClass_GPR]++;
                emitpcode(0xd2, target, operand1.reg, 1);
                result->reg = used_virtual_registers[RegClass_GPR]++;
                emitpcode(0xda, result->reg, target, 1);
                result->kind = 0;
            }
            break;
        case 0xd:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stdouble), 1);
            }
            if (copts.fp_library & 2) {
                result->reg = used_virtual_registers[RegClass_FPR]++;
                emitpcode(0xea, result->reg, operand1.reg);
                result->kind = 1;
            } else {
                target = used_virtual_registers[RegClass_GPR]++;
                emitpcode(0x84, target, operand1.reg, 1);
                result->reg = used_virtual_registers[RegClass_GPR]++;
                emitpcode(0x8a, result->reg, target, 1);
                result->kind = 0;
            }
            break;
        case 0xe:
        case 0xf:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stsignedlong), 1);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stsignedlong), 1);
            }
            emitpcode(id == 0xe ? 0xb6 : 0xb7, operand1.reg, operand2.reg);
            result->reg = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0xa8, result->reg);
            result->kind = 0;
            break;
        case 0x10:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stfloat), 1);
            }
            if (!copts.fp_library) {
                CError_FATAL(0xa79);
            }
            result->reg = used_virtual_registers[RegClass_FPR]++;
            emitpcode(0x101, result->reg, operand1.reg);
            result->kind = 1;
            break;
        case 0x11:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stfloat), 1);
            }
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stfloat), 1);
            }
            if (!copts.fp_library) {
                CError_FATAL(0xa87);
            }
            result->reg = used_virtual_registers[RegClass_FPR]++;
            emitpcode(0x125, result->reg, operand1.reg, operand2.reg);
            result->kind = 1;
            break;
        case 0x13: {
            SInt32 lane;
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stcustomtype113), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 3) {
                CError_Error(0xa7, "__sel");
            }
            lane = operand2.immediate.lo;
            if (lane < 0 || lane > 7) {
                CError_Error(0xa7, "__sel");
            }
            *result = operand1;
            result->unk28 = lane + 1;
            break;
        }
        case 0x14:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 3) {
                CError_Error(0xa7, "__fixed__8x8_vector");
            }
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stcustomtype113), 0);
            }
            *result = operand1;
            result->unk28 = 0;
            break;
        case 0x15:
            *result = fn_004A8EC0(0x1a7, 0x1a8, args->node, args->next->node);
            break;
        case 0x16:
            *result = fn_004A8EC0(0x1a9, 0x1aa, args->node, args->next->node);
            break;
        case 0x17:
            *result = fn_004A8EC0(0x1b5, 0x1b6, args->node, args->next->node);
            break;
        case 0x18:
            *result = fn_004A8EC0(0x1b7, 0x1b8, args->node, args->next->node);
            break;
        case 0x19:
            *result = fn_004A8E20(0x1b9, args->node);
            break;
        case 0x1a:
            *result = fn_004A8E20(0x1ba, args->node);
            break;
        case 0x1b:
            *result = fn_004A8E20(0x1bb, args->node);
            break;
        case 0x23:
        case 0x24:
        case 0x25: {
            SInt32 lane;
            SInt16 opcode;
            gen_func_table[args->next->node->type](args->next->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stcustomtype113), 0);
            }
            gen_func_table[args->node->type](args->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stcustomtype113), 0);
            }
            lane = operand2.unk28 ? operand2.unk28 - 1 : 11;
            opcode = id == 0x23 ? 0x1a1 : id == 0x24 ? 0x1a3 : 0x1a5;
            emitpcode(opcode, operand1.reg, operand2.reg, lane);
            break;
        }
        case 0x26:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x206, target, operand1.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x27:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x207, target, operand1.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x28:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x208, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x29:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x209, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x2a:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x20b, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x2b:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x20c, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x2c:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x20d, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x2d:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x20e, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x2e:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x20f, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x2f:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x210, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x30:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x20a, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x31:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x211, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x32:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x212, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x33:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x213, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x34:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x214, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x35:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x215, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x36:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x216, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x37:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x217, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x38:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x218, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x39:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x219, target, operand1.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x3a:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x21a, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x3b:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x21b, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x3c:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            emitpcode(0x21c, operand1.reg, operand2.reg);
            break;
        case 0x3d:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            emitpcode(0x21d, operand1.reg, operand2.reg);
            break;
        case 0x3e:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            emitpcode(0x21e, operand1.reg, operand2.reg);
            break;
        case 0x3f:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x22c, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x40:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x22d, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x41:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x231, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x42:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x232, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x43:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x233, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x44:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x23f, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x45:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x240, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x46:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x244, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x47:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x245, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x48:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x246, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x49:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x21f, target, operand1.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x4a:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x220, target, operand1.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x4b:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x221, target, operand1.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x4c:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x223, target, operand1.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x4d:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x225, target, operand1.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x4e:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x226, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x4f:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x227, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x50:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x228, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x51:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x229, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x52:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x22a, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x53:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x22b, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x54:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x22e, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x55:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x22f, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x56:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x230, target, operand1.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x57:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x234, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x58:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x235, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x59:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x23d, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x5a:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x23e, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x5b:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x247, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x5c:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x248, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x5d:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x249, target, operand1.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x5e:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x24a, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x5f:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x24b, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x60:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x24c, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x61:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x24d, target, operand1.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x62:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x24e, target, operand1.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x63:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x24f, target, operand1.reg, args->next->node->data.intval.lo);
            *result = make_register_operand(target, 0);
            break;
        case 0x64:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x250, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x65:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x251, target, operand1.reg, args->next->node->data.intval.lo);
            *result = make_register_operand(target, 0);
            break;
        case 0x66:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x252, target, operand1.reg, args->next->node->data.intval.lo);
            *result = make_register_operand(target, 0);
            break;
        case 0x67:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x253, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x68:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x254, target, operand1.reg, args->next->node->data.intval.lo);
            *result = make_register_operand(target, 0);
            break;
        case 0x69:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x255, target, operand1.reg, args->next->node->data.intval.lo);
            *result = make_register_operand(target, 0);
            break;
        case 0x6a:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x256, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x6b:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x257, target, operand1.reg, args->next->node->data.intval.lo);
            *result = make_register_operand(target, 0);
            break;
        case 0x6c:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x258, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x6d:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x259, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x6e:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x25a, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x6f:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x25b, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x70:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x25c, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x71:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x25d, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x72:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x25e, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x73:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x25f, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x74:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x260, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x75:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x261, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x76:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            emitpcode(0x200, operand1.reg, args->next->node->data.intval.lo);
            break;
        case 0x77:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            emitpcode(0x201, operand1.reg, args->next->node->data.intval.lo);
            break;
        case 0x78:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            emitpcode(0x1ff, operand1.reg);
            break;
        case 0x79:
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x1fc, target);
            *result = make_register_operand(target, 0);
            break;
        case 0x7a:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            gen_func_table[args->next->node->type](args->next->node, 0, &operand2);
            if (operand2.kind != 0) {
                Coerce_to_register(&operand2, TYPE(&stint128), 0);
            }
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x262, target, operand1.reg, operand2.reg);
            *result = make_register_operand(target, 0);
            break;
        case 0x7b:
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x236, target);
            *result = make_register_operand(target, 0);
            break;
        case 0x7c:
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x237, target);
            *result = make_register_operand(target, 0);
            break;
        case 0x7d:
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x238, target);
            *result = make_register_operand(target, 0);
            break;
        case 0x7e:
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x239, target);
            *result = make_register_operand(target, 0);
            break;
        case 0x7f:
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x23a, target);
            *result = make_register_operand(target, 0);
            break;
        case 0x80:
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x23b, target);
            *result = make_register_operand(target, 0);
            break;
        case 0x81:
            target = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x23c, target);
            *result = make_register_operand(target, 0);
            break;
        case 0x82:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            emitpcode(0x241, operand1.reg);
            break;
        case 0x83:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            emitpcode(0x242, operand1.reg);
            break;
        case 0x84:
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            emitpcode(0x243, operand1.reg);
            break;
        case 0x85:
            fn_00463AF0(args, 0x269, result);
            break;
        case 0x86:
            fn_00463AF0(args, 0x2af, result);
            break;
        case 0x87:
            fn_00463AF0(args, 0x2b0, result);
            break;
        case 0x88:
            fn_00463AF0(args, 0x26a, result);
            break;
        case 0x89:
            fn_00463AF0(args, 0x26c, result);
            break;
        case 0x8a:
            fn_00463AF0(args, 0x26e, result);
            break;
        case 0x8b:
            fn_00463AF0(args, 0x294, result);
            break;
        case 0x8c:
            fn_00463AF0(args, 0x296, result);
            break;
        case 0x8d:
            fn_00463AF0(args, 0x298, result);
            break;
        case 0x8e:
            fn_00463AF0(args, 0x2a2, result);
            break;
        case 0x8f:
            fn_00463AF0(args, 0x2a3, result);
            break;
        case 0x90:
            fn_00463AF0(args, 0x2a5, result);
            break;
        case 0x91:
            fn_00463AF0(args, 0x2a9, result);
            break;
        case 0x92:
            fn_00463AF0(args, 0x2aa, result);
            break;
        case 0x93:
            fn_00463AF0(args, 0x2ac, result);
            break;
        case 0x94:
            fn_00463AF0(args, 0x2b1, result);
            break;
        case 0x95:
            fn_00463AF0(args, 0x2b3, result);
            break;
        case 0x96:
            fn_00463AF0(args, 0x2b5, result);
            break;
        case 0x97:
            fn_00463AF0(args, 0x2c0, result);
            break;
        case 0x98:
            fn_00463AF0(args, 0x2c2, result);
            break;
        case 0x99:
            fn_00463AF0(args, 0x2c4, result);
            break;
        case 0x9a:
            fn_00463AF0(args, 0x2d9, result);
            break;
        case 0x9b:
            fn_00463AF0(args, 0x2db, result);
            break;
        case 0x9c:
            fn_00463AF0(args, 0x2dd, result);
            break;
        case 0x9d:
            fn_00463AF0(args, 0x2cf, result);
            break;
        case 0x9e:
            fn_00463AF0(args, 0x2d0, result);
            break;
        case 0x9f:
            fn_00463AF0(args, 0x27c, result);
            break;
        case 0xa0:
            fn_00463AF0(args, 0x2d8, result);
            break;
        case 0xa1:
            fn_00463AF0(args, 0x2d4, result);
            break;
        case 0xa2:
            fn_00463AF0(args, 0x2e7, result);
            break;
        case 0xa3:
            fn_00463AF0(args, 0x2d1, result);
            break;
        case 0xa4:
            fn_00463AF0(args, 0x2d2, result);
            break;
        case 0xa5:
            fn_00463AF0(args, 0x2d3, result);
            break;
        case 0xa6:
            fn_00463AF0(args, 0x2d5, result);
            break;
        case 0xa7:
            fn_00463AF0(args, 0x27b, result);
            break;
        case 0xa8:
            fn_00463AF0(args, 0x27d, result);
            break;
        case 0xa9:
            fn_00463AF0(args, 0x27e, result);
            break;
        case 0xaa:
            fn_00463AF0(args, 0x27f, result);
            break;
        case 0xab:
            fn_00463AF0(args, 0x280, result);
            break;
        case 0xac:
            fn_00463AF0(args, 0x28e, result);
            break;
        case 0xad:
            fn_00463AF0(args, 0x28f, result);
            break;
        case 0xae:
            fn_00463AF0(args, 0x290, result);
            break;
        case 0xaf:
            fn_00463AF0(args, 0x291, result);
            break;
        case 0xb0:
            emitpcode(0x2ce);
            break;
        case 0xb1: {
            SInt32 vectorReg;
            SInt32 selectedReg;
            SInt32 integerReg;
            SInt32 resultReg;
            SInt32 tempReg;
            SInt32 fpReg;
            UInt8 selector;
            gen_func_table[args->node->type](args->node, 0, &operand1);
            if (operand1.kind != 0) {
                Coerce_to_register(&operand1, TYPE(&stint128), 0);
            }
            if (operand1.kind == 5 || operand1.kind == 6) {
                vectorReg = operand1.reg;
            } else {
                vectorReg = used_virtual_registers[5]++;
                emitpcode(0x266, operand1.reg, vectorReg);
            }
            selector = (UInt8)args->next->node->data.intval.lo;
            selectedReg = used_virtual_registers[5]++;
            emitpcode(0x2af, selector, selectedReg, vectorReg);
            integerReg = used_virtual_registers[RegClass_GPR]++;
            emitpcode(0x264, integerReg, selectedReg);
            switch (selector) {
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                    resultReg = integerReg;
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                    tempReg = used_virtual_registers[RegClass_GPR]++;
                    emitpcode(0x220, tempReg, integerReg);
                    resultReg = used_virtual_registers[RegClass_GPR]++;
                    emitpcode(0x223, resultReg, tempReg);
                    break;
                case 2:
                case 3:
                    resultReg = used_virtual_registers[RegClass_GPR]++;
                    emitpcode(0x223, resultReg, integerReg);
                    break;
                case 1:
                    resultReg = used_virtual_registers[RegClass_GPR]++;
                    emitpcode(0x21b, resultReg, integerReg, integerReg);
                    tempReg = used_virtual_registers[RegClass_GPR]++;
                    emitpcode(0x220, tempReg, resultReg);
                    resultReg = used_virtual_registers[RegClass_GPR]++;
                    emitpcode(0x223, resultReg, tempReg);
                    break;
                default:
                    CError_Internal("Intrinsics.c", 0xf3c);
                    break;
            }
            fpReg = used_virtual_registers[RegClass_FPR]++;
            emitpcode(0xad, resultReg, fpReg);
            *result = make_register_operand(fpReg, 1);
            break;
        }
        default:
            if (id < 0xc6 || id > 0x4ab) {
                CError_Internal("Intrinsics.c", 0x101c);
            }
            break;
    }
}

void fn_004679A0(
    SInt32 index,
    const char* name,
    Type* arg0,
    Type* arg1,
    Type* arg2,
    Type* arg3,
    Type* arg4,
    Type* arg5
) {
    Object* volatile* slot = &intrinsics_601CD8[index];
    Object* object = CParser_NewRTFunc(
        TYPE(&stint128), GetHashNameNodeExport(name), 0, 6, arg0, arg1, arg2, arg3, arg4, arg5
    );
    *slot = object;
    object->u.intrinsicid = index;
    TYPE_FUNC((*slot)->type)->flags |= 0x200;
    CScope_AddGlobalObject(*slot);
}

void fn_00467A10(
    SInt32 index, const char* name, Type* arg0, Type* arg1, Type* arg2, Type* arg3, Type* arg4
) {
    Object* volatile* slot = &intrinsics_601CD8[index];
    Object* object = CParser_NewRTFunc(
        TYPE(&stint128), GetHashNameNodeExport(name), 0, 5, arg0, arg1, arg2, arg3, arg4
    );
    *slot = object;
    object->u.intrinsicid = index;
    TYPE_FUNC((*slot)->type)->flags |= 0x200;
    CScope_AddGlobalObject(*slot);
}

void fn_00467A80(
    SInt32 index, const char* name, Type* arg0, Type* arg1, Type* arg2, Type* arg3, Type* arg4
) {
    Object* volatile* slot = &intrinsics_601CD8[index];
    Object* object = CParser_NewRTFunc(
        TYPE(&stfloat), GetHashNameNodeExport(name), 0, 5, arg0, arg1, arg2, arg3, arg4
    );
    *slot = object;
    object->u.intrinsicid = index;
    TYPE_FUNC((*slot)->type)->flags |= 0x200;
    CScope_AddGlobalObject(*slot);
}

void fn_00467AF0(SInt32 index, const char* name, Type* arg0, Type* arg1, Type* arg2, Type* arg3) {
    Object* volatile* slot = &intrinsics_601CD8[index];
    Object* object = CParser_NewRTFunc(
        TYPE(&stint128), GetHashNameNodeExport(name), 0, 4, arg0, arg1, arg2, arg3
    );
    *slot = object;
    object->u.intrinsicid = index;
    TYPE_FUNC((*slot)->type)->flags |= 0x200;
    CScope_AddGlobalObject(*slot);
}

void fn_00467B50(SInt32 index, const char* name, Type* arg0, Type* arg1, Type* arg2) {
    Object* volatile* slot = &intrinsics_601CD8[index];
    Object* object =
        CParser_NewRTFunc(TYPE(&stfloat), GetHashNameNodeExport(name), 0, 3, arg0, arg1, arg2);
    *slot = object;
    object->u.intrinsicid = index;
    TYPE_FUNC((*slot)->type)->flags |= 0x200;
    CScope_AddGlobalObject(*slot);
}

void fn_00467BB0(SInt32 index, const char* name, Type* arg0, Type* arg1, Type* arg2) {
    Object* volatile* slot = &intrinsics_601CD8[index];
    Object* object =
        CParser_NewRTFunc(TYPE(&stint128), GetHashNameNodeExport(name), 0, 3, arg0, arg1, arg2);
    *slot = object;
    object->u.intrinsicid = index;
    TYPE_FUNC((*slot)->type)->flags |= 0x200;
    CScope_AddGlobalObject(*slot);
}

void fn_00467C10(SInt32 index, const char* name, Type* arg0) {
    Object* volatile* slot = &intrinsics_601CD8[index];
    Object* object =
        CParser_NewRTFunc(TYPE(&stvoid), GetHashNameNodeExport(name), 0, 2, arg0, arg0);
    *slot = object;
    object->u.intrinsicid = index;
    TYPE_FUNC((*slot)->type)->flags |= 0x200;
    CScope_AddGlobalObject(*slot);
}

void fn_00467C70(SInt32 index, const char* name, Type* arg0) {
    Object* volatile* slot = &intrinsics_601CD8[index];
    Object* object = CParser_NewRTFunc(arg0, GetHashNameNodeExport(name), 0, 2, arg0, arg0);
    *slot = object;
    object->u.intrinsicid = index;
    TYPE_FUNC((*slot)->type)->flags |= 0x200;
    CScope_AddGlobalObject(*slot);
}

void fn_00467CD0(SInt32 index, const char* name, Type* arg0) {
    Object* volatile* slot = &intrinsics_601CD8[index];
    Object* object = CParser_NewRTFunc(arg0, GetHashNameNodeExport(name), 0, 1, arg0);
    *slot = object;
    object->u.intrinsicid = index;
    TYPE_FUNC((*slot)->type)->flags |= 0x200;
    CScope_AddGlobalObject(*slot);
}

#pragma defer_codegen off

void fn_00467F50(void) {
    SInt32 i;
    for (i = 0; i < 16; i++) {
        HashNameNode* hash = GetHashNameNodeExport(intrinsic_export_entries[i].name);
        intrinsic_export_entries[i].object->name = hash;
    }
    cg_runtime_intrinsic_63648C = intrinsic_export_entries[0].object;
}

#pragma opt_strength_reduction off

void fn_00467F90(void) {
    SInt32 i;
    SInt32 offset;
    intrinsic_export_entries[0].object = CParser_NewRTFunc(TYPE(&stvoid), 0, 0, 0);
    intrinsic_export_entries[1].object = CParser_NewRTFunc(TYPE(&stvoid), 0, 0, 0);
    intrinsic_export_entries[2].object = CParser_NewRTFunc(TYPE(&stvoid), 0, 0, 0);
    intrinsic_export_entries[3].object = CParser_NewRTFunc(TYPE(&stsignedint), 0, 0, 0);
    intrinsic_export_entries[4].object = CParser_NewRTFunc(TYPE(&stunsignedint), 0, 0, 0);
    intrinsic_export_entries[5].object = CParser_NewRTFunc(TYPE(&stsignedint), 0, 0, 0);
    intrinsic_export_entries[6].object = CParser_NewRTFunc(TYPE(&stunsignedint), 0, 0, 0);
    intrinsic_export_entries[7].object = CParser_NewRTFunc(TYPE(&stsignedint), 0, 0, 0);
    intrinsic_export_entries[8].object = CParser_NewRTFunc(TYPE(&stunsignedint), 0, 0, 0);
    i = 9;
    offset = 0x90;
    do {
        Object* object = CParser_NewRTFunc(TYPE(&stvoid), 0, 0, 0);
        i++;
        ((IntrinsicExportEntry*)((UInt8*)intrinsic_export_entries + offset))->object = object;
        offset += 16;
    } while (i < 16);
    cg_runtime_intrinsic_63648C = intrinsic_export_entries[0].object;
}

#pragma opt_strength_reduction on
#pragma defer_codegen on

#pragma defer_codegen off
#pragma opt_strength_reduction off
void fn_00467D30(void) {
    UInt16 i;
    SInt32 count;
    SInt32 index;
    const char* fmt;
    Type* types[5];

    for (i = 0; i <= 0x3E5; i++) {
        if (opcode_info[i].unk20) {
            fmt = opcode_info[i].format;
            count = 0;
            while (*fmt) {
                if (*fmt == ';') {
                    break;
                }
                if (count > 4) {
                    CError_Internal("Intrinsics.c", 92);
                }
                switch (*fmt) {
                    case '(':
                    case ')':
                        break;
                    case '<':
                    case '>':
                    case '@':
                    case 'B':
                    case 'C':
                    case 'X':
                    case 'c':
                    case 'k':
                        types[count++] = TYPE(&stunsignedlong);
                        break;
                    case 'D':
                    case 'E':
                    case 'G':
                    case 'N':
                    case 'S':
                    case 'T':
                    case 'V':
                    case 'W':
                    case 'd':
                    case 's':
                    case 't':
                        types[count++] = TYPE(&stsignedlong);
                        break;
                    case 'i':
                    case 'u':
                        types[count++] = TYPE(&stunsignedlong);
                        break;
                    case 'j':
                    case 'o':
                        types[count++] = TYPE(&stsignedlong);
                        break;
                    case 'I':
                        types[count++] = TYPE(&stunsignedlong);
                        break;
                }
                fmt++;
            }
            if (count == 0) {
                ((Object* volatile*)intrinsics_601CD8)[opcode_info[i].unk24] = CParser_NewRTFunc(
                    TYPE(&stunsignedlong),
                    GetHashNameNodeExport((const char*)opcode_info[i].unk20),
                    0,
                    0
                );
            } else if (count == 1) {
                ((Object* volatile*)intrinsics_601CD8)[opcode_info[i].unk24] = CParser_NewRTFunc(
                    TYPE(&stunsignedlong),
                    GetHashNameNodeExport((const char*)opcode_info[i].unk20),
                    0,
                    1,
                    types[0]
                );
            } else if (count == 2) {
                ((Object* volatile*)intrinsics_601CD8)[opcode_info[i].unk24] = CParser_NewRTFunc(
                    TYPE(&stunsignedlong),
                    GetHashNameNodeExport((const char*)opcode_info[i].unk20),
                    0,
                    2,
                    types[0],
                    types[1]
                );
            } else if (count == 3) {
                ((Object* volatile*)intrinsics_601CD8)[opcode_info[i].unk24] = CParser_NewRTFunc(
                    TYPE(&stunsignedlong),
                    GetHashNameNodeExport((const char*)opcode_info[i].unk20),
                    0,
                    3,
                    types[0],
                    types[1],
                    types[2]
                );
            } else if (count == 4) {
                ((Object* volatile*)intrinsics_601CD8)[opcode_info[i].unk24] = CParser_NewRTFunc(
                    TYPE(&stunsignedlong),
                    GetHashNameNodeExport((const char*)opcode_info[i].unk20),
                    0,
                    4,
                    types[0],
                    types[1],
                    types[2],
                    types[3]
                );
            }
            index = opcode_info[i].unk24;
            ((Object* volatile*)intrinsics_601CD8)[index]->u.intrinsicid = index;
            TYPE_FUNC(((Object* volatile*)intrinsics_601CD8)[index]->type)->flags |= FUNC_INTRINSIC;
            CScope_AddGlobalObject(((Object* volatile*)intrinsics_601CD8)[index]);
        }
    }
}

#pragma opt_strength_reduction on
#pragma defer_codegen on

void Intrinsics_SetupRuntimeObjects(void) {
    UInt8 saved = copts.cplusplus;
    SInt32 i;
    copts.cplusplus = FALSE;
    for (i = 0; i < 0x4A8; i++) {
        intrinsics_601CD8[i] = NULL;
    }
    fn_00467D30();
    {
        Object* obj = CParser_NewRTFunc(TYPE(&stvoid), GetHashNameNodeExport("__asm_start"), 0, 0);
        intrinsics_601CD8[1] = obj;
        obj->u.intrinsicid = 1;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj =
            CParser_NewRTFunc(TYPE(&stvoid), GetHashNameNodeExport("__asm_start_reord"), 0, 0);
        intrinsics_601CD8[2] = obj;
        obj->u.intrinsicid = 2;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(TYPE(&stvoid), GetHashNameNodeExport("__asm_end"), 0, 0);
        intrinsics_601CD8[3] = obj;
        obj->u.intrinsicid = 3;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(
            TYPE(&stvoid), GetHashNameNodeExport("__evaluate"), 0, 2, &stsignedlong, &stsignedlong
        );
        intrinsics_601CD8[4] = obj;
        obj->u.intrinsicid = 4;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    if (copts.unknown15) {
        {
            Object* obj = CParser_NewRTFunc(
                TYPE(&stvoid), GetHashNameNodeExport("__builtin_initialize"), 0, 1, &void_ptr
            );
            intrinsics_601CD8[5] = obj;
            obj->u.intrinsicid = 5;
            TYPE_FUNC(obj->type)->flags |= 0x200;
            CScope_AddGlobalObject(obj);
        }
        fn_00467CD0(6, "__builtin_args_info", TYPE(&stsignedint));
    }
    fn_00467CD0(0x26, "_pabsh", TYPE(&stint128));
    fn_00467CD0(0x27, "_pabsw", TYPE(&stint128));
    fn_00467C70(0x28, "_paddb", TYPE(&stint128));
    fn_00467C70(0x29, "_paddh", TYPE(&stint128));
    fn_00467C70(0x2a, "_paddsb", TYPE(&stint128));
    fn_00467C70(0x2b, "_paddsh", TYPE(&stint128));
    fn_00467C70(0x2c, "_paddsw", TYPE(&stint128));
    fn_00467C70(0x2d, "_paddub", TYPE(&stint128));
    fn_00467C70(0x2e, "_padduh", TYPE(&stint128));
    fn_00467C70(0x2f, "_padduw", TYPE(&stint128));
    fn_00467C70(0x30, "_paddw", TYPE(&stint128));
    fn_00467C70(0x31, "_padsbh", TYPE(&stint128));
    fn_00467C70(0x32, "_pand", TYPE(&stint128));
    fn_00467C70(0x33, "_pceqb", TYPE(&stint128));
    fn_00467C70(0x34, "_pceqh", TYPE(&stint128));
    fn_00467C70(0x35, "_pceqw", TYPE(&stint128));
    fn_00467C70(0x36, "_pcgtb", TYPE(&stint128));
    fn_00467C70(0x37, "_pcgth", TYPE(&stint128));
    fn_00467C70(0x38, "_pcgtw", TYPE(&stint128));
    fn_00467CD0(0x39, "_pcpyh", TYPE(&stint128));
    fn_00467C70(0x3a, "_pcpyld", TYPE(&stint128));
    fn_00467C70(0x3b, "_pcpyud", TYPE(&stint128));
    fn_00467C10(0x3c, "_pdivbw", TYPE(&stint128));
    fn_00467C10(0x3d, "_pdivuw", TYPE(&stint128));
    fn_00467C10(0x3e, "_pdivw", TYPE(&stint128));
    fn_00467C70(0x3f, "_phmadh", TYPE(&stint128));
    fn_00467C70(0x40, "_phmsbh", TYPE(&stint128));
    fn_00467C70(0x41, "_pmaddh", TYPE(&stint128));
    fn_00467C70(0x42, "_pmadduw", TYPE(&stint128));
    fn_00467C70(0x43, "_pmaddw", TYPE(&stint128));
    fn_00467C70(0x44, "_pmsubh", TYPE(&stint128));
    fn_00467C70(0x45, "_pmsubw", TYPE(&stint128));
    fn_00467C70(0x46, "_pmulth", TYPE(&stint128));
    fn_00467C70(0x47, "_pmultuw", TYPE(&stint128));
    fn_00467C70(0x48, "_pmultw", TYPE(&stint128));
    fn_00467CD0(0x49, "_pexch", TYPE(&stint128));
    fn_00467CD0(0x4a, "_pexcw", TYPE(&stint128));
    fn_00467CD0(0x4b, "_pexeh", TYPE(&stint128));
    fn_00467CD0(0x4c, "_pexew", TYPE(&stint128));
    fn_00467CD0(0x4d, "_pext5", TYPE(&stint128));
    fn_00467C70(0x4e, "_pextlb", TYPE(&stint128));
    fn_00467C70(0x4f, "_pextlh", TYPE(&stint128));
    fn_00467C70(0x50, "_pextlw", TYPE(&stint128));
    fn_00467C70(0x51, "_pextub", TYPE(&stint128));
    fn_00467C70(0x52, "_pextuh", TYPE(&stint128));
    fn_00467C70(0x53, "_pextuw", TYPE(&stint128));
    fn_00467C70(0x54, "_pinth", TYPE(&stint128));
    fn_00467C70(0x55, "_pinteh", TYPE(&stint128));
    fn_00467CD0(0x56, "_plzcw", TYPE(&stint128));
    fn_00467C70(0x57, "_pmaxh", TYPE(&stint128));
    fn_00467C70(0x58, "_pmaxw", TYPE(&stint128));
    fn_00467C70(0x59, "_pminh", TYPE(&stint128));
    fn_00467C70(0x5a, "_pminw", TYPE(&stint128));
    fn_00467C70(0x5b, "_pnor", TYPE(&stint128));
    fn_00467C70(0x5c, "_por", TYPE(&stint128));
    fn_00467CD0(0x5d, "_ppac5", TYPE(&stint128));
    fn_00467C70(0x5e, "_ppacb", TYPE(&stint128));
    fn_00467C70(0x5f, "_ppach", TYPE(&stint128));
    fn_00467C70(0x60, "_ppacw", TYPE(&stint128));
    fn_00467CD0(0x61, "_prevh", TYPE(&stint128));
    fn_00467CD0(0x62, "_prot3w", TYPE(&stint128));
    fn_00467C70(99, "_psllh", TYPE(&stint128));
    fn_00467C70(100, "_psllvw", TYPE(&stint128));
    fn_00467C70(0x65, "_psllw", TYPE(&stint128));
    fn_00467C70(0x66, "_psrah", TYPE(&stint128));
    fn_00467C70(0x67, "_psravw", TYPE(&stint128));
    fn_00467C70(0x68, "_psraw", TYPE(&stint128));
    fn_00467C70(0x69, "_psrlh", TYPE(&stint128));
    fn_00467C70(0x6a, "_psrlvw", TYPE(&stint128));
    fn_00467C70(0x6b, "_psrlw", TYPE(&stint128));
    fn_00467C70(0x6c, "_psubb", TYPE(&stint128));
    fn_00467C70(0x6d, "_psubh", TYPE(&stint128));
    fn_00467C70(0x6e, "_psubsb", TYPE(&stint128));
    fn_00467C70(0x6f, "_psubsh", TYPE(&stint128));
    fn_00467C70(0x70, "_psubsw", TYPE(&stint128));
    fn_00467C70(0x71, "_psubub", TYPE(&stint128));
    fn_00467C70(0x72, "_psubuh", TYPE(&stint128));
    fn_00467C70(0x73, "_psubuw", TYPE(&stint128));
    fn_00467C70(0x74, "_psubw", TYPE(&stint128));
    fn_00467C70(0x75, "_pxor", TYPE(&stint128));
    fn_00467C10(0x76, "_mtsab", TYPE(&stint128));
    fn_00467C10(0x77, "_mtsah", TYPE(&stint128));
    {
        Object* obj =
            CParser_NewRTFunc(TYPE(&stvoid), GetHashNameNodeExport("_mtsa"), 0, 1, &stint128);
        intrinsics_601CD8[0x78] = obj;
        obj->u.intrinsicid = 0x78;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(TYPE(&stint128), GetHashNameNodeExport("_mfsa"), 0, 0);
        intrinsics_601CD8[0x79] = obj;
        obj->u.intrinsicid = 0x79;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    fn_00467C70(0x7a, "_qfsrv", TYPE(&stint128));
    {
        Object* obj = CParser_NewRTFunc(TYPE(&stint128), GetHashNameNodeExport("_pmfhi"), 0, 0);
        intrinsics_601CD8[0x7b] = obj;
        obj->u.intrinsicid = 0x7b;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(TYPE(&stint128), GetHashNameNodeExport("_pmflo"), 0, 0);
        intrinsics_601CD8[0x7c] = obj;
        obj->u.intrinsicid = 0x7c;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(TYPE(&stint128), GetHashNameNodeExport("_pmfhl_lw"), 0, 0);
        intrinsics_601CD8[0x7d] = obj;
        obj->u.intrinsicid = 0x7d;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(TYPE(&stint128), GetHashNameNodeExport("_pmfhl_uw"), 0, 0);
        intrinsics_601CD8[0x7e] = obj;
        obj->u.intrinsicid = 0x7e;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(TYPE(&stint128), GetHashNameNodeExport("_pmfhl_slw"), 0, 0);
        intrinsics_601CD8[0x7f] = obj;
        obj->u.intrinsicid = 0x7f;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(TYPE(&stint128), GetHashNameNodeExport("_pmfhl_lh"), 0, 0);
        intrinsics_601CD8[0x80] = obj;
        obj->u.intrinsicid = 0x80;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(TYPE(&stint128), GetHashNameNodeExport("_pmfhl_sh"), 0, 0);
        intrinsics_601CD8[0x81] = obj;
        obj->u.intrinsicid = 0x81;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj =
            CParser_NewRTFunc(TYPE(&stvoid), GetHashNameNodeExport("_pmthi"), 0, 1, &stint128);
        intrinsics_601CD8[0x82] = obj;
        obj->u.intrinsicid = 0x82;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj =
            CParser_NewRTFunc(TYPE(&stvoid), GetHashNameNodeExport("_pmtlo"), 0, 1, &stint128);
        intrinsics_601CD8[0x83] = obj;
        obj->u.intrinsicid = 0x83;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj =
            CParser_NewRTFunc(TYPE(&stvoid), GetHashNameNodeExport("_pmthl_lw"), 0, 1, &stint128);
        intrinsics_601CD8[0x84] = obj;
        obj->u.intrinsicid = 0x84;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    fn_00467BB0(0x85, "_vabs", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467BB0(0x86, "_vmove", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467BB0(0x87, "_vmr32", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467AF0(0x88, "_vadd", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467AF0(0x89, "_vaddf", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128), TYPE(&stfloat));
    fn_00467A10(
        0x8a,
        "_vaddbc",
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128)
    );
    fn_00467A10(
        0x8b,
        "_vmadd",
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128)
    );
    fn_00467A10(
        0x8c,
        "_vmaddf",
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stfloat)
    );
    fn_004679A0(
        0x8d,
        "_vmaddbc",
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128)
    );
    fn_00467AF0(0x8e, "_vmax", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467AF0(0x8f, "_vmaxf", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128), TYPE(&stfloat));
    fn_00467A10(
        0x90,
        "_vmaxbc",
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128)
    );
    fn_00467AF0(0x91, "_vmini", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467AF0(0x92, "_vminif", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128), TYPE(&stfloat));
    fn_00467A10(
        0x93,
        "_vminibc",
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128)
    );
    fn_00467A10(
        0x94,
        "_vmsub",
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128)
    );
    fn_00467A10(
        0x95,
        "_vmsubf",
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stfloat)
    );
    fn_004679A0(
        0x96,
        "_vmsubbc",
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128)
    );
    fn_00467AF0(0x97, "_vmul", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467AF0(0x98, "_vmulf", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128), TYPE(&stfloat));
    fn_00467A10(
        0x99,
        "_vmulbc",
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128)
    );
    fn_00467AF0(0x9a, "_vsub", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467AF0(0x9b, "_vsubf", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128), TYPE(&stfloat));
    fn_00467A10(
        0x9c,
        "_vsubbc",
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128)
    );
    fn_00467BB0(0x9d, "_vopmula", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467AF0(
        0x9e, "_vopmsub", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128), TYPE(&stint128)
    );
    fn_00467A80(
        0x9f,
        "_vdiv",
        TYPE(&stfloat),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128)
    );
    fn_00467B50(0xa0, "_vsqrt", TYPE(&stfloat), TYPE(&stint128), TYPE(&stint128));
    fn_00467A80(
        0xa1,
        "_vrsqrt",
        TYPE(&stfloat),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128),
        TYPE(&stint128)
    );
    {
        Object* obj =
            CParser_NewRTFunc(TYPE(&stfloat), GetHashNameNodeExport("_vwaitq"), 0, 1, &stfloat);
        intrinsics_601CD8[0xa2] = obj;
        obj->u.intrinsicid = 0xa2;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(
            TYPE(&stint128),
            GetHashNameNodeExport("_vrget"),
            0,
            3,
            &stint128,
            &stsignedint,
            &stfloat
        );
        intrinsics_601CD8[0xa3] = obj;
        obj->u.intrinsicid = 0xa3;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(
            TYPE(&stfloat),
            GetHashNameNodeExport("_vrinit"),
            0,
            3,
            &stfloat,
            &stint128,
            &stsignedint
        );
        intrinsics_601CD8[0xa4] = obj;
        obj->u.intrinsicid = 0xa4;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(
            TYPE(&stint128),
            GetHashNameNodeExport("_vrnext"),
            0,
            3,
            &stint128,
            &stsignedint,
            &stfloat
        );
        intrinsics_601CD8[0xa5] = obj;
        obj->u.intrinsicid = 0xa5;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(
            TYPE(&stfloat), GetHashNameNodeExport("_vrxor"), 0, 3, &stfloat, &stint128, &stsignedint
        );
        intrinsics_601CD8[0xa6] = obj;
        obj->u.intrinsicid = 0xa6;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(
            TYPE(&stint128), GetHashNameNodeExport("_vclip"), 0, 2, &stint128, &stint128
        );
        intrinsics_601CD8[0xa7] = obj;
        obj->u.intrinsicid = 0xa7;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    fn_00467BB0(0xa8, "_vftoi0", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467BB0(0xa9, "_vftoi4", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467BB0(0xaa, "_vftoi12", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467BB0(0xab, "_vftoi15", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467BB0(0xac, "_vitof0", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467BB0(0xad, "_vitof4", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467BB0(0xae, "_vitof12", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    fn_00467BB0(0xaf, "_vitof15", TYPE(&stint128), TYPE(&stint128), TYPE(&stint128));
    {
        Object* obj = CParser_NewRTFunc(TYPE(&stint128), GetHashNameNodeExport("_vnop"), 0, 0);
        intrinsics_601CD8[0xb0] = obj;
        obj->u.intrinsicid = 0xb0;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(
            TYPE(&stfloat), GetHashNameNodeExport("_vgetf"), 0, 2, &stint128, &stint128
        );
        intrinsics_601CD8[0xb1] = obj;
        obj->u.intrinsicid = 0xb1;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(
            TYPE(&void_ptr), GetHashNameNodeExport("__alloca"), 0, 1, TYPE(&stunsignedlong)
        );
        intrinsics_601CD8[0] = obj;
        obj->u.intrinsicid = 0;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    fn_00467CD0(10, "__abs", TYPE(&stsignedint));
    fn_00467CD0(0xb, "__labs", TYPE(&stsignedlong));
    fn_00467CD0(0xc, "__s_abs", TYPE(&stfloat));
    fn_00467CD0(0xd, "__d_abs", TYPE(&stdouble));
    fn_00467C70(0xe, "__mulhi", TYPE(&stsignedlong));
    fn_00467C70(0xf, "__mulhiu", TYPE(&stsignedlong));
    fn_00467C70(8, "__strcpy", TYPE(&stchar));
    fn_00467C70(0x10, "__sqrt", TYPE(&stfloat));
    fn_00467C70(0x11, "__rsqrt", TYPE(&stfloat));
    {
        Object* obj = CParser_NewRTFunc(
            CDecl_NewPointerType(&stvoid),
            GetHashNameNodeExport("__memcpy"),
            0,
            3,
            CDecl_NewPointerType(&stvoid),
            CDecl_NewPointerType(&stvoid),
            CABI_GetSizeTType()
        );
        intrinsics_601CD8[9] = obj;
        obj->u.intrinsicid = 9;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    {
        Object* obj = CParser_NewRTFunc(
            TYPE(&void_ptr),
            GetHashNameNodeExport("__clear"),
            0,
            2,
            TYPE(&char_ptr),
            CABI_GetSizeTType()
        );
        intrinsics_601CD8[0x12] = obj;
        obj->u.intrinsicid = 0x12;
        TYPE_FUNC(obj->type)->flags |= 0x200;
        CScope_AddGlobalObject(obj);
    }
    copts.cplusplus = saved;
}
