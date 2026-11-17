#define CERROR_FILE "Exceptions.c"
#include "compiler/common.h"
#include "compiler/CompilerTools.h"
#include "compiler/PCode.h"
#include "compiler/Exceptions.h"
#include "compiler/CMangler.h"
#include "compiler/objects.h"
#include "compiler/RegisterInfo.h"
#include "compiler/CFunc.h"
#include "compiler/CException.h"
#include "compiler/Registers.h"
#include "compiler/DIVbyCnst.h"
#include "compiler/Operands.h"

void fn_004A9870(void* obj) {
    struct ExceptionReference* ref;
    struct ExceptionReference* node;

    if (!obj) {
        return;
    }
    if (((Object*)obj)->otype != OT_OBJECT) {
        return;
    }
    if (((Object*)obj)->datatype != DLOCAL) {
        return;
    }
    for (ref = except_refs; ref; ref = ref->next) {
        if (ref->obj == obj) {
            return;
        }
    }
    node = lalloc(sizeof(struct ExceptionReference));
    memclrw(node, sizeof(struct ExceptionReference));
    node->obj = obj;
    node->next = except_refs;
    except_refs = node;
}

void recordexceptionactions(PCode* instr, ExceptionAction* actions) {
    struct ExceptionActionNode* node;

    if (!actions && (!exception_action_tail || !exception_action_tail->actions)) {
        return;
    }
    node = lalloc(sizeof(struct ExceptionActionNode));
    node->next = NULL;
    node->last = instr;
    node->first = node->last;
    node->actions = actions;
    node->prev = exception_action_tail;
    if (exception_action_tail) {
        exception_action_tail->next = node;
    } else {
        exception_action_head = node;
    }
    exception_action_tail = node;
    new_basic_block();
    while (actions) {
        if (actions->type == 13) {
            pcaddedge(instr->block, actions->data.catch_block.catch_label->pclabel, 2);
        } else if (actions->type == 15) {
            pcaddedge(instr->block, actions->data.specification.unexp_label->pclabel, 2);
        }
        actions = actions->prev;
    }
}

void fn_004A97D0(ExceptionAction* action) {
    while (action) {
        switch (action->type) {
            case 1:
                fn_004A9870(action->data.destroy_local.local);
                break;
            case 2:
                fn_004A9870(action->data.destroy_local_cond.local);
                fn_004A9870(action->data.destroy_local_cond.cond);
                break;
            case 3:
                fn_004A9870(action->data.destroy_local_offset.local);
                break;
            case 4:
                fn_004A9870(action->data.destroy_local_pointer.pointer);
                break;
            case 5:
                fn_004A9870(action->data.destroy_local_array.localarray);
                break;
            case 17:
                fn_004A9870(action->data.destroy_base.objectptr);
                break;
            case 6:
                fn_004A9870(action->data.destroy_partial_array.arraypointer);
                fn_004A9870(action->data.destroy_partial_array.arraycounter);
                fn_004A9870(action->data.destroy_partial_array.element_size);
                break;
            case 7:
                fn_004A9870(action->data.destroy_member.objectptr);
                break;
            case 8:
                fn_004A9870(action->data.destroy_member_cond.objectptr);
                fn_004A9870(action->data.destroy_member_cond.cond);
                break;
            case 9:
                fn_004A9870(action->data.destroy_member_array.objectptr);
                break;
            case 10:
            case 11:
                fn_004A9870(action->data.delete_pointer.pointerobject);
                break;
            case 12:
                fn_004A9870(action->data.delete_pointer_cond.pointerobject);
                fn_004A9870(action->data.delete_pointer_cond.cond);
                break;
            case 13:
                fn_004A9870(action->data.catch_block.catch_object);
                fn_004A9870(action->data.catch_block.catch_info_object);
                break;
            case 14:
                fn_004A9870(action->data.active_catch_block.catch_info_object);
                break;
            case 15:
                break;
            case 16:
                break;
            default:
                break;
        }
        action = action->prev;
    }
}

SInt32 fn_004A9330(void) {
    struct ExceptionActionNode* node;
    struct ExceptionActionNode* prev;
    SInt32 count;
    SInt32 index;
    SInt32 i;

    if (!exception_action_head) {
        return 0;
    }
    for (node = exception_action_head; node; node = node->next) {
        if (node->first->block->flags & 0x20) {
            if (node->prev) {
                node->prev->next = node->next;
            } else {
                exception_action_head = node->next;
            }
            if (node->next) {
                node->next->prev = node->prev;
            }
        }
    }
    if (!exception_action_head) {
        return 0;
    }
    i = 0;
    do {
        index = i;
        i += 5;
        exception_action_dag[index] = NULL;
        exception_action_dag[index + 1] = NULL;
        exception_action_dag[index + 2] = NULL;
        exception_action_dag[index + 3] = NULL;
        exception_action_dag[index + 4] = NULL;
    } while (i < 20);
    for (node = exception_action_head; node; node = node->next) {
        node->graph = node->actions ? fn_004AA850(node->actions) : NULL;
    }
    prev = exception_action_head;
    for (node = exception_action_head->next; node; node = node->next) {
        if (node->graph == prev->graph) {
            prev->last = node->last;
            if (node->prev) {
                node->prev->next = node->next;
            } else {
                exception_action_head = node->next;
            }
            if (node->next) {
                node->next->prev = node->prev;
            }
        } else {
            prev = node;
        }
    }
    count = 0;
    for (node = exception_action_head; node; node = node->next) {
        if (!node->actions) {
            if (node->prev) {
                node->prev->next = node->next;
            } else {
                exception_action_head = node->next;
            }
            if (node->next) {
                node->next->prev = node->prev;
            }
        } else {
            count++;
        }
    }
    return count;
}

void noteexceptionactionregisters(ExceptionAction* actions, PCodeArg* ops) {
    Object* obj;
    int reg;

    while (actions) {
        switch (actions->type) {
            case EAT_DESTROYLOCALCOND:
                reg = OBJECT_REG(obj = actions->data.destroy_local_cond.cond);
                if (reg) {
                    ops->kind = PCOp_REGISTER;
                    ops->arg = obj->u.var.info->rclass;
                    ops->data.reg.reg = reg;
                    ops->effect = 9;
                    ops++;
                }
                break;
            case EAT_DESTROYLOCALPOINTER:
                reg = OBJECT_REG(obj = actions->data.destroy_local_pointer.pointer);
                if (reg) {
                    ops->kind = PCOp_REGISTER;
                    ops->arg = obj->u.var.info->rclass;
                    ops->data.reg.reg = reg;
                    ops->effect = 9;
                    ops++;
                }
                break;
            case EAT_DESTROYMEMBER:
                reg = OBJECT_REG(obj = actions->data.destroy_member.objectptr);
                if (reg) {
                    ops->kind = PCOp_REGISTER;
                    ops->arg = obj->u.var.info->rclass;
                    ops->data.reg.reg = reg;
                    ops->effect = 9;
                    ops++;
                }
                break;
            case EAT_DESTROYBASE:
                reg = OBJECT_REG(obj = actions->data.destroy_base.objectptr);
                if (reg) {
                    ops->kind = PCOp_REGISTER;
                    ops->arg = obj->u.var.info->rclass;
                    ops->data.reg.reg = reg;
                    ops->effect = 9;
                    ops++;
                }
                break;
            case EAT_DESTROYMEMBERCOND:
                reg = OBJECT_REG(obj = actions->data.destroy_member_cond.cond);
                if (reg) {
                    ops->kind = PCOp_REGISTER;
                    ops->arg = obj->u.var.info->rclass;
                    ops->data.reg.reg = reg;
                    ops->effect = 9;
                    ops++;
                }
                reg = OBJECT_REG(obj = actions->data.destroy_member_cond.objectptr);
                if (reg) {
                    ops->kind = PCOp_REGISTER;
                    ops->arg = obj->u.var.info->rclass;
                    ops->data.reg.reg = reg;
                    ops->effect = 9;
                    ops++;
                }
                break;
            case EAT_DESTROYMEMBERARRAY:
                reg = OBJECT_REG(obj = actions->data.destroy_member_array.objectptr);
                if (reg) {
                    ops->kind = PCOp_REGISTER;
                    ops->arg = obj->u.var.info->rclass;
                    ops->data.reg.reg = reg;
                    ops->effect = 9;
                    ops++;
                }
                break;
            case EAT_DELETEPOINTER:
            case EAT_DELETELOCALPOINTER:
                reg = OBJECT_REG(obj = actions->data.delete_pointer.pointerobject);
                if (reg) {
                    ops->kind = PCOp_REGISTER;
                    ops->arg = obj->u.var.info->rclass;
                    ops->data.reg.reg = reg;
                    ops->effect = 9;
                    ops++;
                }
                break;
            case EAT_DELETEPOINTERCOND:
                reg = OBJECT_REG(obj = actions->data.delete_pointer_cond.cond);
                if (reg) {
                    ops->kind = PCOp_REGISTER;
                    ops->arg = obj->u.var.info->rclass;
                    ops->data.reg.reg = reg;
                    ops->effect = 9;
                    ops++;
                }
                reg = OBJECT_REG(obj = actions->data.delete_pointer_cond.pointerobject);
                if (reg) {
                    ops->kind = PCOp_REGISTER;
                    ops->arg = obj->u.var.info->rclass;
                    ops->data.reg.reg = reg;
                    ops->effect = 9;
                    ops++;
                }
                break;
            default:
                break;
        }
        actions = actions->prev;
    }
}

SInt32 fn_004AA910(UInt32 value, UInt8* dest) {
    if (value < 0x80) {
        dest[0] = value << 1;
        return 1;
    }
    if (value < 0x4000) {
        dest[0] = (value >> 8) << 2 | 1;
        dest[1] = value;
        return 2;
    }
    if (value < 0x200000) {
        dest[0] = (value >> 16) << 3 | 3;
        dest[1] = value >> 8;
        dest[2] = value;
        return 3;
    }
    if (value < 0x20000000) {
        dest[0] = (value >> 24) << 3 | 7;
        dest[1] = value >> 16;
        dest[2] = value >> 8;
        dest[3] = value;
        return 4;
    }
    CError_FATAL(0xA6);
    return 0;
}

SInt32 fn_004AA9C0(SInt32 value, UInt8* dest) {
    if (value >= -0x40 && value < 0x40) {
        dest[0] = value << 1;
        return 1;
    }
    if (value >= -0x2000 && value < 0x2000) {
        dest[0] = (value >> 8) << 2 | 1;
        dest[1] = value;
        return 2;
    }
    if (value >= -0x100000 && value < 0x100000) {
        dest[0] = (value >> 16) << 3 | 3;
        dest[1] = value >> 8;
        dest[2] = value;
        return 3;
    }
    if (value >= -0x10000000 && value < 0x10000000) {
        dest[0] = (value >> 24) << 3 | 7;
        dest[1] = value >> 16;
        dest[2] = value >> 8;
        dest[3] = value;
        return 4;
    }
    CError_FATAL(0x6D);
    return 0;
}

void fn_004AA820(UInt32 value) {
    UInt8 bytes[4];
    SInt32 size;

    size = fn_004AA910(value, bytes);
    AppendGListData(&exception_action_output, bytes, size);
}

void fn_004A9A30(void) {
    exception_action_head = exception_action_tail = NULL;
}

struct ExceptionActionDAGNode* fn_004AA850(ExceptionAction* action) {
    struct ExceptionActionDAGNode* prev;
    struct ExceptionActionDAGNode* node;
    struct ExceptionActionDAGNode* parent;

    for (node = exception_action_dag[action->type]; node; node = node->next) {
        if (node->action == action) {
            return node;
        }
    }

    if (action->prev) {
        prev = fn_004AA850(action->prev);
    } else {
        prev = NULL;
    }

    parent = prev;
    for (node = exception_action_dag[action->type]; node; node = node->next) {
        if (node->prev == parent && CExcept_ActionCompare(node->action, action)) {
            return node;
        }
    }

    node = lalloc(sizeof(struct ExceptionActionDAGNode));
    node->prev = prev;
    node->action = action;
    node->count = 0;
    node->next = exception_action_dag[action->type];
    exception_action_dag[action->type] = node;
    if (prev) {
        prev->count++;
    }
    return node;
}

SInt32 fn_004A98E0(ExceptionAction* actions) {
    SInt32 count = 0;
    SInt32 reg;

#define ACTION_REG(OBJECT) \
    (((Object*)(OBJECT))->datatype == DLOCAL ? ((Object*)(OBJECT))->u.var.info->reg : 0)

    while (actions) {
        switch (actions->type) {
            case EAT_DESTROYLOCALCOND:
                reg = ACTION_REG(actions->data.destroy_local_cond.cond);
                if (reg) {
                    count++;
                }
                break;
            case EAT_DESTROYLOCALPOINTER:
                reg = ACTION_REG(actions->data.destroy_local_pointer.pointer);
                if (reg) {
                    count++;
                }
                break;
            case EAT_DESTROYMEMBER:
                reg = ACTION_REG(actions->data.destroy_member.objectptr);
                if (reg) {
                    count++;
                }
                break;
            case EAT_DESTROYBASE:
                reg = ACTION_REG(actions->data.destroy_base.objectptr);
                if (reg) {
                    count++;
                }
                break;
            case EAT_DESTROYMEMBERCOND:
                reg = ACTION_REG(actions->data.destroy_member_cond.cond);
                if (reg) {
                    count++;
                }
                reg = ACTION_REG(actions->data.destroy_member_cond.objectptr);
                if (reg) {
                    count++;
                }
                break;
            case EAT_DESTROYMEMBERARRAY:
                reg = ACTION_REG(actions->data.destroy_member_array.objectptr);
                if (reg) {
                    count++;
                }
                break;
            case EAT_DELETEPOINTER:
            case EAT_DELETELOCALPOINTER:
                reg = ACTION_REG(actions->data.delete_pointer.pointerobject);
                if (reg) {
                    count++;
                }
                break;
            case EAT_DELETEPOINTERCOND:
                reg = ACTION_REG(actions->data.delete_pointer_cond.cond);
                if (reg) {
                    count++;
                }
                reg = ACTION_REG(actions->data.delete_pointer_cond.pointerobject);
                if (reg) {
                    count++;
                }
                break;
            default:
                break;
        }
        actions = actions->prev;
    }

#undef ACTION_REG
    return count;
}

void fn_004A90D0(Object* object, void* data) {
    SInt32 count;
    struct ExceptionActionNode* node;
    struct ExceptionActionDAGNode* graph;
    volatile UInt32 before;
    UInt32 start;
    UInt32 end;
    UInt32 range;
    UInt32 pos;
    UInt8* buf;
    UInt8* cursor;
    SInt32 length;
    int shortInstr;

    count = fn_004A9330();
    InitGList(&exception_action_output, 0x100);
    exception_output_ref_last = NULL;
    length = count * 12;
    exception_output_ref_first = *(struct ExceptionOutputRef* volatile*)&exception_output_ref_last;
    do {
        fn_00550790(&exception_action_output);
        before = exception_action_output.size;
        AppendGListNoData(&exception_action_output, length);
        AppendGListByte(&exception_action_output, 0);
        for (node = exception_action_head; node; node = node->next) {
            for (graph = node->graph; graph; graph = graph->prev) {
                graph->offset = 0;
            }
        }
        for (node = exception_action_head; node; node = node->next) {
            graph = node->graph;
            if (graph->count == 0 && graph->offset == 0) {
                allocateactioninfo(graph);
            }
        }
        buf = (UInt8*)*exception_action_output.data;
        cursor = buf + before;
        pos = 0;
        for (node = exception_action_head; node; node = node->next) {
            shortInstr = node->first->op >= 0x37E && node->first->op <= 0x3E5;
            if (shortInstr) {
                start = node->first->unk1C + 6;
            } else {
                start = node->first->unk1C + 8;
            }
            shortInstr = node->last->op >= 0x37E && node->last->op <= 0x3E5;
            if (shortInstr) {
                end = node->last->unk1C + 6;
            } else {
                end = node->last->unk1C + 8;
            }
            range = end - start;
            if (start < pos) {
                CError_FATAL(0x43E);
            }
            cursor += fn_004AA910(start - pos, cursor);
            cursor += fn_004AA910(range, cursor);
            cursor += fn_004AA910(((struct ExceptionActionDAGNode*)node->graph)->offset, cursor);
            pos = start + range;
        }
        count = cursor - ((UInt8*)*exception_action_output.data + before);
        if (count == length) {
            LockGList(&exception_action_output);
            fn_0046D9E0(
                object,
                data,
                *exception_action_output.data,
                exception_action_output.size,
                exception_output_ref_first
            );
            FreeGList(&exception_action_output);
            return;
        }
        exception_action_output.size = 0;
        exception_output_ref_last = NULL;
        exception_output_ref_first =
            *(struct ExceptionOutputRef* volatile*)&exception_output_ref_last;
        length = count;
    } while (TRUE);
}

void allocateactioninfo(struct ExceptionActionDAGNode* node) {
    ExceptionAction* action;
    struct ExceptionOutputRef* ref;
    SInt32 value;
    SInt32 value2;
    SInt32 offset;
    SInt32 index;
    int lastFlag;

#define PUT_SIGNED(V) \
    do { \
        SInt32 encodedValue = (V); \
        UInt8 encoded[4]; \
        SInt32 encodedSize = fn_004AA9C0(encodedValue, encoded); \
        AppendGListData(&exception_action_output, encoded, encodedSize); \
    } while (0)
#define PUT_UNSIGNED(V) \
    do { \
        UInt32 encodedValue = (V); \
        UInt8 encoded[4]; \
        SInt32 encodedSize = fn_004AA910(encodedValue, encoded); \
        AppendGListData(&exception_action_output, encoded, encodedSize); \
    } while (0)
#define ADD_REF(TARGET) \
    do { \
        void* object = (TARGET); \
        SInt32 position = exception_action_output.size; \
        AppendGListLong(&exception_action_output, 0); \
        if (object) { \
            ref = lalloc(sizeof(struct ExceptionOutputRef)); \
            ref->next = NULL; \
            ref->target = object; \
            ref->offset = position; \
            ref->addend = 0; \
            if (exception_output_ref_first) { \
                exception_output_ref_last->next = ref; \
            } else { \
                exception_output_ref_first = ref; \
            } \
            exception_output_ref_last = ref; \
        } \
    } while (0)

    while (node && (!node->offset || !node->prev)) {
        if (!node->offset) {
            node->offset = exception_action_output.size;
        }
        lastFlag = node->prev ? 0 : 0x80;
        action = node->action;
        switch (action->type) {
            case 1:
                AppendGListByte(&exception_action_output, lastFlag | 2);
                PUT_SIGNED(fn_004AFC70(action->data.destroy_local.local));
                ADD_REF(action->data.destroy_local.dtor);
                break;
            case 2:
                value = OBJECT_REG(action->data.destroy_local_cond.cond);
                if (value) {
                    lastFlag |= 0x40;
                }
                AppendGListByte(&exception_action_output, lastFlag | 3);
                PUT_SIGNED(value ? value : fn_004AFC70(action->data.destroy_local_cond.cond));
                PUT_SIGNED(fn_004AFC70(action->data.destroy_local_cond.local));
                ADD_REF(action->data.destroy_local_cond.dtor);
                break;
            case 3:
                AppendGListByte(&exception_action_output, lastFlag | 2);
                PUT_SIGNED(
                    fn_004AFC70(action->data.destroy_local_offset.local) +
                    action->data.destroy_local_offset.offset
                );
                ADD_REF(action->data.destroy_local_offset.dtor);
                break;
            case 4:
                value = OBJECT_REG(action->data.destroy_local_pointer.pointer);
                if (value) {
                    lastFlag |= 0x20;
                }
                AppendGListByte(&exception_action_output, lastFlag | 4);
                PUT_SIGNED(value ? value : fn_004AFC70(action->data.destroy_local_pointer.pointer));
                ADD_REF(action->data.destroy_local_pointer.dtor);
                break;
            case 5:
                AppendGListByte(&exception_action_output, lastFlag | 5);
                PUT_SIGNED(fn_004AFC70(action->data.destroy_local_array.localarray));
                PUT_UNSIGNED(action->data.destroy_local_array.elements);
                PUT_UNSIGNED(action->data.destroy_local_array.element_size);
                ADD_REF(action->data.destroy_local_array.dtor);
                break;
            case 17:
                value = OBJECT_REG(action->data.destroy_base.objectptr);
                if (value) {
                    lastFlag |= 0x20;
                }
                AppendGListByte(&exception_action_output, lastFlag | 6);
                PUT_SIGNED(value ? value : fn_004AFC70(action->data.destroy_base.objectptr));
                PUT_SIGNED(action->data.destroy_base.offset);
                ADD_REF(action->data.destroy_base.dtor);
                break;
            case 7:
                value = OBJECT_REG(action->data.destroy_member.objectptr);
                if (value) {
                    lastFlag |= 0x20;
                }
                AppendGListByte(&exception_action_output, lastFlag | 7);
                PUT_SIGNED(value ? value : fn_004AFC70(action->data.destroy_member.objectptr));
                PUT_SIGNED(action->data.destroy_member.offset);
                ADD_REF(action->data.destroy_member.dtor);
                break;
            case 8:
                value = OBJECT_REG(action->data.destroy_member_cond.cond);
                if (value) {
                    lastFlag |= 0x40;
                }
                value2 = OBJECT_REG(action->data.destroy_member_cond.objectptr);
                if (value2) {
                    lastFlag |= 0x20;
                }
                AppendGListByte(&exception_action_output, lastFlag | 8);
                PUT_SIGNED(value ? value : fn_004AFC70(action->data.destroy_member_cond.cond));
                PUT_SIGNED(
                    value2 ? value2 : fn_004AFC70(action->data.destroy_member_cond.objectptr)
                );
                PUT_SIGNED(action->data.destroy_member_cond.offset);
                ADD_REF(action->data.destroy_member_cond.dtor);
                break;
            case 9:
                value = OBJECT_REG(action->data.destroy_member_array.objectptr);
                if (value) {
                    lastFlag |= 0x20;
                }
                AppendGListByte(&exception_action_output, lastFlag | 9);
                PUT_SIGNED(
                    value ? value : fn_004AFC70(action->data.destroy_member_array.objectptr)
                );
                PUT_SIGNED(action->data.destroy_member_array.offset);
                PUT_UNSIGNED(action->data.destroy_member_array.elements);
                PUT_UNSIGNED(action->data.destroy_member_array.element_size);
                ADD_REF(action->data.destroy_member_array.dtor);
                break;
            case 10:
            case 11:
                value = OBJECT_REG(action->data.delete_pointer.pointerobject);
                if (value) {
                    lastFlag |= 0x20;
                }
                AppendGListByte(&exception_action_output, lastFlag | 10);
                PUT_SIGNED(value ? value : fn_004AFC70(action->data.delete_pointer.pointerobject));
                ADD_REF(action->data.delete_pointer.deletefunc);
                break;
            case 12:
                value = OBJECT_REG(action->data.delete_pointer_cond.cond);
                if (value) {
                    lastFlag |= 0x40;
                }
                value2 = OBJECT_REG(action->data.delete_pointer_cond.pointerobject);
                if (value2) {
                    lastFlag |= 0x20;
                }
                AppendGListByte(&exception_action_output, lastFlag | 11);
                PUT_SIGNED(value ? value : fn_004AFC70(action->data.delete_pointer_cond.cond));
                PUT_SIGNED(
                    value2 ? value2 : fn_004AFC70(action->data.delete_pointer_cond.pointerobject)
                );
                ADD_REF(action->data.delete_pointer_cond.deletefunc);
                break;
            case 13:
                AppendGListByte(&exception_action_output, lastFlag | 12);
                ADD_REF(action->data.catch_block.catch_typeid);
                PUT_UNSIGNED(action->data.catch_block.catch_label->pclabel->block->unk24);
                PUT_SIGNED(fn_004AFC70(action->data.catch_block.catch_info_object));
                break;
            case 14:
                AppendGListByte(&exception_action_output, lastFlag | 13);
                PUT_SIGNED(fn_004AFC70(action->data.active_catch_block.catch_info_object));
                break;
            case 16:
                AppendGListByte(&exception_action_output, lastFlag | 14);
                break;
            case 15:
                AppendGListByte(&exception_action_output, lastFlag | 15);
                PUT_UNSIGNED(action->data.specification.unexp_ids);
                PUT_UNSIGNED(action->data.specification.unexp_label->pclabel->block->unk24);
                PUT_SIGNED(fn_004AFC70(action->data.specification.unexp_info_object));
                for (index = 0; index < action->data.specification.unexp_ids; index++) {
                    ADD_REF(action->data.specification.unexp_id[index]);
                }
                break;
            default:
                CError_FATAL(0x23B);
                break;
        }
        node = node->prev;
    }
    if (node) {
        offset = (UInt16)node->offset - exception_action_output.size;
        AppendGListByte(&exception_action_output, 1);
        PUT_SIGNED(offset);
    }
#undef PUT_SIGNED
#undef PUT_UNSIGNED
#undef ADD_REF
}
