// Source-only/native data: the back end's expression dispatch (data_005D0140,
// 3.0.1 0x5D0140, b145 0x5E88A0), indexed by node type. The copied back end
// is 3.0.1's, whose ENodeType has EMIN/EMAX and ESCOPEBEGIN..ESTMT that GC's
// (b69's) lacks, in another order: this is b145's table reordered by node
// name for b69's enum. The b69-only kinds (EOBJLIST, EVECTOR128CONST) take
// the table's default handler, func_00494940, as b69's own table (0x53C6A0,
// which has the same groups of equal entries) does.
#include "compiler/common.h"
#include "compiler/enode.h"

extern void func_00493B50(void);
extern void func_00493BB0(void);
extern void func_00493D90(void);
extern void func_00493EF0(void);
extern void func_00493F90(void);
extern void func_00494940(void);
extern void func_00494960(void);
extern void func_00494AA0(void);
extern void func_00494AE0(void);
extern void func_004951D0(void);
extern void func_004951F0(void);
extern void func_00495410(void);
extern void func_004954B0(void);
extern void func_00495500(void);
extern void func_00496C70(void);
extern void func_00496CB0(void);
extern void func_00496D50(void);
extern void func_00497360(void);
extern void func_004973C0(void);
extern void func_00497420(void);
extern void func_00497A90(void);
extern void func_00497E50(void);
extern void func_004980C0(void);
extern void func_00498590(void);
extern void func_00499200(void);
extern void func_00499820(void);
extern void func_0049A740(void);
extern void func_0049AEF0(void);
extern void func_0049AF50(void);
extern void func_0049B1D0(void);
extern void func_0049B770(void);
extern void func_0049B870(void);
extern void func_0049B8D0(void);
extern void func_0049C160(void);
extern void func_0049C1E0(void);

// b52's ENodeType is 3.0.1's (enode.h): b145's table in its own order (the
// kinds b69's list leaves out, EMIN/EMAX, ESCOPEBEGIN/END, EPOINTERSTAR,
// EDOTSTAR, ECTORINIT and ESTMT, are b145's default, func_00494940)
void (*data_005D0140[MAXEXPR])(void) = {
    func_0049B8D0, // EPOSTINC (0)
    func_0049B8D0, // EPOSTDEC (1)
    func_0049B870, // EPREINC (2)
    func_0049B870, // EPREDEC (3)
    func_0049B770, // EINDIRECT (4)
    func_0049B1D0, // EMONMIN (5)
    func_0049AF50, // EBINNOT (6)
    func_00493EF0, // ELOGNOT (7)
    func_0049AEF0, // EFORCELOAD (8)
    func_0049A740, // EMUL (9)
    func_00494940, // EMULV (10)
    func_00499820, // EDIV (11)
    func_00499200, // EMODULO (12)
    func_00494940, // EADDV (13)
    func_00494940, // ESUBV (14)
    func_00498590, // EADD (15)
    func_004980C0, // ESUB (16)
    func_00497E50, // ESHL (17)
    func_00497A90, // ESHR (18)
    func_00493F90, // ELESS (19)
    func_00493F90, // EGREATER (20)
    func_00493F90, // ELESSEQU (21)
    func_00493F90, // EGREATEREQU (22)
    func_00493F90, // EEQU (23)
    func_00493F90, // ENOTEQU (24)
    func_00497420, // EAND (25)
    func_004973C0, // EXOR (26)
    func_00497360, // EOR (27)
    func_00493D90, // ELAND (28)
    func_00493D90, // ELOR (29)
    func_00496D50, // EASS (30)
    func_00494940, // EMULASS (31)
    func_00494940, // EDIVASS (32)
    func_00494940, // EMODASS (33)
    func_00494940, // EADDASS (34)
    func_00494940, // ESUBASS (35)
    func_00494940, // ESHLASS (36)
    func_00494940, // ESHRASS (37)
    func_00494940, // EANDASS (38)
    func_00494940, // EXORASS (39)
    func_00494940, // EORASS (40)
    func_00496C70, // ECOMMA (41)
    func_00494940, // EMIN (42)
    func_00494940, // EMAX (43)
    func_00494940, // EPMODULO (44)
    func_00494940, // EROTL (45)
    func_00494940, // EROTR (46)
    func_00494940, // EBCLR (47)
    func_00494940, // EBTST (48)
    func_00494940, // EBSET (49)
    func_00495500, // ETYPCON (50)
    func_004954B0, // EBITFIELD (51)
    func_00495410, // EINTCONST (52)
    func_004951F0, // EFLOATCONST (53)
    func_004951D0, // ESTRINGCONST (54)
    func_00494AE0, // ECOND (55)
    func_00494AA0, // EFUNCCALL (56)
    func_00494940, // EFUNCCALLP (57)
    func_00494960, // EOBJREF (58)
    func_00493BB0, // ENULLCHECK (59)
    func_00493B50, // EPRECOMP (60)
    func_00494940, // ELABEL (61)
    func_00494940, // ESCOPEBEGIN (62)
    func_00494940, // ESCOPEEND (63)
    func_00494940, // EMFPOINTER (64)
    func_00494940, // ETEMP (65)
    func_00494940, // ELOCOBJ (66)
    func_00494940, // EARGOBJ (67)
    func_00494940, // ESETCONST (68)
    func_00494940, // ENEWEXCEPTION (69)
    func_00494940, // ENEWEXCEPTIONARRAY (70)
    func_00494940, // EINITTRYCATCH (71)
    func_00494940, // EMEMBER (72)
    func_00494940, // ETEMPLDEP (73)
    func_00494940, // EPOINTERSTAR (74)
    func_00494940, // EDOTSTAR (75)
    func_00494940, // ECTORINIT (76)
    func_00494940, // ESTMT (77)
    func_00494940, // EINSTRUCTION (78)
    func_0049C1E0, // EDEFINE (79)
    func_0049C160, // EREUSE (80)
    func_00496CB0, // EASSBLK (81)
    func_00494940, // ECONDASS (82)
};
