#include "compiler/objects.h"
#include "compiler/CMachine.h"
#include "compiler/common.h"
#include "compiler/CPrep.h"
#include "compiler/CInt64.h"
#include "compiler/CError.h"
#include "compiler/IroLoop.h"
#include "compiler/IroFlowgraph.h"
#include <string.h>
#include "compiler/CParser.h"
#include "compiler/BitVector.h"
#include "compiler/CMangler.h"
#include "compiler/CompilerTools.h"
#include "compiler/IroMalloc.h"
#include "compiler/IroType.h"
#include "compiler/IroUtil.h"
#include "compiler/CPrepTokenizer.h"
#include <setjmp.h>
#include "compiler/IroPointerAnalysisADTs.h"

#define IRO_DEBUG_CLEAR(obj, size) memset((obj), 0xFF, (size))

UInt32 gcd(UInt32 a, UInt32 b) {
    if (!a) {
        return b;
    }
    if (!b) {
        return a;
    }
    while (1) {
        UInt32 chk = a % b;
        if (!chk) {
            return b;
        }
        a = b;
        b = chk;
    }
}

Boolean CInt64_IsNegative(CInt64* value) {
    return (value->hi & 0x80000000) != 0;
}

#pragma dont_inline on
LocationSet* LocationSetSet_FindFirst(LocationSetSet* lss) {
    return lss->loc;
}
#pragma dont_inline reset

void LocationSetSet_Init(LocationSetSet* lss) {
    IRO_ASSERT(2382, lss != NULL);
    lss->loc = NULL;
    lss->otherLocs = NULL;
    lss->count = 0;
}

LocationSetSet* LocationSetSet_New(void) {
    LocationSetSet* lss = IRO_malloc(sizeof(LocationSetSet));
    IRO_ASSERT(2356, lss != NULL);
    lss->loc = NULL;
    lss->otherLocs = NULL;
    lss->count = 0;
    return lss;
}

Boolean PAHeapBlocks_Equal(PAHeapBlock* a, PAHeapBlock* b) {
    return a == b || a->nd == b->nd;
}

#pragma dont_inline on
Boolean LocationSet_IsUnknown(LocationSet* ls) {
    return ls == stUnknownLs || ls->block == stUnknownMb;
}
#pragma dont_inline reset

void LocationSet_SetRtype(LocationSet* ls, Type* type) {
    ls->rtype = type;
}

#pragma dont_inline on
PAMemoryBlock* LocationSet_restriction(LocationSet* ls) {
    return ls->u.unknown.restriction;
}
#pragma dont_inline reset

#pragma dont_inline on
LocationSet* LocationSet_bitfieldOf(LocationSet* ls) {
    return ls->u.unknown.bitfieldOf;
}
#pragma dont_inline reset

#pragma dont_inline on
void LocationSetSet_Term(LocationSetSet* lss) {
    while (lss && lss->loc) {
        LocationSetSet_Remove(lss, lss->loc);
    }
    lss->loc = NULL;
    lss->otherLocs = NULL;
    lss->count = 0;
}

#pragma dont_inline reset

#pragma dont_inline on
void PointsToFunction_Delete(PointsToFunction* ptf) {
    IRO_DEBUG_CLEAR(ptf, sizeof(PointsToFunction));
    IRO_gfree(ptf);
}

void PointsToEntry_Delete(PointsToEntry* pte) {
    IRO_DEBUG_CLEAR(pte, sizeof(PointsToEntry));
    IRO_gfree(pte);
}

#pragma dont_inline reset

void PointsToFunction_Init(PointsToFunction* ptf) {
    IRO_ASSERT(3454, ptf != NULL);
    ptf->pte = NULL;
    ptf->otherPtes = NULL;
}

PointsToFunction* PointsToFunction_New(void) {
    PointsToFunction* ptf = IRO_malloc(sizeof(PointsToFunction));
    IRO_ASSERT(3430, ptf != NULL);
    ptf->pte = NULL;
    ptf->otherPtes = NULL;
    return ptf;
}

int LocationSetSet_Count(LocationSetSet* lss) {
    return lss->count;
}

PointsToEntry* PointsToEntry_New(void) {
    PointsToEntry* pte = IRO_malloc(sizeof(PointsToEntry));
    pte->loc = NULL;
    pte->locs = NULL;
    return pte;
}

UInt32 LocationSet_stride(LocationSet* ls) {
    return ls->u.known.stride;
}

void ParamMappingFunction_Init(ParamMappingFunction* pmf) {
    pmf->mapping = NULL;
    pmf->otherMappings = NULL;
}

ParamMappingFunction* ParamMappingFunction_New(void) {
    ParamMappingFunction* pmf = IRO_malloc(sizeof(ParamMappingFunction));
    pmf->mapping = NULL;
    pmf->otherMappings = NULL;
    return pmf;
}

void ParamMapping_Copy(ParamMapping* dest, ParamMapping* src) {
    dest->actual = src->actual;
    dest->formal = src->formal;
    dest->extended = src->extended;
}

ParamMapping* ParamMapping_New(void) {
    ParamMapping* pm = IRO_malloc(sizeof(ParamMapping));
    pm->actual = NULL;
    pm->formal = NULL;
    pm->extended = NULL;
    return pm;
}

void ObjectSet_Init(ObjectSet* os) {
    os->obj = NULL;
    os->otherObjs = NULL;
}

ObjectSet* ObjectSet_New(void) {
    ObjectSet* os = IRO_malloc(sizeof(ObjectSet));
    os->obj = NULL;
    os->otherObjs = NULL;
    return os;
}

#pragma dont_inline on
void ObjectSet_Delete(ObjectSet* os) {
    IRO_DEBUG_CLEAR(os, sizeof(ObjectSet));
    IRO_gfree(os);
}

#pragma dont_inline reset
PAMemoryBlock* PAMemoryBlock_New(void) {
    PAMemoryBlock* mb = IRO_malloc(sizeof(PAMemoryBlock));
    mb->kind = 0;
    return mb;
}

#pragma dont_inline on
void PTFList_Delete(PTFList* list) {
    IRO_DEBUG_CLEAR(list, sizeof(PTFList));
    IRO_gfree(list);
}

void ExtendedParamSet_Delete(ExtendedParamSet* set) {
    IRO_DEBUG_CLEAR(set, sizeof(ExtendedParamSet));
    IRO_gfree(set);
}

#pragma dont_inline reset

void ParamMapping_Term(ParamMapping* pm) {
    IRO_DEBUG_CLEAR(pm, sizeof(ParamMapping));
    IRO_gfree(pm);
}

void ParamMapping_Delete(ParamMapping* pm) {
    pm->actual = NULL;
    pm->formal = NULL;
    pm->extended = NULL;
}

#pragma dont_inline on

void ObjectSet_Term(ObjectSet* set) {
    ObjectSet* cur;
    ObjectSet* prev;
    ObjectSet* second;
    Object* target;

    while (set && set->obj) {
        target = set->obj;
        prev = NULL;
        cur = set;
        while (cur && cur->obj) {
            if (fn_004DB110(cur->obj, target)) {
                break;
            }
            prev = cur;
            cur = cur->otherObjs;
        }
        if (prev) {
            prev->otherObjs = cur->otherObjs;
            cur->obj = NULL;
            cur->otherObjs = NULL;
            ObjectSet_Term(cur);
            ObjectSet_Delete(cur);
        } else if (cur->otherObjs) {
            second = cur->otherObjs;
            cur->obj = second->obj;
            cur->otherObjs = second->otherObjs;
            second->obj = NULL;
            second->otherObjs = NULL;
            ObjectSet_Term(second);
            ObjectSet_Delete(second);
        } else {
            cur->obj = NULL;
        }
    }

    set->obj = NULL;
    set->otherObjs = NULL;
}

#pragma dont_inline reset
void PAMemoryBlock_Init(PAMemoryBlock* mb, UInt8 kind, void* thing) {
    mb->kind = kind;
    switch (mb->kind) {
        case 2:
            mb->u.ep = thing;
            break;
        case 3:
            mb->u.localvar = thing;
            break;
        case 4:
            mb->u.heapblock = thing;
            break;
        case 5:
            mb->u.intval = *(CInt64*)thing;
            break;
        case 6:
        case 7:
            mb->u.other = thing;
            break;
        default:
            CError_FATAL(1529);
            break;
    }
}

void LocationSet_InitKnown(
    LocationSet* ls, PAMemoryBlock* block, CInt64 field, UInt32 stride, Type* type
) {
    ls->block = block;
    ls->rtype = type;
    ls->u.known.field = field;
    ls->u.known.stride = stride;
}

void LocationSetSet_Copy(LocationSetSet* dest, LocationSetSet* src) {
    LocationSetSet* cursor = src;
    dest->loc = NULL;
    dest->otherLocs = NULL;
    dest->count = 0;
    if (dest->count) {
        while (cursor && cursor->loc) {
            LocationSetSet_Add(dest, cursor->loc);
            cursor = cursor->otherLocs;
        }
    } else {
        while (cursor && cursor->loc) {
            LocationSetSet_SimpleAddImpl(dest, cursor->loc);
            cursor = cursor->otherLocs;
        }
    }
}

LocationSet* LocationSet_New(void) {
    LocationSet* ls = IRO_malloc(sizeof(LocationSet));
    ls->block = NULL;
    ls->rtype = NULL;
    ls->u.known.field = cint64_zero;
    ls->u.known.stride = 0;
    return ls;
}

#pragma dont_inline on
void LocationSet_Delete(LocationSet* ls) {
    memset(ls, 0xFF, sizeof(LocationSet));
    IRO_gfree(ls);
}

void LocationSetSet_Delete(LocationSetSet* lss) {
    IRO_DEBUG_CLEAR(lss, sizeof(LocationSetSet));
    IRO_gfree(lss);
}

#pragma dont_inline reset

#pragma dont_inline on
void PointsToFunction_Term(PointsToFunction* ptf) {
    while (ptf && ptf->pte) {
        PointsToFunction_RemoveByLocationSet(ptf, ptf->pte->loc);
    }
    ptf->pte = NULL;
    ptf->otherPtes = NULL;
}

#pragma dont_inline reset

void PointsToFunction_Copy(PointsToFunction* dest, PointsToFunction* src) {
    PointsToFunction* cursor = src;
    dest->pte = NULL;
    dest->otherPtes = NULL;
    if (dest->pte) {
        while (cursor && cursor->pte) {
            PointsToFunction_Add(dest, cursor->pte);
            cursor = cursor->otherPtes;
        }
        return;
    }
    while (cursor && cursor->pte) {
        PointsToFunction_SimpleAdd(dest, cursor->pte);
        cursor = cursor->otherPtes;
    }
}

void PointsToEntry_Term(PointsToEntry* pte) {
    LocationSet* ls = pte->loc;
    LocationSetSet* lss;
    if (LocationSet_IsUnknown(ls) && ls->u.unknown.bitfieldOf) {
        LocationSet_Term(ls->u.unknown.bitfieldOf);
        LocationSet_Delete(ls->u.unknown.bitfieldOf);
    }
    ls->block = NULL;
    ls->rtype = NULL;
    ls->u.known.field = cint64_zero;
    ls->u.known.stride = 0;
    LocationSet_Delete(ls);

    lss = pte->locs;
    LocationSetSet_RemoveAll(lss);
    lss->loc = NULL;
    lss->otherLocs = NULL;
    lss->count = 0;
    LocationSetSet_Delete(lss);
    pte->loc = NULL;
    pte->locs = NULL;
}

void PointerAnalysis_Cleanup(void) {
    ObjectList* list;
    ObjectList* next;
    for (list = stParamObjs; list; list = next) {
        IRO_gfree(list->object);
        next = list->next;
        IRO_gfree(list);
    }
    stParamObjs = NULL;
}

void PointerAnalysis_Setup(void) {
    stExtendedParamNum = 0;
    stParamObjs = NULL;
    stMaxPassCount = 0;
    unk6367D4 = 0x103;
    unk6367CC = &stunsignedlong;
    unk6367C2 = 0;
}

#pragma dont_inline on
Boolean PointsToFunction_SimpleAdd(PointsToFunction* ptf, PointsToEntry* pte) {
    PointsToEntry* newPte;

    newPte = IRO_malloc(sizeof(PointsToEntry));
    newPte->loc = NULL;
    newPte->locs = NULL;
    PointsToEntry_Init(newPte, pte->loc, pte->locs);
    if (ptf->pte != NULL) {
        PointsToFunction* newPtf = IRO_malloc(sizeof(PointsToFunction));
        newPtf->pte = NULL;
        newPtf->otherPtes = NULL;
        newPtf->pte = NULL;
        newPtf->otherPtes = NULL;
        newPtf->pte = ptf->pte;
        newPtf->otherPtes = ptf->otherPtes;
        ptf->otherPtes = newPtf;
    }
    ptf->pte = newPte;
    return TRUE;
}
#pragma dont_inline reset

void ObjectSet_Add(ObjectSet* os, Object* obj) {
    ObjectSet* cur = os;
    while (cur && cur->obj) {
        if (cur->obj == obj) {
            break;
        }
        cur = cur->otherObjs;
    }
    if (!cur || !cur->obj) {
        if (os->obj) {
            ObjectSet* next = ObjectSet_New();
            ObjectSet_Init(next);
            next->obj = os->obj;
            next->otherObjs = os->otherObjs;
            os->otherObjs = next;
        }
        os->obj = obj;
    }
}

Boolean PALocalVars_Equal(PALocalVar* a, PALocalVar* b) {
    if (a == b) {
        return TRUE;
    }
    if (!a || !b) {
        return FALSE;
    }
    if (!a->obj || !b->obj) {
        if (a->name) {
            return b->name && strcmp(a->name, b->name) == 0;
        }
    }
    return a->obj == b->obj;
}

Boolean ExtendedParams_Equal(ExtendedParam* ep1, ExtendedParam* ep2) {
    ObjectSet* os1;
    ObjectSet* os2;
    ObjectSet* scan;
    ObjectSet* cur;
    if (ep1 == ep2) {
        return TRUE;
    }
    if (ep1->num != ep2->num) {
        return FALSE;
    }
    os1 = ep1->objectSet;
    os2 = ep2->objectSet;
    if (os1 == os2) {
        return TRUE;
    }
    for (scan = os1; scan && scan->obj; scan = scan->otherObjs) {
        for (cur = os2; cur && cur->obj; cur = cur->otherObjs) {
            if (cur->obj == scan->obj) {
                break;
            }
        }
        if (!cur || !cur->obj) {
            return FALSE;
        }
    }
    for (scan = os2; scan && scan->obj; scan = scan->otherObjs) {
        for (cur = os1; cur && cur->obj; cur = cur->otherObjs) {
            if (cur->obj == scan->obj) {
                break;
            }
        }
        if (!cur || !cur->obj) {
            return FALSE;
        }
    }
    return TRUE;
}

Boolean MemoryBlocks_Equal(PAMemoryBlock* mb1, PAMemoryBlock* mb2) {
    if (mb1 == mb2) {
        return TRUE;
    }
    if (!mb1 || !mb2 || mb1->kind != mb2->kind) {
        return FALSE;
    }

    switch (mb1->kind) {
        case 2: {
            ExtendedParam* ep1 = mb1->u.ep;
            ExtendedParam* ep2 = mb2->u.ep;
            ObjectSet* os1;
            ObjectSet* os2;
            ObjectSet* scan;
            ObjectSet* cur;
            Boolean found;
            if (ep1 == ep2) {
                return TRUE;
            }
            found = ep1->num == ep2->num;
            if (!found) {
                return FALSE;
            }
            os1 = ep1->objectSet;
            os2 = ep2->objectSet;
            if (os1 == os2) {
                return TRUE;
            }
            found = FALSE;
            for (scan = os1; scan && scan->obj; scan = scan->otherObjs) {
                for (cur = os2; cur && cur->obj; cur = cur->otherObjs) {
                    if (cur->obj == scan->obj) {
                        found = TRUE;
                        break;
                    }
                }
                if (!found) {
                    return FALSE;
                }
                found = FALSE;
            }
            for (scan = os2; scan && scan->obj; scan = scan->otherObjs) {
                for (cur = os1; cur && cur->obj; cur = cur->otherObjs) {
                    if (cur->obj == scan->obj) {
                        found = TRUE;
                        break;
                    }
                }
                if (!found) {
                    return FALSE;
                }
                found = FALSE;
            }
            return TRUE;
        }
        case 3: {
            PALocalVar* lv1 = mb1->u.localvar;
            PALocalVar* lv2 = mb2->u.localvar;
            if (lv1 == lv2) {
                return TRUE;
            }
            if (!lv1 || !lv2) {
                return FALSE;
            }
            if (!lv1->obj || !lv2->obj) {
                if (lv1->name) {
                    return lv2->name && strcmp(lv1->name, lv2->name) == 0;
                }
            }
            return lv1->obj == lv2->obj;
        }
        case 4: {
            PAHeapBlock* hb1 = mb1->u.heapblock;
            PAHeapBlock* hb2 = mb2->u.heapblock;
            return hb1 == hb2 || hb1->nd == hb2->nd;
        }
        case 5:
            return CInt64_Equal(mb1->u.intval, mb2->u.intval);
        case 6:
            return mb1->u.other == mb2->u.other;
        case 7: {
            IROLinear* o1 = mb1->u.linear;
            IROLinear* o2 = mb2->u.linear;
            if (o1 == o2) {
                return TRUE;
            }
            return o1->pointsToFunction == o2->pointsToFunction;
        }
        default:
            CError_FATAL(1693);
            return FALSE;
    }
}

ParamMapping* ParamMappingFunction_FindMappingByFormal(ParamMappingFunction* pmf, Object* formal) {
    while (pmf && pmf->mapping) {
        if (pmf->mapping->formal == formal) {
            return pmf->mapping;
        }
        pmf = pmf->otherMappings;
    }
    return NULL;
}

void PointsToEntry_Init(PointsToEntry* pte, LocationSet* loc, LocationSetSet* locs) {
    LocationSet* destLoc = IRO_malloc(sizeof(LocationSet));
    LocationSetSet* destSet;
    destLoc->block = NULL;
    destLoc->rtype = NULL;
    destLoc->u.known.field = cint64_zero;
    destLoc->u.known.stride = 0;
    pte->loc = destLoc;
    destLoc->block = loc->block;
    destLoc->rtype = loc->rtype;
    if (!LocationSet_IsUnknown(loc)) {
        destLoc->u.known.field = loc->u.known.field;
        destLoc->u.known.stride = loc->u.known.stride;
    } else {
        destLoc->u.unknown.restriction = loc->u.unknown.restriction;
        if (loc->u.unknown.bitfieldOf) {
            destLoc->u.unknown.bitfieldOf = LocationSet_New();
            LocationSet_Copy(destLoc->u.unknown.bitfieldOf, loc->u.unknown.bitfieldOf);
        } else {
            destLoc->u.unknown.bitfieldOf = NULL;
        }
    }
    destSet = IRO_malloc(sizeof(LocationSetSet));
    destSet->loc = NULL;
    destSet->otherLocs = NULL;
    destSet->count = 0;
    pte->locs = destSet;
    destSet->loc = NULL;
    destSet->otherLocs = NULL;
    destSet->count = 0;
    if (destSet->count) {
        while (locs && locs->loc) {
            LocationSetSet_Add(destSet, locs->loc);
            locs = locs->otherLocs;
        }
    } else {
        while (locs && locs->loc) {
            LocationSetSet_SimpleAddImpl(destSet, locs->loc);
            locs = locs->otherLocs;
        }
    }
}

void PARegionSet_Init(PARegionSet* set) {
    set->region = NULL;
    set->otherRegions = NULL;
}

PARegionSet* PARegionSet_New(void) {
    PARegionSet* set = IRO_malloc(sizeof(PARegionSet));
    set->region = NULL;
    set->otherRegions = NULL;
    return set;
}

PARegion* PARegion_New(void) {
    PARegion* region = IRO_malloc(sizeof(PARegion));
    region->thing = NULL;
    region->kind = 0;
    return region;
}

#pragma dont_inline on
void PARegionSet_Delete(PARegionSet* set) {
    IRO_DEBUG_CLEAR(set, sizeof(PARegionSet));
    IRO_gfree(set);
}

void PARegion_Delete(PARegion* region) {
    IRO_DEBUG_CLEAR(region, sizeof(PARegion));
    IRO_gfree(region);
}

void PARegion_Term(PARegion* region) {
    if (region->thing) {
        IRO_ofree(region->thing);
        region->thing = NULL;
    }
    region->kind = 0;
}

#pragma dont_inline reset

Boolean PARegions_Equal(PARegion* first, PARegion* second) {
    return first->kind == second->kind && Bv_Compare(first->thing, second->thing);
}

#pragma dont_inline on
PARegion* PARegionSet_Find(PARegionSet* set, PARegion* region) {
    UInt16 kind = region->kind;
    while (set && set->region) {
        if (set->region->kind < kind) {
        } else if (set->region->kind > kind) {
            break;
        } else if (set->region->kind == region->kind &&
                   Bv_Compare(set->region->thing, region->thing)) {
            return set->region;
        }
        set = set->otherRegions;
    }
    return NULL;
}
#pragma dont_inline reset

void PTFList_RemoveAll(PTFList* list) {
    while (list && list->ptf) {
        void* target = list->ptf;
        PTFList* cur = list;
        PTFList* prev = NULL;
        while (cur && cur->ptf) {
            if (cur->ptf == target) {
                if (prev) {
                    prev->otherPTFs = cur->otherPTFs;
                    cur->ptf = NULL;
                    cur->otherPTFs = NULL;
                    PTFList_Term(cur);
                    PTFList_Delete(cur);
                } else if (cur->otherPTFs) {
                    PTFList* second = cur->otherPTFs;
                    cur->ptf = second->ptf;
                    cur->otherPTFs = second->otherPTFs;
                    second->ptf = NULL;
                    second->otherPTFs = NULL;
                    PTFList_Term(second);
                    PTFList_Delete(second);
                } else {
                    cur->ptf = NULL;
                }
                break;
            }
            prev = cur;
            cur = cur->otherPTFs;
        }
    }
}

void PTFList_Term(PTFList* list) {
    while (list && list->ptf) {
        void* target = list->ptf;
        PTFList* cur = list;
        PTFList* prev = NULL;
        while (cur && cur->ptf) {
            if (cur->ptf == target) {
                if (prev) {
                    prev->otherPTFs = cur->otherPTFs;
                    cur->ptf = NULL;
                    cur->otherPTFs = NULL;
                    PTFList_Term(cur);
                    PTFList_Delete(cur);
                } else if (cur->otherPTFs) {
                    PTFList* second = cur->otherPTFs;
                    cur->ptf = second->ptf;
                    cur->otherPTFs = second->otherPTFs;
                    second->ptf = NULL;
                    second->otherPTFs = NULL;
                    PTFList_Term(second);
                    PTFList_Delete(second);
                } else {
                    cur->ptf = NULL;
                }
                break;
            }
            prev = cur;
            cur = cur->otherPTFs;
        }
    }
    list->ptf = NULL;
    list->otherPTFs = NULL;
}

void ExtendedParamSet_Term(ExtendedParamSet* list);

void ExtendedParamSet_RemoveAll(ExtendedParamSet* list) {
    while (list && list->ep) {
        void* target = list->ep;
        ExtendedParamSet* cur = list;
        ExtendedParamSet* prev = NULL;
        while (cur && cur->ep) {
            if (cur->ep == target) {
                if (prev) {
                    prev->otherEps = cur->otherEps;
                    cur->ep = NULL;
                    cur->otherEps = NULL;
                    ExtendedParamSet_Term(cur);
                    ExtendedParamSet_Delete(cur);
                } else if (cur->otherEps) {
                    ExtendedParamSet* second = cur->otherEps;
                    cur->ep = second->ep;
                    cur->otherEps = second->otherEps;
                    second->ep = NULL;
                    second->otherEps = NULL;
                    ExtendedParamSet_Term(second);
                    ExtendedParamSet_Delete(second);
                } else {
                    cur->ep = NULL;
                }
                break;
            }
            prev = cur;
            cur = cur->otherEps;
        }
    }
}

void ExtendedParamSet_Term(ExtendedParamSet* list) {
    while (list && list->ep) {
        void* target = list->ep;
        ExtendedParamSet* cur = list;
        ExtendedParamSet* prev = NULL;
        while (cur && cur->ep) {
            if (cur->ep == target) {
                if (prev) {
                    prev->otherEps = cur->otherEps;
                    cur->ep = NULL;
                    cur->otherEps = NULL;
                    ExtendedParamSet_Term(cur);
                    ExtendedParamSet_Delete(cur);
                } else if (cur->otherEps) {
                    ExtendedParamSet* second = cur->otherEps;
                    cur->ep = second->ep;
                    cur->otherEps = second->otherEps;
                    second->ep = NULL;
                    second->otherEps = NULL;
                    ExtendedParamSet_Term(second);
                    ExtendedParamSet_Delete(second);
                } else {
                    cur->ep = NULL;
                }
                break;
            }
            prev = cur;
            cur = cur->otherEps;
        }
    }
    list->ep = NULL;
    list->otherEps = NULL;
}

void PARegion_Copy(PARegion* dest, PARegion* src) {
    Bv_AllocVector(&dest->thing, src->thing->size);
    Bv_Copy(src->thing, dest->thing);
    dest->kind = src->kind;
}

void PARegionSet_SimpleAdd(PARegionSet* set, PARegion* region) {
    PARegion* newRegion = PARegion_New();
    PARegionSet* prev;
    PARegionSet* newSet;
    UInt16 kind;

    PARegion_Copy(newRegion, region);
    if (!set->region) {
        set->region = newRegion;
    } else {
        kind = region->kind;
        prev = NULL;
        for (; set && set->region && set->region->kind < kind; set = set->otherRegions) {
            prev = set;
        }
        newSet = PARegionSet_New();
        PARegionSet_Init(newSet);
        if (prev) {
            newSet->region = newRegion;
            newSet->otherRegions = prev->otherRegions;
            prev->otherRegions = newSet;
        } else {
            newSet->region = set->region;
            newSet->otherRegions = set->otherRegions;
            set->region = newRegion;
            set->otherRegions = newSet;
        }
    }
}

UInt32 IRO_TypeSize(Type* type) {
    return type->size;
}

Boolean IRO_TypesEqual2(Type* first, Type* second) {
    return first == second || IRO_TypesEqual(first, second);
}

void ParamMappingFunction_Add(ParamMappingFunction* pmf, ParamMapping* mapping) {
    ParamMapping* existing = ParamMappingFunction_FindMappingByFormal(pmf, mapping->formal);
    if (!existing) {
        existing = ParamMapping_New();
        ParamMapping_Copy(existing, mapping);
        if (pmf->mapping) {
            ParamMappingFunction* next = ParamMappingFunction_New();
            ParamMappingFunction_Init(next);
            next->mapping = pmf->mapping;
            next->otherMappings = pmf->otherMappings;
            pmf->otherMappings = next;
        }
        pmf->mapping = existing;
    } else {
        existing->actual = mapping->actual;
        existing->extended = mapping->extended;
    }
}

#pragma dont_inline on
void LocationSet_Copy(LocationSet* dest, LocationSet* src) {
    dest->block = src->block;
    dest->rtype = src->rtype;
    if (src != stUnknownLs && src->block != stUnknownMb) {
        goto known;
    }
    {
        dest->u.unknown.restriction = src->u.unknown.restriction;
        if (src->u.unknown.bitfieldOf) {
            LocationSet* nested = IRO_malloc(sizeof(LocationSet));
            nested->block = NULL;
            nested->rtype = NULL;
            nested->u.known.field = cint64_zero;
            nested->u.known.stride = 0;
            dest->u.unknown.bitfieldOf = nested;
            src = src->u.unknown.bitfieldOf;
            dest = dest->u.unknown.bitfieldOf;
            dest->block = src->block;
            dest->rtype = src->rtype;
            if (LocationSet_IsUnknown(src)) {
                dest->u.unknown.restriction = src->u.unknown.restriction;
                if (src->u.unknown.bitfieldOf) {
                    dest->u.unknown.bitfieldOf = LocationSet_New();
                    LocationSet_Copy(dest->u.unknown.bitfieldOf, src->u.unknown.bitfieldOf);
                } else {
                    dest->u.unknown.bitfieldOf = NULL;
                }
            } else {
                dest->u.known.field = src->u.known.field;
                dest->u.known.stride = src->u.known.stride;
            }
        } else {
            dest->u.unknown.bitfieldOf = NULL;
        }
    }
    return;
known:
    dest->u.known.field = src->u.known.field;
    dest->u.known.stride = src->u.known.stride;
}
#pragma dont_inline reset

#pragma dont_inline on
void ParamMappingFunction_Delete(ParamMappingFunction* pmf) {
    IRO_DEBUG_CLEAR(pmf, sizeof(ParamMappingFunction));
    IRO_gfree(pmf);
}
#pragma dont_inline reset

void ParamMappingFunction_RemoveAll(ParamMappingFunction* pmf) {
    while (pmf && pmf->mapping) {
        ParamMapping* target = pmf->mapping;
        ParamMappingFunction* cur = pmf;
        ParamMappingFunction* prev = NULL;
        while (cur && cur->mapping) {
            if (cur->mapping->formal == target->formal) {
                ParamMapping_Delete(cur->mapping);
                ParamMapping_Term(cur->mapping);
                if (prev) {
                    prev->otherMappings = cur->otherMappings;
                    cur->mapping = NULL;
                    cur->otherMappings = NULL;
                    ParamMappingFunction_Term(cur);
                    ParamMappingFunction_Delete(cur);
                } else if (cur->otherMappings) {
                    ParamMappingFunction* second = cur->otherMappings;
                    cur->mapping = second->mapping;
                    cur->otherMappings = second->otherMappings;
                    second->mapping = NULL;
                    second->otherMappings = NULL;
                    ParamMappingFunction_Term(second);
                    ParamMappingFunction_Delete(second);
                } else {
                    cur->mapping = NULL;
                }
                break;
            }
            prev = cur;
            cur = cur->otherMappings;
        }
    }
}

void ParamMappingFunction_Term(ParamMappingFunction* pmf) {
    while (pmf && pmf->mapping) {
        ParamMapping* target = pmf->mapping;
        ParamMappingFunction* cur = pmf;
        ParamMappingFunction* prev = NULL;
        while (cur && cur->mapping) {
            if (cur->mapping->formal == target->formal) {
                ParamMapping_Delete(cur->mapping);
                ParamMapping_Term(cur->mapping);
                if (prev) {
                    prev->otherMappings = cur->otherMappings;
                    cur->mapping = NULL;
                    cur->otherMappings = NULL;
                    ParamMappingFunction_Term(cur);
                    ParamMappingFunction_Delete(cur);
                } else if (cur->otherMappings) {
                    ParamMappingFunction* second = cur->otherMappings;
                    cur->mapping = second->mapping;
                    cur->otherMappings = second->otherMappings;
                    second->mapping = NULL;
                    second->otherMappings = NULL;
                    ParamMappingFunction_Term(second);
                    ParamMappingFunction_Delete(second);
                } else {
                    cur->mapping = NULL;
                }
                break;
            }
            prev = cur;
            cur = cur->otherMappings;
        }
    }
    pmf->mapping = NULL;
    pmf->otherMappings = NULL;
}

void PARegionSet_RemoveAll(PARegionSet* set) {
    while (set && set->region) {
        PARegion* target = set->region;
        PARegionSet* cur = set;
        PARegionSet* prev = NULL;
        UInt16 kind = target->kind;
        while (cur && cur->region) {
            if (cur->region->kind < kind) {
                prev = cur;
                cur = cur->otherRegions;
                continue;
            }
            if (cur->region->kind > kind) {
                break;
            }
            if (PARegions_Equal(cur->region, target)) {
                PARegion_Term(cur->region);
                PARegion_Delete(cur->region);
                if (prev) {
                    prev->otherRegions = cur->otherRegions;
                    cur->region = NULL;
                    cur->otherRegions = NULL;
                    PARegionSet_Term(cur);
                    PARegionSet_Delete(cur);
                } else if (cur->otherRegions) {
                    PARegionSet* second = cur->otherRegions;
                    cur->region = second->region;
                    cur->otherRegions = second->otherRegions;
                    second->region = NULL;
                    second->otherRegions = NULL;
                    PARegionSet_Term(second);
                    PARegionSet_Delete(second);
                } else {
                    cur->region = NULL;
                }
                break;
            }
            prev = cur;
            cur = cur->otherRegions;
        }
    }
}

void PARegionSet_Term(PARegionSet* set) {
    while (set && set->region) {
        PARegion* target = set->region;
        PARegionSet* cur = set;
        PARegionSet* prev = NULL;
        UInt16 kind = target->kind;
        while (cur && cur->region) {
            if (cur->region->kind < kind) {
                prev = cur;
                cur = cur->otherRegions;
                continue;
            }
            if (cur->region->kind > kind) {
                break;
            }
            if (PARegions_Equal(cur->region, target)) {
                PARegion_Term(cur->region);
                PARegion_Delete(cur->region);
                if (prev) {
                    prev->otherRegions = cur->otherRegions;
                    cur->region = NULL;
                    cur->otherRegions = NULL;
                    PARegionSet_Term(cur);
                    PARegionSet_Delete(cur);
                } else if (cur->otherRegions) {
                    PARegionSet* second = cur->otherRegions;
                    cur->region = second->region;
                    cur->otherRegions = second->otherRegions;
                    second->region = NULL;
                    second->otherRegions = NULL;
                    PARegionSet_Term(second);
                    PARegionSet_Delete(second);
                } else {
                    cur->region = NULL;
                }
                break;
            }
            prev = cur;
            cur = cur->otherRegions;
        }
    }
    set->region = NULL;
    set->otherRegions = NULL;
}

void LocationSet_InitUnknown(
    LocationSet* ls, Type* rtype, PAMemoryBlock* restriction, LocationSet* bitfieldOf
) {
    LocationSet_Copy(ls, stUnknownLs);
    ls->rtype = rtype;
    ls->u.unknown.restriction = restriction;
    if (bitfieldOf) {
        ls->u.unknown.bitfieldOf = LocationSet_New();
        LocationSet_Copy(ls->u.unknown.bitfieldOf, bitfieldOf);
    } else {
        ls->u.unknown.bitfieldOf = NULL;
    }
}

void PointsToFunction_RemoveAll(PointsToFunction* ptf) {
    while (ptf && ptf->pte) {
        PointsToFunction* cur = ptf;
        LocationSet* target = ptf->pte->loc;
        PointsToFunction* prev = NULL;
        while (cur && cur->pte) {
            if (LocationSets_Equal(cur->pte->loc, target)) {
                PointsToEntry_Term(cur->pte);
                PointsToEntry_Delete(cur->pte);
                if (prev) {
                    prev->otherPtes = cur->otherPtes;
                    cur->pte = NULL;
                    cur->otherPtes = NULL;
                    PointsToFunction_Term(cur);
                    PointsToFunction_Delete(cur);
                } else if (cur->otherPtes) {
                    PointsToFunction* second = cur->otherPtes;
                    cur->pte = second->pte;
                    cur->otherPtes = cur->otherPtes->otherPtes;
                    second->pte = NULL;
                    second->otherPtes = NULL;
                    PointsToFunction_Term(second);
                    PointsToFunction_Delete(second);
                } else {
                    cur->pte = NULL;
                }
                break;
            }
            prev = cur;
            cur = cur->otherPtes;
        }
    }
}

void LocationSetSet_RemoveAll(LocationSetSet* set) {
    while (set && set->loc) {
        LocationSetSet* cur = set;
        LocationSet* target = set->loc;
        LocationSetSet* prev = NULL;
        while (cur && cur->loc) {
            if (LocationSets_Equal(cur->loc, target)) {
                if (cur->loc != stUnknownLs) {
                    LocationSet_Term(cur->loc);
                    LocationSet_Delete(cur->loc);
                }
                if (prev) {
                    prev->otherLocs = cur->otherLocs;
                    cur->loc = NULL;
                    cur->otherLocs = NULL;
                    LocationSetSet_Term(cur);
                    LocationSetSet_Delete(cur);
                } else if (cur->otherLocs) {
                    LocationSetSet* second = cur->otherLocs;
                    cur->loc = second->loc;
                    cur->otherLocs = cur->otherLocs->otherLocs;
                    second->loc = NULL;
                    second->otherLocs = NULL;
                    LocationSetSet_Term(second);
                    LocationSetSet_Delete(second);
                } else {
                    cur->loc = NULL;
                }
                set->count--;
                break;
            }
            prev = cur;
            cur = cur->otherLocs;
        }
    }
}

void ObjectSet_RemoveAll(ObjectSet* set) {
    while (set && set->obj) {
        Object* target = set->obj;
        ObjectSet* cur = set;
        ObjectSet* prev = NULL;
        while (cur && cur->obj) {
            if (fn_004DB110(cur->obj, target)) {
                if (prev) {
                    prev->otherObjs = cur->otherObjs;
                    cur->obj = NULL;
                    cur->otherObjs = NULL;
                    ObjectSet_Term(cur);
                    ObjectSet_Delete(cur);
                } else if (cur->otherObjs) {
                    ObjectSet* second = cur->otherObjs;
                    cur->obj = second->obj;
                    cur->otherObjs = second->otherObjs;
                    second->obj = NULL;
                    second->otherObjs = NULL;
                    ObjectSet_Term(second);
                    ObjectSet_Delete(second);
                } else {
                    cur->obj = NULL;
                }
                break;
            }
            prev = cur;
            cur = cur->otherObjs;
        }
    }
}

#pragma dont_inline on
#pragma dont_inline reset

HEADER_INLINE void LocationSetSet_ForEach(
    LocationSetSet* lss, void (*action)(LocationSet*, void*), void* refcon
) {
    while (lss && lss->loc) {
        action(lss->loc, refcon);
        lss = lss->otherLocs;
    }
}

HEADER_INLINE void PointsToFunction_ForEach(
    PointsToFunction* pointsToFunc, void (*action)(PointsToEntry*, void*), void* refcon
) {
    while (pointsToFunc && pointsToFunc->pte) {
        action(pointsToFunc->pte, refcon);
        pointsToFunc = pointsToFunc->otherPtes;
    }
}

HEADER_INLINE PointsToFunction* PartialTransferFunction_finalPointsToFn(
    PartialTransferFunction* ptf
) {
    return ptf->finalPointsToFn;
}

HEADER_INLINE LocationSetSet* PartialTransferFunction_funcModifies(PartialTransferFunction* ptf) {
    return ptf->funcModifies;
}

HEADER_INLINE PartialTransferFunction* PTFList_FindFirst(PTFList* ptfList) {
    return ptfList->ptf;
}

HEADER_INLINE int NoPTF(Object* proc, PartialTransferFunction** ptf) {
    int fail = 1;
    if (proc->u.func.ptfList && (*ptf = PTFList_FindFirst(proc->u.func.ptfList))) {
        fail = 0;
    }
    return fail;
}

HEADER_INLINE int NoPTFList(Object* proc) {
    int fail = 1;
    if (proc->u.func.ptfList && PTFList_FindFirst(proc->u.func.ptfList)) {
        fail = 0;
    }
    return fail;
}

void PointerAnalysis_GetFunctionDependencies(Object* proc, IROLinear* funccall, ObjectList** list) {
    Boolean fail;
    PointsToFunction* finalPointsTo;
    GetFunctionDepsOrKillsActionParams params;
    PartialTransferFunction* ptf;
    ParamMappingFunction* map;

    if (!IRO_ObjectIsAFunction(proc)) {
        return;
    }

    fail = NoPTFList(proc);
    if (!fail) {
        CreateFalseContext(proc, funccall, &ptf, &map);
        fail = TRUE;
        if (ptf && map && (finalPointsTo = PartialTransferFunction_finalPointsToFn(ptf))) {
            fail = FALSE;
        }
        if (!fail) {
            memset(&params, 0, sizeof(params));
            params.proc = proc;
            params.ptf = ptf;
            params.funccall = funccall;
            params.map = map;
            params.list = list;
            PointsToFunction_ForEach(finalPointsTo, GetFunctionDepsOrKillsAction2, &params);
        }
        DestroyFalseContext(proc, ptf, map);
    }

    if (fail) {
        *list = IRO_malloc(sizeof(ObjectList));
        (*list)->object = NULL;
        (*list)->next = NULL;
    }
}

void PointerAnalysis_GetFunctionKills(Object* proc, IROLinear* funccall, ObjectList** list) {
    Boolean fail;
    PartialTransferFunction* ptf;
    LocationSetSet* set;
    GetFunctionDepsOrKillsActionParams params;

    if (!IRO_ObjectIsAFunction(proc)) {
        return;
    }

    fail = NoPTF(proc, &ptf);
    if (!fail) {
        set = PartialTransferFunction_funcModifies(ptf);
        fail = set == NULL;
        if (!fail) {
            memset(&params, 0, sizeof(params));
            params.proc = proc;
            params.ptf = ptf;
            params.funccall = funccall;
            params.map = NULL;
            params.list = list;
            LocationSetSet_ForEach(set, GetFunctionDepsOrKillsAction, &params);
        }
    }

    if (fail) {
        *list = IRO_malloc(sizeof(ObjectList));
        (*list)->object = NULL;
        (*list)->next = NULL;
    }
}

Boolean PointerAnalysis_IsLinearNodePointerExprDefinite(Object* proc, IROLinear* nd) {
    LocationSetSet* lss;
    LocationSet* loc;
    int result;
    int count;

    if (!nd->pointsToFunction) {
        return FALSE;
    }
    lss = LocationSetSet_New();
    LocationSetSet_Init(lss);
    EvalExpr(lss, proc, nd, NULL, NULL, NULL);
    count = lss->count;
    result = FALSE;
    if (count == 1 && (loc = lss->loc) && LocationSetRepresentsSingleLocation(loc, NULL, NULL)) {
        result = TRUE;
    }
    LocationSetSet_RemoveAll(lss);
    lss->loc = NULL;
    lss->otherLocs = NULL;
    lss->count = 0;
    IRO_DEBUG_CLEAR(lss, sizeof(LocationSetSet));
    IRO_gfree(lss);
    return result;
}

void GetFunctionDepsOrKillsAction2(PointsToEntry* pte, void* refcon) {
    GetFunctionDepsOrKillsActionParams* params = refcon;
    PAMemoryBlock* block;
    LocationSet* loc;
    loc = pte->loc;
    block = loc->block;
    if (block && block->kind == 2) {
        LocationSetSet* set = LocationSetSet_New();
        LocationSetSet* scan;
        LocationSetSet_Init(set);
        GetActualLocsOfExtendedParam(set, loc, NULL, &stCallingContextStack, params->map, FALSE);
        scan = set;
        while (scan && scan->loc) {
            GetFunctionDepsOrKillsAction(scan->loc, params);
            scan = scan->otherLocs;
        }
        LocationSetSet_RemoveAll(set);
        set->loc = NULL;
        set->otherLocs = NULL;
        set->count = 0;
        IRO_DEBUG_CLEAR(set, sizeof(LocationSetSet));
        IRO_gfree(set);
    }
}

void LocationSetSet_SimpleAddImpl(LocationSetSet* lss, LocationSet* ls) {
    LocationSet* ls2;

    if (!LocationSet_IsUnknown(ls) && lss->count < 4) {
        if (ls == stUnknownLs) {
            ls2 = stUnknownLs;
        } else {
            ls2 = LocationSet_New();
            LocationSet_Copy(ls2, ls);
        }
        if (lss->loc) {
            LocationSetSet* next = LocationSetSet_New();
            LocationSetSet_Init(next);
            next->loc = lss->loc;
            next->otherLocs = lss->otherLocs;
            lss->otherLocs = next;
        }
        lss->loc = ls2;
        lss->count++;
    } else {
        LocationSetSet_RemoveAll(lss);
        ls2 = LocationSet_New();
        if (LocationSet_IsUnknown(ls)) {
            LocationSet_Copy(ls2, ls);
        } else {
            LocationSet_Copy(ls2, stUnknownLs);
            if (ls->rtype) {
                LocationSet_SetRtype(ls2, ls->rtype);
            }
        }
        lss->loc = ls2;
        lss->count = 1;
    }
}

void PointerAnalysis_LookupLinearNodePointerExpr(
    Object* proc, IROLinear* indirect, IROListNode** list
) {
    LocationSetSet* volatile set;
    LocationSetSet* scan;
    LookupLinearExprActionParams params;

    if (!indirect->pointsToFunction) {
        return;
    }
    set = LocationSetSet_New();
    LocationSetSet_Init(set);
    EvalExpr(set, proc, indirect, NULL, NULL, NULL);
    memset(&params, 0, sizeof(params));
    params.proc = proc;
    params.indirectType = IRO_CopyType(indirect->rtype);
    params.list = list;
    scan = set;
    while (scan && scan->loc) {
        IROListNode* node = *params.list = IRO_malloc(sizeof(IROListNode));
        IRO_InitList(&node->list);
        node->nextList = NULL;
        if (!LocationSet_IsUnknown(scan->loc) &&
            LocationSetRepresentsSingleLocation(scan->loc, NULL, NULL)) {
            CreateExpressionForLocationSet(
                scan->loc, &node->list, params.indirectType, params.proc
            );
        }
        params.list = &node->nextList;
        scan = scan->otherLocs;
    }
    LocationSetSet_RemoveAll(set);
    set->loc = NULL;
    set->otherLocs = NULL;
    set->count = 0;
    IRO_DEBUG_CLEAR(set, sizeof(LocationSetSet));
    IRO_gfree(set);
}

LocationSet* PartialTransferFunction_returnLocation(PartialTransferFunction* ptf) {
    if (!ptf->returnLocation) {
        PAMemoryBlock* block = PAMemoryBlock_New();
        LocationSet* loc;
        PAMemoryBlock_Init(block, 3, NULL);
        loc = LocationSet_New();
        if (block->kind == 5) {
            LocationSet_InitUnknown(loc, (Type*)&void_ptr, NULL, NULL);
        } else {
            LocationSet_InitKnown(loc, block, cint64_zero, 0, (Type*)&void_ptr);
        }
        ptf->returnLocation = loc;
    }
    return ptf->returnLocation;
}

#pragma dont_inline on
void LocationSet_Term(LocationSet* ls) {
    if (LocationSet_IsUnknown(ls) && ls->u.unknown.bitfieldOf) {
        LocationSet* child = ls->u.unknown.bitfieldOf;
        if (LocationSet_IsUnknown(child) && child->u.unknown.bitfieldOf) {
            LocationSet* nested = child->u.unknown.bitfieldOf;
            if (LocationSet_IsUnknown(nested) && nested->u.unknown.bitfieldOf) {
                LocationSet_Term(nested->u.unknown.bitfieldOf);
                memset(nested->u.unknown.bitfieldOf, 0xFF, sizeof(LocationSet));
                IRO_gfree(nested->u.unknown.bitfieldOf);
            }
            nested->block = NULL;
            nested->rtype = NULL;
            nested->u.known.field = cint64_zero;
            nested->u.known.stride = 0;
            memset(nested, 0xFF, sizeof(LocationSet));
            IRO_gfree(nested);
        }
        child->block = NULL;
        child->rtype = NULL;
        child->u.known.field = cint64_zero;
        child->u.known.stride = 0;
        memset(child, 0xFF, sizeof(LocationSet));
        IRO_gfree(child);
    }
    ls->block = NULL;
    ls->rtype = NULL;
    ls->u.known.field = cint64_zero;
    ls->u.known.stride = 0;
}

#pragma dont_inline reset

void PAMemoryBlock_Delete(PAMemoryBlock* mb) {
    memset(mb, 0xFF, sizeof(PAMemoryBlock));
    IRO_gfree(mb);
}

void PAMemoryBlock_Term(PAMemoryBlock* mb) {
    mb->kind = 0;
}

void PointerAnalysis_HeapErrorProc(void) {
    longjmp(stAbortPointerAnalysis, 2);
}

inline Stack* Stack_New(void) {
    Stack* stack = IRO_malloc(sizeof(Stack));
    IRO_ASSERT(265, stack != NULL);
    stack->top = NULL;
    stack->next = NULL;
    return stack;
}

inline void Stack_Init(Stack* stack) {
    IRO_ASSERT(289, stack != NULL);
    stack->top = NULL;
    stack->next = NULL;
}

inline PartialTransferFunction* PartialTransferFunction_New(void) {
    PartialTransferFunction* ptf = IRO_malloc(sizeof(PartialTransferFunction));
    IRO_ASSERT(4110, ptf != NULL);
    ptf->initialPointsToFn = NULL;
    ptf->finalPointsToFn = NULL;
    ptf->funcModifies = NULL;
    ptf->context.nd = NULL;
    ptf->context.ptf = NULL;
    ptf->returnLocation = NULL;
    return ptf;
}

inline void PartialTransferFunction_Init(
    PartialTransferFunction* ptf, IROLinear* contextNd, PartialTransferFunction* contextPTF
) {
    IRO_ASSERT(4142, ptf != NULL);
    IRO_ASSERT(4143, contextNd == NULL || contextNd != NULL);
    IRO_ASSERT(4144, contextPTF == NULL || contextPTF != NULL);
    ptf->initialPointsToFn = PointsToFunction_New();
    PointsToFunction_Init(ptf->initialPointsToFn);
    ptf->finalPointsToFn = PointsToFunction_New();
    PointsToFunction_Init(ptf->finalPointsToFn);
    ptf->funcModifies = LocationSetSet_New();
    LocationSetSet_Init(ptf->funcModifies);
    LocationSetSet_AddUnknown(ptf->funcModifies, NULL, NULL, NULL);
    ptf->returnLocation = NULL;
    ptf->analyzed = FALSE;
    ptf->context.nd = contextNd;
    ptf->context.ptf = contextPTF;
}

inline ExtendedParamSet* ExtendedParamSet_New(void) {
    ExtendedParamSet* epList = IRO_malloc(sizeof(ExtendedParamSet));
    IRO_ASSERT(924, epList != NULL);
    epList->ep = NULL;
    epList->otherEps = NULL;
    return epList;
}

inline void ExtendedParamSet_Init(ExtendedParamSet* epList) {
    IRO_ASSERT(948, epList != NULL);
    epList->ep = NULL;
    epList->otherEps = NULL;
}

inline PTFList* PTFList_New(void) {
    PTFList* ptfList = IRO_malloc(sizeof(PTFList));
    IRO_ASSERT(4393, ptfList != NULL);
    ptfList->ptf = NULL;
    ptfList->otherPTFs = NULL;
    return ptfList;
}

inline void PTFList_Init(PTFList* ptfList) {
    IRO_ASSERT(4417, ptfList != NULL);
    ptfList->ptf = NULL;
    ptfList->otherPTFs = NULL;
}

void PointerAnalysis_Init(void) {
    stCallingContextStack = Stack_New();
    Stack_Init(stCallingContextStack);

    stUnknownPTF = PartialTransferFunction_New();
    PartialTransferFunction_Init(stUnknownPTF, NULL, NULL);

    stExtParamSet = ExtendedParamSet_New();
    ExtendedParamSet_Init(stExtParamSet);

    stPTFList = PTFList_New();
    PTFList_Init(stPTFList);
}

void IRO_AnalyzePointers(Object* function) {
    PAEvalCallActionParams params;
    UInt8 nd[0x44];
    UInt8 fnode[0x44];
    int code;
    void (*volatile saveheaperror)(void);

    PointerAnalysis_Init();
    memset(&params, 0, sizeof(params));
    memset(nd, 0, sizeof(nd));
    nd[0] = 6;
    memset(fnode, 0, sizeof(fnode));
    params.proc = &stUnknown;
    params.fnode = fnode;
    params.nd = nd;
    params.ptf = stUnknownPTF;
    params.map = NULL;
    params.stop = FALSE;
    params.result = FALSE;
    params.visitTarget = TRUE;

    stCurrentProc = function;
    if ((code = setjmp(stAbortPointerAnalysis)) == 0) {
        saveheaperror = (void (*)(void))getheaperror();
        setheaperror(PointerAnalysis_HeapErrorProc);
        EvalCallAction(stCurrentProc, &params);
        PointerAnalysis_Term();
        stCurrentProc = NULL;
        setheaperror(saveheaperror);
    } else {
        setheaperror(saveheaperror);
        InvalidatePointsToFunctions(stCurrentProc);
        PointerAnalysis_Term();
        stCurrentProc = NULL;
        if (code == 2 && saveheaperror) {
            saveheaperror();
        }
    }
}

void InvalidatePointsToFunctions(Object* proc) {
    PAFlowNode* fnode;
    PAFuncLinear* nd;
    ObjectSet* addressed;
    PointsToFunction* ptf;

    for (fnode = (PAFlowNode*)IRO_FunctionFirstNode(proc); fnode; fnode = fnode->nextnode) {
        if (fnode->last) {
            addressed = fnode->addressed;
            if (addressed) {
                ObjectSet_RemoveAll(addressed);
                addressed->obj = NULL;
                addressed->otherObjs = NULL;
                {
                    ObjectSet* deleted = fnode->addressed;
                    memset(deleted, 0xFF, sizeof(ObjectSet));
                    IRO_gfree(deleted);
                }
                fnode->addressed = NULL;
            }
            for (nd = (PAFuncLinear*)fnode->first; nd && nd != (PAFuncLinear*)fnode->last->next;
                 nd = nd->next) {
                ptf = nd->pointsToFunction;
                if (ptf) {
                    PointsToFunction_RemoveAll(ptf);
                    ptf->pte = NULL;
                    ptf->otherPtes = NULL;
                    {
                        PointsToFunction* deleted = nd->pointsToFunction;
                        memset(deleted, 0xFF, sizeof(PointsToFunction));
                        IRO_gfree(deleted);
                    }
                    nd->pointsToFunction = NULL;
                }
            }
        }
    }
}

Boolean PointerAnalysis_IsVariableValueDefinite(
    Object* proc, void* var, PointsToFunction* pointsTo
) {
    LocationSetSet* lss;
    LocationSet* loc;
    int result;
    int count;

    if (!pointsTo) {
        return FALSE;
    }
    lss = LocationSetSet_New();
    LocationSetSet_Init(lss);
    EvalVariable(lss, proc, var, pointsTo, NULL, NULL, NULL);
    count = lss->count;
    result = FALSE;
    if (count == 1 && (loc = lss->loc) && LocationSetRepresentsSingleLocation(loc, NULL, NULL)) {
        result = TRUE;
    }
    LocationSetSet_RemoveAll(lss);
    lss->loc = NULL;
    lss->otherLocs = NULL;
    lss->count = 0;
    IRO_DEBUG_CLEAR(lss, sizeof(LocationSetSet));
    IRO_gfree(lss);
    return result;
}

void PointerAnalysis_PragmaMode(void) {
    if (cparamblkptr->preprocess) {
        skipendofline();
        return;
    }
    if (notendofline()) {
        if (plex() == -3) {
            char* name = (char*)tkidentifier + 10;
            if (!memcmp(name, PA_modeAddr, 5)) {
                copts.opt_pointer_analysis_mode = 0;
                return;
            }
            if (!memcmp(name, PA_modeAnsi, 5)) {
                copts.opt_pointer_analysis_mode = 1;
                return;
            }
            if (!memcmp(name, PA_modeType, 5)) {
                copts.opt_pointer_analysis_mode = 2;
                return;
            }
        }
        CPrep_Warning(6);
        skipendofline();
        return;
    }
    if (copts.warn_illpragma) {
        CPrep_Warning(0x57);
    }
    skipendofline();
}

static inline Boolean memoryblocks_equal(PAMemoryBlock* mb1, PAMemoryBlock* mb2) {
    if (mb1 == mb2) {
        return TRUE;
    }
    if (!mb1 || !mb2 || mb1->kind != mb2->kind) {
        return FALSE;
    }
    switch (mb1->kind) {
        case 2:
            return ExtendedParams_Equal(mb1->u.ep, mb2->u.ep);
        case 3:
            return PALocalVars_Equal(mb1->u.localvar, mb2->u.localvar);
        case 4:
            return PAHeapBlocks_Equal(mb1->u.heapblock, mb2->u.heapblock);
        case 5:
            return CInt64_Equal(mb1->u.intval, mb2->u.intval);
        case 6:
            return mb1->u.other == mb2->u.other;
        case 7: {
            IROLinear* o1 = mb1->u.linear;
            IROLinear* o2 = mb2->u.linear;
            if (o1 == o2) {
                return TRUE;
            }
            return o1->pointsToFunction == o2->pointsToFunction;
        }
        default:
            CError_FATAL(1693);
            return FALSE;
    }
}

Boolean LocationSets_Equal(LocationSet* ls1, LocationSet* ls2) {
    return (ls1 == ls2) ||
        ((ls1 == stUnknownLs || ls1->block == stUnknownMb) &&
         (ls2 == stUnknownLs || ls2->block == stUnknownMb) &&
         memoryblocks_equal(ls1->u.unknown.restriction, ls2->u.unknown.restriction) &&
         (ls1->u.unknown.bitfieldOf == ls2->u.unknown.bitfieldOf ||
          (ls1->u.unknown.bitfieldOf != NULL &&
           ls2->u.unknown.bitfieldOf != NULL &&
           LocationSets_Equal(ls1->u.unknown.bitfieldOf, ls2->u.unknown.bitfieldOf))) &&
         (IRO_TypesEqual2(ls1->rtype, ls2->rtype) ||
          (ls1->rtype != NULL &&
           ls2->rtype != NULL &&
           IRO_TypeSize(ls1->rtype) == IRO_TypeSize(ls2->rtype)))) ||
        (!(ls1 == stUnknownLs || ls1->block == stUnknownMb) &&
         !(ls2 == stUnknownLs || ls2->block == stUnknownMb) &&
         ls1->u.known.stride == ls2->u.known.stride &&
         (IRO_TypesEqual2(ls1->rtype, ls2->rtype) ||
          (ls1->rtype != NULL &&
           ls2->rtype != NULL &&
           IRO_TypeSize(ls1->rtype) == IRO_TypeSize(ls2->rtype))) &&
         CInt64_Equal(ls1->u.known.field, ls2->u.known.field) &&
         memoryblocks_equal(ls1->block, ls2->block));
}

void* fn_004EC660(void** target, void* proc, Boolean enabled) {
    char* name = target[1];
    void* first = target[0];
    void* result = first;
    PAListSearchNode* node;
    Object* candidate;

    if (proc != &stUnknown && !first && name && *name && enabled) {
        node = fn_004E3DD0(proc);
        while (node && !result) {
            candidate = node->value;
            if (candidate && candidate != &stUnknown) {
                HashNameNode* candidateName = candidate->name;
                if (candidateName &&
                    candidateName->name &&
                    strcmp(candidateName->name, name) == 0) {
                    result = candidate;
                }
            }
            node = node->next;
        }
    }
    if (result) {
        target[0] = result;
    }
    return result;
}

void MatchPTFAction1(PointsToEntry* tgtPTE, void* refcon) {
    MatchPTFActionParams* params;
    LocationSet* loc;
    LocationSetSet* locs;
    PAMemoryBlock* block;

    params = refcon;
    if ((loc = tgtPTE->loc) && (locs = tgtPTE->locs)) {
        if ((block = loc->block)) {
            if (block->kind == PAMEMORYBLOCKKIND_LOCALVAR) {
                MatchPTFHelper(loc, locs, params->proc, params->mapCopy, params->ptfCopy);
            } else if (block->kind != PAMEMORYBLOCKKIND_EXTENDEDPARAM) {
                CError_FATAL(2467);
            }
        }
    }
}

static Boolean PA_Bv_IsBitSet(PA_BitVector* bv, UInt32 bit) {
    return (bit >> 5) < bv->size && (bv->data[bit >> 5] & (1 << (bit & 31)));
}

void fn_004DE480(PARegion* region, PARegionBitFilter* ctx) {
    PARegionBitFilter* current = ctx;
    PARegionSet* set;
    if (region != current->skip) {
        if (PA_Bv_IsBitSet((PA_BitVector*)region->thing, *ctx->index)) {
            set = current->regions;
            if (!PARegionSet_Find(set, region)) {
                PARegionSet_SimpleAdd(set, region);
            }
        }
    }
}

#pragma dont_inline on
void fn_004E5590(PointsToEntry* pte, PAVisitContext* context) {
    PointsToFunction* ptf = context->entries;
    LocationSet* loc;

    if (!fn_004D8C70(ptf, pte->loc)) {
        loc = LocationSetSet_FindFirst(pte->locs);
        if (!(loc &&
              LocationSet_IsUnknown(loc) &&
              !LocationSet_bitfieldOf(loc) &&
              !LocationSet_restriction(loc) &&
              !fn_004D84D0(ptf, pte))) {
            fn_004D7590(ptf, pte);
            if (!LocationSet_IsUnknown(pte->loc) || pte->loc->rtype) {
                PointsToFunction_SimpleAdd(ptf, pte);
            }
        }
    }
}
#pragma dont_inline reset

void fn_004E5620(PointsToEntry* pte, void* context) {
    LocationSet* loc;
    PointsToFunction* ptf;
    void** values = (void**)context;

    if (fn_004E58D0(pte->loc, values[0], values[1], values[2])) {
        ptf = values[4];
        if (!fn_004D8C70(ptf, pte->loc)) {
            loc = LocationSetSet_FindFirst(pte->locs);
            if (!(loc &&
                  LocationSet_IsUnknown(loc) &&
                  !LocationSet_bitfieldOf(loc) &&
                  !LocationSet_restriction(loc) &&
                  !fn_004D84D0(ptf, pte))) {
                fn_004D7590(ptf, pte);
                if (!LocationSet_IsUnknown(pte->loc) || pte->loc->rtype) {
                    PointsToFunction_SimpleAdd(ptf, pte);
                }
            }
        }
    }
}

void fn_004E56C0(void* candidate, PAEqualityContext* context) {
    if (!context->matched && fn_004E56F0(context->value, candidate)) {
        context->matched = TRUE;
    }
}

void fn_004E1960(PACallMatchContext* target, PACallMatchContext* ctx) {
    PASecondaryMatch* iter;
    PACallMatchContext* current = ctx;
    if (!current->active) {
        goto work;
    }
done:
    return;
work:
    if (fn_004E8D70(target, current->other, current->entries, current->params, current->extended)) {
        current->active = target;
        return;
    }
    for (iter = (PASecondaryMatch*)current->entries; iter && iter->value; iter = iter->next) {
        void** slot = (void**)((UInt8*)iter->value + 8);
        if (*slot) {
            *slot = NULL;
        }
    }
    if (target->matchedSecondary == current->params &&
        target->matchedPrimary == current->extended) {
        current->matchedPrimary = target;
    }
    goto done;
}

void fn_004DF6F0(PAMemoryBlock* block, PABlockContext* context) {
    PABlockNode* head;
    PABlockNode* iter;
    PABlockNode* next;
    PAMemoryBlock* found = NULL;
    if (block->unk02 != 1) {
        goto done;
    }
    head = context->list;
    iter = head;
    while (iter && (found = iter->block)) {
        if (found == block) {
            break;
        }
        iter = iter->next;
    }
    if (!iter) {
        found = NULL;
    }
    if (found) {
        goto done;
    }
    if (head->block) {
        next = IRO_malloc(sizeof(PABlockNode));
        next->block = NULL;
        next->next = NULL;
        next->block = NULL;
        next->next = NULL;
        next->block = head->block;
        next->next = head->next;
        head->next = next;
    }
    head->block = block;
done:
    return;
}

void fn_004E62E0(PAIndexEntry* candidate, PAIndexEntry* bounds) {
    PAIndexValue* value = candidate->value;
    UInt32 index;
    UInt32 upper;
    if (!value) {
        goto done;
    }
    index = value->index;
    if (bounds->upper) {
        upper = bounds->upper->value->index;
        if (bounds->lower) {
            if (index <= bounds->lower->value->index) {
                goto done;
            }
            if (index >= upper) {
                goto done;
            }
            goto set;
        }
        if (index >= upper) {
            goto done;
        }
        goto set;
    }
    if (bounds->lower && index <= bounds->lower->value->index) {
        goto done;
    }
set:
    bounds->upper = candidate;
done:
    return;
}

Boolean fn_004E3980(void* first, void* second, void* third, void* fourth, void* fifth) {
    PAEvalCallActionParams ctx;
    ObjectSet* objects;
    ObjectSet* iter;
    UInt32 count;
    memset(&ctx, 0, sizeof(ctx));
    ctx.proc = first;
    ctx.fnode = second;
    ctx.nd = third;
    ctx.ptf = fifth;
    ctx.map = fourth;
    ctx.stop = FALSE;
    ctx.result = FALSE;
    ctx.visitTarget = FALSE;
    objects = IRO_malloc(sizeof(ObjectSet));
    objects->obj = NULL;
    objects->otherObjs = NULL;
    objects->obj = NULL;
    objects->otherObjs = NULL;
    ctx.result |= fn_004E8B30(objects, first, third, fourth, fifth);
    iter = objects;
    count = 0;
    while (iter && iter->obj) {
        count++;
        iter = iter->otherObjs;
    }
    ctx.targetCount = count;
    for (iter = objects; iter && iter->obj; iter = iter->otherObjs) {
        EvalCallAction(iter->obj, &ctx);
    }
    ObjectSet_RemoveAll(objects);
    objects->obj = NULL;
    objects->otherObjs = NULL;
    IRO_DEBUG_CLEAR(objects, sizeof(ObjectSet));
    IRO_gfree(objects);
    return ctx.result;
}

void fn_004DC3F0(LocationSetSet* source) {
    LocationSetSet* copied = LocationSetSet_New();
    LocationSetSet* cur;
    LocationSetSet_Init(copied);
    for (cur = source; cur && cur->loc; cur = cur->otherLocs) {
        fn_004DC4A0(cur->loc, copied);
    }
    for (cur = copied; cur && cur->loc; cur = cur->otherLocs) {
        LocationSetSet_Remove(source, cur->loc);
    }
    LocationSetSet_RemoveAll(copied);
    copied->loc = NULL;
    copied->otherLocs = NULL;
    copied->count = 0;
    IRO_DEBUG_CLEAR(copied, sizeof(LocationSetSet));
    IRO_gfree(copied);
}

void fn_004E4A90(
    void* first, void* second, IROLinear* third, PointsToFunction* fourth, LocationSet* loc
) {
    PointsToFunction* list;
    PointsToFunction* temp;
    PAVisitContext context;
    LocationSet* ls = loc;
    if (ls != stUnknownLs && ls->block != stUnknownMb && ls->block && ls->block->kind == 2) {
        if (!third) {
            goto fallback;
        }
        list = third->pointsToFunction;
        if (!list) {
            goto fallback;
        }
        goto process;
fallback:
        list = fourth->otherPtes;
process:
        temp = IRO_malloc(sizeof(PointsToFunction));
        temp->pte = NULL;
        temp->otherPtes = NULL;
        temp->pte = NULL;
        temp->otherPtes = NULL;
        memset(&context, 0, sizeof(context));
        context.first = first;
        context.second = second;
        context.third = third;
        context.fourth = fourth;
        context.entries = temp;
        context.loc = ls;
        context.changed = 0;
        for (; list && list->pte; list = list->otherPtes) {
            fn_004E5150(list->pte, &context);
        }
        for (list = temp; list && list->pte; list = list->otherPtes) {
            fn_004E5090(list->pte, &context);
        }
        PointsToFunction_RemoveAll(temp);
        temp->pte = NULL;
        temp->otherPtes = NULL;
        IRO_DEBUG_CLEAR(temp, sizeof(PointsToFunction));
        IRO_gfree(temp);
    }
}
