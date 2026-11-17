#include "compiler/IroFlowgraph.h"
#include "compiler/IroCSE.h"
#include "compiler/IroLinearForm.h"
#include "compiler/IroPropagate.h"
#include "compiler/IROUseDef.h"
#include "compiler/IroUtil.h"
#include "compiler/BitVector.h"
#include "compiler/CError.h"
#include "compiler/CFunc.h"
#include "compiler/CompilerTools.h"
#include "compiler/enode.h"
#include "compiler/Exceptions.h"
#include "compiler/InlineAsm.h"
#include "compiler/Switch.h"
#include <string.h>

#pragma pack(push, 2)

UInt16 IRO_NumNodes;
IRONode* IRO_FirstNode;
IRONode* IRO_LastNode;
IRONode* IRO_EndNode;
IRONode** IRO_NodeTable;
BitVector* IRO_VarKills;
BitVector* IRO_Avail;
BitVector* IRO_FuncKills;
BitVector* IRO_ExprKills;

static void AddSucc(IRONode* a, IRONode* b) {
    a->succ[a->numsucc++] = b->index;
    b->numpred++;
}

static void AddLabelSucc(IRONode* node, CLabel* label) {
    IRONode* targetnode = (IRONode*)label->stmt;
    if (targetnode) {
        AddSucc(node, targetnode);
        targetnode->smartMustReach = TRUE;
    } else {
        CError_FATAL(126);
    }
}

static void AddSwitchSucc(IRONode* node) {
    SwitchInfo* info = (SwitchInfo*)node->last->u.node;
    SwitchCase* curcase = info->cases;
    SInt32 i = 1;

    while (curcase) {
        i++;
        curcase = curcase->next;
    }

    node->succ = IRO_oalloc(sizeof(UInt16) * i);
    for (curcase = info->cases; curcase; curcase = curcase->next) {
        AddLabelSucc(node, curcase->label);
    }
    AddLabelSucc(node, info->defaultlabel);
}

static void AddPred(UInt32 a, UInt32 b) {
    IRONode* node = IRO_NodeTable[a];
    node->pred[node->numpred++] = b;
}

void IRO_ComputeSuccPred(void) {
    CLabel* label;
    IRONode* node;
    SInt32 count;
    ExceptionAction* action;
    IAEffects effects;
    UInt16 i;
    IROLinear* linear;

    for (label = Labels; label; label = label->next) {
        label->stmt = NULL;
    }

    for (node = IRO_FirstNode; node; node = node->nextnode) {
        node->smartMustReach = FALSE;
        node->numsucc = 0;
        node->numpred = 0;
        node->reachable = FALSE;
        node->visited = FALSE;
        if (node->first && node->first->type == IROLinearLabel) {
            node->first->u.label->stmt = (Statement*)node;
        }
    }

    for (node = IRO_FirstNode; node; node = node->nextnode) {
        if (!node->first) {
            if (node->nextnode) {
                node->succ = IRO_oalloc(sizeof(UInt16));
                AddSucc(node, node->nextnode);
            }
        } else {
            linear = node->last;
next_linear:
            switch (linear->type) {
                case IROLinearReturn:
                case IROLinearEnd:
                    break;
                case IROLinearGoto:
                    node->succ = IRO_oalloc(sizeof(UInt16));
                    AddLabelSucc(node, linear->u.label);
                    break;
                case IROLinearIf:
                case IROLinearIfNot:
                    node->succ = IRO_oalloc(sizeof(UInt16) * 2);
                    AddSucc(node, node->nextnode);
                    AddLabelSucc(node, linear->u.label);
                    break;
                case IROLinearSwitch:
                    AddSwitchSucc(node);
                    break;
                case IROLinearFunccall:
                    count = 1;
                    if (linear->stmt && IRO_FunctionCallMightThrowException(linear)) {
                        for (action = linear->stmt->dobjstack; action; action = action->prev) {
                            if (action->type == EAT_CATCHBLOCK ||
                                action->type == EAT_SPECIFICATION) {
                                count++;
                            }
                        }
                    }
                    node->succ = IRO_oalloc(sizeof(UInt16) * count);
                    AddSucc(node, node->nextnode);
                    if (linear->stmt && IRO_FunctionCallMightThrowException(linear)) {
                        for (action = linear->stmt->dobjstack; action; action = action->prev) {
                            if (action->type == EAT_CATCHBLOCK) {
                                AddLabelSucc(node, action->data.catch_block.catch_label);
                            } else if (action->type == EAT_SPECIFICATION) {
                                AddLabelSucc(node, action->data.specification.unexp_label);
                            }
                        }
                    }
                    break;
                case IROLinearAsm:
                    if (IRO_GetAsmEffects && linear->u.asm_stmt) {
                        effects = IRO_EmptyAsmEffects;
                        IRO_GetAsmEffects(linear->u.asm_stmt, &effects);
                        node->succ = IRO_oalloc(
                            sizeof(UInt16) * (!effects.noFallthrough + effects.numlabels)
                        );
                        if (!effects.noFallthrough) {
                            AddSucc(node, node->nextnode);
                        }
                        for (i = 0; i < effects.numlabels; i++) {
                            AddLabelSucc(node, effects.labels[i]);
                        }
                    }
                    break;
                case IROLinearOp2Arg:
                    if (linear->nodetype == ECOMMA) {
                        linear = linear->u.diadic.right;
                        goto next_linear;
                    }
                    // fall through
                default:
                    if (node->nextnode) {
                        node->succ = IRO_oalloc(sizeof(UInt16));
                        AddSucc(node, node->nextnode);
                    }
            }
        }
    }

    for (node = IRO_FirstNode; node; node = node->nextnode) {
        if (node->numpred) {
            node->pred = IRO_oalloc(sizeof(UInt16) * node->numpred);
        } else {
            node->pred = NULL;
        }
        node->numpred = 0;
    }

    for (node = IRO_FirstNode; node; node = node->nextnode) {
        for (i = 0; i < node->numsucc; i++) {
            AddPred(node->succ[i], node->index);
        }
    }

    for (node = IRO_FirstNode; node; node = node->nextnode) {
        if (node->first && node->first->type == IROLinearLabel) {
            IROLinear* linear = node->first;
            do {
                if (linear->type == IROLinearBeginCatch ||
                    linear->type == IROLinearEndCatch ||
                    linear->type == IROLinearEndCatchDtor) {
                    node->smartMustReach = TRUE;
                    break;
                }
            } while (linear != node->last && (linear = linear->next));
        }
    }
}

IRONode* IRO_NewFlowGraphNode(void) {
    IRONode* node = IRO_oalloc(sizeof(IRONode));
    memset(node, 0, sizeof(IRONode));
    node->index = IRO_NumNodes;
    IRO_NumNodes++;
    node->nextnode = NULL;
    return node;
}

IRONode* IRO_MergeFlowGraphNodes(IRONode* a, IRONode* b) {
    IRONode* succ;
    Boolean found;
    UInt32 i;
    UInt32 j;
    UInt32 k;

    if (a->nextnode == b && a->last && b->first && a->last->next == b->first) {
        if (b->first->type == IROLinearLabel) {
            IRO_NopOut(b->first);
        }

        a->nextnode = b->nextnode;
        a->last = b->last;

        for (i = 0; i < a->numsucc; i++) {
            if (b->index == a->succ[i]) {
                for (j = i; j < a->numsucc; j++) {
                    if ((j + 1) < a->numsucc) {
                        a->succ[j] = a->succ[j + 1];
                    } else {
                        a->succ[j] = 0;
                    }
                }
                a->numsucc--;
                break;
            }
        }

        for (i = 0; i < b->numsucc; i++) {
            succ = IRO_NodeTable[b->succ[i]];
            for (j = 0; j < a->numsucc; j++) {
                if (b->succ[i] == a->succ[j]) {
                    break;
                }
            }

            if (j == a->numsucc) {
                AddSucc(a, succ);
                succ->numpred--;
            }

            found = FALSE;
            for (j = 0; j < succ->numpred; j++) {
                if (a->index == succ->pred[j]) {
                    found = TRUE;
                    break;
                }
            }

            for (j = 0; j < succ->numpred; j++) {
                if (b->index == succ->pred[j]) {
                    if (!found) {
                        succ->pred[j] = a->index;
                    } else {
                        for (k = j; k < succ->numpred; k++) {
                            if ((k + 1) < succ->numpred) {
                                succ->pred[k] = succ->pred[k + 1];
                            } else {
                                succ->pred[k] = 0;
                            }
                        }
                        succ->numpred--;
                    }
                    break;
                }
            }
        }

        b->numsucc = b->numpred = 0;
        b->first = b->last = NULL;
        b->nextnode = NULL;
        b->reachable = FALSE;
        b->visited = FALSE;
        b->mustreach = FALSE;
        b->smartMustReach = FALSE;
        b->loopdepth = 0;

        if (IRO_LastNode == b) {
            IRO_LastNode = a;
        }

        if (IRO_FirstExpr && IRO_LastExpr) {
            IROExpr* expr;
            for (expr = IRO_FirstExpr; expr && expr != IRO_LastExpr->next; expr = expr->next) {
                if (expr->node == b) {
                    expr->node = a;
                }
            }
        }

        if (IRO_FirstAssign && IRO_LastAssign) {
            IROAssign* assign;
            for (assign = IRO_FirstAssign; assign && assign != IRO_LastAssign->next;
                 assign = assign->next) {
                if (assign->node == b) {
                    assign->node = a;
                }
            }
        }

        if (IRO_FirstVarUse && IRO_LastVarUse) {
            IROUse* use;
            for (use = IRO_FirstVarUse; use && use != IRO_LastVarUse->globalnext;
                 use = use->globalnext) {
                if (use->node == b) {
                    use->node = a;
                }
            }
        }

        IRO_NodeTable[b->index] = NULL;
        return a;
    }

    return NULL;
}
