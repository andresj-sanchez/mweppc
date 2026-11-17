#include "compiler/CPrep.h"
#include "compiler/RegisterInfo.h"
#include "compiler/CDecl.h"
#include "compiler/CError.h"
#include "compiler/CFunc.h"
#include "compiler/CExpr.h"
#include "compiler/CParser.h"
#include "compiler/CMangler.h"
#include "compiler/CompilerTools.h"
#include "compiler/CScope.h"
#include "compiler/common.h"
#include "compiler/objects.h"
#include "compiler/scopes.h"
#include "compiler/tokens.h"
#include "compiler/types.h"
#include "compiler/CABI.h"
#include "compiler/CInit.h"
#include "compiler/CMachine.h"
#include "compiler/CSOM.h"
#include "compiler/CClass.h"
#include "compiler/CTemplateTools.h"
#include "compiler/CodeGen.h"
#include "driver/COSToolsCLT.h"
#include "compiler/CTemplateNew.h"
#include "compiler/CExpr2.h"

CMidFunctionEntry* cmid_function_entries;
CMidNonGlobalClassEntry* cmid_non_global_classes;

void CParser_SetCFMFlags(Object* object, DeclInfo* declinfo) {
    Object* base;

    if (declinfo && declinfo->exportflags) {
        object->flags |= declinfo->exportflags;
    }

    base = object;
    while (base->datatype == DALIAS) {
        base = base->u.alias.object;
    }
    if (base->datatype == DDATA) {
        if (copts.cfm_export) {
            object->flags |= 0x40;
        }
        if (copts.cfm_internal) {
            object->flags |= 0x10;
        }
        return;
    }
    if (copts.cfm_internal) {
        object->flags |= 0x10;
        return;
    }
    if (copts.cfm_import) {
        object->flags |= 0x20;
    }
    if (copts.cfm_export) {
        object->flags |= 0x40;
    }
    if (copts.cfm_lib_export) {
        object->flags |= 0x60;
    }
}

void CParser_UpdateObject(Object* object, DeclInfo* declinfo) {
    if (declinfo && declinfo->section) {
        object->section = declinfo->section;
    }
    if (declinfo && object->datatype == DDATA) {
        if (!object->u.data.reginfo) {
            object->u.data.reginfo = galloc(sizeof(ObjRegInfo));
            memclrw(object->u.data.reginfo, sizeof(ObjRegInfo));
            object->u.data.reginfo = declinfo->reginfo;
        } else if (object->u.data.reginfo->reg == 0) {
            if (object->u.data.reginfo->rclass != 5) {
                object->u.data.reginfo = declinfo->reginfo;
            }
        }
    }
    CParser_SetCFMFlags(object, declinfo);
    fn_00433EF0(object);
}

Object* CParser_NewCompilerDefFunctionObject(void) {
    Object* object = galloc(sizeof(Object));
    memclrw(object, sizeof(Object));
    object->otype = OT_OBJECT;
    object->access = ACCESSPUBLIC;
    object->section = 0;
    object->datatype = DFUNC;
    object->nspace = cscope_root;
    return object;
}

Object* CParser_NewCompilerDefDataObject(void) {
    Object* object = galloc(sizeof(Object));
    memclrw(object, sizeof(Object));
    object->otype = OT_OBJECT;
    object->access = ACCESSPUBLIC;
    object->section = 0;
    object->datatype = DDATA;
    object->nspace = cscope_root;
    return object;
}

Object* CParser_NewFunctionObject(DeclInfo* declinfo) {
    Object* object = galloc(sizeof(Object));
    memclrw(object, sizeof(Object));
    object->otype = OT_OBJECT;
    object->access = ACCESSPUBLIC;
    object->section = 0;
    object->datatype = DFUNC;
    object->nspace = cscope_current;

    if (declinfo) {
        object->type = declinfo->thetype;
        object->name = declinfo->name;
        object->qual = declinfo->qual;
        object->sclass = declinfo->storageclass;
        if (copts.cplusplus && !declinfo->is_extern_c) {
            object->qual |= Q_MANGLE_NAME;
        }
    }

    CParser_UpdateObject(object, declinfo);
    return object;
}

Object* CParser_NewGlobalDataObject(DeclInfo* declinfo) {
    Object* object = galloc(sizeof(Object));
    memclrw(object, sizeof(Object));
    object->otype = OT_OBJECT;
    object->access = ACCESSPUBLIC;
    object->section = 0;
    object->datatype = DDATA;
    object->nspace = cscope_current;

    if (declinfo) {
        object->type = declinfo->thetype;
        object->name = declinfo->name;
        object->qual = declinfo->qual;
        object->sclass = declinfo->storageclass;
        if (copts.cplusplus && !declinfo->is_extern_c) {
            object->qual |= Q_MANGLE_NAME;
        }
    }

    CParser_UpdateObject(object, declinfo);
    return object;
}

Object* CParser_NewLocalDataObject(DeclInfo* declinfo, Boolean add_to_locals) {
    Object* object;
    SInt32 local_id;

    if (CTemplTool_IsInTemplate()) {
        object = galloc(sizeof(Object));
    } else {
        object = lalloc(sizeof(Object));
    }
    memclrw(object, sizeof(Object));
    object->otype = OT_OBJECT;
    object->access = ACCESSPUBLIC;
    object->datatype = DLOCAL;
    local_id = uniqueid++;
    object->u.unk2C = local_id;

    if (declinfo) {
        object->type = declinfo->thetype;
        object->name = declinfo->name;
        object->qual = declinfo->qual;
        object->sclass = declinfo->storageclass;
    }

    if (add_to_locals) {
        fn_0047BCC0(object);
    }

    return object;
}

Object* CParser_NewObject(DeclInfo* declinfo) {
    Object* object = galloc(sizeof(Object));
    memclrw(object, sizeof(Object));
    CParser_SetCFMFlags(object, declinfo);
    object->otype = OT_OBJECT;
    object->access = ACCESSPUBLIC;
    object->section = 0;
    return object;
}

Boolean islookaheaddeclaration(void) {
    SInt32 state;

    fn_004446B0(&state);
    return FALSE;
}

Boolean isdeclaration(UInt8 scanDeclarator, UInt8 requireName, UInt8 isNewExpression, short token) {
    SInt32 state;

    fn_004446B0(&state);
    return FALSE;
}

Boolean is_unsigned(Type* type) {
    if (type->type == TYPEENUM) {
        type = TYPE_ENUM(type)->enumtype;
    }

    return CMach_IsUnsignedType(type);
}

Boolean CParser_HasInternalLinkage2(Object* object) {
    if (object->datatype == DLOCAL) {
        return TRUE;
    }
    if (object->qual & 0x60000) {
        return FALSE;
    }
    if (object->sclass == 0x102) {
        return TRUE;
    }
    if (object->qual & 0x10) {
        object->qual |= 0x20000;
    }
    return FALSE;
}

Boolean CParser_HasInternalLinkage(Object* object) {
    NameSpace* nspace = object->nspace;

    while (nspace) {
        if (nspace->is_unnamed) {
            return TRUE;
        }
        nspace = nspace->parent;
    }

    if (object->datatype == DLOCAL) {
        return TRUE;
    }
    if (object->qual & 0x60000) {
        return FALSE;
    }
    if (object->sclass == 0x102) {
        return TRUE;
    }
    if (object->qual & 0x10) {
        object->qual |= 0x20000;
    }
    return FALSE;
}

void CParser_Cleanup(void) {
    CTempl_Cleanup();
    fn_0052DEC0();
    CScope_Cleanup();
    FreeGList(&name_mangle_list);
}

UInt32 CParser_GetTypeQualifiers(Type* type, UInt32 qual) {
    while (IS_TYPE_ARRAY(type)) {
        type = TYPE_POINTER(type)->target;
    }

    switch ((SInt8)type->type) {
        case TYPEPOINTER:
            qual = TYPE_POINTER(type)->qual;
            break;
        case TYPEMEMBERPOINTER:
            qual = TYPE_MEMBER_POINTER(type)->qual;
            break;
        default:
            break;
    }

    return qual;
}

UInt32 CParser_GetCVTypeQualifiers(Type* type, UInt32 qual) {
    while (IS_TYPE_ARRAY(type)) {
        type = TYPE_POINTER(type)->target;
    }

    switch ((SInt8)type->type) {
        case TYPEPOINTER:
            qual = TYPE_POINTER(type)->qual;
            break;
        case TYPEMEMBERPOINTER:
            qual = TYPE_MEMBER_POINTER(type)->qual;
            break;
        default:
            break;
    }

    return qual & 3;
}

Boolean CParserIsConstExpr(ENode* expr) {
    UInt16 qual;
    Type* type;

    qual = expr->flags & 3;
    type = expr->rtype;

    while (type->type == TYPEARRAY) {
        type = TYPE_POINTER(type)->target;
    }

    switch ((SInt8)type->type) {
        case TYPEPOINTER:
            qual = TYPE_POINTER(type)->qual;
            break;
        case TYPEMEMBERPOINTER:
            qual = TYPE_MEMBER_POINTER(type)->qual;
            break;
        default:
            break;
    }

    return (qual & Q_CONST) != 0;
}

Boolean CParserIsVolatileExpr(ENode* expr) {
    UInt16 qual;
    Type* type;

    qual = expr->flags & 3;
    type = expr->rtype;

    while (type->type == TYPEARRAY) {
        type = TYPE_POINTER(type)->target;
    }

    switch ((SInt8)type->type) {
        case TYPEPOINTER:
            qual = TYPE_POINTER(type)->qual;
            break;
        case TYPEMEMBERPOINTER:
            qual = TYPE_MEMBER_POINTER(type)->qual;
            break;
        default:
            break;
    }

    return (qual & Q_VOLATILE) != 0;
}

Type* CParser_GetWCharType(void) {
    if (copts.cplusplus && copts.wchar_type) {
        return TYPE(&stwchar);
    }
    return copts.ushort_wchar_t ? TYPE(&stunsignedshort) : TYPE(&stsignedint);
}

Boolean CParser_IsPublicRuntimeObject(Object* object) {
    if (newp_fobj->first.object == object && !newp_fobj->first.next) {
        return TRUE;
    }
    if (newa_fobj->first.object == object && !newa_fobj->first.next) {
        return TRUE;
    }
    if (delp_fobj->first.object == object && !delp_fobj->first.next) {
        return TRUE;
    }
    if (dela_fobj->first.object == object && !dela_fobj->first.next) {
        return TRUE;
    }
    return CodeGen_IsPublicRuntimeObject(object);
}

Object* CParser_NewAliasObject(Object* object, SInt32 offset) {
    Object* alias = galloc(sizeof(Object));
    *alias = *object;
    alias->datatype = DALIAS;
    alias->u.alias.object = object;
    alias->u.alias.offset = offset;
    alias->nspace = cscope_current;
    CScope_AddObject(cscope_current, alias->name, OBJ_BASE(alias));
    return alias;
}

static Boolean fn_0044AA60(TypeClass* theclass) {
    ClassList* base;
    ObjMemberVar* member;
    Type* type;
    UInt32 qual;

    for (base = theclass->bases; base; base = base->next) {
        if (fn_0044AA60(base->base)) {
            return TRUE;
        }
    }

    for (member = theclass->ivars; member; member = member->next) {
        type = member->type;
        qual = member->qual;
        while (type->type == TYPEARRAY) {
            type = TYPE_POINTER(type)->target;
        }
        switch ((SInt8)type->type) {
            case TYPEPOINTER:
                qual = TYPE_POINTER(type)->qual;
                break;
            case TYPEMEMBERPOINTER:
                qual = TYPE_MEMBER_POINTER(type)->qual;
                break;
            default:
                break;
        }
        if (qual & Q_MUTABLE) {
            return TRUE;
        }
        if (type->type == TYPECLASS) {
            if (fn_0044AA60(TYPE_CLASS(type))) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

Boolean CParser_IsReadOnly(Type* type, UInt32 qual) {
    while (IS_TYPE_ARRAY(type)) {
        type = TYPE_POINTER(type)->target;
    }

    switch ((SInt8)type->type) {
        case TYPEPOINTER:
            qual = TYPE_POINTER(type)->qual;
            break;
        case TYPEMEMBERPOINTER:
            qual = TYPE_MEMBER_POINTER(type)->qual;
            break;
        default:
            break;
    }

    if (qual & Q_CONST) {
        return type->type != TYPECLASS || !fn_0044AA60(TYPE_CLASS(type));
    }
    return FALSE;
}

SInt16 CParser_GetOperator(ENodeType t) {
    switch (t) {
        default:
            CError_FATAL(1981);
        case EMONMIN:
            return '-';
        case EBINNOT:
            return '~';
        case ELOGNOT:
            return '!';
        case EADD:
            return '+';
        case ESUB:
            return '-';
        case EMUL:
            return '*';
        case EDIV:
            return '/';
        case EMODULO:
            return '%';
        case EAND:
            return '&';
        case EXOR:
            return '^';
        case EOR:
            return '|';
        case ESHL:
            return TK_SHL;
        case ESHR:
            return TK_SHR;
        case EMIN:
            return TK_MIN;
        case EMAX:
            return TK_MAX;
        case ELESS:
            return '<';
        case EGREATER:
            return '>';
        case ELESSEQU:
            return TK_LESS_EQUAL;
        case EGREATEREQU:
            return TK_GREATER_EQUAL;
        case EEQU:
            return TK_LOGICAL_EQ;
        case ENOTEQU:
            return TK_LOGICAL_NE;
    }
    return '-';
}

void appendmember(TypeStruct* tstruct, StructMember* member) {
    StructMember* node;

    node = tstruct->members;
    if (!node) {
        tstruct->members = member;
        return;
    }
    while (node->next) {
        node = node->next;
    }
    {
        Type* type = node->type;
        if (type->type == TYPEARRAY && type->size == 0) {
            CError_Error(0x137);
        }
    }
    node->next = member;
}

Type* CParser_RemoveTopMostQualifiers(Type* type, UInt32* qual) {
    switch ((SInt8)type->type) {
        case TYPEARRAY:
            TYPE_POINTER(type)->target =
                CParser_RemoveTopMostQualifiers(TYPE_POINTER(type)->target, qual);
            return type;
        case TYPEPOINTER:
            if (TYPE_POINTER(type)->qual & Q_CONST) {
                TypePointer* newtype = galloc(sizeof(TypePointer));
                *newtype = *TYPE_POINTER(type);
                newtype->qual = 0;
                return TYPE(newtype);
            }
            return type;
        case TYPEMEMBERPOINTER:
            if (TYPE_MEMBER_POINTER(type)->qual & Q_CONST) {
                TypeMemberPointer* newtype = galloc(sizeof(TypeMemberPointer));
                *newtype = *TYPE_MEMBER_POINTER(type);
                newtype->qual = 0;
                return TYPE(newtype);
            }
            return type;
        default:
            *qual = 0;
            return type;
    }
}

#define Q_RESTRICT 0x200000
#define Q_ALIGNED_MASK 0x1E000000

void TypedefDeclInfo(DeclInfo* declinfo, Type* type, UInt32 qual) {
    if (type->type == TYPEPOINTER) {
        if (IS_TYPEPOINTER_REFERENCE(TYPE_POINTER(type))) {
            declinfo->thetype = type;
            declinfo->qual &= ~(Q_RESTRICT | Q_REFERENCE | Q_VOLATILE | Q_CONST);
            declinfo->qual |= qual;
            return;
        }

        declinfo->thetype = galloc(sizeof(TypePointer));
        *TYPE_POINTER(declinfo->thetype) = *TYPE_POINTER(type);
        TYPE_POINTER(declinfo->thetype)->qual |=
            declinfo->qual & (Q_RESTRICT | Q_REFERENCE | Q_VOLATILE | Q_CONST);
        declinfo->qual &= ~(Q_RESTRICT | Q_REFERENCE | Q_VOLATILE | Q_CONST);
        declinfo->qual |= qual & (Q_ALIGNED_MASK | Q_REFERENCE | Q_PASCAL | Q_VOLATILE | Q_CONST);
    } else if (type->type == TYPEMEMBERPOINTER) {
        declinfo->thetype = galloc(sizeof(TypeMemberPointer));
        *TYPE_MEMBER_POINTER(declinfo->thetype) = *TYPE_MEMBER_POINTER(type);
        TYPE_MEMBER_POINTER(declinfo->thetype)->qual |=
            declinfo->qual & (Q_RESTRICT | Q_REFERENCE | Q_VOLATILE | Q_CONST);
        declinfo->qual &= ~(Q_RESTRICT | Q_REFERENCE | Q_VOLATILE | Q_CONST);
        declinfo->qual |= qual & (Q_ALIGNED_MASK | Q_REFERENCE | Q_PASCAL | Q_VOLATILE | Q_CONST);
    } else {
        declinfo->thetype = type;
        declinfo->qual |= qual & (Q_ALIGNED_MASK | Q_REFERENCE | Q_PASCAL | Q_VOLATILE | Q_CONST);
        if (IS_TYPE_ARRAY(declinfo->thetype) && !declinfo->thetype->size) {
            if (declinfo->qual & Q_CONST) {
                CError_Warning(22, 0);
            } else if (declinfo->qual & Q_VOLATILE) {
                CError_Warning(2, 0);
            }
            declinfo->thetype = galloc(sizeof(TypeMemberPointer));
            *TYPE_MEMBER_POINTER(declinfo->thetype) = *TYPE_MEMBER_POINTER(type);
            declinfo->isType = 1;
        }
    }
}

SInt16 GetPrec(SInt16 token) {
    switch (token) {
        case '%':
        case '*':
        case '/':
            return 11;
        case '+':
        case '-':
            return 10;
        case TK_SHL:
        case TK_SHR:
            return 9;
        case '<':
        case '>':
        case TK_LESS_EQUAL:
        case TK_GREATER_EQUAL:
            return 8;
        case TK_LOGICAL_EQ:
        case TK_LOGICAL_NE:
            return 7;
        case '&':
            return 6;
        case '^':
            return 5;
        case '|':
            return 4;
        case TK_LOGICAL_AND:
            return 3;
        case TK_LOGICAL_OR:
            return 2;
        default:
            return 0;
    }
}

#define FUNC_CALL_CONV_MASK 0xF0018001

SInt16 is_memberpointerequal(Type* a, Type* b) {
    FuncArg* arg_a;
    FuncArg* arg_b;

    if (a->type != b->type) {
        return FALSE;
    }
    if (!IS_TYPE_FUNC(a)) {
        return is_typeequal(a, b);
    }

    if (!is_typesame(TYPE_FUNC(a)->functype, TYPE_FUNC(b)->functype)) {
        return FALSE;
    }

    if ((TYPE_FUNC(a)->flags & FUNC_CALL_CONV_MASK) !=
        (TYPE_FUNC(b)->flags & FUNC_CALL_CONV_MASK)) {
        return FALSE;
    }

    CError_ASSERT(1578, arg_a = TYPE_FUNC(a)->args);
    CError_ASSERT(1579, arg_b = TYPE_FUNC(b)->args);

    if (TYPE_FUNC(a)->flags & FUNC_PTM) {
        CError_ASSERT(1584, arg_a = arg_a->next);
    }

    if (TYPE_FUNC(b)->flags & FUNC_PTM) {
        CError_ASSERT(1588, arg_b = arg_b->next);
    }

    if (arg_a->qual != arg_b->qual) {
        return FALSE;
    }

    arg_a = arg_a->next;
    arg_b = arg_b->next;
    return is_arglistsame(arg_a, arg_b);
}

#include <stdarg.h>

Object* CParser_NewRTFunc(Type* rettype, HashNameNode* name, UInt8 kind, int argcount, ...) {
    Object* obj;
    FuncArg* args;
    FuncArg* arg;
    TypeFunc* tfunc;
    va_list va;

    args = NULL;
    if (argcount) {
        va_start(va, argcount);
        while (--argcount >= 0) {
            if (args) {
                arg->next = CParser_NewFuncArg();
                arg = arg->next;
            } else {
                args = arg = CParser_NewFuncArg();
            }
            arg->type = va_arg(va, Type*);
        }
        va_end(va);
    }

    obj = CParser_NewFunctionObject(NULL);

    tfunc = galloc(sizeof(TypeFunc));
    memclrw(tfunc, sizeof(TypeFunc));
    tfunc->type = TYPEFUNC;
    tfunc->functype = rettype;
    tfunc->args = args;
    CDecl_SetFuncFlags(tfunc, 0);

    obj->name = name;
    obj->type = TYPE(tfunc);
    if (kind == 1) {
        obj->qual = Q_MANGLE_NAME;
    }

    fn_00433EF0(obj);
    return obj;
}

void CParser_Setup(void) {
    CScope_Setup();

    name_mangle_list.data = NULL;
    if (InitGList(&name_mangle_list, 256)) {
        CError_NoMem();
    }

    void_ptr.size = 4;
    fn_00455C40();
    fn_00519620();
    fn_00512B20();
    fn_0052DED0();
    CClass_Init();

    cprep_in_assembler = 0;
    in_func_arglist = 0;
    uniqueid = 1;
    dont_set_references = 0;
    copts.sideeffects = 1;
    cparser_unknown_setup_flag = 0;
    cparser_classactions = 0;
    name_obj_check = 0;
    callbackactions = 0;
    cparser_parentcleanup = 0;
    init_expressions = 0;
    cmid_function_entries = 0;
    trychain = 0;
    cmid_non_global_classes = 0;

    memclrw(&cparser_fileoffset, sizeof(FileOffsetInfo));

    memclrw(&catchinfostruct, sizeof(TypeStruct));
    catchinfostruct.type = TYPESTRUCT;
    catchinfostruct.size = 0x18;
    catchinfostruct.stype = 0;
    catchinfostruct.align = 4;

    memclrw(&ptmstruct, sizeof(TypeStruct));
    ptmstruct.type = TYPESTRUCT;
    ptmstruct.size = 0xC;
    ptmstruct.stype = 0;
    ptmstruct.align = 4;

    CMach_Configure();
    CTempl_Setup();

    uniquenamespacename = 0;
    disallowgreaterthan = 0;
    etempid = 0;
    unique_name_file_valid = 0;

    CParser_SetupRuntimeObjects();
}

Boolean CParser_ParseOperatorName(
    short* token, Boolean allowConversion, Boolean parseConversionType
) {
    HashNameNode* name;

    switch ((tk = lex())) {
        case TK_NEW:
        case TK_DELETE:
            if (lookahead() == '[') {
                lex();
                if (lex() != ']') {
                    CError_Error(0x1A);
                }
                tk = (tk == TK_NEW) ? TK_NEW_ARRAY : TK_DELETE_ARRAY;
            }
            break;
        case '(':
            if ((tk = lex()) != ')') {
                CError_Error(0x69);
                return FALSE;
            }
            tk = '(';
            break;
        case '[':
            if ((tk = lex()) != ']') {
                CError_Error(0x69);
                return FALSE;
            }
            tk = '[';
            break;
    }

    if ((name = CMangler_OperatorName(tk))) {
        if (token) {
            *token = tk;
        }
        tk = lex();
        tkidentifier = name;
        return TRUE;
    }

    if (allowConversion) {
        if (parseConversionType) {
            DeclInfo declinfo;
            memclrw(&declinfo, sizeof(DeclInfo));
            conversion_type_name(&declinfo);
            tkidentifier = CMangler_ConversionFuncName(declinfo.thetype, declinfo.qual);
        }
        if (token) {
            *token = 0;
        }
        return TRUE;
    } else {
        CError_Error(0x69);
        return FALSE;
    }
}

Boolean CParser_ReInitRuntimeObjects(Boolean is_precompiler) {
    if (!(newp_fobj = CScope_FindNameSpaceName(cscope_root, CMangler_OperatorName(TK_NEW)))) {
        return FALSE;
    }
    if (!(newa_fobj = CScope_FindNameSpaceName(cscope_root, CMangler_OperatorName(TK_NEW_ARRAY)))) {
        return FALSE;
    }
    if (!(delp_fobj = CScope_FindNameSpaceName(cscope_root, CMangler_OperatorName(TK_DELETE)))) {
        return FALSE;
    }
    if (!(dela_fobj =
              CScope_FindNameSpaceName(cscope_root, CMangler_OperatorName(TK_DELETE_ARRAY)))) {
        return FALSE;
    }

    newh_func->name = GetHashNameNodeExport("__new_hdl");
    runtime_object_6363E8->name = GetHashNameNodeExport("__del_hdl");
    runtime_object_63665C->name = GetHashNameNodeExport("__copy");
    runtime_object_636D78->name = GetHashNameNodeExport("__clear");
    runtime_object_6366E4->name = GetHashNameNodeExport("__get_typeid");
    runtime_object_636810->name = GetHashNameNodeExport("__dynamic_cast");
    runtime_object_635B60->name = GetHashNameNodeExport("__ptmf_cast");
    runtime_object_63605C->name = GetHashNameNodeExport("__ptmf_cmpr");
    runtime_object_636044->name = GetHashNameNodeExport("__ptmf_test");
    runtime_object_635B94->name = GetHashNameNodeExport("__ptmf_call");
    runtime_object_63641C->name = GetHashNameNodeExport("__ptmf_scall");
    runtime_object_63628C->name = GetHashNameNodeExport("__ptmf_call4");
    runtime_object_636A00->name = GetHashNameNodeExport("__ptmf_scall4");
    runtime_object_6361AC->name = GetHashNameNodeExport("__ptmf_null");
    runtime_object_6366DC->name = GetHashNameNodeExport("__som_new");
    runtime_object_635AB8->name = GetHashNameNodeExport("__som_check_new");
    runtime_object_636170->name = GetHashNameNodeExport("__som_check_ev");
    runtime_object_635F08->name = GetHashNameNodeExport("_som_ptrgl4");
    runtime_object_635F0C->name = GetHashNameNodeExport("_som_ptrgl5");
    runtime_object_635F10->name = GetHashNameNodeExport("_som_ptrgl_");
    runtime_object_6361C0->name = GetHashNameNodeExport("__construct_array");
    runtime_object_636638->name = GetHashNameNodeExport("__construct_new_array");
    darr_func->name = GetHashNameNodeExport("__destroy_arr");
    runtime_object_636658->name = GetHashNameNodeExport("__destroy_new_array");
    runtime_object_636948->name = GetHashNameNodeExport("__destroy_new_array3");
    runtime_object_6367B0->name = GetHashNameNodeExport("__register_global_object");
    runtime_object_636D9C->name = GetHashNameNodeExport("__throw");
    runtime_object_636D38->name = GetHashNameNodeExport("__init__catch");
    runtime_object_636D94->name = GetHashNameNodeExport("__end__catch");
    runtime_object_636D2C->name = GetHashNameNodeExport("__unexpected");
    runtime_object_635B80->name = GetHashNameNodeExport("__register_classobj");
    runtime_object_636840->name = GetHashNameNodeExport("__unregister_classobj");

    CMangler_Setup();

    no_name_node_635BA0 = GetHashNameNodeExport("@no_name@");
    temp_argument_name_636C9C = GetHashNameNodeExport("@temp_ptr@");
    this_name_node = GetHashNameNodeExport("this");
    self_name_node_636D84 = GetHashNameNodeExport("self");
    vptr_name_node = GetHashNameNodeExport("__vptr$");

    CSOM_Setup(is_precompiler);
    return CodeGen_ReInitRuntimeObjects(is_precompiler);
}

void CParser_SetupRuntimeObjects(void) {
    Object* func;
    Type* sizet;
    ExceptSpecList* exspecs;

    exspecs = galloc(sizeof(ExceptSpecList));
    memclrw(exspecs, sizeof(ExceptSpecList));

    sizet = CABI_GetSizeTType();

    func = CParser_NewRTFunc(TYPE(&void_ptr), CMangler_OperatorName(TK_NEW), 1, 1, sizet);
    CScope_AddGlobalObject(func);

    func = CParser_NewRTFunc(TYPE(&void_ptr), CMangler_OperatorName(TK_NEW_ARRAY), 1, 1, sizet);
    CScope_AddGlobalObject(func);

    func = CParser_NewRTFunc(TYPE(&stvoid), CMangler_OperatorName(TK_DELETE), 1, 1, &void_ptr);
    CError_ASSERT(414, IS_TYPE_FUNC(func->type));
    TYPE_FUNC(func->type)->exspecs = exspecs;
    CScope_AddGlobalObject(func);

    func =
        CParser_NewRTFunc(TYPE(&stvoid), CMangler_OperatorName(TK_DELETE_ARRAY), 1, 1, &void_ptr);
    CError_ASSERT(422, IS_TYPE_FUNC(func->type));
    TYPE_FUNC(func->type)->exspecs = exspecs;
    CScope_AddGlobalObject(func);

    newh_func = CParser_NewRTFunc(TYPE(&void_ptr), NULL, 0, 1, sizet);
    runtime_object_6363E8 = CParser_NewRTFunc(TYPE(&stvoid), NULL, 0, 1, &void_ptr);

    runtime_object_6366E4 =
        CParser_NewRTFunc(TYPE(&void_ptr), NULL, 0, 2, &stsignedlong, &void_ptr);
    runtime_object_636810 = CParser_NewRTFunc(
        TYPE(&void_ptr), NULL, 0, 5, &void_ptr, &stsignedlong, &void_ptr, &void_ptr, &stsignedshort
    );

    runtime_object_63665C =
        CParser_NewRTFunc(TYPE(&void_ptr), NULL, 2, 3, &void_ptr, &void_ptr, sizet);
    runtime_object_636D78 = CParser_NewRTFunc(TYPE(&void_ptr), NULL, 2, 2, &void_ptr, sizet);

    runtime_object_635B60 =
        CParser_NewRTFunc(TYPE(&void_ptr), NULL, 2, 3, &stsignedlong, &void_ptr, &void_ptr);
    runtime_object_63605C =
        CParser_NewRTFunc(TYPE(&stsignedlong), NULL, 2, 2, &void_ptr, &void_ptr);
    runtime_object_636044 = CParser_NewRTFunc(TYPE(&stsignedlong), NULL, 2, 1, &void_ptr);

    runtime_object_635B94 = CParser_NewRTFunc(TYPE(&stvoid), NULL, 2, 0);
    runtime_object_63641C = CParser_NewRTFunc(TYPE(&stvoid), NULL, 2, 0);
    runtime_object_63628C = CParser_NewRTFunc(TYPE(&stvoid), NULL, 2, 0);
    runtime_object_636A00 = CParser_NewRTFunc(TYPE(&stvoid), NULL, 2, 0);

    runtime_object_6361AC = CParser_NewGlobalDataObject(NULL);
    runtime_object_6361AC->type = TYPE(&stvoid);

    runtime_object_635B10 =
        CParser_NewRTFunc(TYPE(&stvoid), GetHashNameNodeExport("__builtin_next_arg"), 2, 0);
    TYPE_FUNC(runtime_object_635B10->type)->flags |= 0x200;

    runtime_object_6366DC =
        CParser_NewRTFunc(TYPE(&void_ptr), NULL, 2, 3, &void_ptr, &stsignedlong, &stsignedlong);
    runtime_object_635AB8 = CParser_NewRTFunc(TYPE(&stvoid), NULL, 0, 1, &void_ptr);
    runtime_object_636170 = CParser_NewRTFunc(TYPE(&stvoid), NULL, 0, 1, &void_ptr);
    runtime_object_635F08 = CParser_NewRTFunc(TYPE(&stvoid), NULL, 2, 0);
    runtime_object_635F0C = CParser_NewRTFunc(TYPE(&stvoid), NULL, 2, 0);
    runtime_object_635F10 = CParser_NewRTFunc(TYPE(&stvoid), NULL, 2, 0);

    runtime_object_6361C0 =
        CParser_NewRTFunc(TYPE(&stvoid), NULL, 0, 5, &void_ptr, &void_ptr, &void_ptr, sizet, sizet);
    runtime_object_636638 = CParser_NewRTFunc(
        TYPE(&void_ptr), NULL, 0, 5, &void_ptr, &void_ptr, &void_ptr, sizet, sizet
    );
    darr_func = CParser_NewRTFunc(TYPE(&stvoid), NULL, 0, 4, &void_ptr, &void_ptr, sizet, sizet);
    runtime_object_636658 = CParser_NewRTFunc(TYPE(&stvoid), NULL, 0, 2, &void_ptr, &void_ptr);
    runtime_object_636948 = CParser_NewRTFunc(
        TYPE(&stvoid), NULL, 0, 4, &void_ptr, &void_ptr, &void_ptr, &stsignedshort
    );

    runtime_object_6367B0 =
        CParser_NewRTFunc(TYPE(&void_ptr), NULL, 0, 3, &void_ptr, &void_ptr, &void_ptr);
    runtime_object_636D9C =
        CParser_NewRTFunc(TYPE(&stvoid), NULL, 0, 3, &void_ptr, &void_ptr, &void_ptr);
    runtime_object_636D38 = CParser_NewRTFunc(TYPE(&stvoid), NULL, 0, 1, &void_ptr);
    runtime_object_636D94 = CParser_NewRTFunc(TYPE(&stvoid), NULL, 0, 1, &void_ptr);
    runtime_object_636D2C = CParser_NewRTFunc(TYPE(&stvoid), NULL, 0, 1, &void_ptr);

    runtime_object_635B80 = CParser_NewRTFunc(TYPE(&void_ptr), NULL, 0, 2, &void_ptr, &void_ptr);
    runtime_object_636840 = CParser_NewRTFunc(TYPE(&void_ptr), NULL, 0, 1, &void_ptr);

    CodeGen_SetupRuntimeObjects();
    CError_ASSERT(581, CParser_ReInitRuntimeObjects(0));
}

static void CParser_PrintUniqueID(char* buf);

HashNameNode* CParser_AppendUniqueNameFile(char* prefix) {
    char buf[256];
    char* src;
    char* dst;
    char c;
    int i;
    int j;
    int len;

    dst = buf;
    for (i = 0; *prefix && i < 200; i++) {
        *(dst++) = *(prefix++);
    }
    *(dst++) = '$';

    CParser_PrintUniqueID(dst);

    while (*dst) {
        dst++;
        i++;
    }

    COS_FileGetFileSpecInfo(&cparamblkptr->sourcefile, NULL, NULL, unique_name_file_601770, 0x104);
    unique_name_file_valid = 1;
    src = (char*)&unique_name_file_601770[1];
    len = unique_name_file_601770[0];
    for (j = 0; j < len && i < 255; j++, i++) {
        c = *(src++);
        if (!(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') && !(c >= '0' && c <= '9')) {
            c = '_';
        }
        *(dst++) = c;
    }

    dst[0] = 0;
    return GetHashNameNodeExport(buf);
}

static void CParser_PrintUniqueID(char* buf) {
    char* ptr;
    SInt32 id;
    char mybuf[16];

    ptr = mybuf;
    id = CParser_GetUniqueID();
    while (id) {
        *(ptr++) = '0' + id % 10;
        id = id / 10;
    }

    while (ptr > mybuf) {
        *(buf++) = *(--ptr);
    }

    *buf = 0;
}

SInt32 CParser_GetUniqueID(void) {
    return uniqueid++;
}

void CParser_SetUniqueID(SInt32 id) {
    uniqueid = id;
}

HashNameNode* CParser_GetUniqueName(void) {
    char buf[16];
    buf[0] = '@';
    CParser_PrintUniqueID(buf + 1);
    return GetHashNameNodeExport(buf);
}

HashNameNode* CParser_AppendUniqueName(char* prefix) {
    char buf[256];
    char* dst;
    int i;

    dst = buf;
    for (i = 0; *prefix && i < 240; i++) {
        *(dst++) = *(prefix++);
    }
    *(dst++) = '$';

    CParser_PrintUniqueID(dst);
    return GetHashNameNodeExport(buf);
}

Boolean IsTempName(HashNameNode* name) {
    return name == NULL || (name->name[0] == '@') || (name->name[0] == '$');
}

StructMember* ismember(TypeStruct* tstruct, HashNameNode* name) {
    StructMember* member;

    for (member = tstruct->members; member; member = member->next) {
        if (member->name == name) {
            return member;
        }
    }

    return NULL;
}

FuncArg* CParser_NewFuncArg(void) {
    FuncArg* arg = galloc(sizeof(FuncArg));
    memclrw(arg, sizeof(FuncArg));
    return arg;
}

Type* CParser_GetBoolType(void) {
    if (copts.cplusplus && copts.booltruefalse) {
        return TYPE(&stbool);
    } else {
        return TYPE(&stsignedint);
    }
}

Boolean CParser_IsMoreCVQualified(UInt32 a, UInt32 b) {
    if ((a & Q_CONST) && !(b & Q_CONST)) {
        return (((a & Q_VOLATILE) != 0) || (b & Q_VOLATILE) == 0);
    } else if ((a & Q_VOLATILE) && !(b & Q_VOLATILE)) {
        return ((a & Q_CONST) || (b & Q_CONST) == 0);
    }
    return FALSE;
}

Boolean CParser_IsSameOrMoreCVQualified(UInt32 a, UInt32 b) {
    if ((a & (Q_CONST | Q_VOLATILE)) == (b & (Q_CONST | Q_VOLATILE))) {
        return TRUE;
    }
    if ((a & Q_CONST) && !(b & Q_CONST)) {
        return (a & Q_VOLATILE) || !(b & Q_VOLATILE);
    }
    if (!(a & Q_VOLATILE)) {
        return FALSE;
    }
    if (b & Q_VOLATILE) {
        return FALSE;
    }
    return (a & Q_CONST) || !(b & Q_CONST);
}

Boolean CParser_IsConst(Type* type, UInt32 qual) {
    while (IS_TYPE_ARRAY(type)) {
        type = TYPE_POINTER(type)->target;
    }

    switch ((SInt8)type->type) {
        case TYPEPOINTER:
            qual = TYPE_POINTER(type)->qual;
            break;
        case TYPEMEMBERPOINTER:
            qual = TYPE_MEMBER_POINTER(type)->qual;
            break;
        default:
            break;
    }

    return (qual & Q_CONST) != 0;
}

Boolean CParser_IsVolatile(Type* type, UInt32 qual) {
    while (IS_TYPE_ARRAY(type)) {
        type = TYPE_POINTER(type)->target;
    }

    switch ((SInt8)type->type) {
        case TYPEPOINTER:
            qual = TYPE_POINTER(type)->qual;
            break;
        case TYPEMEMBERPOINTER:
            qual = TYPE_MEMBER_POINTER(type)->qual;
            break;
        default:
            break;
    }

    return (qual & Q_VOLATILE) != 0;
}

Boolean is_const_object(Object* obj) {
    return CParser_IsConst(obj->type, obj->qual);
}

Boolean is_volatile_object(Object* obj) {
    UInt32 qual = obj->qual;
    Type* type = obj->type;

    while (IS_TYPE_ARRAY(type)) {
        type = TYPE_POINTER(type)->target;
    }

    switch ((SInt8)type->type) {
        case TYPEPOINTER:
            qual = TYPE_POINTER(type)->qual;
            break;
        case TYPEMEMBERPOINTER:
            qual = TYPE_MEMBER_POINTER(type)->qual;
            break;
        default:
            break;
    }

    return (qual & Q_VOLATILE) != 0;
}

Boolean CParser_IsVirtualFunction(Object* obj, TypeClass** tclass, SInt32* index) {
    if (obj->datatype == DVFUNC) {
        *tclass = TYPE_METHOD(obj->type)->theclass;
        *index = TYPE_METHOD(obj->type)->vtbl_index;
        return TRUE;
    }

    return FALSE;
}

Boolean is_pascal_object(Object* obj) {
    return IS_TYPE_FUNC(obj->type) && (TYPE_FUNC(obj->type)->flags & FUNC_PASCAL);
}

static Boolean oldstylecompatible(FuncArg* arg) {
    if (copts.ignore_oldstyle) {
        return TRUE;
    }

    while (arg) {
        if (arg == &elipsis) {
            return FALSE;
        }
        switch (arg->type->type) {
            case TYPEINT:
                if (TYPE_INTEGRAL(arg->type)->integral < IT_INT) {
                    return FALSE;
                }
                break;
            case TYPEFLOAT:
                if (TYPE_INTEGRAL(arg->type)->integral < IT_DOUBLE) {
                    return FALSE;
                }
                break;
            default:
                break;
        }
        arg = arg->next;
    }

    return TRUE;
}

Boolean is_arglistsame(FuncArg* a, FuncArg* b) {
    if (a == &oldstyle) {
        if (b == &oldstyle) {
            return TRUE;
        } else {
            return oldstylecompatible(b);
        }
    } else {
        if (b == &oldstyle) {
            return oldstylecompatible(a);
        }
    }

    while (1) {
        if (!a || !b || a == &elipsis || b == &elipsis) {
            return a == b;
        }

        if (a->type->type == TYPEPOINTER) {
            if (b->type->type != TYPEPOINTER ||
                (a->qual & (Q_CONST | Q_VOLATILE)) != (b->qual & (Q_CONST | Q_VOLATILE)) ||
                !is_typesame(TYPE_POINTER(a->type)->target, TYPE_POINTER(b->type)->target) ||
                IS_TYPEPOINTER_REFERENCE(a->type) != IS_TYPEPOINTER_REFERENCE(b->type)) {
                return FALSE;
            }
        } else {
            if (!is_typesame(a->type, b->type)) {
                return FALSE;
            }
        }

        a = a->next;
        b = b->next;
    }
}

void CMid_RegisterDummyCtorFunction(void* function, void* data) {
    CMidFunctionEntry* entry = lalloc(sizeof(CMidFunctionEntry));
    entry->next = cmid_function_entries;
    entry->function = function;
    entry->ctorData = data;
    entry->expression = NULL;
    cmid_function_entries = entry;
}

void CMid_RegisterSingleExprFunction(void* function, void* expression) {
    CMidFunctionEntry* entry = lalloc(sizeof(CMidFunctionEntry));
    entry->next = cmid_function_entries;
    entry->function = function;
    entry->ctorData = NULL;
    entry->expression = expression;
    cmid_function_entries = entry;
}

void CMid_RegisterNonGlobalClass(void* value) {
    CMidNonGlobalClassEntry* entry = lalloc(sizeof(CMidNonGlobalClassEntry));
    entry->next = cmid_non_global_classes;
    entry->value = value;
    cmid_non_global_classes = entry;
}
