#include "compiler/IrOptimizer.h"
#include "compiler/common.h"
#include "compiler/CParser.h"
#include "compiler/IroDump.h"

void IRO_Cleanup(void) {
    if (IRO_IsSetup_61B991) {
        IRO_CleanupDump();
        IRO_IsSetup_61B991 = FALSE;
    }
}

void fn_004D0C80(void) {
    copts.unknownF6 = 0;
    copts.opt_prelinearize = 1;
    copts.opt_generateconditionalassignments = 0;
    copts.opt_alwaysdumpopts = 0;
    copts.opt_movepostops = 0;
    copts.opt_findoptimalunrollfactor = 1;
    copts.opt_findoptimalvectorunrollfactor = 1;
    copts.opt_unrollpostloop = 1;
    copts.opt_unrollvectorpostloop = 1;
    copts.opt_createvectortemps = 1;
    copts.opt_unpromotetypes = 0;
    copts.opt_dumptypes = 0;
    copts.opt_scalarizebitfields = 1;
    copts.opt_propagatemultiplydivideassignments = 1;
    copts.opt_moveinvariantsinaddressexpr = 1;
    copts.opt_convertfpconsts = 1;
    copts.opt_markcounterloops = 0;
    copts.opt_markvariablecounterloops = 0;
    copts.opt_pulloutconstants = 0;
    copts.opt_defineargorder = 0;
    copts.unknown11B = 0;
    copts.unknown11C = 0;
    copts.unknown11D = 0;
    copts.unknown11E = 0;
    copts.unknown11F = 0;
    copts.unknown120 = 0;
    copts.unknown121 = 1;
    copts.unknown122 = 0;
    copts.unknown123 = 0;
    copts.unknown124 = 1;
    copts.unknown125 = 0;
    copts.unknown126 = 0;
    copts.unknown127 = 1;
    copts.unknown129 = 0;
    copts.unknown12D = 0;
    copts.opt_branchlessexpensivethandiv = 0;
    copts.strengthreductionstrict = 1;
    copts.unknown12C = ((SInt8)copts.optimizationlevel >= 1);
    copts.unknown12F = copts.unknown12C;
    copts.unknown12B = copts.unknown12F;
    copts.opt_rebuildlogicals = copts.unknown12B;
    copts.opt_rebuildconditionals = copts.opt_rebuildlogicals;
    copts.opt_arithtransformation = copts.opt_rebuildconditionals;
    copts.opt_treetransformation = copts.opt_arithtransformation;
    copts.opt_scalarize = copts.opt_treetransformation;
    copts.deadcode = copts.opt_scalarize;
    copts.propagation = ((SInt8)copts.optimizationlevel >= 2);
    copts.unknown128 = copts.propagation;
    copts.commonsubs = copts.unknown128;
    copts.deadstore = copts.commonsubs;
    copts.vectorizeloops = ((SInt8)copts.optimizationlevel >= 3);
    copts.unknown132 = copts.vectorizeloops;
    copts.unknown131 = copts.unknown132;
    copts.unknown130 = copts.unknown131;
    copts.unrolling = copts.unknown130;
    copts.lifetimes = copts.unrolling;
    copts.strengthreduction = copts.lifetimes;
    copts.loopinvariants = copts.strengthreduction;
    copts.opt_removeemptyloops = copts.loopinvariants;
    copts.opt_optimizenonregaccess = copts.opt_removeemptyloops;
    copts.opt_rotateloops = copts.opt_optimizenonregaccess;
    copts.multiplepasses = ((SInt8)copts.optimizationlevel >= 4);
}
