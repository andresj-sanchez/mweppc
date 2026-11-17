#include "compiler/IroSubable.h"
#include "compiler/enode.h"

void IRO_InitializeIsSubableOpArray(void) {
    int i;

    for (i = 0; i < 0x7C; i++) {
        register_use_flags_61B910[i] = FALSE;
    }

    register_use_flags_61B910[EPOSTINC] = FALSE;
    register_use_flags_61B910[EPOSTDEC] = FALSE;
    register_use_flags_61B910[EPREINC] = FALSE;
    register_use_flags_61B910[EPREDEC] = FALSE;
    register_use_flags_61B910[EINDIRECT] = FALSE;
    register_use_flags_61B910[EMONMIN] = TRUE;
    register_use_flags_61B910[EBINNOT] = TRUE;
    register_use_flags_61B910[ELOGNOT] = TRUE;
    register_use_flags_61B910[EFORCELOAD] = FALSE;
    register_use_flags_61B910[EMUL] = TRUE;
    register_use_flags_61B910[EMULV] = TRUE;
    register_use_flags_61B910[EDIV] = TRUE;
    register_use_flags_61B910[EMODULO] = TRUE;
    register_use_flags_61B910[EADDV] = TRUE;
    register_use_flags_61B910[ESUBV] = TRUE;
    register_use_flags_61B910[EADD] = TRUE;
    register_use_flags_61B910[ESUB] = TRUE;
    register_use_flags_61B910[ESHL] = TRUE;
    register_use_flags_61B910[ESHR] = TRUE;
    register_use_flags_61B910[ELESS] = FALSE;
    register_use_flags_61B910[EGREATER] = FALSE;
    register_use_flags_61B910[ELESSEQU] = FALSE;
    register_use_flags_61B910[EGREATEREQU] = FALSE;
    register_use_flags_61B910[EEQU] = FALSE;
    register_use_flags_61B910[ENOTEQU] = FALSE;
    register_use_flags_61B910[EAND] = TRUE;
    register_use_flags_61B910[EXOR] = TRUE;
    register_use_flags_61B910[EOR] = TRUE;
    register_use_flags_61B910[ELAND] = FALSE;
    register_use_flags_61B910[ELOR] = FALSE;
    register_use_flags_61B910[EASS] = FALSE;
    register_use_flags_61B910[EMULASS] = FALSE;
    register_use_flags_61B910[EDIVASS] = FALSE;
    register_use_flags_61B910[EMODASS] = FALSE;
    register_use_flags_61B910[EADDASS] = FALSE;
    register_use_flags_61B910[ESUBASS] = FALSE;
    register_use_flags_61B910[ESHLASS] = FALSE;
    register_use_flags_61B910[ESHRASS] = FALSE;
    register_use_flags_61B910[EANDASS] = FALSE;
    register_use_flags_61B910[EXORASS] = FALSE;
    register_use_flags_61B910[EORASS] = FALSE;
    register_use_flags_61B910[ECOMMA] = FALSE;
    register_use_flags_61B910[EMIN] = TRUE;
    register_use_flags_61B910[EMAX] = TRUE;
    register_use_flags_61B910[0x2C] = FALSE;
    register_use_flags_61B910[0x2D] = FALSE;
    register_use_flags_61B910[0x2E] = FALSE;
    register_use_flags_61B910[0x2F] = FALSE;
    register_use_flags_61B910[0x30] = FALSE;
    register_use_flags_61B910[0x31] = FALSE;
    register_use_flags_61B910[0x32] = FALSE;
    register_use_flags_61B910[0x33] = FALSE;
    register_use_flags_61B910[0x34] = FALSE;
    register_use_flags_61B910[0x35] = FALSE;
    register_use_flags_61B910[0x36] = FALSE;
    register_use_flags_61B910[0x37] = FALSE;
    register_use_flags_61B910[0x38] = FALSE;
    register_use_flags_61B910[0x39] = FALSE;
    register_use_flags_61B910[0x3A] = FALSE;
    register_use_flags_61B910[0x3B] = FALSE;
    register_use_flags_61B910[0x3C] = FALSE;
    register_use_flags_61B910[0x3D] = FALSE;
    register_use_flags_61B910[0x3E] = FALSE;
    register_use_flags_61B910[0x3F] = FALSE;
    register_use_flags_61B910[0x40] = FALSE;
    register_use_flags_61B910[0x41] = FALSE;
    register_use_flags_61B910[0x42] = FALSE;
    register_use_flags_61B910[0x43] = FALSE;
    register_use_flags_61B910[0x44] = FALSE;
    register_use_flags_61B910[0x45] = FALSE;
    register_use_flags_61B910[0x46] = FALSE;
    register_use_flags_61B910[0x47] = FALSE;
    register_use_flags_61B910[0x48] = FALSE;
    register_use_flags_61B910[0x49] = FALSE;
    register_use_flags_61B910[0x4A] = FALSE;
    register_use_flags_61B910[0x4B] = FALSE;
    register_use_flags_61B910[0x4C] = FALSE;
    register_use_flags_61B910[0x4D] = FALSE;
    register_use_flags_61B910[0x4E] = FALSE;
    register_use_flags_61B910[0x4F] = FALSE;
    register_use_flags_61B910[0x50] = FALSE;
    register_use_flags_61B910[0x51] = FALSE;
    register_use_flags_61B910[0x52] = FALSE;
}
