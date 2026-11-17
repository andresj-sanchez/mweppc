#include "compiler/cos.h"
#include "compiler/CPrec.h"
void fn_00506470(void) {}
void fn_00506460(void) {}

#include "compiler/CompilerTools.h"

void CleanupPrecompiler(void) {
    if (cprec_refnum_61BB64) {
        COS_FileClose(cprec_refnum_61BB64);
        cprec_refnum_61BB64 = 0;
    }
    if (cprec_glist_61BB54.data) {
        FreeGList(&cprec_glist_61BB54);
    }
}

void SetupPrecompiler(Boolean isPrecompiling) {
    cprec_refnum_61BB64 = 0;
    cprec_glist_61BB54.data = 0;
    cprec_header_61BB50 = 0;
    cprec_staticdata_61BB20 = 0;
}
