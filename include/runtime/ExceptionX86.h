#ifndef RUNTIME_EXCEPTIONX86_H
#define RUNTIME_EXCEPTIONX86_H

#include "compiler/common.h"

typedef struct FunctionTableEntry FunctionTableEntry;
typedef struct ExceptionInfo ExceptionInfo;
typedef struct ActionIterator ActionIterator;

struct FunctionTableEntry {
    unsigned char* Pc;
    char* exctable;
};

struct ExceptionTableHeader {
    FunctionTableEntry* First;
    FunctionTableEntry* Last;
    ExceptionTableHeader* Next;
};

struct ExceptionInfo {
    char* exception_record;
    char* current_function;
    unsigned char* action_pointer;
};

struct ActionIterator {
    ExceptionInfo info;
    char* current_bp;
    int unk10;
    int unk14;
    int unk18;
};

#ifdef __cplusplus
extern "C" {
#endif

extern ExceptionTableHeader* FirstExceptionTable;
extern int ExX86_UnexpectedContext[10];
extern unsigned long ExX86_CPUInstructionSet;
void ExX86_InitializeCPUInstructionSet(void);
void ExX86_FindExceptionRecord(char* returnaddr, ExceptionInfo* info);
void _RegisterExceptionTables(ExceptionTableHeader* header);
int ExX86_FindCatcher(void* bp, char* return_address);

#ifdef __cplusplus
}
#endif

#endif
