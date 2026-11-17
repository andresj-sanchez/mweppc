#include "compiler/RegisterInfo.h"
#include "compiler/ValueNumbering.h"
#include "compiler/PCodeInfo.h"
#include "compiler/common.h"
#include "compiler/Alias.h"
#include "compiler/AliasPropagation.h"
#include "compiler/PCode.h"
#include "compiler/CError.h"
#include "compiler/CompilerTools.h"
#include "compiler/CFunc.h"
#include "compiler/objects.h"
#include "compiler/CParser.h"
#include "compiler/BitVectors.h"
#include "compiler/CMangler.h"
#include "compiler/Registers.h"
#include "compiler/UseDefChains.h"
#include "compiler/InlineAsmMIPS.h"
#include "compiler/CClass.h"
#include "compiler/types.h"
#include "compiler/CExprConvMatch.h"
#include "compiler/CodeGen.h"
#include "compiler/Exceptions.h"
#undef CERROR_FILE
#define CERROR_FILE "Alias.c"

void update_all_alias_values(void) {
    Alias* alias;

    for (alias = aliases; alias; alias = alias->next) {
        killmemory(alias, NULL);
    }
}

void initialize_alias_values(void) {
    Alias* alias;

    for (alias = aliases; alias; alias = alias->next) {
        alias->valuenumber = nextvaluenumber++;
        alias->valuepcode = NULL;
    }
}

Boolean may_alias_object(PCode* pc, void* object) {
    return fn_00490010((void*)pc->alias, make_alias(object, 0, 0));
}

Boolean instr_may_alias_alias(PCode* pc) {
    return fn_00490010((void*)pc->alias, worst_case_ptr);
}

Boolean must_alias_object(PCode* pc, Object* object) {
    AliasType type = ((Alias*)pc->alias)->type;
    if (type <= AliasType_ObjectPart &&
        ((Alias*)pc->alias)->object == object &&
        ((Alias*)pc->alias)->size == object->type->size &&
        ((Alias*)pc->alias)->size == nbytes_loaded_or_stored_by(pc->op)) {
        return TRUE;
    }
    return FALSE;
}

Boolean instr_must_alias_alias(PCode* pc, Alias* alias) {
    Alias* a = (Alias*)pc->alias;
    if (fn_00490010(a, alias) &&
        a &&
        alias &&
        a == alias &&
        a->type <= 1 &&
        alias->type <= 1 &&
        a->size == alias->size &&
        a->size == nbytes_loaded_or_stored_by(pc->op)) {
        return TRUE;
    }
    return FALSE;
}

Boolean may_alias(PCode* a, PCode* b) {
    return fn_00490010(a->alias, b->alias);
}

Boolean uniquely_aliases(PCode* a, PCode* b) {
    Alias* first = a->alias;
    Alias* second = b->alias;
    if (fn_00490010(first, second) &&
        first &&
        second &&
        first == second &&
        first->type <= 1 &&
        second->type <= 1 &&
        first->size == second->size &&
        first->size == nbytes_loaded_or_stored_by(a->op) &&
        first->size == nbytes_loaded_or_stored_by(b->op)) {
        return TRUE;
    }
    return FALSE;
}

Alias* make_displ_alias(Alias* alias, SInt32 offset, SInt32 size) {
    Alias* result;

    if (size && alias->size != alias->object->type->size) {
        result = make_alias(alias->object, alias->offset + offset, size);
        if (!result) {
            fn_00490570(worst_case_ptr, alias);
            result = worst_case_ptr;
        }
    } else {
        result = make_alias(alias->object, 0, 0);
    }
    CError_ASSERT(786, result);
    return result;
}

Alias* fn_00490600(Alias* a, Alias* b) {
    if (a == b) {
        return a;
    }
    {
        AliasType type1 = a->type;
        if (type1 <= 1) {
            AliasType type2 = b->type;
            if (type2 <= 1 && a->object == b->object) {
                return make_alias(a->object, 0, 0);
            }
        }
    }
    fn_00490570(worst_case_ptr, a);
    fn_00490570(worst_case_ptr, b);
    return worst_case_ptr;
}

void fn_00490570(Alias* parent, Alias* child) {
    AliasMember* member;

    if (parent == child) {
        return;
    }
    if (child->type == 2) {
        for (member = child->parents; member; member = member->nextParent) {
            fn_00490570(parent, member->child);
        }
    } else {
        if (parent->type == 2 && child->type == 1) {
            child = make_alias(child->object, 0, 0);
        }
        for (member = parent->parents; member; member = member->nextParent) {
            if (member->child == child) {
                return;
            }
        }
        member = lalloc(sizeof(AliasMember));
        member->parent = parent;
        member->child = child;
        member->nextParent = parent->parents;
        parent->parents = member;
        member->nextChild = child->children;
        child->children = member;
    }
}

Alias* make_alias(Object* object, SInt32 offset, SInt32 size) {
    Alias* alias;
    Alias* whole;
    UInt32 hash;
    Type* type;
    SInt32 fullsize;

    CError_ASSERT(470, object);
    type = object->type;
    fullsize = type->size;
    if (!size) {
        offset = 0;
        size = fullsize;
    }
    hash = (UInt32)(object->name->hashval * offset * size) % 997;
    for (alias = alias_hash[hash]; alias; alias = alias->hashNext) {
        if (alias->object == object && alias->offset == offset && alias->size == size) {
            break;
        }
    }
    if (!alias) {
        if (offset > 0 || size != fullsize) {
            whole = make_alias(object, 0, fullsize);
            alias = lalloc(sizeof(Alias));
            memclrw(alias, sizeof(Alias));
            alias->type = 1;
            alias->index = n_aliases++;
            alias->next = aliases;
            aliases = alias;
            alias->object = object;
            alias->offset = offset;
            alias->size = size;
            alias->hashNext = alias_hash[hash];
            alias_hash[hash] = alias;
            fn_00490570(whole, alias);
        } else {
            alias = lalloc(sizeof(Alias));
            memclrw(alias, sizeof(Alias));
            alias->type = 0;
            alias->index = n_aliases++;
            alias->next = aliases;
            aliases = alias;
            alias->object = object;
            alias->offset = offset;
            alias->size = size;
            alias->hashNext = alias_hash[hash];
            alias_hash[hash] = alias;
        }
        if (object->datatype != DLOCAL && !fn_00490D20(object)) {
            fn_00490570(worst_case_ptr, make_alias(object, 0, 0));
        }
    }
    type = object->type;
    if ((type->type != TYPEARRAY || type->size > 0) && offset > type->size) {
        return NULL;
    }
    return alias;
}

void initialize_aliases(void) {
    int i;
    struct ExceptionReference* list;
    Alias* alias;

    memclrw(&worst_case_obj, sizeof(Object));
    worst_case_obj.otype = OT_OBJECT;
    worst_case_obj.type = TYPE(&worst_case_memory_type);
    worst_case_obj.datatype = DDATA;
    worst_case_obj.name = GetHashNameNodeExport("@worst_case@");

    aliases = NULL;
    set_aliases = NULL;
    n_aliases = 0;
    n_gathered_aliases = 0;
    for (i = 0; i < 997; i++) {
        alias_hash[i] = NULL;
    }

    alias = lalloc(sizeof(Alias));
    memclrw(alias, sizeof(Alias));
    alias->type = 2;
    alias->index = n_aliases++;
    alias->next = aliases;
    aliases = alias;
    alias->object = NULL;
    alias->offset = 0;
    alias->size = 0;
    worst_case_ptr = alias;
    fn_00490570(worst_case_ptr, make_alias(&worst_case_obj, 0, 0));

    alias = lalloc(sizeof(Alias));
    memclrw(alias, sizeof(Alias));
    alias->type = 2;
    alias->index = n_aliases++;
    alias->next = aliases;
    aliases = alias;
    alias->object = NULL;
    alias->offset = 0;
    alias->size = 0;
    worst_case_func = alias;
    fn_00490570(worst_case_func, make_alias(&worst_case_obj, 0, 0));

    for (list = except_refs; list; list = list->next) {
        void* object = list->obj;
        if (((VarInfo*)Registers_GetVarInfo(object))->noregister) {
            fn_00490570(worst_case_func, make_alias(object, 0, 0));
        }
    }
}

void fn_00490B90(Alias* alias) {
    AliasMember* member;
    Object* object;
    Boolean needs_alias;

    if (alias->type == AliasType_Set) {
        for (member = alias->parents; member; member = member->nextParent) {
            fn_00490B90(member->child);
        }
    } else if (alias->type <= AliasType_ObjectPart) {
        object = alias->object;
        if (object->datatype == DLOCAL) {
            fn_00490570(worst_case_ptr, make_alias(object, 0, 0));
        } else {
            needs_alias = object->type == (Type*)virtual_object_type_5E8098;
            if (!needs_alias) {
                needs_alias = object->datatype == DDATA && *((UInt8*)object + 0x40);
            }
            if (needs_alias && !fn_00490D20(object)) {
                fn_00490570(worst_case_ptr, make_alias(object, 0, 0));
            }
        }
    } else {
        CError_FATAL(353);
    }
}

#pragma dont_inline on
Boolean fn_00490D20(Object* object) {
    Type* type;

    type = object->type;
    while (type->type == TYPEARRAY) {
        type = TYPE_POINTER(type)->target;
    }
    if (type->type == TYPEINT ||
        type->type == TYPEENUM ||
        type->type == TYPEPOINTER ||
        (type->type == TYPEMEMBERPOINTER && type->size == 4) ||
        type->type == TYPEFLOAT ||
        type->type == TYPESTRUCT) {
        return is_const_object(object);
    }
    if (type->type == TYPECLASS) {
        return is_const_object(object) && CClass_IsPODClass(TYPE_CLASS(type));
    }
    return FALSE;
}
#pragma dont_inline reset

void fn_00490DA0(Statement* stmt, Object* value, Object* operand, Boolean isRead, Boolean append) {
    Statement* nextstmt;
    OpcodeInfo* info;
    InlineAsmMIPSRecord* ia;

    if (append) {
        nextstmt = CFunc_InsertStatement(ST_ASM, stmt);
        if (curstmt == stmt && stmt->next) {
            curstmt = stmt->next;
        }
    } else {
        nextstmt = CFunc_InsertBeforeStatement(ST_ASM, stmt);
        if (curstmt == stmt && stmt->next) {
            curstmt = stmt->next;
        }
    }
    nextstmt->expr = NULL;
    info = isRead ? &opcode_info[614] : &opcode_info[612];
    ia = lalloc(sizeof(InlineAsmMIPSRecord) + sizeof(InlineAsmMIPSOperand) * info->operandCount);
    if (isRead) {
        ia->opcode = 614;
    } else {
        ia->opcode = 612;
    }
    ia->flags = 32;
    ia->argcount = 2;
    ia->args[0].kind = 1;
    ia->args[0].value = 0;
    ia->args[0].object = value;
    ia->args[1].kind = 1;
    ia->args[1].value = 0;
    ia->args[1].object = operand;
    ia->args[0].reg.rclass = 0;
    ia->args[1].reg.rclass = 5;
    if (isRead) {
        ia->args[0].reg.effect = 1;
        ia->args[1].reg.effect = 2;
    } else {
        ia->args[0].reg.effect = 2;
        ia->args[1].reg.effect = 1;
    }
    nextstmt->expr = (ENode*)ia;
}

void fn_00490EA0(Statement* stmt, Object* value, Object* operand, Boolean isRead, Boolean append) {
    Statement* nextstmt;
    OpcodeInfo* info;
    InlineAsmMIPSRecord* ia;

    if (append) {
        nextstmt = CFunc_InsertStatement(ST_ASM, stmt);
        if (curstmt == stmt && stmt->next) {
            curstmt = stmt->next;
        }
    } else {
        nextstmt = CFunc_InsertBeforeStatement(ST_ASM, stmt);
        if (curstmt == stmt && stmt->next) {
            curstmt = stmt->next;
        }
    }
    nextstmt->expr = NULL;
    info = isRead ? &opcode_info[173] : &opcode_info[165];
    ia = lalloc(sizeof(InlineAsmMIPSRecord) + sizeof(InlineAsmMIPSOperand) * info->operandCount);
    if (isRead) {
        ia->opcode = 173;
    } else {
        ia->opcode = 165;
    }
    ia->flags = 32;
    ia->argcount = 2;
    ia->args[0].kind = 1;
    ia->args[0].value = 0;
    ia->args[0].object = value;
    ia->args[1].kind = 1;
    ia->args[1].value = 0;
    ia->args[1].object = operand;
    ia->args[0].reg.rclass = 0;
    ia->args[1].reg.rclass = 1;
    if (isRead) {
        ia->args[0].reg.effect = 1;
        ia->args[1].reg.effect = 2;
    } else {
        ia->args[0].reg.effect = 2;
        ia->args[1].reg.effect = 1;
    }
    nextstmt->expr = (ENode*)ia;
}

Alias* fn_00490C60(Object* obj) {
    Boolean is_readonly;
    Type* type;
    Object* object;
    HashNameNode* name;

    is_readonly = FALSE;
    if (obj->type->type != TYPEPOINTER) {
        return NULL;
    }
    type = obj->type;

    if (!(CParser_GetTypeQualifiers(obj->type, obj->qual) & 0x200000)) {
        if (!copts.unknown133) {
            return NULL;
        }
        if (!CParser_IsReadOnly(TYPE_POINTER(type)->target, obj->qual)) {
            return NULL;
        }
        if (type->type == TYPECLASS && !CClass_IsPODClass(TYPE_CLASS(type))) {
            return NULL;
        }
        is_readonly = TRUE;
    }
    name = CParser_GetUniqueName();
    object = lalloc(0x44);
    memclrw(object, 0x44);
    object->otype = 5;
    object->type = (Type*)virtual_object_type_5E8098;
    object->datatype = 0;
    object->name = name;
    if (is_readonly) {
        object->qual = 1;
    }
    return make_alias(object, 0x7FFFFF, 1);
}

static Boolean same_alias_object(Object* a, Object* b) {
    if (a == b) {
        return TRUE;
    }
    if (a->name == b->name &&
        a->nspace == b->nspace &&
        a->datatype == b->datatype &&
        (a->datatype == 3 || a->datatype == 0)) {
        return CMangler_GetLinkName(a) == CMangler_GetLinkName(b);
    }
    return FALSE;
}

Boolean fn_00490010(Alias* a, Alias* b) {
    switch (a->type * 3 + b->type) {
        case 0:
            return a == b;
        case 1:
        case 3:
            return same_alias_object(a->object, b->object);
        case 4:
            return same_alias_object(a->object, b->object) &&
                (Boolean)(a->offset == b->offset ||
                          (a->offset > b->offset && a->offset < b->offset + b->size) ||
                          (b->offset > a->offset && b->offset < a->offset + a->size));
        case 2:
        case 5:
            return ((((UInt32*)b->membervec)[a->index >> 5] >> a->index) & 1) != 0;
        case 6:
        case 7:
            return ((((UInt32*)a->membervec)[b->index >> 5] >> b->index) & 1) != 0;
        case 8:
            return a == b || !bitvectorintersectionisempty(a->membervec, b->membervec, n_aliases);
        default:
            CError_FATAL(1020);
            return TRUE;
    }
}

void update_alias_value(Alias* alias, PCode* pcode) {
    AliasMember* member2;
    AliasMember* member3;
    AliasMember* member;

    switch (alias->type) {
        case 0:
            killmemory(alias, pcode);
            for (member2 = alias->children; member2; member2 = member2->nextChild) {
                CError_ASSERT(1186, member2->parent->type == 2);
                killmemory(member2->parent, NULL);
            }
            for (member2 = alias->parents; member2; member2 = member2->nextParent) {
                CError_ASSERT(1191, member2->child->type == 1);
                killmemory(member2->child, NULL);
                for (member3 = member2->child->children; member3; member3 = member3->nextChild) {
                    if (member3->parent != alias) {
                        CError_ASSERT(1197, member3->parent->type == 2);
                        killmemory(member3->parent, NULL);
                    }
                }
            }
            break;
        case 1:
            killmemory(alias, pcode);
            for (member2 = alias->children; member2; member2 = member2->nextChild) {
                killmemory(member2->parent, NULL);
                if (member2->parent->type == 0) {
                    for (member3 = member2->parent->parents; member3;
                         member3 = member3->nextParent) {
                        if (member3->child != alias &&
                            (alias->offset == member3->child->offset ||
                             (alias->offset > member3->child->offset &&
                              alias->offset < member3->child->offset + member3->child->size) ||
                             (member3->child->offset > alias->offset &&
                              member3->child->offset < alias->offset + alias->size))) {
                            killmemory(member3->child, NULL);
                        }
                    }
                }
            }
            break;
        case 2:
            killmemory(alias, NULL);
            for (member = alias->parents; member; member = member->nextParent) {
                killmemory(member->child, NULL);
                for (member2 = member->child->children; member2; member2 = member2->nextChild) {
                    if (member2->parent != alias) {
                        killmemory(member2->parent, NULL);
                    }
                }
                for (member3 = member->child->parents; member3; member3 = member3->nextParent) {
                    killmemory(member3->child, NULL);
                    for (member2 = member3->child->children; member2;
                         member2 = member2->nextChild) {
                        if (member2->parent != member->child) {
                            killmemory(member2->parent, NULL);
                        }
                    }
                }
            }
            break;
    }
}

void gather_alias_info(void) {
    Alias* alias;
    AliasMember* member;
    AliasMember* member2;
    PCodeBlock* block;
    PCode* pcode;
    RegUseOrDef* list;
    UseOrDef* def;
    SInt32 defID;
    UInt32* vec;

    if (coloring) {
        propagatealiasinfo(codegen_current_function_636498);
        fn_00490570(worst_case_func, worst_case_ptr);
        vec = oalloc(4 * ((number_of_Defs + 31) >> 5));
        for (block = pcbasicblocks; block; block = block->nextBlock) {
            bitvectorcopy(vec, usedefinfo[block->blockIndex].defin, number_of_Defs);
            for (pcode = block->firstPCode; pcode; pcode = pcode->nextPCode) {
                if (pcode->flags & 6) {
                    fn_0054A9B0(pcode, vec);
                } else if ((pcode->flags & 8) && !pcode->alias) {
                    pcode->alias = worst_case_func;
                }
                for (defID = pcode->defID, def = Defs + defID;
                     defID < number_of_Defs && def->pcode == pcode;
                     ++def, ++defID) {
                    if (def->v.kind == 0 && def->v.arg == 0) {
                        for (list = reg_Defs[def->v.arg][def->v.u.reg]; list; list = list->next) {
                            vec[list->id >> 5] &= ~(1U << (list->id & 31));
                        }
                    }
                    vec[defID >> 5] |= 1U << (defID & 31);
                }
            }
        }
        freeoheap();
    } else {
        for (block = pcbasicblocks; block; block = block->nextBlock) {
            for (pcode = block->firstPCode; pcode; pcode = pcode->nextPCode) {
                if (!pcode->alias) {
                    if (pcode->flags & 6) {
                        pcode->alias = worst_case_ptr;
                    } else if (pcode->flags & 8) {
                        pcode->alias = worst_case_func;
                    }
                }
            }
        }
    }

    if (n_gathered_aliases != n_aliases) {
        for (alias = aliases; alias; alias = alias->next) {
            if (alias->type == 2) {
                alias->membervec = lalloc(4 * ((n_aliases + 31) >> 5));
                bitvectorinitialize(alias->membervec, n_aliases, 0);
                for (member = alias->parents; member; member = member->nextParent) {
                    alias->membervec[member->child->index >> 5] |= 1U
                        << (member->child->index & 31);
                    for (member2 = member->child->parents; member2; member2 = member2->nextParent) {
                        alias->membervec[member2->child->index >> 5] |= 1U
                            << (member2->child->index & 31);
                    }
                }
            }
        }
        n_gathered_aliases = n_aliases;
    }
}
