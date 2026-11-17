#include "compiler/CodeGen.h"
#include "compiler/PCodeInfo.h"
#include "compiler/common.h"
#include "compiler/PCode.h"
#include "compiler/objects.h"
#include "compiler/Registers.h"
#include "compiler/BitVectors.h"
#include "compiler/Alias.h"
#include "compiler/AliasPropagation.h"
#include "compiler/CompilerTools.h"
#include "compiler/CParser.h"
#include "compiler/RegisterInfo.h"
#include "compiler/CopyPropagation.h"
#include "compiler/Peephole.h"

void propagatealiasinfo(void* arg) {
    SInt32 level;

    if (copts.optimizationlevel >= 4) {
        level = 4;
    } else {
        level = 1;
    }
    fn_005553A0(arg, alias_propagation_name, level, 1);
}

void finishpropagatealiases(void) {
    alias_propagation_enabled = TRUE;
}

#include "compiler/UseDefChains.h"

SInt32 addresspropagatestouse(register SInt32 candidateID, SInt32 useID) {
    PCode* candidate_pcode;
    PCode* use_pcode;
    Alias* alias;
    SInt32 reg;
    UInt32 flags;

    candidate_pcode = Candidates[candidateID].pcode;
    use_pcode = Uses[useID].pcode;
    if (candidate_pcode->op == 0x3AA &&
        (UInt16)candidate_pcode->args[2].data.reg.reg == 0x1D &&
        candidate_pcode->args[1].kind == PCOp_IMMEDIATE &&
        candidate_pcode->args[1].data.imm.value == 0) {
        return FALSE;
    }
    reg = candidate_pcode->args[0].data.reg.reg;
    if (!fn_0054B940(
            candidate_pcode,
            candidate_pcode->defID,
            reg,
            use_pcode,
            usedefinfo[use_pcode->block->blockIndex].defin,
            0
        )) {
        return TRUE;
    }
    alias = fn_0054B2E0(candidate_pcode);
    if (alias) {
        if (alias != worst_case_ptr) {
            flags = use_pcode->flags;
            if (!(flags & 4) ||
                use_pcode->args[0].kind ||
                use_pcode->args[0].arg ||
                use_pcode->args[0].data.reg.reg != reg) {
                if (!(flags & 6) && use_pcode->args[0].data.reg.reg < n_real_registers[0]) {
                    fn_00490B90(alias);
                }
            } else {
                fn_00490B90(alias);
            }
        }
        fn_0054AEE0(use_pcode, reg, alias);
    }
    return TRUE;
}

#include "compiler/CError.h"
#include "compiler/InstrSelection.h"

Alias* fn_0054B360(Operand* op, PCode* instr) {
    Object* object;
    Alias* alias;

    alias = NULL;
    if (!op->object) {
        CError_Internal(alias_source_file, 37);
    }
    object = op->object;
    if (object->datatype == DLOCAL) {
        if (op->reg != stack_pointer_register) {
            alias = make_alias(object, 0, 0);
        } else {
            alias = make_alias(
                object,
                instr->args[1].data.mem.offset - instr->args[1].data.mem.obj->u.unk30,
                nbytes_loaded_or_stored_by(instr->op)
            );
        }
    } else if (object->datatype == DDATA) {
        alias = make_alias(
            op->object, instr->args[1].data.mem.offset, nbytes_loaded_or_stored_by(instr->op)
        );
    } else if (object->datatype == DFUNC) {
        alias = make_alias(
            op->object, instr->args[1].data.mem.offset, nbytes_loaded_or_stored_by(instr->op)
        );
    } else {
        CError_Internal(alias_source_file, 57);
    }
    return alias;
}

#pragma dont_inline on
Alias* fn_0054B2E0(PCode* pcode) {
    SInt32 disp;
    Object* address;
    Alias* result;

    if (pcode->argCount > 2 && pcode->args[1].kind == PCOp_MEMORY) {
        address = pcode->args[1].data.mem.obj;
        if (address) {
            result = pcode->alias;
            disp = pcode->args[1].data.mem.offset;
            if (!result) {
                result = make_alias(address, disp, 1);
                pcode->alias = result;
            }
            return result;
        }
    }
    return pcode->alias;
}

#pragma dont_inline reset

SInt32 is_address_load(PCode* pcode) {
    switch (pcode->op) {
        case 3:
        case 4:
        case 0x78:
        case 0x79:
        case 0xAA:
        case 0xAB:
        case 0x44E:
            return TRUE;
        default:
            return FALSE;
    }
}

void fn_0054AEE0(PCode* use_pcode, SInt32 reg, Alias* reference) {
    PCode* instr;
    Alias* node;
    Object* object;
    Alias* target;
    UInt16 opcode;
    UInt16 count;
    UInt32 flags;
    SInt32 size;
    SInt32 displacement;
    SInt32 i;
    PCodeArg* arg;

    instr = use_pcode;
    node = reference;
    opcode = instr->op;
    flags = 1;
    displacement = 0;
    if (instr->flags & 6) {
        flags = nbytes_loaded_or_stored_by(instr->op);
    }
    if (opcode == move_opcode || opcode == 0x1CC || opcode == 0x2AF) {
        return;
    }
    count = (UInt16)instr->argCount;
    if (count < 3) {
        return;
    }
    if (opcode == 0x44E) {
        if (instr->args[1].kind == 4) {
            object = instr->args[1].data.mem.obj;
            if (object) {
                if (object != node->object) {
                    CError_Internal(alias_source_file, 175);
                }
                return;
            }
        }
        displacement = instr->args[1].data.mem.offset;
    } else if (opcode == 4) {
        displacement = instr->args[1].data.reg.reg;
        if (displacement != 0 || (instr->args[1].effect & 1)) {
            displacement = 0;
            flags = 0;
        }
    } else if (instr->flags & 6) {
        if (instr->args[2].data.reg.reg != reg) {
            return;
        }
        if (instr->args[1].kind == 2 || instr->args[1].kind == 4) {
            displacement = instr->args[1].data.mem.offset;
        } else if (instr->args[1].kind == 6) {
            return;
        } else {
            CError_Internal(alias_source_file, 241);
            return;
        }
    } else {
        if (instr->args[2].kind == 2) {
            if (instr->args[1].kind || instr->args[1].arg || instr->args[1].data.reg.reg != reg) {
                return;
            }
            if (opcode == 3) {
                displacement = instr->args[2].data.imm.value;
            } else if (opcode == 0x90) {
                displacement = instr->args[2].data.imm.value << 16;
            } else {
                fn_00490570(worst_case_ptr, node);
                return;
            }
        } else if (instr->args[2].kind == 3) {
            if (instr->args[1].kind || instr->args[1].arg || instr->args[1].data.reg.reg != reg) {
                return;
            }
            switch (opcode) {
                case 3:
                case 4:
                    displacement = instr->args[2].data.cint.lo;
                    break;
                default:
                    displacement = instr->args[1].data.mem.offset;
                    break;
            }
        } else {
            size = instr->argCount;
            arg = instr->args;
            for (i = 0; i < size; i++, arg++) {
                if (arg->kind == PCOp_REGISTER &&
                    arg->arg == 0 &&
                    (arg->effect & 2) &&
                    arg->data.reg.reg == reg) {
                    fn_00490570(worst_case_ptr, node);
                    return;
                }
            }
            return;
        }
    }
    if (node->type <= AliasType_ObjectPart) {
        node = make_displ_alias(node, displacement, flags);
    }
    target = ((PCode*)instr)->alias;
    if (!target) {
        ((PCode*)instr)->alias = node;
        if (opcode == 3 ||
            opcode == 4 ||
            opcode == move_opcode ||
            opcode == 0x1CC ||
            opcode == 0x2AF) {
            alias_graph_dirty = 1;
        }
        alias_propagation_enabled = 1;
    } else if (target == worst_case_ptr) {
        fn_00490570(worst_case_ptr, node);
        alias_propagation_enabled = 1;
    } else if (target != node) {
        if (target->type <= AliasType_ObjectPart && node->type <= AliasType_ObjectPart) {
            if (target->object != node->object) {
                ((PCode*)instr)->alias = fn_00490600(node, target);
            } else if (target->offset != node->offset || target->size != node->size) {
                ((PCode*)instr)->alias = make_alias(node->object, 0, 0);
            }
        } else {
            ((PCode*)instr)->alias = fn_00490600(node, target);
        }
        alias_propagation_enabled = 1;
    }
}

void fn_0054AC50(PCode* first, PCode* second) {
    Alias* alias;
    Object* address;
    Object* object;
    SInt32 offset;
    UInt16 reg;
    UInt32 flags;

    if (!first || !second) {
        return;
    }
    if (second->op == 0x44E) {
        flags = first->flags;
        if ((flags & 6) && (flags & 0x20)) {
            if (first->args[1].kind != PCOp_MEMORY) {
                return;
            }
            alias = second->alias;
            if (alias->type != AliasType_ObjectPart) {
                return;
            }
            address = second->args[1].data.mem.obj;
            if (!address || !address->name) {
                return;
            }
            if ((UInt16)second->args[0].data.reg.reg != (UInt16)first->args[2].data.reg.reg) {
                return;
            }
            reg = second->args[2].data.reg.reg;
            if (reg != (UInt16)stack_pointer_register &&
                (SInt32)(SInt16)reg != 28 &&
                reg != 0 &&
                (SInt32)(SInt16)reg != 23) {
                return;
            }
            if (first->args[1].data.mem.obj != NULL &&
                first->args[1].data.mem.obj != second->args[1].data.mem.obj) {
                return;
            }
            offset = first->args[1].data.mem.offset + alias->offset;
            if (first->op <= 0x40A) {
                return;
            }
            if (copts.debuglisting) {
                fn_004BB070(second, first);
            }
            object = alias->object;
            if (object) {
                first->args[1].data.mem.offset = offset + object->u.unk30;
            }
            object = alias->object;
            if (object->datatype == DLOCAL) {
                first->args[1].data.mem.offset = offset + object->u.unk30;
            } else {
                if (object->datatype != DDATA) {
                    CError_Internal(alias_source_file, 586);
                }
                first->args[1].data.mem.offset = offset;
            }
            first->args[1].data.mem.obj = second->args[1].data.mem.obj;
            first->args[2].data.reg.reg = second->args[2].data.reg.reg;
            first->flags &= ~0x20;
            if (copts.debuglisting) {
                fn_004BB060(first);
            }
        }
    }
}
