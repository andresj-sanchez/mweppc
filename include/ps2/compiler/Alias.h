#ifndef COMPILER_ALIAS_H
#define COMPILER_ALIAS_H

#include "compiler/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum AliasType {
    AliasType_Object,
    AliasType_ObjectPart,
    AliasType_Set
} AliasType;

struct Alias {
    Alias* next;
    Alias* hashNext;
    AliasMember* children;
    AliasMember* parents;
    Object* object;
    SInt32 offset;
    SInt32 size;
    int valuenumber;
    PCode* valuepcode;
    UInt32* membervec;
    int index;
    AliasType type;
};

struct AliasMember {
    AliasMember* nextParent;
    AliasMember* nextChild;
    Alias* parent;
    Alias* child;
};

extern void initialize_aliases(void);
extern void gather_alias_info(void);
extern void update_all_alias_values(void);
extern void initialize_alias_values(void);
extern Boolean may_alias_object(PCode* pc, void* object);
extern Boolean instr_may_alias_alias(PCode* pc);
extern Boolean must_alias_object(PCode* pc, Object* object);
extern Boolean may_alias(PCode* a, PCode* b);
extern Boolean uniquely_aliases(PCode* a, PCode* b);
extern Alias* make_alias(Object* object, SInt32 offset, SInt32 size);
extern Alias* make_displ_alias(Alias* alias, SInt32 offset, SInt32 size);
extern void fn_00490570(Alias* parent, Alias* child);
extern Alias* fn_00490600(Alias* a, Alias* b);
extern void fn_00490B90(Alias* alias);
extern Alias* fn_00490C60(Object* obj);
extern Boolean fn_00490010(Alias* a, Alias* b);
extern Boolean fn_00490D20(Object* object);
extern void fn_00490DA0(
    Statement* stmt, Object* value, Object* operand, Boolean isRead, Boolean append
);
extern void fn_00490EA0(
    Statement* stmt, Object* value, Object* operand, Boolean isRead, Boolean append
);

extern Alias* worst_case_ptr;
extern Object worst_case_obj;
extern Alias* worst_case_func;

extern Alias* alias_hash[997];
extern Alias* aliases;
extern Alias* set_aliases;
extern SInt32 n_aliases;
extern SInt32 n_gathered_aliases;
extern TypeIntegral worst_case_memory_type;
extern UInt8 virtual_object_type_5E8098[];

extern void update_alias_value(Alias* alias, PCode* pcode);

#ifdef __cplusplus
}
#endif

#endif
