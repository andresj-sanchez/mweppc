#include "compiler/CPrepTokenizer.h"
#include "compiler/CPrep.h"
#include "compiler/common.h"
#include "compiler/CParser.h"
#include "compiler/CError.h"
#include "compiler/Registers.h"

SInt16 lookahead(void) {
    SInt32 state;
    SInt16 token;

    fn_00444780(&state);
    token = lex();
    fn_00444740(&state);
    return token;
}

SInt16 fn_00468780(void) {
    SInt32 state;
    SInt16 token;

    fn_00444780(&state);
    do {
        token = lex();
    } while (token == -7);
    fn_00444740(&state);
    return token;
}

SInt32 fn_004680B0(void) {
    SInt32 character;
    char* saved;

    character = prepskipnextchar();
    pos = nextcharpos;
    register_newline_637381 = 0;
    if ((SInt16)character >= 'a' && (SInt16)character <= 'z' ||
        (SInt16)character >= 'A' && (SInt16)character <= 'Z' ||
        (SInt16)character == '_') {
        pos = fn_0046C430(pos - 1);
        return -3;
    }
    if ((SInt16)character == '.') {
        saved = pos;
        if (fn_0046C840() == '.' && fn_0046C840() == '.') {
            return 0x173;
        }
        pos = saved;
    }
    return character;
}

SInt16 t0x1a(void) {
    if (*pos == 0) {
        return 0;
    }
    return -6;
}

SInt16 tisid(void) {
    if (tokenstacklevel == 0) {
        CError_Error(6);
    }
    return 0;
}

SInt16 tdoll(SInt16 token) {
    register_newline_637381 = FALSE;
    return token;
}

SInt16 token(SInt16 t) {
    register_newline_637381 = FALSE;
    return t;
}

SInt16 tille(void) {
    return -6;
}

SInt16 tcret(void) {
    fn_00442790();
    if (cprep_eoltokens) {
        return -7;
    }
    return 0;
}

SInt16 tnull(SInt16 t) {
    if (tokenstacklevel > 0 || pos >= prep_file_end) {
        register_newline_637381 = FALSE;
        return t;
    }
    pos[-1] = -64;
    return -6;
}

SInt16 thash(void) {
    if (register_newline_637381) {
        fn_0043A580();
        return 0;
    }
    return '#';
}

SInt16 tdivi(SInt16 t) {
    register_newline_637381 = FALSE;
    if ((t = fn_0046C560()) == '=') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x15F;
    }
    return '/';
}

SInt16 tpowe(SInt16 t) {
    register_newline_637381 = FALSE;
    if ((t = fn_0046C560()) == '=') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x166;
    }
    return '^';
}

SInt16 tampe(SInt16 t) {
    register_newline_637381 = FALSE;
    if ((t = fn_0046C560()) == '=') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x165;
    }
    if (t == '&') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x169;
    }
    return '&';
}

SInt16 torrr(SInt16 t) {
    register_newline_637381 = FALSE;
    if ((t = fn_0046C560()) == '=') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x167;
    }
    if (t == '|') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x168;
    }
    return '|';
}

SInt16 tmult(void) {
    register_newline_637381 = FALSE;
    if ((SInt16)fn_0046C560() == '=') {
        if (fn_0046C840() != '=') {
            CError_Internal(register_assert_source, 623);
        }
        return 0x15E;
    }
    return '*';
}

SInt16 texcl(void) {
    register_newline_637381 = FALSE;
    if ((SInt16)fn_0046C560() == '=') {
        if (fn_0046C840() != '=') {
            CError_Internal(register_assert_source, 623);
        }
        return 0x16B;
    }
    return '!';
}

SInt16 tequa(void) {
    register_newline_637381 = FALSE;
    if ((SInt16)fn_0046C560() == '=') {
        if (fn_0046C840() != '=') {
            CError_Internal(register_assert_source, 623);
        }
        return 0x16A;
    }
    return '=';
}

SInt16 tplus(SInt16 t) {
    register_newline_637381 = FALSE;
    if ((t = fn_0046C560()) == '=') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x161;
    }
    if (t == '+') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x170;
    }
    return '+';
}

SInt16 tminu(SInt16 t) {
    register_newline_637381 = FALSE;
    if ((t = fn_0046C560()) == '=') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x162;
    }
    if (t == '-') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x171;
    }
    if (t == '>') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        if (copts.cplusplus && (SInt16)fn_0046C560() == '*') {
            if (fn_0046C840() != '*') {
                CError_Internal(register_assert_source, 623);
            }
            return 0x175;
        }
        return 0x172;
    }
    return '-';
}

SInt16 tperc(SInt16 t) {
    SInt8 next;
    register_newline_637381 = FALSE;
    next = fn_0046C560();
    if (next == '=') {
        if ((SInt16)fn_0046C840() != (SInt16)next) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x160;
    }
    if (next == '>' && (copts.cplusplus || copts.c9x) && !register_preprocessing_only_6373A3) {
        if ((SInt16)fn_0046C840() != (SInt16)next) {
            CError_Internal(register_assert_source, 623);
        }
        return '}';
    }
    return '%';
}

SInt16 tatsg(SInt16 t) {
    UInt8 save_idarray;
    char* start_pos = pos;

    register_newline_637381 = FALSE;
    if (cprep_in_assembler) {
        save_idarray = register_character_class['@'];
        register_character_class['@'] = 1;
        pos = fn_0046C430(pos - 1);
        register_character_class['@'] = save_idarray;
        if (cprep_in_assembler) {
            return -3;
        }
        pos = start_pos;
    }
    return '@';
}

SInt16 tiden(void) {
    register_newline_637381 = FALSE;
    pos = fn_0046C430(pos - 1);
    if (cprep_macrocheck && fn_0043F720()) {
        return 0;
    }
    return -3;
}

SInt16 tgrea(SInt16 t) {
    register_newline_637381 = FALSE;
    if ((t = fn_0046C560()) == '=') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x16D;
    }
    if (t == '>') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        if ((SInt16)fn_0046C560() == '=') {
            if (fn_0046C840() != '=') {
                CError_Internal(register_assert_source, 623);
            }
            return 0x164;
        }
        return 0x16F;
    }
    if (t == '?' && copts.gcc_extensions) {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x178;
    }
    return '>';
}

SInt16 tless(SInt16 t) {
    register_newline_637381 = FALSE;
    if ((t = fn_0046C560()) == '=') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x16C;
    }
    if (t == '<') {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        if ((SInt16)fn_0046C560() == '=') {
            if (fn_0046C840() != '=') {
                CError_Internal(register_assert_source, 623);
            }
            return 0x163;
        }
        return 0x16E;
    }
    if (t == '%' && (copts.cplusplus || copts.c9x) && !register_preprocessing_only_6373A3) {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return '{';
    }
    if (t == ':' && (copts.cplusplus || copts.c9x) && !register_preprocessing_only_6373A3) {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return '[';
    }
    if (t == '?' && copts.gcc_extensions) {
        if (fn_0046C840() != t) {
            CError_Internal(register_assert_source, 623);
        }
        return 0x177;
    }
    return '<';
}
