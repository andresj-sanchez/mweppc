#ifndef COMPILER_TOKENS_H
#define COMPILER_TOKENS_H

#include "compiler/common.h"
#include "compiler/CInt64.h"
#include "compiler/enode.h"

#define TK_STRING_WIDE -5
#define TK_STRING -4
#define TK_IDENTIFIER -3
#define TK_FLOATCONST -2
#define TK_INTCONST -1
#define TK_VOID 262
#define TK_TYPEDEF 260
#define TK_CONST 289
#define TK_VOLATILE 290
#define TK_DELETE 327
#define TK_NEW 329
#define TK_MULT_ASSIGN 350
#define TK_DIV_ASSIGN 351
#define TK_STATIC 258
#define TK_AUTO 256
#define TK_REGISTER 257
#define TK_MOD_ASSIGN 352
#define TK_ADD_ASSIGN 353
#define TK_SUB_ASSIGN 354
#define TK_SHL_ASSIGN 355
#define TK_SHR_ASSIGN 356
#define TK_AND_ASSIGN 357
#define TK_XOR_ASSIGN 358
#define TK_OR_ASSIGN 359
#define TK_THROW 336
#define TK_LOGICAL_OR 360
#define TK_LOGICAL_AND 361
#define TK_LOGICAL_EQ 362
#define TK_LOGICAL_NE 363
#define TK_LESS_EQUAL 364
#define TK_GREATER_EQUAL 365
#define TK_SHL 366
#define TK_SHR 367
#define TK_INCREMENT 368
#define TK_DECREMENT 369
#define TK_ARROW 370
#define TK_ARROW_STAR 373
#define TK_MIN 375
#define TK_MAX 376
#define TK_NEW_ARRAY 390
#define TK_DELETE_ARRAY 391
#define TK_EXTERN 259
#define TK_INLINE 261
#define TK_CHAR 263
#define TK_SHORT 264
#define TK_INT 265
#define TK_LONG 266
#define TK_FLOAT 267
#define TK_DOUBLE 268
#define TK_SIGNED 269
#define TK_UNSIGNED 270
#define TK_STRUCT 271
#define TK_UNION 272
#define TK_ENUM 273
#define TK_CLASS 274
#define TK_UU_FIXED_8X8 275
#define TK_UU_INT128 276
#define TK_UU_TYPEOF_UU 283
#define TK_BOOL 284
#define TK_WCHAR_T 285
#define TK_U_COMPLEX 286
#define TK_U_IMAGINARY 287
#define TK_TYPENAME 288
#define TK_PASCAL 291
#define TK_UU_DECLSPEC 292
#define TK_UU_STDCALL 293
#define TK_UU_CDECL 294
#define TK_UU_FASTCALL 295
#define TK_UU_3DCALL 296
#define TK_UU_SSECALL 297
#define TK_UU_SSE2CALL 298
#define TK_UU_FAR 299
#define TK_EXPLICIT 300
#define TK_MUTABLE 301
#define TK_ONEWAY 302
#define TK_IN 303
#define TK_INOUT 304
#define TK_OUT 305
#define TK_BYCOPY 306
#define TK_BYREF 307
#define TK_ASM 312
#define TK_CASE 313
#define TK_DEFAULT 314
#define TK_IF 315
#define TK_ELSE 316
#define TK_SWITCH 317
#define TK_WHILE 318
#define TK_DO 319
#define TK_FOR 320
#define TK_GOTO 321
#define TK_CONTINUE 322
#define TK_BREAK 323
#define TK_RETURN 324
#define TK_SIZEOF 325
#define TK_CATCH 326
#define TK_FRIEND 328
#define TK_OPERATOR 330
#define TK_PRIVATE 331
#define TK_PROTECTED 332
#define TK_PUBLIC 333
#define TK_TEMPLATE 334
#define TK_THIS 335
#define TK_TRY 337
#define TK_VIRTUAL 338
#define TK_INHERITED 339
#define TK_CONST_CAST 340
#define TK_DYNAMIC_CAST 341
#define TK_NAMESPACE 342
#define TK_REINTERPRET_CAST 343
#define TK_STATIC_CAST 344
#define TK_USING 345
#define TK_TRUE 346
#define TK_FALSE 347
#define TK_TYPEID 348
#define TK_EXPORT 349
#define TK_ELLIPSIS 371
#define TK_DOT_STAR 372
#define TK_COLON_COLON 374
#define TK_AT_INTERFACE 377
#define TK_AT_IMPLEMENTATION 378
#define TK_AT_PROTOCOL 379
#define TK_AT_END 380
#define TK_AT_PRIVATE 381
#define TK_AT_PROTECTED 382
#define TK_AT_PUBLIC 383
#define TK_AT_CLASS 384
#define TK_AT_SELECTOR 385
#define TK_AT_ENCODE 386
#define TK_AT_DEFS 387
#define TK_SELF 388
#define TK_SUPER 389
#define TK_UU_ALIGNOF_UU 392
#define TK_RESTRICT 393
#define TK_UU_ATTRIBUTE_UU 394
#define TK_UU_UUIDOF 395

typedef union TData {
    struct HashNameNode* tkidentifier;
    CInt64 tkintconst;
    Float tkfloatconst;
    struct {
        char* data;
        SInt32 size;
    } tkstring;
} TData;

#pragma pack(4)
struct TStreamElement {
    SInt16 tokentype;
    SInt16 subtype;
    struct CPrepFileInfo* tokenfile;
    SInt32 tokenoffset;
    SInt32 tokenline;
    TData data;
};
#pragma pack()

struct FileOffsetInfo {
    struct CPrepFileInfo* file; /* 0x00 */
    SInt32 tokenline; /* 0x04 */
    SInt32 tokenoffset; /* 0x08 */
    Boolean is_inline; /* 0x0C */
};

struct TokenStream {
    SInt32 tokens;
    TStreamElement* firsttoken;
    SInt32 unk8;
    SInt32 unkC;
    SInt32 unk10;
};

extern CInt64 tkintconst;

#endif
