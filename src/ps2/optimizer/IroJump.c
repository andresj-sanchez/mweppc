#include "compiler/IroJump.h"
#include "compiler/IroDump.h"
#include "compiler/IroFlowgraph.h"
#include "compiler/IroLinearForm.h"
#include "compiler/IroUtil.h"
#include "compiler/CFunc.h"
#include "compiler/Exceptions.h"
#include "compiler/Switch.h"

static Boolean CheckChain(CLabel** labelptr) {
    IRONode* node;

    for (node = IRO_FirstNode; node; node = node->nextnode) {
        if (node->first &&
            node->first->type == IROLinearLabel &&
            node->first->u.label == *labelptr) {
            IROLinear* linear;
            CLabel* lab;
            for (linear = node->first->next, lab = NULL;
                 linear && (linear->type == IROLinearLabel || linear->type == IROLinearNop);
                 linear = linear->next) {
                if (linear->type == IROLinearLabel) {
                    lab = linear->u.label;
                }
            }

            if (linear->type == IROLinearGoto && *labelptr != linear->u.label) {
                *labelptr = linear->u.label;
                IRO_Dump("Chaining goto at %d\n", linear->index);
                return TRUE;
            }

            if (lab && *labelptr != lab) {
                *labelptr = lab;
            }
            return FALSE;
        }
    }

    return FALSE;
}

Boolean IRO_DoJumpChaining(void) {
    IRONode* node;
    IROLinear* linear;
    Boolean changed;
    SwitchInfo* info;
    SwitchCase* curcase;
    Boolean result;

    result = FALSE;
    do {
        changed = FALSE;
        for (node = IRO_FirstNode; node; node = node->nextnode) {
            if (node->first) {
                linear = node->last;
                switch (linear->type) {
                    case IROLinearGoto:
                        if (CheckChain(&linear->u.label)) {
                            changed = TRUE;
                        }
                        break;
                    case IROLinearIf:
                    case IROLinearIfNot:
                        if (CheckChain(&linear->u.label)) {
                            changed = TRUE;
                        }
                        break;
                    case IROLinearSwitch:
                        info = linear->u.swtch.info;
                        for (curcase = info->cases; curcase; curcase = curcase->next) {
                            if (CheckChain(&curcase->label)) {
                                changed = TRUE;
                            }
                        }
                        if (CheckChain(&info->defaultlabel)) {
                            changed = TRUE;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        result |= changed;
        IRO_CheckForUserBreak();
    } while (changed);

    return result;
}

Boolean IRO_RemoveLabels(void) {
    Boolean result;
    IRONode* node;

    result = FALSE;
    IRO_ComputeSuccPred();
    for (node = IRO_FirstNode; node; node = node->nextnode) {
        if (node->first && node->first->type == IROLinearLabel && !node->marked) {
            node->first->type = IROLinearNop;
            node->first->flags &= ~IROLF_StartsNode;
            result = TRUE;
        }
    }

    IRO_CheckForUserBreak();
    return result;
}
