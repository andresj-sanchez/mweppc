#ifndef COMPILER_EXCEPTIONS_H
#define COMPILER_EXCEPTIONS_H

#include "compiler/common.h"

typedef enum ExceptionActionType {
    EAT_NOP,
    EAT_DESTROYLOCAL,
    EAT_DESTROYLOCALCOND,
    EAT_DESTROYLOCALOFFSET,
    EAT_DESTROYLOCALPOINTER,
    EAT_DESTROYLOCALARRAY,
    EAT_DESTROYPARTIALARRAY,
    EAT_DESTROYMEMBER,
    EAT_DESTROYMEMBERCOND,
    EAT_DESTROYMEMBERARRAY,
    EAT_DELETEPOINTER,
    EAT_DELETELOCALPOINTER,
    EAT_DELETEPOINTERCOND,
    EAT_CATCHBLOCK,
    EAT_ACTIVECATCHBLOCK,
    EAT_SPECIFICATION,
    EAT_TERMINATE,
    EAT_DESTROYBASE,
    EAT_DELETEVLA,
    EAT_DESTROYLOCALOBJECT,
    EAT_NACTIONS
} ExceptionActionType;

struct ExceptionAction {
    struct ExceptionAction* prev;
    union {
        struct {
            Object* local;
            Object* dtor;
        } destroy_local;
        struct {
            Object* local;
            Object* cond;
            Object* dtor;
        } destroy_local_cond;
        struct {
            Object* local;
            Object* dtor;
            SInt32 offset;
        } destroy_local_offset;
        struct {
            Object* pointer;
            Object* dtor;
        } destroy_local_pointer;
        struct {
            Object* localarray;
            Object* dtor;
            SInt32 elements;
            SInt32 element_size;
        } destroy_local_array;
        struct {
            Object* arraypointer;
            Object* arraycounter;
            Object* dtor;
            Object* element_size;
        } destroy_partial_array;
        struct {
            Object* objectptr;
            Object* dtor;
            SInt32 offset;
        } destroy_base;
        struct {
            Object* objectptr;
            Object* dtor;
            SInt32 offset;
        } destroy_member;
        struct {
            Object* objectptr;
            Object* cond;
            Object* dtor;
            SInt32 offset;
        } destroy_member_cond;
        struct {
            Object* objectptr;
            Object* dtor;
            SInt32 offset;
            SInt32 elements;
            SInt32 element_size;
        } destroy_member_array;
        struct {
            Object* pointerobject;
            Object* deletefunc;
        } delete_pointer;
        struct {
            Object* pointerobject;
            Object* deletefunc;
            Object* cond;
        } delete_pointer_cond;
        struct {
            Object* catch_object;
            Object* catch_info_object;
            struct CLabel* catch_label;
            Object* catch_typeid;
            Type* catch_type;
            UInt32 catch_qual;
        } catch_block;
        struct {
            Object* catch_info_object;
            Boolean call_dtor;
        } active_catch_block;
        struct {
            SInt32 unexp_ids;
            Object** unexp_id;
            struct CLabel* unexp_label;
            Object* unexp_info_object;
        } specification;
        struct {
            Object* vlaobject;
            Object* deletefunc;
        } delete_vla;
        struct {
            Object* local;
            Boolean call_dtor;
        } destroy_local_object;
    } data;
    ExceptionActionType type;
};

struct ExceptionReference {
    struct ExceptionReference* next;
    void* obj;
};

struct ExceptionActionNode {
    struct ExceptionActionNode* next;
    struct ExceptionActionNode* prev;
    PCode* first;
    PCode* last;
    ExceptionAction* actions;
    struct ExceptionActionDAGNode* graph;
};

struct ExceptionActionDAGNode {
    struct ExceptionActionDAGNode* next;
    struct ExceptionActionDAGNode* prev;
    ExceptionAction* action;
    UInt16 count;
    UInt16 offset;
};

struct ExceptionOutputRef {
    struct ExceptionOutputRef* next;
    void* target;
    SInt32 offset;
    SInt32 addend;
};

extern void fn_004A9A30(void);

extern struct ExceptionReference* except_refs;
extern struct ExceptionActionNode* exception_action_head;
extern struct ExceptionActionNode* exception_action_tail;
extern struct ExceptionActionDAGNode* exception_action_dag[20];
extern GList exception_action_output;
extern struct ExceptionOutputRef* exception_output_ref_last;
extern struct ExceptionOutputRef* exception_output_ref_first;

extern void fn_004A90D0(Object* object, void* data);
extern SInt32 fn_004A9330(void);
extern void recordexceptionactions(PCode* instr, ExceptionAction* actions);
extern void noteexceptionactionregisters(ExceptionAction* actions, PCodeArg* ops);
extern void fn_004A97D0(ExceptionAction* action);
extern void fn_004A9870(void* obj);
extern SInt32 fn_004A98E0(ExceptionAction* actions);
extern void allocateactioninfo(struct ExceptionActionDAGNode* node);
extern void fn_004AA820(UInt32 value);
extern struct ExceptionActionDAGNode* fn_004AA850(ExceptionAction* action);
extern SInt32 fn_004AA910(UInt32 value, UInt8* dest);
extern SInt32 fn_004AA9C0(SInt32 value, UInt8* dest);

#endif
