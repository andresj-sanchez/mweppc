#include "compiler/CParser.h"
#include "compiler/Alias.h"
#define CERROR_FILE "PCodeUtilities.c"
#include "compiler/PCodeUtilities.h"
#include "compiler/CError.h"
#include "compiler/PCodeInfo.h"
#include "compiler/RegisterInfo.h"
#include "compiler/CompilerTools.h"
#include "compiler/CodeGen.h"
#include <string.h>
#include "compiler/objects.h"
#include "compiler/VReg.h"

#pragma defer_codegen off
#pragma opt_propagation off
void branch_always(PCodeLabel* target) {
    short op = PC_B;
    emitpcode(op, target);
    pcaddedge(pclastblock, target, 0);
    makepcblock();
}
#pragma opt_propagation on
#pragma defer_codegen on

void fn_0048F530(int opcode, short dstReg, short srcReg, SInt32 value) {
    PCode* pc;
    PCodeArg arg;

    pc = make_instruction(opcode, 3);
    pc->args[0].kind = PCOp_REGISTER;
    pc->args[0].arg = 0;
    pc->args[0].data.reg.reg = dstReg;
    pc->args[0].effect = 2;
    pc->args[1].kind = PCOp_REGISTER;
    pc->args[1].arg = 0;
    pc->args[1].data.reg.reg = srcReg;
    pc->args[1].effect = 1;
    arg = arg_from_int(value);
    pc->args[2] = arg;
    appendpcode(pclastblock, pc);
}

PCode* make_instruction(int opcode, short argCount) {
    int count;
    PCodeArg implicit[20];
    UInt32 flags;
    PCode* pc;
    int i;

    count = 0;
    flags = opcode_info[opcode].flags2;
    if (flags & 0x20000) {
        implicit[count].kind = PCOp_REGISTER;
        implicit[count].arg = 2;
        implicit[count].effect = 2;
        implicit[count].data.reg.reg = 0;
        count++;
    }
    if (flags & 0x80000) {
        implicit[count].kind = PCOp_REGISTER;
        implicit[count].arg = 2;
        implicit[count].effect = 2;
        implicit[count].data.reg.reg = 1;
        count++;
    }
    if (flags & 0x10000) {
        implicit[count].kind = PCOp_REGISTER;
        implicit[count].arg = 2;
        implicit[count].effect = 1;
        implicit[count].data.reg.reg = 0;
        count++;
    }
    if (flags & 0x40000) {
        implicit[count].kind = PCOp_REGISTER;
        implicit[count].arg = 2;
        implicit[count].effect = 1;
        implicit[count].data.reg.reg = 1;
        count++;
    }
    if (flags & 0x800) {
        implicit[count].kind = PCOp_REGISTER;
        implicit[count].arg = 0;
        implicit[count].effect = 2;
        implicit[count].data.reg.reg = 31;
        count++;
    }
    if (flags & 0x2000) {
        implicit[count].kind = PCOp_REGISTER;
        implicit[count].arg = 2;
        implicit[count].effect = 1;
        implicit[count].data.reg.reg = 4;
        count++;
    }
    if (flags & 0x1000) {
        implicit[count].kind = PCOp_REGISTER;
        implicit[count].arg = 2;
        implicit[count].effect = 2;
        implicit[count].data.reg.reg = 4;
        count++;
    }
    if (flags & 0x8000) {
        implicit[count].kind = PCOp_REGISTER;
        implicit[count].arg = 2;
        implicit[count].effect = 1;
        implicit[count].data.reg.reg = 3;
        count++;
    }
    if (flags & 0x8000) {
        implicit[count].kind = PCOp_REGISTER;
        implicit[count].arg = 2;
        implicit[count].effect = 1;
        implicit[count].data.reg.reg = 5;
        count++;
    }
    if (flags & 0x4000) {
        implicit[count].kind = PCOp_REGISTER;
        implicit[count].arg = 2;
        implicit[count].effect = 2;
        implicit[count].data.reg.reg = 3;
        count++;
    }
    if (flags & 0x4000) {
        implicit[count].kind = PCOp_REGISTER;
        implicit[count].arg = 2;
        implicit[count].effect = 2;
        implicit[count].data.reg.reg = 5;
        count++;
    }

    pc = lalloc(sizeof(PCode) + sizeof(PCodeArg) * ((short)argCount + count));
    memset(pc, 0, sizeof(PCode) + sizeof(PCodeArg) * ((short)argCount + count));
    pc->argCount = (short)argCount + count;
    pc->op = opcode;
    pc->sourceoffset = asm_source_offset_6361A0;
    if (count != 0) {
        i = ((short)argCount + count) - 1;
        do {
            pc->args[i] = implicit[count - 1];
            i--;
            count--;
        } while (count != 0);
    }
    pc->flags = opcode_info[opcode].flags;
    return pc;
}

PCode* fn_0048F5D0(short opcode, short destReg, short baseReg, Object* object, int offset) {
    PCode* pc;
    PCodeArg output;
    PCodeArg base;

    pc = make_instruction(opcode, 3);
    output.kind = PCOp_REGISTER;
    output.arg = 1;
    output.data.reg.reg = destReg;
    pc->args[0] = output;
    pc->args[0].effect = 1;
    pc->args[1].kind = 4;
    pc->args[1].data.mem.offset = offset;
    pc->args[1].data.mem.obj = object;
    base.kind = PCOp_REGISTER;
    base.arg = 0;
    base.effect = 1;
    base.data.reg.reg = baseReg;
    pc->args[2] = base;
    if (object) {
        if (object->datatype == DLOCAL) {
            if (baseReg == stack_pointer_register) {
                pc->alias = make_alias(
                    object, offset - object->u.unk30, nbytes_loaded_or_stored_by(pc->op)
                );
            } else {
                pc->alias = make_alias(object, 0, 0);
            }
        } else if (object->datatype == DDATA) {
            pc->alias = make_alias(object, 0, 0);
        } else {
            CError_Internal("PCodeUtilities.c", 261);
        }
    }
    if (!pc->alias) {
        pc->flags |= 0x20;
    }
    appendpcode(pclastblock, pc);
    return pc;
}

PCode* fn_0048F710(short opcode, short destReg, short baseReg, Object* object, int offset) {
    PCode* pc;
    PCodeArg output;
    PCodeArg base;
    pc = make_instruction(opcode, 3);
    output.kind = PCOp_REGISTER;
    output.arg = 0;
    output.data.reg.reg = destReg;
    pc->args[0] = output;
    pc->args[0].effect = 1;
    pc->args[1].kind = 4;
    pc->args[1].data.mem.offset = offset;
    pc->args[1].data.mem.obj = object;
    base.kind = PCOp_REGISTER;
    base.arg = 0;
    base.effect = 1;
    base.data.reg.reg = baseReg;
    pc->args[2] = base;
    if (object) {
        if (object->datatype == DLOCAL) {
            if (baseReg == stack_pointer_register) {
                pc->alias = make_alias(
                    object, offset - object->u.unk30, nbytes_loaded_or_stored_by(pc->op)
                );
            } else {
                pc->alias = make_alias(object, 0, 0);
            }
        } else if (object->datatype == DDATA) {
            pc->alias = make_alias(object, 0, 0);
        } else {
            CError_Internal("PCodeUtilities.c", 222);
        }
    }
    if (!pc->alias) {
        pc->flags |= 0x20;
    }
    appendpcode(pclastblock, pc);
    return pc;
}

void fn_0048F850(short opcode, short destReg, short baseReg, Object* object, int offset) {
    PCode* pc;
    UInt16 specialOpcode;

    pc = make_instruction(opcode, 3);
    pc->args[0].kind = PCOp_REGISTER;
    pc->args[0].arg = 1;
    pc->args[0].data.reg.reg = destReg;
    pc->args[0].effect = 2;
    if (copts.unknown0C < 2 || !(copts.fp_library & 2)) {
        specialOpcode = 0x452;
    } else {
        specialOpcode = 0x455;
    }
    if (opcode == specialOpcode) {
        pc->args[0].effect |= 0x8000;
        if (copts.unknown1A) {
            pc->args[0].effect |= 0x20;
        }
    }
    pc->args[1].kind = 4;
    pc->args[1].data.mem.obj = object;
    pc->args[1].data.mem.offset = offset;
    pc->args[2].kind = PCOp_REGISTER;
    pc->args[2].arg = 0;
    pc->args[2].data.reg.reg = baseReg;
    pc->args[2].effect = 1;
    if (object) {
        if (object->datatype == DLOCAL && baseReg != stack_pointer_register) {
            pc->alias = make_alias(object, 0, 0);
        } else if (object->datatype != DLOCAL) {
            if (object->datatype != DDATA) {
                CError_Internal("PCodeUtilities.c", 148);
            }
            pc->alias = make_alias(object, 0, 0);
        } else {
            pc->alias =
                make_alias(object, offset - object->u.unk30, nbytes_loaded_or_stored_by(pc->op));
        }
    }
    if (object && is_volatile_object(object)) {
        pc->flags |= 0x80;
    }
    if (object && is_const_object(object)) {
        pc->flags |= 0x40;
    }
    appendpcode(pclastblock, pc);
}

void fn_0048F990(short opcode, short destReg, short baseReg, Object* object, int offset) {
    PCode* pc;
    PCodeArg output;
    PCodeArg base;
    VarInfo* location;
    short size;
    UInt16 specialOpcode;

    if (object && object->datatype == DLOCAL) {
        location = (VarInfo*)Registers_GetVarInfo(object);
    } else {
        location = NULL;
    }
    if (location && (size = (UInt16)location->reg) != 0) {
        SInt32 wideOpcode = opcode;
        if (wideOpcode != 0x467 && wideOpcode != 0x1D1) {
            emitpcode(move_opcode, destReg, size);
        } else {
            emitpcode(move_opcode_637350, destReg, size);
        }
        goto done;
    }

    pc = make_instruction(opcode, 3);
    output.kind = PCOp_REGISTER;
    output.arg = 0;
    output.data.reg.reg = destReg;
    pc->args[0] = output;
    pc->args[0].effect = 2;
    if (copts.unknown0C < 2 || !(copts.fp_library & 2)) {
        specialOpcode = 0x452;
    } else {
        specialOpcode = 0x455;
    }
    if (opcode == specialOpcode) {
        pc->args[0].effect |= 0x8000;
        if (copts.unknown1A) {
            pc->args[0].effect |= 0x20;
        }
    }
    pc->args[1].kind = 4;
    pc->args[1].data.mem.obj = object;
    pc->args[1].data.mem.offset = offset;
    base.kind = PCOp_REGISTER;
    base.arg = 0;
    base.data.reg.reg = baseReg;
    pc->args[2] = base;
    pc->args[2].effect = 1;
    if (object) {
        if (object->datatype == DLOCAL &&
            (UInt16)pc->args[2].data.reg.reg != (UInt16)stack_pointer_register) {
            pc->alias = make_alias(object, 0, 0);
        } else if (object->datatype == DLOCAL) {
            pc->alias =
                make_alias(object, offset - object->u.unk30, nbytes_loaded_or_stored_by(pc->op));
        } else if (object->datatype == DDATA) {
            pc->alias = make_alias(object, 0, 0);
        } else {
            CError_Internal("PCodeUtilities.c", 96);
        }
    }
    if (!pc->alias) {
        pc->flags |= 0x20;
    }
    if (object && is_volatile_object(object)) {
        pc->flags |= 0x80;
    }
    if (object && is_const_object(object)) {
        pc->flags |= 0x40;
    }
    appendpcode(pclastblock, pc);
done:
    return;
}

void note_object_alignment(int align, Object* operand) {
    PCode* pc;
    PCode* newpc;
    UInt16 opcode;
    SInt32 alignment;
    UInt16 firstop;
    UInt16 nextop;

    pc = pclastblock->lastPCode;
    opcode = pc->op;
    alignment = align;
    switch (opcode) {
        case 0x467:
            CError_FATAL(577);
            break;
        case 0x454:
            if ((UInt16)pc->args[2].data.reg.reg == (UInt16)stack_pointer_register ||
                pc->args[2].data.reg.reg == 28) {
                return;
            }
            if (alignment < 8) {
                if (copts.littleendian) {
                    firstop = 0x459;
                    nextop = 0x458;
                } else {
                    firstop = 0x458;
                    nextop = 0x459;
                }
                pc->op = firstop;
                newpc = make_instruction(nextop, 3);
                newpc->flags = pc->flags;
                newpc->args[0] = pc->args[0];
                newpc->args[0].effect |= 1;
                newpc->args[1] = pc->args[1];
                newpc->args[2] = pc->args[2];
                newpc->alias = pc->alias;
                insertpcodeafter(pc, newpc);
                newpc->args[1].data.mem.offset += 7;
            }
            break;
        case 0x45F:
            if ((UInt16)pc->args[2].data.reg.reg == (UInt16)stack_pointer_register ||
                pc->args[2].data.reg.reg == 28) {
                return;
            }
            if (alignment < 4) {
                if (copts.littleendian) {
                    firstop = 0x465;
                    nextop = 0x464;
                } else {
                    firstop = 0x464;
                    nextop = 0x465;
                }
                pc->op = firstop;
                newpc = make_instruction(nextop, 3);
                newpc->flags = pc->flags;
                newpc->args[0] = pc->args[0];
                newpc->args[0].effect |= 1;
                newpc->args[1] = pc->args[1];
                newpc->args[2] = pc->args[2];
                newpc->alias = pc->alias;
                insertpcodeafter(pc, newpc);
                newpc->args[1].data.mem.offset += 3;
            }
            break;
    }
}
