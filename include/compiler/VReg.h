#ifndef COMPILER_VREG_H
#define COMPILER_VREG_H

#include "compiler/common.h"
#include "compiler/CInt64.h"
#include "compiler/PCode.h"
#include "compiler/InstrSelection.h"

extern Operand operand_from_pcode_arg(PCodeArg arg); //! @reconstructed
extern PCodeArg arg_from_label(CLabel* label); //! @reconstructed
extern PCodeArg arg_from_operand(Operand* op); //! @reconstructed
extern PCodeArg arg_from_cint64(CInt64 value); //! @reconstructed
extern PCodeArg arg_from_int(SInt32 value); //! @reconstructed
extern PCodeArg new_register_arg(SInt8 kind); //! @reconstructed
extern Operand make_register_operand(int reg, int kind); //! @reconstructed
extern PCodeArg fn_004D02D0(int reg, int kind, int effect);

extern Boolean fn_004D0330(IROLinear* expr);

extern Boolean (*iro_reg_callback_6356E8)(IROLinear*);
extern Boolean IRO_IsAddressMultiplyDefault(void);

#endif
