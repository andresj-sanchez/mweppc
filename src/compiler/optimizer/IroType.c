#include "compiler/common.h"
#include "compiler/CompilerTools.h"
#include <stdio.h>
#include <string.h>
#include "compiler/CDecl.h"
#include "compiler/CError.h"
#include "compiler/CMachine.h"
#include "compiler/types.h"
#include "compiler/CParser.h"
#include "compiler/CABI.h"
#include "compiler/IroLoop.h"
#include "compiler/IroLinearForm.h"
#include "compiler/IroType.h"
#include "compiler/IroUtil.h"
#include "compiler/objects.h"

Boolean IRO_TypesEqual(Type* a, Type* b) {
    if (a == b) {
        return TRUE;
    }

    if (IS_TYPE_BITFIELD(a)) {
        if (IS_TYPE_BITFIELD(b)) {
            return TYPE_BITFIELD(a)->bitfieldtype == TYPE_BITFIELD(b)->bitfieldtype &&
                TYPE_BITFIELD(a)->offset == TYPE_BITFIELD(b)->offset &&
                TYPE_BITFIELD(a)->bitlength == TYPE_BITFIELD(b)->bitlength;
        }
        return FALSE;
    }

    if (a->type == TYPEPOINTER && b->type == TYPEPOINTER) {
        return TRUE;
    }

    return is_typeequal(a, b) != 0;
}

Type* IRO_CopyType(Type* type) {
    TypePointer* copy;
    TypeMemberPointer* mcopy;

    if (!type) {
        return NULL;
    }
    switch ((SInt8)type->type) {
        case TYPEVOID:
        case TYPEINT:
        case TYPEFLOAT:
        case TYPEENUM:
        case TYPESTRUCT:
        case TYPEBITFIELD:
            return type;
        case TYPECLASS:
        case TYPETEMPLATE:
            return type;
        case TYPEPOINTER:
        case TYPEARRAY:
            copy = (TypePointer*)galloc(sizeof(TypePointer));
            *copy = *(TypePointer*)type;
            copy->target = IRO_CopyType(((TypePointer*)type)->target);
            copy->qual &= 0x200003;
            return (Type*)copy;
        case TYPEMEMBERPOINTER:
            mcopy = (TypeMemberPointer*)galloc(sizeof(TypeMemberPointer));
            *mcopy = *(TypeMemberPointer*)type;
            mcopy->ty1 = IRO_CopyType(((TypeMemberPointer*)type)->ty1);
            mcopy->qual &= 0x200003;
            return (Type*)mcopy;
        case TYPEFUNC:
            return type;
        case TYPETEMPLDEP:
        case 13:
        case 14:
        case -1:
            return type;
        default:
            CError_Internal("IroType.c", 143);
            return NULL;
    }
}

Type* IRO_UnsignedType(Type* type) {
    TypeIntegral* t = TYPE_INTEGRAL(type);

    if (!IS_TYPE_ENUM(t) &&
        !IS_TYPE_POINTER_ONLY(t) &&
        (!IS_TYPE_INT(t) || TYPE_INTEGRAL(t)->integral >= 0x11)) {
fatal:
        CError_FATAL(245);
        return NULL;
    }

    if (t == (TypeIntegral*)&stbool ||
        t == (TypeIntegral*)&stwchar ||
        t == (TypeIntegral*)&stchar ||
        t == (TypeIntegral*)&stsignedchar ||
        t == (TypeIntegral*)&stunsignedchar) {
        goto bysize;
    }
    if (t == (TypeIntegral*)&stsignedshort || t == (TypeIntegral*)&stunsignedshort) {
        return TYPE(&stunsignedshort);
    }
    if (t == (TypeIntegral*)&stsignedint || t == (TypeIntegral*)&stunsignedint) {
        return TYPE(&stunsignedint);
    }
    if (t == (TypeIntegral*)&stsignedlong || t == (TypeIntegral*)&stunsignedlong) {
        return TYPE(&stunsignedlong);
    }
    if (t == (TypeIntegral*)&stunsignedlonglong) {
        return TYPE(&stunsignedlonglong);
    }

bysize:
    if (t->size == stunsignedchar.size) {
        return TYPE(&stunsignedchar);
    }
    if (t->size == stunsignedshort.size) {
        return TYPE(&stunsignedshort);
    }
    if (t->size == stunsignedint.size) {
        return TYPE(&stunsignedint);
    }
    if (t->size == stunsignedlong.size) {
        return TYPE(&stunsignedlong);
    }
    if (t->size == stunsignedlonglong.size) {
        return TYPE(&stunsignedlonglong);
    }
    goto fatal;
}

Boolean IRO_is_CPtypeequal(Type* a, Type* b) {
    Boolean ok;

    if (IS_TYPE_POINTER_ONLY(a) && IS_TYPE_POINTER_ONLY(b)) {
        return TRUE;
    }

    if (copts.unknown122) {
        ok = (IS_TYPE_INT(a) && TYPE_INTEGRAL(a)->integral < 0x11) ||
            IS_TYPE_ENUM(a) ||
            IS_TYPE_POINTER_ONLY(a) ||
            (a->type == TYPEMEMBERPOINTER && a->size == 4);
        if (ok) {
            ok = (IS_TYPE_INT(b) && TYPE_INTEGRAL(b)->integral < 0x11) ||
                IS_TYPE_ENUM(b) ||
                IS_TYPE_POINTER_ONLY(b) ||
                (b->type == TYPEMEMBERPOINTER && b->size == 4);
        }
        if (ok) {
            if (a->size == b->size && is_unsigned(a) == is_unsigned(b)) {
                return TRUE;
            }
            return FALSE;
        }
    }

    return is_typeequal(a, b) != 0;
}

Type* IRO_SignedType(Type* type) {
    TypeIntegral* t = TYPE_INTEGRAL(type);

    if (!IS_TYPE_ENUM(t) &&
        !IS_TYPE_POINTER_ONLY(t) &&
        (!IS_TYPE_INT(t) || TYPE_INTEGRAL(t)->integral >= 0x11)) {
fatal:
        CError_FATAL(305);
        return NULL;
    }

    if (t == (TypeIntegral*)&stbool ||
        t == (TypeIntegral*)&stwchar ||
        t == (TypeIntegral*)&stchar ||
        t == (TypeIntegral*)&stsignedchar ||
        t == (TypeIntegral*)&stunsignedchar) {
        goto bysize;
    }
    if (t == (TypeIntegral*)&stsignedshort || t == (TypeIntegral*)&stunsignedshort) {
        goto shortsize;
    }
    if (t == (TypeIntegral*)&stsignedint || t == (TypeIntegral*)&stunsignedint) {
        goto intsize;
    }
    if (t == (TypeIntegral*)&stsignedlong || t == (TypeIntegral*)&stunsignedlong) {
        goto longsize;
    }
    if (t == (TypeIntegral*)&stsignedlonglong || t == (TypeIntegral*)&stunsignedlonglong) {
        goto longlongsize;
    }

bysize:
    if (t->size == stsignedchar.size) {
        return TYPE(&stsignedchar);
    }
shortsize:
    if (t->size == stsignedshort.size) {
        return TYPE(&stsignedshort);
    }
intsize:
    if (t->size == stsignedint.size) {
        return TYPE(&stsignedint);
    }
longsize:
    if (t->size == stsignedlong.size) {
        return TYPE(&stsignedlong);
    }
longlongsize:
    if (t->size == stsignedlonglong.size) {
        return TYPE(&stsignedlonglong);
    }
    goto fatal;
}

Type* IRO_LogicalType(void) {
    if (copts.cplusplus && copts.booltruefalse) {
        return TYPE(&stbool);
    }
    return TYPE(&stsignedint);
}

void IRO_SpellType(Type* type, char* buf) {
    char mybuf[256];
    char mybuf2[256];

    switch ((SInt8)type->type) {
        case TYPEVOID:
            strcpy(buf, "void");
            break;
        case TYPEINT:
            switch (TYPE_INTEGRAL(type)->integral) {
                case IT_BOOL:
                    strcpy(buf, "bool");
                    break;
                case IT_CHAR:
                    strcpy(buf, "char");
                    break;
                case IT_SCHAR:
                    strcpy(buf, "signed char");
                    break;
                case IT_UCHAR:
                    strcpy(buf, "unsigned char");
                    break;
                case IT_WCHAR_T:
                    strcpy(buf, "wchar_t");
                    break;
                case IT_SHORT:
                    strcpy(buf, "short");
                    break;
                case IT_USHORT:
                    strcpy(buf, "unsigned short");
                    break;
                case IT_INT:
                    strcpy(buf, "int");
                    break;
                case IT_UINT:
                    strcpy(buf, "unsigned int");
                    break;
                case IT_LONG:
                    strcpy(buf, "long");
                    break;
                case IT_ULONG:
                    strcpy(buf, "unsigned long");
                    break;
                case IT_LONGLONG:
                    strcpy(buf, "long long");
                    break;
                case IT_ULONGLONG:
                    strcpy(buf, "unsigned long long");
                    break;
            }
            break;
        case TYPEFLOAT:
            switch (TYPE_INTEGRAL(type)->integral) {
                case IT_FLOAT:
                    strcpy(buf, "float");
                    break;
                case IT_SHORTDOUBLE:
                    strcpy(buf, "short double");
                    break;
                case IT_DOUBLE:
                    strcpy(buf, "double");
                    break;
                case IT_LONGDOUBLE:
                    strcpy(buf, "long double");
                    break;
            }
            break;
        case TYPEENUM:
            strcpy(buf, "enum ");
            if (TYPE_ENUM(type)->enumname) {
                strcat(buf, TYPE_ENUM(type)->enumname->name);
            }
            break;
        case TYPESTRUCT:
            strcpy(buf, "struct ");
            if (TYPE_STRUCT(type)->name) {
                strcat(buf, TYPE_STRUCT(type)->name->name);
            }
            break;
        case TYPECLASS:
            strcpy(buf, "class ");
            if (TYPE_CLASS(type)->classname) {
                strcat(buf, TYPE_CLASS(type)->classname->name);
            }
            break;
        case TYPEFUNC:
            IRO_SpellType(TYPE_FUNC(type)->functype, mybuf);
            strcpy(buf, "freturns(");
            strcat(buf, mybuf);
            strcat(buf, ")");
            break;
        case TYPEBITFIELD:
            IRO_SpellType(TYPE_BITFIELD(type)->bitfieldtype, mybuf);
            sprintf(
                buf,
                "bitfield(%s){%d:%d}",
                mybuf,
                TYPE_BITFIELD(type)->offset,
                TYPE_BITFIELD(type)->bitlength
            );
            break;
        case TYPETEMPLDEP:
            strcpy(buf, "label");
            break;
        case TYPEMEMBERPOINTER:
            IRO_SpellType(TYPE_MEMBER_POINTER(type)->ty2, mybuf);
            IRO_SpellType(TYPE_MEMBER_POINTER(type)->ty1, mybuf2);
            strcpy(buf, "memberpointer(");
            strcat(buf, mybuf);
            strcat(buf, ",");
            strcat(buf, mybuf2);
            strcat(buf, ")");
            break;
        case TYPEPOINTER:
            IRO_SpellType(TYPE_POINTER(type)->target, mybuf);
            strcpy(buf, "pointer(");
            strcat(buf, mybuf);
            strcat(buf, ")");
            break;
        case TYPEARRAY:
            IRO_SpellType(TYPE_ARRAY(type)->target, mybuf);
            strcpy(buf, "array(");
            strcat(buf, mybuf);
            strcat(buf, ")");
            break;
    }
}

void IRO_TruncateValueToType(CInt64* val, Type* type) {
    if (is_unsigned(type)) {
        switch (type->size) {
            case 1:
                CInt64_ConvertUInt8(val);
                break;
            case 2:
                CInt64_ConvertUInt16(val);
                break;
            case 4:
                CInt64_ConvertUInt32(val);
                break;
            case 8:
                CInt64_ConvertUInt64(val);
                break;
        }
    } else {
        switch (type->size) {
            case 1:
                CInt64_ConvertInt8(val);
                break;
            case 2:
                CInt64_ConvertInt16(val);
                break;
            case 4:
                CInt64_ConvertInt32(val);
                break;
            case 8:
                CInt64_ConvertInt64(val);
                break;
        }
    }
}

static void IRO_MarkWalker(IROLinear* nd); //! @reconstructed
static Boolean IRO_CPTypeChecker(Type* type, IROLinear* nd, int flag); //! @reconstructed

void IRO_MarkUnpromotableTypesLinear(void) {
    IROLinear* nd;
    IROLinear* next;
    Type* saved;
    SInt32 sizeclass;

    for (nd = IRO_FirstLinear; nd; nd = nd->next) {
        nd->unk18 = nd->rtype;
    }

    for (nd = IRO_FirstLinear; nd; nd = next) {
        next = nd->next;
        while (next && (next->flags & 2)) {
            next = next->next;
        }

        switch (nd->type) {
            case IROLinearOperand:
            case IROLinearOp1Arg:
            case IROLinearOp2Arg:
            case IROLinearOp3Arg:
            case IROLinear5:
            case IROLinearFunccall:
                IRO_MarkWalker(nd);
                break;
            case IROLinearGotoExpr:
            case IROLinearReturn:
                if (nd->type == IROLinearGotoExpr) {
                    IRO_MarkWalker(nd->u.monadic);
                }
                if (nd->u.monadic && FunctionName) {
                    if (!CMach_GetFunctionResultClass(TYPE_FUNC(FunctionName->type))) {
                        if (!IRO_IsIntConstant(nd->u.monadic)) {
                            saved = nd->u.monadic->unk18;
                            if (saved->type == TYPEENUM || saved->type == TYPEINT) {
                                if (saved->type != TYPEINT ||
                                    TYPE_INTEGRAL(saved)->integral < 0x11) {
                                    if (saved->size == 1) {
                                        sizeclass = 0;
                                    } else if (saved->size == 2) {
                                        sizeclass = 1;
                                    } else if (saved->size == 4) {
                                        sizeclass = 2;
                                    } else if (saved->size == 8) {
                                        sizeclass = 3;
                                    } else {
                                        sizeclass = -1;
                                    }
                                    if (sizeclass >= 0 && sizeclass <= 2) {
                                        IRO_CPTypeChecker(saved, nd->u.monadic, 1);
                                    }
                                }
                            }
                        }
                    }
                }
                IRO_MarkWalker(nd->u.monadic);
                break;
            case IROLinearIf:
            case IROLinearIfNot:
            case IROLinearSwitch:
                IRO_MarkWalker(nd->u.diadic.right);
                break;
            case IROLinearBeginCatch:
            case IROLinearEndCatch:
            case IROLinearEndCatchDtor:
                IRO_MarkWalker(nd->u.monadic);
                break;
            case 20:
            case 21:
                CError_FATAL(1310);
                break;
        }
    }
}

static Boolean IRO_CPTypeChecker(Type* type, IROLinear* nd, int flag) {
    IROLinear* node = nd;
    Boolean result;
    SInt8 tclass;
    SInt8 nclass;
    Type* nsaved;
    Type* lsaved;
    Type* rsaved;
    IROLinear* left;
    IROLinear* right;
    Type* swaptype;

    result = FALSE;
    if (type->size == 1) {
        tclass = 0;
    } else if (type->size == 2) {
        tclass = 1;
    } else if (type->size == 4) {
        tclass = 2;
    } else if (type->size == 8) {
        tclass = 3;
    } else {
        tclass = -1;
    }

    nsaved = node->unk18;
    if (nsaved->size == 1) {
        nclass = 0;
    } else if (nsaved->size == 2) {
        nclass = 1;
    } else if (nsaved->size == 4) {
        nclass = 2;
    } else if (nsaved->size == 8) {
        nclass = 3;
    } else {
        nclass = -1;
    }

    switch (nd->type) {
        case IROLinearOp2Arg:
            switch (nd->nodetype) {
                case 9:
                case 15:
                case 16:
                case 17:
                case 25:
                case 26:
                case 27:
                    break;
                default:
                    goto done;
            }
            left = node->u.diadic.left;
            right = node->u.diadic.right;
            lsaved = left->unk18;
            rsaved = right->unk18;
            if (!(lsaved->type == TYPEENUM ||
                  (lsaved->type == TYPEINT && TYPE_INTEGRAL(lsaved)->integral < 0x11))) {
                goto kindmismatch;
            }
            if (!(rsaved->type == TYPEENUM ||
                  (rsaved->type == TYPEINT && TYPE_INTEGRAL(rsaved)->integral < 0x11))) {
                goto kindmismatch;
            }
            if (nclass < tclass) {
                nsaved = rsaved;
                tclass = nclass;
            }
            if (nsaved->type != lsaved->type) {
                goto kindmismatch3;
            }
            if (nsaved->type != rsaved->type) {
                goto kindmismatch3;
            }
            if (!IRO_CPTypeChecker(nsaved, left, 0)) {
                goto enumtail;
            }
            if (!IRO_CPTypeChecker(nsaved, right, 0)) {
                goto enumtail;
            }
            if (!(copts.unknown124 || nd->nodetype != 9 || tclass != 0)) {
                goto enumtail;
            }
            if (!flag) {
                goto enumtail;
            }
            result = TRUE;
            if (!IRO_CPTypeChecker(nsaved, left, flag)) {
                goto enumtail;
            }
            if (!IRO_CPTypeChecker(nsaved, right, flag)) {
                goto enumtail;
            }
            if (!!is_unsigned(node->unk18) != !!is_unsigned(nsaved)) {
                if (!is_unsigned(node->unk18)) {
                    nsaved = IRO_SignedType(nsaved);
                } else {
                    nsaved = IRO_UnsignedType(nsaved);
                }
                node->unk18 = nsaved;
                nclass = tclass;
            }
enumtail:
            if (!copts.unknown125) {
                goto done;
            }
            if (nd->type != TYPEENUM || nd->nodetype != 9) {
                goto done;
            }
            goto done;
        case IROLinearOp1Arg:
            switch (nd->nodetype) {
                case 4:
                    if (nclass != tclass) {
                        if (nclass <= tclass) {
                            goto done;
                        }
                    }
                    if (nd->nodetype == 3 && nd->type == 2) {
                        node = node->u.monadic;
                    }
                    if (nd->type == 2 && nd->nodetype == 0x33) {
                        goto done;
                    }
                    goto done;
                case 5:
                case 6:
                    left = node->u.diadic.left;
                    lsaved = left->unk18;
                    if (!(lsaved->type == TYPEENUM ||
                          (lsaved->type == TYPEINT && TYPE_INTEGRAL(lsaved)->integral < 0x11))) {
                        goto done;
                    }
                    if (nclass < tclass) {
                        nsaved = left->unk18;
                    }
                    if (nsaved->type != lsaved->type) {
                        goto done;
                    }
                    result = IRO_CPTypeChecker(nsaved, left, flag);
                    if (!result) {
                        goto done;
                    }
                    if (!flag) {
                        goto done;
                    }
                    lsaved = left->unk18;
                    if (!!is_unsigned(node->unk18) != !!is_unsigned(lsaved)) {
                        if (!is_unsigned(node->unk18)) {
                            node->unk18 = IRO_SignedType(lsaved);
                        } else {
                            node->unk18 = IRO_UnsignedType(lsaved);
                        }
                    }
                    goto done;
                case 0x33:
                    left = node->u.diadic.left;
                    lsaved = left->unk18;
                    if (lsaved->type != nsaved->type) {
                        goto mismatchsigned;
                    }
                    if (!(lsaved->type == TYPEENUM ||
                          (lsaved->type == TYPEINT && TYPE_INTEGRAL(lsaved)->integral < 0x11))) {
                        goto classle;
                    }
                    if (!flag) {
                        goto done;
                    }
                    result = TRUE;
                    if (nclass < tclass) {
                        goto done;
                    }
                    if (!IRO_CPTypeChecker(nsaved, node->u.monadic, flag)) {
                        goto done;
                    }
                    if (!!is_unsigned(node->unk18) != !!is_unsigned(node->u.monadic->unk18)) {
                        if (!is_unsigned(node->unk18)) {
                            node->unk18 = IRO_SignedType(node->u.monadic->unk18);
                        } else {
                            node->unk18 = IRO_UnsignedType(node->u.monadic->unk18);
                        }
                    }
                    goto done;
                default:
                    goto done;
            }
        case IROLinearOperand:
            switch (nd->u.node->type) {
                case EINTCONST:
                    if (!IRO_IsIntConstant(nd)) {
                        goto done;
                    }
                    if (!flag) {
                        goto done;
                    }
                    result = TRUE;
                    if (!!is_unsigned(nsaved) != !!is_unsigned(node->unk18)) {
                        if (!is_unsigned(nsaved)) {
                            node->unk18 = IRO_SignedType(node->unk18);
                        } else {
                            node->unk18 = IRO_UnsignedType(node->unk18);
                        }
                    }
                    goto done;
                case EOBJREF:
                    if (!flag) {
                        result = TRUE;
                        goto done;
                    }
                    if (!!is_unsigned(nsaved) != !!is_unsigned(node->unk18)) {
                        if (!is_unsigned(nsaved)) {
                            node->unk18 = IRO_SignedType(node->unk18);
                        } else {
                            node->unk18 = IRO_UnsignedType(node->unk18);
                        }
                    }
                    goto done;
                default:
                    goto done;
            }
        default:
            goto done;
    }
kindmismatch:
    goto enumtail;
kindmismatch3:
    if (!copts.unknown123) {
        goto enumtail;
    }
    goto enumtail;
mismatchsigned:
    if (!copts.unknown123) {
        goto done;
    }
    goto done;
classle:
    result = (nclass <= tclass);
    goto done;
done:
    return result;
}

static void IRO_MarkWalker(IROLinear* nd) {
    Type* saved;
    SInt8 sizeclass;
    IROLinear* left;
    IROLinear* right;
    IROLinear* inner;
    IROLinear* sub;
    Type* leftsaved;
    Type* rightsaved;
    SInt32 i;

    saved = nd->unk18;
    if (saved->size == 1) {
        sizeclass = 0;
    } else if (saved->size == 2) {
        sizeclass = 1;
    } else if (saved->size == 4) {
        sizeclass = 2;
    } else if (saved->size == 8) {
        sizeclass = 3;
    } else {
        sizeclass = -1;
    }

    switch (nd->type) {
        case IROLinearOperand:
            return;
        case IROLinearOp1Arg:
            if (!(IRO_IsAssignOp[nd->nodetype])) {
                IRO_MarkWalker(nd->u.monadic);
                return;
            }
            sub = nd->u.monadic;
            if (sub->type == 2 && sub->nodetype == 4) {
                sub = sub->u.monadic;
            }
            if (sub->type != 2 || sub->nodetype != 0x33) {
                IRO_MarkWalker(nd->u.monadic);
                return;
            }
            if (nd->type == 3) {
                IRO_MarkWalker(nd->u.diadic.right);
            }
            IRO_MarkWalker(sub->u.monadic->u.monadic);
            return;
        case IROLinearOp2Arg:
            right = nd->u.diadic.right;
            left = nd->u.diadic.left;
            if (IRO_IsAssignOp[nd->nodetype]) {
                inner = left;
                if (inner->type == 2 && inner->nodetype == 4) {
                    inner = inner->u.monadic;
                }
                if (inner->type == 2 && inner->nodetype == 0x33) {
                    if (nd->type == 3) {
                        IRO_MarkWalker(right);
                    }
                    IRO_MarkWalker(inner->u.monadic);
                    return;
                }
            }
            if (!(nd->nodetype + 0xE2 <= 0xA + 0xE2 - 0xE2 + 10 &&
                  ((1 << (nd->nodetype + 0xE2)) & 0x773))) {
                IRO_MarkWalker(left);
                IRO_MarkWalker(right);
                return;
            }
            if (!(saved->type == TYPEENUM ||
                  (saved->type == TYPEINT && TYPE_INTEGRAL(saved)->integral < 0x11))) {
                IRO_MarkWalker(left);
                IRO_MarkWalker(right);
                return;
            }
            if (sizeclass == -1) {
                if (saved->size == 1) {
                    sizeclass = 0;
                    if (!is_unsigned(saved) == !is_unsigned(TYPE(&stsignedchar))) {
                        nd->unk18 = TYPE(&stsignedchar);
                    } else if (!is_unsigned(saved)) {
                        nd->unk18 = IRO_SignedType(TYPE(&stsignedchar));
                    } else {
                        nd->unk18 = IRO_UnsignedType(TYPE(&stsignedchar));
                    }
                } else if (saved->size == 2) {
                    sizeclass = 1;
                    if (!is_unsigned(saved) == !is_unsigned(TYPE(&stsignedshort))) {
                        nd->unk18 = TYPE(&stsignedshort);
                    } else if (!is_unsigned(saved)) {
                        nd->unk18 = IRO_SignedType(TYPE(&stsignedshort));
                    } else {
                        nd->unk18 = IRO_UnsignedType(TYPE(&stsignedshort));
                    }
                } else if (saved->size == 4) {
                    sizeclass = 2;
                    if (!is_unsigned(saved) == !is_unsigned(TYPE(&stsignedint))) {
                        nd->unk18 = TYPE(&stsignedint);
                    } else if (!is_unsigned(saved)) {
                        nd->unk18 = IRO_SignedType(TYPE(&stsignedint));
                    } else {
                        nd->unk18 = IRO_UnsignedType(TYPE(&stsignedint));
                    }
                } else if (saved->size == 8) {
                    sizeclass = 3;
                    if (!is_unsigned(saved) == !is_unsigned(TYPE(&stsignedlonglong))) {
                        nd->unk18 = TYPE(&stsignedlonglong);
                    } else if (!is_unsigned(saved)) {
                        nd->unk18 = IRO_SignedType(TYPE(&stsignedlonglong));
                    } else {
                        nd->unk18 = IRO_UnsignedType(TYPE(&stsignedlonglong));
                    }
                }
                saved = nd->unk18;
            }
            if (sizeclass >= 0 && sizeclass <= 2) {
                leftsaved = left->unk18;
                rightsaved = right->unk18;
                if ((leftsaved->type == TYPEENUM ||
                     (leftsaved->type == TYPEINT && TYPE_INTEGRAL(leftsaved)->integral < 0x11)) &&
                    (rightsaved->type == TYPEENUM ||
                     (rightsaved->type == TYPEINT && TYPE_INTEGRAL(rightsaved)->integral < 0x11)) &&
                    leftsaved->type == rightsaved->type) {
                    if (copts.unknown124 || nd->nodetype != 0x1F || sizeclass != 0) {
                        IRO_CPTypeChecker(leftsaved, right, 1);
                    }
                }
            }
            IRO_MarkWalker(left);
            IRO_MarkWalker(right);
            return;
        case IROLinearOp3Arg:
            IRO_MarkWalker(nd->u.args3.a);
            IRO_MarkWalker(nd->u.args3.b);
            IRO_MarkWalker(nd->u.args3.c);
            return;
        case IROLinear5:
            return;
        case IROLinearFunccall:
            for (i = 0; i < nd->u.funccall.argCount; i++) {
                IRO_MarkWalker(nd->u.funccall.args[i]);
            }
            IRO_MarkWalker(nd->u.funccall.callee);
            return;
        default:
            CError_FATAL(1215);
            return;
    }
}

void IRO_DumpType(Type* type) {
    char buf[256];

    IRO_SpellType(type, buf);
    IRO_Dump(" (%s)", buf);
}

CInt64 IRO_MinValueOfIntegerType(Type* type) {
    CInt64 result;
    CInt64 count;

    if (!is_unsigned(type)) {
        goto signed_path;
    }

    result = cint64_zero;
    return result;

signed_path:
    CInt64_SetLong(&count, type->size * 8 - 1);
    result = CInt64_Shl(cint64_one, count);
    IRO_TruncateValueToType(&result, type);
    return result;
}

CInt64 IRO_MaxValueOfIntegerType(Type* type) {
    CInt64 result;
    CInt64 count;
    CInt64 shifted;

    if (!is_unsigned(type)) {
        goto signed_path;
    }

    result = cint64_negone;
    IRO_TruncateValueToType(&result, type);
    return result;

signed_path:
    CInt64_SetLong(&count, type->size * 8 - 1);
    shifted = CInt64_Shl(cint64_one, count);
    result = CInt64_Sub(shifted, cint64_one);
    return result;
}

Type* IRO_PromotedIntegralType(Type* type) {
    if (type->type == TYPEENUM) {
        type = TYPE_ENUM(type)->enumtype;
    }
    if (TYPE_INTEGRAL(type)->integral < 7) {
        if (is_unsigned(type)) {
            return TYPE(&stunsignedint);
        }
        return TYPE(&stsignedint);
    }
    return type;
}

Type* IRO_GetPointerType(Type* type) {
    return TYPE(&stunsignedint);
}

Type* IRO_GetPtrDiffTType(void) {
    return CABI_GetPtrDiffTType();
}

IROLinear* IRO_AddTypeConversion(IROLinear* linear, Type* type, IROList* list) {
    IROLinear* a;
    IROLinear* b;

    if (type->type == TYPEINT && TYPE_INTEGRAL(type)->integral == IT_BOOL) {
        a = IRO_NewLinear(IROLinearOp1Arg);
        *a = *linear;
        a->type = IROLinearOp1Arg;
        a->nodetype = ELOGNOT;
        a->index = IRO_NumLinear++;
        a->rtype = type;
        a->u.monadic = linear;
        b = IRO_NewLinear(IROLinearOp1Arg);
        a->next = b;
        *b = *linear;
        b->type = IROLinearOp1Arg;
        b->nodetype = ELOGNOT;
        b->index = IRO_NumLinear++;
        b->rtype = type;
        b->u.monadic = a;
        b->next = NULL;
        if (list) {
            IRO_AddToList(a, list);
        } else {
            IRO_PasteAfter(a, b, linear);
        }
        return b;
    }

    b = IRO_NewLinear(IROLinearOp1Arg);
    *b = *linear;
    b->type = IROLinearOp1Arg;
    b->nodetype = ETYPCON;
    b->index = IRO_NumLinear++;
    b->rtype = type;
    b->u.monadic = linear;
    b->next = NULL;
    if (IRO_IsValuePreservingTypcon(b) &&
        linear->nodetype == ETYPCON &&
        IRO_IsValuePreservingTypcon(linear)) {
        linear->rtype = type;
        b = linear;
    }
    if (list) {
        IRO_AddToList(b, list);
    } else {
        IRO_PasteAfter(b, b, linear);
    }
    return b;
}

Boolean IRO_IsValuePreservingTypcon(IROLinear* linear) {
    Type* fromtype;
    Type* totype;
    SInt32 fromsize;
    SInt32 tosize;
    Boolean fromunsigned;
    Boolean tounsigned;

    fromtype = linear->u.monadic->rtype;
    totype = linear->rtype;

    if (fromtype->type != TYPEINT ||
        TYPE_INTEGRAL(fromtype)->integral >= IT_17 ||
        totype->type != TYPEINT ||
        TYPE_INTEGRAL(totype)->integral >= IT_17) {
        return FALSE;
    }

    fromsize = fromtype->size;
    tosize = totype->size;
    fromunsigned = is_unsigned(fromtype);
    tounsigned = is_unsigned(totype);
    if (fromunsigned == tounsigned) {
        return tosize >= fromsize;
    }
    if (fromunsigned != TRUE) {
        return FALSE;
    }
    if (tounsigned != FALSE) {
        return FALSE;
    }
    return tosize > fromsize;
}
