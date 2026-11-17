#include "compiler/common.h"
#include "compiler/tokens.h"
#include "compiler/CError.h"
#include "compiler/CInt64.h"
#include "compiler/CFunc.h"
#include "compiler/IroUtil.h"
#include "compiler/CompilerTools.h"
#include "compiler/CMachine.h"
#include "compiler/CExpr.h"
#include "compiler/CInit.h"
#include "compiler/CInline.h"
#include "compiler/CMangler.h"
#include "compiler/COptimizer.h"
#include "compiler/CPrep.h"
#include "compiler/CPrepTokenizer.h"
#include "compiler/CScope.h"
#include "compiler/InlineAsm.h"
#include "compiler/InlineAsmMIPS.h"
#include "compiler/objects.h"
#include "compiler/UseDefChains.h"
#include "compiler/scopes.h"
#include "msl/string.h"
#include <string.h>
#include <stdio.h>

#include <setjmp.h>
#include "compiler/CParser.h"

void InlineAsm_SyntaxError(SInt16 code) {
    if (inline_asm_backtracking) {
        longjmp(inline_asm_backtrack_context, 1);
    }
    CError_Error(code);
}

void AssemblerError(void) {
    longjmp(inline_asm_jump_context, 1);
}

CLabel* InlineAsm_LookupLabel(HashNameNode* name) {
    CLabel* label;

    for (label = Labels; label; label = label->next) {
        if (name == label->name) {
            break;
        }
    }
    return label;
}

CLabel* InlineAsm_DeclareLabel(HashNameNode* name) {
    CLabel* label = newlabel();
    label->name = name;
    label->next = Labels;
    Labels = label;
    return label;
}

#pragma dont_inline on
CInt64 InlineAsm_ConstantExpression64bit(void) {
    CInt64 value;
    CInt64 result;
    CInt64* source;

    value = InlineAsm_PrimaryExpression();
    if (GetPrec(tk) != 0) {
        result = InlineAsm_ConstantExpressionTail(value);
        source = &result;
    } else {
        source = &value;
    }
    return *source;
}
#pragma dont_inline reset

#pragma dont_inline on
SInt32 InlineAsm_ConstantExpression32bit(void) {
    CInt64 value = InlineAsm_ConstantExpression64bit();
    return value.lo;
}
#pragma dont_inline reset

void InlineAsm_Assemble(void) {
    SInt16 endToken;
    UInt8 save_directives;
    UInt8 save_comment;

    endToken = (tk == '(') ? ')' : '}';
    save_comment = register_slash_comment_enabled;
    save_directives = copts.asmpoundcomment;
    inline_asm_saved_slash_comment = save_comment;
    cprep_nostring = TRUE;
    CFunc_AppendStatement(1);
    first_ST_ASM = curstmt;
    ASMstmtnb++;
    cprep_eoltokens = TRUE;
    cprep_in_assembler = TRUE;
    tk = lex();
    InlineAsm_ScanStatements(endToken);
    inline_asm_setjmp_valid = FALSE;
    cprep_in_assembler = FALSE;
    cprep_eoltokens = FALSE;
    cprep_nostring = FALSE;
    copts.asmpoundcomment = save_directives;
    register_slash_comment_enabled = inline_asm_saved_slash_comment;
}

HashNameNode* MakeLocalLabel(CInt64 num) {
    char buf[80];
    sprintf(buf, inline_asm_local_label_format, ASMstmtnb, num.lo);
    return GetHashNameNodeExport(buf);
}

void InlineAsm_UnpackAsmStatement(
    Statement* stmt, CLabel** labelArray, Boolean inlineCopy, void* data, SInt32 size
) {
    InlineAsmMIPSRecord* ia = galloc(size);
    InlineAsmMIPSOperand* op;
    SInt32 i;

    memcpy(ia, data, size);
    i = 0;
    op = ia->args;
    if (ia->argcount > 0) {
        do {
            switch (op->kind) {
                case 0:
                    break;
                case 1:
                case 3:
                    op->object = CInline_GetLocalObj(op->object, inlineCopy);
                    break;
                case 4:
                    op->label = labelArray[(SInt16)op->value];
                    break;
                default:
                    break;
            }
            i++;
            op++;
        } while (i < ia->argcount);
    }
    stmt->expr = (ENode*)ia;
}

void InlineAsm_PackAsmStatement(Statement* stmt, Statement* first, void** output, SInt32* outsize) {
    InlineAsmMIPSRecord* src = (InlineAsmMIPSRecord*)stmt->expr;
    SInt32 size = 8 + 20 * src->argcount;
    InlineAsmMIPSRecord* ia = galloc(size);
    InlineAsmMIPSOperand* op;
    SInt32 i;

    memcpy(ia, src, size);
    i = 0;
    op = ia->args;
    if (ia->argcount > 0) {
        do {
            switch (op->kind) {
                case 0:
                    break;
                case 1:
                case 3:
                    if (op->object && op->object->datatype == DLOCAL) {
                        op->object = CInline_GetLocalID(*(Object* volatile*)&op->object);
                    }
                    break;
                case 4:
                    op->value = (SInt16)(SInt32)fn_00509850(first, op->label->stmt);
                    break;
                default:
                    break;
            }
            i++;
            op++;
        } while (i < ia->argcount);
    }
    *output = ia;
    *outsize = size;
}

void InlineAsm_CheckLocalUsage(Statement* stmt) {
    InlineAsmMIPSRecord* ia;
    InlineAsmMIPSOperand* op;
    SInt32 i;

    ia = (InlineAsmMIPSRecord*)stmt->expr;
    i = 0;
    op = ia->args;
    if (ia->argcount > 0) {
        do {
            switch (op->kind) {
                case 1:
                    if (op->object) {
                        SetVarUsage(op->object, FALSE);
                    }
                    break;
                case 3:
                    SetVarUsage(op->object, TRUE);
                    break;
                default:
                    break;
            }
            i++;
            op++;
        } while (i < ia->argcount);
    }
}

void* InlineAsm_GetReferencedLabelIndexed(void* p, SInt32 index, SInt32* offset) {
    InlineAsmMIPSRecord* ia;
    InlineAsmMIPSOperand* op;
    SInt32 i;
    SInt32 counter;

    ia = p;
    for (i = 0, counter = 0, op = ia->args; i < ia->argcount; i++, op++) {
        if (op->kind == 2) {
            if (counter++ == index) {
                *offset = (char*)&op->object - (char*)ia;
                return op->object;
            }
        }
    }
    return NULL;
}

CLabel* InlineAsm_GetReferencedLabel2(Statement* stmt) {
    InlineAsmMIPSRecord* ia;
    InlineAsmMIPSOperand* op;
    SInt32 i;

    ia = (InlineAsmMIPSRecord*)stmt->expr;
    for (i = 0, op = ia->args; i < ia->argcount; i++, op++) {
        if (op->kind == 4) {
            return op->label;
        }
    }
    return NULL;
}

char* InlineAsm_DumpStatement(Statement* stmt) {
    InlineAsmMIPSRecord* ia;
    InlineAsmMIPSOperand* op;
    SInt32 i;
    ia = (InlineAsmMIPSRecord*)stmt->expr;
    *(SInt16*)InlineAsm_dumpBuffer = *(const SInt16*)InlineAsm_fmt_quote;
    strcat(InlineAsm_dumpBuffer, InlineAsm_GetMnemonic((const SInt32*)ia));
    strcat(InlineAsm_dumpBuffer, InlineAsm_fmt_quote);
    i = 0;
    op = (InlineAsmMIPSOperand*)ia->args;
    if ((SInt16)ia->argcount > 0) {
        do {
            char argbuf[1024];
            switch (op->kind) {
                case 0:
                    sprintf(argbuf, InlineAsm_fmt_imm, *(CInt64*)&op->value);
                    break;
                case 1:
                    if (op->object) {
                        sprintf(argbuf, InlineAsm_fmt_reg, op->object->name->name);
                    } else {
                        sprintf(argbuf, InlineAsm_fmt_regnum, (SInt32)op->value);
                    }
                    break;
                case 2:
                case 3:
                    if ((SInt32)op->value > 0) {
                        sprintf(
                            argbuf,
                            InlineAsm_fmt_objpositive,
                            op->object->name->name,
                            (SInt32)op->value
                        );
                    } else if ((SInt32)op->value < 0) {
                        sprintf(
                            argbuf,
                            InlineAsm_fmt_objnegative,
                            op->object->name->name,
                            -(SInt32)op->value
                        );
                    } else {
                        sprintf(argbuf, InlineAsm_fmt_obj, op->object->name->name);
                    }
                    break;
                case 4:
                    sprintf(argbuf, InlineAsm_fmt_lab, op->label->uniquename->name);
                    break;
                default:
                    break;
            }
            strcat(InlineAsm_dumpBuffer, argbuf);
            i++;
            op++;
        } while (i < (SInt16)ia->argcount);
    }
    return InlineAsm_dumpBuffer;
}

Boolean InlineAsm_LookupSymbolOrTag(HashNameNode* name, IALookupResult* result) {
    CLabel* label;
    NameSpace* nspace;
    NameSpaceObjectList* list;
    Object* object;
    CInt64 value;

    result->name = name;
    result->object = NULL;
    result->label = NULL;
    result->type = NULL;
    result->has_value = FALSE;
    for (label = Labels; label; label = label->next) {
        if (name == label->name) {
            break;
        }
    }
    result->label = label;
    if (result->label == NULL) {
        for (nspace = cscope_current; nspace; nspace = nspace->parent) {
            list = CScope_FindName(nspace, name);
            if (list) {
                object = list->object;
                switch (object->otype) {
                    case OT_ENUMCONST:
                        result->has_value = TRUE;
                        result->value = ((ObjEnumConst*)list->object)->val;
                        return TRUE;
                    case OT_OBJECT:
                        if (object->datatype == DABSOLUTE) {
                            result->has_value = TRUE;
                            CInt64_SetLong(&value, object->u.tail_to44[0]);
                            result->value = value;
                        } else {
                            if (object->datatype == DDATA && (object->qual & 0x10000)) {
                                CInit_ExportConst(object);
                            }
                            if (object->datatype == DALIAS) {
                                result->object = object->u.alias.object;
                            } else {
                                result->object = object;
                            }
                        }
                        return TRUE;
                    case OT_TYPE:
                        result->type = ((ObjType*)object)->type;
                        return TRUE;
                    case OT_TYPETAG:
                    case OT_NAMESPACE:
                    case OT_MEMBERVAR:
                        return FALSE;
                    default:
                        CError_Internal((char*)InlineAsm_file_name, 237);
                        break;
                }
            }
        }
        return FALSE;
    }
    return TRUE;
}

SInt32 InlineAsm_StructMemberOffset(Type* type) {
    StructMember* member;
    ObjMemberVar* ivar;
    ClassList* bases;
    NameSpaceObjectList* list;
    HashNameNode* name;
    SInt32 offset = 0;

    do {
        if (type->type == TYPESTRUCT) {
            tk = lex();
            if (tk != -3) {
                if (inline_asm_backtracking) {
                    longjmp(inline_asm_backtrack_context, 1);
                }
                CError_Error(8);
            }
            member = ismember(TYPE_STRUCT(type), tkidentifier);
            if (!member) {
                CError_Error(0x33, tkidentifier->name);
            }
            offset += member->offset;
            type = member->type;
            tk = lex();
        } else if (type->type == TYPECLASS) {
            tk = lex();
            if (tk != -3) {
                if (inline_asm_backtracking) {
                    longjmp(inline_asm_backtrack_context, 1);
                }
                CError_Error(8);
            }
            name = tkidentifier;
            list = CScope_FindName(TYPE_CLASS(type)->nspace, name);
            if (list && list->object->otype == OT_MEMBERVAR) {
                ivar = (ObjMemberVar*)list->object;
            } else {
                ivar = NULL;
                for (bases = TYPE_CLASS(type)->bases; bases; bases = bases->next) {
                    list = CScope_FindName(bases->base->nspace, name);
                    if (list) {
                        ivar = (ObjMemberVar*)list->object;
                        if (ivar->otype == OT_MEMBERVAR) {
                            break;
                        }
                    }
                    ivar = NULL;
                }
            }
            if (!ivar) {
                CError_Error(0x33, name->name);
            }
            offset += ivar->offset;
            type = ivar->type;
            tk = lex();
        } else {
            CError_Error(0x32);
        }
    } while (tk == '.');
    return offset;
}

SInt32 InlineAsm_StructArrayMemberOffset(Type* type) {
    StructMember* member;
    ObjMemberVar* ivar;
    ClassList* bases;
    NameSpaceObjectList* list;
    HashNameNode* name;
    SInt32 offset = 0;

    do {
        if (tk == '.') {
            if (type->type == TYPESTRUCT) {
                tk = lex();
                if (tk != -3) {
                    if (inline_asm_backtracking) {
                        longjmp(inline_asm_backtrack_context, 1);
                    }
                    CError_Error(8);
                }
                member = ismember(TYPE_STRUCT(type), tkidentifier);
                if (!member) {
                    CError_Error(0x33, tkidentifier->name);
                }
                offset += member->offset;
                type = member->type;
                tk = lex();
            } else if (type->type == TYPECLASS) {
                tk = lex();
                if (tk != -3) {
                    if (inline_asm_backtracking) {
                        longjmp(inline_asm_backtrack_context, 1);
                    }
                    CError_Error(8);
                }
                name = tkidentifier;
                list = CScope_FindName(TYPE_CLASS(type)->nspace, name);
                if (list && list->object->otype == OT_MEMBERVAR) {
                    ivar = (ObjMemberVar*)list->object;
                } else {
                    ivar = NULL;
                    bases = TYPE_CLASS(type)->bases;
                    while (bases && !ivar) {
                        list = CScope_FindName(bases->base->nspace, name);
                        if (list && list->object->otype == OT_MEMBERVAR) {
                            ivar = (ObjMemberVar*)list->object;
                        } else {
                            bases = bases->next;
                        }
                    }
                }
                if (!ivar) {
                    CError_Error(0x33, name->name);
                }
                offset += ivar->offset;
                type = ivar->type;
                tk = lex();
            } else {
                CError_Error(0x32);
            }
        } else {
            if (type->type == TYPEARRAY) {
                type = TYPE_ARRAY(type)->target;
                tk = lex();
                offset += type->size * InlineAsm_ConstantExpression32bit();
                if (tk != ']') {
                    InlineAsm_SyntaxError(0x1A);
                }
                tk = lex();
            } else {
                CError_Error(0x31);
            }
        }
    } while (tk == '.' || tk == '[');
    return offset;
}

#define InlineAsm_DefineLabel(input_name) \
    do { \
        HashNameNode* label_name = (input_name); \
        CLabel* label; \
        Statement* statement; \
        for (label = Labels; label; label = label->next) { \
            if (label_name == label->name) { \
                break; \
            } \
        } \
        if (!label) { \
            CLabel* created = newlabel(); \
            created->name = label_name; \
            created->next = Labels; \
            Labels = created; \
            label = created; \
        } else if (label->stmt) { \
            CError_Error(0x48, label_name->name); \
        } \
        statement = CFunc_AppendStatement(2); \
        statement->label = label; \
        label->stmt = statement; \
    } while (0)

void InlineAsm_ScanOptionalLabel(void) {
    if (tk == -1) {
        if (lookahead() != ':') {
            return;
        }
        InlineAsm_DefineLabel(MakeLocalLabel(tkintconst));
        tk = lex();
        tk = lex();
        return;
    } else if (tkidentifier->name[0] == '@') {
        HashNameNode* name = tkidentifier;
        InlineAsm_DefineLabel(name);
        tk = lex();
        if (tk == ':') {
            tk = lex();
        }
    } else {
        HashNameNode* name = tkidentifier;
        SInt16 t = lookahead();
        tkidentifier = name;
        if (t != ':') {
            return;
        }
        InlineAsm_DefineLabel(tkidentifier);
        tk = lex();
        tk = lex();
    }
}

void InlineAsm_ScanStatements(volatile SInt16 endToken) {
    InlineAsm_Initialize();
    inline_asm_setjmp_valid = FALSE;
    if (setjmp(inline_asm_jump_context)) {
        while (tk != ';' && tk != -7 && tk != endToken && tk != '}' && tk) {
            tk = lex();
        }
        if (tk == ';' || tk == -7) {
            tk = lex();
        }
    }
    while (tk && tk != endToken) {
        inline_asm_backtracking = FALSE;
        sourceoffset = CPrep_GetFileOffsetInfo(&cparser_fileoffset);
        if (tk == '"') {
            if (inline_asm_setjmp_valid) {
                tk = lex();
                while (tk == -7) {
                    CPrep_TokenStreamFlush();
                    tk = lex();
                }
                if (tk == '"') {
                    tk = lex();
                    while (tk == -7) {
                        CPrep_TokenStreamFlush();
                        tk = lex();
                    }
                }
                if (tk == ':') {
                    InlineAsm_gcc_parse();
                    inline_asm_setjmp_valid = FALSE;
                }
                if (tk == ';' || tk == -7) {
                    CPrep_TokenStreamFlush();
                    tk = lex();
                }
                if (tk == endToken) {
                    break;
                }
            } else {
                inline_asm_setjmp_valid = TRUE;
                register_slash_comment_enabled = FALSE;
                tk = lex();
            }
        }
        if (tk == '.') {
            InlineAsm_ScanAssemblyDirective();
        } else {
            if (tk == -3) {
                InlineAsm_ScanOptionalLabel();
            } else if (tk == -1) {
                InlineAsm_ScanOptionalLabel();
            }
            if (tk != -7 && tk != '"') {
                InlineAsm_ScanAssemblyInstruction();
            }
        }
        if (tk == ';' || tk == -7) {
            CPrep_TokenStreamFlush();
            tk = lex();
        } else if (tk != -4 && tk != '"' && tk != -3 && tk != endToken) {
            if (endToken == ')') {
                CError_Error(0x10);
            } else {
                CError_Error(0x0E);
            }
        }
    }
}

CInt64 InlineAsm_PrimaryExpression(void) {
    IALookupResult lookup;
    CInt64 value;
    CInt64 operand_neg;
    CInt64 operand_not;
    CInt64 operand_bitnot;

    CInt64_SetULong(&value, 0);
    switch (tk) {
        case -3:
            if (InlineAsm_LookupSymbolOrTag(tkidentifier, &lookup)) {
                if (lookup.has_value) {
                    tk = lex();
                    return lookup.value;
                }
                if (lookup.type &&
                    (lookup.type->type == TYPESTRUCT || lookup.type->type == TYPECLASS)) {
                    tk = lex();
                    if (tk != '.') {
                        InlineAsm_SyntaxError(0x15);
                    }
                    if (inline_asm_allow_array_expressions) {
                        CInt64_SetLong(&value, InlineAsm_StructArrayMemberOffset(lookup.type));
                    } else {
                        CInt64_SetLong(&value, InlineAsm_StructMemberOffset(lookup.type));
                    }
                    return value;
                }
                InlineAsm_SyntaxError(0x19);
            } else {
                InlineAsm_SyntaxError(0x19);
            }
            break;
        case -1:
            value = tkintconst;
            tk = lex();
            return value;
        case 325:
            CInt64_SetLong(&value, scansizeof());
            return value;
        case '+':
            tk = lex();
            return InlineAsm_PrimaryExpression();
        case '-':
            tk = lex();
            operand_neg = InlineAsm_PrimaryExpression();
            return CInt64_Neg(operand_neg);
        case '!':
            tk = lex();
            operand_not = InlineAsm_PrimaryExpression();
            return CInt64_Not(operand_not);
        case '~':
            tk = lex();
            operand_bitnot = InlineAsm_PrimaryExpression();
            return CInt64_Inv(operand_bitnot);
        case '(':
            tk = lex();
            value = InlineAsm_ConstantExpression64bit();
            if (tk != ')') {
                CError_Error(0x10);
            }
            tk = lex();
            return value;
        default:
            InlineAsm_SyntaxError(0x15);
    }
    return value;
}

CInt64 InlineAsm_ConstantExpressionTail(CInt64 value) {
    CInt64 right;
    SInt16 left_token;
    SInt16 right_prec;

    while (TRUE) {
        left_token = tk;
        tk = lex();
        right = InlineAsm_PrimaryExpression();
        right_prec = GetPrec(tk);
        if (right_prec == 0) {
            return CMach_CalcIntDiadic((Type*)&stsignedlonglong, value, left_token, right);
        }
        if (GetPrec(left_token) >= right_prec) {
            value = CMach_CalcIntDiadic((Type*)&stsignedlonglong, value, left_token, right);
        } else {
            value = CMach_CalcIntDiadic(
                (Type*)&stsignedlonglong, value, left_token, InlineAsm_ConstantExpressionTail(right)
            );
            if (GetPrec(tk) == 0) {
                return value;
            }
        }
    }
}
