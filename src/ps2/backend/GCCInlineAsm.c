#include "compiler/GCCInlineAsm.h"
#include "compiler/RegListQueries.h"
#include "compiler/CPrep.h"
#include "compiler/CPrepTokenizer.h"
#include "compiler/CFunc.h"
#include "compiler/CExpr.h"
#include "compiler/CParser.h"
#include "compiler/CError.h"
#include "compiler/CompilerTools.h"
#include "compiler/MIPSError.h"
#include "compiler/Registers.h"
#include "compiler/InlineAsmMIPS.h"
#include "compiler/InlineAsm.h"
#include "compiler/InlineAsmMIPS.h"
#include "compiler/BE_macro.h"
#include "compiler/PCodeInfo.h"
#include "compiler/Alias.h"
#include "compiler/tokens.h"
#include "compiler/objects.h"
#include <string.h>

//! @reconstructed
static void gcc_parse_attribute(SInt32* result, SInt8 isOutput) {
    volatile UInt8 attribute[4];
    volatile SInt32 flags;
    volatile SInt32 regnum;
    flags = 0;
    attribute[0] = 0xFF;
    attribute[1] = 0;
    regnum = -1;

    while (tk == -7) {
        tk = lex();
    }
    if (tk != '"') {
        CError_Error(6);
    }
    tk = lex();

    if (!isOutput) {
        while (tk == '$' || tk == '%' || tk == '&') {
            tk = lex();
        }
        flags |= 1;
        attribute[1] = 2;
    } else if (tk == '+') {
        flags |= 3;
        attribute[1] = 3;
        tk = lex();
        while (tk == '&' || tk == '%' || tk == '$') {
            tk = lex();
        }
    } else if (tk == '=') {
        flags |= 2;
        attribute[1] = 1;
        tk = lex();
        while (tk == '&' || tk == '%' || tk == '$') {
            tk = lex();
        }
    } else {
        CError_Error(6);
    }

    if (tk != ')') {
        if (tk == -1) {
            regnum = tkintconst.lo;
            if (regnum < 0) {
                CError_Error(0x2D);
            }
        } else if (tk == -3) {
            switch (tkidentifier->name[0]) {
                case 'I':
                case 'i':
                    attribute[0] = 0;
                    break;
                case 'n':
                    attribute[0] = 1;
                    break;
                case 'g':
                case 'r':
                    attribute[0] = 2;
                    break;
                case 'f':
                    attribute[0] = 3;
                    break;
                case 'j':
                    attribute[0] = 4;
                    break;
                case 'm':
                    attribute[0] = 5;
                    break;
                case 'o':
                    attribute[0] = 6;
                    break;
                default:
                    CError_Error(6);
                    attribute[0] = -1;
                    break;
            }
            if (attribute[0] != -1) {
                copts.cplusplus = inline_asm_saved_slash_comment;
            }
            if (attribute[0] == 4) {
                copts.enable_vu0_registers = 1;
            }
        } else {
            CError_Error(6);
        }
        tk = lex();
        if (tk == '&') {
            if (!isOutput) {
                CError_Error(6);
            }
            tk = lex();
        }
        if (tk != '"') {
            CError_Error(6);
        }
        tk = lex();
    } else {
        attribute[0] = -1;
        attribute[1] = 0;
        flags = 0;
        regnum = -1;
    }
    result[0] = attribute[0] | ((UInt32)attribute[1] << 8);
    result[1] = flags;
    result[2] = regnum;
}

void gcc_replace_arg_st_asm(Statement* stmt) {
    InlineAsmMIPSRecord* ia;
    InlineAsmMIPSOperand* arg;
    GCCAsmOperand* operand;
    GCCAsmConstraint* constraint;
    Object* object;
    SInt32 reg;
    SInt32 index;
    SInt16 mapped;
    InlineAsmMIPSRecord* copied;
    UInt16 mode;
    SInt16 rclass;
    UInt8 component;

    ia = (InlineAsmMIPSRecord*)stmt->expr;
    if (!ia || ia->argcount <= 0) {
        return;
    }

    for (index = 0, arg = ia->args; index < ia->argcount; index++, arg++) {
        switch (arg->kind) {
            case 5:
                reg = arg->value;
                constraint = &gcc_asm_constraints[reg];
                if (constraint->registerClass == 5) {
                    if ((opcode_info[ia->opcode].flags & 6) && ia->opcode > 0x40A) {
                        mapped = be_macro_expanded_opcode_6371D0[ia->opcode - 0x40A];
                        copied = lalloc(
                            sizeof(InlineAsmMIPSRecord) +
                            opcode_info[(UInt16)mapped].operandCount * sizeof(InlineAsmMIPSOperand)
                        );
                        memset(
                            copied,
                            0,
                            sizeof(InlineAsmMIPSRecord) +
                                opcode_info[(UInt16)mapped].operandCount *
                                    sizeof(InlineAsmMIPSOperand)
                        );
                        copied->opcode = mapped;
                        copied->flags = ia->flags;
                        copied->argcount = opcode_info[(UInt16)mapped].operandCount;
                        copied->args[0] = ia->args[0];
                        copied->args[1].kind = 0;
                        *((CInt64*)&copied->args[1].value) = cint64_zero;
                        copied->args[2] = ia->args[1];
                        stmt->expr = (ENode*)copied;
                        gcc_replace_arg_st_asm(stmt);
                        return;
                    }
                }
                operand = &gcc_asm_operands[reg];
                object = operand->object;
                if (!object || !object->type) {
                    if (operand->isImmediate) {
                        arg->kind = 0;
                        *((CInt64*)&arg->value) = operand->immediate;
                    } else {
                        CError_Error(0x2D);
                    }
                    break;
                }
                mode = arg->registerOptions.operandMode;
                rclass = arg->registerOptions.registerClass;
                component = arg->componentMask;
                arg->kind = 1;
                arg->value = 0;
                if (constraint->registerClass == 4 && reg <= gcc_asm_operand_count) {
                    arg->extendedComponentMask = component;
                }
                if ((mode & 3) == 3 && !(constraint->effects & 1)) {
                    mode &= ~1;
                }
                if (constraint->kind == 3 && !(mode & 1)) {
                    mode |= 1;
                }
                arg->extendedRegisterOptions.operandMode = mode;
                arg->extendedRegisterOptions.registerClass = rclass;
                arg->unk8 = 0;
                if (reg > gcc_asm_operand_count) {
                    MIPSError_Error(4, tk_stmt_name_5EA329, reg);
                }
                if (rclass != -1 && operand->object) {
                    if (constraint->registerClass == 2 && rclass != 0) {
                        MIPSError_Error(
                            8, tk_stmt_name_5EA329, reg, InlineAsm_GetMnemonic((SInt32*)&ia->opcode)
                        );
                    } else if (constraint->registerClass == 3 && rclass != 1) {
                        MIPSError_Error(
                            8, tk_stmt_name_5EA329, reg, InlineAsm_GetMnemonic((SInt32*)&ia->opcode)
                        );
                    } else if (constraint->registerClass == 4 && rclass != 5) {
                        MIPSError_Error(
                            8, tk_stmt_name_5EA329, reg, InlineAsm_GetMnemonic((SInt32*)&ia->opcode)
                        );
                    } else if (constraint->registerClass == 5 && rclass != 0) {
                        MIPSError_Error(
                            8, tk_stmt_name_5EA329, reg, InlineAsm_GetMnemonic((SInt32*)&ia->opcode)
                        );
                    }
                }
                if (constraint->kind == 1 || constraint->kind == 3) {
                    Object* original = object;
                    Object* temp = object;
                    ENode* left;
                    ENode* right;
                    Statement* inserted;
                    VarInfo* info;
                    if (object->datatype == DDATA) {
                        temp = create_temp_object((Type*)&stsignedint);
                        info = temp->u.var.info;
                        if (info) {
                            info->used = TRUE;
                        }
                        left = create_objectnode(original);
                        right = create_objectnode(temp);
                        left = makediadicnode(left, right, EASS);
                        inserted = CFunc_InsertStatement(ST_EXPRESSION, stmt);
                        inserted->expr = left;
                        if (curstmt == stmt) {
                            curstmt = inserted;
                        }
                    } else if (constraint->registerClass == 3 && object->type->type == TYPEINT) {
                        temp = create_temp_object((Type*)&stfloat);
                        info = temp->u.var.info;
                        if (info) {
                            info->used = TRUE;
                        }
                        if (arg->extendedRegisterOptions.operandMode & 2) {
                            fn_00490EA0(stmt, original, temp, FALSE, TRUE);
                        }
                        if (arg->extendedRegisterOptions.operandMode & 1) {
                            fn_00490EA0(stmt, original, temp, TRUE, FALSE);
                            stmt = stmt->next;
                        }
                    } else if (constraint->registerClass == 2 &&
                               object->type->type == TYPEFLOAT &&
                               object->type->size == 4) {
                        temp = create_temp_object((Type*)&stsignedint);
                        if (arg->extendedRegisterOptions.operandMode & 2) {
                            fn_00490EA0(stmt, temp, original, TRUE, TRUE);
                        }
                        if (arg->extendedRegisterOptions.operandMode & 1) {
                            fn_00490EA0(stmt, temp, original, FALSE, FALSE);
                            stmt = stmt->next;
                        }
                    }
                    if (temp->u.var.info) {
                        temp->u.var.info->used = TRUE;
                    }
                    arg->unk8 = (UInt32)temp;
                    if (constraint->kind == 1) {
                        constraint->kind = 3;
                        constraint->effects |= 3;
                    }
                } else if (constraint->kind == 2 &&
                           object->datatype == DLOCAL &&
                           object->u.var.info) {
                    object->u.var.info->used = TRUE;
                }
                break;
            case 6:
                arg->kind = 0;
                break;
            default:
                break;
        }
    }
}

//! @reconstructed
static ENode* gcc_copy_expr(ENode* expr) {
    ENode* copy;

    copy = lalloc(sizeof(ENode));
    memset(copy, 0, sizeof(ENode));
    *copy = *expr;

    switch (copy->type) {
        case EPOSTINC:
        case EPOSTDEC:
        case EPREINC:
        case EPREDEC:
        case EMONMIN:
        case EBINNOT:
        case ELOGNOT:
        case EFORCELOAD:
        case ETYPCON:
        case EBITFIELD:
            copy->data.monadic = gcc_copy_expr(copy->data.monadic);
            break;
        case EMUL:
        case EMULV:
        case EDIV:
        case EMODULO:
        case EADDV:
        case ESUBV:
        case EADD:
        case ESUB:
        case ESHL:
        case ESHR:
        case ELESS:
        case EGREATER:
        case ELESSEQU:
        case EGREATEREQU:
        case EEQU:
        case ENOTEQU:
        case EAND:
        case EXOR:
        case EOR:
        case ELAND:
        case ELOR:
        case EASS:
        case EMULASS:
        case EDIVASS:
        case EMODASS:
        case EADDASS:
        case ESUBASS:
        case ESHLASS:
        case ESHRASS:
        case EANDASS:
        case EXORASS:
        case EORASS:
        case ECOMMA:
        case EMIN:
        case EMAX:
        case EPMODULO:
        case EROTL:
        case EROTR:
        case EBCLR:
        case EBTST:
        case EBSET:
        case EPOINTERSTAR:
        case EDOTSTAR:
            copy->data.diadic.left = gcc_copy_expr(copy->data.diadic.left);
            copy->data.diadic.right = gcc_copy_expr(copy->data.diadic.right);
            break;
        case ECOND:
            copy->data.cond.cond = gcc_copy_expr(copy->data.cond.cond);
            copy->data.cond.expr1 = gcc_copy_expr(copy->data.cond.expr1);
            copy->data.cond.expr2 = gcc_copy_expr(copy->data.cond.expr2);
            break;
        case EMFPOINTER:
            copy->data.mfpointer.accessnode = gcc_copy_expr(copy->data.mfpointer.accessnode);
            copy->data.mfpointer.mfpointer = gcc_copy_expr(copy->data.mfpointer.mfpointer);
            break;
        case EFUNCCALL:
        case EFUNCCALLP:
        case ENULLCHECK:
        case EPRECOMP:
            CError_Internal("GCCInlineAsm.c", 1134);
            break;
        case EINDIRECT:
            copy->data.monadic = gcc_copy_expr(copy->data.monadic);
            break;
        case ENEWEXCEPTION:
        case ENEWEXCEPTIONARRAY:
            copy->data.newexception.initexpr = gcc_copy_expr(copy->data.newexception.initexpr);
            copy->data.newexception.tryexpr = gcc_copy_expr(copy->data.newexception.tryexpr);
            break;
        case EINITTRYCATCH:
            copy->data.itc.initexpr = gcc_copy_expr(copy->data.itc.initexpr);
            copy->data.itc.tryexpr = gcc_copy_expr(copy->data.itc.tryexpr);
            copy->data.itc.catchexpr = gcc_copy_expr(copy->data.itc.catchexpr);
            copy->data.itc.result = gcc_copy_expr(copy->data.itc.result);
            break;
        case EINTCONST:
        case EFLOATCONST:
        case ESTRINGCONST:
        case EOBJREF:
        case ELABEL:
        case ETEMP:
        case ETEMPLDEP:
        case EINSTRUCTION:
            break;
        default:
            CError_Internal("GCCInlineAsm.c", 1158);
            break;
    }

    return copy;
}

void gcc_allocate_operand(SInt32 mode, SInt32 kindreg, SInt32 effects, SInt32 index) {
    Boolean consumed;
    SInt32 reg;
    SInt16 nextToken;
    GCCAsmOperand* operand;
    GCCAsmConstraint* constraint;
    ENode* expr;
    ENode* node;
    ENode* scan;
    Object* object;
    Statement* stmt;

    consumed = FALSE;
    while (tk == -7) {
        tk = lex();
    }
    if (tk != '(') {
        CError_Error(0xF);
    }
    nextToken = lookahead();

    if ((SInt8)mode) {
        gcc_asm_operand_count++;
        reg = gcc_asm_operand_count;
        if (reg >= 30) {
            CError_Internal("GCCInlineAsm.c", 0x152);
        }
        gcc_asm_constraints[reg].kind = (UInt8)(kindreg >> 8);
        gcc_asm_constraints[reg].effects = effects;
        gcc_asm_constraints[reg].registerClass = (SInt8)kindreg;
        constraint = &gcc_asm_constraints[reg];
        if (constraint->registerClass != 5) {
            consumed = TRUE;
        }
        if (constraint->registerClass == 5) {
            expr = expression();
            consumed = TRUE;
            if (!expr || expr->type != EINDIRECT) {
                if (nextToken == '&') {
                    MIPSError_Warning(0xD, tk_stmt_name_5EA32B);
                }
                gcc_asm_operands[reg].object = NULL;
                goto finish;
            }
            expr = expr->data.monadic;
            object = create_temp_object(expr->rtype);
            node = create_objectnode2(object);
            gcc_asm_operands[reg].object = object;
            node = makediadicnode(node, expr, EASS);
            stmt = CFunc_InsertBeforeStatement(ST_EXPRESSION, first_ST_ASM);
            first_ST_ASM = stmt->next;
            if (!first_ST_ASM->next) {
                curstmt = first_ST_ASM;
            }
            stmt->expr = node;
        } else {
            expr = expression();
            if (!gcc_asm_vector_type && constraint->registerClass == 4) {
                for (scan = expr; scan && scan->type == EINDIRECT; scan = scan->data.monadic) {
                    if (CABI_HasSideEffects(scan->rtype)) {
                        gcc_asm_vector_type = scan->rtype;
                    } else if (CABI_ExprHasSideEffects(scan->rtype)) {
                        gcc_asm_vector_type = TYPE_POINTER(scan->rtype)->target;
                    }
                    if (gcc_asm_vector_type) {
                        break;
                    }
                }
            }
            if (!expr) {
                goto finish;
            }
            if (constraint->registerClass == 4) {
                if (CABI_HasSideEffects(expr->rtype)) {
                    expr->rtype = (Type*)&stint128;
                } else if (expr->rtype->type != TYPEINT || expr->rtype->size != 16) {
                    MIPSError_Error(0xB);
                }
            }
            if (CABI_HasSideEffects(expr->rtype) && constraint->registerClass == 2) {
                expr->rtype = (Type*)&stint128;
            }
            object = create_temp_object(expr->rtype);
            node = create_objectnode(object);
            if (node->rtype != expr->rtype) {
                node->rtype = expr->rtype;
            }
            gcc_asm_operands[reg].object = object;
            if (effects & 1) {
                ENode* copy = gcc_copy_expr(expr);
                copy = makediadicnode(node, copy, EASS);
                stmt = CFunc_InsertBeforeStatement(ST_EXPRESSION, first_ST_ASM);
                stmt->expr = copy;
            }
            expr = makediadicnode(expr, node, EASS);
            curstmt = CFunc_InsertStatement(ST_EXPRESSION, curstmt);
            curstmt->expr = expr;
        }
        operand = &gcc_asm_operands[reg];
        object = operand->object;
        if (object) {
            object->flags |= OBJECT_USED;
            if (object->u.var.info) {
                object->u.var.info->used = TRUE;
            }
        }
    } else {
        tk = lex();
        cprep_in_assembler = FALSE;
        cprep_nostring = FALSE;
        copts.cplusplus = TRUE;

        if (index == -1 || gcc_asm_operand_count == -1) {
            gcc_asm_operand_count++;
            reg = gcc_asm_operand_count;
            operand = &gcc_asm_operands[reg];
            memset(operand, 0, sizeof(*operand));
            constraint = &gcc_asm_constraints[reg];
            constraint->kind = (UInt8)(kindreg >> 8);
            constraint->effects = effects;
            constraint->registerClass = (SInt8)kindreg;
        } else {
            reg = index;
            if (gcc_asm_operand_count < index) {
                CError_Error(0x2D);
            }
            operand = &gcc_asm_operands[reg];
            constraint = &gcc_asm_constraints[reg];
            constraint->effects |= 1;
            constraint->kind = 3;
        }

        expr = expression();
        if (constraint->registerClass == 0) {
            if (expr->type != EINTCONST) {
                CError_Error(0x2D);
            }
            operand->immediate = expr->data.intval;
            operand->isImmediate = TRUE;
        } else if (constraint->registerClass == 1) {
            if (expr->type != EINTCONST) {
                CError_Error(0x2D);
            }
            operand->immediate = expr->data.intval;
            operand->isImmediate = TRUE;
        } else {
            if (constraint->registerClass == 5) {
                if (!expr || expr->type != EINDIRECT) {
                    if (nextToken == '&') {
                        MIPSError_Warning(0xD, tk_stmt_name_5EA32B);
                    }
                    operand->object = NULL;
                    goto finish;
                }
                expr = expr->data.monadic;
            } else if (constraint->registerClass == 4 && !gcc_asm_vector_type) {
                for (scan = expr; scan && scan->type == EINDIRECT; scan = scan->data.monadic) {
                    if (CABI_HasSideEffects(scan->rtype)) {
                        gcc_asm_vector_type = scan->rtype;
                    } else if (CABI_ExprHasSideEffects(scan->rtype)) {
                        gcc_asm_vector_type = TYPE_POINTER(scan->rtype)->target;
                    }
                    if (gcc_asm_vector_type) {
                        break;
                    }
                }
            }

            if (operand->object) {
                object = operand->object;
                node = create_objectnode(object);
            } else {
                if (constraint->registerClass == 4 && gcc_asm_vector_type) {
                    if (CABI_HasSideEffects(expr->rtype)) {
                        expr->rtype = (Type*)&stint128;
                    } else if (expr->rtype->type != TYPEINT || expr->rtype->size != 16) {
                        MIPSError_Error(9, tk_stmt_name_5EA329, reg);
                    }
                }
                if (CABI_HasSideEffects(expr->rtype) && constraint->registerClass == 2) {
                    object = create_temp_object((Type*)&stint128);
                    node = create_objectnode(object);
                    expr->rtype = node->rtype;
                } else {
                    object = create_temp_object(expr->rtype);
                    node = create_objectnode(object);
                }
            }
            if (object->u.var.info) {
                object->u.var.info->used = TRUE;
            }
            expr = makediadicnode(node, expr, EASS);
            stmt = CFunc_InsertBeforeStatement(ST_EXPRESSION, first_ST_ASM);
            first_ST_ASM = stmt->next;
            if (!first_ST_ASM->next) {
                curstmt = first_ST_ASM;
            }
            stmt->expr = expr;
            operand->name = object->name;
            operand->unknown4 = 0;
            operand->object = object;
            operand->unknownC = 0;
            operand->isImmediate = FALSE;
        }
    }
finish:
    cprep_nostring = TRUE;
    cprep_in_assembler = TRUE;
    if (tk != ')' && !consumed) {
        CError_Error(0x10);
    }
    if (!consumed) {
        tk = lex();
    }
}

void InlineAsm_gcc_parse(void) {
    Statement* stmt;
    SInt32 output[3];
    SInt32 input[3];
    SInt32 start;
    SInt32 i;

    gcc_asm_operand_count = -1;
    i = 0;
    do {
        start = i;
        i += 6;
        gcc_asm_constraints[start].kind = 0;
        gcc_asm_constraints[start].registerClass = -1;
        gcc_asm_constraints[start].effects = 0;
        gcc_asm_constraints[start + 1].kind = 0;
        gcc_asm_constraints[start + 1].registerClass = -1;
        gcc_asm_constraints[start + 1].effects = 0;
        gcc_asm_constraints[start + 2].kind = 0;
        gcc_asm_constraints[start + 2].registerClass = -1;
        gcc_asm_constraints[start + 2].effects = 0;
        gcc_asm_constraints[start + 3].kind = 0;
        gcc_asm_constraints[start + 3].registerClass = -1;
        gcc_asm_constraints[start + 3].effects = 0;
        gcc_asm_constraints[start + 4].kind = 0;
        gcc_asm_constraints[start + 4].registerClass = -1;
        gcc_asm_constraints[start + 4].effects = 0;
        gcc_asm_constraints[start + 5].kind = 0;
        gcc_asm_constraints[start + 5].registerClass = -1;
        gcc_asm_constraints[start + 5].effects = 0;
    } while (i < 30);
    gcc_asm_vector_type = NULL;
    cprep_eoltokens = 0;
    if (tk == -7) {
        tk = lex();
    }

    if (tk == ':') {
        tk = lex();
        if (tk != ':' && tk != ')' && tk != '}') {
            for (;;) {
                gcc_parse_attribute(output, 1);
                if (output[1] == 0 && (SInt8)output[0] == -1) {
                    break;
                }
                gcc_allocate_operand(1, output[0], output[1], output[2]);
                if (tk != ',') {
                    break;
                }
                tk = lex();
            }
            copts.cplusplus = 0;
        }
    }

    if (tk == ':') {
        tk = lex();
        if (tk != ':' && tk != ')' && tk != '}') {
            for (;;) {
                gcc_parse_attribute(input, 0);
                if (input[1] == 0 && (SInt8)input[0] == -1) {
                    break;
                }
                gcc_allocate_operand(0, input[0], input[1], input[2]);
                if (tk != ',') {
                    break;
                }
                tk = lex();
            }
        }
    }

    if (tk == ':') {
        while ((tk = lex()), (stmt = first_ST_ASM), tk == '"') {
            tk = lex();
            while (tk != '"') {
                tk = lex();
            }
            if (lookahead() != ',') {
                tk = lex();
                stmt = first_ST_ASM;
                break;
            }
            tk = lex();
        }
    } else {
        stmt = first_ST_ASM;
    }
    for (; stmt; stmt = stmt->next) {
        if (stmt->type == ST_ASM) {
            gcc_replace_arg_st_asm(stmt);
        }
    }
}

GCCStaticEntry* gcc_statics_hash[512];

void InlineAsm_SkipComment(void) {
    for (;;) {
        if (tk != '/') {
            break;
        }
        if (lookahead() != '*') {
            break;
        }

        tk = lex();
        while (!((tk = lex()) == '*' && (tk = lex()) == '/')) {}
        tk = lex();
    }
}

GCCNameEntry* gcc_names_hash[256];

void gcc_clear_name_tables(void) {
    int i;

    for (i = 0; i < 0x100; i++) {
        gcc_names_hash[i] = 0;
    }
}

GCCNameInfo* gcc_find_name(char* name) {
    GCCNameEntry* entry;

    entry = gcc_names_hash[CHash(name) & 0xFF];
    while (entry) {
        if (strcmp(entry->info.name, name) == 0) {
            return &entry->info;
        }
        entry = entry->next;
    }
    return NULL;
}

GCCStaticInfo* gcc_find_value(char* name) {
    GCCStaticEntry* entry;

    entry = gcc_statics_hash[(UInt32)CHash(name) & 0x1FF];
    while (entry) {
        GCCStaticInfo* info = &entry->info;
        if (strcmp(info->name, name) == 0) {
            return info;
        }
        entry = entry->next;
    }
    return NULL;
}

void gcc_add_static(char* name, SInt16 value, SInt16 rclass, SInt32 reg) {
    GCCStaticEntry** bucket;
    GCCStaticEntry* entry;

    bucket = &gcc_statics_hash[(UInt32)CHash(name) & 0x1FF];
    entry = lalloc(0x14);
    entry->info.name = name;
    entry->info.value = value;
    entry->info.rclass = rclass;
    entry->info.reg = reg;
    entry->next = *bucket;
    *bucket = entry;
}

void gcc_clear_statics(void) {
    int i;

    for (i = 0; i < 0x200; i++) {
        gcc_statics_hash[i] = 0;
    }
}

GCCNameInfo* gcc_add_value(char* name, SInt32 value) {
    GCCNameEntry** bucket;
    GCCNameEntry* entry;
    SInt32* oldvalues;
    SInt32* newvalues;
    SInt32 count;
    SInt32 i;
    SInt32 hash = CHash(name);

    bucket = &gcc_names_hash[(UInt8)hash];
    for (entry = *bucket; entry; entry = entry->next) {
        if (strcmp(entry->info.name, name) == 0) {
            if (entry->info.count == entry->info.capacity) {
                count = entry->info.capacity;
                oldvalues = entry->info.operands;
                newvalues = lalloc(count * 8);
                entry->info.operands = newvalues;
                for (i = 0; i < count; i++) {
                    newvalues[i] = oldvalues[i];
                }
                entry->info.capacity <<= 1;
            }
            entry->info.operands[entry->info.count++] = value;
            return &entry->info;
        }
    }

    entry = lalloc(sizeof(GCCNameEntry));
    entry->info.name = lalloc(strlen(name) + 1);
    strcpy(entry->info.name, name);
    entry->info.capacity = 1;
    entry->info.operands = lalloc(4);
    entry->info.count = 1;
    entry->info.operands[0] = value;
    entry->next = *bucket;
    *bucket = entry;
    return &entry->info;
}
