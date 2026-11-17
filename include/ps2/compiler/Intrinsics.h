#ifndef COMPILER_INTRINSICS_H
#define COMPILER_INTRINSICS_H

#include "compiler/common.h"
#include "compiler/InstrSelection.h"

struct IntrinsicOpRecord {
    Operand operand;
    ENode* node;
    SInt32 reg;
    UInt8 category;
};

extern ObjRegInfo* fn_004662F0(Operand* op, SInt8 klass, SInt32 reg);

extern Operand intrinsic_sequence_operands[5];
extern SInt32 intrinsic_sequence_count;
extern Boolean intrinsic_sequence_pending;

extern void Intrinsics_GenerateIntrinsicCall(ENode* node, short reg, Operand* result);
extern int is_intrinsic_function_call(ENode* node);

extern void fn_00467F50(void);
extern void fn_00467F90(void);

extern void fn_00463AF0(ENodeList* args, SInt16 opcode, Operand* result);

extern void fn_00467D30(void);
extern void Intrinsics_SetupRuntimeObjects(void);

extern Object* intrinsics_601CD8[];
extern void* cg_runtime_intrinsic_63648C;
extern void fn_00466380(ENode* node, SInt16 number);
extern UInt8 fn_00459690(Boolean exporting);

typedef struct IntrinsicExportEntry {
    const char* name;
    SInt32 unk4;
    Type* type;
    Object* object;
} IntrinsicExportEntry;

extern IntrinsicExportEntry intrinsic_export_entries[16];

#endif
