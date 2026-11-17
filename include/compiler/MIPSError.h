#ifndef COMPILER_MIPSERROR_H
#define COMPILER_MIPSERROR_H

#include "compiler/common.h"

enum {
    MIPSERR_MAX = 35
};

extern void MIPSError_Error(int code, ...); //! @reconstructed
extern void MIPSError_Warning(int code, ...); //! @reconstructed

#endif
