#ifndef COMPILER_PCODEINFO_H
#define COMPILER_PCODEINFO_H

#include "compiler/common.h"
#include <stdarg.h>

#define PC_ADDIU ((UInt16)0x03)
#define PC_ADDU ((UInt16)0x04)
#define PC_AND ((UInt16)0x06)
#define PC_BC1F ((UInt16)0x0C)
#define PC_BC1T ((UInt16)0x0E)
#define PC_BEQ ((UInt16)0x18)
#define PC_BEQZ ((UInt16)0x1A)
#define PC_BGEZ ((UInt16)0x1C)
#define PC_BNE ((UInt16)0x28)
#define PC_BNEZ ((UInt16)0x2A)
#define PC_BAL ((UInt16)0x38)
#define PC_B ((UInt16)0x39)
#define PC_J ((UInt16)0x3A)
#define PC_JR ((UInt16)0x3B)
#define PC_JAL ((UInt16)0x3C)
#define PC_JALR ((UInt16)0x3D)
#define PC_BREAKC ((UInt16)0x3E)
#define PC_C_EQ_D ((UInt16)0x44)
#define PC_C_EQ_S ((UInt16)0x45)
#define PC_C_LE_D ((UInt16)0x48)
#define PC_C_LE_S ((UInt16)0x49)
#define PC_C_LT_D ((UInt16)0x4A)
#define PC_C_LT_S ((UInt16)0x4B)
#define PC_DADDIU ((UInt16)0x78)
#define PC_DDIV ((UInt16)0x7A)
#define PC_DMFC1 ((UInt16)0x7F)
#define PC_DDIVU ((UInt16)0x7B)
#define PC_DIV ((UInt16)0x7C)
#define PC_DIVU ((UInt16)0x7D)
#define PC_DNEGU ((UInt16)0x83)
#define PC_DSLL ((UInt16)0x84)
#define PC_DSLL32 ((UInt16)0x85)
#define PC_DSLLV ((UInt16)0x86)
#define PC_DSRA ((UInt16)0x87)
#define PC_DSRA32 ((UInt16)0x88)
#define PC_DSRAV ((UInt16)0x89)
#define PC_DSRL ((UInt16)0x8A)
#define PC_DSRL32 ((UInt16)0x8B)
#define PC_DSRLV ((UInt16)0x8C)
#define PC_LUI ((UInt16)0x90)
#define PC_MFC1 ((UInt16)0xA5)
#define PC_MTC1 ((UInt16)0xAD)
#define PC_DMTC1 ((UInt16)0xB1)
#define PC_MFHI ((UInt16)0xA8)
#define PC_MFLO ((UInt16)0xA9)
#define PC_MUL_D ((UInt16)0xB4)
#define PC_MUL_S ((UInt16)0xB5)
#define PC_MULT ((UInt16)0xB6)
#define PC_MULTU ((UInt16)0xB7)
#define PC_NEGU ((UInt16)0xB9)
#define PC_NOP ((UInt16)0xBA)
#define PC_OR ((UInt16)0xBE)
#define PC_ORI ((UInt16)0xBF)
#define PC_SLL ((UInt16)0xD2)
#define PC_SLLV ((UInt16)0xD3)
#define PC_SLTIU ((UInt16)0xD6)
#define PC_SRA ((UInt16)0xD8)
#define PC_SRAV ((UInt16)0xD9)
#define PC_SRL ((UInt16)0xDA)
#define PC_SRLV ((UInt16)0xDB)
#define PC_SUBU ((UInt16)0xDD)
#define PC_XOR ((UInt16)0xE4)
#define PC_LA ((UInt16)0x44E)
#define PC_END ((UInt16)0x4A6)
#define PC_XORI ((UInt16)0xE5)
#define PC_DIV_D ((UInt16)0xF2)
#define PC_DIV_S ((UInt16)0xF3)
#define PC_RECIP_D ((UInt16)0x122)
#define PC_RECIP_S ((UInt16)0x123)

#define PC_B_38A ((UInt16)0x38A) //! @reconstructed
#define PC_B_38B ((UInt16)0x38B) //! @reconstructed

#define PC_ANDI3 ((UInt16)0x40C) //! @reconstructed
#define PC_DDIVU3 ((UInt16)0x43D) //! @reconstructed
#define PC_DIVU3 ((UInt16)0x442) //! @reconstructed
#define PC_EXT3 ((UInt16)0x48B) //! @reconstructed
#define PC_EXTU3 ((UInt16)0x48C) //! @reconstructed
#define PC_ORI3 ((UInt16)0x473) //! @reconstructed
#define PC_XORI3 ((UInt16)0x4A1) //! @reconstructed

struct OpcodeInfo {
    const char* name;
    const char* unk4;
    const char* format;
    union {
        SInt32 unkC;
        struct {
            UInt8 operandCount;
            UInt8 instructionWidth;
            UInt16 unkE;
        };
    };
    UInt32 encoding;
    SInt32 unk14;
    UInt32 flags;
    UInt32 flags2;
    const char* unk20;
    SInt32 unk24;
    SInt32 unk28;
    UInt16 field2C;
    UInt16 unk2E;
};
extern OpcodeInfo opcode_info[]; //! @reconstructed

#define OPCODE_IS_CALL 8 //! @reconstructed

extern PCode* vformatpcode(UInt16 op, va_list list);
extern PCodeArg* get_label_operand(PCode* pcode); //! @reconstructed
extern short get_nop_opcode(void); //! @reconstructed

extern int nbytes_loaded_or_stored_by(UInt16 opcode);
extern PCode* makecopyinstruction(PCodeArg* src, PCodeArg* dst);
extern int is_location_independent(PCode* pcode);
extern int can_reuse_stored_value(PCode* load, PCode* store);
extern PCode* makecopyforload(int kind, PCodeArg* src, PCodeArg* dst);

extern void fn_0049BA60(void);

#endif
