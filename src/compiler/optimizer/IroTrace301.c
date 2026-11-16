// Debug: a comparable listing of the linear IR after each Iro phase, for
// diffing a traced hybrid (MWCCPSP_EXTRA_FLAGS=-DMWCCPSP_TRACE=1) against the
// native build (MWCCPSP_TRACE in the environment). Not compiled into the
// matching build's calls (IRO_TRACE expands to nothing there).
#include "compiler/IroLinearForm.h"
#include "compiler/enode.h"
#include "compiler/objects.h"
#include "compiler/types.h"
#include "compiler/IroVars.h"
#include <stdio.h>
#include <stdlib.h>

// nodes are named by their position in the list (indices repeat and differ
// between runs that allocate a different number of temporaries)
static IROLinear** pos_table;
static int pos_count;

static int idx(IROLinear* l) {
    int i;

    if (!l) {
        return -1;
    }
    for (i = 0; i < pos_count; i++) {
        if (pos_table[i] == l) {
            return i;
        }
    }
    return -2;
}

void IRO_TraceLinear(char* phase) {
    IROLinear* l;
    ENode* e;

    if (!getenv("MWCCPSP_TRACE")) {
        return;
    }
    fprintf(stderr, "== IRO %s\n", phase);
    pos_count = 0;
    for (l = IRO_FirstLinear; l; l = l->next) {
        pos_count++;
    }
    pos_table = (IROLinear**)malloc(sizeof(IROLinear*) * (pos_count + 1));
    pos_count = 0;
    for (l = IRO_FirstLinear; l; l = l->next) {
        pos_table[pos_count++] = l;
    }
    for (l = IRO_FirstLinear; l; l = l->next) {
        fprintf(
            stderr,
            "  %d t%d n%02x f%x s%d",
            idx(l),
            l->type,
            l->nodetype,
            (unsigned)l->flags,
            l->rtype ? (int)l->rtype->size : -1
        );
        switch (l->type) {
            case IROLinearOperand:
                e = l->u.node;
                if (e) {
                    if (e->type == EINTCONST) {
                        fprintf(
                            stderr,
                            " #%ld:%lu",
                            (long)e->data.intval.hi,
                            (unsigned long)e->data.intval.lo
                        );
                    } else if (e->type == EOBJREF && e->data.objref && e->data.objref->name) {
                        fprintf(stderr, " &%s", e->data.objref->name->name);
                    } else {
                        fprintf(stderr, " e%02x", e->type);
                    }
                }
                break;
            case IROLinearOp1Arg:
                fprintf(stderr, " (%d)", idx(l->u.monadic));
                break;
            case IROLinearOp2Arg:
                fprintf(stderr, " (%d,%d)", idx(l->u.diadic.left), idx(l->u.diadic.right));
                break;
            case IROLinearOp3Arg:
                fprintf(
                    stderr, " (%d,%d,%d)", idx(l->u.args3.a), idx(l->u.args3.b), idx(l->u.args3.c)
                );
                break;
            case IROLinearIf:
            case IROLinearIfNot:
                fprintf(stderr, " ?%d", idx(l->u.label.cond));
                break;
            case IROLinearReturn:
                fprintf(stderr, " r%d", idx(l->u.monadic));
                break;
            default:
                break;
        }
        fprintf(stderr, "\n");
    }
    free(pos_table);
    pos_table = NULL;
    pos_count = 0;
}
