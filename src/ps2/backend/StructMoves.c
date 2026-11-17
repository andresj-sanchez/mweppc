#include "compiler/StructMoves.h"
#include "compiler/Registers.h"
#include "compiler/ObjGen.h"
#include "compiler/PCodeInfo.h"
#include "compiler/InstrSelection.h"
#include "compiler/mips.h"
#include "compiler/Operands.h"
#include "compiler/CError.h"
#include "compiler/types.h"
#include "compiler/CodeGen.h"
#include "compiler/CompilerTools.h"
#include "compiler/PCodeUtilities.h"
#include "compiler/CParser.h"
#include "compiler/VReg.h"

void fn_0053F420(Operand* dst, Operand* src, int size) {
    UInt16 loadOpcode;
    UInt16 storeOpcode;
    Operand value;

    if (size == 0) {
        return;
    }
    if (size == 1) {
        loadOpcode = 0x453;
        storeOpcode = 0x47F;
    } else if (size == 2) {
        loadOpcode = 0x45B;
        storeOpcode = 0x480;
    } else if (size == 3) {
        fn_0053F420(dst, src, 2);
        dst->immOffset += 2;
        src->immOffset += 2;
        fn_0053F420(dst, src, 1);
        return;
    } else {
        loadOpcode = 0x45F;
        storeOpcode = 0x481;
    }
    value = ((Operand (*)(UInt16, Operand*, char))emit_operand_to_register)(loadOpcode, src, 0);
    fn_00486660(storeOpcode, &value, dst);
}

void fn_0053F4D0(Operand* op) {
    ObjGenRegisterLocation location;
    UInt8 kind = op->kind;
    if (kind == 0) {
        if (op->kind != 0) {
            CError_Internal("StructMoves.c", 0x37);
        }
        op->kind = 2;
        op->object = NULL;
        op->unk20 = 0;
        op->immOffset = 0;
        op->immediate = cint64_zero;
    } else if (kind == 2) {
        if (op->object) {
            fn_0046EDD0(op->object, &location);
            if (location.number == 0x1C && location.isDirect != 0) {
                op->kind = 4;
                return;
            }
        }
        kind = op->kind;
        if (kind == 2) {
            if (kind != 0) {
                Coerce_to_register(op, TYPE(&void_ptr), 0);
            }
        } else if (kind == 4) {
            if (kind != 0) {
                Coerce_to_register(op, TYPE(&void_ptr), 0);
            }
            if (op->kind != 0) {
                CError_Internal("StructMoves.c", 0x37);
            }
            op->kind = 2;
            op->object = NULL;
            op->unk20 = 0;
            op->immOffset = 0;
            op->immediate = cint64_zero;
        } else if (kind == 3) {
            if (kind != 0) {
                Coerce_to_register(op, TYPE(&void_ptr), 0);
            }
            if (op->kind != 0) {
                CError_Internal("StructMoves.c", 0x37);
            }
            op->kind = 2;
            op->object = NULL;
            op->unk20 = 0;
            op->immOffset = 0;
            op->immediate = cint64_zero;
        }

        if (op->kind != 0) {
            CError_Internal("StructMoves.c", 0x37);
        }
        op->kind = 2;
        op->object = NULL;
        op->unk20 = 0;
        op->immOffset = 0;
        op->immediate = cint64_zero;
    } else if (kind == 4) {
        if (kind != 0) {
            Coerce_to_register(op, TYPE(&void_ptr), 0);
        }
        if (op->kind != 0) {
            CError_Internal("StructMoves.c", 0x37);
        }
        op->kind = 2;
        op->object = NULL;
        op->unk20 = 0;
        op->immOffset = 0;
        op->immediate = cint64_zero;
    } else if (kind == 3) {
        if (kind != 0) {
            Coerce_to_register(op, TYPE(&void_ptr), 0);
        }
        if (op->kind != 0) {
            CError_Internal("StructMoves.c", 0x37);
        }
        op->kind = 2;
        op->object = NULL;
        op->unk20 = 0;
        op->immOffset = 0;
        op->immediate = cint64_zero;
    } else {
        CError_Internal("StructMoves.c", 0x72);
    }
}

void fn_0053CC40(Operand* dst, Operand* src, SInt32 size, SInt32 alignment) {
    Operand destinationCopy;
    Operand sourceCopy;
    Operand byteDestination;
    Operand byteSource;
    Operand halfDestination;
    Operand halfSource;
    Operand wordDestination;
    Operand wordSource;
    Operand doubleDestination;
    Operand doubleSource;
    Operand quadDestination;
    Operand quadSource;
    SInt32 inlineThreshold;
    SInt32 limit;
    UInt16 opcode;

    sourceCopy = *src;
    destinationCopy = *dst;
    inlineThreshold = copts.optimizesize ? 1 : 3;
    if (src->kind == 1) {
        if (size == 4) {
            opcode = 0x483;
        } else if (copts.unknown0C < 2 || !(copts.fp_library & 2)) {
            opcode = 0x476;
        } else {
            opcode = 0x47A;
        }
        fn_00486660(opcode, src, dst);
        return;
    }
    if (!copts.unknown41 || alignment < 16) {
        if (copts.unknown0C < 3 || alignment < 8) {
            if (alignment < 4) {
                if (alignment == 2) {
                    limit = copts.unknown33 * 2;
                    if (size <= limit) {
                        fn_0053EFB0(&destinationCopy, &sourceCopy, size);
                    } else {
                        halfDestination = destinationCopy;
                        halfSource = sourceCopy;
                        fn_0053D440(
                            0x45B,
                            0x480,
                            2,
                            &halfDestination,
                            &halfSource,
                            &destinationCopy,
                            &sourceCopy,
                            size
                        );
                        fn_0053F420(&halfDestination, &halfSource, size & 1);
                    }
                } else if (copts.unknown33 < size) {
                    byteDestination = destinationCopy;
                    byteSource = sourceCopy;
                    fn_0053D440(
                        0x45A,
                        0x47F,
                        1,
                        &byteDestination,
                        &byteSource,
                        &destinationCopy,
                        &sourceCopy,
                        size
                    );
                } else {
                    fn_0053F230(&destinationCopy, &sourceCopy, size);
                }
            } else {
                limit = copts.unknown33 * inlineThreshold * 4;
                if (size <= limit) {
                    fn_0053EB60(&destinationCopy, &sourceCopy, size);
                } else {
                    wordDestination = destinationCopy;
                    wordSource = sourceCopy;
                    fn_0053D440(
                        0x45F,
                        0x481,
                        4,
                        &wordDestination,
                        &wordSource,
                        &destinationCopy,
                        &sourceCopy,
                        size
                    );
                    fn_0053F420(&wordDestination, &wordSource, size & 3);
                }
            }
        } else {
            limit = copts.unknown33 * inlineThreshold * 8;
            if (size <= limit) {
                fn_0053E560(&destinationCopy, &sourceCopy, size);
            } else {
                doubleDestination = destinationCopy;
                doubleSource = sourceCopy;
                fn_0053D440(
                    0x454,
                    0x479,
                    8,
                    &doubleDestination,
                    &doubleSource,
                    &destinationCopy,
                    &sourceCopy,
                    size
                );
                fn_0053CC40(&doubleDestination, &doubleSource, size & 7, 8);
            }
        }
    } else {
        limit = copts.unknown33 * inlineThreshold * 8;
        if (size <= limit) {
            fn_0053DE80(&destinationCopy, &sourceCopy, size);
        } else {
            quadDestination = destinationCopy;
            quadSource = sourceCopy;
            fn_0053D440(
                0x467, 0x489, 16, &quadDestination, &quadSource, &destinationCopy, &sourceCopy, size
            );
            fn_0053CC40(&quadDestination, &quadSource, size & 15, 16);
        }
    }
}

void fn_0053D440(
    UInt16 loadOpcode,
    UInt16 storeOpcode,
    SInt32 step,
    Operand* output,
    Operand* input,
    Operand* destination,
    Operand* source,
    SInt32 size
) {
    CLabel* label;
    SInt32 srcReg;
    SInt32 dstReg;
    SInt32 counterReg;
    SInt32 tempReg;
    SInt32 highReg;
    SInt32 count;
    Boolean split;
    UInt8 kind;
    PCode* pc;
    Operand counter;
    Operand sourceMemory;
    Operand destinationMemory;

    label = galloc(0x14);
    srcReg = (*(volatile int*)&used_virtual_registers[RegClass_GPR])++;
    dstReg = (*(volatile int*)&used_virtual_registers[RegClass_GPR])++;
    counterReg = (*(volatile int*)&used_virtual_registers[RegClass_GPR])++;
    tempReg = (*(volatile int*)&used_virtual_registers[RegClass_GPR])++;
    label->pclabel = makepclabel();
    sourceMemory.kind = 2;
    sourceMemory.immOffset = 0;
    sourceMemory.object = NULL;
    destinationMemory.kind = 2;
    destinationMemory.immOffset = 0;
    destinationMemory.object = NULL;
    input->reg = srcReg;
    input->immOffset = 0;
    input->object = NULL;
    input->unk20 = 0;
    output->reg = dstReg;
    output->immOffset = 0;
    output->object = NULL;
    output->unk20 = 0;
    kind = source->kind;
    if (kind == 2) {
        if (kind != 0) {
            Coerce_to_register(source, TYPE(&void_ptr), 0);
        }
    } else if (kind == 4) {
        if (kind != 0) {
            Coerce_to_register(source, TYPE(&void_ptr), 0);
        }
        if (source->kind != 0) {
            CError_Internal("StructMoves.c", 0x37);
        }
        source->kind = 2;
        source->object = NULL;
        source->unk20 = 0;
        source->immOffset = 0;
        source->immediate = cint64_zero;
    } else if (kind == 3) {
        if (kind != 0) {
            Coerce_to_register(source, TYPE(&void_ptr), 0);
        }
        if (source->kind != 0) {
            CError_Internal("StructMoves.c", 0x37);
        }
        source->kind = 2;
        source->object = NULL;
        source->unk20 = 0;
        source->immOffset = 0;
        source->immediate = cint64_zero;
    }
    kind = destination->kind;
    if (kind == 2) {
        if (kind != 0) {
            Coerce_to_register(destination, TYPE(&void_ptr), 0);
        }
    } else if (kind == 4) {
        if (kind != 0) {
            Coerce_to_register(destination, TYPE(&void_ptr), 0);
        }
        if (destination->kind != 0) {
            CError_Internal("StructMoves.c", 0x37);
        }
        destination->kind = 2;
        destination->object = NULL;
        destination->unk20 = 0;
        destination->immOffset = 0;
        destination->immediate = cint64_zero;
    } else if (kind == 3) {
        if (kind != 0) {
            Coerce_to_register(destination, TYPE(&void_ptr), 0);
        }
        if (destination->kind != 0) {
            CError_Internal("StructMoves.c", 0x37);
        }
        destination->kind = 2;
        destination->object = NULL;
        destination->unk20 = 0;
        destination->immOffset = 0;
        destination->immediate = cint64_zero;
    }
    count = size / step;
    split = count >= 6 && !(count & 1);
    if (split) {
        highReg = used_virtual_registers[RegClass_GPR]++;
        count = count / 2;
    }
    counter = make_constant_operand(count);
    emitpcode(move_opcode, counterReg, counter.reg);
    counter.reg = counterReg;
    emitpcode(move_opcode, srcReg, source->reg);
    emitpcode(move_opcode, dstReg, destination->reg);
    fn_0049BA60();
    emit_label(label->pclabel);
    pc = make_instruction(loadOpcode, 3);
    pc->flags |= 0x20;
    pc->args[0] = fn_004D02D0(tempReg, 0, 0x12);
    pc->args[1] = arg_from_operand(&sourceMemory);
    pc->args[2] = fn_004D02D0(srcReg, 0, 0x11);
    if (pc->op == 0x467 || pc->op == 0x1D1) {
        pc->args[0].effect |= 0x2000;
    }
    appendpcode(pclastblock, pc);
    if (split) {
        pc = make_instruction(loadOpcode, 3);
        pc->flags |= 0x20;
        pc->args[0] = fn_004D02D0(highReg, 0, 0x12);
        if (pc->op == 0x467 || pc->op == 0x1D1) {
            pc->args[0].effect |= 0x2000;
        }
        sourceMemory.immOffset += step;
        pc->args[1] = arg_from_operand(&sourceMemory);
        pc->args[2] = fn_004D02D0(srcReg, 0, 0x11);
        appendpcode(pclastblock, pc);
    }
    pc = makepcode(3, input->reg, input->reg, split ? step * 2 : step);
    pc->args[0].effect |= 0x10;
    pc->args[1].effect |= 0x10;
    appendpcode(pclastblock, pc);
    pc = makepcode(3, counter.reg, counter.reg, -1);
    pc->args[0].effect |= 0x10;
    pc->args[1].effect |= 0x10;
    appendpcode(pclastblock, pc);
    pc = make_instruction(storeOpcode, 3);
    pc->flags |= 0x20;
    pc->args[0] = fn_004D02D0(tempReg, 0, 0x11);
    if (pc->op == 0x489 || pc->op == 0x1D2) {
        pc->args[0].effect |= 0x2000;
    }
    pc->args[1] = arg_from_operand(&destinationMemory);
    pc->args[2] = fn_004D02D0(dstReg, 0, 0x11);
    appendpcode(pclastblock, pc);
    if (split) {
        pc = make_instruction(storeOpcode, 3);
        pc->flags |= 0x20;
        pc->args[0] = fn_004D02D0(highReg, 0, 0x11);
        if (pc->op == 0x467 || pc->op == 0x1D1) {
            pc->args[0].effect |= 0x2000;
        }
        destinationMemory.immOffset += step;
        pc->args[1] = arg_from_operand(&destinationMemory);
        pc->args[2] = fn_004D02D0(dstReg, 0, 0x11);
        appendpcode(pclastblock, pc);
    }
    if (split) {
        pc = makepcode(3, output->reg, output->reg, step * 2);
        pc->args[0].effect |= 0x10;
        pc->args[1].effect |= 0x10;
        appendpcode(pclastblock, pc);
    } else {
        pc = makepcode(3, output->reg, output->reg, step);
        pc->args[0].effect |= 0x10;
        pc->args[1].effect |= 0x10;
        appendpcode(pclastblock, pc);
    }
    emit_branch(0x20, &counter, label);
}

void fn_0053DE80(Operand* dst, Operand* src, SInt32 count) {
    Operand quadInteger;
    Operand half;
    Operand doubleInteger;
    Operand word;
    Operand doublePaired;
    Operand byte;
    Operand wordPaired;
    Operand values[6];
    Operand destinationCopy;
    Operand sourceCopy;
    SInt32 remaining;
    SInt32 i;
    SInt32 toStore;

    sourceCopy = *src;
    destinationCopy = *dst;
    fn_0053F4D0(&sourceCopy);
    fn_0053F4D0(&destinationCopy);
    while (count > 0) {
        remaining = count;
        for (i = 0; i < (SInt8)copts.unknown33; i++) {
            if (remaining >= 16) {
                quadInteger = emit_operand_to_register(0x467, &sourceCopy, 0);
                values[i] = quadInteger;
                sourceCopy.immOffset += 16;
                remaining -= 16;
            } else if (remaining >= 8) {
                if (copts.nofpregforblkmv == 0 && (copts.fp_library & 2)) {
                    doublePaired = emit_operand_to_register(0x455, &sourceCopy, 1);
                    values[i] = doublePaired;
                } else {
                    doubleInteger = emit_operand_to_register(0x454, &sourceCopy, 0);
                    values[i] = doubleInteger;
                }
                sourceCopy.immOffset += 8;
                remaining -= 8;
            } else if (remaining >= 4) {
                if (copts.nofpregforblkmv == 0 && (copts.fp_library & 1)) {
                    wordPaired = emit_operand_to_register(0x460, &sourceCopy, 1);
                    values[i] = wordPaired;
                } else {
                    word = emit_operand_to_register(0x45F, &sourceCopy, 0);
                    values[i] = word;
                }
                sourceCopy.immOffset += 4;
                remaining -= 4;
            } else if (remaining >= 2) {
                half = emit_operand_to_register(0x45B, &sourceCopy, 0);
                values[i] = half;
                sourceCopy.immOffset += 2;
                remaining -= 2;
            } else if (remaining > 0) {
                byte = emit_operand_to_register(0x453, &sourceCopy, 0);
                values[i] = byte;
                sourceCopy.immOffset++;
                remaining--;
            }
        }
        toStore = count;
        for (i = 0; i < (SInt8)copts.unknown33; i++) {
            if (count >= 16) {
                fn_00486660(0x489, &values[i], &destinationCopy);
                destinationCopy.immOffset += 16;
                count -= 16;
                toStore -= 16;
            } else if (count >= 8) {
                if (copts.nofpregforblkmv == 0 && (copts.fp_library & 2)) {
                    fn_00486660(0x47A, &values[i], &destinationCopy);
                } else {
                    fn_00486660(0x479, &values[i], &destinationCopy);
                }
                destinationCopy.immOffset += 8;
                count -= 8;
                toStore -= 8;
            } else if (count >= 4) {
                if (copts.nofpregforblkmv == 0 && (copts.fp_library & 1)) {
                    fn_00486660(0x483, &values[i], &destinationCopy);
                } else {
                    fn_00486660(0x481, &values[i], &destinationCopy);
                }
                destinationCopy.immOffset += 4;
                count -= 4;
                toStore -= 4;
            } else if (count >= 2) {
                fn_00486660(0x480, &values[i], &destinationCopy);
                destinationCopy.immOffset += 2;
                count -= 2;
                toStore -= 2;
            } else if (count > 0) {
                fn_00486660(0x47F, &values[i], &destinationCopy);
                destinationCopy.immOffset++;
                count--;
                toStore--;
            }
        }
        count = toStore;
    }
}

void fn_0053E560(Operand* dst, Operand* src, SInt32 count) {
    Operand doubleInteger;
    Operand word;
    Operand wordPaired;
    Operand byte;
    Operand half;
    Operand doublePaired;
    Operand values[6];
    Operand destinationCopy;
    Operand sourceCopy;
    SInt32 remaining;
    SInt32 i;
    SInt32 toStore;

    sourceCopy = *src;
    destinationCopy = *dst;
    fn_0053F4D0(&sourceCopy);
    fn_0053F4D0(&destinationCopy);
    while (count > 0) {
        remaining = count;
        for (i = 0; i < (SInt8)copts.unknown33; i++) {
            if (remaining >= 8) {
                if (copts.nofpregforblkmv == 0 && (copts.fp_library & 2)) {
                    doublePaired = emit_operand_to_register(0x455, &sourceCopy, 1);
                    values[i] = doublePaired;
                } else {
                    doubleInteger = emit_operand_to_register(0x454, &sourceCopy, 0);
                    values[i] = doubleInteger;
                }
                sourceCopy.immOffset += 8;
                remaining -= 8;
            } else if (remaining >= 4) {
                if (copts.nofpregforblkmv == 0 && (copts.fp_library & 1)) {
                    wordPaired = emit_operand_to_register(0x460, &sourceCopy, 1);
                    values[i] = wordPaired;
                } else {
                    word = emit_operand_to_register(0x45F, &sourceCopy, 0);
                    values[i] = word;
                }
                sourceCopy.immOffset += 4;
                remaining -= 4;
            } else if (remaining >= 2) {
                half = emit_operand_to_register(0x45B, &sourceCopy, 0);
                values[i] = half;
                sourceCopy.immOffset += 2;
                remaining -= 2;
            } else if (remaining > 0) {
                byte = emit_operand_to_register(0x453, &sourceCopy, 0);
                values[i] = byte;
                sourceCopy.immOffset++;
                remaining--;
            }
        }
        toStore = count;
        for (i = 0; i < (SInt8)copts.unknown33; i++) {
            if (count >= 8) {
                if (copts.nofpregforblkmv == 0 && (copts.fp_library & 2)) {
                    fn_00486660(0x47A, &values[i], &destinationCopy);
                } else {
                    fn_00486660(0x479, &values[i], &destinationCopy);
                }
                destinationCopy.immOffset += 8;
                count -= 8;
                toStore -= 8;
            } else if (count >= 4) {
                if (copts.nofpregforblkmv == 0 && (copts.fp_library & 1)) {
                    fn_00486660(0x483, &values[i], &destinationCopy);
                } else {
                    fn_00486660(0x481, &values[i], &destinationCopy);
                }
                destinationCopy.immOffset += 4;
                count -= 4;
                toStore -= 4;
            } else if (count >= 2) {
                fn_00486660(0x480, &values[i], &destinationCopy);
                destinationCopy.immOffset += 2;
                count -= 2;
                toStore -= 2;
            } else if (count > 0) {
                fn_00486660(0x47F, &values[i], &destinationCopy);
                destinationCopy.immOffset++;
                count--;
                toStore--;
            }
        }
        count = toStore;
    }
}

void fn_0053EB60(Operand* dst, Operand* src, SInt32 count) {
    Operand byte;
    Operand word;
    Operand half;
    Operand wordPaired;
    Operand values[6];
    Operand destinationCopy;
    Operand sourceCopy;
    SInt32 remaining;
    SInt32 i;
    SInt32 toStore;

    sourceCopy = *src;
    destinationCopy = *dst;
    fn_0053F4D0(&sourceCopy);
    fn_0053F4D0(&destinationCopy);
    while (count > 0) {
        remaining = count;
        for (i = 0; i < (SInt8)copts.unknown33; i++) {
            if (remaining >= 4) {
                if (copts.nofpregforblkmv == 0 && (copts.fp_library & 1)) {
                    wordPaired = emit_operand_to_register(0x460, &sourceCopy, 1);
                    values[i] = wordPaired;
                } else {
                    word = emit_operand_to_register(0x45F, &sourceCopy, 0);
                    values[i] = word;
                }
                sourceCopy.immOffset += 4;
                remaining -= 4;
            } else if (remaining >= 2) {
                half = emit_operand_to_register(0x45B, &sourceCopy, 0);
                values[i] = half;
                sourceCopy.immOffset += 2;
                remaining -= 2;
            } else if (remaining > 0) {
                byte = emit_operand_to_register(0x453, &sourceCopy, 0);
                values[i] = byte;
                sourceCopy.immOffset++;
                remaining--;
            }
        }
        toStore = count;
        for (i = 0; i < (SInt8)copts.unknown33; i++) {
            if (count >= 4) {
                if (copts.nofpregforblkmv == 0 && (copts.fp_library & 1)) {
                    fn_00486660(0x483, &values[i], &destinationCopy);
                } else {
                    fn_00486660(0x481, &values[i], &destinationCopy);
                }
                destinationCopy.immOffset += 4;
                count -= 4;
                toStore -= 4;
            } else if (count >= 2) {
                fn_00486660(0x480, &values[i], &destinationCopy);
                destinationCopy.immOffset += 2;
                count -= 2;
                toStore -= 2;
            } else if (count > 0) {
                fn_00486660(0x47F, &values[i], &destinationCopy);
                destinationCopy.immOffset++;
                count--;
                toStore--;
            }
        }
        count = toStore;
    }
}

void fn_0053EFB0(Operand* dst, Operand* src, SInt32 count) {
    Operand byte;
    Operand half;
    Operand values[6];
    Operand dstCopy;
    Operand srcCopy;
    SInt32 i;
    SInt32 remaining;

    srcCopy = *src;
    dstCopy = *dst;
    fn_0053F4D0(&srcCopy);
    fn_0053F4D0(&dstCopy);
    remaining = count;
    for (i = 0; i < (SInt8)copts.unknown33; i++) {
        if (remaining >= 2) {
            half = emit_operand_to_register(0x45B, &srcCopy, 0);
            values[i] = half;
            srcCopy.immOffset += 2;
            remaining -= 2;
        } else if (remaining > 0) {
            byte = emit_operand_to_register(0x453, &srcCopy, 0);
            values[i] = byte;
            srcCopy.immOffset++;
            remaining--;
        }
    }
    for (i = 0; i < (SInt8)copts.unknown33; i++) {
        if (count >= 2) {
            fn_00486660(0x480, &values[i], &dstCopy);
            dstCopy.immOffset += 2;
            count -= 2;
        } else if (count > 0) {
            fn_00486660(0x47F, &values[i], &dstCopy);
            dstCopy.immOffset++;
            count--;
        }
    }
}

void fn_0053F230(Operand* dst, Operand* src, SInt32 count) {
    Operand values[6];
    Operand srcCopy;
    Operand dstCopy;
    Operand value;
    SInt32 remaining;
    SInt32 i;

    srcCopy = *src;
    dstCopy = *dst;
    fn_0053F4D0(&srcCopy);
    fn_0053F4D0(&dstCopy);
    remaining = count;
    for (i = 0; i < (SInt8)copts.unknown33; i++) {
        if (remaining > 0) {
            value = emit_operand_to_register(0x453, &srcCopy, 0);
            values[i] = value;
            srcCopy.immOffset++;
            remaining--;
        }
    }
    for (i = 0; i < (SInt8)copts.unknown33; i++) {
        if (count > 0) {
            fn_00486660(0x47F, &values[i], &dstCopy);
            dstCopy.immOffset++;
            count--;
        }
    }
}
