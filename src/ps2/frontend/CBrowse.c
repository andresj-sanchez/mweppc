#include "compiler/CBrowse.h"
#include "compiler/CDecl.h"
#include "compiler/CError.h"
#include "compiler/CMachine.h"
#include "compiler/CMangler.h"
#include "compiler/CParser.h"
#include "compiler/CPrep.h"
#include "compiler/CompilerTools.h"
#include "compiler/CTemplateTools.h"
#include "compiler/common.h"
#include "compiler/objects.h"
#include "compiler/scopes.h"
#include "compiler/templates.h"
#include "compiler/tokens.h"
#include "compiler/types.h"
#include "compiler/cos.h"
#include <string.h>

Boolean gForceSourceLoc;
Boolean gUseNameTable;
static GList gBrowseData;
static GList gClassData;
static GList gMemberFuncList;
static int gNextMemberFuncID;

enum ELanguage {
    langUnknown,
    langC,
    langCPlus,
    langPascal,
    langObjectPascal,
    langJava,
    langAssembler,
    langFortran,
    langRez
};

enum EBrowserItem {
    browseFunction,
    browseGlobal,
    browseClass,
    browseMacro,
    browseEnum,
    browseTypedef,
    browseConstant,
    browseTemplate,
    browsePackage,
    browseCompSymbolStart = 0x70,
    browseEnd = 0xFF
};

enum {
    kAbstract = 1,
    kStatic = 2,
    kFinal = 4,
    kMember = 8,

    kInterface = 0x80,
    kPublic = 0x100,

    kInline = 0x80,
    kPascal = 0x100,
    kAsm = 0x200,
    kVirtual = 0x400,
    kCtor = 0x800,
    kDtor = 0x1000,
    kNative = 0x2000,
    kSynch = 0x4000,
    kIntrinsic = 0x8000,
    kConst = 0x10000,

    kTransient = 0x80,
    kVolatile = 0x100
};

enum EAccess {
    accessNone = 0,
    accessPrivate = 1,
    accessProtected = 2,
    accessPublic = 4
};

enum EMember {
    memberFunction,
    memberData,
    memberEnd = 0xFF
};

enum ETemplateType {
    templateClass,
    templateFunction
};

static enum EAccess gFromAccessType[] = {accessPublic, accessPrivate, accessProtected, accessNone};

char gBrowseFileName[10] = "CBrowse.c";
char unk5E95F7[3] = "\r\n";

typedef struct BrowseHeader {
    SInt32 browse_header;
    SInt32 browse_version;
    SInt16 browse_language;
    SInt16 uses_name_table;
    SInt32 earliest_compatible_version;
    SInt32 reserved[15];
} BrowseHeader;

static void RecordUndefinedMemberFunctions(void);

#undef CERROR_FILE
#define CERROR_FILE "CPreprocess.c"

static void CPrep_DumpWString(UInt16* str, SInt16 len) {
    SInt32 divisor;
    SInt32 i;

    while (len--) {
        if (*str < 32) {
            AppendGListByte(&pplist, '\\');
            switch (*str) {
                case 7:
                    AppendGListByte(&pplist, 'a');
                    break;
                case 8:
                    AppendGListByte(&pplist, 'b');
                    break;
                case 27:
                    AppendGListByte(&pplist, 'e');
                    break;
                case 12:
                    AppendGListByte(&pplist, 'f');
                    break;
                case 10:
                    AppendGListByte(&pplist, 'n');
                    break;
                case 13:
                    AppendGListByte(&pplist, 'r');
                    break;
                case 9:
                    AppendGListByte(&pplist, 't');
                    break;
                case 11:
                    AppendGListByte(&pplist, 'v');
                    break;
                default:
                    if (*str >= 8) {
                        AppendGListByte(&pplist, '0' + (*str / 8));
                    }
                    AppendGListByte(&pplist, '0' + (*str % 8));
            }
        } else if (*str > 255) {
            AppendGListByte(&pplist, '\\');
            AppendGListByte(&pplist, 'x');

            divisor = 0x1000;
            for (i = 0; i < 4; i++) {
                AppendGListByte(&pplist, "0123456789ABCDEF"[(*str / divisor) % 16]);
                divisor /= 16;
            }
        } else {
            switch (*str) {
                case '"':
                case '\\':
                    AppendGListByte(&pplist, '\\');
                default:
                    AppendGListByte(&pplist, *str);
            }
        }
        str++;
    }
}

static void CPrep_DumpString(UInt8* str, SInt16 len) {
    while (len--) {
        if (*str < 32) {
            AppendGListByte(&pplist, '\\');
            switch (*str) {
                case 7:
                    AppendGListByte(&pplist, 'a');
                    break;
                case 8:
                    AppendGListByte(&pplist, 'b');
                    break;
                case 12:
                    AppendGListByte(&pplist, 'f');
                    break;
                case 10:
                    AppendGListByte(&pplist, 'n');
                    break;
                case 13:
                    AppendGListByte(&pplist, 'r');
                    break;
                case 9:
                    AppendGListByte(&pplist, 't');
                    break;
                case 11:
                    AppendGListByte(&pplist, 'v');
                    break;
                default:
                    if (*str >= 8) {
                        AppendGListByte(&pplist, '0' + (*str / 8));
                    }
                    AppendGListByte(&pplist, '0' + (*str % 8));
            }
        } else {
            switch (*str) {
                case '"':
                case '\\':
                    AppendGListByte(&pplist, '\\');
                default:
                    AppendGListByte(&pplist, *str);
            }
        }
        str++;
    }
}

void CPrep_Preprocess(void) {
    int innertoken;
    short token;
    short lasttoken;
    char startToken;
    char endToken;
    int depth;
    Boolean save_asmsemicolcomment;
    Boolean save_asmpoundcomment;
    Boolean save_cplusplus;
    char* p;

    startToken = 0;
    lasttoken = 0;

    nlflag = FALSE;
    spaceskip = FALSE;

    if ((token = lex())) {
        do {
            if (nlflag) {
                if (!copts.line_prepdump) {
                    AppendGListData(&pplist, unk5E95F7, 2);
                }
            } else if (spaceskip) {
                AppendGListByte(&pplist, ' ');
            } else if (token == TK_COLON_COLON && lasttoken == '<') {
                AppendGListByte(&pplist, ' ');
            } else if (lasttoken == TK_IDENTIFIER &&
                       (token == TK_IDENTIFIER || token == TK_INTCONST || token == TK_FLOATCONST)) {
                AppendGListByte(&pplist, ' ');
            }

            while (1) {
                switch ((innertoken = token)) {
                    case TK_INTCONST:
                    case TK_FLOATCONST:
                        if (tokenstacklevel > 0) {
                            p = macropos;
                        } else {
                            p = prep_file_start + ts_current[-1].tokenoffset;
                        }
                        AppendGListData(&pplist, p, pos - p);
                        break;

                    case TK_IDENTIFIER:
                        AppendGListData(&pplist, tkidentifier->name, strlen(tkidentifier->name));
                        break;

                    case TK_AUTO:
                        AppendGListData(&pplist, "auto", 4);
                        break;
                    case TK_REGISTER:
                        AppendGListData(&pplist, "register", 8);
                        break;
                    case TK_STATIC:
                        AppendGListData(&pplist, "static", 6);
                        break;
                    case TK_EXTERN:
                        AppendGListData(&pplist, "extern", 6);
                        break;
                    case TK_TYPEDEF:
                        AppendGListData(&pplist, "typedef", 7);
                        break;
                    case TK_INLINE:
                        AppendGListData(&pplist, "inline", 6);
                        break;
                    case TK_VOID:
                        AppendGListData(&pplist, "void", 4);
                        break;
                    case TK_CHAR:
                        AppendGListData(&pplist, "char", 4);
                        break;
                    case TK_SHORT:
                        AppendGListData(&pplist, "short", 5);
                        break;
                    case TK_INT:
                        AppendGListData(&pplist, "int", 3);
                        break;
                    case TK_LONG:
                        AppendGListData(&pplist, "long", 4);
                        break;
                    case TK_FLOAT:
                        AppendGListData(&pplist, "float", 5);
                        break;
                    case TK_DOUBLE:
                        AppendGListData(&pplist, "double", 6);
                        break;
                    case TK_SIGNED:
                        AppendGListData(&pplist, "signed", 6);
                        break;
                    case TK_UNSIGNED:
                        AppendGListData(&pplist, "unsigned", 8);
                        break;
                    case TK_STRUCT:
                        AppendGListData(&pplist, "struct", 6);
                        break;
                    case TK_UNION:
                        AppendGListData(&pplist, "union", 5);
                        break;
                    case TK_ENUM:
                        AppendGListData(&pplist, "enum", 4);
                        break;
                    case TK_CLASS:
                        AppendGListData(&pplist, "class", 5);
                        break;
                    case TK_CONST:
                        AppendGListData(&pplist, "const", 5);
                        break;
                    case TK_VOLATILE:
                        AppendGListData(&pplist, "volatile", 8);
                        break;
                    case TK_PASCAL:
                        AppendGListData(&pplist, "pascal", 6);
                        break;
                    case TK_UU_FAR:
                        AppendGListData(&pplist, "__far", 5);
                        break;
                    case TK_ONEWAY:
                        AppendGListData(&pplist, "oneway", 6);
                        break;
                    case TK_IN:
                        AppendGListData(&pplist, "in", 2);
                        break;
                    case TK_OUT:
                        AppendGListData(&pplist, "out", 3);
                        break;
                    case TK_INOUT:
                        AppendGListData(&pplist, "inout", 5);
                        break;
                    case TK_BYCOPY:
                        AppendGListData(&pplist, "bycopy", 6);
                        break;
                    case TK_BYREF:
                        AppendGListData(&pplist, "byref", 5);
                        break;

                    case TK_ASM:
                        AppendGListData(&pplist, "asm", 3);
                        startToken = endToken = 0;
                        token = lex();
                        if (token == TK_VOLATILE) {
                            AppendGListByte(&pplist, ' ');
                            AppendGListData(&pplist, "volatile", 8);
                            token = lex();
                        }
                        if (token) {
                            if (token < ' ' || token > 255) {
                                AppendGListByte(&pplist, ' ');
                                continue;
                            }
                            AppendGListByte(&pplist, token);

                            if (token == '(') {
                                startToken = '(';
                                endToken = ')';
                            } else if (token == '{') {
                                startToken = '{';
                                endToken = '}';
                            } else {
                                continue;
                            }

                            cprep_nostring = TRUE;
                            cprep_in_assembler = TRUE;
                            save_asmsemicolcomment = copts.asmsemicolcomment;
                            save_asmpoundcomment = copts.asmpoundcomment;
                            save_cplusplus = copts.cplusplus;
                            depth = 1;

                            token = lex();
                            if (token == '"') {
                                AppendGListByte(&pplist, token);
                                copts.cplusplus = FALSE;
                                break;
                            }
                            if (token) {
                                cprep_nostring = FALSE;
                                continue;
                            }
                        }
                        break;

                    case TK_CASE:
                        AppendGListData(&pplist, "case", 4);
                        break;
                    case TK_DEFAULT:
                        AppendGListData(&pplist, "default", 7);
                        break;
                    case TK_IF:
                        AppendGListData(&pplist, "if", 2);
                        break;
                    case TK_ELSE:
                        AppendGListData(&pplist, "else", 4);
                        break;
                    case TK_SWITCH:
                        AppendGListData(&pplist, "switch", 6);
                        break;
                    case TK_WHILE:
                        AppendGListData(&pplist, "while", 5);
                        break;
                    case TK_DO:
                        AppendGListData(&pplist, "do", 2);
                        break;
                    case TK_FOR:
                        AppendGListData(&pplist, "for", 3);
                        break;
                    case TK_GOTO:
                        AppendGListData(&pplist, "goto", 4);
                        break;
                    case TK_CONTINUE:
                        AppendGListData(&pplist, "continue", 8);
                        break;
                    case TK_BREAK:
                        AppendGListData(&pplist, "break", 5);
                        break;
                    case TK_RETURN:
                        AppendGListData(&pplist, "return", 6);
                        break;
                    case TK_SIZEOF:
                        AppendGListData(&pplist, "sizeof", 6);
                        break;
                    case TK_CATCH:
                        AppendGListData(&pplist, "catch", 5);
                        break;
                    case TK_DELETE:
                        AppendGListData(&pplist, "delete", 6);
                        break;
                    case TK_FRIEND:
                        AppendGListData(&pplist, "friend", 6);
                        break;
                    case TK_NEW:
                        AppendGListData(&pplist, "new", 3);
                        break;
                    case TK_OPERATOR:
                        AppendGListData(&pplist, "operator", 8);
                        break;
                    case TK_PRIVATE:
                        AppendGListData(&pplist, "private", 7);
                        break;
                    case TK_PROTECTED:
                        AppendGListData(&pplist, "protected", 9);
                        break;
                    case TK_PUBLIC:
                        AppendGListData(&pplist, "public", 6);
                        break;
                    case TK_TEMPLATE:
                        AppendGListData(&pplist, "template", 8);
                        break;
                    case TK_THIS:
                        AppendGListData(&pplist, "this", 4);
                        break;
                    case TK_THROW:
                        AppendGListData(&pplist, "throw", 5);
                        break;
                    case TK_TRY:
                        AppendGListData(&pplist, "try", 3);
                        break;
                    case TK_VIRTUAL:
                        AppendGListData(&pplist, "virtual", 7);
                        break;
                    case TK_INHERITED:
                        AppendGListData(&pplist, "inherited", 9);
                        break;
                    case TK_CONST_CAST:
                        AppendGListData(&pplist, "const_cast", 10);
                        break;
                    case TK_DYNAMIC_CAST:
                        AppendGListData(&pplist, "dynamic_cast", 12);
                        break;
                    case TK_EXPLICIT:
                        AppendGListData(&pplist, "explicit", 8);
                        break;
                    case TK_MUTABLE:
                        AppendGListData(&pplist, "mutable", 7);
                        break;
                    case TK_NAMESPACE:
                        AppendGListData(&pplist, "namespace", 9);
                        break;
                    case TK_REINTERPRET_CAST:
                        AppendGListData(&pplist, "reinterpret_cast", 16);
                        break;
                    case TK_STATIC_CAST:
                        AppendGListData(&pplist, "static_cast", 11);
                        break;
                    case TK_USING:
                        AppendGListData(&pplist, "using", 5);
                        break;
                    case TK_WCHAR_T:
                        AppendGListData(&pplist, "wchar_t", 7);
                        break;
                    case TK_TYPENAME:
                        AppendGListData(&pplist, "typename", 8);
                        break;
                    case TK_TRUE:
                        AppendGListData(&pplist, "true", 4);
                        break;
                    case TK_FALSE:
                        AppendGListData(&pplist, "false", 5);
                        break;
                    case TK_TYPEID:
                        AppendGListData(&pplist, "typeid", 6);
                        break;
                    case TK_EXPORT:
                        AppendGListData(&pplist, "export", 6);
                        break;
                    case TK_UU_STDCALL:
                        AppendGListData(&pplist, "__stdcall", 9);
                        break;
                    case TK_UU_CDECL:
                        AppendGListData(&pplist, "__cdecl", 7);
                        break;
                    case TK_UU_FASTCALL:
                        AppendGListData(&pplist, "__fastcall", 10);
                        break;
                    case TK_UU_3DCALL:
                        AppendGListData(&pplist, "__3dcall", 8);
                        break;
                    case TK_UU_SSECALL:
                        AppendGListData(&pplist, "__ssecall", 9);
                        break;
                    case TK_UU_SSE2CALL:
                        AppendGListData(&pplist, "__sse2call", 10);
                        break;
                    case TK_UU_DECLSPEC:
                        AppendGListData(&pplist, "__declspec", 10);
                        break;
                    case TK_MULT_ASSIGN:
                        AppendGListData(&pplist, "*=", 2);
                        break;
                    case TK_DIV_ASSIGN:
                        AppendGListData(&pplist, "/=", 2);
                        break;
                    case TK_MOD_ASSIGN:
                        AppendGListData(&pplist, "%=", 2);
                        break;
                    case TK_ADD_ASSIGN:
                        AppendGListData(&pplist, "+=", 2);
                        break;
                    case TK_SUB_ASSIGN:
                        AppendGListData(&pplist, "-=", 2);
                        break;
                    case TK_SHL_ASSIGN:
                        AppendGListData(&pplist, "<<=", 3);
                        break;
                    case TK_SHR_ASSIGN:
                        AppendGListData(&pplist, ">>=", 3);
                        break;
                    case TK_AND_ASSIGN:
                        AppendGListData(&pplist, "&=", 2);
                        break;
                    case TK_XOR_ASSIGN:
                        AppendGListData(&pplist, "^=", 2);
                        break;
                    case TK_OR_ASSIGN:
                        AppendGListData(&pplist, "|=", 2);
                        break;
                    case TK_LOGICAL_OR:
                        AppendGListData(&pplist, "||", 2);
                        break;
                    case TK_LOGICAL_AND:
                        AppendGListData(&pplist, "&&", 2);
                        break;
                    case TK_LOGICAL_EQ:
                        AppendGListData(&pplist, "==", 2);
                        break;
                    case TK_LOGICAL_NE:
                        AppendGListData(&pplist, "!=", 2);
                        break;
                    case TK_LESS_EQUAL:
                        AppendGListData(&pplist, "<=", 2);
                        break;
                    case TK_GREATER_EQUAL:
                        AppendGListData(&pplist, ">=", 2);
                        break;
                    case TK_SHL:
                        AppendGListData(&pplist, "<<", 2);
                        break;
                    case TK_SHR:
                        AppendGListData(&pplist, ">>", 2);
                        break;
                    case TK_INCREMENT:
                        AppendGListData(&pplist, "++", 2);
                        break;
                    case TK_DECREMENT:
                        AppendGListData(&pplist, "--", 2);
                        break;
                    case TK_ARROW:
                        AppendGListData(&pplist, "->", 2);
                        break;
                    case TK_ELLIPSIS:
                        AppendGListData(&pplist, "...", 3);
                        break;
                    case TK_DOT_STAR:
                        AppendGListData(&pplist, ".*", 2);
                        break;
                    case TK_ARROW_STAR:
                        AppendGListData(&pplist, "->*", 3);
                        break;
                    case TK_COLON_COLON:
                        AppendGListData(&pplist, "::", 2);
                        break;
                    case TK_AT_INTERFACE:
                        AppendGListData(&pplist, "@interface", 10);
                        break;
                    case TK_AT_IMPLEMENTATION:
                        AppendGListData(&pplist, "@implementation", 15);
                        break;
                    case TK_AT_PROTOCOL:
                        AppendGListData(&pplist, "@protocol", 9);
                        break;
                    case TK_AT_END:
                        AppendGListData(&pplist, "@end", 4);
                        break;
                    case TK_AT_PRIVATE:
                        AppendGListData(&pplist, "@private", 8);
                        break;
                    case TK_AT_PROTECTED:
                        AppendGListData(&pplist, "@protected", 10);
                        break;
                    case TK_AT_PUBLIC:
                        AppendGListData(&pplist, "@public", 7);
                        break;
                    case TK_AT_CLASS:
                        AppendGListData(&pplist, "@class", 6);
                        break;
                    case TK_AT_SELECTOR:
                        AppendGListData(&pplist, "@selector", 9);
                        break;
                    case TK_AT_ENCODE:
                        AppendGListData(&pplist, "@encode", 7);
                        break;
                    case TK_AT_DEFS:
                        AppendGListData(&pplist, "@defs", 5);
                        break;
                    case TK_SELF:
                        AppendGListData(&pplist, "self", 4);
                        break;
                    case TK_SUPER:
                        AppendGListData(&pplist, "super", 5);
                        break;
                    case TK_BOOL:
                        if (!copts.cplusplus && copts.c9x) {
                            AppendGListData(&pplist, "_Bool", 5);
                        } else {
                            AppendGListData(&pplist, "bool", 4);
                        }
                        break;
                    case TK_RESTRICT:
                        if (copts.c9x) {
                            AppendGListData(&pplist, "restrict", 8);
                        } else {
                            AppendGListData(&pplist, "__restrict", 10);
                        }
                        break;
                    case TK_UU_FIXED_8X8:
                        AppendGListData(&pplist, "__fixed__8x8", 12);
                        break;
                    case TK_UU_INT128:
                        AppendGListData(&pplist, "__int128", 8);
                        break;
                    case TK_UU_TYPEOF_UU:
                        AppendGListData(&pplist, "__typeof__", 10);
                        break;
                    case TK_UU_ATTRIBUTE_UU:
                        AppendGListData(&pplist, "__attribute__", 13);
                        break;
                    case TK_UU_ALIGNOF_UU:
                        AppendGListData(&pplist, "__alignof__", 11);
                        break;
                    case TK_UU_UUIDOF:
                        AppendGListData(&pplist, "__uuidof", 8);
                        break;
                    case TK_U_COMPLEX:
                        AppendGListData(&pplist, "_Complex", 8);
                        break;
                    case TK_U_IMAGINARY:
                        AppendGListData(&pplist, "_Imaginary", 10);
                        break;

                    case TK_STRING:
                        if (ispascalstring) {
                            AppendGListData(&pplist, "\"\\p", 3);
                            CPrep_DumpString((UInt8*)tkstring + 1, tksize - 1);
                        } else {
                            AppendGListByte(&pplist, '"');
                            CPrep_DumpString((UInt8*)tkstring, tksize - 1);
                        }
                        AppendGListByte(&pplist, '"');
                        break;

                    case TK_STRING_WIDE:
                        AppendGListData(&pplist, "L\"", 2);
                        CPrep_DumpWString((UInt16*)tkstring, (tksize / stwchar.size) - 1);
                        AppendGListByte(&pplist, '"');
                        break;

                    default:
                        if (cprep_nostring) {
                            if (innertoken == startToken) {
                                depth++;
                            } else if (innertoken == endToken) {
                                if (--depth == 0) {
                                    cprep_nostring = FALSE;
                                    cprep_in_assembler = FALSE;
                                    copts.cplusplus = save_cplusplus;
                                    copts.asmsemicolcomment = save_asmsemicolcomment;
                                    copts.asmpoundcomment = save_asmpoundcomment;
                                }
                            }
                        }
                        if (token >= 32 && token <= 255) {
                            AppendGListByte(&pplist, token);
                        } else {
                            CError_FATAL(656);
                        }
                }
                break;
            }

            CPrep_TokenStreamFlush();
            nlflag = FALSE;
            spaceskip = FALSE;
            lasttoken = token;
        } while ((token = lex()));
    }

    AppendGListData(&pplist, unk5E95F7, 3);
    COS_ResizeHandle(pplist.data, pplist.size);
}

#undef CERROR_FILE
#define CERROR_FILE gBrowseFileName

void CBrowse_Setup(CompilerLinkerParamBlk* params) {
    BrowseHeader hdr;
    UInt8 lang;

    CError_ASSERT(122, params != NULL);

    params->object.browsedata = NULL;

    if (InitGList(&gBrowseData, 0x10000)) {
        CError_NoMem();
    }

    if (InitGList(&gMemberFuncList, 1024)) {
        CError_NoMem();
    }

    gNextMemberFuncID = 1;
    gForceSourceLoc = FALSE;
    gUseNameTable = FALSE;

    memclrw(&hdr, sizeof(hdr));
    hdr.browse_header = 0xBEABBAEB;
    hdr.browse_version = 2;
    hdr.earliest_compatible_version = 2;
    if (copts.cplusplus) {
        lang = langCPlus;
    } else {
        lang = langC;
    }

    hdr.browse_language = lang;
    hdr.uses_name_table = gUseNameTable;

    AppendGListData(&gBrowseData, &hdr, sizeof(hdr));
}

void CBrowse_Finish(CompilerLinkerParamBlk* params) {
    CError_ASSERT(155, params != NULL);

    if (gBrowseData.size >= sizeof(BrowseHeader)) {
        RecordUndefinedMemberFunctions();
        AppendGListByte(&gBrowseData, -1);

        CCompilerSaveBrowserData(params, &gBrowseData);
    }
}

void CBrowse_Cleanup(CompilerLinkerParamBlk* params) {
    FreeGList(&gBrowseData);
    FreeGList(&gClassData);
    FreeGList(&gMemberFuncList);
}

static void AppendGList(GList* dst, GList* src) {
    SInt32 offset = dst->size;

    AppendGListNoData(dst, src->size);
    memcpy(*dst->data + offset, *src->data, src->size);
}

static void RecordName(GList* gl, const char* str, SInt32 id) {
    HashNameNode* name;

    CError_ASSERT(189, gl && str && *str);

    if (id < 0 && gUseNameTable) {
        for (name = name_hash_nodes[CHash(str)]; name; name = name->next) {
            if (!strcmp(str, name->name)) {
                id = name->id;
                break;
            }
        }
    }

    if (id >= 0 && gUseNameTable) {
        AppendGListWord(gl, -1);
        AppendGListLong(gl, id);
    } else {
        int len = strlen(str);
        AppendGListWord(gl, len);
        if (len) {
            AppendGListData(gl, str, len + 1);
        }
    }
}

void CBrowse_BeginClass(DeclInfo* di, GList* gl) {
    ClassList* base;
    SInt32 i;
    TypeClass* tclass;
    char* baseName;

    CError_ASSERT(226, di && di->thetype && gl);

    *gl = gClassData;

    if (!di->file ||
        !di->file->fileID ||
        !di->file->recordbrowseinfo ||
        !di->file2 ||
        !di->file2->fileID ||
        di->sourceoffset <= 0) {
        memclrw(&gClassData, sizeof(gClassData));
        return;
    }

    if (IsTempName(TYPE_CLASS(di->thetype)->classname)) {
        memclrw(&gClassData, sizeof(gClassData));
        return;
    }

    if (InitGList(&gClassData, 0x4000)) {
        CError_NoMem();
    }

    AppendGListByte(&gClassData, browseClass);
    AppendGListWord(&gClassData, di->file->fileID);
    AppendGListWord(&gClassData, di->file2->fileID);
    AppendGListLong(&gClassData, di->sourceoffset - 1);
    CError_ASSERT(269, gClassData.size == 9);
    AppendGListLong(&gClassData, di->sourceoffset - 1);
    AppendGListLong(&gClassData, 0);
    RecordName(
        &gClassData,
        TYPE_CLASS(di->thetype)->classname->name,
        TYPE_CLASS(di->thetype)->classname->id
    );

    baseName = CError_GetTypeName(di->thetype, 0, 0);

    if (strcmp(TYPE_CLASS(di->thetype)->classname->name, baseName)) {
        RecordName(&gClassData, baseName, -1);
    } else {
        AppendGListWord(&gClassData, 0);
    }

    AppendGListLong(&gClassData, 0);

    i = 0;
    {
        ClassList* countBase;
        for (countBase = TYPE_CLASS(di->thetype)->bases; countBase; countBase = countBase->next) {
            i++;
        }
    }

    AppendGListByte(&gClassData, i);

    for (base = TYPE_CLASS(di->thetype)->bases; base; base = base->next) {
        AppendGListByte(&gClassData, gFromAccessType[base->access]);
        AppendGListByte(&gClassData, base->is_virtual);

        tclass = base->base;
        if (tclass->flags & CLASS_IS_TEMPL_INST) {
            TemplClassInst* inst = CTemplTool_Class2TemplClassInst(tclass);
            if (!inst->is_specialized) {
                tclass = CTemplTool_Template2TemplClass(inst->templ);
            }
        }

        baseName = CError_GetTypeName(TYPE(tclass), 0, 0);
        while (*baseName && *baseName >= '0' && *baseName <= '9') {
            baseName++;
        }

        i = base->base->classname->id;
        while (*baseName && *baseName >= '0' && *baseName <= '9') {
            baseName++;
            i = -1;
        }

        RecordName(&gClassData, baseName, i);
    }
}

void CBrowse_AddClassMemberVar(ObjMemberVar* ivar, SInt32 startOffset, SInt32 endOffset) {
    short len;

    CError_ASSERT(372, ivar);

    if (gClassData.data && startOffset > 0 && endOffset >= startOffset) {
        if (tk == ';') {
            endOffset++;
        }

        AppendGListByte(&gClassData, memberData);
        AppendGListByte(&gClassData, gFromAccessType[ivar->access]);
        AppendGListLong(&gClassData, 0);
        AppendGListLong(&gClassData, startOffset - 1);
        AppendGListLong(&gClassData, endOffset - 1);

        len = strlen(ivar->name->name);
        AppendGListWord(&gClassData, len);
        AppendGListData(&gClassData, ivar->name->name, len + 1);
    }
}

void CBrowse_AddClassMemberFunction(Object* object, SInt32 startOffset, SInt32 endOffset) {
    SInt32 flags;
    SInt32 id;
    TypeMemberFunc* tfunc;

    CError_ASSERT(392, object);

    if (gClassData.data && startOffset > 0 && endOffset >= startOffset) {
        flags = 0;
        CError_ASSERT(403, object->type && IS_TYPE_FUNC(object->type));
        tfunc = TYPE_METHOD(object->type);

        if (!(tfunc->flags & FUNC_AUTO_GENERATED)) {
            if (object->datatype == DVFUNC) {
                flags |= kVirtual;
            }
            if (tfunc->flags & FUNC_PURE) {
                flags |= kAbstract;
            }
            if (tfunc->is_static) {
                flags |= kStatic;
            }
            if (tfunc->flags & FUNC_IS_CTOR) {
                flags |= kCtor;
            }
            if (tfunc->flags & FUNC_IS_DTOR) {
                flags |= kDtor;
            }

            AppendGListByte(&gClassData, memberFunction);
            AppendGListByte(&gClassData, gFromAccessType[object->access]);
            AppendGListLong(&gClassData, flags);

            id = tfunc->funcid;
            if (id <= 0) {
                if (!(tfunc->flags & FUNC_DEFINED) || id == -1) {
                    AppendGListLong(&gMemberFuncList, (SInt32)object);
                }
                id = gNextMemberFuncID++;
                tfunc->funcid = id;
            }

            AppendGListLong(&gClassData, id);
            AppendGListLong(&gClassData, startOffset - 1);
            AppendGListLong(&gClassData, endOffset);
        }
    }
}

void CBrowse_AddClassMemberData(Object* object, SInt32 startOffset, SInt32 endOffset) {
    short len;

    CError_ASSERT(447, object);

    if (gClassData.data &&
        startOffset > 0 &&
        endOffset >= startOffset &&
        CParser_IsDataObject(object)) {
        if (tk == ';') {
            endOffset++;
        }

        AppendGListByte(&gClassData, memberData);
        AppendGListByte(&gClassData, gFromAccessType[object->access]);
        AppendGListLong(&gClassData, kStatic);
        AppendGListLong(&gClassData, startOffset - 1);
        AppendGListLong(&gClassData, endOffset - 1);

        len = strlen(object->name->name);
        AppendGListWord(&gClassData, len);
        AppendGListData(&gClassData, object->name->name, len + 1);
    }
}

void CBrowse_EndClass(volatile SInt32 offset, GList* gl) {
    CError_ASSERT(465, gl);

    if (gClassData.data) {
        if (gClassData.size > 0) {
            if (tk == ';') {
                offset++;
            }
            ((BrowseClassHeader*)*gClassData.data)->end_offset = offset;
            AppendGList(&gBrowseData, &gClassData);
            AppendGListByte(&gBrowseData, (SInt8)memberEnd);
        }
        FreeGList(&gClassData);
    }

    gClassData = *gl;
}

void CBrowse_BeginStruct(DeclInfo* di, TypeStruct* tstruct, GList* gl) {
    HashNameNode* name;

    CError_ASSERT(492, di && gl);

    *gl = gClassData;

    if (!di->file ||
        !di->file->fileID ||
        !di->file->recordbrowseinfo ||
        !di->file2 ||
        !di->file2->fileID ||
        di->sourceoffset <= 0) {
        memclrw(&gClassData, sizeof(gClassData));
        return;
    }

    name = tstruct->name;
    if (!name || IsTempName(name)) {
        memclrw(&gClassData, sizeof(gClassData));
        return;
    }

    if (InitGList(&gClassData, 0x4000)) {
        CError_NoMem();
    }

    AppendGListByte(&gClassData, browseClass);
    AppendGListWord(&gClassData, di->file->fileID);
    AppendGListWord(&gClassData, di->file2->fileID);
    AppendGListLong(&gClassData, di->sourceoffset - 1);
    CError_ASSERT(533, gClassData.size == 9);
    AppendGListLong(&gClassData, di->sourceoffset - 1);
    AppendGListLong(&gClassData, 0);
    RecordName(&gClassData, name->name, name->id);
    AppendGListWord(&gClassData, 0);
    AppendGListLong(&gClassData, 0);
    AppendGListByte(&gClassData, 0);
}

void CBrowse_AddStructMember(Object* member, SInt32 startOffset, SInt32 endOffset) {
    short len;

    if (tk == ';') {
        endOffset++;
    }

    if (gClassData.data && member && startOffset > 0 && endOffset >= startOffset) {
        AppendGListByte(&gClassData, memberData);
        AppendGListByte(&gClassData, accessPublic);
        AppendGListLong(&gClassData, 0);
        AppendGListLong(&gClassData, startOffset - 1);
        AppendGListLong(&gClassData, endOffset - 1);

        len = strlen(member->name->name);
        AppendGListWord(&gClassData, len);
        AppendGListData(&gClassData, member->name->name, len + 1);
    }
}

void CBrowse_EndStruct(volatile SInt32 offset, GList* gl) {
    CError_ASSERT(570, gl);

    if (gClassData.data) {
        if (offset > 0 && gClassData.size > 0) {
            ((BrowseClassHeader*)*gClassData.data)->end_offset = offset;
            AppendGList(&gBrowseData, &gClassData);
            AppendGListByte(&gBrowseData, (SInt8)memberEnd);
        }
        FreeGList(&gClassData);
    }

    gClassData = *gl;
}

static void EmitStandardData(
    enum EBrowserItem item,
    int fileID1,
    int fileID2,
    SInt32 startOffset,
    SInt32 endOffset,
    const char* str,
    SInt32 id,
    const char* str2,
    SInt32 id2
) {
    CError_ASSERT(596, str);

    AppendGListByte(&gBrowseData, item);
    AppendGListWord(&gBrowseData, fileID1);
    AppendGListWord(&gBrowseData, fileID2);
    AppendGListLong(&gBrowseData, startOffset - 1);
    AppendGListLong(&gBrowseData, endOffset - 1);
    AppendGListLong(&gBrowseData, 0);

    RecordName(&gBrowseData, str, id);
    if (str2 && str2 != str) {
        RecordName(&gBrowseData, str2, id2);
    } else {
        AppendGListWord(&gBrowseData, 0);
    }
}

void CBrowse_NewTypedef(
    NameSpace* nspace,
    HashNameNode* name,
    CPrepFileInfo* file1,
    CPrepFileInfo* file2,
    SInt32 startOffset,
    SInt32 endOffset
) {
    CError_ASSERT(630, file1 && file1->recordbrowseinfo);

    if (file2 && file2->fileID && startOffset > 0 && endOffset >= startOffset) {
        char* qname = CError_GetQualifiedName(nspace, name);
        EmitStandardData(
            browseTypedef,
            file1->fileID,
            file2->fileID,
            startOffset,
            endOffset,
            name->name,
            name->id,
            qname,
            -1
        );
    }
}

void CBrowse_NewEnum(
    NameSpace* nspace,
    HashNameNode* name,
    CPrepFileInfo* file1,
    CPrepFileInfo* file2,
    SInt32 startOffset,
    SInt32 endOffset
) {
    CError_ASSERT(644, file1 && file1->recordbrowseinfo);

    if (file2 && file2->fileID && startOffset > 0 && endOffset >= startOffset) {
        char* qname = CError_GetQualifiedName(nspace, name);
        EmitStandardData(
            browseEnum,
            file1->fileID,
            file2->fileID,
            startOffset,
            endOffset,
            name->name,
            name->id,
            qname,
            -1
        );
    }
}

void CBrowse_NewEnumConstant(
    NameSpace* nspace,
    HashNameNode* name,
    CPrepFileInfo* file1,
    CPrepFileInfo* file2,
    SInt32 startOffset,
    SInt32 endOffset
) {
    CError_ASSERT(658, file1 && file1->recordbrowseinfo);

    if (tk == ',') {
        endOffset++;
    }

    if (file2 && file2->fileID && startOffset > 0 && endOffset >= startOffset) {
        char* qname = CError_GetQualifiedName(nspace, name);
        EmitStandardData(
            browseConstant,
            file1->fileID,
            file2->fileID,
            startOffset,
            endOffset,
            name->name,
            name->id,
            qname,
            -1
        );
    }
}

static void RecordFunction(
    Object* object, int fileID1, int fileID2, SInt32 startOffset, SInt32 endOffset
) {
    TypeMemberFunc* tfunc;
    SInt32 flags;
    SInt32 funcid;
    char* namestr;
    SInt32 nameid;
    char* namestr2;
    SInt32 nameid2;
    HashNameNode* objname;
    char dtorname[256];
    char* opname;

    CError_ASSERT(761, object->type && IS_TYPE_FUNC(object->type));

    tfunc = TYPE_METHOD(object->type);
    if ((tfunc->flags & (FUNC_AUTO_GENERATED | FUNC_INTRINSIC)) && (!fileID2 || startOffset < 0)) {
        return;
    }

    objname = GetHashNameNodeExport(CError_GetObjectName(object));
    namestr2 = objname->name;
    nameid2 = objname->id;

    if (tfunc->flags & FUNC_IS_CTOR) {
        namestr = tfunc->theclass->classname->name;
        nameid = tfunc->theclass->classname->id;
    } else if (tfunc->flags & FUNC_IS_DTOR) {
        dtorname[0] = '~';
        strncpy(&dtorname[1], tfunc->theclass->classname->name, sizeof(dtorname) - 1);
        dtorname[255] = 0;
        namestr = dtorname;
        nameid = -1;
    } else if ((opname = CMangler_GetOperator(object->name)) != NULL) {
        namestr = opname;
        nameid = -1;
    } else if (!IsTempName(object->name)) {
        namestr = object->name->name;
        nameid = object->name->id;
    } else {
        namestr = namestr2;
        nameid = nameid2;
    }

    EmitStandardData(
        browseFunction, fileID1, fileID2, startOffset, endOffset, namestr, nameid, namestr2, nameid2
    );

    flags = 0;
    if (object->qual & Q_INLINE) {
        flags |= kInline;
    }
    if (object->qual & Q_PASCAL) {
        flags |= kPascal;
    }
    if (object->qual & Q_ASM) {
        flags |= kAsm;
    }
    if (object->sclass == TK_STATIC) {
        flags |= kStatic;
    }
    if (tfunc->flags & FUNC_METHOD) {
        flags |= kMember;
    }
    AppendGListLong(&gBrowseData, flags);

    funcid = 0;
    if (tfunc->flags & FUNC_METHOD) {
        funcid = tfunc->funcid;
        if (funcid <= 0) {
            funcid = gNextMemberFuncID++;
            tfunc->funcid = funcid;
        }
    }
    AppendGListLong(&gBrowseData, funcid);
}

void CBrowse_NewFunction(
    Object* object, CPrepFileInfo* file1, CPrepFileInfo* file2, SInt32 startOffset, SInt32 endOffset
) {
    CError_ASSERT(952, file1 && file1->recordbrowseinfo);

    if (file2 && file2->fileID && startOffset > 0 && (endOffset + 1) >= startOffset) {
        RecordFunction(object, file1->fileID, file2->fileID, startOffset, endOffset + 1);
    }
}

void CBrowse_NewData(
    Object* object, CPrepFileInfo* file1, CPrepFileInfo* file2, SInt32 startOffset, SInt32 endOffset
) {
    char* namestr = NULL;
    SInt32 flags = 0;
    Boolean is_const = is_const_object(object);
    SInt32 linkid;
    SInt32 nameid;

    CError_ASSERT(974, file1 && file1->recordbrowseinfo);
    CError_ASSERT(975, object);

    if (tk == ';') {
        endOffset++;
    }

    if (file2 && file2->fileID && startOffset > 0 && endOffset >= startOffset) {
        HashNameNode* name = GetHashNameNodeExport(CError_GetObjectName(object));
        if (object->name != name) {
            namestr = name->name;
        }

        linkid = name->id;
        nameid = object->name->id;
        EmitStandardData(
            is_const ? browseConstant : browseGlobal,
            file1->fileID,
            file2->fileID,
            startOffset,
            endOffset,
            object->name->name,
            nameid,
            namestr,
            linkid
        );

        if (!is_const) {
            if (object->sclass == TK_STATIC) {
                flags |= kStatic;
            }
            AppendGListLong(&gBrowseData, flags);
        }
    }
}

void CBrowse_NewMacro(Macro* macro, CPrepFileInfo* file, SInt32 startOffset, SInt32 endOffset) {
    CError_ASSERT(1013, !file || (file->recordbrowseinfo && !macro->is_special));

    if (file && file->fileID && startOffset > 0 && endOffset >= startOffset) {
        EmitStandardData(
            browseMacro,
            file->fileID,
            file->fileID,
            startOffset,
            endOffset,
            macro->name->name,
            macro->name->id,
            NULL,
            -1
        );
    }
}

void CBrowse_NewTemplateClass(
    TemplClass* tmclass, CPrepFileInfo* file, SInt32 startOffset, SInt32 endOffset
) {
    CError_ASSERT(1025, !file || file->recordbrowseinfo);

    if (file && file->fileID && startOffset > 0 && endOffset >= startOffset) {
        HashNameNode* objname =
            GetHashNameNodeExport(CError_GetTypeName(TYPE(&tmclass->theclass), 0, 0));
        EmitStandardData(
            browseTemplate,
            file->fileID,
            file->fileID,
            startOffset,
            endOffset,
            objname->name,
            objname->id,
            NULL,
            -1
        );
        AppendGListByte(&gBrowseData, templateClass);
    }
}

void CBrowse_NewTemplateFunc(TemplateFunction* tmfunc) {
    CError_ASSERT(1047, !tmfunc->srcfile || tmfunc->srcfile->recordbrowseinfo);

    if (tmfunc->srcfile &&
        tmfunc->srcfile->fileID &&
        tmfunc->startoffset > 0 &&
        tmfunc->endoffset >= tmfunc->startoffset) {
        HashNameNode* objname = GetHashNameNodeExport(CError_GetObjectName(tmfunc->tfunc));
        EmitStandardData(
            browseTemplate,
            tmfunc->srcfile->fileID,
            tmfunc->srcfile->fileID,
            tmfunc->startoffset,
            tmfunc->endoffset,
            objname->name,
            objname->id,
            NULL,
            -1
        );
        AppendGListByte(&gBrowseData, templateFunction);
    }
}

static void RecordUndefinedMemberFunctions(void) {
    int i;
    int count;
    Object** array;

    COS_LockHandleHi(gMemberFuncList.data);

    count = gMemberFuncList.size / sizeof(Object*);
    array = (Object**)*gMemberFuncList.data;
    for (i = 0; i < count; i++, array++) {
        if (IS_TYPE_FUNC((*array)->type) && !(TYPE_FUNC((*array)->type)->flags & FUNC_DEFINED)) {
            RecordFunction(*(Object* volatile*)array, 0, 0, -1, -1);
        }
    }

    COS_UnlockHandle(gMemberFuncList.data);
}
