#include "compiler/IroVars.h"
#include "compiler/IroLoop.h"
#include "compiler/BitVector.h"
#include "compiler/IroLinearForm.h"
#include "compiler/CParser.h"
#include "compiler/CError.h"
#include "compiler/IroDependsKills.h"
#include "compiler/IroFlowgraph.h"
#include "compiler/IroCSE.h"
#include "compiler/IroPointerAnalysisADTs.h"

void fn_0057F200(Object* obj) {
    UInt32* record = (UInt32*)IRO_FindVar(obj, 1, 1);
    UInt32 bit;

    if (!record) {
        CError_Internal(IRO_DependsKillsSourceFile, 0x304);
    }
    bit = *record;
    if (!bit) {
        CError_Internal(IRO_DependsKillsSourceFile, 0x306);
    }
    if ((bit >> 5) >= IRO_VarKills->size) {
        CError_Internal(IRO_BitVectorFileName, 80);
    }
    IRO_VarKills->data[bit >> 5] |= 1 << (bit & 31);
}

void fn_0057FE60(Object* obj) {
    UInt32* record = (UInt32*)IRO_FindVar(obj, 1, 1);
    UInt32 bit;

    if (!record) {
        CError_Internal(IRO_DependsKillsSourceFile, 0xD7);
    }
    bit = *record;
    if (!bit) {
        CError_Internal(IRO_DependsKillsSourceFile, 0xD9);
    }
    if (is_volatile_object(obj)) {
        IRO_IsVolatile = TRUE;
        IRO_NotSubable = TRUE;
    }
    if ((bit >> 5) >= IRO_Depends->size) {
        CError_Internal(IRO_BitVectorFileName, 80);
    }
    IRO_Depends->data[bit >> 5] |= 1 << (bit & 31);
}

void fn_0057E440(BitVector* result, IROLinear* node) {
    BitVector** table;
    SInt32 size;

    if (node->type == IROLinearOp1Arg && node->nodetype == 4 && (node->flags & 4)) {
        size = node->rtype->size;
        if (size > 1024) {
            size = 1024;
        }
        table = IRO_KillTable_61F30C;
    } else {
        size = 0;
        table = IRO_KillZeroTable_620314;
    }
    if (IRO_KillTableMax_61F308 < size) {
        Bv_Clear(result);
        if (!result->size) {
            CError_Internal(IRO_BitVectorFileName, 80);
        }
        result->data[0] |= 1;
    } else {
        Bv_Copy(table[size], result);
    }
}
