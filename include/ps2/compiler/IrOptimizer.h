#ifndef COMPILER_IROPTIMIZER_H
#define COMPILER_IROPTIMIZER_H

#include "compiler/common.h"
#include "compiler/CParser.h"

extern SInt8 unrollinstrfactor;
extern void IRO_Cleanup(void); //! @reconstructed
extern void fn_004D0C80(void);

extern UInt8 IRO_IsSetup_61B991;

extern void IRO_Setup(void* callbacks);

extern Statement* IRO_Optimizer(Object* obj, Statement* stmt);

#endif
