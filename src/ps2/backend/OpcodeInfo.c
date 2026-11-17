#include "compiler/OpcodeInfo.h"
#include "compiler/mips.h"
#include "compiler/PCodeInfo.h"
#include "compiler/CodeGen.h"
#include <string.h>
#include "compiler/CParser.h"

static char opcode_strings[] = "\000"
                               "InternalError\000"
                               "__I_add\000"
                               "=d,s,t\000"
                               "add\000"
                               "__I_addu\000"
                               "addu\000"
                               "__I_and\000"
                               "and\000"
                               "__I_nor\000"
                               "nor\000"
                               "__I_or\000"
                               "or\000"
                               "__I_xor\000"
                               "xor\000"
                               "__I_sub\000"
                               "sub\000"
                               "__I_subu\000"
                               "subu\000"
                               "__I_not\000"
                               "=d,s\000"
                               "not\000"
                               "__I_move\000"
                               "move\000"
                               "__I_slt\000"
                               "slt\000"
                               "__I_sltu\000"
                               "sltu\000"
                               "+d,t\000"
                               "+d\000"
                               "__I_neg\000"
                               "=d,t\000"
                               "neg\000"
                               "__I_negu\000"
                               "negu\000"
                               "__I_sll\000"
                               "=d,t,<\000"
                               "sll\000"
                               "__I_sra\000"
                               "sra\000"
                               "__I_srl\000"
                               "srl\000"
                               "__I_sllv\000"
                               "=d,t,s\000"
                               "sllv\000"
                               "__I_srav\000"
                               "srav\000"
                               "__I_srlv\000"
                               "srlv\000"
                               "+d,<\000"
                               "+d,s\000"
                               "__I_addi\000"
                               "=t,s,j\000"
                               "addi\000"
                               "__I_addiu\000"
                               "addiu\000"
                               "__I_andi\000"
                               "=t,s,i\000"
                               "andi\000"
                               "__I_ori\000"
                               "ori\000"
                               "__I_xori\000"
                               "xori\000"
                               "__I_slti\000"
                               "slti\000"
                               "__I_sltiu\000"
                               "sltiu\000"
                               "+t,j\000"
                               "+t,i\000"
                               "=t,j\000"
                               "li\000"
                               "__I_lui\000"
                               "=t,i\000"
                               "lui\000"
                               "__I_nop\000"
                               "nop\000"
                               "nop.bl\000"
                               "__I_div\000"
                               "s,t;=h\000"
                               "div\000"
                               "__I_divu\000"
                               "divu\000"
                               "__I_mult\000"
                               "mult\000"
                               "__I_multu\000"
                               "multu\000"
                               "p\000"
                               "b\000"
                               "q;=r\000"
                               "bal\000"
                               "a\000"
                               "j\000"
                               "a;=r\000"
                               "jal\000"
                               "=d,s;=r\000"
                               "jalr\000"
                               "s\000"
                               "jr\000"
                               "s,t,p\000"
                               "beq\000"
                               "s,p\000"
                               "beqz\000"
                               "bgez\000"
                               "s,p;=r\000"
                               "bgezal\000"
                               "bgtz\000"
                               "blez\000"
                               "bltz\000"
                               "bltzal\000"
                               "bne\000"
                               "bnez\000"
                               "p;0\000"
                               "bc0f\000"
                               "bc0t\000"
                               "p;1\000"
                               "bc1f\000"
                               "bc1t\000"
                               "p;3\000"
                               "bc3f\000"
                               "bc3t\000"
                               "beql\000"
                               "beqzl\000"
                               "bgezl\000"
                               "bgezall\000"
                               "bgtzl\000"
                               "blezl\000"
                               "bltzl\000"
                               "bltzall\000"
                               "bnel\000"
                               "bnezl\000"
                               "bc0fl\000"
                               "bc0tl\000"
                               "bc1fl\000"
                               "bc1tl\000"
                               "bc3fl\000"
                               "bc3tl\000"
                               "__I_break\000"
                               "B\000"
                               "breakc\000"
                               "__I_cache\000"
                               "k,o(s)\000"
                               "cache\000"
                               "__I_eret\000"
                               "eret\000"
                               "__I_rfe\000"
                               "rfe\000"
                               "__I_syscall\000"
                               "syscall\000"
                               "00000000000000000000000000001111\000"
                               "__I_c0\000"
                               "C\000"
                               "cop0\000"
                               "__I_c1\000"
                               "cop1\000"
                               "__I_c3\000"
                               "cop3\000"
                               "__I_cfc1\000"
                               "=t,S;1\000"
                               "cfc1\000"
                               "__I_ctc1\000"
                               "t,S;=1\000"
                               "ctc1\000"
                               "__I_mfc0\000"
                               "=t,G\000"
                               "mfc0\000"
                               "__I_mfc1\000"
                               "=t,S\000"
                               "mfc1\000"
                               "__I_mfc3\000"
                               "mfc3\000"
                               "__I_mtc0\000"
                               "t,=G\000"
                               "mtc0\000"
                               "__I_mtc1\000"
                               "t,=S\000"
                               "mtc1\000"
                               "__I_mtc3\000"
                               "mtc3\000"
                               "__I_lb\000"
                               "=t,o(s)\000"
                               "lb\000"
                               "__I_lbu\000"
                               "lbu\000"
                               "__I_lh\000"
                               "lh\000"
                               "__I_lhu\000"
                               "lhu\000"
                               "__I_lw\000"
                               "lw\000"
                               "__I_lwl\000"
                               "+t,o(s)\000"
                               "lwl\000"
                               "__I_lwr\000"
                               "lwr\000"
                               "__I_lwu\000"
                               "lwu\000"
                               "__I_sb\000"
                               "t,o(s)\000"
                               "sb\000"
                               "__I_sc\000"
                               "sc\000"
                               "__I_scd\000"
                               "scd\000"
                               "__I_sh\000"
                               "sh\000"
                               "__I_sw\000"
                               "sw\000"
                               "__I_swl\000"
                               "swl\000"
                               "__I_swr\000"
                               "swr\000"
                               "__I_lwc1\000"
                               "=T,o(s)\000"
                               "lwc1\000"
                               "__I_swc1\000"
                               "T,o(s)\000"
                               "swc1\000"
                               "__I_lwc3\000"
                               "=E,o(s)\000"
                               "lwc3\000"
                               "__I_swc0\000"
                               "E,o(s)\000"
                               "swc0\000"
                               "__I_swc3\000"
                               "swc3\000"
                               "__I_ll\000"
                               "ll\000"
                               "__I_mfhi\000"
                               "=d;h\000"
                               "mfhi\000"
                               "__I_mflo\000"
                               "mflo\000"
                               "__I_mthi\000"
                               "s;=h\000"
                               "mthi\000"
                               "__I_mtlo\000"
                               "mtlo\000"
                               "__I_teq\000"
                               "s,t\000"
                               "teq\000"
                               "__I_teqi\000"
                               "s,j\000"
                               "teqi\000"
                               "__I_tge\000"
                               "tge\000"
                               "__I_tgei\000"
                               "tgei\000"
                               "__I_tgeiu\000"
                               "tgeiu\000"
                               "__I_tgeu\000"
                               "tgeu\000"
                               "__I_tlt\000"
                               "tlt\000"
                               "__I_tlti\000"
                               "tlti\000"
                               "__I_tltiu\000"
                               "tltiu\000"
                               "__I_tltu\000"
                               "tltu\000"
                               "__I_tne\000"
                               "tne\000"
                               "__I_tnei\000"
                               "tnei\000"
                               "__I_tlbp\000"
                               "tlbp\000"
                               "__I_tlbr\000"
                               "tlbr\000"
                               "__I_tlbwi\000"
                               "tlbwi\000"
                               "__I_tlbwr\000"
                               "tlbwr\000"
                               "__I_word\000"
                               "I\000"
                               "word\000"
                               "U\000"
                               "S,T;=1\000"
                               "c.f.s\000"
                               "c.eq.s\000"
                               "c.le.s\000"
                               "c.lt.s\000"
                               "__I_abs_d\000"
                               "=!D,!S\000"
                               "abs.d\000"
                               "__I_add_d\000"
                               "=!D,!S,!T\000"
                               "add.d\000"
                               "__I_cvt_d_l\000"
                               "=!D,S\000"
                               "cvt.d.l\000"
                               "__I_cvt_d_s\000"
                               "cvt.d.s\000"
                               "__I_cvt_d_w\000"
                               "cvt.d.w\000"
                               "__I_cvt_l_d\000"
                               "=D,!S\000"
                               "cvt.l.d\000"
                               "__I_cvt_s_d\000"
                               "cvt.s.d\000"
                               "__I_cvt_w_d\000"
                               "cvt.w.d\000"
                               "__I_div_d\000"
                               "div.d\000"
                               "__I_mov_d\000"
                               "mov.d\000"
                               "__I_mul_d\000"
                               "mul.d\000"
                               "__I_neg_d\000"
                               "neg.d\000"
                               "__I_sub_d\000"
                               "sub.d\000"
                               "+!D\000"
                               "+!D,!T\000"
                               "+D\000"
                               "__I_c_eq_d\000"
                               "!S,!T;=1\000"
                               "c.eq.d\000"
                               "__I_c_f_d\000"
                               "c.f.d\000"
                               "__I_c_le_d\000"
                               "c.le.d\000"
                               "__I_c_lt_d\000"
                               "c.lt.d\000"
                               "__I_c_nge_d\000"
                               "c.nge.d\000"
                               "__I_c_ngl_d\000"
                               "c.ngl.d\000"
                               "__I_c_ngle_d\000"
                               "c.ngle.d\000"
                               "__I_c_ngt_d\000"
                               "c.ngt.d\000"
                               "__I_c_ole_d\000"
                               "c.ole.d\000"
                               "__I_c_olt_d\000"
                               "c.olt.d\000"
                               "__I_c_seq_d\000"
                               "c.seq.d\000"
                               "__I_c_sf_d\000"
                               "c.sf.d\000"
                               "__I_c_ueq_d\000"
                               "c.ueq.d\000"
                               "__I_c_ule_d\000"
                               "c.ule.d\000"
                               "__I_c_ult_d\000"
                               "c.ult.d\000"
                               "__I_c_un_d\000"
                               "c.un.d\000"
                               "__I_abs_s\000"
                               "=D,S\000"
                               "abs.s\000"
                               "__I_add_s\000"
                               "=D,S,T\000"
                               "add.s\000"
                               "__I_cvt_l_s\000"
                               "cvt.l.s\000"
                               "__I_cvt_s_l\000"
                               "cvt.s.l\000"
                               "__I_cvt_s_w\000"
                               "cvt.s.w\000"
                               "__I_cvt_w_s\000"
                               "cvt.w.s\000"
                               "__I_div_s\000"
                               "div.s\000"
                               "__I_mov_s\000"
                               "mov.s\000"
                               "__I_mul_s\000"
                               "mul.s\000"
                               "__I_neg_s\000"
                               "neg.s\000"
                               "__I_sub_s\000"
                               "sub.s\000"
                               "+D,T\000"
                               "__I_c2\000"
                               "cop2\000"
                               "__I_cfc2\000"
                               "=t,2d;K\000"
                               "cfc2\000"
                               "=t,2+d;K\000"
                               "__I_ctc2\000"
                               "t,=2d;=K\000"
                               "ctc2\000"
                               "t,=2+d;=K\000"
                               "__I_mfc2\000"
                               "=t,2S\000"
                               "mfc2\000"
                               "__I_mtc2\000"
                               "t,=2S\000"
                               "mtc2\000"
                               "__I_lwc2\000"
                               "=2T,o(s)\000"
                               "lwc2\000"
                               "__I_swc2\000"
                               "2T,o(s)\000"
                               "swc2\000"
                               "cfc2.i\000"
                               "ctc2.i\000"
                               "01001000001000000000000000000000\000"
                               "sync\000"
                               "00000000000000000000010000001111\000"
                               "sync.p\000"
                               "sync.l\000"
                               "qmfc2\000"
                               "01001000001000000000000000000001\000"
                               "qmfc2.i\000"
                               "01001000101000000000000000000000\000"
                               "qmtc2\000"
                               "01001000101000000000000000000001\000"
                               "qmtc2.i\000"
                               "11011000000000000000000000000000\000"
                               "lqc2\000"
                               "11111000000000000000000000000000\000"
                               "sqc2\000"
                               "qmfc2.ni\000"
                               "qmtc2.ni\000"
                               "ctc2.ni\000"
                               "cfc2.ni\000"
                               "p;2C\000"
                               "bc2f\000"
                               "bc2t\000"
                               "bc2fl\000"
                               "bc2tl\000"
                               "01111000000000000000000000000000\000"
                               "__I_ceil_l_d\000"
                               "ceil.l.d\000"
                               "__I_ceil_l_s\000"
                               "ceil.l.s\000"
                               "__I_ceil_w_d\000"
                               "ceil.w.d\000"
                               "__I_ceil_w_s\000"
                               "ceil.w.s\000"
                               "__I_floor_l_d\000"
                               "floor.l.d\000"
                               "__I_floor_l_s\000"
                               "floor.l.s\000"
                               "__I_floor_w_d\000"
                               "floor.w.d\000"
                               "__I_floor_w_s\000"
                               "floor.w.s\000"
                               "__I_round_l_d\000"
                               "round.l.d\000"
                               "__I_round_l_s\000"
                               "round.l.s\000"
                               "__I_round_w_d\000"
                               "round.w.d\000"
                               "__I_round_w_s\000"
                               "round.w.s\000"
                               "__I_trunc_l_d\000"
                               "trunc.l.d\000"
                               "__I_trunc_l_s\000"
                               "trunc.l.s\000"
                               "__I_trunc_w_d\000"
                               "trunc.w.d\000"
                               "__I_trunc_w_s\000"
                               "trunc.w.s\000"
                               "__I_dmfc1\000"
                               "=t,!S\000"
                               "dmfc1\000"
                               "__I_dmtc1\000"
                               "t,=!S\000"
                               "dmtc1\000"
                               "__I_dmfc0\000"
                               "dmfc0\000"
                               "__I_dmtc0\000"
                               "dmtc0\000"
                               "__I_ldc1\000"
                               "ldc1\000"
                               "__I_ldc2\000"
                               "ldc2\000"
                               "__I_ldc3\000"
                               "ldc3\000"
                               "__I_sdc1\000"
                               "sdc1\000"
                               "__I_sdc2\000"
                               "sdc2\000"
                               "__I_sdc3\000"
                               "sdc3\000"
                               "=D,T\000"
                               "sqrt.s\000"
                               "sqrt.d\000"
                               "clo\000"
                               "clz\000"
                               "dclo\000"
                               "dclz\000"
                               "__I_dadd\000"
                               "dadd\000"
                               "__I_daddu\000"
                               "daddu\000"
                               "__I_dmove\000"
                               "dmove\000"
                               "__I_dsub\000"
                               "dsub\000"
                               "__I_dsubu\000"
                               "dsubu\000"
                               "__I_daddi\000"
                               "daddi\000"
                               "__I_daddiu\000"
                               "daddiu\000"
                               "__I_ddiv\000"
                               "ddiv\000"
                               "__I_ddivu\000"
                               "ddivu\000"
                               "__I_dmult\000"
                               "dmult\000"
                               "__I_dmultu\000"
                               "dmultu\000"
                               "__I_dneg\000"
                               "dneg\000"
                               "__I_dnegu\000"
                               "dnegu\000"
                               "__I_dsll\000"
                               "dsll\000"
                               "__I_dsra\000"
                               "dsra\000"
                               "__I_dsrl\000"
                               "dsrl\000"
                               "__I_dsll32\000"
                               "dsll32\000"
                               "__I_dsra32\000"
                               "dsra32\000"
                               "__I_dsrl32\000"
                               "dsrl32\000"
                               "__I_dsllv\000"
                               "dsllv\000"
                               "__I_dsrav\000"
                               "dsrav\000"
                               "__I_dsrlv\000"
                               "dsrlv\000"
                               "__I_ld\000"
                               "ld\000"
                               "__I_ldl\000"
                               "ldl\000"
                               "__I_ldr\000"
                               "ldr\000"
                               "__I_lld\000"
                               "lld\000"
                               "__I_sd\000"
                               "sd\000"
                               "__I_sdl\000"
                               "sdl\000"
                               "__I_sdr\000"
                               "sdr\000"
                               "+d,s,t\000"
                               "movn\000"
                               "movz\000"
                               "pref\000"
                               "k,A\000"
                               "=!D,t(s)\000"
                               "ldxc1\000"
                               "=D,t(s)\000"
                               "lwxc1\000"
                               "!S,t(s)\000"
                               "sdxc1\000"
                               "S,t(s)\000"
                               "swxc1\000"
                               "+d,s,b;1\000"
                               "movf\000"
                               "movt\000"
                               "+!D,!S,b;1\000"
                               "movf.d\000"
                               "+D,S,b;1\000"
                               "movf.s\000"
                               "movt.d\000"
                               "movt.s\000"
                               "+!D,!S,t\000"
                               "movn.d\000"
                               "+D,S,t\000"
                               "movn.s\000"
                               "movz.d\000"
                               "movz.s\000"
                               "=!D,!R,!S,!T\000"
                               "madd.d\000"
                               "=D,R,S,T\000"
                               "madd.s\000"
                               "msub.d\000"
                               "msub.s\000"
                               "nmadd.d\000"
                               "nmadd.s\000"
                               "nmsub.d\000"
                               "nmsub.s\000"
                               "recip.d\000"
                               "recip.s\000"
                               "rsqrt.s\000"
                               "abs.ps\000"
                               "add.ps\000"
                               "=!D,!S,!T,s\000"
                               "alnv.ps\000"
                               "c.eq.ps\000"
                               "c.f.ps\000"
                               "c.le.ps\000"
                               "c.lt.ps\000"
                               "c.nge.ps\000"
                               "c.ngl.ps\000"
                               "c.ngle.ps\000"
                               "c.ngt.ps\000"
                               "c.ole.ps\000"
                               "c.olt.ps\000"
                               "c.seq.ps\000"
                               "c.sf.ps\000"
                               "c.ueq.ps\000"
                               "c.ule.ps\000"
                               "c.ult.ps\000"
                               "c.un.ps\000"
                               "=!D,S,T\000"
                               "cvt.ps.s\000"
                               "cvt.s.pl\000"
                               "cvt.s.pu\000"
                               "luxc1\000"
                               "madd.ps\000"
                               "mov.ps\000"
                               "mul.ps\000"
                               "neg.ps\000"
                               "nmadd.ps\000"
                               "nmsub.ps\000"
                               "pll.ps\000"
                               "plu.ps\000"
                               "pul.ps\000"
                               "puu.ps\000"
                               "sub.ps\000"
                               "!D,t(s)\000"
                               "suxc1.ps\000"
                               "=$t,o(s)\000"
                               "lq\000"
                               "01111100000000000000000000000000\000"
                               "$t,o(s)\000"
                               "sq\000"
                               "01000010000000000000000000111001\000"
                               "di\000"
                               "01000010000000000000000000111000\000"
                               "ei\000"
                               "01110000000000000000000000000000\000"
                               "=d,s,t;+h\000"
                               "madd\000"
                               "01110000000000000000000000000001\000"
                               "maddu\000"
                               "00000000000000000000000000011000\000"
                               "=d,s,t;=h\000"
                               "00000000000000000000000000011001\000"
                               "=d,s,t;=H\000"
                               "mult1\000"
                               "multu1\000"
                               "s,t;=H\000"
                               "div1\000"
                               "divu1\000"
                               "=d,s,t;+H\000"
                               "madd1\000"
                               "maddu1\000"
                               "s,t;+H\000"
                               "S,T;=Z\000"
                               "adda.s\000"
                               "suba.s\000"
                               "mula.s\000"
                               "=D,S,T;Z\000"
                               "S,T;+Z\000"
                               "madda.s\000"
                               "msuba.s\000"
                               "=D,S,T;=1\000"
                               "max.s\000"
                               "min.s\000"
                               "=t;0\000"
                               "mfbpc\000"
                               "mfdab\000"
                               "mfdabm\000"
                               "mfdvb\000"
                               "mfdvbm\000"
                               "mfiab\000"
                               "mfiabm\000"
                               "t;=0\000"
                               "mtbpc\000"
                               "mtdab\000"
                               "mtdabm\000"
                               "mtdvb\000"
                               "mtdvbm\000"
                               "mtiab\000"
                               "mtiabm\000"
                               "=d;H\000"
                               "mfhi1\000"
                               "mflo1\000"
                               "s;=H\000"
                               "mthi1\000"
                               "mtlo1\000"
                               "01000000000000001100100000000001\000"
                               "=t,P\000"
                               "mfpc\000"
                               "01000000000000001100100000000000\000"
                               "mfps\000"
                               "01000000100000001100100000000001\000"
                               "t,=P\000"
                               "mtpc\000"
                               "01000000100000001100100000000000\000"
                               "mtps\000"
                               "=d;Y\000"
                               "mfsa\000"
                               "s;=Y\000"
                               "mtsa\000"
                               "s,j;=Y\000"
                               "mtsab\000"
                               "mtsah\000"
                               "=$d,$s\000"
                               "qmove\000"
                               "pabsh\000"
                               "pabsw\000"
                               "pcpyh\000"
                               "pexch\000"
                               "pexcw\000"
                               "pexeh\000"
                               "pexoh\000"
                               "pexew\000"
                               "pexow\000"
                               "pext5\000"
                               "ppac5\000"
                               "prevh\000"
                               "prot3w\000"
                               "plzcw\000"
                               "pmfhi\000"
                               "pmflo\000"
                               "d;h\000"
                               "pmfhl.lw\000"
                               "pmfhl.uw\000"
                               "pmfhl.slw\000"
                               "pmfhl.lh\000"
                               "pmfhl.sh\000"
                               "pmthi\000"
                               "pmtlo\000"
                               "pmthl.lw\000"
                               "paddb\000"
                               "paddh\000"
                               "paddw\000"
                               "paddsb\000"
                               "paddsh\000"
                               "paddsw\000"
                               "paddub\000"
                               "padduh\000"
                               "padduw\000"
                               "padsbh\000"
                               "pand\000"
                               "pceqb\000"
                               "pceqh\000"
                               "pceqw\000"
                               "pcgtb\000"
                               "pcgth\000"
                               "pcgtw\000"
                               "pcpyld\000"
                               "pcpyud\000"
                               "pextlb\000"
                               "pextlh\000"
                               "pextlw\000"
                               "pextub\000"
                               "pextuh\000"
                               "pextuw\000"
                               "pinth\000"
                               "pinteh\000"
                               "pmaxh\000"
                               "pmaxw\000"
                               "pminh\000"
                               "pminw\000"
                               "pmsubh\000"
                               "pmsubw\000"
                               "pnor\000"
                               "por\000"
                               "ppacb\000"
                               "ppach\000"
                               "ppacw\000"
                               "psubb\000"
                               "psubh\000"
                               "psubsb\000"
                               "psubsh\000"
                               "psubsw\000"
                               "psubub\000"
                               "psubuh\000"
                               "psubuw\000"
                               "psubw\000"
                               "pxor\000"
                               "=d,s,t;Y\000"
                               "qfsrv\000"
                               "phmadh\000"
                               "phmsbh\000"
                               "pmaddh\000"
                               "pmadduw\000"
                               "pmaddw\000"
                               "pmulth\000"
                               "pmultuw\000"
                               "pmultw\000"
                               "psllvw\000"
                               "psllh\000"
                               "psllw\000"
                               "psrah\000"
                               "psravw\000"
                               "psraw\000"
                               "psrlh\000"
                               "psrlvw\000"
                               "psrlw\000"
                               "=s,t;=h\000"
                               "pdivbw\000"
                               "pdivuw\000"
                               "pdivw\000"
                               "&+2T,2S\000"
                               "vabs\000"
                               "&+2D,2S,2T\000"
                               "vadd\000"
                               "&+2D,2S,2I;K\000"
                               "vaddi\000"
                               "&+2D,2S,2Q;K\000"
                               "vaddq\000"
                               "&+2D,2S,2N\000"
                               "vaddw\000"
                               "vaddx\000"
                               "vaddy\000"
                               "vaddz\000"
                               "&+2A,2T,2S\000"
                               "vadda\000"
                               "&+2A,2S,2I;K\000"
                               "vaddai\000"
                               "&+2A,2S,2Q;K\000"
                               "vaddaq\000"
                               "&+2A,2S,2N\000"
                               "vaddaw\000"
                               "vaddax\000"
                               "vadday\000"
                               "vaddaz\000"
                               "2b\000"
                               "vcallms\000"
                               "2i\000"
                               "01001010000000001101100000111001\000"
                               "9\000"
                               "vcallmsr\000"
                               "2S,2T\000"
                               "vclipw\000"
                               "vclipw.xyz\000"
                               "=2Q,2'S,2'T;=K\000"
                               "vdiv\000"
                               "vftoi0\000"
                               "vftoi4\000"
                               "vftoi12\000"
                               "vftoi15\000"
                               "=2x,2d,2t\000"
                               "viadd\000"
                               "=2t,2d,>\000"
                               "viaddi\000"
                               "viand\000"
                               "=2t,(2d)\000"
                               "vilwr.x\000"
                               "vilwr.y\000"
                               "vilwr.z\000"
                               "vilwr.w\000"
                               "vior\000"
                               "2t,(2d)\000"
                               "viswr.x\000"
                               "viswr.y\000"
                               "viswr.z\000"
                               "viswr.w\000"
                               "visub\000"
                               "vitof0\000"
                               "vitof4\000"
                               "vitof12\000"
                               "vitof15\000"
                               "&+2T,(-2d)\000"
                               "vlqd\000"
                               "&+2T,(2d|)\000"
                               "vlqi\000"
                               "&+2D,2S,2T;2A\000"
                               "vmadd\000"
                               "&+2D,2S,2I;2A;K\000"
                               "vmaddi\000"
                               "&+2D,2S,2Q;2A;K\000"
                               "vmaddq\000"
                               "&+2D,2S,2N;2A\000"
                               "vmaddw\000"
                               "vmaddx\000"
                               "vmaddy\000"
                               "vmaddz\000"
                               "&+2A,2S,2T;2A\000"
                               "vmadda\000"
                               "&+2A,2S,2I;2A;K\000"
                               "vmaddai\000"
                               "&+2A,2S,2Q;2A;K\000"
                               "vmaddaq\000"
                               "vmaddaw\000"
                               "vmaddax\000"
                               "vmadday\000"
                               "vmaddaz\000"
                               "vmax\000"
                               "vmaxi\000"
                               "vmaxw\000"
                               "vmaxx\000"
                               "vmaxy\000"
                               "vmaxz\000"
                               "&+2T,2d\000"
                               "vmfir\000"
                               "vmini\000"
                               "&+2D,2S,2I\000"
                               "vminii\000"
                               "vminiw\000"
                               "vminix\000"
                               "vminiy\000"
                               "vminiz\000"
                               "&=2T,2S\000"
                               "vmove\000"
                               "vmr32\000"
                               "vmsub\000"
                               "vmsubi\000"
                               "vmsubq\000"
                               "vmsubw\000"
                               "vmsubx\000"
                               "vmsuby\000"
                               "vmsubz\000"
                               "vmsuba\000"
                               "vmsubai\000"
                               "vmsubaq\000"
                               "vmsubaw\000"
                               "vmsubax\000"
                               "vmsubay\000"
                               "vmsubaz\000"
                               "=2t,2'S\000"
                               "vmtir\000"
                               "vmul\000"
                               "vmuli\000"
                               "vmulq\000"
                               "vmulw\000"
                               "vmulx\000"
                               "vmuly\000"
                               "vmulz\000"
                               "&+2A,2S,2T\000"
                               "vmula\000"
                               "vmulai\000"
                               "vmulaq\000"
                               "vmulaw\000"
                               "vmulax\000"
                               "vmulay\000"
                               "vmulaz\000"
                               "vnop\000"
                               "=2A,2S,2T\000"
                               "vopmula\000"
                               "vopmula.xyz\000"
                               "=2D,2S,2T;2A\000"
                               "vopmsub\000"
                               "vopmsub.xyz\000"
                               "&+2T,2R;K\000"
                               "vrget\000"
                               "=2R,2'S\000"
                               "vrinit\000"
                               "vrnext\000"
                               "vrsqrt\000"
                               "+2R,2'S;=K\000"
                               "vrxor\000"
                               "&2S,(-2t)\000"
                               "vsqd\000"
                               "&2S,(2t|)\000"
                               "vsqi\000"
                               "=2Q,2'T;=K\000"
                               "vsqrt\000"
                               "vsub\000"
                               "vsubi\000"
                               "vsubq\000"
                               "vsubw\000"
                               "vsubx\000"
                               "vsuby\000"
                               "vsubz\000"
                               "vsuba\000"
                               "vsubai\000"
                               "vsubaq\000"
                               "vsubaw\000"
                               "vsubax\000"
                               "vsubay\000"
                               "vsubaz\000"
                               "vwaitq\000"
                               "=t,A\000"
                               "la\000"
                               "=!T,A\000"
                               "l.d\000"
                               "!T,A\000"
                               "s.d\000"
                               "=E,A\000"
                               "+t,A\000"
                               "=T,A\000"
                               "l.s\000"
                               "t,A\000"
                               "E,A\000"
                               "T,A\000"
                               "s.s\000"
                               "=t,s,I\000"
                               "=d,s,I\000"
                               "s,I\000"
                               "d,s,I\000"
                               "sle\000"
                               "sleu\000"
                               "s,I,p\000"
                               "ble\000"
                               "blel\000"
                               "bleu\000"
                               "bleul\000"
                               "bgt\000"
                               "bgtl\000"
                               "bgtu\000"
                               "bgtul\000"
                               "blt\000"
                               "bltl\000"
                               "bltu\000"
                               "bltul\000"
                               "bge\000"
                               "bgel\000"
                               "bgeu\000"
                               "bgeul\000"
                               ".div1\000"
                               "=d,s,I;=h\000"
                               "diviu\000"
                               "ddivi\000"
                               "ddiviu\000"
                               "rem\000"
                               "remu\000"
                               "drem\000"
                               "dremu\000"
                               "muli\000"
                               "muloi\000"
                               "muloiu\000"
                               "dmuli\000"
                               "dmuloi\000"
                               "dmuloiu\000"
                               "mul\000"
                               "mulo\000"
                               "mulou\000"
                               "d,s,t;=h\000"
                               "dmul\000"
                               "dmulo\000"
                               "dmulou\000"
                               "=t,I\000"
                               "dli\000"
                               "=T,f\000"
                               "li.s\000"
                               "=d,s,<\000"
                               "sext\000"
                               "zext\000"
                               "sgt\000"
                               "sgtu\000"
                               "sge\000"
                               "sgeu\000"
                               "seq\000"
                               "sne\000"
                               "=$t,A\000"
                               "=2T,A\000"
                               ".restoreGP\000"
                               "fcopy\000"
                               "dcopy\000"
                               ".start\000"
                               ".end\000"
                               ".set\000"
                               "I,I\000"
                               "p2align\000"
                               "p2alignw\000"
                               "p2alignl";

void fn_0047CC40(void) {
    UInt32 encoding;
    char* digits;
    UInt16 macroOpcode;
    char digit;

    memset(opcode_info, 0, 0xE0D0);
    opcode_info[0x40A].flags = opcode_info[0x40A].flags | 0x4000;
    opcode_info[0x40B].flags = opcode_info[0x40B].flags | 0x4000;
    opcode_info[0x40C].flags = opcode_info[0x40C].flags | 0x4000;
    opcode_info[0x40D].flags = opcode_info[0x40D].flags | 0x38400001;
    opcode_info[0x40E].flags = opcode_info[0x40E].flags | 0x38400001;
    opcode_info[0x40F].flags = opcode_info[0x40F].flags | 0x38400001;
    opcode_info[0x410].flags = opcode_info[0x410].flags | 0x38400001;
    opcode_info[0x411].flags = opcode_info[0x411].flags | 0x38400001;
    opcode_info[0x412].flags = opcode_info[0x412].flags | 0x38400001;
    opcode_info[0x413].flags = opcode_info[0x413].flags | 0x38400001;
    opcode_info[0x414].flags = opcode_info[0x414].flags | 0x38400001;
    opcode_info[0x415].flags = opcode_info[0x415].flags | 0x38400001;
    opcode_info[0x416].flags = opcode_info[0x416].flags | 0x38400001;
    opcode_info[0x417].flags = opcode_info[0x417].flags | 0x38400001;
    opcode_info[0x418].flags = opcode_info[0x418].flags | 0x38400001;
    opcode_info[0x419].flags = opcode_info[0x419].flags | 0x38400001;
    opcode_info[0x41A].flags = opcode_info[0x41A].flags | 0x38400001;
    opcode_info[0x41B].flags = opcode_info[0x41B].flags | 0x38400001;
    opcode_info[0x41C].flags = opcode_info[0x41C].flags | 0x38400001;
    opcode_info[0x41D].flags = opcode_info[0x41D].flags | 0x38400001;
    opcode_info[0x41E].flags = opcode_info[0x41E].flags | 0x38400001;
    opcode_info[0x41F].flags = opcode_info[0x41F].flags | 0x38400001;
    opcode_info[0x420].flags = opcode_info[0x420].flags | 0x38400001;
    opcode_info[0x421].flags = opcode_info[0x421].flags | 0x38400001;
    opcode_info[0x422].flags = opcode_info[0x422].flags | 0x38400001;
    opcode_info[0x423].flags = opcode_info[0x423].flags | 0x38400001;
    opcode_info[0x424].flags = opcode_info[0x424].flags | 0x38400001;
    opcode_info[0x425].flags = opcode_info[0x425].flags | 0x38400001;
    opcode_info[0x426].flags = opcode_info[0x426].flags | 0x38400001;
    opcode_info[0x427].flags = opcode_info[0x427].flags | 0x38400001;
    opcode_info[0x428].flags = opcode_info[0x428].flags | 0x38400001;
    opcode_info[0x429].flags = opcode_info[0x429].flags | 0x38400001;
    opcode_info[0x42A].flags = opcode_info[0x42A].flags | 0x38400001;
    opcode_info[0x42B].flags = opcode_info[0x42B].flags | 0x38400001;
    opcode_info[0x42C].flags = opcode_info[0x42C].flags | 0x38400001;
    opcode_info[0x42D].flags = opcode_info[0x42D].flags | 0x38400001;
    opcode_info[0x42E].flags = opcode_info[0x42E].flags | 0x38400001;
    opcode_info[0x42F].flags = opcode_info[0x42F].flags | 0x38400001;
    opcode_info[0x430].flags = opcode_info[0x430].flags | 0x38400001;
    opcode_info[0x437].flags = opcode_info[0x437].flags | 0x8400100;
    opcode_info[0x438].flags = opcode_info[0x438].flags | 0x8400100;
    opcode_info[0x43B].flags = opcode_info[0x43B].flags | 0x4000;
    opcode_info[0x43C].flags = opcode_info[0x43C].flags | 0x404000;
    opcode_info[0x43D].flags = opcode_info[0x43D].flags | 0x4000;
    opcode_info[0x43E].flags = opcode_info[0x43E].flags | 0x404000;
    opcode_info[0x43F].flags = opcode_info[0x43F].flags | 0x4000;
    opcode_info[0x440].flags = opcode_info[0x440].flags | 0x4000;
    opcode_info[0x441].flags = opcode_info[0x441].flags | 0x404000;
    opcode_info[0x442].flags = opcode_info[0x442].flags | 0x4000;
    opcode_info[0x443].flags = opcode_info[0x443].flags | 0x404000;
    opcode_info[0x444].flags = opcode_info[0x444].flags | 0x6000;
    opcode_info[0x44C].flags = opcode_info[0x44C].flags | 0x404000;
    opcode_info[0x44D].flags = opcode_info[0x44D].flags | 0x404000;
    opcode_info[0x44E].flags = opcode_info[0x44E].flags | 0x404000;
    opcode_info[0x45A].flags = opcode_info[0x45A].flags | 0x404002;
    opcode_info[0x453].flags = opcode_info[0x453].flags | 0x404002;
    opcode_info[0x454].flags = opcode_info[0x454].flags | 0x404002;
    opcode_info[0x455].flags = opcode_info[0x455].flags | 0x404002;
    opcode_info[0x458].flags = opcode_info[0x458].flags | 0x404102;
    opcode_info[0x459].flags = opcode_info[0x459].flags | 0x404102;
    opcode_info[0x45B].flags = opcode_info[0x45B].flags | 0x404002;
    opcode_info[0x45C].flags = opcode_info[0x45C].flags | 0x404002;
    opcode_info[0x450].flags = opcode_info[0x450].flags | 0x404000;
    opcode_info[0x45D].flags = opcode_info[0x45D].flags | 0x404002;
    opcode_info[0x45E].flags = opcode_info[0x45E].flags | 0x404002;
    opcode_info[0x45F].flags = opcode_info[0x45F].flags | 0x404002;
    opcode_info[0x460].flags = opcode_info[0x460].flags | 0x404002;
    opcode_info[0x461].flags = opcode_info[0x461].flags | 0x404002;
    opcode_info[0x464].flags = opcode_info[0x464].flags | 0x404102;
    opcode_info[0x465].flags = opcode_info[0x465].flags | 0x404102;
    opcode_info[0x466].flags = opcode_info[0x466].flags | 0x404002;
    opcode_info[0x467].flags = opcode_info[0x467].flags | 0x400002;
    opcode_info[0x468].flags = opcode_info[0x468].flags | 0x400002;
    opcode_info[0x469].flags = opcode_info[0x469].flags | 0x6000;
    opcode_info[0x473].flags = opcode_info[0x473].flags | 0x404000;
    opcode_info[0x474].flags = opcode_info[0x474].flags | 0x4000;
    opcode_info[0x475].flags = opcode_info[0x475].flags | 0x4000;
    opcode_info[0x47F].flags = opcode_info[0x47F].flags | 0x400004;
    opcode_info[0x477].flags = opcode_info[0x477].flags | 0x400004;
    opcode_info[0x478].flags = opcode_info[0x478].flags | 0x400004;
    opcode_info[0x479].flags = opcode_info[0x479].flags | 0x400004;
    opcode_info[0x47A].flags = opcode_info[0x47A].flags | 0x400004;
    opcode_info[0x47B].flags = opcode_info[0x47B].flags | 0x400004;
    opcode_info[0x47C].flags = opcode_info[0x47C].flags | 0x400004;
    opcode_info[0x47D].flags = opcode_info[0x47D].flags | 0x400004;
    opcode_info[0x47E].flags = opcode_info[0x47E].flags | 0x400004;
    opcode_info[0x48D].flags = opcode_info[0x48D].flags | 0x6000;
    opcode_info[0x48E].flags = opcode_info[0x48E].flags | 0x404000;
    opcode_info[0x48B].flags = opcode_info[0x48B].flags | 0x4000;
    opcode_info[0x48F].flags = opcode_info[0x48F].flags | 0x4000;
    opcode_info[0x490].flags = opcode_info[0x490].flags | 0x404000;
    opcode_info[0x491].flags = opcode_info[0x491].flags | 0x4000;
    opcode_info[0x492].flags = opcode_info[0x492].flags | 0x4000;
    opcode_info[0x493].flags = opcode_info[0x493].flags | 0x4000;
    opcode_info[0x494].flags = opcode_info[0x494].flags | 0x404000;
    opcode_info[0x495].flags = opcode_info[0x495].flags | 0x4000;
    opcode_info[0x496].flags = opcode_info[0x496].flags | 0x404000;
    opcode_info[0x480].flags = opcode_info[0x480].flags | 0x400004;
    opcode_info[0x497].flags = opcode_info[0x497].flags | 0x4000;
    opcode_info[0x498].flags = opcode_info[0x498].flags | 0x404000;
    opcode_info[0x499].flags = opcode_info[0x499].flags | 0x4000;
    opcode_info[0x49A].flags = opcode_info[0x49A].flags | 0x404000;
    opcode_info[0x49B].flags = opcode_info[0x49B].flags | 0x404000;
    opcode_info[0x49C].flags = opcode_info[0x49C].flags | 0x404000;
    opcode_info[0x49E].flags = opcode_info[0x49E].flags | 0x404000;
    opcode_info[0x49F].flags = opcode_info[0x49F].flags | 0x404000;
    opcode_info[0x4A0].flags = opcode_info[0x4A0].flags | 0x404000;
    opcode_info[0x49D].flags = opcode_info[0x49D].flags | 0x6000;
    opcode_info[0x481].flags = opcode_info[0x481].flags | 0x400004;
    opcode_info[0x482].flags = opcode_info[0x482].flags | 0x400004;
    opcode_info[0x483].flags = opcode_info[0x483].flags | 0x400004;
    opcode_info[0x484].flags = opcode_info[0x484].flags | 0x400004;
    opcode_info[0x485].flags = opcode_info[0x485].flags | 0x400004;
    opcode_info[0x486].flags = opcode_info[0x486].flags | 0x400004;
    opcode_info[0x487].flags = opcode_info[0x487].flags | 0x400004;
    opcode_info[0x488].flags = opcode_info[0x488].flags | 0x400004;
    opcode_info[0x489].flags = opcode_info[0x489].flags | 0x400004;
    opcode_info[0x48A].flags = opcode_info[0x48A].flags | 0x400004;
    opcode_info[0x431].flags = opcode_info[0x431].flags | 0x8400000;
    opcode_info[0x432].flags = opcode_info[0x432].flags | 0x8400000;
    opcode_info[0x433].flags = opcode_info[0x433].flags | 0x8400000;
    opcode_info[0x434].flags = opcode_info[0x434].flags | 0x8400000;
    opcode_info[0x435].flags = opcode_info[0x435].flags | 0x8400000;
    opcode_info[0x436].flags = opcode_info[0x436].flags | 0x8400000;
    opcode_info[0x4A1].flags = opcode_info[0x4A1].flags | 0x404000;
    opcode_info[0x48C].flags = opcode_info[0x48C].flags | 0x4000;
    opcode_info[0x452].flags = opcode_info[0x452].flags | 0x404002;
    opcode_info[0x476].flags = opcode_info[0x476].flags | 0x400004;
    opcode_info[0x4A3].flags = opcode_info[0x4A3].flags | 0x4010;
    opcode_info[0x4A4].flags = opcode_info[0x4A4].flags | 0x4010;
    opcode_info[0x4A5].flags = opcode_info[0x4A5].flags | 0x100;
    opcode_info[0x4A6].flags = opcode_info[0x4A6].flags | 0x100;
    opcode_info[0x4A7].flags = opcode_info[0x4A7].flags | 0x100;
    mips_opcode_count_607E20 = 0xc6;
    fn_00485D90(0, (opcode_strings + 0x1), (opcode_strings + 0x0), 0, 0, 0, 0, 1);
    fn_00485D90(
        1,
        (opcode_strings + 0x1E),
        (opcode_strings + 0x17),
        0x20,
        0x6000,
        (opcode_strings + 0xF),
        0xc000,
        1
    );
    fn_00485D90(
        4,
        (opcode_strings + 0x2B),
        (opcode_strings + 0x17),
        0x21,
        0x6000,
        (opcode_strings + 0x22),
        0xc000,
        1
    );
    fn_00485D90(
        6,
        (opcode_strings + 0x38),
        (opcode_strings + 0x17),
        0x24,
        0x6000,
        (opcode_strings + 0x30),
        0xc000,
        1
    );
    fn_00485D90(
        0xbc,
        (opcode_strings + 0x44),
        (opcode_strings + 0x17),
        0x27,
        0x6000,
        (opcode_strings + 0x3C),
        0xc000,
        1
    );
    fn_00485D90(
        0xbe,
        (opcode_strings + 0x4F),
        (opcode_strings + 0x17),
        0x25,
        0x6000,
        (opcode_strings + 0x48),
        0xc000,
        1
    );
    fn_00485D90(
        0xe4,
        (opcode_strings + 0x5A),
        (opcode_strings + 0x17),
        0x26,
        0x6000,
        (opcode_strings + 0x52),
        0xc000,
        1
    );
    fn_00485D90(
        0xdc,
        (opcode_strings + 0x66),
        (opcode_strings + 0x17),
        0x22,
        0x4000,
        (opcode_strings + 0x5E),
        0xc000,
        1
    );
    fn_00485D90(
        0xdd,
        (opcode_strings + 0x73),
        (opcode_strings + 0x17),
        0x23,
        0x4000,
        (opcode_strings + 0x6A),
        0xc000,
        1
    );
    fn_00485D90(
        0xbd,
        (opcode_strings + 0x85),
        (opcode_strings + 0x80),
        0x27,
        0x4000,
        (opcode_strings + 0x78),
        0xc000,
        1
    );
    fn_00485D90(
        0xaa,
        (opcode_strings + 0x92),
        (opcode_strings + 0x80),
        0x21,
        0x4010,
        (opcode_strings + 0x89),
        0xc000,
        1
    );
    fn_00485D90(
        0xd4,
        (opcode_strings + 0x9F),
        (opcode_strings + 0x17),
        0x2a,
        0x4000,
        (opcode_strings + 0x97),
        0xc000,
        1
    );
    fn_00485D90(
        0xd7,
        (opcode_strings + 0xAC),
        (opcode_strings + 0x17),
        0x2b,
        0x4000,
        (opcode_strings + 0xA3),
        0xc000,
        1
    );
    fn_00485D90(0x2ef, (opcode_strings + 0x1E), (opcode_strings + 0xB1), 0x20, 0, 0, 0xc000, 1);
    fn_00485D90(0x2f0, (opcode_strings + 0x2B), (opcode_strings + 0xB1), 0x21, 0, 0, 0xc000, 1);
    fn_00485D90(0x2f1, (opcode_strings + 0x38), (opcode_strings + 0xB1), 0x24, 0, 0, 0xc000, 1);
    fn_00485D90(0x2f2, (opcode_strings + 0x44), (opcode_strings + 0xB1), 0x27, 0, 0, 0xc000, 1);
    fn_00485D90(0x2f3, (opcode_strings + 0x4F), (opcode_strings + 0xB1), 0x25, 0, 0, 0xc000, 1);
    fn_00485D90(0x2f4, (opcode_strings + 0x5A), (opcode_strings + 0xB1), 0x26, 0, 0, 0xc000, 1);
    fn_00485D90(0x2f5, (opcode_strings + 0x66), (opcode_strings + 0xB1), 0x22, 0, 0, 0xc000, 1);
    fn_00485D90(0x2f6, (opcode_strings + 0x73), (opcode_strings + 0xB1), 0x23, 0, 0, 0xc000, 1);
    fn_00485D90(0x2f7, (opcode_strings + 0x85), (opcode_strings + 0xB6), 0x27, 0, 0, 0xc000, 1);
    fn_00485D90(0x2f8, (opcode_strings + 0x9F), (opcode_strings + 0xB1), 0x2a, 0, 0, 0xc000, 1);
    fn_00485D90(0x2f9, (opcode_strings + 0xAC), (opcode_strings + 0xB1), 0x2b, 0, 0, 0xc000, 1);
    fn_00485D90(
        0xb8,
        (opcode_strings + 0xC6),
        (opcode_strings + 0xC1),
        0x22,
        0x4000,
        (opcode_strings + 0xB9),
        0xc000,
        1
    );
    fn_00485D90(
        0xb9,
        (opcode_strings + 0xD3),
        (opcode_strings + 0xC1),
        0x23,
        0x4000,
        (opcode_strings + 0xCA),
        0xc000,
        1
    );
    fn_00485D90(
        0xd2,
        (opcode_strings + 0xE7),
        (opcode_strings + 0xE0),
        0,
        0x4000,
        (opcode_strings + 0xD8),
        0xc000,
        1
    );
    fn_00485D90(
        0xd8,
        (opcode_strings + 0xF3),
        (opcode_strings + 0xE0),
        3,
        0x4000,
        (opcode_strings + 0xEB),
        0xc000,
        1
    );
    fn_00485D90(
        0xda,
        (opcode_strings + 0xFF),
        (opcode_strings + 0xE0),
        2,
        0x4000,
        (opcode_strings + 0xF7),
        0xc000,
        1
    );
    fn_00485D90(
        0xd3,
        (opcode_strings + 0x113),
        (opcode_strings + 0x10C),
        4,
        0x4000,
        (opcode_strings + 0x103),
        0xc000,
        1
    );
    fn_00485D90(
        0xd9,
        (opcode_strings + 0x121),
        (opcode_strings + 0x10C),
        7,
        0x4000,
        (opcode_strings + 0x118),
        0xc000,
        1
    );
    fn_00485D90(
        0xdb,
        (opcode_strings + 0x12F),
        (opcode_strings + 0x10C),
        6,
        0x4000,
        (opcode_strings + 0x126),
        0xc000,
        1
    );
    fn_00485D90(0x338, (opcode_strings + 0xC6), (opcode_strings + 0xB6), 0x22, 0, 0, 0xc000, 1);
    fn_00485D90(0x339, (opcode_strings + 0xD3), (opcode_strings + 0xB6), 0x23, 0, 0, 0xc000, 1);
    fn_00485D90(0x33a, (opcode_strings + 0xE7), (opcode_strings + 0x134), 0, 0, 0, 0xc000, 1);
    fn_00485D90(0x33b, (opcode_strings + 0xF3), (opcode_strings + 0x134), 3, 0, 0, 0xc000, 1);
    fn_00485D90(0x33c, (opcode_strings + 0xFF), (opcode_strings + 0x134), 2, 0, 0, 0xc000, 1);
    fn_00485D90(0x33d, (opcode_strings + 0x113), (opcode_strings + 0x139), 4, 0, 0, 0xc000, 1);
    fn_00485D90(0x33e, (opcode_strings + 0x121), (opcode_strings + 0x139), 7, 0, 0, 0xc000, 1);
    fn_00485D90(0x33f, (opcode_strings + 0x12F), (opcode_strings + 0x139), 6, 0, 0, 0xc000, 1);
    fn_00485D90(
        2,
        (opcode_strings + 0x14E),
        (opcode_strings + 0x147),
        0x20000000,
        0x4000,
        (opcode_strings + 0x13E),
        0xc000,
        1
    );
    fn_00485D90(
        3,
        (opcode_strings + 0x15D),
        (opcode_strings + 0x147),
        0x24000000,
        0x4000,
        (opcode_strings + 0x153),
        0xc000,
        1
    );
    fn_00485D90(
        7,
        (opcode_strings + 0x173),
        (opcode_strings + 0x16C),
        0x30000000,
        0x4000,
        (opcode_strings + 0x163),
        0xc000,
        1
    );
    fn_00485D90(
        0xbf,
        (opcode_strings + 0x180),
        (opcode_strings + 0x16C),
        0x34000000,
        0x4000,
        (opcode_strings + 0x178),
        0xc000,
        1
    );
    fn_00485D90(
        0xe5,
        (opcode_strings + 0x18D),
        (opcode_strings + 0x16C),
        0x38000000,
        0x4000,
        (opcode_strings + 0x184),
        0xc000,
        1
    );
    fn_00485D90(
        0xd5,
        (opcode_strings + 0x19B),
        (opcode_strings + 0x147),
        0x28000000,
        0x4000,
        (opcode_strings + 0x192),
        0xc000,
        1
    );
    fn_00485D90(
        0xd6,
        (opcode_strings + 0x1AA),
        (opcode_strings + 0x147),
        0x2c000000,
        0x4000,
        (opcode_strings + 0x1A0),
        0xc000,
        1
    );
    fn_00485D90(
        0x32f, (opcode_strings + 0x14E), (opcode_strings + 0x1B0), 0x20000000, 0, 0, 0xc000, 1
    );
    fn_00485D90(
        0x330, (opcode_strings + 0x15D), (opcode_strings + 0x1B0), 0x24000000, 0, 0, 0xc000, 1
    );
    fn_00485D90(
        0x331, (opcode_strings + 0x173), (opcode_strings + 0x1B5), 0x30000000, 0, 0, 0xc000, 1
    );
    fn_00485D90(
        0x332, (opcode_strings + 0x180), (opcode_strings + 0x1B5), 0x34000000, 0, 0, 0xc000, 1
    );
    fn_00485D90(
        0x333, (opcode_strings + 0x18D), (opcode_strings + 0x1B5), 0x38000000, 0, 0, 0xc000, 1
    );
    fn_00485D90(
        0x334, (opcode_strings + 0x19B), (opcode_strings + 0x1B0), 0x28000000, 0, 0, 0xc000, 1
    );
    fn_00485D90(
        0x335, (opcode_strings + 0x1AA), (opcode_strings + 0x1B0), 0x2c000000, 0, 0, 0xc000, 1
    );
    fn_00485D90(
        5, (opcode_strings + 0x1BF), (opcode_strings + 0x1BA), 0x20000000, 0x4000, 0, 0xc000, 1
    );
    fn_00485D90(
        0x90,
        (opcode_strings + 0x1CF),
        (opcode_strings + 0x1CA),
        0x3c000000,
        0x4000,
        (opcode_strings + 0x1C2),
        0xc000,
        1
    );
    fn_00485D90(
        0xba,
        (opcode_strings + 0x1DB),
        (opcode_strings + 0x0),
        0,
        0x100,
        (opcode_strings + 0x1D3),
        0xc000,
        1
    );
    fn_00485D90(0xbb, (opcode_strings + 0x1DF), (opcode_strings + 0x0), 0, 0x100, 0, 0xc000, 1);
    fn_00485D90(
        0x7c,
        (opcode_strings + 0x1F5),
        (opcode_strings + 0x1EE),
        0x1a,
        0,
        (opcode_strings + 0x1E6),
        0x20014002,
        0x24
    );
    fn_00485D90(
        0x7d,
        (opcode_strings + 0x202),
        (opcode_strings + 0x1EE),
        0x1b,
        0,
        (opcode_strings + 0x1F9),
        0x20014002,
        0x24
    );
    fn_00485D90(
        0xb6,
        (opcode_strings + 0x210),
        (opcode_strings + 0x1EE),
        0x18,
        0,
        (opcode_strings + 0x207),
        0x10014002,
        4
    );
    fn_00485D90(
        0xb7,
        (opcode_strings + 0x21F),
        (opcode_strings + 0x1EE),
        0x19,
        0,
        (opcode_strings + 0x215),
        0x10014002,
        4
    );
    fn_00485D90(
        0x39,
        (opcode_strings + 0x227),
        (opcode_strings + 0x225),
        0x10000000,
        0x28000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x38,
        (opcode_strings + 0x22E),
        (opcode_strings + 0x229),
        0x4110000,
        0x28000009,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x3a,
        (opcode_strings + 0x234),
        (opcode_strings + 0x232),
        0x8000000,
        0x28000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x3c,
        (opcode_strings + 0x23B),
        (opcode_strings + 0x236),
        0xc000000,
        0x28000009,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x3d, (opcode_strings + 0x247), (opcode_strings + 0x23F), 9, 0x28000009, 0, 0x4000, 1
    );
    fn_00485D90(
        0x3b, (opcode_strings + 0x24E), (opcode_strings + 0x24C), 8, 0x28000001, 0, 0x4000, 1
    );
    fn_00485D90(
        0x18,
        (opcode_strings + 0x257),
        (opcode_strings + 0x251),
        0x10000000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x1a,
        (opcode_strings + 0x25F),
        (opcode_strings + 0x25B),
        0x10000000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x1c,
        (opcode_strings + 0x264),
        (opcode_strings + 0x25B),
        0x4010000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x1e,
        (opcode_strings + 0x270),
        (opcode_strings + 0x269),
        0x4110000,
        0x38000009,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x20,
        (opcode_strings + 0x277),
        (opcode_strings + 0x25B),
        0x1c000000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x22,
        (opcode_strings + 0x27C),
        (opcode_strings + 0x25B),
        0x18000000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x24,
        (opcode_strings + 0x281),
        (opcode_strings + 0x25B),
        0x4000000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x26,
        (opcode_strings + 0x286),
        (opcode_strings + 0x269),
        0x4100000,
        0x38000009,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x28,
        (opcode_strings + 0x28D),
        (opcode_strings + 0x251),
        0x14000000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x2a,
        (opcode_strings + 0x291),
        (opcode_strings + 0x25B),
        0x14000000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        8, (opcode_strings + 0x29A), (opcode_strings + 0x296), 0x41000000, 0x38000001, 0, 0x44000, 1
    );
    fn_00485D90(
        10,
        (opcode_strings + 0x29F),
        (opcode_strings + 0x296),
        0x41010000,
        0x38000001,
        0,
        0x44000,
        1
    );
    fn_00485D90(
        0xc,
        (opcode_strings + 0x2A8),
        (opcode_strings + 0x2A4),
        0x45000000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0xe,
        (opcode_strings + 0x2AD),
        (opcode_strings + 0x2A4),
        0x45010000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x14,
        (opcode_strings + 0x2B6),
        (opcode_strings + 0x2B2),
        0x4d000000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x16,
        (opcode_strings + 0x2BB),
        (opcode_strings + 0x2B2),
        0x4d010000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x19,
        (opcode_strings + 0x2C0),
        (opcode_strings + 0x251),
        0x50000000,
        0x38800001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x1b,
        (opcode_strings + 0x2C5),
        (opcode_strings + 0x25B),
        0x50000000,
        0x38800001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x1d,
        (opcode_strings + 0x2CB),
        (opcode_strings + 0x25B),
        0x4030000,
        0x38800001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x1f,
        (opcode_strings + 0x2D1),
        (opcode_strings + 0x269),
        0x4130000,
        0x38800009,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x21,
        (opcode_strings + 0x2D9),
        (opcode_strings + 0x25B),
        0x5c000000,
        0x38800001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x23,
        (opcode_strings + 0x2DF),
        (opcode_strings + 0x25B),
        0x58000000,
        0x38800001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x25,
        (opcode_strings + 0x2E5),
        (opcode_strings + 0x25B),
        0x4020000,
        0x38800001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x27,
        (opcode_strings + 0x2EB),
        (opcode_strings + 0x269),
        0x4120000,
        0x38800009,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x29,
        (opcode_strings + 0x2F3),
        (opcode_strings + 0x251),
        0x54000000,
        0x38800001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x2b,
        (opcode_strings + 0x2F8),
        (opcode_strings + 0x25B),
        0x54000000,
        0x38800001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        9, (opcode_strings + 0x2FE), (opcode_strings + 0x296), 0x41020000, 0x38800001, 0, 0x44000, 1
    );
    fn_00485D90(
        0xb,
        (opcode_strings + 0x304),
        (opcode_strings + 0x296),
        0x41030000,
        0x38800001,
        0,
        0x44000,
        1
    );
    fn_00485D90(
        0xd,
        (opcode_strings + 0x30A),
        (opcode_strings + 0x2A4),
        0x45020000,
        0x38800001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0xf,
        (opcode_strings + 0x310),
        (opcode_strings + 0x2A4),
        0x45030000,
        0x38800001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x15,
        (opcode_strings + 0x316),
        (opcode_strings + 0x2B2),
        0x4d020000,
        0x38800001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x17,
        (opcode_strings + 0x31C),
        (opcode_strings + 0x2B2),
        0x4d030000,
        0x38800001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x3e,
        (opcode_strings + 0x32E),
        (opcode_strings + 0x32C),
        0xd,
        0x8000100,
        (opcode_strings + 0x322),
        0x30000,
        1
    );
    fn_00485D90(
        100,
        (opcode_strings + 0x346),
        (opcode_strings + 0x33F),
        0xbc000000,
        0x8000100,
        (opcode_strings + 0x335),
        0x40000,
        4
    );
    fn_00485D90(
        0x8f,
        (opcode_strings + 0x355),
        (opcode_strings + 0x0),
        0x42000018,
        0x8000100,
        (opcode_strings + 0x34C),
        0x24000,
        1
    );
    fn_00485D90(
        0xc0,
        (opcode_strings + 0x362),
        (opcode_strings + 0x0),
        0x42000010,
        0x8000100,
        (opcode_strings + 0x35A),
        0x4000,
        1
    );
    fn_00485D90(
        0x3f,
        (opcode_strings + 0x372),
        (opcode_strings + 0x32C),
        0xc,
        0x8000100,
        (opcode_strings + 0x366),
        0x30000,
        1
    );
    if (copts.unknown0C >= 2) {
        digits = (opcode_strings + 0x37A);
        encoding = 0;
        digit = *digits;
        while (digit != '\0') {
            encoding += encoding;
            digits += 1;
            encoding |= (int)digit - 0x30U;
            digit = *digits;
        }
        fn_00485D90(
            0xde,
            (opcode_strings + 0xACD),
            (opcode_strings + 0x0),
            encoding,
            0x8000100,
            0,
            0x20000,
            4
        );
        digits = (opcode_strings + 0xAD2);
        encoding = 0;
        digit = *digits;
        while (digit != '\0') {
            encoding += encoding;
            digits += 1;
            encoding |= (int)digit - 0x30U;
            digit = *digits;
        }
        fn_00485D90(
            0xdf,
            (opcode_strings + 0xAF3),
            (opcode_strings + 0x0),
            encoding,
            0x8000100,
            0,
            0x20000,
            4
        );
        opcode_info[0xDE].unk4 = (opcode_strings + 0xAFA);
    }
    fn_00485D90(
        0x40,
        (opcode_strings + 0x3A4),
        (opcode_strings + 0x3A2),
        0x42000000,
        0x8000100,
        (opcode_strings + 0x39B),
        0,
        4
    );
    fn_00485D90(
        0x41,
        (opcode_strings + 0x3B0),
        (opcode_strings + 0x3A2),
        0x46000000,
        0x8000100,
        (opcode_strings + 0x3A9),
        0,
        4
    );
    fn_00485D90(
        0x43,
        (opcode_strings + 0x3BC),
        (opcode_strings + 0x3A2),
        0x4e000000,
        0x8000100,
        (opcode_strings + 0x3B5),
        0,
        4
    );
    fn_00485D90(
        0x66,
        (opcode_strings + 0x3D1),
        (opcode_strings + 0x3CA),
        0x44400000,
        0x8000000,
        (opcode_strings + 0x3C1),
        0x400000,
        1
    );
    fn_00485D90(
        0x67,
        (opcode_strings + 0x3E6),
        (opcode_strings + 0x3DF),
        0x44c00000,
        0x8000100,
        (opcode_strings + 0x3D6),
        0x400003,
        1
    );
    fn_00485D90(
        0xa4,
        (opcode_strings + 0x3F9),
        (opcode_strings + 0x3F4),
        0x40000000,
        0x9000000,
        (opcode_strings + 0x3EB),
        0x40000,
        1
    );
    fn_00485D90(
        0xa5,
        (opcode_strings + 0x40C),
        (opcode_strings + 0x407),
        0x44000000,
        0x9000000,
        (opcode_strings + 0x3FE),
        0x8003,
        2
    );
    fn_00485D90(
        0xa7,
        (opcode_strings + 0x41A),
        (opcode_strings + 0x3F4),
        0x4c000000,
        0x9000000,
        (opcode_strings + 0x411),
        0,
        1
    );
    fn_00485D90(
        0xac,
        (opcode_strings + 0x42D),
        (opcode_strings + 0x428),
        0x40800000,
        0x89000100,
        (opcode_strings + 0x41F),
        0x40000,
        1
    );
    fn_00485D90(
        0xad,
        (opcode_strings + 0x440),
        (opcode_strings + 0x43B),
        0x44800000,
        0x89004000,
        (opcode_strings + 0x432),
        0x8003,
        2
    );
    fn_00485D90(
        0xaf,
        (opcode_strings + 0x44E),
        (opcode_strings + 0x428),
        0x4c800000,
        0x89000100,
        (opcode_strings + 0x445),
        0,
        1
    );
    fn_00485D90(
        0x91,
        (opcode_strings + 0x462),
        (opcode_strings + 0x45A),
        0x80000000,
        0x4002,
        (opcode_strings + 0x453),
        0x8000,
        7
    );
    fn_00485D90(
        0x92,
        (opcode_strings + 0x46D),
        (opcode_strings + 0x45A),
        0x90000000,
        0x4002,
        (opcode_strings + 0x465),
        0x8000,
        7
    );
    fn_00485D90(
        0x94,
        (opcode_strings + 0x478),
        (opcode_strings + 0x45A),
        0x84000000,
        0x4002,
        (opcode_strings + 0x471),
        0x8000,
        7
    );
    fn_00485D90(
        0x95,
        (opcode_strings + 0x483),
        (opcode_strings + 0x45A),
        0x94000000,
        0x4002,
        (opcode_strings + 0x47B),
        0x8000,
        7
    );
    fn_00485D90(
        0x9d,
        (opcode_strings + 0x48E),
        (opcode_strings + 0x45A),
        0x8c000000,
        0x4002,
        (opcode_strings + 0x487),
        0x8000,
        7
    );
    fn_00485D90(
        0x9e,
        (opcode_strings + 0x4A1),
        (opcode_strings + 0x499),
        0x88000000,
        0x4002,
        (opcode_strings + 0x491),
        0x8000,
        7
    );
    fn_00485D90(
        0x9f,
        (opcode_strings + 0x4AD),
        (opcode_strings + 0x499),
        0x98000000,
        0x4002,
        (opcode_strings + 0x4A5),
        0x8000,
        7
    );
    fn_00485D90(
        0xa0,
        (opcode_strings + 0x4B9),
        (opcode_strings + 0x45A),
        0x9c000000,
        0x4002,
        (opcode_strings + 0x4B1),
        0x8000,
        7
    );
    fn_00485D90(
        0xc3,
        (opcode_strings + 0x4CB),
        (opcode_strings + 0x4C4),
        0xa0000000,
        4,
        (opcode_strings + 0x4BD),
        0x8000,
        1
    );
    fn_00485D90(
        0xc1,
        (opcode_strings + 0x4D5),
        (opcode_strings + 0x4C4),
        0xe0000000,
        4,
        (opcode_strings + 0x4CE),
        0x8000,
        1
    );
    fn_00485D90(
        0xc2,
        (opcode_strings + 0x4E0),
        (opcode_strings + 0x4C4),
        0xf0000000,
        4,
        (opcode_strings + 0x4D8),
        0x8000,
        1
    );
    fn_00485D90(
        0xca,
        (opcode_strings + 0x4EB),
        (opcode_strings + 0x4C4),
        0xa4000000,
        4,
        (opcode_strings + 0x4E4),
        0x8000,
        1
    );
    fn_00485D90(
        0xcb,
        (opcode_strings + 0x4F5),
        (opcode_strings + 0x4C4),
        0xac000000,
        4,
        (opcode_strings + 0x4EE),
        0x8000,
        1
    );
    fn_00485D90(
        0xd0,
        (opcode_strings + 0x500),
        (opcode_strings + 0x4C4),
        0xa8000000,
        4,
        (opcode_strings + 0x4F8),
        0x8000,
        1
    );
    fn_00485D90(
        0xd1,
        (opcode_strings + 0x50C),
        (opcode_strings + 0x4C4),
        0xb8000000,
        4,
        (opcode_strings + 0x504),
        0x8000,
        1
    );
    fn_00485D90(
        0xa1,
        (opcode_strings + 0x521),
        (opcode_strings + 0x519),
        0xc4000000,
        0x4002,
        (opcode_strings + 0x510),
        0x408000,
        7
    );
    fn_00485D90(
        0xcd,
        (opcode_strings + 0x536),
        (opcode_strings + 0x52F),
        0xe4000000,
        4,
        (opcode_strings + 0x526),
        0x408000,
        1
    );
    fn_00485D90(
        0xa3,
        (opcode_strings + 0x54C),
        (opcode_strings + 0x544),
        0xcc000000,
        0x4002,
        (opcode_strings + 0x53B),
        0x8000,
        7
    );
    fn_00485D90(
        0xcc,
        (opcode_strings + 0x561),
        (opcode_strings + 0x55A),
        0xe0000000,
        4,
        (opcode_strings + 0x551),
        0x8000,
        1
    );
    fn_00485D90(
        0xcf,
        (opcode_strings + 0x56F),
        (opcode_strings + 0x55A),
        0xec000000,
        4,
        (opcode_strings + 0x566),
        0x8000,
        1
    );
    fn_00485D90(
        0x9b,
        (opcode_strings + 0x57B),
        (opcode_strings + 0x45A),
        0xc0000000,
        2,
        (opcode_strings + 0x574),
        0x8000,
        7
    );
    fn_00485D90(
        0xa8,
        (opcode_strings + 0x58C),
        (opcode_strings + 0x587),
        0x10,
        0x8004000,
        (opcode_strings + 0x57E),
        0x10000,
        3
    );
    fn_00485D90(
        0xa9,
        (opcode_strings + 0x59A),
        (opcode_strings + 0x587),
        0x12,
        0x8004000,
        (opcode_strings + 0x591),
        0x10000,
        3
    );
    fn_00485D90(
        0xb2,
        (opcode_strings + 0x5AD),
        (opcode_strings + 0x5A8),
        0x11,
        0x88004000,
        (opcode_strings + 0x59F),
        0x10000,
        3
    );
    fn_00485D90(
        0xb3,
        (opcode_strings + 0x5BB),
        (opcode_strings + 0x5A8),
        0x13,
        0x88004000,
        (opcode_strings + 0x5B2),
        0x10000,
        3
    );
    fn_00485D90(
        0x2c,
        (opcode_strings + 0x5CC),
        (opcode_strings + 0x5C8),
        0x34,
        0x8000000,
        (opcode_strings + 0x5C0),
        0,
        1
    );
    fn_00485D90(
        0x2d,
        (opcode_strings + 0x5DD),
        (opcode_strings + 0x5D9),
        0x40c0000,
        0x8000000,
        (opcode_strings + 0x5D0),
        0,
        1
    );
    fn_00485D90(
        0x2e,
        (opcode_strings + 0x5EA),
        (opcode_strings + 0x5C8),
        0x30,
        0x8000000,
        (opcode_strings + 0x5E2),
        0,
        1
    );
    fn_00485D90(
        0x2f,
        (opcode_strings + 0x5F7),
        (opcode_strings + 0x5D9),
        0x4080000,
        0x8000000,
        (opcode_strings + 0x5EE),
        0,
        1
    );
    fn_00485D90(
        0x30,
        (opcode_strings + 0x606),
        (opcode_strings + 0x5D9),
        0x4090000,
        0x8000000,
        (opcode_strings + 0x5FC),
        0,
        1
    );
    fn_00485D90(
        0x31,
        (opcode_strings + 0x615),
        (opcode_strings + 0x5C8),
        0x31,
        0x8000000,
        (opcode_strings + 0x60C),
        0,
        1
    );
    fn_00485D90(
        0x32,
        (opcode_strings + 0x622),
        (opcode_strings + 0x5C8),
        0x32,
        0x8000000,
        (opcode_strings + 0x61A),
        0,
        1
    );
    fn_00485D90(
        0x33,
        (opcode_strings + 0x62F),
        (opcode_strings + 0x5D9),
        0x40a0000,
        0x8000000,
        (opcode_strings + 0x626),
        0,
        1
    );
    fn_00485D90(
        0x34,
        (opcode_strings + 0x63E),
        (opcode_strings + 0x5D9),
        0x40b0000,
        0x8000000,
        (opcode_strings + 0x634),
        0,
        1
    );
    fn_00485D90(
        0x35,
        (opcode_strings + 0x64D),
        (opcode_strings + 0x5C8),
        0x33,
        0x8000000,
        (opcode_strings + 0x644),
        0,
        1
    );
    fn_00485D90(
        0x36,
        (opcode_strings + 0x65A),
        (opcode_strings + 0x5C8),
        0x36,
        0x8000000,
        (opcode_strings + 0x652),
        0,
        1
    );
    fn_00485D90(
        0x37,
        (opcode_strings + 0x667),
        (opcode_strings + 0x5D9),
        0x40e0000,
        0x8000000,
        (opcode_strings + 0x65E),
        0,
        1
    );
    fn_00485D90(
        0xe0,
        (opcode_strings + 0x675),
        (opcode_strings + 0x0),
        0x42000008,
        0x100,
        (opcode_strings + 0x66C),
        0,
        1
    );
    fn_00485D90(
        0xe1,
        (opcode_strings + 0x683),
        (opcode_strings + 0x0),
        0x42000001,
        0x100,
        (opcode_strings + 0x67A),
        0,
        1
    );
    fn_00485D90(
        0xe2,
        (opcode_strings + 0x692),
        (opcode_strings + 0x0),
        0x42000002,
        0x100,
        (opcode_strings + 0x688),
        0,
        1
    );
    fn_00485D90(
        0xe3,
        (opcode_strings + 0x6A2),
        (opcode_strings + 0x0),
        0x42000006,
        0x100,
        (opcode_strings + 0x698),
        0,
        1
    );
    fn_00485D90(
        0x2ed,
        (opcode_strings + 0x6B3),
        (opcode_strings + 0x6B1),
        0,
        0x100,
        (opcode_strings + 0x6A8),
        0,
        1
    );
    fn_00485D90(0x2ee, (opcode_strings + 0x6B3), (opcode_strings + 0x6B8), 0, 0x100, 0, 0, 1);
    fn_00485D90(
        0x47,
        (opcode_strings + 0x6C1),
        (opcode_strings + 0x6BA),
        0x46000030,
        0x8000000,
        0,
        0x4003,
        2
    );
    fn_00485D90(
        0x45,
        (opcode_strings + 0x6C7),
        (opcode_strings + 0x6BA),
        0x46000032,
        0x8000000,
        0,
        0x4003,
        2
    );
    fn_00485D90(
        0x49,
        (opcode_strings + 0x6CE),
        (opcode_strings + 0x6BA),
        0x46000036,
        0x8000000,
        0,
        0x4003,
        2
    );
    fn_00485D90(
        0x4b,
        (opcode_strings + 0x6D5),
        (opcode_strings + 0x6BA),
        0x46000034,
        0x8000000,
        0,
        0x4003,
        2
    );
    if ((copts.fp_library & 2) != 0) {
        fn_00485D90(
            0xea,
            (opcode_strings + 0x6ED),
            (opcode_strings + 0x6E6),
            0x46200005,
            0x4000,
            (opcode_strings + 0x6DC),
            0x4003,
            1
        );
        fn_00485D90(
            0xec,
            (opcode_strings + 0x707),
            (opcode_strings + 0x6FD),
            0x46200000,
            0x6000,
            (opcode_strings + 0x6F3),
            0x4003,
            4
        );
        fn_00485D90(
            0x6c,
            (opcode_strings + 0x71F),
            (opcode_strings + 0x719),
            0x46a00021,
            0x4000,
            (opcode_strings + 0x70D),
            0x4003,
            6
        );
        fn_00485D90(
            0x6d,
            (opcode_strings + 0x733),
            (opcode_strings + 0x719),
            0x46000021,
            0x4000,
            (opcode_strings + 0x727),
            0x4003,
            2
        );
        fn_00485D90(
            0x6e,
            (opcode_strings + 0x747),
            (opcode_strings + 0x719),
            0x46800021,
            0x4000,
            (opcode_strings + 0x73B),
            0x4003,
            6
        );
        fn_00485D90(
            0x6f,
            (opcode_strings + 0x761),
            (opcode_strings + 0x75B),
            0x46200025,
            0x4000,
            (opcode_strings + 0x74F),
            0x4003,
            6
        );
        fn_00485D90(
            0x71,
            (opcode_strings + 0x775),
            (opcode_strings + 0x75B),
            0x46200020,
            0x4000,
            (opcode_strings + 0x769),
            0x4003,
            3
        );
        fn_00485D90(
            0x74,
            (opcode_strings + 0x789),
            (opcode_strings + 0x75B),
            0x46200024,
            0x4000,
            (opcode_strings + 0x77D),
            0x4003,
            6
        );
        fn_00485D90(
            0xf2,
            (opcode_strings + 0x79B),
            (opcode_strings + 0x6FD),
            0x46200003,
            0x4000,
            (opcode_strings + 0x791),
            0x4003,
            0x3b
        );
        fn_00485D90(
            0xf8,
            (opcode_strings + 0x7AB),
            (opcode_strings + 0x6E6),
            0x46200006,
            0x4010,
            (opcode_strings + 0x7A1),
            0x4003,
            1
        );
        fn_00485D90(
            0xb4,
            (opcode_strings + 0x7BB),
            (opcode_strings + 0x6FD),
            0x46200002,
            0x6000,
            (opcode_strings + 0x7B1),
            0x4003,
            6
        );
        fn_00485D90(
            0xfa,
            (opcode_strings + 0x7CB),
            (opcode_strings + 0x6E6),
            0x46200007,
            0x4000,
            (opcode_strings + 0x7C1),
            0x4003,
            1
        );
        fn_00485D90(
            0x102,
            (opcode_strings + 0x7DB),
            (opcode_strings + 0x6FD),
            0x46200001,
            0x4000,
            (opcode_strings + 0x7D1),
            0x4003,
            4
        );
        fn_00485D90(
            0x354, (opcode_strings + 0x6ED), (opcode_strings + 0x7E1), 0x46200005, 0, 0, 0x4003, 1
        );
        fn_00485D90(
            0x355, (opcode_strings + 0x707), (opcode_strings + 0x7E5), 0x46200000, 0, 0, 0x4003, 4
        );
        fn_00485D90(
            0x356, (opcode_strings + 0x71F), (opcode_strings + 0x7E1), 0x46a00021, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x357, (opcode_strings + 0x733), (opcode_strings + 0x7E1), 0x46000021, 0, 0, 0x4003, 2
        );
        fn_00485D90(
            0x358, (opcode_strings + 0x747), (opcode_strings + 0x7E1), 0x46800021, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x359, (opcode_strings + 0x761), (opcode_strings + 0x7EC), 0x46200025, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x362, (opcode_strings + 0x775), (opcode_strings + 0x7EC), 0x46200020, 0, 0, 0x4003, 3
        );
        fn_00485D90(
            0x35a, (opcode_strings + 0x789), (opcode_strings + 0x7EC), 0x46200024, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x35b,
            (opcode_strings + 0x79B),
            (opcode_strings + 0x7E5),
            0x46200003,
            0,
            0,
            0x4003,
            0x3b
        );
        fn_00485D90(
            0x35c, (opcode_strings + 0x7BB), (opcode_strings + 0x7E5), 0x46200002, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x35d, (opcode_strings + 0x7CB), (opcode_strings + 0x7E1), 0x46200007, 0, 0, 0x4003, 1
        );
        fn_00485D90(
            0x35e, (opcode_strings + 0x7DB), (opcode_strings + 0x7E5), 0x46200001, 0, 0, 0x4003, 4
        );
        fn_00485D90(
            0x44,
            (opcode_strings + 0x803),
            (opcode_strings + 0x7FA),
            0x46200032,
            0x8000000,
            (opcode_strings + 0x7EF),
            0x4003,
            2
        );
        fn_00485D90(
            0x46,
            (opcode_strings + 0x814),
            (opcode_strings + 0x7FA),
            0x46200030,
            0x8000000,
            (opcode_strings + 0x80A),
            0x4003,
            2
        );
        fn_00485D90(
            0x48,
            (opcode_strings + 0x825),
            (opcode_strings + 0x7FA),
            0x4620003e,
            0x8000000,
            (opcode_strings + 0x81A),
            0x4003,
            2
        );
        fn_00485D90(
            0x4a,
            (opcode_strings + 0x837),
            (opcode_strings + 0x7FA),
            0x4620003c,
            0x8000000,
            (opcode_strings + 0x82C),
            0x4003,
            2
        );
        fn_00485D90(
            0x4c,
            (opcode_strings + 0x84A),
            (opcode_strings + 0x7FA),
            0x4620003d,
            0x8000000,
            (opcode_strings + 0x83E),
            0x4003,
            2
        );
        fn_00485D90(
            0x4e,
            (opcode_strings + 0x85E),
            (opcode_strings + 0x7FA),
            0x4620003b,
            0x8000000,
            (opcode_strings + 0x852),
            0x4003,
            2
        );
        fn_00485D90(
            0x50,
            (opcode_strings + 0x873),
            (opcode_strings + 0x7FA),
            0x46200039,
            0x8000000,
            (opcode_strings + 0x866),
            0x4003,
            2
        );
        fn_00485D90(
            0x52,
            (opcode_strings + 0x888),
            (opcode_strings + 0x7FA),
            0x4620003f,
            0x8000000,
            (opcode_strings + 0x87C),
            0x4003,
            2
        );
        fn_00485D90(
            0x54,
            (opcode_strings + 0x89C),
            (opcode_strings + 0x7FA),
            0x46200036,
            0x8000000,
            (opcode_strings + 0x890),
            0x4003,
            2
        );
        fn_00485D90(
            0x56,
            (opcode_strings + 0x8B0),
            (opcode_strings + 0x7FA),
            0x46200034,
            0x8000000,
            (opcode_strings + 0x8A4),
            0x4003,
            2
        );
        fn_00485D90(
            0x58,
            (opcode_strings + 0x8C4),
            (opcode_strings + 0x7FA),
            0x4620003a,
            0x8000000,
            (opcode_strings + 0x8B8),
            0x4003,
            2
        );
        fn_00485D90(
            0x5a,
            (opcode_strings + 0x8D7),
            (opcode_strings + 0x7FA),
            0x46200038,
            0x8000000,
            (opcode_strings + 0x8CC),
            0x4003,
            2
        );
        fn_00485D90(
            0x5c,
            (opcode_strings + 0x8EA),
            (opcode_strings + 0x7FA),
            0x46200033,
            0x8000000,
            (opcode_strings + 0x8DE),
            0x4003,
            2
        );
        fn_00485D90(
            0x5e,
            (opcode_strings + 0x8FE),
            (opcode_strings + 0x7FA),
            0x46200037,
            0x8000000,
            (opcode_strings + 0x8F2),
            0x4003,
            2
        );
        fn_00485D90(
            0x60,
            (opcode_strings + 0x912),
            (opcode_strings + 0x7FA),
            0x46200035,
            0x8000000,
            (opcode_strings + 0x906),
            0x4003,
            2
        );
        fn_00485D90(
            0x62,
            (opcode_strings + 0x925),
            (opcode_strings + 0x7FA),
            0x46200031,
            0x8000000,
            (opcode_strings + 0x91A),
            0x4003,
            2
        );
    }
    fn_00485D90(
        0xeb,
        (opcode_strings + 0x93B),
        (opcode_strings + 0x936),
        0x46000005,
        0x4000,
        (opcode_strings + 0x92C),
        0x4003,
        1
    );
    fn_00485D90(
        0xed,
        (opcode_strings + 0x952),
        (opcode_strings + 0x94B),
        0x46000000,
        0x6000,
        (opcode_strings + 0x941),
        0x4003,
        4
    );
    fn_00485D90(
        0x70,
        (opcode_strings + 0x964),
        (opcode_strings + 0x936),
        0x46000025,
        0x4000,
        (opcode_strings + 0x958),
        0x4003,
        6
    );
    fn_00485D90(
        0x72,
        (opcode_strings + 0x978),
        (opcode_strings + 0x936),
        0x46a00020,
        0x4000,
        (opcode_strings + 0x96C),
        0x4003,
        6
    );
    fn_00485D90(
        0x73,
        (opcode_strings + 0x98C),
        (opcode_strings + 0x936),
        0x46800020,
        0x4000,
        (opcode_strings + 0x980),
        0x4003,
        6
    );
    fn_00485D90(
        0x75,
        (opcode_strings + 0x9A0),
        (opcode_strings + 0x936),
        0x46000024,
        0x4000,
        (opcode_strings + 0x994),
        0x4003,
        6
    );
    fn_00485D90(
        0xf3,
        (opcode_strings + 0x9B2),
        (opcode_strings + 0x94B),
        0x46000003,
        0x4000,
        (opcode_strings + 0x9A8),
        0x4003,
        0x1f
    );
    fn_00485D90(
        0xf9,
        (opcode_strings + 0x9C2),
        (opcode_strings + 0x936),
        0x46000006,
        0x4010,
        (opcode_strings + 0x9B8),
        0x404003,
        1
    );
    fn_00485D90(
        0xb5,
        (opcode_strings + 0x9D2),
        (opcode_strings + 0x94B),
        0x46000002,
        0x6000,
        (opcode_strings + 0x9C8),
        0x4003,
        5
    );
    fn_00485D90(
        0xfb,
        (opcode_strings + 0x9E2),
        (opcode_strings + 0x936),
        0x46000007,
        0x4000,
        (opcode_strings + 0x9D8),
        0x4003,
        1
    );
    fn_00485D90(
        0x103,
        (opcode_strings + 0x9F2),
        (opcode_strings + 0x94B),
        0x46000001,
        0x4000,
        (opcode_strings + 0x9E8),
        0x4003,
        4
    );
    fn_00485D90(
        0x35f, (opcode_strings + 0x93B), (opcode_strings + 0x7EC), 0x46000005, 0, 0, 0x4003, 1
    );
    fn_00485D90(
        0x360, (opcode_strings + 0x952), (opcode_strings + 0x9F8), 0x46000000, 0, 0, 0x4003, 4
    );
    fn_00485D90(
        0x361, (opcode_strings + 0x964), (opcode_strings + 0x7EC), 0x46000025, 0, 0, 0x4003, 6
    );
    fn_00485D90(
        0x363, (opcode_strings + 0x978), (opcode_strings + 0x7EC), 0x46a00020, 0, 0, 0x4003, 6
    );
    fn_00485D90(
        0x364, (opcode_strings + 0x98C), (opcode_strings + 0x7EC), 0x46800020, 0, 0, 0x4003, 6
    );
    fn_00485D90(
        0x365, (opcode_strings + 0x9A0), (opcode_strings + 0x7EC), 0x46000024, 0, 0, 0x4003, 6
    );
    fn_00485D90(
        0x366, (opcode_strings + 0x9B2), (opcode_strings + 0x9F8), 0x46000003, 0, 0, 0x4003, 0x1f
    );
    fn_00485D90(
        0x367, (opcode_strings + 0x9D2), (opcode_strings + 0x9F8), 0x46000002, 0, 0, 0x4003, 5
    );
    fn_00485D90(
        0x368, (opcode_strings + 0x9E2), (opcode_strings + 0x7EC), 0x46000007, 0, 0, 0x4003, 1
    );
    fn_00485D90(
        0x369, (opcode_strings + 0x9F2), (opcode_strings + 0x9F8), 0x46000001, 0, 0, 0x4003, 4
    );
    fn_00485D90(
        0x42,
        (opcode_strings + 0xA04),
        (opcode_strings + 0x3A2),
        0x4a000000,
        0x8000100,
        (opcode_strings + 0x9FD),
        0,
        4
    );
    fn_00485D90(
        0x68,
        (opcode_strings + 0xA1A),
        (opcode_strings + 0xA12),
        0x48400000,
        0x8000000,
        (opcode_strings + 0xA09),
        0x800004,
        1
    );
    fn_00485D90(
        0x69,
        (opcode_strings + 0xA1A),
        (opcode_strings + 0xA1F),
        0x48400000,
        0x8000000,
        0,
        0x800004,
        1
    );
    fn_00485D90(
        0x6a,
        (opcode_strings + 0xA3A),
        (opcode_strings + 0xA31),
        0x48c00000,
        0x88000100,
        (opcode_strings + 0xA28),
        0x800004,
        1
    );
    fn_00485D90(
        0x6b,
        (opcode_strings + 0xA3A),
        (opcode_strings + 0xA3F),
        0x48c00000,
        0x88000100,
        0,
        0x800004,
        1
    );
    fn_00485D90(
        0xa6,
        (opcode_strings + 0xA58),
        (opcode_strings + 0xA52),
        0x48000000,
        0x9000000,
        (opcode_strings + 0xA49),
        0x800000,
        1
    );
    fn_00485D90(
        0xae,
        (opcode_strings + 0xA6C),
        (opcode_strings + 0xA66),
        0x48800000,
        0x89000100,
        (opcode_strings + 0xA5D),
        0x800000,
        1
    );
    fn_00485D90(
        0xa2,
        (opcode_strings + 0xA83),
        (opcode_strings + 0xA7A),
        0xc8000000,
        0x4002,
        (opcode_strings + 0xA71),
        0x808000,
        7
    );
    fn_00485D90(
        0xce,
        (opcode_strings + 0xA99),
        (opcode_strings + 0xA91),
        0xe8000000,
        4,
        (opcode_strings + 0xA88),
        0x808000,
        1
    );
    fn_00485D90(
        0x1cd,
        (opcode_strings + 0xA9E),
        (opcode_strings + 0xA12),
        0x48400001,
        0x8000000,
        0,
        0x800000,
        1
    );
    fn_00485D90(
        0x1cf,
        (opcode_strings + 0xA9E),
        (opcode_strings + 0xA1F),
        0x48400001,
        0x8000000,
        0,
        0x800000,
        1
    );
    fn_00485D90(
        0x1ce,
        (opcode_strings + 0xAA5),
        (opcode_strings + 0xA31),
        0x48c00001,
        0x88000100,
        0,
        0x800000,
        1
    );
    fn_00485D90(
        0x1d0,
        (opcode_strings + 0xAA5),
        (opcode_strings + 0xA3F),
        0x48c00001,
        0x88000100,
        0,
        0x800000,
        1
    );
    digits = (opcode_strings + 0xAAC);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x264, (opcode_strings + 0xB01), (opcode_strings + 0xA52), encoding, 0x4000, 0, 0x800000, 1
    );
    digits = (opcode_strings + 0xB07);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x265, (opcode_strings + 0xB28), (opcode_strings + 0xA52), encoding, 0x4000, 0, 0x800000, 1
    );
    digits = (opcode_strings + 0xB30);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x266,
        (opcode_strings + 0xB51),
        (opcode_strings + 0xA66),
        encoding,
        0x80004000,
        0,
        0x800000,
        1
    );
    digits = (opcode_strings + 0xB57);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x267,
        (opcode_strings + 0xB78),
        (opcode_strings + 0xA66),
        encoding,
        0x80004000,
        0,
        0x800000,
        1
    );
    digits = (opcode_strings + 0xB80);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x263, (opcode_strings + 0xBA1), (opcode_strings + 0xA7A), encoding, 0x4002, 0, 0x800000, 1
    );
    digits = (opcode_strings + 0xBA6);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x268, (opcode_strings + 0xBC7), (opcode_strings + 0xA91), encoding, 4, 0, 0x800000, 1
    );
    opcode_info[0x264].unk4 = (opcode_strings + 0xBCC);
    opcode_info[0x266].unk4 = (opcode_strings + 0xBD5);
    opcode_info[0x6A].unk4 = (opcode_strings + 0xBDE);
    opcode_info[0x68].unk4 = (opcode_strings + 0xBE6);
    opcode_info[0x6B].unk4 = (opcode_strings + 0xBDE);
    opcode_info[0x69].unk4 = (opcode_strings + 0xBE6);
    fn_00485D90(
        0x10,
        (opcode_strings + 0xBF3),
        (opcode_strings + 0xBEE),
        0x49000000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x12,
        (opcode_strings + 0xBF8),
        (opcode_strings + 0xBEE),
        0x49010000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x11,
        (opcode_strings + 0xBFD),
        (opcode_strings + 0xBEE),
        0x49020000,
        0x38000001,
        0,
        0x4000,
        1
    );
    fn_00485D90(
        0x13,
        (opcode_strings + 0xC03),
        (opcode_strings + 0xBEE),
        0x49030000,
        0x38000001,
        0,
        0x4000,
        1
    );
    if (copts.unknown0C >= 2) {
        fn_00485D90(
            0xee,
            (opcode_strings + 0xC37),
            (opcode_strings + 0x75B),
            0x4620000a,
            0x4000,
            (opcode_strings + 0xC2A),
            0x4003,
            6
        );
        fn_00485D90(
            0xef,
            (opcode_strings + 0xC4D),
            (opcode_strings + 0x936),
            0x4600000a,
            0x4000,
            (opcode_strings + 0xC40),
            0x4003,
            6
        );
        fn_00485D90(
            0xf0,
            (opcode_strings + 0xC63),
            (opcode_strings + 0x75B),
            0x4620000e,
            0x4000,
            (opcode_strings + 0xC56),
            0x4003,
            6
        );
        fn_00485D90(
            0xf1,
            (opcode_strings + 0xC79),
            (opcode_strings + 0x936),
            0x4600000e,
            0x4000,
            (opcode_strings + 0xC6C),
            0x4003,
            6
        );
        fn_00485D90(
            0xf4,
            (opcode_strings + 0xC90),
            (opcode_strings + 0x75B),
            0x4620000b,
            0x4000,
            (opcode_strings + 0xC82),
            0x4003,
            6
        );
        fn_00485D90(
            0xf5,
            (opcode_strings + 0xCA8),
            (opcode_strings + 0x936),
            0x4600000b,
            0x4000,
            (opcode_strings + 0xC9A),
            0x4003,
            6
        );
        fn_00485D90(
            0xf6,
            (opcode_strings + 0xCC0),
            (opcode_strings + 0x75B),
            0x4620000f,
            0x4000,
            (opcode_strings + 0xCB2),
            0x4003,
            6
        );
        fn_00485D90(
            0xf7,
            (opcode_strings + 0xCD8),
            (opcode_strings + 0x936),
            0x4600000f,
            0x4000,
            (opcode_strings + 0xCCA),
            0x4003,
            6
        );
        fn_00485D90(
            0xfc,
            (opcode_strings + 0xCF0),
            (opcode_strings + 0x75B),
            0x46200008,
            0x4000,
            (opcode_strings + 0xCE2),
            0x4003,
            6
        );
        fn_00485D90(
            0xfd,
            (opcode_strings + 0xD08),
            (opcode_strings + 0x936),
            0x46000008,
            0x4000,
            (opcode_strings + 0xCFA),
            0x4003,
            6
        );
        fn_00485D90(
            0xfe,
            (opcode_strings + 0xD20),
            (opcode_strings + 0x75B),
            0x4620000c,
            0x4000,
            (opcode_strings + 0xD12),
            0x4003,
            6
        );
        fn_00485D90(
            0xff,
            (opcode_strings + 0xD38),
            (opcode_strings + 0x936),
            0x4600000c,
            0x4000,
            (opcode_strings + 0xD2A),
            0x4003,
            6
        );
        fn_00485D90(
            0x104,
            (opcode_strings + 0xD50),
            (opcode_strings + 0x75B),
            0x46200009,
            0x4000,
            (opcode_strings + 0xD42),
            0x4003,
            6
        );
        fn_00485D90(
            0x105,
            (opcode_strings + 0xD68),
            (opcode_strings + 0x936),
            0x46000009,
            0x4000,
            (opcode_strings + 0xD5A),
            0x4003,
            6
        );
        fn_00485D90(
            0x106,
            (opcode_strings + 0xD80),
            (opcode_strings + 0x75B),
            0x4620000d,
            0x4000,
            (opcode_strings + 0xD72),
            0x4003,
            6
        );
        fn_00485D90(
            0x107,
            (opcode_strings + 0xD98),
            (opcode_strings + 0x936),
            0x4600000d,
            0x4000,
            (opcode_strings + 0xD8A),
            0x4003,
            6
        );
        fn_00485D90(
            0x36a, (opcode_strings + 0xC37), (opcode_strings + 0x7EC), 0x4620000a, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x36b, (opcode_strings + 0xC4D), (opcode_strings + 0x7EC), 0x4600000a, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x36c, (opcode_strings + 0xC63), (opcode_strings + 0x7EC), 0x4620000e, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x36d, (opcode_strings + 0xC79), (opcode_strings + 0x7EC), 0x4600000e, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x36e, (opcode_strings + 0xC90), (opcode_strings + 0x7EC), 0x4620000b, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x36f, (opcode_strings + 0xCA8), (opcode_strings + 0x7EC), 0x4600000b, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x370, (opcode_strings + 0xCC0), (opcode_strings + 0x7EC), 0x4620000f, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x371, (opcode_strings + 0xCD8), (opcode_strings + 0x7EC), 0x4600000f, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x372, (opcode_strings + 0xCF0), (opcode_strings + 0x7EC), 0x46200008, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x373, (opcode_strings + 0xD08), (opcode_strings + 0x7EC), 0x46000008, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x374, (opcode_strings + 0xD20), (opcode_strings + 0x7EC), 0x4620000c, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x375, (opcode_strings + 0xD38), (opcode_strings + 0x7EC), 0x4600000c, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x376, (opcode_strings + 0xD50), (opcode_strings + 0x7EC), 0x46200009, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x377, (opcode_strings + 0xD68), (opcode_strings + 0x7EC), 0x46000009, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x378, (opcode_strings + 0xD80), (opcode_strings + 0x7EC), 0x4620000d, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x379, (opcode_strings + 0xD98), (opcode_strings + 0x7EC), 0x4600000d, 0, 0, 0x4003, 6
        );
        fn_00485D90(
            0x7f,
            (opcode_strings + 0xDB2),
            (opcode_strings + 0xDAC),
            0x44200000,
            0x9004000,
            (opcode_strings + 0xDA2),
            3,
            2
        );
        fn_00485D90(
            0xb1,
            (opcode_strings + 0xDC8),
            (opcode_strings + 0xDC2),
            0x44a00000,
            0x89004000,
            (opcode_strings + 0xDB8),
            3,
            2
        );
        fn_00485D90(
            0x7e,
            (opcode_strings + 0xDD8),
            (opcode_strings + 0x3F4),
            0x40200000,
            0x9000000,
            (opcode_strings + 0xDCE),
            0,
            4
        );
        fn_00485D90(
            0xb0,
            (opcode_strings + 0xDE8),
            (opcode_strings + 0x428),
            0x40a00000,
            0x89000000,
            (opcode_strings + 0xDDE),
            3,
            4
        );
        fn_00485D90(
            0x96,
            (opcode_strings + 0xDF7),
            (opcode_strings + 0x519),
            0xd4000000,
            0x4002,
            (opcode_strings + 0xDEE),
            0x8003,
            7
        );
        fn_00485D90(
            0x97,
            (opcode_strings + 0xE05),
            (opcode_strings + 0x544),
            0xd8000000,
            0x4002,
            (opcode_strings + 0xDFC),
            0x8000,
            7
        );
        fn_00485D90(
            0x98,
            (opcode_strings + 0xE13),
            (opcode_strings + 0x544),
            0xdc000000,
            0x4002,
            (opcode_strings + 0xE0A),
            0x8000,
            7
        );
        fn_00485D90(
            0xc5,
            (opcode_strings + 0xE21),
            (opcode_strings + 0x52F),
            0xf4000000,
            4,
            (opcode_strings + 0xE18),
            0x8003,
            1
        );
        fn_00485D90(
            0xc6,
            (opcode_strings + 0xE2F),
            (opcode_strings + 0x55A),
            0xf8000000,
            4,
            (opcode_strings + 0xE26),
            0x8000,
            1
        );
        fn_00485D90(
            199,
            (opcode_strings + 0xE3D),
            (opcode_strings + 0x55A),
            0xfc000000,
            4,
            (opcode_strings + 0xE34),
            0x8000,
            1
        );
    }
    if (copts.sqrt != '\0') {
        fn_00485D90(
            0x101,
            (opcode_strings + 0xE47),
            (opcode_strings + 0xE42),
            0x46000004,
            0x8004000,
            0,
            0x4003,
            0x1f
        );
        fn_00485D90(
            0x37a,
            (opcode_strings + 0xE47),
            (opcode_strings + 0x7EC),
            0x46000004,
            0x8000000,
            0,
            0x4003,
            0x1f
        );
        if ((copts.fp_library & 2) != 0) {
            fn_00485D90(
                0x100,
                (opcode_strings + 0xE4E),
                (opcode_strings + 0x6E6),
                0x46200004,
                0x4000,
                0,
                0x4003,
                0x3b
            );
            fn_00485D90(
                0x37b,
                (opcode_strings + 0xE4E),
                (opcode_strings + 0x7E1),
                0x46200004,
                0,
                0,
                0x4003,
                0x3b
            );
        }
    }
    if (copts.unknown31 != '\0') {
        fn_00485D90(
            0xe6, (opcode_strings + 0xE55), (opcode_strings + 0x80), 0x70000021, 0x4000, 0, 0, 1
        );
        fn_00485D90(
            0xe7, (opcode_strings + 0xE59), (opcode_strings + 0x80), 0x70000020, 0x4000, 0, 0, 1
        );
        fn_00485D90(
            0xe8, (opcode_strings + 0xE5D), (opcode_strings + 0x80), 0x70000025, 0x4000, 0, 0, 1
        );
        fn_00485D90(
            0xe9, (opcode_strings + 0xE62), (opcode_strings + 0x80), 0x70000024, 0x4000, 0, 0, 1
        );
    }
    if (copts.unknown0C >= 3) {
        fn_00485D90(
            0x76,
            (opcode_strings + 0xE70),
            (opcode_strings + 0x17),
            0x2c,
            0x6000,
            (opcode_strings + 0xE67),
            0xc000,
            1
        );
        fn_00485D90(
            0x79,
            (opcode_strings + 0xE7F),
            (opcode_strings + 0x17),
            0x2d,
            0x6000,
            (opcode_strings + 0xE75),
            0xc000,
            1
        );
        fn_00485D90(
            0xab,
            (opcode_strings + 0xE8F),
            (opcode_strings + 0x80),
            0x2d,
            0x4010,
            (opcode_strings + 0xE85),
            0xc000,
            1
        );
        fn_00485D90(
            0x8d,
            (opcode_strings + 0xE9E),
            (opcode_strings + 0x17),
            0x2e,
            0x4000,
            (opcode_strings + 0xE95),
            0xc000,
            1
        );
        fn_00485D90(
            0x8e,
            (opcode_strings + 0xEAD),
            (opcode_strings + 0x17),
            0x2f,
            0x4000,
            (opcode_strings + 0xEA3),
            0xc000,
            1
        );
        fn_00485D90(
            0x2fa, (opcode_strings + 0xE70), (opcode_strings + 0xB1), 0x2c, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x2fb, (opcode_strings + 0xE7F), (opcode_strings + 0xB1), 0x2d, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x2fc, (opcode_strings + 0xE9E), (opcode_strings + 0xB1), 0x2e, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x2fd, (opcode_strings + 0xEAD), (opcode_strings + 0xB1), 0x2f, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x77,
            (opcode_strings + 0xEBD),
            (opcode_strings + 0x147),
            0x60000000,
            0x4000,
            (opcode_strings + 0xEB3),
            0xc000,
            1
        );
        fn_00485D90(
            0x78,
            (opcode_strings + 0xECE),
            (opcode_strings + 0x147),
            0x64000000,
            0x4000,
            (opcode_strings + 0xEC3),
            0xc000,
            1
        );
        fn_00485D90(
            0x336, (opcode_strings + 0xEBD), (opcode_strings + 0x1B0), 0x60000000, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x337, (opcode_strings + 0xECE), (opcode_strings + 0x1B0), 0x64000000, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x7a,
            (opcode_strings + 0xEDE),
            (opcode_strings + 0x1EE),
            0x1e,
            0,
            (opcode_strings + 0xED5),
            0xc002,
            0x44
        );
        fn_00485D90(
            0x7b,
            (opcode_strings + 0xEED),
            (opcode_strings + 0x1EE),
            0x1f,
            0,
            (opcode_strings + 0xEE3),
            0xc002,
            0x44
        );
        fn_00485D90(
            0x80,
            (opcode_strings + 0xEFD),
            (opcode_strings + 0x1EE),
            0x1c,
            0,
            (opcode_strings + 0xEF3),
            0xc002,
            6
        );
        fn_00485D90(
            0x81,
            (opcode_strings + 0xF0E),
            (opcode_strings + 0x1EE),
            0x1d,
            0,
            (opcode_strings + 0xF03),
            0xc002,
            6
        );
        fn_00485D90(
            0x82,
            (opcode_strings + 0xF1E),
            (opcode_strings + 0xC1),
            0x2e,
            0x4000,
            (opcode_strings + 0xF15),
            0xc000,
            1
        );
        fn_00485D90(
            0x83,
            (opcode_strings + 0xF2D),
            (opcode_strings + 0xC1),
            0x2f,
            0x4000,
            (opcode_strings + 0xF23),
            0xc000,
            1
        );
        fn_00485D90(
            0x84,
            (opcode_strings + 0xF3C),
            (opcode_strings + 0xE0),
            0x38,
            0x4000,
            (opcode_strings + 0xF33),
            0xc000,
            1
        );
        fn_00485D90(
            0x87,
            (opcode_strings + 0xF4A),
            (opcode_strings + 0xE0),
            0x3b,
            0x4000,
            (opcode_strings + 0xF41),
            0xc000,
            1
        );
        fn_00485D90(
            0x8a,
            (opcode_strings + 0xF58),
            (opcode_strings + 0xE0),
            0x3a,
            0x4000,
            (opcode_strings + 0xF4F),
            0xc000,
            1
        );
        fn_00485D90(
            0x85,
            (opcode_strings + 0xF68),
            (opcode_strings + 0xE0),
            0x3c,
            0x4000,
            (opcode_strings + 0xF5D),
            0xc000,
            1
        );
        fn_00485D90(
            0x88,
            (opcode_strings + 0xF7A),
            (opcode_strings + 0xE0),
            0x3f,
            0x4000,
            (opcode_strings + 0xF6F),
            0xc000,
            1
        );
        fn_00485D90(
            0x8b,
            (opcode_strings + 0xF8C),
            (opcode_strings + 0xE0),
            0x3e,
            0x4000,
            (opcode_strings + 0xF81),
            0xc000,
            1
        );
        fn_00485D90(
            0x86,
            (opcode_strings + 0xF9D),
            (opcode_strings + 0x10C),
            0x14,
            0x4000,
            (opcode_strings + 0xF93),
            0xc000,
            1
        );
        fn_00485D90(
            0x89,
            (opcode_strings + 0xFAD),
            (opcode_strings + 0x10C),
            0x17,
            0x4000,
            (opcode_strings + 0xFA3),
            0xc000,
            1
        );
        fn_00485D90(
            0x8c,
            (opcode_strings + 0xFBD),
            (opcode_strings + 0x10C),
            0x16,
            0x4000,
            (opcode_strings + 0xFB3),
            0xc000,
            1
        );
        fn_00485D90(
            0x340, (opcode_strings + 0xF1E), (opcode_strings + 0xB6), 0x2e, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x341, (opcode_strings + 0xF2D), (opcode_strings + 0xB6), 0x2f, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x342, (opcode_strings + 0xF3C), (opcode_strings + 0x134), 0x38, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x343, (opcode_strings + 0xF4A), (opcode_strings + 0x134), 0x3b, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x344, (opcode_strings + 0xF58), (opcode_strings + 0x134), 0x3a, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x345, (opcode_strings + 0xF68), (opcode_strings + 0x134), 0x3c, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x346, (opcode_strings + 0xF7A), (opcode_strings + 0x134), 0x3f, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x347, (opcode_strings + 0xF8C), (opcode_strings + 0x134), 0x3e, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x348, (opcode_strings + 0xF9D), (opcode_strings + 0x139), 0x14, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x349, (opcode_strings + 0xFAD), (opcode_strings + 0x139), 0x17, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x34a, (opcode_strings + 0xFBD), (opcode_strings + 0x139), 0x16, 0, 0, 0xc000, 1
        );
        fn_00485D90(
            0x93,
            (opcode_strings + 0xFCA),
            (opcode_strings + 0x45A),
            0xdc000000,
            0x4002,
            (opcode_strings + 0xFC3),
            0x8000,
            7
        );
        fn_00485D90(
            0x99,
            (opcode_strings + 0xFD5),
            (opcode_strings + 0x499),
            0x68000000,
            0x4002,
            (opcode_strings + 0xFCD),
            0x8000,
            7
        );
        fn_00485D90(
            0x9a,
            (opcode_strings + 0xFE1),
            (opcode_strings + 0x499),
            0x6c000000,
            0x4002,
            (opcode_strings + 0xFD9),
            0x8000,
            7
        );
        fn_00485D90(
            0x9c,
            (opcode_strings + 0xFED),
            (opcode_strings + 0x45A),
            0xd0000000,
            2,
            (opcode_strings + 0xFE5),
            0x8000,
            7
        );
        fn_00485D90(
            0xc4,
            (opcode_strings + 0xFF8),
            (opcode_strings + 0x4C4),
            0xfc000000,
            4,
            (opcode_strings + 0xFF1),
            0x8000,
            1
        );
        fn_00485D90(
            200,
            (opcode_strings + 0x1003),
            (opcode_strings + 0x4C4),
            0xb0000000,
            4,
            (opcode_strings + 0xFFB),
            0x8000,
            1
        );
        fn_00485D90(
            0xc9,
            (opcode_strings + 0x100F),
            (opcode_strings + 0x4C4),
            0xb4000000,
            4,
            (opcode_strings + 0x1007),
            0x8000,
            1
        );
    }
    if ((copts.conditional_move != '\0') || (copts.late_conditional_move != '\0')) {
        fn_00485D90(0x110, (opcode_strings + 0x101A), (opcode_strings + 0x1013), 0xb, 0, 0, 0, 1);
        fn_00485D90(0x111, (opcode_strings + 0x101F), (opcode_strings + 0x1013), 10, 0, 0, 0, 1);
    }
    if (copts.prefetch != '\0') {
        fn_00485D90(
            0x65,
            (opcode_strings + 0x1024),
            (opcode_strings + 0x33F),
            0xcc000000,
            0x8000100,
            0,
            0x40000,
            1
        );
        fn_00485CF0(0x438, (opcode_strings + 0x1024), (opcode_strings + 0x1029), 0x65, 0, 1);
    }
    if (copts.unknown0C >= 4) {
        fn_00485D90(
            0x10a,
            (opcode_strings + 0x1036),
            (opcode_strings + 0x102D),
            0x4c000001,
            0x4002,
            0,
            0x8003,
            7
        );
        fn_00485D90(
            0x10b,
            (opcode_strings + 0x1044),
            (opcode_strings + 0x103C),
            0x4c000000,
            0x4002,
            0,
            0x8003,
            7
        );
        fn_00485D90(
            0x10c, (opcode_strings + 0x1052), (opcode_strings + 0x104A), 0x4c000009, 4, 0, 0x8003, 1
        );
        fn_00485D90(
            0x10d, (opcode_strings + 0x105F), (opcode_strings + 0x1058), 0x4c000008, 4, 0, 0x8003, 1
        );
        fn_00485D90(
            0x112, (opcode_strings + 0x106E), (opcode_strings + 0x1065), 1, 0, 0, 0x8003, 2
        );
        fn_00485D90(
            0x113, (opcode_strings + 0x1073), (opcode_strings + 0x1065), 0x10001, 0, 0, 0x8003, 2
        );
        fn_00485D90(
            0x114, (opcode_strings + 0x1083), (opcode_strings + 0x1078), 0x46200011, 0, 0, 0x8003, 2
        );
        fn_00485D90(
            0x115, (opcode_strings + 0x1093), (opcode_strings + 0x108A), 0x46000011, 0, 0, 0x8003, 2
        );
        fn_00485D90(
            0x116, (opcode_strings + 0x109A), (opcode_strings + 0x1078), 0x46210011, 0, 0, 0x8003, 2
        );
        fn_00485D90(
            0x117, (opcode_strings + 0x10A1), (opcode_strings + 0x108A), 0x46010011, 0, 0, 0x8003, 2
        );
        fn_00485D90(
            0x118, (opcode_strings + 0x10B1), (opcode_strings + 0x10A8), 0x46200013, 0, 0, 0x8003, 2
        );
        fn_00485D90(
            0x119, (opcode_strings + 0x10BF), (opcode_strings + 0x10B8), 0x46000013, 0, 0, 0x8003, 2
        );
        fn_00485D90(
            0x11a, (opcode_strings + 0x10C6), (opcode_strings + 0x10A8), 0x46200012, 0, 0, 0x8003, 2
        );
        fn_00485D90(
            0x11b, (opcode_strings + 0x10CD), (opcode_strings + 0x10B8), 0x46000012, 0, 0, 0x4003, 2
        );
        fn_00485D90(
            0x10e,
            (opcode_strings + 0x10E1),
            (opcode_strings + 0x10D4),
            0x4c000021,
            0x4000,
            0,
            0x4003,
            10
        );
        fn_00485D90(
            0x10f,
            (opcode_strings + 0x10F1),
            (opcode_strings + 0x10E8),
            0x4c000020,
            0x4000,
            0,
            0x4003,
            9
        );
        fn_00485D90(
            0x11c,
            (opcode_strings + 0x10F8),
            (opcode_strings + 0x10D4),
            0x4c000029,
            0x4000,
            0,
            0x4003,
            10
        );
        fn_00485D90(
            0x11d,
            (opcode_strings + 0x10FF),
            (opcode_strings + 0x10E8),
            0x4c000028,
            0x4000,
            0,
            0x4003,
            9
        );
        fn_00485D90(
            0x11e,
            (opcode_strings + 0x1106),
            (opcode_strings + 0x10D4),
            0x4c000031,
            0x4000,
            0,
            0x4003,
            10
        );
        fn_00485D90(
            0x11f,
            (opcode_strings + 0x110E),
            (opcode_strings + 0x10E8),
            0x4c000030,
            0x4000,
            0,
            0x4003,
            9
        );
        fn_00485D90(
            0x120,
            (opcode_strings + 0x1116),
            (opcode_strings + 0x10D4),
            0x4c000039,
            0x4000,
            0,
            0x4003,
            10
        );
        fn_00485D90(
            0x121,
            (opcode_strings + 0x111E),
            (opcode_strings + 0x10E8),
            0x4c000038,
            0x4000,
            0,
            0x4003,
            9
        );
        fn_00485D90(
            0x122,
            (opcode_strings + 0x1126),
            (opcode_strings + 0x6E6),
            0x46200015,
            0x4000,
            0,
            0x4003,
            0x3b
        );
        fn_00485D90(
            0x123,
            (opcode_strings + 0x112E),
            (opcode_strings + 0x936),
            0x46000015,
            0x4000,
            0,
            0x4003,
            0x1f
        );
    }
    if (copts.unknown23 != '\0') {
        fn_00485D90(
            0x125,
            (opcode_strings + 0x1136),
            (opcode_strings + 0x94B),
            0x46000016,
            0x8004000,
            0,
            0x4003,
            0x3d
        );
    }
    if (copts.unknown0C >= 5) {
        fn_00485D90(
            0x14e, (opcode_strings + 0x113E), (opcode_strings + 0x6E6), 0x46c00005, 0x4000, 0, 3, 2
        );
        fn_00485D90(
            0x14f, (opcode_strings + 0x1145), (opcode_strings + 0x6FD), 0x46c00000, 0x6000, 0, 3, 2
        );
        fn_00485D90(
            0x150, (opcode_strings + 0x1158), (opcode_strings + 0x114C), 0x4c00001e, 0, 0, 3, 2
        );
        fn_00485D90(
            0x151,
            (opcode_strings + 0x1160),
            (opcode_strings + 0x7FA),
            0x46c00032,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x152,
            (opcode_strings + 0x1168),
            (opcode_strings + 0x7FA),
            0x46c00030,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x153,
            (opcode_strings + 0x116F),
            (opcode_strings + 0x7FA),
            0x46c0003e,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x154,
            (opcode_strings + 0x1177),
            (opcode_strings + 0x7FA),
            0x46c0003c,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x155,
            (opcode_strings + 0x117F),
            (opcode_strings + 0x7FA),
            0x46c0003d,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x156,
            (opcode_strings + 0x1188),
            (opcode_strings + 0x7FA),
            0x46c0003b,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x157,
            (opcode_strings + 0x1191),
            (opcode_strings + 0x7FA),
            0x46c00039,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x158,
            (opcode_strings + 0x119B),
            (opcode_strings + 0x7FA),
            0x46c0003f,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x159,
            (opcode_strings + 0x11A4),
            (opcode_strings + 0x7FA),
            0x46c00036,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x15a,
            (opcode_strings + 0x11AD),
            (opcode_strings + 0x7FA),
            0x46c00034,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x15b,
            (opcode_strings + 0x11B6),
            (opcode_strings + 0x7FA),
            0x46c0003a,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x15c,
            (opcode_strings + 0x11BF),
            (opcode_strings + 0x7FA),
            0x46c00038,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x15d,
            (opcode_strings + 0x11C7),
            (opcode_strings + 0x7FA),
            0x46c00033,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x15e,
            (opcode_strings + 0x11D0),
            (opcode_strings + 0x7FA),
            0x46c00037,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x15f,
            (opcode_strings + 0x11D9),
            (opcode_strings + 0x7FA),
            0x46c00035,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x160,
            (opcode_strings + 0x11E2),
            (opcode_strings + 0x7FA),
            0x46c00031,
            0x8000000,
            0,
            3,
            2
        );
        fn_00485D90(
            0x161, (opcode_strings + 0x11F2), (opcode_strings + 0x11EA), 0x46000026, 0x4000, 0, 3, 3
        );
        fn_00485D90(
            0x162, (opcode_strings + 0x11FB), (opcode_strings + 0x6E6), 0x46c00028, 0x4000, 0, 3, 3
        );
        fn_00485D90(
            0x163, (opcode_strings + 0x1204), (opcode_strings + 0x6E6), 0x46c00020, 0x4000, 0, 3, 3
        );
        fn_00485D90(
            0x164, (opcode_strings + 0x120D), (opcode_strings + 0x102D), 0x4c000005, 2, 0, 3, 7
        );
        fn_00485D90(
            0x165, (opcode_strings + 0x1213), (opcode_strings + 0x10D4), 0x4c000025, 0, 0, 3, 3
        );
        fn_00485D90(
            0x166, (opcode_strings + 0x121B), (opcode_strings + 0x6E6), 0x46c00006, 0x4000, 0, 3, 1
        );
        fn_00485D90(
            0x16a, (opcode_strings + 0x1222), (opcode_strings + 0x6FD), 0x46c00002, 0x6000, 0, 3, 2
        );
        fn_00485D90(
            0x16b, (opcode_strings + 0x1229), (opcode_strings + 0x6E6), 0x46c00007, 0x4000, 0, 3, 2
        );
        fn_00485D90(
            0x16c, (opcode_strings + 0x1230), (opcode_strings + 0x10D4), 0x4c000036, 0, 0, 3, 3
        );
        fn_00485D90(
            0x16d, (opcode_strings + 0x1239), (opcode_strings + 0x10D4), 0x4c00003e, 0, 0, 3, 3
        );
        fn_00485D90(
            0x16e, (opcode_strings + 0x1242), (opcode_strings + 0x6FD), 0x46c0002c, 0x4000, 0, 3, 3
        );
        fn_00485D90(
            0x16f, (opcode_strings + 0x1249), (opcode_strings + 0x6FD), 0x46c0002d, 0x4000, 0, 3, 3
        );
        fn_00485D90(
            0x170, (opcode_strings + 0x1250), (opcode_strings + 0x6FD), 0x46c0002e, 0x4000, 0, 3, 3
        );
        fn_00485D90(
            0x171, (opcode_strings + 0x1257), (opcode_strings + 0x6FD), 0x46c0002f, 0x4000, 0, 3, 3
        );
        fn_00485D90(
            0x172, (opcode_strings + 0x125E), (opcode_strings + 0x6FD), 0x46c00001, 0x4000, 0, 3, 3
        );
        fn_00485D90(
            0x173, (opcode_strings + 0x126D), (opcode_strings + 0x1265), 0x4c00000d, 4, 0, 3, 1
        );
    }
    digits = (opcode_strings + 0xC09);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x1d1, (opcode_strings + 0x127F), (opcode_strings + 0x1276), encoding, 0x4002, 0, 0x8000, 7
    );
    digits = (opcode_strings + 0x1282);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x1d2, (opcode_strings + 0x12AB), (opcode_strings + 0x12A3), encoding, 4, 0, 0x8000, 1
    );
    digits = (opcode_strings + 0x12AE);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x108, (opcode_strings + 0x12CF), (opcode_strings + 0x0), encoding, 0x100, 0, 0x40000, 1
    );
    digits = (opcode_strings + 0x12D2);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x109, (opcode_strings + 0x12F3), (opcode_strings + 0x0), encoding, 0x100, 0, 0x40000, 1
    );
    digits = (opcode_strings + 0x12F6);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x1d5,
        (opcode_strings + 0x1321),
        (opcode_strings + 0x1317),
        encoding,
        0x2000,
        0,
        0x10010000,
        3
    );
    digits = (opcode_strings + 0x1326);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x1d6,
        (opcode_strings + 0x1347),
        (opcode_strings + 0x1317),
        encoding,
        0x2000,
        0,
        0x10010000,
        3
    );
    digits = (opcode_strings + 0x134D);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x1db,
        (opcode_strings + 0x210),
        (opcode_strings + 0x136E),
        encoding,
        0x6000,
        0,
        0x10010000,
        3
    );
    digits = (opcode_strings + 0x1378);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x1dc,
        (opcode_strings + 0x21F),
        (opcode_strings + 0x136E),
        encoding,
        0x6000,
        0,
        0x10010000,
        3
    );
    fn_00485D90(
        0x202,
        (opcode_strings + 0x13A3),
        (opcode_strings + 0x1399),
        0x70000018,
        0x6000,
        0,
        0x10020000,
        3
    );
    fn_00485D90(
        0x203,
        (opcode_strings + 0x13A9),
        (opcode_strings + 0x1399),
        0x70000019,
        0x6000,
        0,
        0x10020000,
        3
    );
    fn_00485D90(
        0x204, (opcode_strings + 0x13A3), (opcode_strings + 0x13B0), 0x70000018, 0, 0, 0x10020000, 3
    );
    fn_00485D90(
        0x205, (opcode_strings + 0x13A9), (opcode_strings + 0x13B0), 0x70000019, 0, 0, 0x10020000, 3
    );
    fn_00485D90(
        0x1d3, (opcode_strings + 0x13B7), (opcode_strings + 0x13B0), 0x7000001a, 0, 0, 0x20020000, 3
    );
    fn_00485D90(
        0x1d4, (opcode_strings + 0x13BC), (opcode_strings + 0x13B0), 0x7000001b, 0, 0, 0x20020000, 3
    );
    fn_00485D90(
        0x1d7,
        (opcode_strings + 0x13CC),
        (opcode_strings + 0x13C2),
        0x70000020,
        0x2000,
        0,
        0x10020000,
        3
    );
    fn_00485D90(
        0x1d8,
        (opcode_strings + 0x13D2),
        (opcode_strings + 0x13C2),
        0x70000021,
        0x2000,
        0,
        0x10020000,
        3
    );
    fn_00485D90(
        0x1d9, (opcode_strings + 0x13CC), (opcode_strings + 0x13D9), 0x70000020, 0, 0, 0x10020000, 3
    );
    fn_00485D90(
        0x1da, (opcode_strings + 0x13D2), (opcode_strings + 0x13D9), 0x70000021, 0, 0, 0x10020000, 3
    );
    fn_00485D90(
        0x1dd, (opcode_strings + 0x13E7), (opcode_strings + 0x13E0), 0x46000018, 0, 0, 3, 3
    );
    fn_00485D90(
        0x1e5, (opcode_strings + 0x13EE), (opcode_strings + 0x13E0), 0x46000019, 0, 0, 3, 3
    );
    fn_00485D90(
        0x1e4, (opcode_strings + 0x13F5), (opcode_strings + 0x13E0), 0x4600001a, 0, 0, 3, 3
    );
    fn_00485D90(
        0x1de, (opcode_strings + 0x10F1), (opcode_strings + 0x13FC), 0x4600001c, 0x2000, 0, 3, 3
    );
    fn_00485D90(
        0x1e2, (opcode_strings + 0x10FF), (opcode_strings + 0x13FC), 0x4600001d, 0x2000, 0, 3, 3
    );
    fn_00485D90(
        0x1df, (opcode_strings + 0x140C), (opcode_strings + 0x1405), 0x4600001e, 0, 0, 3, 3
    );
    fn_00485D90(
        0x1e3, (opcode_strings + 0x1414), (opcode_strings + 0x1405), 0x4600001f, 0, 0, 3, 3
    );
    fn_00485D90(
        0x1e0, (opcode_strings + 0x1426), (opcode_strings + 0x141C), 0x46000028, 0x6000, 0, 3, 3
    );
    fn_00485D90(
        0x1e1, (opcode_strings + 0x142C), (opcode_strings + 0x141C), 0x46000029, 0x6000, 0, 3, 3
    );
    fn_00485D90(
        0x1e8,
        (opcode_strings + 0x1437),
        (opcode_strings + 0x1432),
        0x4000c000,
        0x100,
        0,
        0x40000,
        1
    );
    fn_00485D90(
        0x1e9,
        (opcode_strings + 0x143D),
        (opcode_strings + 0x1432),
        0x4000c004,
        0x100,
        0,
        0x40000,
        1
    );
    fn_00485D90(
        0x1ea,
        (opcode_strings + 0x1443),
        (opcode_strings + 0x1432),
        0x4000c005,
        0x100,
        0,
        0x40000,
        1
    );
    fn_00485D90(
        0x1eb,
        (opcode_strings + 0x144A),
        (opcode_strings + 0x1432),
        0x4000c006,
        0x100,
        0,
        0x40000,
        1
    );
    fn_00485D90(
        0x1ec,
        (opcode_strings + 0x1450),
        (opcode_strings + 0x1432),
        0x4000c007,
        0x100,
        0,
        0x40000,
        1
    );
    fn_00485D90(
        0x1ed,
        (opcode_strings + 0x1457),
        (opcode_strings + 0x1432),
        0x4000c002,
        0x100,
        0,
        0x40000,
        1
    );
    fn_00485D90(
        0x1ee,
        (opcode_strings + 0x145D),
        (opcode_strings + 0x1432),
        0x4000c003,
        0x100,
        0,
        0x40000,
        1
    );
    fn_00485D90(
        0x1ef,
        (opcode_strings + 0x1469),
        (opcode_strings + 0x1464),
        0x4080c000,
        0x100,
        0,
        0x40000,
        1
    );
    fn_00485D90(
        0x1f0,
        (opcode_strings + 0x146F),
        (opcode_strings + 0x1464),
        0x4080c004,
        0x100,
        0,
        0x40000,
        1
    );
    fn_00485D90(
        0x1f1,
        (opcode_strings + 0x1475),
        (opcode_strings + 0x1464),
        0x4080c005,
        0x100,
        0,
        0x40000,
        1
    );
    fn_00485D90(
        0x1f2,
        (opcode_strings + 0x147C),
        (opcode_strings + 0x1464),
        0x4080c006,
        0x100,
        0,
        0x40000,
        1
    );
    fn_00485D90(
        499, (opcode_strings + 0x1482), (opcode_strings + 0x1464), 0x4080c007, 0x100, 0, 0x40000, 1
    );
    fn_00485D90(
        500, (opcode_strings + 0x1489), (opcode_strings + 0x1464), 0x4080c002, 0x100, 0, 0x40000, 1
    );
    fn_00485D90(
        0x1f5,
        (opcode_strings + 0x148F),
        (opcode_strings + 0x1464),
        0x4080c003,
        0x100,
        0,
        0x40000,
        1
    );
    fn_00485D90(
        0x1fa,
        (opcode_strings + 0x149B),
        (opcode_strings + 0x1496),
        0x70000010,
        0x8004000,
        0,
        0x20000,
        3
    );
    fn_00485D90(
        0x1fb,
        (opcode_strings + 0x14A1),
        (opcode_strings + 0x1496),
        0x70000012,
        0x8004000,
        0,
        0x20000,
        3
    );
    fn_00485D90(
        0x1fd,
        (opcode_strings + 0x14AC),
        (opcode_strings + 0x14A7),
        0x70000011,
        0x8004000,
        0,
        0x20000,
        3
    );
    fn_00485D90(
        0x1fe,
        (opcode_strings + 0x14B2),
        (opcode_strings + 0x14A7),
        0x70000013,
        0x8004000,
        0,
        0x20000,
        3
    );
    digits = (opcode_strings + 0x14B8);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x1f6, (opcode_strings + 0x14DE), (opcode_strings + 0x14D9), encoding, 0, 0, 0x48000, 1
    );
    digits = (opcode_strings + 0x14E3);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x1f7, (opcode_strings + 0x1504), (opcode_strings + 0x14D9), encoding, 0, 0, 0x48000, 1
    );
    digits = (opcode_strings + 0x1509);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x1f8, (opcode_strings + 0x152F), (opcode_strings + 0x152A), encoding, 0, 0, 0x80048000, 1
    );
    digits = (opcode_strings + 0x1534);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x1f9, (opcode_strings + 0x1555), (opcode_strings + 0x152A), encoding, 0, 0, 0x80048000, 1
    );
    fn_00485D90(
        0x1fc, (opcode_strings + 0x155F), (opcode_strings + 0x155A), 0x28, 0x100, 0, 0x10000, 1
    );
    fn_00485D90(
        0x1ff, (opcode_strings + 0x1569), (opcode_strings + 0x1564), 0x29, 0x100, 0, 0x10000, 1
    );
    fn_00485D90(
        0x200, (opcode_strings + 0x1575), (opcode_strings + 0x156E), 0x4180000, 0x100, 0, 0x10000, 1
    );
    fn_00485D90(
        0x201, (opcode_strings + 0x157B), (opcode_strings + 0x156E), 0x4190000, 0x100, 0, 0x10000, 1
    );
    fn_00485D90(
        0x1cc,
        (opcode_strings + 0x1588),
        (opcode_strings + 0x1581),
        0x70000628,
        0x4010,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x206,
        (opcode_strings + 0x158E),
        (opcode_strings + 0xC1),
        0x70000168,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x207,
        (opcode_strings + 0x1594),
        (opcode_strings + 0xC1),
        0x70000068,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x219,
        (opcode_strings + 0x159A),
        (opcode_strings + 0xC1),
        0x700006e9,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x21f,
        (opcode_strings + 0x15A0),
        (opcode_strings + 0xC1),
        0x700006a9,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x220,
        (opcode_strings + 0x15A6),
        (opcode_strings + 0xC1),
        0x700007a9,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x221,
        (opcode_strings + 0x15AC),
        (opcode_strings + 0xC1),
        0x70000689,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x222,
        (opcode_strings + 0x15B2),
        (opcode_strings + 0xC1),
        0x70000689,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x223,
        (opcode_strings + 0x15B8),
        (opcode_strings + 0xC1),
        0x70000789,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x224,
        (opcode_strings + 0x15BE),
        (opcode_strings + 0xC1),
        0x70000789,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x225,
        (opcode_strings + 0x15C4),
        (opcode_strings + 0xC1),
        0x70000788,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x249,
        (opcode_strings + 0x15CA),
        (opcode_strings + 0xC1),
        0x700007c8,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x24d,
        (opcode_strings + 0x15D0),
        (opcode_strings + 0xC1),
        0x700006c9,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x24e,
        (opcode_strings + 0x15D6),
        (opcode_strings + 0xC1),
        0x700007c9,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x230,
        (opcode_strings + 0x15DD),
        (opcode_strings + 0x80),
        0x70000004,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x236,
        (opcode_strings + 0x15E3),
        (opcode_strings + 0x587),
        0x70000209,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x237,
        (opcode_strings + 0x15E9),
        (opcode_strings + 0x587),
        0x70000249,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x238,
        (opcode_strings + 0x15F3),
        (opcode_strings + 0x15EF),
        0x70000030,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x239,
        (opcode_strings + 0x15FC),
        (opcode_strings + 0x15EF),
        0x70000070,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x23a,
        (opcode_strings + 0x1605),
        (opcode_strings + 0x15EF),
        0x700000b0,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x23b,
        (opcode_strings + 0x160F),
        (opcode_strings + 0x15EF),
        0x700000f0,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x23c,
        (opcode_strings + 0x1618),
        (opcode_strings + 0x15EF),
        0x70000130,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x241,
        (opcode_strings + 0x1621),
        (opcode_strings + 0x5A8),
        0x70000229,
        0x80004000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x242,
        (opcode_strings + 0x1627),
        (opcode_strings + 0x5A8),
        0x70000269,
        0x80004000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x243,
        (opcode_strings + 0x162D),
        (opcode_strings + 0x5A8),
        0x70000031,
        0x80004000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x208,
        (opcode_strings + 0x1636),
        (opcode_strings + 0x17),
        0x70000208,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x209,
        (opcode_strings + 0x163C),
        (opcode_strings + 0x17),
        0x70000108,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x20a,
        (opcode_strings + 0x1642),
        (opcode_strings + 0x17),
        0x70000008,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x20b,
        (opcode_strings + 0x1648),
        (opcode_strings + 0x17),
        0x70000608,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x20c,
        (opcode_strings + 0x164F),
        (opcode_strings + 0x17),
        0x70000508,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x20d,
        (opcode_strings + 0x1656),
        (opcode_strings + 0x17),
        0x70000408,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x20e,
        (opcode_strings + 0x165D),
        (opcode_strings + 0x17),
        0x70000628,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x20f,
        (opcode_strings + 0x1664),
        (opcode_strings + 0x17),
        0x70000528,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x210,
        (opcode_strings + 0x166B),
        (opcode_strings + 0x17),
        0x70000428,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x211,
        (opcode_strings + 0x1672),
        (opcode_strings + 0x17),
        0x70000128,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x212,
        (opcode_strings + 0x1679),
        (opcode_strings + 0x17),
        0x70000489,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x213,
        (opcode_strings + 0x167E),
        (opcode_strings + 0x17),
        0x700002a8,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x214,
        (opcode_strings + 0x1684),
        (opcode_strings + 0x17),
        0x700001a8,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x215,
        (opcode_strings + 0x168A),
        (opcode_strings + 0x17),
        0x700000a8,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x216,
        (opcode_strings + 0x1690),
        (opcode_strings + 0x17),
        0x70000288,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x217,
        (opcode_strings + 0x1696),
        (opcode_strings + 0x17),
        0x70000188,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x218,
        (opcode_strings + 0x169C),
        (opcode_strings + 0x17),
        0x70000088,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x21a,
        (opcode_strings + 0x16A2),
        (opcode_strings + 0x17),
        0x70000389,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x21b,
        (opcode_strings + 0x16A9),
        (opcode_strings + 0x17),
        0x700003a9,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x226,
        (opcode_strings + 0x16B0),
        (opcode_strings + 0x17),
        0x70000688,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x227,
        (opcode_strings + 0x16B7),
        (opcode_strings + 0x17),
        0x70000588,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x228,
        (opcode_strings + 0x16BE),
        (opcode_strings + 0x17),
        0x70000488,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x229,
        (opcode_strings + 0x16C5),
        (opcode_strings + 0x17),
        0x700006a8,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x22a,
        (opcode_strings + 0x16CC),
        (opcode_strings + 0x17),
        0x700005a8,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x22b,
        (opcode_strings + 0x16D3),
        (opcode_strings + 0x17),
        0x700004a8,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x22e,
        (opcode_strings + 0x16DA),
        (opcode_strings + 0x17),
        0x70000289,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x22f,
        (opcode_strings + 0x16E0),
        (opcode_strings + 0x17),
        0x700002a9,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x234,
        (opcode_strings + 0x16E7),
        (opcode_strings + 0x17),
        0x700001c8,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x235,
        (opcode_strings + 0x16ED),
        (opcode_strings + 0x17),
        0x700000c8,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x23d,
        (opcode_strings + 0x16F3),
        (opcode_strings + 0x17),
        0x700001e8,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x23e,
        (opcode_strings + 0x16F9),
        (opcode_strings + 0x17),
        0x700000e8,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x23f,
        (opcode_strings + 0x16FF),
        (opcode_strings + 0x17),
        0x70000509,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x240,
        (opcode_strings + 0x1706),
        (opcode_strings + 0x17),
        0x70000109,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x247,
        (opcode_strings + 0x170D),
        (opcode_strings + 0x17),
        0x700004e9,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x248,
        (opcode_strings + 0x1712),
        (opcode_strings + 0x17),
        0x700004a9,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x24a,
        (opcode_strings + 0x1716),
        (opcode_strings + 0x17),
        0x700006c8,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x24b,
        (opcode_strings + 0x171C),
        (opcode_strings + 0x17),
        0x700005c8,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x24c,
        (opcode_strings + 0x1722),
        (opcode_strings + 0x17),
        0x700004c8,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        600, (opcode_strings + 0x1728), (opcode_strings + 0x17), 0x70000248, 0x4000, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x259,
        (opcode_strings + 0x172E),
        (opcode_strings + 0x17),
        0x70000148,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x25a,
        (opcode_strings + 0x1734),
        (opcode_strings + 0x17),
        0x70000648,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x25b,
        (opcode_strings + 0x173B),
        (opcode_strings + 0x17),
        0x70000548,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x25c,
        (opcode_strings + 0x1742),
        (opcode_strings + 0x17),
        0x70000448,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x25d,
        (opcode_strings + 0x1749),
        (opcode_strings + 0x17),
        0x70000668,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x25e,
        (opcode_strings + 0x1750),
        (opcode_strings + 0x17),
        0x70000568,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x25f,
        (opcode_strings + 0x1757),
        (opcode_strings + 0x17),
        0x70000468,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x260,
        (opcode_strings + 0x175E),
        (opcode_strings + 0x17),
        0x70000048,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x261,
        (opcode_strings + 0x1764),
        (opcode_strings + 0x17),
        0x700004c9,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x262, (opcode_strings + 0x1772), (opcode_strings + 0x1769), 0x700006e8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x2fe, (opcode_strings + 0x1636), (opcode_strings + 0xB1), 0x70000208, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x2ff, (opcode_strings + 0x163C), (opcode_strings + 0xB1), 0x70000108, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x300, (opcode_strings + 0x1642), (opcode_strings + 0xB1), 0x70000008, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x301, (opcode_strings + 0x1648), (opcode_strings + 0xB1), 0x70000608, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x302, (opcode_strings + 0x164F), (opcode_strings + 0xB1), 0x70000508, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x303, (opcode_strings + 0x1656), (opcode_strings + 0xB1), 0x70000408, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x304, (opcode_strings + 0x165D), (opcode_strings + 0xB1), 0x70000628, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x305, (opcode_strings + 0x1664), (opcode_strings + 0xB1), 0x70000528, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x306, (opcode_strings + 0x166B), (opcode_strings + 0xB1), 0x70000428, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x307, (opcode_strings + 0x1672), (opcode_strings + 0xB1), 0x70000128, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x308, (opcode_strings + 0x1679), (opcode_strings + 0xB1), 0x70000489, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x309, (opcode_strings + 0x167E), (opcode_strings + 0xB1), 0x700002a8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x30a, (opcode_strings + 0x1684), (opcode_strings + 0xB1), 0x700001a8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x30b, (opcode_strings + 0x168A), (opcode_strings + 0xB1), 0x700000a8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x30c, (opcode_strings + 0x1690), (opcode_strings + 0xB1), 0x70000288, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x30d, (opcode_strings + 0x1696), (opcode_strings + 0xB1), 0x70000188, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x30e, (opcode_strings + 0x169C), (opcode_strings + 0xB1), 0x70000088, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x30f, (opcode_strings + 0x16A2), (opcode_strings + 0xB1), 0x70000389, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x310, (opcode_strings + 0x16A9), (opcode_strings + 0xB1), 0x700003a9, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x311, (opcode_strings + 0x16B0), (opcode_strings + 0xB1), 0x70000688, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x312, (opcode_strings + 0x16B7), (opcode_strings + 0xB1), 0x70000588, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x313, (opcode_strings + 0x16BE), (opcode_strings + 0xB1), 0x70000488, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x314, (opcode_strings + 0x16C5), (opcode_strings + 0xB1), 0x700006a8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x315, (opcode_strings + 0x16CC), (opcode_strings + 0xB1), 0x700005a8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x316, (opcode_strings + 0x16D3), (opcode_strings + 0xB1), 0x700004a8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x317, (opcode_strings + 0x16DA), (opcode_strings + 0xB1), 0x70000289, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x318, (opcode_strings + 0x16E0), (opcode_strings + 0xB1), 0x700002a9, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x319, (opcode_strings + 0x16E7), (opcode_strings + 0xB1), 0x700001c8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x31a, (opcode_strings + 0x16ED), (opcode_strings + 0xB1), 0x700000c8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x31b, (opcode_strings + 0x16F3), (opcode_strings + 0xB1), 0x700001e8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x31c, (opcode_strings + 0x16F9), (opcode_strings + 0xB1), 0x700000e8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x31d, (opcode_strings + 0x16FF), (opcode_strings + 0xB1), 0x70000509, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x31e, (opcode_strings + 0x1706), (opcode_strings + 0xB1), 0x70000109, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        799, (opcode_strings + 0x170D), (opcode_strings + 0xB1), 0x700004e9, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        800, (opcode_strings + 0x1712), (opcode_strings + 0xB1), 0x700004a9, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x321, (opcode_strings + 0x1716), (opcode_strings + 0xB1), 0x700006c8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x322, (opcode_strings + 0x171C), (opcode_strings + 0xB1), 0x700005c8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x323, (opcode_strings + 0x1722), (opcode_strings + 0xB1), 0x700004c8, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x324, (opcode_strings + 0x1728), (opcode_strings + 0xB1), 0x70000248, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x325, (opcode_strings + 0x172E), (opcode_strings + 0xB1), 0x70000148, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x326, (opcode_strings + 0x1734), (opcode_strings + 0xB1), 0x70000648, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x327, (opcode_strings + 0x173B), (opcode_strings + 0xB1), 0x70000548, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x328, (opcode_strings + 0x1742), (opcode_strings + 0xB1), 0x70000448, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x329, (opcode_strings + 0x1749), (opcode_strings + 0xB1), 0x70000668, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x32a, (opcode_strings + 0x1750), (opcode_strings + 0xB1), 0x70000568, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x32b, (opcode_strings + 0x1757), (opcode_strings + 0xB1), 0x70000468, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x32c, (opcode_strings + 0x175E), (opcode_strings + 0xB1), 0x70000048, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x32d, (opcode_strings + 0x1764), (opcode_strings + 0xB1), 0x700004c9, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x22c,
        (opcode_strings + 0x1778),
        (opcode_strings + 0x136E),
        0x70000449,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x22d,
        (opcode_strings + 0x177F),
        (opcode_strings + 0x136E),
        0x70000549,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x231,
        (opcode_strings + 0x1786),
        (opcode_strings + 0x136E),
        0x70000409,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x232,
        (opcode_strings + 0x178D),
        (opcode_strings + 0x136E),
        0x70000029,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x233,
        (opcode_strings + 0x1795),
        (opcode_strings + 0x136E),
        0x70000009,
        0x6000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x244,
        (opcode_strings + 0x179C),
        (opcode_strings + 0x136E),
        0x70000709,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x245,
        (opcode_strings + 0x17A3),
        (opcode_strings + 0x136E),
        0x70000329,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x246,
        (opcode_strings + 0x17AB),
        (opcode_strings + 0x136E),
        0x70000309,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x250,
        (opcode_strings + 0x17B2),
        (opcode_strings + 0x10C),
        0x70000089,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x24f,
        (opcode_strings + 0x17B9),
        (opcode_strings + 0xE0),
        0x70000034,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x251,
        (opcode_strings + 0x17BF),
        (opcode_strings + 0xE0),
        0x7000003c,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x252,
        (opcode_strings + 0x17C5),
        (opcode_strings + 0xE0),
        0x70000037,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x253,
        (opcode_strings + 0x17CB),
        (opcode_strings + 0x10C),
        0x700000e9,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x254,
        (opcode_strings + 0x17D2),
        (opcode_strings + 0xE0),
        0x7000003f,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x255,
        (opcode_strings + 0x17D8),
        (opcode_strings + 0xE0),
        0x70000036,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x256,
        (opcode_strings + 0x17DE),
        (opcode_strings + 0x10C),
        0x700000c9,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        599, (opcode_strings + 0x17E5), (opcode_strings + 0xE0), 0x7000003e, 0x4000, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x34b, (opcode_strings + 0x17B2), (opcode_strings + 0x139), 0x70000089, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x34c, (opcode_strings + 0x17B9), (opcode_strings + 0x134), 0x70000034, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x34d, (opcode_strings + 0x17BF), (opcode_strings + 0x134), 0x7000003c, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x34e, (opcode_strings + 0x17C5), (opcode_strings + 0x134), 0x70000037, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x34f, (opcode_strings + 0x17CB), (opcode_strings + 0x139), 0x700000e9, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x350, (opcode_strings + 0x17D2), (opcode_strings + 0x134), 0x7000003f, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x351, (opcode_strings + 0x17D8), (opcode_strings + 0x134), 0x70000036, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x352, (opcode_strings + 0x17DE), (opcode_strings + 0x139), 0x700000c9, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x353, (opcode_strings + 0x17E5), (opcode_strings + 0x134), 0x7000003e, 0, 0, 0x1000000, 1
    );
    fn_00485D90(
        0x21c,
        (opcode_strings + 0x17F3),
        (opcode_strings + 0x17EB),
        0x70000749,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x21d,
        (opcode_strings + 0x17FA),
        (opcode_strings + 0x17EB),
        0x70000369,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x21e,
        (opcode_strings + 0x1801),
        (opcode_strings + 0x17EB),
        0x70000349,
        0x4000,
        0,
        0x1000000,
        1
    );
    fn_00485D90(
        0x269, (opcode_strings + 0x180F), (opcode_strings + 0x1807), 0x4a0001fd, 0, 0, 4, 1
    );
    fn_00485D90(
        0x26a, (opcode_strings + 0x181F), (opcode_strings + 0x1814), 0x4a000028, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x26b, (opcode_strings + 0x1831), (opcode_strings + 0x1824), 0x4a000022, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x26c, (opcode_strings + 0x1844), (opcode_strings + 0x1837), 0x4a000020, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x26d, (opcode_strings + 0x1855), (opcode_strings + 0x184A), 0x4a000003, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x26e, (opcode_strings + 0x185B), (opcode_strings + 0x184A), 0x4a000000, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x26f, (opcode_strings + 0x1861), (opcode_strings + 0x184A), 0x4a000001, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x270, (opcode_strings + 0x1867), (opcode_strings + 0x184A), 0x4a000002, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x271, (opcode_strings + 0x1878), (opcode_strings + 0x186D), 0x4a0002bc, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x272, (opcode_strings + 0x188B), (opcode_strings + 0x187E), 0x4a00023e, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x273, (opcode_strings + 0x189F), (opcode_strings + 0x1892), 0x4a00023c, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x274, (opcode_strings + 0x18B1), (opcode_strings + 0x18A6), 0x4a00003f, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x275, (opcode_strings + 0x18B8), (opcode_strings + 0x18A6), 0x4a00003c, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x276, (opcode_strings + 0x18BF), (opcode_strings + 0x18A6), 0x4a00003d, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x277, (opcode_strings + 0x18C6), (opcode_strings + 0x18A6), 0x4a00003e, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x278, (opcode_strings + 0x18D0), (opcode_strings + 0x18CD), 0x4a000038, 0x8000001, 0, 4, 1
    );
    fn_00485D90(
        0x279, (opcode_strings + 0x18D0), (opcode_strings + 0x18D8), 0x4a000038, 0x8000001, 0, 4, 1
    );
    digits = (opcode_strings + 0x18DB);
    encoding = 0;
    digit = *digits;
    while (digit != '\0') {
        encoding += encoding;
        digits += 1;
        encoding |= (int)digit - 0x30U;
        digit = *digits;
    }
    fn_00485D90(
        0x27a, (opcode_strings + 0x18FE), (opcode_strings + 0x18FC), encoding, 0x8000009, 0, 4, 1
    );
    fn_00485D90(
        0x27b, (opcode_strings + 0x190D), (opcode_strings + 0x1907), 0x4bc001ff, 0x100, 0, 4, 1
    );
    opcode_info[0x27B].unk4 = (opcode_strings + 0x1914);
    fn_00485D90(
        0x27c, (opcode_strings + 0x192E), (opcode_strings + 0x191F), 0x4a0003bc, 0, 0, 4, 1
    );
    fn_00485D90(
        0x27d, (opcode_strings + 0x1933), (opcode_strings + 0x1807), 0x4a00017c, 0, 0, 4, 1
    );
    fn_00485D90(
        0x27e, (opcode_strings + 0x193A), (opcode_strings + 0x1807), 0x4a00017d, 0, 0, 4, 1
    );
    fn_00485D90(
        0x27f, (opcode_strings + 0x1941), (opcode_strings + 0x1807), 0x4a00017e, 0, 0, 4, 1
    );
    fn_00485D90(
        0x280, (opcode_strings + 0x1949), (opcode_strings + 0x1807), 0x4a00017f, 0, 0, 4, 1
    );
    fn_00485D90(
        0x281, (opcode_strings + 0x195B), (opcode_strings + 0x1951), 0x4a000030, 0, 0, 4, 1
    );
    fn_00485D90(
        0x282, (opcode_strings + 0x196A), (opcode_strings + 0x1961), 0x4a000032, 0, 0, 4, 1
    );
    fn_00485D90(
        0x283, (opcode_strings + 0x1971), (opcode_strings + 0x1951), 0x4a000034, 0, 0, 4, 1
    );
    fn_00485D90(
        0x284, (opcode_strings + 0x1980), (opcode_strings + 0x1977), 0x4b0003fe, 2, 0, 4, 1
    );
    fn_00485D90(
        0x285, (opcode_strings + 0x1988), (opcode_strings + 0x1977), 0x4a8003fe, 2, 0, 4, 1
    );
    fn_00485D90(
        0x286, (opcode_strings + 0x1990), (opcode_strings + 0x1977), 0x4a4003fe, 2, 0, 4, 1
    );
    fn_00485D90(
        0x287, (opcode_strings + 0x1998), (opcode_strings + 0x1977), 0x4a2003fe, 2, 0, 4, 1
    );
    fn_00485D90(
        0x288, (opcode_strings + 0x19A0), (opcode_strings + 0x1951), 0x4a000035, 0, 0, 4, 1
    );
    fn_00485D90(
        0x289, (opcode_strings + 0x19AD), (opcode_strings + 0x19A5), 0x4b0003ff, 4, 0, 4, 1
    );
    fn_00485D90(
        0x28a, (opcode_strings + 0x19B5), (opcode_strings + 0x19A5), 0x4a8003ff, 4, 0, 4, 1
    );
    fn_00485D90(
        0x28b, (opcode_strings + 0x19BD), (opcode_strings + 0x19A5), 0x4a4003ff, 4, 0, 4, 1
    );
    fn_00485D90(
        0x28c, (opcode_strings + 0x19C5), (opcode_strings + 0x19A5), 0x4a2003ff, 4, 0, 4, 1
    );
    fn_00485D90(
        0x28d, (opcode_strings + 0x19CD), (opcode_strings + 0x1951), 0x4a000031, 0, 0, 4, 1
    );
    fn_00485D90(
        0x28e, (opcode_strings + 0x19D3), (opcode_strings + 0x1807), 0x4a00013c, 0, 0, 4, 1
    );
    fn_00485D90(
        0x28f, (opcode_strings + 0x19DA), (opcode_strings + 0x1807), 0x4a00013d, 0, 0, 4, 1
    );
    fn_00485D90(
        0x290, (opcode_strings + 0x19E1), (opcode_strings + 0x1807), 0x4a00013e, 0, 0, 4, 1
    );
    fn_00485D90(
        0x291, (opcode_strings + 0x19E9), (opcode_strings + 0x1807), 0x4a00013f, 0, 0, 4, 1
    );
    fn_00485D90(
        0x292, (opcode_strings + 0x19FC), (opcode_strings + 0x19F1), 0x4a00037e, 0, 0, 4, 7
    );
    fn_00485D90(
        0x293, (opcode_strings + 0x1A0C), (opcode_strings + 0x1A01), 0x4a00037c, 0, 0, 4, 7
    );
    fn_00485D90(
        0x294, (opcode_strings + 0x1A1F), (opcode_strings + 0x1A11), 0x4a000029, 0, 0, 4, 1
    );
    fn_00485D90(
        0x295, (opcode_strings + 0x1A35), (opcode_strings + 0x1A25), 0x4a000023, 0, 0, 4, 1
    );
    fn_00485D90(
        0x296, (opcode_strings + 0x1A4C), (opcode_strings + 0x1A3C), 0x4a000021, 0, 0, 4, 1
    );
    fn_00485D90(
        0x297, (opcode_strings + 0x1A61), (opcode_strings + 0x1A53), 0x4a00000b, 0, 0, 4, 1
    );
    fn_00485D90(
        0x298, (opcode_strings + 0x1A68), (opcode_strings + 0x1A53), 0x4a000008, 0, 0, 4, 1
    );
    fn_00485D90(
        0x299, (opcode_strings + 0x1A6F), (opcode_strings + 0x1A53), 0x4a000009, 0, 0, 4, 1
    );
    fn_00485D90(
        0x29a, (opcode_strings + 0x1A76), (opcode_strings + 0x1A53), 0x4a00000a, 0, 0, 4, 1
    );
    fn_00485D90(
        0x29b, (opcode_strings + 0x1A8B), (opcode_strings + 0x1A7D), 0x4a0002bd, 0, 0, 4, 1
    );
    fn_00485D90(
        0x29c, (opcode_strings + 0x1AA2), (opcode_strings + 0x1A92), 0x4a00023f, 0, 0, 4, 1
    );
    fn_00485D90(
        0x29d, (opcode_strings + 0x1ABA), (opcode_strings + 0x1AAA), 0x4a00023d, 0, 0, 4, 1
    );
    fn_00485D90(
        0x29e, (opcode_strings + 0x1AC2), (opcode_strings + 0x18A6), 0x4a0000bf, 0, 0, 4, 1
    );
    fn_00485D90(
        0x29f, (opcode_strings + 0x1ACA), (opcode_strings + 0x18A6), 0x4a0000bc, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2a0, (opcode_strings + 0x1AD2), (opcode_strings + 0x18A6), 0x4a0000bd, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2a1, (opcode_strings + 0x1ADA), (opcode_strings + 0x18A6), 0x4a0000be, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2a2, (opcode_strings + 0x1AE2), (opcode_strings + 0x1814), 0x4a00002b, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2a3, (opcode_strings + 0x1AE7), (opcode_strings + 0x1824), 0x4a00001d, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2a4, (opcode_strings + 0x1AED), (opcode_strings + 0x184A), 0x4a000013, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2a5, (opcode_strings + 0x1AF3), (opcode_strings + 0x184A), 0x4a000010, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2a6, (opcode_strings + 0x1AF9), (opcode_strings + 0x184A), 0x4a000011, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2a7, (opcode_strings + 0x1AFF), (opcode_strings + 0x184A), 0x4a000012, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2a8, (opcode_strings + 0x1B0D), (opcode_strings + 0x1B05), 0x4a0003fd, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2a9, (opcode_strings + 0x1B13), (opcode_strings + 0x1814), 0x4a00002f, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2aa, (opcode_strings + 0x1B24), (opcode_strings + 0x1B19), 0x4a00001f, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2ab, (opcode_strings + 0x1B2B), (opcode_strings + 0x184A), 0x4a000017, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2ac, (opcode_strings + 0x1B32), (opcode_strings + 0x184A), 0x4a000014, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2ad, (opcode_strings + 0x1B39), (opcode_strings + 0x184A), 0x4a000015, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2ae, (opcode_strings + 0x1B40), (opcode_strings + 0x184A), 0x4a000016, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2af, (opcode_strings + 0x1B4F), (opcode_strings + 0x1B47), 0x4a00033c, 0x4010, 0, 4, 1
    );
    fn_00485D90(
        0x2b0, (opcode_strings + 0x1B55), (opcode_strings + 0x1B47), 0x4a00033d, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2b1, (opcode_strings + 0x1B5B), (opcode_strings + 0x1A11), 0x4a00002d, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2b2, (opcode_strings + 0x1B61), (opcode_strings + 0x1A25), 0x4a000027, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2b3, (opcode_strings + 0x1B68), (opcode_strings + 0x1A3C), 0x4a000025, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2b4, (opcode_strings + 0x1B6F), (opcode_strings + 0x1A53), 0x4a00000f, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2b5, (opcode_strings + 0x1B76), (opcode_strings + 0x1A53), 0x4a00000c, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2b6, (opcode_strings + 0x1B7D), (opcode_strings + 0x1A53), 0x4a00000d, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2b7, (opcode_strings + 0x1B84), (opcode_strings + 0x1A53), 0x4a00000e, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2b8, (opcode_strings + 0x1B8B), (opcode_strings + 0x186D), 0x4a0002fd, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2b9, (opcode_strings + 0x1B92), (opcode_strings + 0x187E), 0x4a00027f, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2ba, (opcode_strings + 0x1B9A), (opcode_strings + 0x1892), 0x4a00027d, 0, 0, 4, 1
    );
    fn_00485D90(699, (opcode_strings + 0x1BA2), (opcode_strings + 0x18A6), 0x4a0000ff, 0, 0, 4, 1);
    fn_00485D90(700, (opcode_strings + 0x1BAA), (opcode_strings + 0x18A6), 0x4a0000fc, 0, 0, 4, 1);
    fn_00485D90(
        0x2bd, (opcode_strings + 0x1BB2), (opcode_strings + 0x18A6), 0x4a0000fd, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2be, (opcode_strings + 0x1BBA), (opcode_strings + 0x18A6), 0x4a0000fe, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2bf, (opcode_strings + 0x1BCA), (opcode_strings + 0x1BC2), 0x4a0003fc, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2c0, (opcode_strings + 0x1BD0), (opcode_strings + 0x1814), 0x4a00002a, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x2c1, (opcode_strings + 0x1BD5), (opcode_strings + 0x1824), 0x4a00001e, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2c2, (opcode_strings + 0x1BDB), (opcode_strings + 0x1837), 0x4a00001c, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2c3, (opcode_strings + 0x1BE1), (opcode_strings + 0x184A), 0x4a00001b, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x2c4, (opcode_strings + 0x1BE7), (opcode_strings + 0x184A), 0x4a000018, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x2c5, (opcode_strings + 0x1BED), (opcode_strings + 0x184A), 0x4a000019, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x2c6, (opcode_strings + 0x1BF3), (opcode_strings + 0x184A), 0x4a00001a, 0x2000, 0, 4, 1
    );
    fn_00485D90(
        0x2c7, (opcode_strings + 0x1C04), (opcode_strings + 0x1BF9), 0x4a0002be, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2c8, (opcode_strings + 0x1C0A), (opcode_strings + 0x187E), 0x4a0001fe, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2c9, (opcode_strings + 0x1C11), (opcode_strings + 0x1892), 0x4a0001fc, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2ca, (opcode_strings + 0x1C18), (opcode_strings + 0x18A6), 0x4a0001bf, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2cb, (opcode_strings + 0x1C1F), (opcode_strings + 0x18A6), 0x4a0001bc, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2cc, (opcode_strings + 0x1C26), (opcode_strings + 0x18A6), 0x4a0001bd, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2cd, (opcode_strings + 0x1C2D), (opcode_strings + 0x18A6), 0x4a0001be, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2ce, (opcode_strings + 0x1C34), (opcode_strings + 0x0), 0x4a0002ff, 0x100, 0, 4, 1
    );
    fn_00485D90(
        0x2cf, (opcode_strings + 0x1C43), (opcode_strings + 0x1C39), 0x4bc002fe, 0, 0, 4, 1
    );
    opcode_info[0x2CF].unk4 = (opcode_strings + 0x1C4B);
    fn_00485D90(
        0x2d0, (opcode_strings + 0x1C64), (opcode_strings + 0x1C57), 0x4bc0002e, 0, 0, 4, 1
    );
    opcode_info[0x2D0].unk4 = (opcode_strings + 0x1C6C);
    fn_00485D90(
        0x2d1, (opcode_strings + 0x1C82), (opcode_strings + 0x1C78), 0x4a00043d, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2d2, (opcode_strings + 0x1C90), (opcode_strings + 0x1C88), 0x4a00043e, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2d3, (opcode_strings + 0x1C97), (opcode_strings + 0x1C78), 0x4a00043c, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2d4, (opcode_strings + 0x1C9E), (opcode_strings + 0x191F), 0x4a0003be, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2d5, (opcode_strings + 0x1CB0), (opcode_strings + 0x1CA5), 0x4a00043f, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2d6, (opcode_strings + 0x1CC0), (opcode_strings + 0x1CB6), 0x4a00037f, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2d7, (opcode_strings + 0x1CCF), (opcode_strings + 0x1CC5), 0x4a00037d, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2d8, (opcode_strings + 0x1CDF), (opcode_strings + 0x1CD4), 0x4a0003bd, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2d9, (opcode_strings + 0x1CE5), (opcode_strings + 0x1814), 0x4a00002c, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2da, (opcode_strings + 0x1CEA), (opcode_strings + 0x1824), 0x4a000026, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2db, (opcode_strings + 0x1CF0), (opcode_strings + 0x1837), 0x4a000024, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2dc, (opcode_strings + 0x1CF6), (opcode_strings + 0x184A), 0x4a000007, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2dd, (opcode_strings + 0x1CFC), (opcode_strings + 0x184A), 0x4a000004, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2de, (opcode_strings + 0x1D02), (opcode_strings + 0x184A), 0x4a000005, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2df, (opcode_strings + 0x1D08), (opcode_strings + 0x184A), 0x4a000006, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2e0, (opcode_strings + 0x1D0E), (opcode_strings + 0x1BF9), 0x4a0002fc, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2e1, (opcode_strings + 0x1D14), (opcode_strings + 0x187E), 0x4a00027e, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2e2, (opcode_strings + 0x1D1B), (opcode_strings + 0x1892), 0x4a00027c, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2e3, (opcode_strings + 0x1D22), (opcode_strings + 0x18A6), 0x4a00007f, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2e4, (opcode_strings + 0x1D29), (opcode_strings + 0x18A6), 0x4a00007c, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2e5, (opcode_strings + 0x1D30), (opcode_strings + 0x18A6), 0x4a00007d, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2e6, (opcode_strings + 0x1D37), (opcode_strings + 0x18A6), 0x4a00007e, 0, 0, 4, 1
    );
    fn_00485D90(
        0x2e7, (opcode_strings + 0x1D3E), (opcode_strings + 0x0), 0x4a0003bf, 0x100, 0, 4, 1
    );
    fn_00485CF0(0x437, (opcode_strings + 0x346), (opcode_strings + 0x1029), 100, 0, 1);
    fn_00485CF0(0x44e, (opcode_strings + 0x1D4A), (opcode_strings + 0x1D45), 0, 0, 1);
    if ((copts.unknown0C < 2) || ((copts.fp_library & 2) == 0)) {
        macroOpcode = 0x460;
    } else {
        macroOpcode = 0x455;
    }
    fn_00485CF0(0x452, (opcode_strings + 0x1D53), (opcode_strings + 0x1D4D), macroOpcode, 0, 7);
    macroOpcode = (copts.unknown0C >= 2 && (copts.fp_library & 2)) ? 0x47a : 0x483;
    fn_00485CF0(0x476, (opcode_strings + 0x1D5C), (opcode_strings + 0x1D57), macroOpcode, 0, 7);
    fn_00485CF0(0x45a, (opcode_strings + 0x462), (opcode_strings + 0x1D45), 0x91, 0, 7);
    fn_00485CF0(0x453, (opcode_strings + 0x46D), (opcode_strings + 0x1D45), 0x92, 0, 7);
    fn_00485CF0(0x454, (opcode_strings + 0xFCA), (opcode_strings + 0x1D45), 0x93, 0, 7);
    fn_00485CF0(0x455, (opcode_strings + 0xDF7), (opcode_strings + 0x1D4D), 0x96, 0, 7);
    fn_00485CF0(0x456, (opcode_strings + 0xE05), (opcode_strings + 0x1D60), 0x97, 0, 7);
    fn_00485CF0(0x457, (opcode_strings + 0xE13), (opcode_strings + 0x1D60), 0x98, 0, 7);
    fn_00485CF0(0x458, (opcode_strings + 0xFD5), (opcode_strings + 0x1D65), 0x99, 0, 7);
    fn_00485CF0(0x459, (opcode_strings + 0xFE1), (opcode_strings + 0x1D65), 0x9a, 0, 7);
    fn_00485CF0(0x45b, (opcode_strings + 0x478), (opcode_strings + 0x1D45), 0x94, 0, 7);
    fn_00485CF0(0x45c, (opcode_strings + 0x483), (opcode_strings + 0x1D45), 0x95, 0, 7);
    fn_00485CF0(0x45d, (opcode_strings + 0x57B), (opcode_strings + 0x1D45), 0x9b, 0, 7);
    fn_00485CF0(0x45e, (opcode_strings + 0xFED), (opcode_strings + 0x1D45), 0x9c, 0, 7);
    fn_00485CF0(0x45f, (opcode_strings + 0x48E), (opcode_strings + 0x1D45), 0x9d, 0, 7);
    fn_00485CF0(0x460, (opcode_strings + 0x521), (opcode_strings + 0x1D6A), 0xa1, 0, 7);
    fn_00485CF0(0x461, (opcode_strings + 0x1D6F), (opcode_strings + 0x1D6A), 0xa1, 0, 7);
    fn_00485CF0(0x462, (opcode_strings + 0xA83), (opcode_strings + 0x1D60), 0xa2, 0, 7);
    fn_00485CF0(0x463, (opcode_strings + 0x54C), (opcode_strings + 0x1D60), 0xa3, 0, 7);
    fn_00485CF0(0x464, (opcode_strings + 0x4A1), (opcode_strings + 0x1D65), 0x9e, 0, 7);
    fn_00485CF0(0x465, (opcode_strings + 0x4AD), (opcode_strings + 0x1D65), 0x9f, 0, 7);
    fn_00485CF0(0x466, (opcode_strings + 0x4B9), (opcode_strings + 0x1D45), 0xa0, 0, 7);
    fn_00485CF0(0x47f, (opcode_strings + 0x4CB), (opcode_strings + 0x1D73), 0xc3, 0, 1);
    fn_00485CF0(0x477, (opcode_strings + 0x4D5), (opcode_strings + 0x1D73), 0xc1, 0, 1);
    fn_00485CF0(0x478, (opcode_strings + 0x4E0), (opcode_strings + 0x1D73), 0xc2, 0, 1);
    fn_00485CF0(0x479, (opcode_strings + 0xFF8), (opcode_strings + 0x1D73), 0xc4, 0, 1);
    fn_00485CF0(0x47a, (opcode_strings + 0xE21), (opcode_strings + 0x1D57), 0xc5, 0, 1);
    fn_00485CF0(0x47b, (opcode_strings + 0xE2F), (opcode_strings + 0x1D77), 0xc6, 0, 1);
    fn_00485CF0(0x47c, (opcode_strings + 0xE3D), (opcode_strings + 0x1D77), 199, 0, 1);
    fn_00485CF0(0x47d, (opcode_strings + 0x1003), (opcode_strings + 0x1D73), 200, 0, 1);
    fn_00485CF0(0x47e, (opcode_strings + 0x100F), (opcode_strings + 0x1D73), 0xc9, 0, 1);
    fn_00485CF0(0x480, (opcode_strings + 0x4EB), (opcode_strings + 0x1D73), 0xca, 0, 1);
    fn_00485CF0(0x481, (opcode_strings + 0x4F5), (opcode_strings + 0x1D73), 0xcb, 0, 1);
    fn_00485CF0(0x482, (opcode_strings + 0x561), (opcode_strings + 0x1D77), 0xcc, 0, 1);
    fn_00485CF0(0x483, (opcode_strings + 0x536), (opcode_strings + 0x1D7B), 0xcd, 0, 1);
    fn_00485CF0(0x484, (opcode_strings + 0x1D7F), (opcode_strings + 0x1D7B), 0xcd, 0, 1);
    fn_00485CF0(0x485, (opcode_strings + 0xA99), (opcode_strings + 0x1D77), 0xce, 0, 1);
    fn_00485CF0(0x486, (opcode_strings + 0x56F), (opcode_strings + 0x1D77), 0xcf, 0, 1);
    fn_00485CF0(0x487, (opcode_strings + 0x500), (opcode_strings + 0x1D73), 0xd0, 0, 1);
    fn_00485CF0(0x488, (opcode_strings + 0x50C), (opcode_strings + 0x1D73), 0xd1, 0, 1);
    fn_00485CF0(0x40c, (opcode_strings + 0x38), (opcode_strings + 0x1D83), 7, 6, 1);
    fn_00485CF0(0x473, (opcode_strings + 0x4F), (opcode_strings + 0x1D83), 0xbf, 0xbe, 1);
    fn_00485CF0(0x4a1, (opcode_strings + 0x5A), (opcode_strings + 0x1D83), 0xe5, 0xe4, 1);
    fn_00485CF0(0x40a, (opcode_strings + 0x1E), (opcode_strings + 0x1D83), 2, 1, 1);
    fn_00485CF0(0x40b, (opcode_strings + 0x15D), (opcode_strings + 0x1D83), 3, 4, 1);
    fn_00485CF0(0x439, (opcode_strings + 0xE70), (opcode_strings + 0x1D83), 0x77, 0x76, 1);
    fn_00485CF0(0x43a, (opcode_strings + 0xE7F), (opcode_strings + 0x1D83), 0x78, 0x79, 1);
    fn_00485CF0(0x49b, (opcode_strings + 0x9F), (opcode_strings + 0x1D8A), 0xd5, 0xd4, 1);
    fn_00485CF0(0x49c, (opcode_strings + 0xAC), (opcode_strings + 0x1D8A), 0xd6, 0xd7, 1);
    fn_00485CF0(0x431, (opcode_strings + 0x5CC), (opcode_strings + 0x1D91), 0x2d, 0x2c, 1);
    fn_00485CF0(0x432, (opcode_strings + 0x5EA), (opcode_strings + 0x1D91), 0x2f, 0x2e, 1);
    fn_00485CF0(0x433, (opcode_strings + 0x615), (opcode_strings + 0x1D91), 0x30, 0x31, 1);
    fn_00485CF0(0x434, (opcode_strings + 0x622), (opcode_strings + 0x1D91), 0x33, 0x32, 1);
    fn_00485CF0(0x435, (opcode_strings + 0x64D), (opcode_strings + 0x1D91), 0x34, 0x35, 1);
    fn_00485CF0(0x436, (opcode_strings + 0x65A), (opcode_strings + 0x1D91), 0x37, 0x36, 1);
    fn_00485CF0(0x498, (opcode_strings + 0x1D9B), (opcode_strings + 0x1D95), 0x49b, 0, 1);
    fn_00485CF0(0x49a, (opcode_strings + 0x1D9F), (opcode_strings + 0x1D95), 0x49c, 0, 1);
    fn_00485CF0(0x40d, (opcode_strings + 0x257), (opcode_strings + 0x1DA4), 0, 0x18, 1);
    fn_00485CF0(0x40e, (opcode_strings + 0x2C0), (opcode_strings + 0x1DA4), 0, 0x19, 1);
    fn_00485CF0(0x42f, (opcode_strings + 0x28D), (opcode_strings + 0x1DA4), 0, 0x28, 1);
    fn_00485CF0(0x430, (opcode_strings + 0x2F3), (opcode_strings + 0x1DA4), 0, 0x29, 1);
    fn_00485CF0(0x41f, (opcode_strings + 0x1DAA), (opcode_strings + 0x251), 0x40f, 0, 1);
    fn_00485CF0(0x420, (opcode_strings + 0x1DAE), (opcode_strings + 0x251), 0x410, 0, 1);
    fn_00485CF0(0x423, (opcode_strings + 0x1DB3), (opcode_strings + 0x251), 0x413, 0, 1);
    fn_00485CF0(0x424, (opcode_strings + 0x1DB8), (opcode_strings + 0x251), 0x414, 0, 1);
    fn_00485CF0(0x417, (opcode_strings + 0x1DBE), (opcode_strings + 0x251), 0x427, 0, 1);
    fn_00485CF0(0x418, (opcode_strings + 0x1DC2), (opcode_strings + 0x251), 0x428, 0, 1);
    fn_00485CF0(0x41b, (opcode_strings + 0x1DC7), (opcode_strings + 0x251), 0x42b, 0, 1);
    fn_00485CF0(0x41c, (opcode_strings + 0x1DCC), (opcode_strings + 0x251), 0x42c, 0, 1);
    fn_00485CF0(0x427, (opcode_strings + 0x1DD2), (opcode_strings + 0x251), 0xd4, 0x28, 1);
    fn_00485CF0(0x428, (opcode_strings + 0x1DD6), (opcode_strings + 0x251), 0xd4, 0x29, 1);
    fn_00485CF0(0x42b, (opcode_strings + 0x1DDB), (opcode_strings + 0x251), 0xd7, 0x28, 1);
    fn_00485CF0(0x42c, (opcode_strings + 0x1DE0), (opcode_strings + 0x251), 0xd7, 0x29, 1);
    fn_00485CF0(0x40f, (opcode_strings + 0x1DE6), (opcode_strings + 0x251), 0xd4, 0x18, 1);
    fn_00485CF0(0x410, (opcode_strings + 0x1DEA), (opcode_strings + 0x251), 0xd4, 0x19, 1);
    fn_00485CF0(0x413, (opcode_strings + 0x1DEF), (opcode_strings + 0x251), 0xd7, 0x18, 1);
    fn_00485CF0(0x414, (opcode_strings + 0x1DF4), (opcode_strings + 0x251), 0xd7, 0x19, 1);
    fn_00485CF0(0x411, (opcode_strings + 0x1DE6), (opcode_strings + 0x1DA4), 0x49b, 0x18, 1);
    fn_00485CF0(0x412, (opcode_strings + 0x1DEA), (opcode_strings + 0x1DA4), 0x49b, 0x19, 1);
    fn_00485CF0(0x415, (opcode_strings + 0x1DEF), (opcode_strings + 0x1DA4), 0x49c, 0x18, 1);
    fn_00485CF0(0x416, (opcode_strings + 0x1DF4), (opcode_strings + 0x1DA4), 0x49c, 0x19, 1);
    fn_00485CF0(0x429, (opcode_strings + 0x1DD2), (opcode_strings + 0x1DA4), 0x49b, 0x28, 1);
    fn_00485CF0(0x42a, (opcode_strings + 0x1DD6), (opcode_strings + 0x1DA4), 0x49b, 0x29, 1);
    fn_00485CF0(0x42d, (opcode_strings + 0x1DDB), (opcode_strings + 0x1DA4), 0x49c, 0x28, 1);
    fn_00485CF0(0x42e, (opcode_strings + 0x1DE0), (opcode_strings + 0x1DA4), 0x49c, 0x29, 1);
    fn_00485CF0(0x421, (opcode_strings + 0x1DAA), (opcode_strings + 0x1DA4), 0x49b, 0x28, 1);
    fn_00485CF0(0x422, (opcode_strings + 0x1DAE), (opcode_strings + 0x1DA4), 0x49b, 0x29, 1);
    fn_00485CF0(0x425, (opcode_strings + 0x1DB3), (opcode_strings + 0x1DA4), 0x49c, 0x28, 1);
    fn_00485CF0(0x426, (opcode_strings + 0x1DB8), (opcode_strings + 0x1DA4), 0x49c, 0x29, 1);
    fn_00485CF0(0x419, (opcode_strings + 0x1DBE), (opcode_strings + 0x1DA4), 0x49b, 0x18, 1);
    fn_00485CF0(0x41a, (opcode_strings + 0x1DC2), (opcode_strings + 0x1DA4), 0x49b, 0x19, 1);
    fn_00485CF0(0x41d, (opcode_strings + 0x1DC7), (opcode_strings + 0x1DA4), 0x49c, 0x18, 1);
    fn_00485CF0(0x41e, (opcode_strings + 0x1DCC), (opcode_strings + 0x1DA4), 0x49c, 0x19, 1);
    fn_00485CF0(0x44c, (opcode_strings + 0xE9E), (opcode_strings + 0x1D8A), 0x439, 0, 1);
    fn_00485CF0(0x44d, (opcode_strings + 0xEAD), (opcode_strings + 0x1D8A), 0x43a, 0, 1);
    fn_00485CF0(0x49f, (opcode_strings + 0x66), (opcode_strings + 0x1D8A), 0x40a, 0, 1);
    fn_00485CF0(0x4a0, (opcode_strings + 0x73), (opcode_strings + 0x1D8A), 0x40b, 0, 1);
    fn_00485CF0(0x440, (opcode_strings + 0x1DFA), (opcode_strings + 0x1399), 0x1d3, 0x1fb, 1);
    fn_00485CF0(0x43f, (opcode_strings + 0x1F5), (opcode_strings + 0x136E), 0x7c, 0xa9, 1);
    fn_00485CF0(0x442, (opcode_strings + 0x202), (opcode_strings + 0x136E), 0x7d, 0xa9, 1);
    fn_00485CF0(0x441, (opcode_strings + 0x1F5), (opcode_strings + 0x1E00), 0x43f, 0, 1);
    fn_00485CF0(0x443, (opcode_strings + 0x1E0A), (opcode_strings + 0x1E00), 0x442, 0, 1);
    fn_00485CF0(0x43b, (opcode_strings + 0xEDE), (opcode_strings + 0x136E), 0x7a, 0xa9, 1);
    fn_00485CF0(0x43d, (opcode_strings + 0xEED), (opcode_strings + 0x136E), 0x7b, 0xa9, 1);
    fn_00485CF0(0x43c, (opcode_strings + 0x1E10), (opcode_strings + 0x1E00), 0x43b, 0, 1);
    fn_00485CF0(0x43e, (opcode_strings + 0x1E16), (opcode_strings + 0x1E00), 0x43d, 0, 1);
    fn_00485CF0(0x474, (opcode_strings + 0x1E1D), (opcode_strings + 0x136E), 0x7c, 0xa8, 1);
    fn_00485CF0(0x475, (opcode_strings + 0x1E21), (opcode_strings + 0x136E), 0x7d, 0xa8, 1);
    fn_00485CF0(0x44a, (opcode_strings + 0x1E26), (opcode_strings + 0x136E), 0x7a, 0xa8, 1);
    fn_00485CF0(1099, (opcode_strings + 0x1E2B), (opcode_strings + 0x136E), 0x7b, 0xa8, 1);
    fn_00485CF0(0x46e, (opcode_strings + 0x1E31), (opcode_strings + 0x1E00), 0x469, 0, 1);
    fn_00485CF0(0x470, (opcode_strings + 0x1E36), (opcode_strings + 0x1E00), 0x46f, 0, 1);
    fn_00485CF0(0x472, (opcode_strings + 0x1E3C), (opcode_strings + 0x1E00), 0x471, 0, 1);
    fn_00485CF0(0x445, (opcode_strings + 0x1E43), (opcode_strings + 0x1E00), 0x444, 0, 1);
    fn_00485CF0(0x447, (opcode_strings + 0x1E49), (opcode_strings + 0x1E00), 0x446, 0, 1);
    fn_00485CF0(0x449, (opcode_strings + 0x1E50), (opcode_strings + 0x1E00), 0x448, 0, 1);
    fn_00485CF0(0x469, (opcode_strings + 0x1E58), (opcode_strings + 0x136E), 0xb6, 0xa9, 1);
    fn_00485CF0(0x46f, (opcode_strings + 0x1E5C), (opcode_strings + 0x136E), 0xb6, 0xa9, 1);
    fn_00485CF0(0x471, (opcode_strings + 0x1E61), (opcode_strings + 0x136E), 0xb7, 0xa9, 1);
    fn_00485CF0(0x444, (opcode_strings + 0x1E70), (opcode_strings + 0x1E67), 0x80, 0xa9, 1);
    fn_00485CF0(0x446, (opcode_strings + 0x1E75), (opcode_strings + 0x136E), 0, 0, 1);
    fn_00485CF0(0x448, (opcode_strings + 0x1E7B), (opcode_strings + 0x136E), 0, 0, 1);
    fn_00485CF0(0x450, (opcode_strings + 0x1BF), (opcode_strings + 0x1E82), 0, 0, 1);
    opcode_info[0x450].unk4 = (opcode_strings + 0x1E87);
    fn_00485CF0(0x451, (opcode_strings + 0x1E90), (opcode_strings + 0x1E8B), 0, 0, 2);
    fn_00485CF0(0x48b, (opcode_strings + 0x1E9C), (opcode_strings + 0x1E95), 0, 0, 1);
    fn_00485CF0(0x48c, (opcode_strings + 0x1EA1), (opcode_strings + 0x1E95), 0, 0, 1);
    fn_00485CF0(0x494, (opcode_strings + 0x1EA6), (opcode_strings + 0x1D8A), 0xd4, 0, 1);
    fn_00485CF0(0x496, (opcode_strings + 0x1EAA), (opcode_strings + 0x1D8A), 0xd7, 0, 1);
    fn_00485CF0(0x490, (opcode_strings + 0x1EAF), (opcode_strings + 0x1D8A), 0x49b, 0, 1);
    fn_00485CF0(0x492, (opcode_strings + 0x1EB3), (opcode_strings + 0x1D8A), 0x49c, 0, 1);
    fn_00485CF0(0x497, (opcode_strings + 0x1D9B), (opcode_strings + 0x17), 0xd4, 0, 1);
    fn_00485CF0(0x499, (opcode_strings + 0x1D9F), (opcode_strings + 0x17), 0xd7, 0, 1);
    fn_00485CF0(0x48d, (opcode_strings + 0x1EB8), (opcode_strings + 0x17), 0, 0, 1);
    fn_00485CF0(0x48e, (opcode_strings + 0x1EB8), (opcode_strings + 0x1D8A), 0, 0, 1);
    fn_00485CF0(0x49d, (opcode_strings + 0x1EBC), (opcode_strings + 0x17), 0, 0, 1);
    fn_00485CF0(0x49e, (opcode_strings + 0x1EBC), (opcode_strings + 0x1D8A), 0, 0, 1);
    fn_00485CF0(0x493, (opcode_strings + 0x1EA6), (opcode_strings + 0x17), 0xd4, 0, 1);
    fn_00485CF0(0x495, (opcode_strings + 0x1EAA), (opcode_strings + 0x17), 0xd7, 0, 1);
    fn_00485CF0(0x48f, (opcode_strings + 0x1EAF), (opcode_strings + 0x17), 0x497, 0, 1);
    fn_00485CF0(0x491, (opcode_strings + 0x1EB3), (opcode_strings + 0x17), 0x499, 0, 1);
    fn_00485CF0(0x467, (opcode_strings + 0x127F), (opcode_strings + 0x1EC0), 0x1d1, 0, 7);
    fn_00485CF0(0x468, (opcode_strings + 0xBA1), (opcode_strings + 0x1EC6), 0x263, 0, 7);
    fn_00485CF0(0x489, (opcode_strings + 0x12AB), (opcode_strings + 0x1EC0), 0x1d2, 0, 1);
    fn_00485CF0(0x48a, (opcode_strings + 0xBC7), (opcode_strings + 0x1EC6), 0x268, 0, 1);
    fn_00485CF0(0x4a2, (opcode_strings + 0x1ECC), (opcode_strings + 0x0), 0, 0, 1);
    fn_00485CF0(0x4a3, (opcode_strings + 0x1ED7), (opcode_strings + 0x936), 0, 0, 1);
    fn_00485CF0(0x4a4, (opcode_strings + 0x1EDD), (opcode_strings + 0x6E6), 0, 0, 1);
    fn_00485CF0(0x4a5, (opcode_strings + 0x1EE3), (opcode_strings + 0x0), 0, 0, 1);
    fn_00485CF0(0x4a6, (opcode_strings + 0x1EEA), (opcode_strings + 0x0), 0, 0, 1);
    fn_00485CF0(0x4a7, (opcode_strings + 0x1EEA), (opcode_strings + 0x0), 0, 0, 1);
    fn_00485CF0(0x4ae, (opcode_strings + 0x1EEF), (opcode_strings + 0x0), 0, 0, 0);
    fn_00485D90(0x4a8, (opcode_strings + 0x1EF8), (opcode_strings + 0x1EF4), 0, 0x8100000, 0, 0, 0);
    fn_00485D90(0x4a9, (opcode_strings + 0x1EF8), (opcode_strings + 0x6B1), 0, 0x8100000, 0, 0, 0);
    fn_00485D90(0x4aa, (opcode_strings + 0x1F00), (opcode_strings + 0x1EF4), 0, 0x8100000, 0, 0, 0);
    fn_00485D90(0x4ab, (opcode_strings + 0x1F00), (opcode_strings + 0x6B1), 0, 0x8100000, 0, 0, 0);
    fn_00485D90(0x4ac, (opcode_strings + 0x1F09), (opcode_strings + 0x1EF4), 0, 0x8100000, 0, 0, 0);
    fn_00485D90(0x4ad, (opcode_strings + 0x1F09), (opcode_strings + 0x6B1), 0, 0x8100000, 0, 0, 0);
    return;
}
