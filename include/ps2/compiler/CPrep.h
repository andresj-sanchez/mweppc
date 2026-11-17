#ifndef COMPILER_CPREP_H
#define COMPILER_CPREP_H

#include "compiler/common.h"
#include "compiler/tokens.h"
#include "driver/CWPlugins.h"
#include "driver/DropInCompilerLinker.h"

struct CompilerLinkerParamBlk {
    struct CWPluginPrivateContext* context; /* 0x00 */
    CWObjectData object; /* 0x04 */
    SInt32 request; /* 0x34 */
    SInt32 version; /* 0x38 */
    CWFileSpec targetfile; /* 0x3C */
    SInt32 numfiles; /* 0x140 */
    SInt32 whichfile; /* 0x144 */
    CWFileSpec sourcefile; /* 0x148 */
    const char* sourcetext; /* 0x24C */
    SInt32 sourcetextsize; /* 0x250 */
    Boolean precompile; /* 0x254 */
    Boolean autoprecompile; /* 0x255 */
    Boolean preprocess; /* 0x256 */
    Boolean SYMinfo; /* 0x257 */
    Boolean caching_includes; /* 0x258 */
    CWBrowseOptions browseoptions; /* 0x25A */
    Boolean recordbrowseinfo; /* 0x26A */
    SInt16 browserfileID; /* 0x26C */
    SInt32 targetOS; /* 0x270 */
    SInt32 targetCPU; /* 0x274 */
    char* idetargetname; /* 0x278 */
};

struct COptPush {
    struct COptPush* next;
    SInt32 id;
    Boolean pushed;
    UInt8 value;
};

extern CompilerLinkerParamBlk* cparamblkptr;

extern SInt16 notendofline(void);
extern TStreamElement* CPrep_CurStreamElement(void);
extern void fn_00443E10(void* stream, UInt32* state);
extern void fn_00443DA0(void* stream, UInt32* state);
extern TStreamElement symdecltoken_00623E50;
extern GList pplist;
extern SInt16 filesp;

#pragma pack(push, 2)
struct CPrepFileInfo {
    CWFileSpec textfile;
    char* filename;
    SInt32 fileModDate;
    SInt32 linedelta;
    char* textbuffer;
    SInt32 textlength;
    SInt32 linenumber;
    SInt32 pos;
    Boolean hasprepline;
    SInt16 fileID; /* 0x122 */
    Boolean recordbrowseinfo; /* 0x124 */
};
#pragma pack(pop)

struct Macro {
    struct Macro* next;
    struct HashNameNode* name;
    char* text;
    UInt16 nargs;
    UInt8 isExpanding;
    Boolean is_special;
    struct HashNameNode* names[1];
};

extern Macro** macrohashtable;
extern void CPrep_RemoveSpecialMacros(void);
extern void CPrep_InsertSpecialMacros(void);
extern void CPrep_InsertSpecialMacro(Macro* macro, const char* name);

extern void CPrep_PushOption(SInt32 id, char value);

extern char* tkstring;
extern HashNameNode* tkidentifier;
extern SInt32 tksize;
extern SInt16 ispascalstring;
extern SInt16 nlflag;
extern char* prep_file_end;
extern UInt8 spaceskip;
extern UInt8 cprep_nostring;
extern SInt16 tokenstacklevel;
extern char* pos;
extern char* macropos;
extern char* prep_file_start;
extern TStreamElement* ts_current;
extern void CPrep_Warning(SInt16 code);
extern void CPrep_TokenStreamFlush(void);

extern SInt32 CPrep_GetFileOffsetInfo(void* info);

extern void fn_00444740(void* state);
extern void fn_00444780(void* state);

extern void fn_00442830(void);

extern void fn_00443640(UInt32 token);

extern TStreamElement dummyelement;
extern TStreamElement* ts_first;
extern UInt8 cprep_eoltokens;
extern UInt8 cprep_in_assembler;

extern void fn_00443380(void* fileoffset);

extern void fn_004446B0(SInt32* state);

extern UInt8 fn_0043F720(void);
extern void fn_0043A580(void);
extern void fn_00442790(void);

extern void fn_00439D90(void);
extern void fn_00439DA0(void);
extern void fn_00439DB0(void);
extern void fn_00439DC0(void);
extern void fn_0043A3C0(void);

extern struct COptPush* coptpush;
extern SInt32 lines;

#endif
