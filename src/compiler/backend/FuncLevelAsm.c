#include "compiler/CInline.h"
#include "compiler/CPrep.h"
#include "compiler/CodeGen.h"
#include "compiler/InlineAsmMIPS.h"
#include "compiler/common.h"
#include "compiler/CFunc.h"
#include "compiler/CError.h"
#include "compiler/CompilerTools.h"
#include "compiler/PCode.h"
#include "compiler/objects.h"
#include "compiler/CParser.h"
#include "compiler/CMangler.h"
#include "compiler/Operands.h"
#include "compiler/Registers.h"
#include "compiler/FuncLevelAsm.h"

char FuncLevelAsm_CFile_5E9C99[] = "FuncLevelAsm.c";

void fn_00506480(Object* func) {
    Statement* stmt;
    PCodeLabel* label;
    PCodeBlock* block;

    asm_source_offset_6361A0 = functionbodyoffset;
    except_refs = NULL;
    init_registers();
    fn_00434FF0(0);
    cg_codelist_reg_kind_637332 = 0x4088;
    asm_pending_branch_63603C = NULL;
    stmt = curstmt;
    cprep_eoltokens = TRUE;
    cprep_in_assembler = TRUE;
    InlineAsm_ScanStatements(125);
    cprep_in_assembler = FALSE;
    cprep_eoltokens = FALSE;
    CheckCLabels();
    if (cg_codegen_error_flag_637364) {
        return;
    }
    if (copts.unknown134) {
        cg_codegen_active_line_636A04 = CPrep_GetFileOffsetInfo(&cparser_fileoffset);
        fn_00443380(&cparser_fileoffset);
    }
    codegen_register_option_637385 = TRUE;
    CMangler_GetLinkName(func);
    func->flags |= 4;
    codegen_flag_63738A = FALSE;
    fn_004AFDE0();
    cg_debug_register_mask_636208 = 1;
    if (cg_codegen_error_flag_637364) {
        return;
    }
    initpcode();
    pclabelblock(block = makepcblock(), makepclabel());
    pcbasicblocks = block;
    fn_00436FA0(func, 0);
    assign_labels(stmt->next);
    for (stmt = stmt->next; stmt; stmt = stmt->next) {
        switch (stmt->type) {
            case ST_ASM:
                fn_00437300(stmt->sourceoffset, 0);
                InlineAsm_TranslateIRtoPCode(stmt);
                break;
            case ST_LABEL:
                label = stmt->label->pclabel;
                if (!label->resolved) {
                    emit_label(label);
                    pclastblock->flags = cg_codelist_reg_kind_637332;
                    cg_codegen_emit_flag_63735A = 1;
                }
                break;
            default:
                CError_Internal(FuncLevelAsm_CFile_5E9C99, 169);
        }
    }
    if (cg_codegen_error_flag_637364) {
        return;
    }
    codegen_current_function_636498 = func;
    fn_00435D60(func, 0, 1, 1);
}
