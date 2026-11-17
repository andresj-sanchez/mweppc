#include "compiler/RegListQueries.h"
#include "compiler/common.h"
#include <string.h>
#include "compiler/Alias.h"
#include "compiler/BitVector.h"
#include "compiler/CFunc.h"
#include "compiler/CExpr.h"
#include "compiler/PCode.h"
#include "compiler/CompilerTools.h"
#include "compiler/DIVbyCnst.h"
#include "compiler/BitVectors.h"
#include "compiler/CError.h"
#include "compiler/CParser.h"
#include "compiler/CPrepTokenizer.h"
#include "compiler/MIPSError.h"
#include "compiler/Registers.h"
#include "compiler/UseDefChains.h"
#include "compiler/PCodeInfo.h"

void computeusedefchains(int includeMemory) {
    UseDefInfo* info;
    int rclass;
    SInt32 i;

    if (includeMemory) {
        gather_alias_info();
        precomputememoryusedefs();
    }

    precomputeusedefcounts(includeMemory);
    computeusedeflists(includeMemory);
    init_compressed_bitvectors(number_of_Defs > number_of_Uses ? number_of_Defs : number_of_Uses);

    for (i = 0, info = usedefinfo; i < pcblockcount; i++, info++) {
        info->defgen = bitvectorcreate_range(info->defstart, info->defcount);
        info->defkill = bitvector_empty_compressed();
        info->defin = oalloc(4 * ((number_of_Defs + 31) >> 5));
        info->defout = oalloc(4 * ((number_of_Defs + 31) >> 5));
        info->usegen = bitvectorcreate_range(info->usestart, info->usecount);
        info->usekill = bitvector_empty_compressed();
        info->usein = oalloc(4 * ((number_of_Uses + 31) >> 5));
        info->useout = oalloc(4 * ((number_of_Uses + 31) >> 5));
    }

    computelocalusedefinfo();
    computedepthfirstordering();
    computeglobalusedefinfo();
    computeglobaldefuseinfo();

    for (rclass = 0; rclass < 7; rclass++) {
        last_usedef_register[rclass] = used_virtual_registers[rclass];
    }
}

static void addobjectusedef(Object* object);
static void addaliasusedef(Alias* alias);

static void precomputeusedefcounts(SInt32 includeMemory) {
    PCodeBlock* block;
    PCode* pcode;
    PCodeArg* op;
    SInt32 i;
    ObjectUseDef* oud;

    number_of_Defs = 0;
    number_of_Uses = number_of_Defs;

    usedefinfo = oalloc(pcblockcount * sizeof(UseDefInfo));
    memclrw(usedefinfo, pcblockcount * sizeof(UseDefInfo));

    for (block = pcbasicblocks; block; block = block->nextBlock) {
        UseDefInfo* info = &usedefinfo[block->blockIndex];

        info->usestart = number_of_Uses;
        info->defstart = number_of_Defs;
        for (pcode = block->firstPCode; pcode; pcode = pcode->nextPCode) {
            if (pcode->argCount) {
                pcode->useID = number_of_Uses;
                pcode->defID = number_of_Defs;
                op = pcode->args;
                for (i = pcode->argCount; i--; op++) {
                    if (op->kind == 0 && op->data.reg.reg >= n_real_registers[(SInt8)op->arg]) {
                        if (op->effect & 1) {
                            number_of_Uses++;
                        }
                        if (op->effect & 2) {
                            number_of_Defs++;
                        }
                    }
                }

                if (includeMemory) {
                    Alias* alias = pcode->alias;
                    if (pcode->flags & 0x2) {
                        if (alias->type == 0 || alias->type == 1) {
                            number_of_Uses++;
                        } else {
                            for (oud = objectusedeflist; oud; oud = oud->next) {
                                if (may_alias_object(pcode, oud->object)) {
                                    number_of_Uses++;
                                }
                            }
                        }
                    } else if (pcode->flags & 0x4) {
                        if (alias->type == 0 || alias->type == 1) {
                            number_of_Defs++;
                        } else {
                            for (oud = objectusedeflist; oud; oud = oud->next) {
                                if (may_alias_object(pcode, oud->object)) {
                                    number_of_Defs++;
                                }
                            }
                        }
                    } else if (pcode->flags & 0x8) {
                        for (oud = objectusedeflist; oud; oud = oud->next) {
                            if (may_alias_object(pcode, oud->object)) {
                                number_of_Uses++;
                                number_of_Defs++;
                            }
                        }
                    }
                }
            }
        }
        info->usecount = number_of_Uses - info->usestart;
        info->defcount = number_of_Defs - info->defstart;
    }
}

static int updateusedefset(
    UInt32* out, UInt32* in, CompressedBitVector* kill, BitVectorRange* gen, int bit_count
) {
    int words;
    int runs;
    int genStart;
    int genLength;
    int changed;
    CompressedBitRun* run;
    int runStart;
    int runLength;
    int i;
    UInt32 bits;
    UInt32 old;

    words = (bit_count + 31) >> 5;
    runs = kill->nruns;
    genStart = gen->firstword;
    genLength = gen->nwords;
    changed = FALSE;
    run = NULL;
    if (runs) {
        run = kill->runs;
        runStart = run->firstword;
        runLength = run->nwords;
    }
    for (i = 0; i < words; i++) {
        old = out[i];
        bits = in[i];
        if (run) {
            if (i >= runStart + runLength) {
                if (--runs != 0) {
                    run = (CompressedBitRun*)&run->words[runLength];
                    runStart = run->firstword;
                    runLength = run->nwords;
                } else {
                    run = NULL;
                }
            }
            if (run && i >= runStart) {
                bits &= ~run->words[i - runStart];
            }
        }
        if (i >= genStart && i < genStart + genLength) {
            bits |= gen->words[i - genStart];
        }
        if (bits != old) {
            changed = TRUE;
            out[i] = bits;
        }
    }
    return changed;
}

static ObjectUseDef* findobjectusedef(Object* object) {
    ObjectUseDef* oud;

    oud = *&objectusedefs;
    while (oud) {
        if (object < oud->object) {
            oud = oud->leftchild;
        } else if (object > oud->object) {
            oud = oud->rightchild;
        } else {
            return oud;
        }
    }

    return NULL;
}

static void computelocalusedefinfo(void) {
    PCodeBlock* block;
    PCode* pcode;
    UseOrDef* use;
    UseOrDef* def;
    SInt32 useID;
    SInt32 defID;
    ObjectUseDef* oud;
    RegUseOrDef* list;
    UseDefInfo* info;

    for (block = pcbasicblocks; block; block = block->nextBlock) {
        info = &usedefinfo[block->blockIndex];
        bitvectorinitialize(*(UInt32**)&info->defgen, number_of_Defs, 0);
        bitvectorinitialize(*(UInt32**)&info->defkill, number_of_Defs, 0);
        bitvectorinitialize(*(UInt32**)&info->usegen, number_of_Uses, 0);
        bitvectorinitialize(*(UInt32**)&info->usekill, number_of_Uses, 0);
        bitvectorinitialize(info->defin, number_of_Defs, 0);
        bitvectorinitialize(info->defout, number_of_Defs, 0);
        bitvectorinitialize(info->usein, number_of_Uses, 0);
        bitvectorinitialize(info->useout, number_of_Uses, 0);

        for (pcode = block->firstPCode; pcode; pcode = pcode->nextPCode) {
            if (!(pcode->flags & 0x10) && pcode->argCount) {
                for (use = Uses + (useID = pcode->useID);
                     useID < number_of_Uses && use->pcode == pcode;
                     use++, useID++) {
                    bitvectorsetbit(useID, *(UInt32**)&info->usegen);
                    if (use->v.kind == 0) {
                        for (list = reg_Uses[use->v.arg][use->v.u.reg]; list; list = list->next) {
                            if (bitvectorgetbit(list->id, *(UInt32**)&info->defgen)) {
                                bitvectorclearbit(useID, *(UInt32**)&info->usegen);
                            }
                        }
                    } else {
                        for (list = findobjectusedef(use->v.u.object)->defs; list;
                             list = list->next) {
                            if (uniquely_aliases(pcode, Defs[list->id].pcode)) {
                                if (Defs[list->id].v.arg == 0 &&
                                    bitvectorgetbit(list->id, *(UInt32**)&info->defgen)) {
                                    bitvectorclearbit(useID, *(UInt32**)&info->usegen);
                                }
                            }
                        }
                    }
                }

                for (def = Defs + (defID = pcode->defID);
                     defID < number_of_Defs && def->pcode == pcode;
                     def++, defID++) {
                    if (def->v.kind == 0) {
                        for (list = reg_Defs[def->v.arg][def->v.u.reg]; list; list = list->next) {
                            if (Uses[list->id].pcode->block != (PCodeBlock*)block) {
                                bitvectorsetbit(list->id, *(UInt32**)&info->usekill);
                            }
                        }
                        for (list = reg_Uses[def->v.arg][def->v.u.reg]; list; list = list->next) {
                            if (Defs[list->id].pcode->block != (PCodeBlock*)block) {
                                bitvectorsetbit(list->id, *(UInt32**)&info->defkill);
                            } else {
                                bitvectorclearbit(list->id, *(UInt32**)&info->defgen);
                            }
                        }
                    } else if (def->v.arg == 0) {
                        oud = findobjectusedef(def->v.u.object);
                        for (list = oud->uses; list; list = list->next) {
                            if (may_alias(pcode, Uses[list->id].pcode)) {
                                if (Uses[list->id].pcode->block != (PCodeBlock*)block) {
                                    bitvectorsetbit(list->id, *(UInt32**)&info->usekill);
                                }
                            }
                        }
                        for (list = oud->defs; list; list = list->next) {
                            if (uniquely_aliases(pcode, Defs[list->id].pcode)) {
                                if (Defs[list->id].pcode->block != (PCodeBlock*)block) {
                                    bitvectorsetbit(list->id, *(UInt32**)&info->defkill);
                                } else {
                                    bitvectorclearbit(list->id, *(UInt32**)&info->defgen);
                                }
                            }
                        }
                    }

                    bitvectorsetbit(defID, *(UInt32**)&info->defgen);
                }
            }
        }
    }
}

static void computeglobalusedefinfo(void) {
    PCLink* preds;
    UseDefInfo* udi;
    PCodeBlock* block;
    int i;
    Boolean changed;

    changed = TRUE;
    while (changed) {
        changed = FALSE;
        i = 0;
        while (i < pcblockcount) {
            if ((block = depthfirstordering[i])) {
                udi = &usedefinfo[block->blockIndex];
                if ((preds = block->predecessors)) {
                    bitvectorcopy(
                        udi->defin, usedefinfo[preds->from->blockIndex].defout, number_of_Defs
                    );
                    for (preds = preds->nextPredecessor; preds; preds = preds->nextPredecessor) {
                        bitvectorunion(
                            udi->defin, usedefinfo[preds->from->blockIndex].defout, number_of_Defs
                        );
                    }
                }
                changed |= updateusedefset(
                    udi->defout, udi->defin, udi->defkill, udi->defgen, number_of_Defs
                );
            }
            i++;
        }
    }
}

static void computeglobaldefuseinfo(void) {
    PCLink* succs;
    UseDefInfo* udi;
    PCodeBlock* block;
    int i;
    Boolean changed;

    changed = TRUE;
    while (changed) {
        changed = FALSE;
        i = pcblockcount;
        while (i) {
            if ((block = depthfirstordering[--i])) {
                udi = &usedefinfo[block->blockIndex];
                if ((succs = block->successors)) {
                    bitvectorcopy(
                        udi->useout, usedefinfo[succs->to->blockIndex].usein, number_of_Uses
                    );
                    for (succs = succs->nextSuccessor; succs; succs = succs->nextSuccessor) {
                        bitvectorunion(
                            udi->useout, usedefinfo[succs->to->blockIndex].usein, number_of_Uses
                        );
                    }
                }
                changed |= updateusedefset(
                    udi->usein, udi->useout, udi->usekill, udi->usegen, number_of_Uses
                );
            }
        }
    }
}

static void computeusedeflists(SInt32 includeMemory) {
    ObjectUseDef* oud;
    PCodeBlock* block;
    PCode* pcode;
    PCodeArg* op;
    SInt32 i;
    SInt32 useID;
    SInt32 defID;
    SInt32 rclass;
    SInt8 rclass8;
    UseOrDef* use;
    UseOrDef* def;
    RegUseOrDef* list;
    ObjectUseDef* objUseDef;

    Uses = oalloc(12 * number_of_Uses);
    Defs = oalloc(12 * number_of_Defs);

    for (rclass8 = 0; rclass8 < 7; rclass8++) {
        reg_Uses[rclass8] = oalloc(4 * used_virtual_registers[rclass8]);
        reg_Defs[rclass8] = oalloc(4 * used_virtual_registers[rclass8]);
        for (i = 0; i < used_virtual_registers[rclass8]; i++) {
            reg_Uses[rclass8][i] = NULL;
            reg_Defs[rclass8][i] = NULL;
        }
    }

    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (pcode = block->firstPCode; pcode; pcode = pcode->nextPCode) {
            if (!pcode->argCount) {
                continue;
            }
            use = Uses + pcode->useID;
            def = Defs + pcode->defID;
            op = pcode->args;
            for (i = pcode->argCount; i--; op++) {
                if (op->kind == 0 && op->data.reg.reg >= n_real_registers[(SInt8)op->arg]) {
                    if (op->effect & 1) {
                        use->pcode = pcode;
                        use->v.kind = op->kind;
                        use->v.arg = op->arg;
                        use->v.u.reg = op->data.reg.reg;
                        list = oalloc(8);
                        list->id = pcode->useID;
                        list->next = reg_Defs[(SInt32)(SInt8)op->arg][op->data.reg.reg];
                        reg_Defs[(SInt32)(SInt8)op->arg][op->data.reg.reg] = list;
                        use++;
                        (pcode->useID)++;
                    }
                    if (op->effect & 2) {
                        def->pcode = pcode;
                        def->v.kind = op->kind;
                        def->v.arg = op->arg;
                        def->v.u.reg = op->data.reg.reg;
                        list = oalloc(8);
                        list->id = pcode->defID;
                        list->next = reg_Uses[(SInt32)(SInt8)op->arg][op->data.reg.reg];
                        reg_Uses[(SInt32)(SInt8)op->arg][op->data.reg.reg] = list;
                        def++;
                        (pcode->defID)++;
                    }
                }
            }

            if (!includeMemory) {
                continue;
            }

            if (pcode->flags & 0x2) {
                Alias* alias = pcode->alias;
                if (alias->type == 0 || alias->type == 1) {
                    Object* object = alias->object;
                    objUseDef = NULL;
                    for (oud = *&objectusedefs; oud;) {
                        if (object < oud->object) {
                            oud = oud->leftchild;
                        } else if (object > oud->object) {
                            oud = oud->rightchild;
                        } else {
                            objUseDef = oud;
                            break;
                        }
                    }
                    use->pcode = pcode;
                    use->v.kind = 4;
                    use->v.arg = 0;
                    use->v.u.object = object;
                    list = oalloc(8);
                    list->id = pcode->useID;
                    list->next = objUseDef->uses;
                    objUseDef->uses = list;
                    objUseDef->directuses++;
                    use++;
                    (pcode->useID)++;
                } else {
                    for (oud = objectusedeflist; oud; oud = oud->next) {
                        if (may_alias_object(pcode, oud->object)) {
                            use->pcode = pcode;
                            use->v.kind = 4;
                            use->v.arg = 1;
                            use->v.u.object = oud->object;
                            list = oalloc(8);
                            list->id = pcode->useID;
                            list->next = oud->uses;
                            oud->uses = list;
                            oud->indirectuses++;
                            use++;
                            (pcode->useID)++;
                        }
                    }
                }
            } else if (pcode->flags & 0x4) {
                Alias* alias = pcode->alias;
                if (alias->type == 0 || alias->type == 1) {
                    Object* object = alias->object;
                    objUseDef = NULL;
                    for (oud = *&objectusedefs; oud;) {
                        if (object < oud->object) {
                            oud = oud->leftchild;
                        } else if (object > oud->object) {
                            oud = oud->rightchild;
                        } else {
                            objUseDef = oud;
                            break;
                        }
                    }
                    def->pcode = pcode;
                    def->v.kind = 4;
                    def->v.u.object = object;
                    if (must_alias_object(pcode, object)) {
                        def->v.arg = 0;
                        objUseDef->directdefs++;
                    } else {
                        def->v.arg = 1;
                        objUseDef->indirectdefs++;
                    }
                    list = oalloc(8);
                    list->id = pcode->defID;
                    list->next = objUseDef->defs;
                    objUseDef->defs = list;
                    def++;
                    (pcode->defID)++;
                } else {
                    for (oud = objectusedeflist; oud; oud = oud->next) {
                        if (may_alias_object(pcode, oud->object)) {
                            def->pcode = pcode;
                            def->v.kind = 4;
                            def->v.arg = 1;
                            def->v.u.object = oud->object;
                            list = oalloc(8);
                            list->id = pcode->defID;
                            list->next = oud->defs;
                            oud->defs = list;
                            oud->indirectdefs++;
                            def++;
                            (pcode->defID)++;
                        }
                    }
                }
            } else if (pcode->flags & 0x8) {
                for (oud = objectusedeflist; oud; oud = oud->next) {
                    if (may_alias_object(pcode, oud->object)) {
                        use->pcode = pcode;
                        use->v.kind = 4;
                        use->v.arg = 1;
                        use->v.u.object = oud->object;
                        list = oalloc(8);
                        list->id = pcode->useID;
                        list->next = oud->uses;
                        oud->uses = list;
                        oud->indirectuses++;
                        use++;
                        (pcode->useID)++;
                        def->pcode = pcode;
                        def->v.kind = 4;
                        def->v.arg = 1;
                        def->v.u.object = oud->object;
                        list = oalloc(8);
                        list->id = pcode->defID;
                        list->next = oud->defs;
                        oud->defs = list;
                        oud->indirectdefs++;
                        def++;
                        (pcode->defID)++;
                    }
                }
            }
        }
    }
}

static void precomputememoryusedefs(void) {
    PCodeBlock* block;
    PCode* pcode;

    objectusedefs = NULL;
    objectusedeflist = NULL;

    for (block = pcbasicblocks; block; block = block->nextBlock) {
        for (pcode = block->firstPCode; pcode; pcode = pcode->nextPCode) {
            if (pcode->flags & 0x6) {
                Alias* alias = pcode->alias;
                if (alias) {
                    if (alias->type > 1 || alias->size != nbytes_loaded_or_stored_by(pcode->op)) {
                        pcode->flags |= 0x20;
                    }
                    addaliasusedef(alias);
                } else {
                    pcode->flags |= 0x20;
                }
            }
        }
    }

    addobjectusedef(&worst_case_obj);
}

static void addaliasusedef(Alias* alias) {
    AliasMember* member;
    ObjectUseDef** link;
    ObjectUseDef* oud;

    if (alias && alias != worst_case_ptr && alias != worst_case_func) {
        if (alias->type == 2) {
            for (member = alias->parents; member; member = member->nextParent) {
                Alias* child = member->child;
                if (child && child != worst_case_ptr && child != worst_case_func) {
                    if (child->type == 2) {
                        AliasMember* member2;
                        for (member2 = child->parents; member2; member2 = member2->nextParent) {
                            addaliasusedef(member2->child);
                        }
                    } else {
                        Object* object = child->object;
                        oud = NULL;
                        link = &objectusedefs;
                        while (*link) {
                            oud = *link;
                            if (object < oud->object) {
                                link = &oud->leftchild;
                            } else if (object > oud->object) {
                                link = &oud->rightchild;
                            } else {
                                break;
                            }
                        }
                        if (object != oud->object) {
                            oud = oalloc(sizeof(ObjectUseDef));
                            memclrw(oud, sizeof(ObjectUseDef));
                            oud->object = object;
                            oud->next = objectusedeflist;
                            objectusedeflist = oud;
                            *link = oud;
                        }
                    }
                }
            }
        } else {
            Object* object = alias->object;
            oud = NULL;
            link = &objectusedefs;
            while (*link) {
                oud = *link;
                if (object < oud->object) {
                    link = &oud->leftchild;
                } else if (object > oud->object) {
                    link = &oud->rightchild;
                } else {
                    break;
                }
            }
            if (object != oud->object) {
                oud = oalloc(sizeof(ObjectUseDef));
                memclrw(oud, sizeof(ObjectUseDef));
                oud->object = object;
                oud->next = objectusedeflist;
                objectusedeflist = oud;
                *link = oud;
            }
        }
    }
}

static void addobjectusedef(Object* object) {
    ObjectUseDef* oud;
    ObjectUseDef** link;

    link = &objectusedefs;
    for (oud = *link; oud;) {
        if (object < oud->object) {
            link = &oud->leftchild;
            oud = oud->leftchild;
        } else if (object > oud->object) {
            link = &oud->rightchild;
            oud = oud->rightchild;
        } else {
            return;
        }
    }

    oud = oalloc(sizeof(ObjectUseDef));
    memclrw(oud, sizeof(ObjectUseDef));
    oud->object = object;
    oud->next = objectusedeflist;
    objectusedeflist = oud;
    *link = oud;
}

SInt32 fn_0054B940(
    PCode* def_pcode, SInt32 defID, SInt32 reg, PCode* use_pcode, UInt32* defin, SInt32 rclass
) {
    PCodeBlock* block;
    PCode* scan;
    PCodeArg* operand;
    SInt32 count;
    SInt32 i;

    block = use_pcode->block;
    if (def_pcode->block != block) {
        if (!((defin[defID >> 5] >> defID) & 1)) {
            return FALSE;
        }
        scan = block->firstPCode;
        while (scan && scan != use_pcode) {
            count = scan->argCount;
            operand = scan->args;
            for (i = 0; i < count; i++, operand++) {
                if (operand->kind == PCOp_REGISTER &&
                    operand->arg == (UInt8)rclass &&
                    (operand->effect & 2) &&
                    operand->data.reg.reg == reg) {
                    return FALSE;
                }
            }
            scan = scan->nextPCode;
        }
        return TRUE;
    }
    scan = def_pcode->nextPCode;
    while (scan) {
        if (scan == use_pcode) {
            break;
        }
        scan = scan->nextPCode;
    }
    if (!scan) {
        if (!((defin[defID >> 5] >> defID) & 1)) {
            return FALSE;
        }
    }
    scan = block->firstPCode;
    while (scan && scan != use_pcode) {
        count = scan->argCount;
        operand = scan->args;
        for (i = 0; i < count; i++, operand++) {
            if (operand->kind == PCOp_REGISTER &&
                operand->arg == (UInt8)rclass &&
                (operand->effect & 2) &&
                operand->data.reg.reg == reg) {
                return FALSE;
            }
        }
        scan = scan->nextPCode;
    }
    return TRUE;
}
