#ifndef COMPILER_CPREPTOKENIZER_H
#define COMPILER_CPREPTOKENIZER_H
#include "compiler/common.h"
extern SInt32 prepskipnextchar(void);
extern char* nextcharpos;
extern SInt16 lookahead(void);
extern SInt16 fn_00468780(void);
extern SInt32 fn_004680B0(void);
extern SInt16 t0x1a(void);
extern SInt16 tisid(void);
extern SInt16 tdoll(SInt16 token);
extern SInt16 token(SInt16 t);
extern SInt16 tille(void);
extern SInt16 tcret(void);
extern SInt16 tnull(SInt16 t);
extern SInt16 thash(void);
extern SInt16 tdivi(SInt16 t);
extern SInt16 tpowe(SInt16 t);
extern SInt16 tampe(SInt16 t);
extern SInt16 torrr(SInt16 t);
extern SInt16 tmult(void);
extern SInt16 texcl(void);
extern SInt16 tequa(void);
extern SInt16 tplus(SInt16 t);
extern SInt16 tminu(SInt16 t);
extern SInt16 tperc(SInt16 t);
extern SInt16 tcolo(void);
extern SInt16 tatsg(SInt16 t);
extern SInt16 tiden(void);
extern SInt16 tgrea(SInt16 t);
extern SInt16 tless(SInt16 t);

extern SInt16 plex(void);
extern void skipendofline(void);

extern StringPtr CPrep_SkipNewComment(StringPtr str);
extern UInt8 CPrep_SkipNewCommentChar;

extern UInt8 cprep_macrocheck;
extern UInt8 register_newline_637381;
extern UInt8 register_preprocessing_only_6373A3;

#endif
