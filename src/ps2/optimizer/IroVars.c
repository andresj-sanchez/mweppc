#include "compiler/CError.h"
#include "compiler/common.h"
#include "compiler/IroUtil.h"
#include "compiler/objects.h"
#include "compiler/types.h"
#include "compiler/IroVars.h"

Boolean IRO_IsRegable(Object* obj) {
    if (obj->datatype == DLOCAL && obj->u.varptr) {
        return !((VarInfo*)obj->u.varptr)->noregister;
    }
    return FALSE;
}

void IRO_ZapVarPtrs(void) {
    struct VarRecord* var;

    for (var = IRO_FirstVar; var; var = var->next) {
        ;
    }
}
