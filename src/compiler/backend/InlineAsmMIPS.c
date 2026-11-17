#define _MSL_USE_INLINE 1
#define CERROR_FILE "InlineAsmMIPS.c"
#include "compiler/CodeGen.h"
#include "compiler/InlineAsmMIPS.h"
#include "compiler/objects.h"
#include "compiler/PCodeInfo.h"
#include "compiler/PCode.h"
#include "compiler/CFunc.h"
#include "compiler/CompilerTools.h"
#include "compiler/CError.h"
#include "compiler/MIPSError.h"
#include "compiler/CInt64.h"
#include "compiler/CParser.h"
#include "compiler/CExpr.h"
#include "compiler/InlineAsm.h"
#include <string.h>
#include "compiler/CPrep.h"
#include "compiler/CPrepTokenizer.h"
#include "compiler/UseDefChains.h"
#include "compiler/Registers.h"
#include "compiler/RegisterInfo.h"
#include "compiler/RegListQueries.h"
#include "msl/ctype_api.h"
#include "msl/setjmp_x86.h"

const char* InlineAsm_GetMnemonic(const SInt32* opcode) {
    return opcode_info[*opcode].name;
}

void InlineAsm_TranslateIRtoPCode(Statement* stmt) {
    InlineAsmMIPSRecord* ia;
    UInt32 opcode;
    UInt32 argcount;
    PCode* pc;
    PCodeArg* arg;
    PCodeLabel* label;
    SInt32 cls;
    SInt32 index;
    SInt32 reg;

    ia = (InlineAsmMIPSRecord*)stmt->expr;
    opcode = ia->opcode;
    argcount = (UInt8)opcode_info[opcode].unkC;
    if (opcode == 0x4AE) {
        if (ia->flags & 4) {
            cmach_context_flag_63736A = 1;
        }
        if (ia->flags & 0x10) {
            cmach_context_flag_63736A = 0;
        }
        if (ia->flags & 1) {
            asm_flag_637388 = 0;
            new_basic_block();
            cg_codelist_reg_kind_637332 &= 0xBF77;
            pclastblock->flags = cg_codelist_reg_kind_637332;
        }
        if (ia->flags & 2) {
            asm_flag_637388 = 1;
            new_basic_block();
            cg_codelist_reg_kind_637332 |= 0x4088;
            pclastblock->flags = cg_codelist_reg_kind_637332;
        }
        return;
    }
    if (opcode_info[opcode].flags & 8) {
        SInt32 excluded = fn_004727F0();
        SInt32 total = fn_0046D3F0();
        argcount += total - excluded;
    }
    pc = fn_00491320(ia, argcount);
    pc->flags |= 0x200000;
    appendpcode(pclastblock, pc);
    if ((UInt16)(pc->op - 0x278) <= 2) {
        pclastblock->flags |= 0x80;
    }
    if (pc->flags & fPCodeIsBranch) {
        if (opcode_info[pc->op].flags & 8) {
            arg = &pc->args[argcount - 1];
            for (cls = 0; cls < RegClassMax; cls++) {
                for (index = 0; index < n_scratch_registers[cls]; index++) {
                    if (cls == 5) {
                        break;
                    }
                    reg = scratch_registers[cls][index];
                    arg->kind = PCOp_REGISTER;
                    arg->arg = cls;
                    arg->data.reg.reg = reg;
                    arg->effect = 3;
                    arg--;
                }
            }
        }
        if (pc->op < 0x40A) {
            asm_pending_branch_63603C = pc;
        }
        if (pc->flags & fPCodeConditionalBranch) {
            label = makepclabel();
            for (arg = pc->args; arg->kind != PCOp_LABEL; arg++) {}
            pcaddedge(pclastblock, arg->data.label.label, 0);
            pcaddedge(pclastblock, label, 0);
            pclastblock->flags |= cg_codelist_reg_kind_637332;
            makepcblock();
            pclabelblock(pclastblock, label);
        } else if (pc->op == 0x3B) {
            pclastblock->flags |= cg_codelist_reg_kind_637332;
            makepcblock();
        } else if (pc->args[0].kind == PCOp_LABEL) {
            pcaddedge(pclastblock, pc->args[0].data.label.label, 0);
            pclastblock->flags |= cg_codelist_reg_kind_637332;
            makepcblock();
        } else {
            pclastblock->flags |= cg_codelist_reg_kind_637332;
            new_basic_block();
        }
    } else {
        if (asm_pending_branch_63603C &&
            (cg_codelist_reg_kind_637332 & 0x4000) &&
            pc->op < 0x40A &&
            !cg_codegen_emit_flag_63735A) {
            deletepcode(pc);
            insertpcodebefore(asm_pending_branch_63603C, pc);
            pc->flags |= fPCodeInDelaySlot;
        }
        asm_pending_branch_63603C = NULL;
    }
    if (cg_codegen_emit_flag_63735A) {
        cg_codegen_emit_flag_63735A = 0;
    }
    pclastblock->flags |= cg_codelist_reg_kind_637332;
    cg_codegen_cursor_636018 = pclastblock;
}

PCode* fn_00491320(InlineAsmMIPSRecord* ia, int argcount) {
    OpcodeInfo* info = &opcode_info[ia->opcode];
    const char* format = info->format;
    UInt32 size = sizeof(PCode) + argcount * sizeof(PCodeArg);
    PCode* pc;
    InlineAsmMIPSOperand* src = (InlineAsmMIPSOperand*)ia->args;
    PCodeArg* dst;
    int effect;
    Boolean component;
    Boolean pair;
    Boolean special;

    CError_ASSERT(4044, *format != '#');
    pc = lalloc(size);
    dst = pc->args;
    memset(pc, 0, size);
    pc->op = (UInt16)ia->opcode;
    pc->argCount = (SInt16)argcount;
    pc->sourceoffset = asm_source_offset_6361A0;
    pc->flags |= info->flags;
    if ((cmach_context_flag_63736A || asm_flag_637388 || (ia->flags & 4)) && !(ia->flags & 0x20)) {
        pc->flags |= 0x100;
        pc->flags &= ~0x4010;
    }

    while (*format) {
        effect = 1;
        component = FALSE;
        pair = FALSE;
        special = FALSE;
        if (*format == ';') {
            format++;
        }
        if (*format == ',') {
            format++;
        }
        if (*format == '(' || *format == '[') {
            format++;
        }
        if (*format == '=') {
            effect = 2;
            format++;
        } else if (*format == '+') {
            effect = 3;
            format++;
        }
        if (*format == '!') {
            effect |= 0xFFFF8000;
            if (copts.unknown1A) {
                effect |= 0x20;
            }
            format++;
        }
        if (*format == '$') {
            effect |= 0x2000;
            format++;
        }
        if (*format == '2') {
            pair = TRUE;
            format++;
            if (*format == '+') {
                special = TRUE;
                format++;
            }
        }
        if (*format == '\'') {
            component = TRUE;
            format++;
        }

        switch (*format) {
            case '-':
            case '|':
                format++;
                continue;
            case '&':
            case '<':
            case '>':
            case '@':
            case 'X':
            case 'B':
            case 'C':
            case 'M':
            case 'c':
            case 'v':
                dst->kind = PCOp_IMMEDIATE;
                dst->data.imm.value = src->unk10;
                if (*format == '&' && dst->data.imm.value < 15) {
                    pc->flags &= ~0x4010;
                }
                src++;
                dst++;
                break;
            case '0':
                dst->kind = PCOp_REGISTER;
                dst->arg = 2;
                dst->data.reg.reg = 4;
                dst->effect = effect;
                src++;
                dst++;
                break;
            case '1':
                dst->kind = PCOp_REGISTER;
                dst->arg = 2;
                dst->data.reg.reg = 3;
                dst->effect = effect;
                pc->flags |= 0x100;
                src++;
                dst++;
                break;
            case '9':
                dst->kind = PCOp_REGISTER;
                dst->arg = 3;
                dst->data.reg.reg = 27;
                dst->effect = effect;
                src++;
                dst++;
                break;
            case 'G':
                dst->kind = PCOp_REGISTER;
                dst->arg = 2;
                dst->data.reg.reg = src->value;
                dst->effect = effect;
                src++;
                dst++;
                break;
            case 'H':
                dst->kind = PCOp_REGISTER;
                dst->arg = 2;
                dst->data.reg.reg = 1;
                dst->effect = effect;
                pc->flags |= 0x100;
                src++;
                dst++;
                break;
            case 'Y':
                dst->kind = PCOp_REGISTER;
                dst->arg = 2;
                dst->data.reg.reg = 6;
                dst->effect = effect;
                pc->flags |= 0x100;
                src++;
                dst++;
                break;
            case 'Z':
                dst->kind = PCOp_REGISTER;
                dst->arg = 2;
                dst->data.reg.reg = 2;
                dst->effect = effect;
                pc->flags |= 0x100;
                src++;
                dst++;
                break;
            case 'h':
                dst->kind = PCOp_REGISTER;
                dst->arg = 2;
                dst->data.reg.reg = 0;
                dst->effect = effect;
                pc->flags |= 0x100;
                src++;
                dst++;
                break;
            case 'r':
            case 'P':
                dst->kind = PCOp_REGISTER;
                dst->arg = 0;
                dst->data.reg.reg = *format == 'r' ? 31 : src->value;
                dst->effect = effect;
                if (*format == 'r') {
                    pc->flags |= 0x100;
                }
                src++;
                dst++;
                break;
            case 'D':
            case 'S':
                if (src->object && src->object->type->type != TYPEFLOAT && !pair) {
                    src->value = 0;
                    CError_Error(45);
                }
            case 'E':
            case 'N':
            case 'T':
            case 'V':
            case 'W': {
                Boolean explicitRegister = TRUE;
                PCodeArg* mask;
                SInt32 count;
                if (src->object) {
                    src->value = src->object->datatype == DLOCAL ? src->object->u.var.info->reg : 0;
                    explicitRegister = FALSE;
                    if (fn_004D1D40(src->object) && src->value == 0) {
                        MIPSError_Error(10, src->object->name->name);
                    } else if (src->value == 0) {
                        CError_Error(164, src->object->name->name);
                    }
                }
                dst->kind = PCOp_REGISTER;
                dst->arg = pair ? 5 : 1;
                dst->data.reg.reg = src->value;
                if ((effect & 2) && !(src->unkC & 1) && (effect & 1)) {
                    effect &= ~1;
                }
                dst->effect = effect;
                if (pair) {
                    if ((dst->effect & 2) && *opcode_info[pc->op].format == '&') {
                        count = 0;
                        mask = dst;
                        while (mask->kind != PCOp_IMMEDIATE) {
                            count++;
                            CError_ASSERT(4713, count <= 4);
                            mask--;
                        }
                        if (mask->data.imm.value != 15) {
                            pc->flags &= ~0x4010;
                            dst->effect |= 3;
                        }
                    }
                    if (src->extendedComponentMask && component) {
                        if (src->unk10 & 1) {
                            dst->effect |= 0x800;
                        }
                        if (src->unk10 & 2) {
                            dst->effect |= 0x400;
                        }
                        if (src->unk10 & 4) {
                            dst->effect |= 0x200;
                        }
                        if (src->unk10 & 8) {
                            dst->effect |= 0x100;
                        }
                    }
                    if (dst->data.reg.reg >= 0x4020 && dst->data.reg.reg <= 0x403F) {
                        if (component) {
                            dst->effect |= 0x800;
                        }
                        if (explicitRegister) {
                            dst->data.reg.reg -= 0x4020;
                        }
                    } else if (dst->data.reg.reg >= 0x4040 && dst->data.reg.reg <= 0x405F) {
                        if (component) {
                            dst->effect |= 0x400;
                        }
                        if (explicitRegister) {
                            dst->data.reg.reg -= 0x4040;
                        }
                    } else if (dst->data.reg.reg >= 0x4080 && dst->data.reg.reg <= 0x409F) {
                        if (component) {
                            dst->effect |= 0x200;
                        }
                        if (explicitRegister) {
                            dst->data.reg.reg -= 0x4080;
                        }
                    } else if (dst->data.reg.reg >= 0x40A0 && dst->data.reg.reg <= 0x40BF) {
                        if (component) {
                            dst->effect |= 0x100;
                        }
                        if (explicitRegister) {
                            dst->data.reg.reg -= 0x40A0;
                        }
                    }
                }
                src++;
                dst++;
                break;
            }
            case 'd':
            case 's':
            case 't':
            case 'x':
                if (src->object) {
                    src->value = src->object->datatype == DLOCAL ? src->object->u.var.info->reg : 0;
                    if (!src->value) {
                        CError_Error(164, src->object->name->name);
                    }
                }
                dst->kind = PCOp_REGISTER;
                dst->arg = pair ? (special ? 4 : 3) : 0;
                dst->data.reg.reg = src->value;
                dst->effect = effect;
                src++;
                dst++;
                break;
            case 'I':
            case 'f':
            case 'i':
            case 'j':
            case 'k':
                if (*format == 'I' && pair) {
                    dst->kind = PCOp_REGISTER;
                    dst->arg = 6;
                    dst->data.reg.reg = src->value;
                    dst->effect = effect;
                } else {
                    dst->kind = PCOp_CINT64;
                    dst->data.cint.hi = src->value;
                    dst->data.cint.unk4 = src->unk8;
                    dst->data.cint.unk8 = src->unkC;
                    dst->data.cint.lo = src->unk10;
                }
                src++;
                dst++;
                break;
            case 'J':
            case 'L':
            case 'Q':
            case 'R':
                if (*format == 'J' || *format == 'L' || pair) {
                    dst->kind = PCOp_REGISTER;
                    dst->arg = 6;
                    dst->data.reg.reg = src->value;
                    dst->effect = effect;
                }
                src++;
                dst++;
                break;
            case 'l':
            case 'p':
                dst->kind = PCOp_LABEL;
                dst->data.label.label = src->label->pclabel;
                CError_ASSERT(4429, dst->data.label.label != NULL);
                src++;
                dst++;
                break;
            default:
                if (*format == 'r' ||
                    *format == 'R' ||
                    *format == 'f' ||
                    *format == 'F' ||
                    *format == 'g' ||
                    *format == 'G' ||
                    *format == 'v' ||
                    *format == 'V') {
                    dst->kind = 0;
                    dst->arg = 2;
                    dst->effect = (UInt16)effect;
                    dst->data.reg.reg = src->value;
                    pc->flags |= 0x100;
                } else if (*format == 'l' || *format == 'L') {
                    dst->kind = 6;
                    dst->data.label.label = (PCodeLabel*)src->value;
                    pc->unk38 = 0;
                } else if (*format == 'i' ||
                           *format == 'I' ||
                           *format == 's' ||
                           *format == 'S' ||
                           *format == 'u' ||
                           *format == 'U') {
                    dst->kind = 0;
                    dst->arg = 6;
                    dst->effect = (UInt16)effect;
                    dst->data.reg.reg = src->value;
                } else if (*format == 'm' || *format == 'M') {
                    dst->kind = 3;
                    dst->data.cint.hi = src->value;
                    dst->data.cint.unk4 = src->unk8;
                    dst->data.cint.unk8 = src->unkC;
                    dst->data.cint.lo = src->unk10;
                } else {
                    dst->kind = 2;
                    dst->data.imm.value = src->unk10;
                }
                src++;
                dst++;
                break;
        }
        format++;
        if (*format == ')' || *format == ']') {
            format++;
        }
    }

    if ((pc->flags & 6) && !pc->alias) {
        pc->flags |= 0x20;
    }
    return pc;
}

void InlineAsm_ScanAssemblyInstruction(void) {
    char name[300];
    volatile int position;
    int state;
    Statement* stmt;
    GCCNameInfo* mnemonic;

    if (tk == '}' || tk == ')') {
        return;
    }
    if (inline_asm_setjmp_valid && tk == '"') {
        tk = lex();
        InlineAsm_gcc_parse();
        return;
    }

    InlineAsm_SkipComment();
    fn_004944A0(name);
    mnemonic = gcc_find_name(name);
    if ((SInt32)mnemonic == 0) {
        CError_Error(162);
    }
    stmt = CFunc_AppendStatement(ST_ASM);
    stmt->expr = NULL;
    position = 0;
    fn_00444780(&state);
    setjmp(inline_asm_backtrack_context);
    fn_00444740(&state);
    inline_asm_backtracking = position + 1 < mnemonic->count;
    tk = lex();
    stmt->expr = (ENode*)fn_004946C0((UInt16)mnemonic->operands[position++]);
}

void InlineAsm_ScanAssemblyDirective(void) {
    Statement* stmt;
    InlineAsmMIPSRecord* ia;
    const char* name;
    InlineAsmMIPSOptionsFrame* frame;

    InlineAsm_SkipComment();
    tk = lex();
    if (tk != -3) {
        InlineAsm_SyntaxError(8);
    }
    name = tkidentifier->name;
    if (!memcmp(name, "set", 4)) {
        stmt = CFunc_AppendStatement(ST_ASM);
        ia = lalloc(8);
        memclrw(ia, 8);
        stmt->expr = (ENode*)ia;
        ia->opcode = 0x4AE;

        InlineAsm_SkipComment();
        tk = lex();
        if (tk != -3) {
            InlineAsm_SyntaxError(8);
        }
        name = tkidentifier->name;

        if (!memcmp(name, "asm_volatile", 13)) {
            ia->flags = 4;
        } else if (!memcmp(name, "asm_nonvolatile", 16)) {
            ia->flags = 0x10;
        } else if (!memcmp(name, "reorder", 8)) {
            ia->flags = 1;
        } else if (!memcmp(name, "noreorder", 10)) {
            ia->flags = 2;
        } else if (!memcmp(name, "at", 3)) {
            asm_noat_flag_637374 = 0;
        } else if (!memcmp(name, "noat", 5)) {
            asm_noat_flag_637374 = 1;
        } else if (!memcmp(name, "nomacro", 8)) {
            asm_macro_flag_63735B = 1;
        } else if (!memcmp(name, "macro", 6)) {
            asm_macro_flag_63735B = 0;
        } else if (!memcmp(name, "push", 5)) {
            frame = lalloc(sizeof(InlineAsmMIPSOptionsFrame));
            if (frame) {
                frame->previous = asm_option_stack_608DD0;
                frame->reorder = cmach_context_flag_63736A;
                frame->volatileasm = asm_flag_637388;
                frame->macro = asm_macro_flag_63735B;
                frame->noat = asm_noat_flag_637374;
                frame->flags = asm_option_flags_637336;
                asm_option_stack_608DD0 = frame;
            } else {
                CError_FATAL(3474);
            }
        } else if (!memcmp(name, "pop", 4) && asm_option_stack_608DD0) {
            frame = asm_option_stack_608DD0;
            asm_option_stack_608DD0 = frame->previous;
            cmach_context_flag_63736A = frame->reorder;
            asm_flag_637388 = frame->volatileasm;
            asm_macro_flag_63735B = frame->macro;
            asm_noat_flag_637374 = frame->noat;
            asm_option_flags_637336 = frame->flags;
        } else {
            CError_Error(45);
        }
        tk = lex();
        return;
    }
    InlineAsm_ScanAssemblyInstruction();
}

void InlineAsm_Initialize(void) {
    UInt16 opcode;
    OpcodeInfo* info;
    cmach_context_flag_63736A |= copts.volatileasm;
    asm_macro_flag_63735B = copts.nomacro;
    asm_noat_flag_637374 = 0;
    asm_flag_637388 = 0;
    inline_asm_allow_array_expressions = 1;
    asm_pending_branch_63603C = NULL;
    gcc_clear_name_tables();
    for (opcode = 0; opcode < 0x4AF; opcode++) {
        info = &opcode_info[opcode];
        if (info->name && *info->name != '_') {
            gcc_add_value((char*)info->name, opcode);
            if (info->unk4) {
                gcc_add_value((char*)info->unk4, opcode);
            }
        }
    }
    gcc_clear_statics();
    gcc_add_static("r0", 0, 0, 0);
    gcc_add_static("r1", 0, 1, 0);
    gcc_add_static("r2", 0, 2, 0);
    gcc_add_static("r3", 0, 3, 0);
    gcc_add_static("r4", 0, 4, 0);
    gcc_add_static("r5", 0, 5, 0);
    gcc_add_static("r6", 0, 6, 0);
    gcc_add_static("r7", 0, 7, 0);
    gcc_add_static("r8", 0, 8, 0);
    gcc_add_static("r9", 0, 9, 0);
    gcc_add_static("r10", 0, 10, 0);
    gcc_add_static("r11", 0, 11, 0);
    gcc_add_static("r12", 0, 12, 0);
    gcc_add_static("r13", 0, 13, 0);
    gcc_add_static("r14", 0, 14, 0);
    gcc_add_static("r15", 0, 15, 0);
    gcc_add_static("r16", 0, 16, 0);
    gcc_add_static("r17", 0, 17, 0);
    gcc_add_static("r18", 0, 18, 0);
    gcc_add_static("r19", 0, 19, 0);
    gcc_add_static("r20", 0, 20, 0);
    gcc_add_static("r21", 0, 21, 0);
    gcc_add_static("r22", 0, 22, 0);
    gcc_add_static("r23", 0, 23, 0);
    gcc_add_static("r24", 0, 24, 0);
    gcc_add_static("r25", 0, 25, 0);
    gcc_add_static("r26", 0, 26, 0);
    gcc_add_static("r27", 0, 27, 0);
    gcc_add_static("r28", 0, 28, 0);
    gcc_add_static("r29", 0, 29, 0);
    gcc_add_static("r30", 0, 30, 0);
    gcc_add_static("r31", 0, 31, 0);
    gcc_add_static("zero", 0, 0, 0);
    gcc_add_static("at", 0, 1, 0);
    gcc_add_static("AT", 0, 1, 0);
    gcc_add_static("v0", 0, 2, 0);
    gcc_add_static("v1", 0, 3, 0);
    gcc_add_static("a0", 0, 4, 0);
    gcc_add_static("a1", 0, 5, 0);
    gcc_add_static("a2", 0, 6, 0);
    gcc_add_static("a3", 0, 7, 0);
    if (copts.unknown15) {
        gcc_add_static("a4", 0, 8, 0);
        gcc_add_static("a5", 0, 9, 0);
        gcc_add_static("a6", 0, 10, 0);
        gcc_add_static("a7", 0, 11, 0);
    }
    gcc_add_static("t0", 0, 8, 0);
    gcc_add_static("t1", 0, 9, 0);
    gcc_add_static("t2", 0, 10, 0);
    gcc_add_static("t3", 0, 11, 0);
    gcc_add_static("t4", 0, 12, 0);
    gcc_add_static("t5", 0, 13, 0);
    gcc_add_static("t6", 0, 14, 0);
    gcc_add_static("t7", 0, 15, 0);
    gcc_add_static("s0", 0, 16, 0);
    gcc_add_static("s1", 0, 17, 0);
    gcc_add_static("s2", 0, 18, 0);
    gcc_add_static("s3", 0, 19, 0);
    gcc_add_static("s4", 0, 20, 0);
    gcc_add_static("s5", 0, 21, 0);
    gcc_add_static("s6", 0, 22, 0);
    gcc_add_static("s7", 0, 23, 0);
    gcc_add_static("t8", 0, 24, 0);
    gcc_add_static("t9", 0, 25, 0);
    gcc_add_static("kt0", 0, 26, 0);
    gcc_add_static("k0", 0, 26, 0);
    gcc_add_static("kt1", 0, 27, 0);
    gcc_add_static("k1", 0, 27, 0);
    gcc_add_static("gp", 0, 28, 0);
    gcc_add_static("sp", 0, 29, 0);
    gcc_add_static("s8", 0, 30, 0);
    gcc_add_static("fp", 0, 30, 0);
    gcc_add_static("ra", 0, 31, 0);
    gcc_add_static("f0", 1, 0, 0);
    gcc_add_static("f1", 1, 1, 0);
    gcc_add_static("f2", 1, 2, 0);
    gcc_add_static("f3", 1, 3, 0);
    gcc_add_static("f4", 1, 4, 0);
    gcc_add_static("f5", 1, 5, 0);
    gcc_add_static("f6", 1, 6, 0);
    gcc_add_static("f7", 1, 7, 0);
    gcc_add_static("f8", 1, 8, 0);
    gcc_add_static("f9", 1, 9, 0);
    gcc_add_static("f10", 1, 10, 0);
    gcc_add_static("f11", 1, 11, 0);
    gcc_add_static("f12", 1, 12, 0);
    gcc_add_static("f13", 1, 13, 0);
    gcc_add_static("f14", 1, 14, 0);
    gcc_add_static("f15", 1, 15, 0);
    gcc_add_static("f16", 1, 16, 0);
    gcc_add_static("f17", 1, 17, 0);
    gcc_add_static("f18", 1, 18, 0);
    gcc_add_static("f19", 1, 19, 0);
    gcc_add_static("f20", 1, 20, 0);
    gcc_add_static("f21", 1, 21, 0);
    gcc_add_static("f22", 1, 22, 0);
    gcc_add_static("f23", 1, 23, 0);
    gcc_add_static("f24", 1, 24, 0);
    gcc_add_static("f25", 1, 25, 0);
    gcc_add_static("f26", 1, 26, 0);
    gcc_add_static("f27", 1, 27, 0);
    gcc_add_static("f28", 1, 28, 0);
    gcc_add_static("f29", 1, 29, 0);
    gcc_add_static("f30", 1, 30, 0);
    gcc_add_static("f31", 1, 31, 0);
    gcc_add_static("d0", 1, 0, 0);
    gcc_add_static("d1", 1, 1, 0);
    gcc_add_static("d2", 1, 2, 0);
    gcc_add_static("d3", 1, 3, 0);
    gcc_add_static("d4", 1, 4, 0);
    gcc_add_static("d5", 1, 5, 0);
    gcc_add_static("d6", 1, 6, 0);
    gcc_add_static("d7", 1, 7, 0);
    gcc_add_static("d8", 1, 8, 0);
    gcc_add_static("d9", 1, 9, 0);
    gcc_add_static("d10", 1, 10, 0);
    gcc_add_static("d11", 1, 11, 0);
    gcc_add_static("d12", 1, 12, 0);
    gcc_add_static("d13", 1, 13, 0);
    gcc_add_static("d14", 1, 14, 0);
    gcc_add_static("d15", 1, 15, 0);
    gcc_add_static("d16", 1, 16, 0);
    gcc_add_static("d17", 1, 17, 0);
    gcc_add_static("d18", 1, 18, 0);
    gcc_add_static("d19", 1, 19, 0);
    gcc_add_static("d20", 1, 20, 0);
    gcc_add_static("d21", 1, 21, 0);
    gcc_add_static("d22", 1, 22, 0);
    gcc_add_static("d23", 1, 23, 0);
    gcc_add_static("d24", 1, 24, 0);
    gcc_add_static("d25", 1, 25, 0);
    gcc_add_static("d26", 1, 26, 0);
    gcc_add_static("d27", 1, 27, 0);
    gcc_add_static("d28", 1, 28, 0);
    gcc_add_static("d29", 1, 29, 0);
    gcc_add_static("d30", 1, 30, 0);
    gcc_add_static("d31", 1, 31, 0);
    gcc_add_static("vi0", 3, 0, 0);
    gcc_add_static("vi1", 3, 1, 0);
    gcc_add_static("vi2", 3, 2, 0);
    gcc_add_static("vi3", 3, 3, 0);
    gcc_add_static("vi4", 3, 4, 0);
    gcc_add_static("vi5", 3, 5, 0);
    gcc_add_static("vi6", 3, 6, 0);
    gcc_add_static("vi7", 3, 7, 0);
    gcc_add_static("vi8", 3, 8, 0);
    gcc_add_static("vi9", 3, 9, 0);
    gcc_add_static("vi00", 3, 0, 0);
    gcc_add_static("vi01", 3, 1, 0);
    gcc_add_static("vi02", 3, 2, 0);
    gcc_add_static("vi03", 3, 3, 0);
    gcc_add_static("vi04", 3, 4, 0);
    gcc_add_static("vi05", 3, 5, 0);
    gcc_add_static("vi06", 3, 6, 0);
    gcc_add_static("vi07", 3, 7, 0);
    gcc_add_static("vi08", 3, 8, 0);
    gcc_add_static("vi09", 3, 9, 0);
    gcc_add_static("vi10", 3, 10, 0);
    gcc_add_static("vi11", 3, 11, 0);
    gcc_add_static("vi12", 3, 12, 0);
    gcc_add_static("vi13", 3, 13, 0);
    gcc_add_static("vi14", 3, 14, 0);
    gcc_add_static("vi15", 3, 15, 0);
    gcc_add_static("vi16", 4, 16, 0);
    gcc_add_static("vi17", 4, 17, 0);
    gcc_add_static("vi18", 4, 18, 0);
    gcc_add_static("vi19", 4, 19, 0);
    gcc_add_static("vi20", 4, 20, 0);
    gcc_add_static("vi21", 4, 21, 0);
    gcc_add_static("vi22", 4, 22, 0);
    gcc_add_static("vi23", 4, 23, 0);
    gcc_add_static("vi24", 4, 24, 0);
    gcc_add_static("vi25", 4, 25, 0);
    gcc_add_static("vi26", 4, 26, 0);
    gcc_add_static("vi27", 4, 27, 0);
    gcc_add_static("vi28", 4, 28, 0);
    gcc_add_static("vi29", 4, 29, 0);
    gcc_add_static("vi30", 4, 30, 0);
    gcc_add_static("vi31", 4, 31, 0);
    gcc_add_static("vf0", 5, 0, 0);
    gcc_add_static("vf1", 5, 1, 0);
    gcc_add_static("vf2", 5, 2, 0);
    gcc_add_static("vf3", 5, 3, 0);
    gcc_add_static("vf4", 5, 4, 0);
    gcc_add_static("vf5", 5, 5, 0);
    gcc_add_static("vf6", 5, 6, 0);
    gcc_add_static("vf7", 5, 7, 0);
    gcc_add_static("vf8", 5, 8, 0);
    gcc_add_static("vf9", 5, 9, 0);
    gcc_add_static("vf00", 5, 0, 0);
    gcc_add_static("vf01", 5, 1, 0);
    gcc_add_static("vf02", 5, 2, 0);
    gcc_add_static("vf03", 5, 3, 0);
    gcc_add_static("vf04", 5, 4, 0);
    gcc_add_static("vf05", 5, 5, 0);
    gcc_add_static("vf06", 5, 6, 0);
    gcc_add_static("vf07", 5, 7, 0);
    gcc_add_static("vf08", 5, 8, 0);
    gcc_add_static("vf09", 5, 9, 0);
    gcc_add_static("vf10", 5, 10, 0);
    gcc_add_static("vf11", 5, 11, 0);
    gcc_add_static("vf12", 5, 12, 0);
    gcc_add_static("vf13", 5, 13, 0);
    gcc_add_static("vf14", 5, 14, 0);
    gcc_add_static("vf15", 5, 15, 0);
    gcc_add_static("vf16", 5, 16, 0);
    gcc_add_static("vf17", 5, 17, 0);
    gcc_add_static("vf18", 5, 18, 0);
    gcc_add_static("vf19", 5, 19, 0);
    gcc_add_static("vf20", 5, 20, 0);
    gcc_add_static("vf21", 5, 21, 0);
    gcc_add_static("vf22", 5, 22, 0);
    gcc_add_static("vf23", 5, 23, 0);
    gcc_add_static("vf24", 5, 24, 0);
    gcc_add_static("vf25", 5, 25, 0);
    gcc_add_static("vf26", 5, 26, 0);
    gcc_add_static("vf27", 5, 27, 0);
    gcc_add_static("vf28", 5, 28, 0);
    gcc_add_static("vf29", 5, 29, 0);
    gcc_add_static("vf30", 5, 30, 0);
    gcc_add_static("vf31", 5, 31, 0);
    gcc_add_static("vf0x", 5, 16416, 0);
    gcc_add_static("vf1x", 5, 16417, 0);
    gcc_add_static("vf2x", 5, 16418, 0);
    gcc_add_static("vf3x", 5, 16419, 0);
    gcc_add_static("vf4x", 5, 16420, 0);
    gcc_add_static("vf5x", 5, 16421, 0);
    gcc_add_static("vf6x", 5, 16422, 0);
    gcc_add_static("vf7x", 5, 16423, 0);
    gcc_add_static("vf8x", 5, 16424, 0);
    gcc_add_static("vf9x", 5, 16425, 0);
    gcc_add_static("vf00x", 5, 16416, 0);
    gcc_add_static("vf01x", 5, 16417, 0);
    gcc_add_static("vf02x", 5, 16418, 0);
    gcc_add_static("vf03x", 5, 16419, 0);
    gcc_add_static("vf04x", 5, 16420, 0);
    gcc_add_static("vf05x", 5, 16421, 0);
    gcc_add_static("vf06x", 5, 16422, 0);
    gcc_add_static("vf07x", 5, 16423, 0);
    gcc_add_static("vf08x", 5, 16424, 0);
    gcc_add_static("vf09x", 5, 16425, 0);
    gcc_add_static("vf10x", 5, 16426, 0);
    gcc_add_static("vf11x", 5, 16427, 0);
    gcc_add_static("vf12x", 5, 16428, 0);
    gcc_add_static("vf13x", 5, 16429, 0);
    gcc_add_static("vf14x", 5, 16430, 0);
    gcc_add_static("vf15x", 5, 16431, 0);
    gcc_add_static("vf16x", 5, 16432, 0);
    gcc_add_static("vf17x", 5, 16433, 0);
    gcc_add_static("vf18x", 5, 16434, 0);
    gcc_add_static("vf19x", 5, 16435, 0);
    gcc_add_static("vf20x", 5, 16436, 0);
    gcc_add_static("vf21x", 5, 16437, 0);
    gcc_add_static("vf22x", 5, 16438, 0);
    gcc_add_static("vf23x", 5, 16439, 0);
    gcc_add_static("vf24x", 5, 16440, 0);
    gcc_add_static("vf25x", 5, 16441, 0);
    gcc_add_static("vf26x", 5, 16442, 0);
    gcc_add_static("vf27x", 5, 16443, 0);
    gcc_add_static("vf28x", 5, 16444, 0);
    gcc_add_static("vf29x", 5, 16445, 0);
    gcc_add_static("vf30x", 5, 16446, 0);
    gcc_add_static("vf31x", 5, 16447, 0);
    gcc_add_static("vf0y", 5, 16448, 0);
    gcc_add_static("vf1y", 5, 16449, 0);
    gcc_add_static("vf2y", 5, 16450, 0);
    gcc_add_static("vf3y", 5, 16451, 0);
    gcc_add_static("vf4y", 5, 16452, 0);
    gcc_add_static("vf5y", 5, 16453, 0);
    gcc_add_static("vf6y", 5, 16454, 0);
    gcc_add_static("vf7y", 5, 16455, 0);
    gcc_add_static("vf8y", 5, 16456, 0);
    gcc_add_static("vf9y", 5, 16457, 0);
    gcc_add_static("vf00y", 5, 16448, 0);
    gcc_add_static("vf01y", 5, 16449, 0);
    gcc_add_static("vf02y", 5, 16450, 0);
    gcc_add_static("vf03y", 5, 16451, 0);
    gcc_add_static("vf04y", 5, 16452, 0);
    gcc_add_static("vf05y", 5, 16453, 0);
    gcc_add_static("vf06y", 5, 16454, 0);
    gcc_add_static("vf07y", 5, 16455, 0);
    gcc_add_static("vf08y", 5, 16456, 0);
    gcc_add_static("vf09y", 5, 16457, 0);
    gcc_add_static("vf10y", 5, 16458, 0);
    gcc_add_static("vf11y", 5, 16459, 0);
    gcc_add_static("vf12y", 5, 16460, 0);
    gcc_add_static("vf13y", 5, 16461, 0);
    gcc_add_static("vf14y", 5, 16462, 0);
    gcc_add_static("vf15y", 5, 16463, 0);
    gcc_add_static("vf16y", 5, 16464, 0);
    gcc_add_static("vf17y", 5, 16465, 0);
    gcc_add_static("vf18y", 5, 16466, 0);
    gcc_add_static("vf19y", 5, 16467, 0);
    gcc_add_static("vf20y", 5, 16468, 0);
    gcc_add_static("vf21y", 5, 16469, 0);
    gcc_add_static("vf22y", 5, 16470, 0);
    gcc_add_static("vf23y", 5, 16471, 0);
    gcc_add_static("vf24y", 5, 16472, 0);
    gcc_add_static("vf25y", 5, 16473, 0);
    gcc_add_static("vf26y", 5, 16474, 0);
    gcc_add_static("vf27y", 5, 16475, 0);
    gcc_add_static("vf28y", 5, 16476, 0);
    gcc_add_static("vf29y", 5, 16477, 0);
    gcc_add_static("vf30y", 5, 16478, 0);
    gcc_add_static("vf31y", 5, 16479, 0);
    gcc_add_static("vf0z", 5, 16512, 0);
    gcc_add_static("vf1z", 5, 16513, 0);
    gcc_add_static("vf2z", 5, 16514, 0);
    gcc_add_static("vf3z", 5, 16515, 0);
    gcc_add_static("vf4z", 5, 16516, 0);
    gcc_add_static("vf5z", 5, 16517, 0);
    gcc_add_static("vf6z", 5, 16518, 0);
    gcc_add_static("vf7z", 5, 16519, 0);
    gcc_add_static("vf8z", 5, 16520, 0);
    gcc_add_static("vf9z", 5, 16521, 0);
    gcc_add_static("vf00z", 5, 16512, 0);
    gcc_add_static("vf01z", 5, 16513, 0);
    gcc_add_static("vf02z", 5, 16514, 0);
    gcc_add_static("vf03z", 5, 16515, 0);
    gcc_add_static("vf04z", 5, 16516, 0);
    gcc_add_static("vf05z", 5, 16517, 0);
    gcc_add_static("vf06z", 5, 16518, 0);
    gcc_add_static("vf07z", 5, 16519, 0);
    gcc_add_static("vf08z", 5, 16520, 0);
    gcc_add_static("vf09z", 5, 16521, 0);
    gcc_add_static("vf10z", 5, 16522, 0);
    gcc_add_static("vf11z", 5, 16523, 0);
    gcc_add_static("vf12z", 5, 16524, 0);
    gcc_add_static("vf13z", 5, 16525, 0);
    gcc_add_static("vf14z", 5, 16526, 0);
    gcc_add_static("vf15z", 5, 16527, 0);
    gcc_add_static("vf16z", 5, 16528, 0);
    gcc_add_static("vf17z", 5, 16529, 0);
    gcc_add_static("vf18z", 5, 16530, 0);
    gcc_add_static("vf19z", 5, 16531, 0);
    gcc_add_static("vf20z", 5, 16532, 0);
    gcc_add_static("vf21z", 5, 16533, 0);
    gcc_add_static("vf22z", 5, 16534, 0);
    gcc_add_static("vf23z", 5, 16535, 0);
    gcc_add_static("vf24z", 5, 16536, 0);
    gcc_add_static("vf25z", 5, 16537, 0);
    gcc_add_static("vf26z", 5, 16538, 0);
    gcc_add_static("vf27z", 5, 16539, 0);
    gcc_add_static("vf28z", 5, 16540, 0);
    gcc_add_static("vf29z", 5, 16541, 0);
    gcc_add_static("vf30z", 5, 16542, 0);
    gcc_add_static("vf31z", 5, 16543, 0);
    gcc_add_static("vf0w", 5, 16544, 0);
    gcc_add_static("vf1w", 5, 16545, 0);
    gcc_add_static("vf2w", 5, 16546, 0);
    gcc_add_static("vf3w", 5, 16547, 0);
    gcc_add_static("vf4w", 5, 16548, 0);
    gcc_add_static("vf5w", 5, 16549, 0);
    gcc_add_static("vf6w", 5, 16550, 0);
    gcc_add_static("vf7w", 5, 16551, 0);
    gcc_add_static("vf8w", 5, 16552, 0);
    gcc_add_static("vf9w", 5, 16553, 0);
    gcc_add_static("vf00w", 5, 16544, 0);
    gcc_add_static("vf01w", 5, 16545, 0);
    gcc_add_static("vf02w", 5, 16546, 0);
    gcc_add_static("vf03w", 5, 16547, 0);
    gcc_add_static("vf04w", 5, 16548, 0);
    gcc_add_static("vf05w", 5, 16549, 0);
    gcc_add_static("vf06w", 5, 16550, 0);
    gcc_add_static("vf07w", 5, 16551, 0);
    gcc_add_static("vf08w", 5, 16552, 0);
    gcc_add_static("vf09w", 5, 16553, 0);
    gcc_add_static("vf10w", 5, 16554, 0);
    gcc_add_static("vf11w", 5, 16555, 0);
    gcc_add_static("vf12w", 5, 16556, 0);
    gcc_add_static("vf13w", 5, 16557, 0);
    gcc_add_static("vf14w", 5, 16558, 0);
    gcc_add_static("vf15w", 5, 16559, 0);
    gcc_add_static("vf16w", 5, 16560, 0);
    gcc_add_static("vf17w", 5, 16561, 0);
    gcc_add_static("vf18w", 5, 16562, 0);
    gcc_add_static("vf19w", 5, 16563, 0);
    gcc_add_static("vf20w", 5, 16564, 0);
    gcc_add_static("vf21w", 5, 16565, 0);
    gcc_add_static("vf22w", 5, 16566, 0);
    gcc_add_static("vf23w", 5, 16567, 0);
    gcc_add_static("vf24w", 5, 16568, 0);
    gcc_add_static("vf25w", 5, 16569, 0);
    gcc_add_static("vf26w", 5, 16570, 0);
    gcc_add_static("vf27w", 5, 16571, 0);
    gcc_add_static("vf28w", 5, 16572, 0);
    gcc_add_static("vf29w", 5, 16573, 0);
    gcc_add_static("vf30w", 5, 16574, 0);
    gcc_add_static("vf31w", 5, 16575, 0);
    gcc_add_static("I", 6, 1, 0);
    gcc_add_static("Q", 6, 2, 0);
    gcc_add_static("R", 6, 4, 0);
    gcc_add_static("ACC", 6, 3, 0);
    gcc_add_static("acc", 6, 3, 0);
}

void fn_004944A0(char* name) {
    const char* token;
    GCCNameInfo* mnemonic;
    const char* format;
    SInt32 i;

    i = 0;
    while (tkidentifier->name[i]) {
        name[i] = tolower((signed char)tkidentifier->name[i]);
        i++;
    }
    name[i] = 0;
    asm_instruction_suffix_63737D = 0;
    asm_instruction_extra_6373A9 = 0;
    mnemonic = gcc_find_name(name);
    if (mnemonic) {
        format = opcode_info[mnemonic->operands[0]].format;
        if (*format == '&') {
            if (lookahead() == '.') {
                tk = lex();
                tk = lex();
                if (tk != -3) {
                    CError_Error(8);
                }
                token = tkidentifier->name;
                while (*token) {
                    if (*token == 'x') {
                        asm_instruction_suffix_63737D |= 8;
                        token++;
                    } else if (*token == 'y') {
                        asm_instruction_suffix_63737D |= 4;
                        token++;
                    } else if (*token == 'z') {
                        asm_instruction_suffix_63737D |= 2;
                        token++;
                    } else if (*token == 'w') {
                        asm_instruction_suffix_63737D |= 1;
                        token++;
                    } else {
                        CError_Error(8);
                    }
                }
            } else {
                asm_instruction_suffix_63737D = 15;
            }
            return;
        }
    }
    while (lookahead() == '.') {
        tk = lex();
        strcat(name, ".");
        tk = lex();
        if (tk != -3) {
            CError_Error(8);
        }
        i = 0;
        while (tkidentifier->name[i]) {
            tkidentifier->name[i] = tolower((signed char)tkidentifier->name[i]);
            i++;
        }
        strcat(name, tkidentifier->name);
    }
}

InlineAsmMIPSRecord* fn_004946C0(UInt16 opcode) {
    OpcodeInfo* info = &opcode_info[opcode];
    InlineAsmMIPSRecord* ia;
    InlineAsmMIPSOperand* operand;
    const char* format;
    UInt32 effect;
    UInt8 paired;
    UInt8 indirect;
    UInt8 quoted;
    SInt8 cls;
    UInt16 constraint;
    SInt32 high;
    SInt32 low;
    UInt32 encoding;
    CInt64 constant;

    if (opcode >= 0x37E && opcode <= 0x3E5) {
        InlineAsm_SyntaxError(0xA2);
    }
    if (asm_macro_flag_63735B && opcode >= 0x40A) {
        MIPSError_Warning(1);
    }
    if (!asm_noat_flag_637374) {
        if ((info->flags & 0x400000) && copts.warn_inlineasm) {
            MIPSError_Warning(3);
        }
    } else if (asm_noat_flag_637374 == 1 && (info->flags & 0x400000) && copts.warn_inlineasm) {
        MIPSError_Error(2);
    }

    ia = lalloc(sizeof(InlineAsmMIPSRecord) + 20 * info->operandCount);
    ia->opcode = opcode;
    ia->flags = (asm_flag_637388 || cmach_context_flag_63736A) ? 4 : 0;
    ia->argcount = 0;
    cg_codegen_reserved_635C08 = 0;
    format = info->format;
    operand = ia->args;
    while (*format) {
        InlineAsm_SkipComment();
        paired = FALSE;
        indirect = FALSE;
        quoted = FALSE;
        if (*format == ',') {
            if (tk != ',') {
                InlineAsm_SyntaxError(0x11);
            }
            if (inline_asm_setjmp_valid && format[1] == 'p') {
                asm_lexer_p_63739F = TRUE;
                tk = lex();
                asm_lexer_p_63739F = FALSE;
            } else {
                tk = lex();
            }
            format++;
        }
        effect = 1;
        if (*format == '=') {
            effect = 2;
            format++;
        } else if (*format == '+') {
            effect = 3;
            format++;
        }
        if (*format == '!') {
            effect |= 0xFFFF8000;
            format++;
            if (copts.unknown1A) {
                effect |= 0x20;
            }
        }
        if (*format == '$') {
            effect |= 0x2000;
            format++;
        }
        if (*format == '2') {
            paired = TRUE;
            format++;
            if (*format == '+') {
                indirect = TRUE;
                format++;
            }
        }
        if (*format == '\'') {
            quoted = TRUE;
            format++;
        } else if (*format == ';') {
            break;
        }

        switch (*format) {
            case '&':
                operand->kind = 0;
                CInt64_SetLong((CInt64*)&operand->value, (SInt8)asm_instruction_suffix_63737D);
                ia->argcount++;
                operand++;
                goto next_format;
            case '(':
                if (tk != '(') {
                    InlineAsm_SyntaxError(15);
                }
                tk = lex();
                goto next_format;
            case ')':
                if (tk != ')') {
                    InlineAsm_SyntaxError(16);
                }
                tk = lex();
                goto next_format;
            case '-':
                if (tk != 0x171) {
                    InlineAsm_SyntaxError(45);
                }
                tk = lex();
                goto next_format;
            case '|':
                if (tk != 0x170) {
                    InlineAsm_SyntaxError(45);
                }
                tk = lex();
                goto next_format;
            case '9':
                fn_004950D0(operand, 4, 0x1B, (SInt16)effect);
                ia->argcount++;
                operand++;
                goto next_format;
            case '<':
            case 'k':
            case 'X':
                fn_00495A00(operand, 0, 31);
                ia->argcount++;
                operand++;
                goto next_format;
            case '>':
                if ((UInt16)(opcode - 0x3EE) < 2) {
                    low = 0;
                    high = 31;
                } else {
                    low = -16;
                    high = 15;
                }
                fn_00495A00(operand, low, high);
                ia->argcount++;
                operand++;
                goto next_format;
            case '@':
                fn_00495A00(operand, 1, 32);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'A':
                if (paired) {
                    fn_004950D0(operand, 6, 3, (SInt16)effect);
                } else {
                    fn_00495730(operand, FALSE, (SInt16)effect);
                }
                ia->argcount++;
                operand++;
                goto next_format;
            case 'B':
                fn_00495A00(operand, 0, 0xFFFFF);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'C':
                fn_00495A00(operand, 0, 0x1FFFFFF);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'N':
                encoding = info->encoding & 3;
                if (encoding == 0) {
                    asm_instruction_extra_6373A9 |= 8;
                } else if (encoding == 1) {
                    asm_instruction_extra_6373A9 |= 4;
                } else if (encoding == 2) {
                    asm_instruction_extra_6373A9 |= 2;
                } else if (encoding == 3) {
                    asm_instruction_extra_6373A9 |= 1;
                }
            case 'D':
            case 'E':
            case 'S':
            case 'T':
            case 'V':
            case 'W':
                constraint = quoted ? 0xFFFF : 0xFFFD;
                cls = paired ? 5 : 1;
                fn_004950D0(operand, cls, constraint, (SInt16)effect);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'G':
                fn_004950D0(operand, 0, 0xFFFF, (SInt16)effect);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'I':
                if (paired) {
                    fn_004950D0(operand, 6, 1, (SInt16)effect);
                } else {
                    operand->kind = 0;
                    constant = InlineAsm_ConstantExpression64bit();
                    *(CInt64*)&operand->value = constant;
                }
                ia->argcount++;
                operand++;
                goto next_format;
            case 'J':
            case 'L':
                fn_004950D0(operand, 6, 0xFFFD, (SInt16)effect);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'P':
                fn_004950D0(operand, 0, 0xFFFE, (SInt16)effect);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'Q':
            case 'R':
                if (paired) {
                    fn_004950D0(operand, 6, (UInt16)(*format == 'Q' ? 2 : 4), (SInt16)effect);
                } else {
                    goto next_format;
                }
                ia->argcount++;
                operand++;
                goto next_format;
            case 'U':
            case 'o':
                fn_00495730(operand, TRUE, (SInt16)effect);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'a':
            case 'q':
                fn_00494EC0(operand, 1);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'b':
                if (!paired) {
                    CError_FATAL(0x81A);
                }
                fn_00494EC0(operand, 1);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'c':
                fn_00495A00(operand, 0, 0x3FF);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'f':
                fn_00495930(operand);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'i':
                fn_00495A00(operand, 0, paired ? 0x3FFFF : 0xFFFF);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'j':
                fn_00495A00(operand, -32768, 32767);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'l':
            case 'p':
                fn_00494EC0(operand, 0);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'v':
                fn_00495A00(operand, 0, 16);
                ia->argcount++;
                operand++;
                goto next_format;
            case 'x':
                if (!paired) {
                    CError_FATAL(0x7C7);
                }
            case 'd':
            case 's':
            case 't':
                cls = !paired ? 0 : (indirect ? 4 : 3);
                fn_004950D0(operand, cls, 0xFFFF, (SInt16)effect);
                ia->argcount++;
                operand++;
                goto next_format;
            default:
                CError_FATAL(0x844);
                ia->argcount++;
                operand++;
                goto next_format;
        }
next_format:
        format++;
    }
    if (tk == ',') {
        InlineAsm_SyntaxError(0xA2);
    }
    if (cg_codegen_reserved_635C08) {
        ia->flags |= 8;
    }
    return ia;
}

void fn_00494EC0(InlineAsmMIPSOperand* operand, SInt32 label_allowed) {
    InlineAsmMIPSLookupResult lookup;
    HashNameNode* name;
    CLabel* label;
    char buffer[80];
    UInt8 forward;
    char suffix;

    if (tk == -1) {
        name = MakeLocalLabel(tkintconst);
        tk = lex();
        if (tk != -3) {
            InlineAsm_SyntaxError(45);
        }
        suffix = tkidentifier->name[0];
        if (suffix == 'b' || suffix == 'B') {
            forward = FALSE;
        } else if (suffix == 'f' || suffix == 'F') {
            forward = TRUE;
        } else {
            CError_Error(45);
        }
        if (tkidentifier->name[1]) {
            CError_Error(45);
        }
        tk = lex();
        label = InlineAsm_LookupLabel(name);
        if (forward) {
            if (label && label->stmt) {
                sprintf(buffer, "%i", tkintconst.lo);
                CError_Error(72, buffer);
            }
            if (!label) {
                label = InlineAsm_DeclareLabel(name);
            }
        } else if (!label || !label->stmt) {
            sprintf(buffer, "%i", tkintconst.lo);
            CError_Error(60, buffer);
        }
        operand->kind = 4;
        operand->label = label;
    } else if (tk == -3) {
        if (!InlineAsm_LookupSymbolOrTag(tkidentifier, &lookup)) {
            lookup.label = InlineAsm_DeclareLabel(tkidentifier);
        }
        if (lookup.label) {
            operand->kind = 4;
            operand->label = lookup.label;
        } else if ((UInt8)label_allowed && lookup.object && lookup.object->datatype == DFUNC) {
            operand->kind = 2;
            operand->object = lookup.object;
            operand->value = 0;
        } else {
            CError_Error(45);
        }
        tk = lex();
    } else {
        InlineAsm_SyntaxError(45);
    }
}

void fn_004950D0(InlineAsmMIPSOperand* operand, SInt8 register_type, SInt32 mode, SInt32 effect) {
    CInt64 value = cint64_zero;
    Boolean numbered = FALSE;
    Boolean suffix = FALSE;
    Object* object = NULL;
    char name[256];
    char* pos;
    GCCStaticInfo* result;
    UInt32 number;

    if (mode == -2) {
        numbered = TRUE;
    } else if (mode == 3 && register_type == 6) {
        suffix = TRUE;
    } else if (mode == -3 && register_type == 5) {
        suffix = TRUE;
    }
    if (tk == '$') {
        numbered = TRUE;
        tk = lex();
        if (!asm_noat_flag_637374 &&
            (!memcmp(tkidentifier->name, "at", 3) || !memcmp(tkidentifier->name, "AT", 3))) {
            MIPSError_Warning(3);
        }
    } else if (tk == '%' && inline_asm_setjmp_valid) {
        tk = lex();
        if (tk == -1) {
            CInt64 integer = tkintconst;
            number = integer.lo;
            if ((SInt32)number < 0) {
                CError_Error(45);
            }
        } else {
            CError_Error(45);
        }
        cg_codegen_reserved_635C08 = TRUE;
        operand->kind = 5;
        operand->value = number;
        operand->registerOptions.operandMode = effect;
        operand->registerOptions.registerClass = register_type;
        operand->componentMask = 0;
        tk = lex();
        if (tk == -3) {
            if (tkidentifier->name[0] == 'x') {
                operand->componentMask |= 1;
            } else if (tkidentifier->name[0] == 'y') {
                operand->componentMask |= 2;
            } else if (tkidentifier->name[0] == 'z') {
                operand->componentMask |= 4;
            } else if (tkidentifier->name[0] == 'w') {
                operand->componentMask |= 8;
            } else {
                CError_Error(45);
            }
            if (operand->componentMask) {
                tk = lex();
            }
        }
        return;
    }
    if (tk == -3) {
        strcpy(name, tkidentifier->name);
        if (suffix) {
            pos = name;
            while (*pos) {
                pos++;
            }
            pos--;
            while (pos > name) {
                if (asm_instruction_extra_6373A9) {
                    if (*pos == 'x' && (asm_instruction_extra_6373A9 & 8)) {
                        *pos = 0;
                    } else if (*pos == 'y' && (asm_instruction_extra_6373A9 & 4)) {
                        *pos = 0;
                    } else if (*pos == 'z' && (asm_instruction_extra_6373A9 & 2)) {
                        *pos = 0;
                    } else if (*pos == 'w' && (asm_instruction_extra_6373A9 & 1)) {
                        *pos = 0;
                    } else {
                        if ((UInt8)(*pos - 'w') < 4) {
                            CError_Error(45);
                        }
                        break;
                    }
                } else if (asm_instruction_suffix_63737D) {
                    if (*pos == 'x' && (asm_instruction_suffix_63737D & 8)) {
                        *pos = 0;
                    } else if (*pos == 'y' && (asm_instruction_suffix_63737D & 4)) {
                        *pos = 0;
                    } else if (*pos == 'z' && (asm_instruction_suffix_63737D & 2)) {
                        *pos = 0;
                    } else if (*pos == 'w' && (asm_instruction_suffix_63737D & 1)) {
                        *pos = 0;
                    } else {
                        if ((UInt8)(*pos - 'w') < 4) {
                            CError_Error(45);
                        }
                        break;
                    }
                } else {
                    if ((UInt8)(*pos - 'w') >= 4) {
                        break;
                    }
                    *pos = 0;
                }
                pos--;
            }
        }
    }
    if (tk == -3 && (result = gcc_find_value(name)) && result->value == register_type) {
        operand->kind = 1;
        operand->object = result->object;
        operand->value = result->rclass;
        operand->extendedRegisterOptions.operandMode = effect;
        operand->extendedRegisterOptions.registerClass = register_type;
        operand->extendedComponentMask = 0;
        if (result->object) {
            result->object->flags |= 1;
        }
        tk = lex();
    } else if (tk == -1) {
        CInt64 integer = tkintconst;
        number = integer.lo;
        tk = lex();
        if (number == 0 && tk == -1) {
            CInt64 nextInteger = tkintconst;
            number = nextInteger.lo;
            tk = lex();
        }
        if (numbered && (SInt32)number < n_real_registers[register_type]) {
            operand->kind = 1;
            operand->object = NULL;
            operand->value = number;
            operand->extendedRegisterOptions.operandMode = effect;
            operand->extendedRegisterOptions.registerClass = register_type;
        } else {
            InlineAsm_SyntaxError(45);
        }
    } else if (tk == -3 && gcc_find_value(name)) {
        InlineAsm_SyntaxError(45);
    } else if (!CExpr_ParseAsmExpr(&object, &value)) {
        InlineAsm_SyntaxError(45);
    } else if (!object) {
        InlineAsm_SyntaxError(45);
    } else {
        cg_codegen_reserved_635C08 = TRUE;
        operand->kind = 1;
        operand->object = object;
        operand->value = 0;
        operand->extendedRegisterOptions.operandMode = effect;
        operand->extendedRegisterOptions.registerClass = register_type;
        operand->extendedComponentMask = 0;
        object->flags |= 1;
    }
}

void fn_00495730(InlineAsmMIPSOperand* operand, UInt8 allow_immediate, SInt16 effect) {
    InlineAsmMIPSLookupResult lookup;
    Object* object;
    CInt64 value;

    if (tk == -3) {
        if (InlineAsm_LookupSymbolOrTag(tkidentifier, &lookup)) {
            object = (Object*)lookup.object;
            if (object) {
                if (object->datatype == DLOCAL) {
                    operand->kind = 3;
                    object->u.var.info->noregister = 1;
                    object->u.var.info->used = 1;
                } else {
                    operand->kind = 2;
                    operand->unkC = effect;
                    operand->unk10 = 0;
                }
                ((Object*)lookup.object)->u.unk30 = 0;
                operand->unk8 = (UInt32)lookup.object;
                operand->value = 0;
                ((Object*)lookup.object)->flags |= OBJECT_USED;
                tk = lex();
                if (tk == '.') {
                    operand->value += InlineAsm_StructMemberOffset(object->type);
                }
                if (tk == '+') {
                    tk = lex();
                    operand->value += InlineAsm_ConstantExpression32bit();
                } else if (tk == '-') {
                    tk = lex();
                    operand->value -= InlineAsm_ConstantExpression32bit();
                }
                return;
            }
            if (lookup.label) {
                tk = lex();
                operand->kind = 4;
                operand->value = (UInt32)lookup.label;
                return;
            }
        }
    }

    if (tk == '%' && inline_asm_setjmp_valid) {
        tk = lex();
        if (tk == -1) {
            value = tkintconst;
            if (value.lo < 0) {
                CError_Error(45);
            }
        } else {
            CError_Error(45);
        }
        operand->kind = 5;
        operand->value = value.lo;
        operand->registerOptions.operandMode = effect;
        operand->registerOptions.registerClass = -1;
        tk = lex();
        return;
    }
    if (allow_immediate) {
        fn_00495A00(operand, -32768, 32767);
    } else {
        InlineAsm_SyntaxError(45);
    }
}

void fn_00495930(InlineAsmMIPSOperand* operand) {
    float value;
    UInt8 negate = 0;
    if (tk == '-') {
        negate = 1;
        tk = lex();
    } else if (tk == '+') {
        tk = lex();
    }
    if (tk == -2) {
        value = (float)asm_tkfloat_623D58;
    } else if (tk == -1) {
        value = (float)(UInt32)codegen_reg_634814;
    } else if (tk != -2) {
        CError_Error(45);
        value = 0.0f;
    }
    if (negate) {
        *(UInt32*)&value ^= 0x80000000;
    }
    {
        SInt32 bits = *(SInt32*)&value;
        operand->kind = 0;
        CInt64_SetLong((CInt64*)&operand->value, bits);
    }
    tk = lex();
}

void fn_00495A00(InlineAsmMIPSOperand* operand, SInt32 low, SInt32 high) {
    CInt64 value;
    InlineAsmMIPSLookupResult lookup;
    UInt32 number = 0;
    UInt32 parsedNumber;
    char* name;
    Object* object;

    if (inline_asm_setjmp_valid && tk == '?') {
        tk = lex();
        if (tk == -1) {
            value = tkintconst;
            parsedNumber = value.lo;
            number = parsedNumber;
            if ((SInt32)number < 0) {
                InlineAsm_SyntaxError(45);
            }
        } else {
            InlineAsm_SyntaxError(45);
        }
        operand->kind = 6;
        operand->value = number;
        tk = lex();
        return;
    }

    if (!inline_asm_setjmp_valid || tk != '%') {
        goto parse_constant;
    }

    tk = lex();
    if (tk == -3) {
        name = tkidentifier->name;
        if (memcmp(name, "hi", 3) != 0 && memcmp(name, "lo", 3) != 0) {
            goto done;
        }
        if (memcmp(name, "hi", 3) == 0) {
            asm_option_flags_637336 = 1;
        } else {
            asm_option_flags_637336 = 2;
        }
        tk = lex();
        if (tk == '(') {
            tk = lex();
            if (tk == -1) {
                operand->kind = 0;
                if (asm_option_flags_637336 & 1) {
                    number = (UInt32)tkintconst.lo >> 16;
                } else {
                    number = tkintconst.lo & 0xFFFF;
                }
            } else if (tk == -3) {
                InlineAsm_LookupSymbolOrTag(tkidentifier, &lookup);
                if (lookup.object == NULL) {
                    InlineAsm_SyntaxError(45);
                }
                object = lookup.object;
                if (object->datatype == DDATA && (object->qual & 0x10000)) {
                    if (asm_option_flags_637336 & 1) {
                        number = (UInt32)object->u.data.unk34 >> 16;
                    } else {
                        number = (UInt16)object->u.data.unk34;
                    }
                } else {
                    InlineAsm_SyntaxError(45);
                }
            } else {
                InlineAsm_SyntaxError(45);
            }
            CInt64_SetLong((CInt64*)&operand->value, number);
            tk = lex();
            if (tk == ')') {
                tk = lex();
            } else {
                InlineAsm_SyntaxError(16);
            }
            return;
        }
    } else if (tk == -1) {
        value = tkintconst;
        number = value.lo;
        if ((SInt32)number < 0) {
            CError_Error(45);
        }
        cg_codegen_reserved_635C08 = 1;
        operand->kind = 6;
        operand->value = number;
        tk = lex();
    }
done:
    return;

parse_constant:
    value = InlineAsm_ConstantExpression64bit();
    if (CInt64_IsInRange(value, 4) && value.lo >= low && value.lo <= high) {
        operand->kind = 0;
        *(CInt64*)&operand->value = value;
        return;
    }
    InlineAsm_SyntaxError(55);
    goto done;
}
