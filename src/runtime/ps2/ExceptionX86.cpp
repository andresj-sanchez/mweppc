#include "runtime/ExceptionX86.h"
#include "runtime/terminate.h"
#include "msl/runinit.h"
#include "msl/setjmp_x86.h"

extern "C" {

static unsigned short ExX86_CurrentAction(const ActionIterator* iter);
static unsigned short ExX86_NextAction(ActionIterator* iter);
extern "C" void ExX86_FindExceptionRecord(char* returnaddr, ExceptionInfo* info);

static void ExX86_UnexpectedHandler(void) {
    longjmp(ExX86_UnexpectedContext, 1);
}

extern "C" int ExX86_FindCatcher(void* bp, char* returnaddr) {
    ExceptionInfo info;
    ActionIterator iter;
    int result;
    exception_handler old_unexpected;
    exception_handler old_terminate;
    old_unexpected = set_unexpected(ExX86_UnexpectedHandler);
    old_terminate = set_terminate(ExX86_UnexpectedHandler);
    if (setjmp(ExX86_UnexpectedContext) != 0) {
        result = 0;
    } else {
        ExX86_FindExceptionRecord(returnaddr, &info);
        if (!info.exception_record) {
            result = 0;
        } else {
            iter.info = info;
            iter.current_bp = (char*)bp;
            iter.unk14 = 0;
            iter.unk18 = 0;
            iter.unk10 = 0;
            unsigned short action = ExX86_CurrentAction(&iter);
            for (;;) {
                switch (action) {
                case 16:
                    result = 1;
                    break;
                case 0:
                case 3:
                case 13:
                case 14:
                case 15:
                case 18:
                default:
                    result = 0;
                    break;
                case 1:
                case 2:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 17:
                case 19:
                case 20:
                case 21:
                    action = ExX86_NextAction(&iter);
                    continue;
                }
                break;
            }
        }
    }
    set_unexpected(old_unexpected);
    set_terminate(old_terminate);
    return result;
}

#pragma dont_inline on
static unsigned short ExX86_NextAction(ActionIterator* iter) {
    unsigned char* action = iter->info.action_pointer;
    if (!action || (*(unsigned short*)action & 0x8000)) {
        return 0;
    }
    unsigned short kind = *(unsigned short*)action;
    switch (kind) {
    case 1: case 4: case 10: iter->info.action_pointer += 10; break;
    case 2: case 7: case 11: case 12: case 16: iter->info.action_pointer += 14; break;
    case 5: case 6: case 8: case 15: iter->info.action_pointer += 18; break;
    case 9: iter->info.action_pointer += 22; break;
    case 17: case 20: iter->info.action_pointer += 6; break;
    case 19: iter->info.action_pointer += 12 + ((unsigned short*)action)[1] * 4; break;
    case 21: iter->info.action_pointer += 18; break;
    default: terminate(); break;
    }
    return ExX86_CurrentAction(iter);
}

static unsigned short ExX86_CurrentAction(const ActionIterator* iter) {
    if (!iter->info.action_pointer) {
        return 0;
    }
    return *(unsigned short*)iter->info.action_pointer & 0xff;
}

#pragma dont_inline reset

extern "C" void ExX86_FindExceptionRecord(char* returnaddr, ExceptionInfo* info) {
    FunctionTableEntry* functab;
    int num;
    int min;
    int max;
    int mid;
    ExceptionTableHeader* table;
    info->exception_record = 0;
    info->action_pointer = 0;
    table = FirstExceptionTable;
    while (table && table->First->Pc > (unsigned char*)returnaddr) {
        table = table->Next;
    }
    if (!table) {
        return;
    }
    functab = table->First;
    num = table->Last - functab;
    min = 0;
    max = num - 1;
    while (min <= max) {
        mid = (max + min) / 2;
        if ((unsigned char*)returnaddr >= functab[mid].Pc &&
            (unsigned char*)returnaddr < functab[mid + 1].Pc) {
            unsigned char* ptr = (unsigned char*)functab[mid].exctable;
            info->exception_record = (char*)ptr;
            if (!(*ptr & 1)) {
                unsigned short count = *(unsigned short*)(ptr + 5);
                unsigned int i;
                ptr += 7;
                for (i = 0; i < count; ++i) {
                    unsigned char* pc = *(unsigned char**)ptr;
                    int mod, rm;
                    if (*pc == 0xe8 || *pc == 0xcc) {
                        pc += 5;
                    } else if (*pc == 0xff) {
                        ++pc;
                        mod = *pc >> 6;
                        rm = *pc & 7;
                        ++pc;
                        if (mod != 3) {
                            int sib = 0;
                            if (rm == 4) {
                                sib = *pc & 7;
                                ++pc;
                            }
                            if ((mod == 0 && rm == 5) || (mod == 0 && sib == 5)) {
                                pc += 4;
                            } else if (mod == 1) {
                                ++pc;
                            } else if (mod == 2) {
                                pc += 4;
                            }
                        }
                    } else {
                        terminate();
                    }
                    if ((char*)pc == returnaddr) {
                        info->action_pointer = *(unsigned char**)(ptr + 4);
                        return;
                    }
                    ptr += 8;
                }
            }
            info->action_pointer = 0;
            return;
        }
        if (functab[mid].Pc < (unsigned char*)returnaddr) {
            min = mid + 1;
        } else {
            max = mid - 1;
        }
    }
}

extern "C" void _RegisterExceptionTables(ExceptionTableHeader* header) {
    ExceptionTableHeader* prev = 0;
    ExceptionTableHeader* next = FirstExceptionTable;
    while (next && next->First->Pc > header->First->Pc) {
        prev = next;
        next = next->Next;
    }
    header->Next = next;
    if (prev) {
        prev->Next = header;
    } else {
        FirstExceptionTable = header;
    }
}

void ExX86_InitializeCPUInstructionSet(void) {
    ExX86_CPUInstructionSet = __detect_cpu_instruction_set();
}


}
