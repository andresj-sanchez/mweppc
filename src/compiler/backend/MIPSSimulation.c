#include "compiler/CParser.h"
#define CERROR_FILE "MIPSSimulation.c"
#include "compiler/common.h"
#include "compiler/Alias.h"
#include "compiler/PCode.h"
#include "compiler/PCodeInfo.h"
#include "compiler/CompilerTools.h"
#include "compiler/CError.h"
#include "compiler/MIPSSimulation.h"
#include "compiler/Scheduler.h"
#include "compiler/LiveRegisters.h"

SInt32 fn_004BF3E0(PCode* pcode) {
    int kind = opcode_info[pcode->op].unk28;
    int first = kind & 0xFFF0000;
    int low = kind & 0xFFF;
    unsigned int flags;
    switch (first) {
        case 0x1000000:
            return 0x1000000;
        case 0x400000:
            return 0x100000;
        case 0x800000:
            return 0x800000;
        case 0x100000:
            return 0x100000;
        case 0x200000:
            return 0x200000;
        case 0x30000:
            return 0x30000;
        case 0x80000:
            return 0x80000;
        case 0x40000:
            return 0x40000;
        case 0x10000:
            return 0x10000;
        case 0x20000:
            return 0x20000;
    }
    switch (low) {
        case 0:
            flags = pcode->flags;
            if ((flags & 2) || (flags & 4)) {
                return 0x40000;
            }
            if (flags & 1) {
                return 0x80000;
            }
            return 0x30000;
        case 1:
            return 0x40000;
        case 3:
            return 0x100000;
        case 4:
            return 0x200000;
        default:
            CError_FATAL(1812);
            return 0x30000;
        case 2:
            CError_FATAL(1815);
            return 0;
    }
}

int fn_004C0470(PCode* pcode) {
    if (pcode->flags & fPCodeReadsMemory) {
        return 3;
    }
    switch (pcode->op) {
        case 0xB6:
        case 0xB7:
        case 0x400:
        case 0x401:
        case 0x405:
        case 0x406:
        case 0x469:
        case 0x46E:
        case 0x46F:
        case 0x470:
        case 0x471:
        case 0x472:
            return 5;
    }
    return simulation_callback(pcode);
}

void fn_004C04D0(void) {
    PCodeBlock* block;
    gather_alias_info();
    compute_live_registers();
    for (block = pcbasicblocks; block; block = block->nextBlock) {
        if (block->pcodeCount >= 2 && !(block->flags & 0x11B)) {
            fn_004C0520(block);
            block->flags |= 8;
        }
    }
    freeoheap();
}

void fn_004BFD70(void) {
    int i;
    int j;
    simulation_tick++;
    for (i = 0; i < 6; i++) {
        if (simulation_wait_reg[i]) {
            simulation_wait_reg[i]--;
        }
        if (!simulation_wait_reg[i]) {
            simulation_last_reg[i] = simulation_class_use_0_array[i];
            simulation_last_cycle[i] = simulation_tick;
            simulation_class_use_0_array[i] = 0;
        }
    }
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            if (simulation_wait_pair[i][j] > 0) {
                simulation_wait_pair[i][j]--;
            }
            if (!simulation_wait_pair[i][j]) {
                simulation_pair_pending[i][j] = 0;
            }
        }
    }
    for (i = 0; i < 2; i++) {
        if (simulation_wait_special[i] > 0) {
            simulation_wait_special[i]--;
        }
        if (!simulation_wait_special[i]) {
            simulation_special_pending[i] = 0;
        }
    }
}

int fn_004C0340(PCode* pcode) {
    UInt16 opcode = pcode->op;
    switch (opcode) {
        case 0x7C:
        case 0x7D:
        case 0x1D3:
        case 0x1D4:
        case 0x43F:
        case 0x440:
        case 0x441:
        case 0x442:
        case 0x443:
        case 0x474:
        case 0x475:
            return 0x25;
        case 0xB6:
        case 0xB7:
        case 0x1D5:
        case 0x1D6:
        case 0x1D7:
        case 0x1D8:
        case 0x1DB:
        case 0x1DC:
        case 0x469:
        case 0x46E:
        case 0x46F:
        case 0x470:
        case 0x471:
        case 0x472:
            return 4;
        case 0x22C:
        case 0x22D:
        case 0x231:
        case 0x232:
        case 0x233:
        case 0x23F:
        case 0x240:
        case 0x244:
        case 0x245:
        case 0x246:
            return 4;
        case 0x21C:
        case 0x21D:
        case 0x21E:
            return 0x25;
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            return 2;
    }
    if (opcode > 0x21E && opcode <= 0x2E7) {
        int result;
        switch (opcode) {
            case 0x27C:
            case 0x2D8:
                result = 7;
                break;
            case 0x281:
            case 0x282:
            case 0x283:
            case 0x288:
            case 0x28D:
            case 0x2D2:
            case 0x2D5:
                result = 1;
                break;
            case 0x2D4:
                result = 13;
                break;
            case 0x2E7:
                result = 3;
                break;
            default:
                result = 4;
                break;
        }
        return result;
    }
    return fn_004C0050(pcode);
}

void fn_004BFE60(PCode* pcode, PCode* delay) {
    PCodeArg* other;
    PCodeArg* arg;
    int j;
    int i;
    if (!(delay->flags & 0x20000000)) {
        pcode->flags |= 0x2000000;
        return;
    }
    if (pcode->flags & 2 || (opcode_info[pcode->op].flags2 & 0x5000)) {
        if (copts.unknown0C < 2) {
            pcode->flags |= 0x2000000;
            return;
        }
    }
    if (opcode_info[delay->op].flags & 8) {
        i = 0;
        arg = pcode->args;
        for (; i < pcode->argCount; i++, arg++) {
            if (arg->kind == 0 && arg->arg == 0) {
                if (arg->data.reg.reg == 31) {
                    pcode->flags |= 0x2000000;
                    return;
                }
                if (delay->op == 0x3B &&
                    delay->args[0].data.reg.reg == arg->data.reg.reg &&
                    (arg->effect & 2)) {
                    pcode->flags |= 0x2000000;
                    return;
                }
                if (delay->op == 0x3D &&
                    delay->args[1].data.reg.reg == arg->data.reg.reg &&
                    (arg->effect & 2)) {
                    pcode->flags |= 0x2000000;
                    return;
                }
            }
        }
        return;
    } else if (delay->op != PC_J && delay->op != PC_B) {
        i = 0;
        arg = pcode->args;
        for (; i < pcode->argCount; i++, arg++) {
            if (arg->kind == 0 && (arg->effect & 2)) {
                other = delay->args;
                for (j = 0; j < delay->argCount; j++, other++) {
                    if (other->kind == 0 &&
                        other->arg == arg->arg &&
                        other->data.reg.reg == arg->data.reg.reg) {
                        pcode->flags |= 0x2000000;
                        return;
                    }
                }
            }
        }
    }
}

int fn_004C0050(PCode* pcode) {
    switch (pcode->op) {
        case 0x7C:
        case 0x7D:
        case 0x43F:
        case 0x441:
        case 0x442:
        case 0x443:
        case 0x474:
        case 0x475:
            return 36;
        case 0xB6:
        case 0xB7:
        case 0x400:
        case 0x401:
        case 0x405:
        case 0x406:
        case 0x469:
        case 0x46E:
        case 0x46F:
        case 0x470:
        case 0x471:
        case 0x472:
            return 3;
        case 0x7A:
        case 0x7B:
        case 0x43B:
        case 0x43C:
        case 0x43D:
        case 0x43E:
        case 0x44A:
        case 0x44B:
            return 68;
        case 0x80:
        case 0x81:
        case 0x444:
        case 0x445:
        case 0x446:
        case 0x447:
        case 0x448:
        case 0x449:
            return 6;
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
            return 2;
        case 0xEC:
        case 0xED:
        case 0x102:
        case 0x103:
            return 4;
        case 0x6C:
            return 6;
        case 0x6D:
            return 2;
        case 0x6E:
        case 0x6F:
        case 0x70:
            return 6;
        case 0x71:
            return 3;
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
            return 6;
        case 0xF2:
            return 59;
        case 0xF3:
            return 31;
        case 0xB4:
            return 6;
        case 0xB5:
            return 5;
        case 0xEE:
        case 0xEF:
        case 0xF0:
        case 0xF1:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
        case 0x104:
        case 0x105:
        case 0x106:
        case 0x107:
            return 6;
        case 0x100:
            return 59;
        case 0x101:
            return 31;
        case 0x112:
        case 0x113:
        case 0x114:
        case 0x115:
        case 0x116:
        case 0x117:
        case 0x118:
        case 0x119:
        case 0x11A:
        case 0x11B:
            return 2;
        case 0x10F:
        case 0x11D:
        case 0x11F:
        case 0x121:
            return 9;
        case 0x10E:
        case 0x11C:
        case 0x11E:
        case 0x120:
            return 10;
        case 0x123:
            return 31;
        case 0x122:
            return 59;
        case 0x125:
            return 61;
        case 0x124:
            return 121;
        case 0x174:
        case 0x175:
        case 0x176:
        case 0x46A:
        case 0x46B:
            return 3;
        case 0x17C:
        case 0x17D:
        case 0x17E:
        case 0x17F:
        case 0x180:
        case 0x181:
            return 3;
        case 0x184:
        case 0x185:
            return 34;
        case 0x182:
        case 0x183:
            return 68;
        default:
            return 1;
    }
}

void fn_004BFC00(void) {
    int i;
    int j;
    simulation_tick = 0;
    simulation_class_use_0_array[0] = 0;
    simulation_wait_reg[0] = 0;
    simulation_last_reg[0] = 0;
    simulation_last_cycle[0] = 0;
    simulation_class_use_0_array[1] = 0;
    simulation_wait_reg[1] = 0;
    simulation_last_reg[1] = 0;
    simulation_last_cycle[1] = 0;
    simulation_class_use_0_array[2] = 0;
    simulation_wait_reg[2] = 0;
    simulation_last_reg[2] = 0;
    simulation_last_cycle[2] = 0;
    simulation_class_use_0_array[3] = 0;
    simulation_wait_reg[3] = 0;
    simulation_last_reg[3] = 0;
    simulation_last_cycle[3] = 0;
    simulation_class_use_0_array[4] = 0;
    simulation_wait_reg[4] = 0;
    simulation_last_reg[4] = 0;
    simulation_last_cycle[4] = 0;
    simulation_class_use_0_array[5] = 0;
    simulation_wait_reg[5] = 0;
    simulation_last_reg[5] = 0;
    simulation_last_cycle[5] = 0;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            simulation_wait_pair[i][j] = 0;
            simulation_pair_pending[i][j] = NULL;
        }
    }
    simulation_wait_special[0] = 0;
    simulation_special_pending[0] = 0;
    simulation_wait_special[1] = 0;
    simulation_special_pending[1] = 0;
    simulation_callback = fn_004C0340;
}

int fn_004BF530(PCode* pcode) {
    if (fn_004BF3E0(pcode) == 0x30000) {
        if (simulation_class_use_0_array[0] && simulation_class_use_0_array[1]) {
            return 0;
        }
    } else if (fn_004BF3E0(pcode) == 0x10000) {
        unsigned int flags = opcode_info[pcode->op].unk28;
        if (!(flags & 0x10000) || !(flags & 0x10000000)) {
            if (!(flags & 0x10000) || !(flags & 0x20000000)) {
            } else {
                if (simulation_special_pending[0]) {
                    return 0;
                }
            }
            if (simulation_class_use_0_array[0]) {
                return 0;
            }
        } else {
            if (simulation_pair_pending[0][0] && simulation_pair_pending[0][1]) {
                return 0;
            }
            if (simulation_pair_pending[0][0] &&
                !simulation_pair_pending[0][1] &&
                simulation_wait_pair[0][0] > 2) {
                return 0;
            }
            if (!simulation_pair_pending[0][0] &&
                simulation_pair_pending[0][1] &&
                simulation_wait_pair[0][1] > 2) {
                return 0;
            }
            if (simulation_class_use_0_array[0]) {
                return 0;
            }
        }
    } else if (fn_004BF3E0(pcode) == 0x20000) {
        unsigned int flags = opcode_info[pcode->op].unk28;
        if (!(flags & 0x20000) || !(flags & 0x10000000)) {
            if (!(flags & 0x20000) || !(flags & 0x20000000)) {
                if (simulation_class_use_0_array[1]) {
                    return 0;
                }
            } else {
                if (simulation_special_pending[1]) {
                    return 0;
                }
                if (simulation_class_use_0_array[1]) {
                    return 0;
                }
            }
        } else {
            if (simulation_pair_pending[1][0] && simulation_pair_pending[1][1]) {
                return 0;
            }
            if (simulation_pair_pending[1][0] &&
                !simulation_pair_pending[1][1] &&
                simulation_wait_pair[1][0] > 2) {
                return 0;
            }
            if (!simulation_pair_pending[1][0] &&
                simulation_pair_pending[1][1] &&
                simulation_wait_pair[1][1] > 2) {
                return 0;
            }
            if (simulation_class_use_0_array[1]) {
                return 0;
            }
        }
    } else if (fn_004BF3E0(pcode) == 0x40000) {
        if (simulation_class_use_0_array[2]) {
            return 0;
        }
    } else if (fn_004BF3E0(pcode) == 0x80000) {
        if (simulation_class_use_0_array[3]) {
            return 0;
        }
    } else if (fn_004BF3E0(pcode) == 0x100000) {
        if (simulation_class_use_0_array[0] || simulation_class_use_0_array[4]) {
            return 0;
        }
    } else if (fn_004BF3E0(pcode) == 0x200000) {
        if (simulation_class_use_0_array[0] || simulation_class_use_0_array[5]) {
            return 0;
        }
    } else if (fn_004BF3E0(pcode) == 0x400000) {
        if (simulation_class_use_0_array[2] || simulation_class_use_0_array[4]) {
            return 0;
        }
    } else if (fn_004BF3E0(pcode) == 0x800000) {
        if (simulation_class_use_0_array[2] || simulation_class_use_0_array[5]) {
            return 0;
        }
    } else if (fn_004BF3E0(pcode) == 0x1000000) {
        if (simulation_class_use_0_array[0] || simulation_class_use_0_array[1]) {
            return 0;
        }
    }
    return 1;
}

void fn_004BF840(PCode* pcode) {
    if (fn_004BF3E0(pcode) == 0x30000) {
        if (simulation_class_use_0_array[0]) {
            simulation_class_use_0_array[1] = pcode;
            simulation_wait_reg[1] = simulation_callback(pcode);
        } else {
            simulation_class_use_0_array[0] = pcode;
            simulation_wait_reg[0] = simulation_callback(pcode);
        }
    } else if (fn_004BF3E0(pcode) == 0x10000) {
        UInt32 flags = opcode_info[pcode->op].unk28;
        if ((flags & 0x10000) && (flags & 0x10000000)) {
            if (simulation_class_use_0_array[0]) {
                CError_ASSERT(1488, FALSE);
            } else {
                simulation_class_use_0_array[0] = pcode;
                simulation_wait_reg[0] = 1;
                if (simulation_pair_pending[0][0]) {
                    simulation_pair_pending[0][1] = pcode;
                    simulation_wait_pair[0][1] = simulation_callback(pcode);
                } else {
                    simulation_pair_pending[0][0] = pcode;
                    simulation_wait_pair[0][0] = simulation_callback(pcode);
                }
            }
        } else {
            if (!(flags & 0x10000) || !(flags & 0x20000000)) {
                simulation_class_use_0_array[0] = pcode;
                simulation_wait_reg[0] = simulation_callback(pcode);
            } else {
                simulation_class_use_0_array[0] = pcode;
                simulation_wait_reg[0] = 1;
                simulation_wait_special[0] = simulation_callback(pcode);
            }
        }
    } else if (fn_004BF3E0(pcode) == 0x20000) {
        UInt32 flags = opcode_info[pcode->op].unk28;
        if ((flags & 0x20000) && (flags & 0x10000000)) {
            if (simulation_class_use_0_array[1]) {
                CError_ASSERT(1517, FALSE);
            } else {
                simulation_class_use_0_array[1] = pcode;
                simulation_wait_reg[1] = 1;
                if (simulation_pair_pending[1][0]) {
                    simulation_pair_pending[1][1] = pcode;
                    simulation_wait_pair[1][1] = simulation_callback(pcode);
                } else {
                    simulation_pair_pending[1][0] = pcode;
                    simulation_wait_pair[1][0] = simulation_callback(pcode);
                }
            }
        } else {
            if (!(flags & 0x20000) || !(flags & 0x20000000)) {
                simulation_class_use_0_array[1] = pcode;
                simulation_wait_reg[1] = simulation_callback(pcode);
            } else {
                simulation_class_use_0_array[1] = pcode;
                simulation_wait_reg[1] = 1;
                simulation_wait_special[1] = simulation_callback(pcode);
            }
        }
    } else if (fn_004BF3E0(pcode) == 0x40000) {
        simulation_class_use_0_array[2] = pcode;
        simulation_wait_reg[2] = simulation_callback(pcode);
    } else if (fn_004BF3E0(pcode) == 0x80000) {
        simulation_class_use_0_array[3] = pcode;
        simulation_wait_reg[3] = simulation_callback(pcode);
    } else if (fn_004BF3E0(pcode) == 0x100000) {
        simulation_class_use_0_array[0] = pcode;
        simulation_wait_reg[0] = 1;
        simulation_class_use_0_array[4] = pcode;
        simulation_wait_reg[4] = simulation_callback(pcode);
    } else if (fn_004BF3E0(pcode) == 0x200000) {
        simulation_class_use_0_array[0] = pcode;
        simulation_wait_reg[0] = 1;
        simulation_class_use_0_array[5] = pcode;
        simulation_wait_reg[5] = simulation_callback(pcode);
    } else if (fn_004BF3E0(pcode) == 0x400000) {
        simulation_class_use_0_array[2] = pcode;
        simulation_wait_reg[2] = simulation_callback(pcode);
        simulation_class_use_0_array[4] = pcode;
        simulation_wait_reg[4] = simulation_callback(pcode);
    } else if (fn_004BF3E0(pcode) == 0x800000) {
        simulation_class_use_0_array[2] = pcode;
        simulation_wait_reg[2] = simulation_callback(pcode);
        simulation_class_use_0_array[5] = pcode;
        simulation_wait_reg[5] = simulation_callback(pcode);
    } else if (fn_004BF3E0(pcode) == 0x1000000) {
        simulation_class_use_0_array[0] = pcode;
        simulation_wait_reg[0] = simulation_callback(pcode);
        simulation_class_use_0_array[1] = pcode;
        simulation_wait_reg[1] = simulation_callback(pcode);
    }
}
