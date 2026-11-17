#ifndef COMPILER_CINIT_H
#define COMPILER_CINIT_H

#include "compiler/common.h"

struct OLinkList {
    OLinkList* next;
    Object* obj;
    SInt32 offset;
    SInt32 addend;
};

extern void CInit_RewriteStringConst(ENode* expr); //! @reconstructed
extern void fn_00512B20(void);
extern void fn_00519620(void);

extern void fn_00512E20(void);

extern void CInit_ExportConst(Object* object);

extern unsigned int cinit_state_61BC7C;
extern unsigned int cinit_state_61BC8C;
extern unsigned int cinit_state_61BC90;
extern unsigned int cinit_state_61BC94;
extern unsigned int cinit_state_61BC98;
extern unsigned int cinit_state_636138;
extern unsigned int cinit_state_636D4C;

extern void CInit_RewriteString(ENode*, SInt32);

#endif
