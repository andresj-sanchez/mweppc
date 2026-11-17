#include "compiler/IroLoop.h"
#include "compiler/IroLinearForm.h"
#include "compiler/IroDump.h"
#include "compiler/IroFlowgraph.h"
#include "compiler/IroCodePositioning.h"

void IRO_DumpLoopFlags(UInt32 flags) {
    if (!flags) {
        IRO_Dump("LP_FLAGS_NONE");
    } else {
        if (flags & 0x1) {
            IRO_Dump("LP_INDUCTION_AT_LEFT ");
        }
        if (flags & 0x2) {
            IRO_Dump("LP_HAS_CALLS ");
        }
        if (flags & 0x4) {
            IRO_Dump("LP_HAS_CNTRLFLOW ");
        }
        if (flags & 0x8) {
            IRO_Dump("LP_HAS_NONASSIGN ");
        }
        if (flags & 0x10) {
            IRO_Dump("LP_INDUCTION_NOT_FOUND ");
        }
        if (flags & 0x20) {
            IRO_Dump("LP_IF_EXPR_NONCANONICAL ");
        }
        if (flags & 0x40) {
            IRO_Dump("LP_HAS_MULTIPLE_INDUCTIONS ");
        }
        if (flags & 0x80) {
            IRO_Dump("LP_STEP_IS_ONE ");
        }
        if (flags & 0x100) {
            IRO_Dump("LP_HEADER_FOLLOWS_UPDATE ");
        }
        if (flags & 0x200) {
            IRO_Dump("LP_STEP_IS_POS ");
        }
        if (flags & 0x400) {
            IRO_Dump("LP_IND_USED_IN_LOOP ");
        }
        if (flags & 0x800) {
            IRO_Dump("LP_HAS_MULTIPLE_EXITS ");
        }
        if (flags & 0x1000) {
            IRO_Dump("LP_STEP_IS_NEG_ONE ");
        }
        if (flags & 0x2000) {
            IRO_Dump("LP_STEP_IS_NEG ");
        }
        if (flags & 0x4000) {
            IRO_Dump("LP_HAS_ASM ");
        }
        if (flags & 0x8000) {
            IRO_Dump("LP_WHILE_LOOP ");
        }
        if (flags & 0x10000) {
            IRO_Dump("LP_RECURSIVE_LOOP ");
        }
        if (flags & 0x20000) {
            IRO_Dump("LP_IS_REDUCTION_CAND ");
        }
        if (flags & 0x40000) {
            IRO_Dump("LP_HAS_NONCANONICAL_STEP ");
        }
        if (flags & 0x80000) {
            IRO_Dump("LP_INNER_LOOP ");
        }
        if (flags & 0x100000) {
            IRO_Dump("LP_TO_BE_VISITED ");
        }
        if (flags & 0x200000) {
            IRO_Dump("LP_FORM_UNRECOGNIZABLE ");
        }
        if (flags & 0xFFC00000) {
            IRO_Dump("%x", flags & 0xFFC00000);
        }
    }
}
