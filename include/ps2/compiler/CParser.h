#ifndef COMPILER_CPARSER_H
#define COMPILER_CPARSER_H

#include "compiler/common.h"
#include "compiler/types.h"
#include "compiler/CDecl.h"
#include "compiler/enode.h"
#include <setjmp.h>
#include "compiler/tokens.h"

#pragma pack(push, 2)
struct CompilerLinkerOptions {
    Boolean littleendian; // 0x000
    UInt8 unknown01; // 0x001
    UInt16 unknown02; // 0x002
    SInt16 unknown04; // 0x004
    SInt8 fp_library; // 0x006
    UInt8 unknown07; // 0x007
    UInt8 unknown08; // 0x008
    UInt8 unknown09; // 0x009
    UInt16 unknown0A; // 0x00a
    SInt16 unknown0C; // 0x00c
    UInt8 unknown0E; // 0x00e
    Boolean unknown0F; // 0x00f
    UInt8 unknown10; // 0x010
    UInt8 unknown11; // 0x011
    UInt8 inline_intrinsics; // 0x012
    UInt8 debuglisting; // 0x013
    UInt8 interrupt; // 0x014
    UInt8 unknown15; // 0x015
    UInt8 unknown16; // 0x016
    UInt8 unknown17; // 0x017
    UInt8 cats; // 0x018
    UInt8 unknown19; // 0x019
    UInt8 unknown1A; // 0x01a
    UInt8 warn_inlineasm; // 0x01b
    UInt8 conditional_move; // 0x01c
    UInt8 late_conditional_move; // 0x01d
    UInt8 gencvtcheck; // 0x01e
    UInt8 far_call; // 0x01f
    UInt8 prefetch; // 0x020
    UInt8 trunc; // 0x021
    UInt8 sqrt; // 0x022
    UInt8 unknown23; // 0x023
    UInt8 floatsp32; // 0x024
    UInt8 unknown25; // 0x025
    UInt8 unknown26; // 0x026
    Boolean no_branch_likely; // 0x027
    UInt8 debug_1; // 0x028
    UInt8 debug_2; // 0x029
    UInt8 debug_3; // 0x02a
    UInt8 debug_4; // 0x02b
    UInt8 debug_5; // 0x02c
    UInt8 unknown2D; // 0x02d
    Boolean unknown2E; // 0x02e
    UInt8 unknown2F; // 0x02f
    UInt8 unknown30; // 0x030
    UInt8 unknown31; // 0x031
    UInt8 nofpregforblkmv; // 0x032
    UInt8 unknown33; // 0x033
    UInt8 schedule; // 0x034
    UInt8 tailcall; // 0x035
    UInt8 unknown36; // 0x036
    UInt8 divbyzerocheck; // 0x037
    UInt8 nomacro; // 0x038
    UInt8 unknown39; // 0x039
    UInt8 unknown3A; // 0x03a
    UInt8 cse_hard_reg_gpr; // 0x03b
    UInt8 cse_hard_reg_fpr; // 0x03c
    UInt8 cse_hard_reg_special; // 0x03d
    UInt8 dividefix; // 0x03e
    UInt8 usefloatacc; // 0x03f
    UInt8 padloop; // 0x040
    UInt8 unknown41; // 0x041
    UInt8 fast_fptosi; // 0x042
    UInt8 irocseglobaladdresses; // 0x043
    UInt8 enable_vu0_registers; // 0x044
    UInt8 fastmath; // 0x045
    UInt8 vu0_register_number; // 0x046
    UInt8 unknown47; // 0x047
    UInt8 unknown48; // 0x048
    UInt8 cse_hard_reg_vu0; // 0x049
    UInt8 cse_vector_type; // 0x04a
    UInt8 volatileasm; // 0x04b
    UInt8 memoryrewrite; // 0x04c
    UInt8 unknown4D; // 0x04d
    UInt8 unknown4E; // 0x04e
    Boolean mwabi_bitfields; // 0x04f
    Boolean asmsemicolcomment; // 0x050
    UInt8 asmpoundcomment; // 0x051
    UInt8 unknown52; // 0x052
    UInt8 unknown53; // 0x053
    StringPtr oldprefixname; // 0x054
    SInt16 inlinelevel; // 0x058
    SInt16 unknown5A; // 0x05a
    SInt32 inlinemaxsize; // 0x05c
    SInt32 inlinemaxtotalsize; // 0x060
    Boolean inline_bottom_up; // 0x064
    Boolean inline_bottom_up_once; // 0x065
    UInt8 cplusplus; // 0x066
    UInt8 ecplusplus; // 0x067
    char ARMscoping; // 0x068
    Boolean checkprotos; // 0x069
    UInt8 trigraphs; // 0x06a
    UInt8 onlystdkeywords; // 0x06b
    UInt8 enumsalwaysint; // 0x06c
    UInt8 ANSIstrict; // 0x06d
    UInt8 mpwc_relax; // 0x06e
    UInt8 mpwc_newline; // 0x06f
    Boolean ignore_oldstyle; // 0x070
    Boolean cpp_extensions; // 0x071
    UInt8 pointercast_lvalue; // 0x072
    char RTTI; // 0x073
    UInt8 delete_exception; // 0x074
    UInt8 unknown75; // 0x075
    Boolean oldalignment; // 0x076
    Boolean unsigned_char; // 0x077
    UInt8 multibyteaware; // 0x078
    UInt8 auto_inline; // 0x079
    UInt8 defer_codegen; // 0x07a
    UInt8 direct_to_som; // 0x07b
    UInt8 som_env_check; // 0x07c
    UInt8 som_call_optimize; // 0x07d
    UInt8 booltruefalse; // 0x07e
    UInt8 longlong; // 0x07f
    UInt8 longlong_enums; // 0x080
    Boolean no_tfuncinline; // 0x081
    UInt8 unknown82; // 0x082
    char flat_include; // 0x083
    char syspath_once; // 0x084
    char always_import; // 0x085
    UInt8 simple_class_byval; // 0x086
    UInt8 wchar_type; // 0x087
    UInt8 vbase_ctor_offset; // 0x088
    char vbase_abi_v2; // 0x089
    char def_inherited; // 0x08a
    UInt8 template_patch; // 0x08b
    UInt8 template_friends; // 0x08c
    Boolean faster_pch_gen; // 0x08d
    UInt8 array_new_delete; // 0x08e
    UInt8 dollar_identifiers; // 0x08f
    Boolean def_inline_tfuncs; // 0x090
    UInt8 arg_dep_lookup; // 0x091
    char simple_prepdump; // 0x092
    UInt8 line_prepdump; // 0x093
    UInt8 fullpath_prepdump; // 0x094
    UInt8 unknown95; // 0x095
    Boolean old_mtemplparser; // 0x096
    UInt8 suppress_init_code; // 0x097
    Boolean reverse_bitfields; // 0x098
    UInt8 c9x; // 0x099
    Boolean float_constants; // 0x09a
    Boolean no_static_dtors; // 0x09b
    Boolean longlong_prepeval; // 0x09c
    UInt8 const_strings; // 0x09d
    UInt8 dumpir; // 0x09e
    UInt8 experimental; // 0x09f
    UInt8 late_fixes; // 0x0a0
    UInt8 gcc_extensions; // 0x0a1
    Boolean stdc_fp_contract; // 0x0a2
    UInt8 stdc_fenv_access; // 0x0a3
    Boolean stdc_cx_limitedr; // 0x0a4
    UInt8 old_argmatch; // 0x0a5
    char optEH; // 0x0a6
    Boolean optEH2; // 0x0a7
    Boolean new_mangler; // 0x0a8
    UInt8 parse_func_templ; // 0x0a9
    UInt8 parse_mfunc_templ; // 0x0aa
    UInt8 old_depfunc_lookup; // 0x0ab
    UInt8 show_error_filestack; // 0x0ac
    UInt8 defer_defarg_parsing; // 0x0ad
    UInt8 reg_class_allocs; // 0x0ae
    UInt8 old_pragma_once; // 0x0af
    UInt8 new_calldeduction; // 0x0b0
    Boolean fullpath_file; // 0x0b1
    UInt8 opt_classresults; // 0x0b2
    UInt8 ushort_wchar_t; // 0x0b3
    UInt8 thread_safe_init; // 0x0b4
    Boolean microsoft_EH; // 0x0b5
    UInt8 unknownB6; // 0x0b6
    char warningerrors; // 0x0b7
    UInt8 access_errors; // 0x0b8
    char pedantic; // 0x0b9
    UInt8 checkprecompflags; // 0x0ba
    Boolean suppress_warnings; // 0x0bb
    UInt8 warn_illpragma; // 0x0bc
    UInt8 warn_emptydecl; // 0x0bd
    UInt8 warn_possunwant; // 0x0be
    char warn_unusedvar; // 0x0bf
    char warn_unusedarg; // 0x0c0
    UInt8 warn_extracomma; // 0x0c1
    Boolean warn_hidevirtual; // 0x0c2
    char warn_largeargs; // 0x0c3
    Boolean warn_implicitconv; // 0x0c4
    UInt8 warn_impl_i2f_conv; // 0x0c5
    UInt8 warn_impl_f2i_conv; // 0x0c6
    UInt8 warn_impl_s2u_conv; // 0x0c7
    UInt8 warn_notinlined; // 0x0c8
    SInt8 warn_structclass; // 0x0c9
    Boolean warn_padding; // 0x0ca
    Boolean warn_no_side_effect; // 0x0cb
    Boolean warn_resultnotused; // 0x0cc
    UInt8 warn_ptr_int_conv; // 0x0cd
    UInt8 warn_no_typename; // 0x0ce
    UInt8 warn_filenamecaps; // 0x0cf
    UInt8 warn_filenamecaps_system; // 0x0d0
    UInt8 warn_no_explicit_virtual; // 0x0d1
    UInt8 warn_pch_portability; // 0x0d2
    UInt8 warn_uninitializedvar; // 0x0d3
    UInt8 warn_illunionmembers; // 0x0d4
    UInt8 warn_undefmacro; // 0x0d5
    char structalignment; // 0x0d6
    UInt8 alignarraymembers; // 0x0d7
    Boolean dont_reuse_strings; // 0x0d8
    UInt8 poolstrings; // 0x0d9
    UInt8 explicit_zero_data; // 0x0da
    UInt8 readonly_strings; // 0x0db
    UInt8 exceptions; // 0x0dc
    Boolean dddummy; // 0x0dd
    UInt8 dontinline; // 0x0de
    Boolean alwaysinline; // 0x0df
    UInt8 peephole; // 0x0e0
    UInt8 globaloptimizer; // 0x0e1
    UInt8 sideeffects; // 0x0e2
    UInt8 cfm_internal; // 0x0e3
    UInt8 cfm_import; // 0x0e4
    UInt8 cfm_export; // 0x0e5
    UInt8 cfm_lib_export; // 0x0e6
    char nosyminline; // 0x0e7
    UInt8 force_active; // 0x0e8
    unsigned char block_assign; // 0x0e9
    UInt8 has8bytebitfields; // 0x0ea
    SInt8 optimizationlevel; // 0x0eb
    UInt8 optimizesize; // 0x0ec
    UInt8 optimizewithasm; // 0x0ed
    UInt8 crippled; // 0x0ee
    UInt8 commonsubs; // 0x0ef
    UInt8 loopinvariants; // 0x0f0
    UInt8 propagation; // 0x0f1
    UInt8 deadstore; // 0x0f2
    UInt8 strengthreduction; // 0x0f3
    UInt8 strengthreductionstrict; // 0x0f4
    UInt8 deadcode; // 0x0f5
    UInt8 unknownF6; // 0x0f6
    UInt8 lifetimes; // 0x0f7
    UInt8 unrolling; // 0x0f8
    Boolean vectorizeloops; // 0x0f9
    UInt8 multiplepasses; // 0x0fa
    Boolean opt_pointer_analysis; // 0x0fb
    UInt8 opt_pointer_analysis_mode; // 0x0fc
    SInt8 unrollfactor; // 0x0fd
    SInt8 unrollinstrfactor; // 0x0fe
    UInt8 unknownFF; // 0x0ff
    UInt8 opt_scalarize; // 0x100
    UInt8 opt_prelinearize; // 0x101
    UInt8 opt_treetransformation; // 0x102
    UInt8 opt_arithtransformation; // 0x103
    UInt8 opt_rebuildconditionals; // 0x104
    UInt8 opt_rebuildlogicals; // 0x105
    UInt8 opt_generateconditionalassignments; // 0x106
    UInt8 opt_removeemptyloops; // 0x107
    UInt8 opt_alwaysdumpopts; // 0x108
    UInt8 opt_rotateloops; // 0x109
    UInt8 opt_optimizenonregaccess; // 0x10a
    UInt8 opt_movepostops; // 0x10b
    UInt8 opt_findoptimalunrollfactor; // 0x10c
    UInt8 opt_findoptimalvectorunrollfactor; // 0x10d
    UInt8 opt_unrollpostloop; // 0x10e
    UInt8 opt_unrollvectorpostloop; // 0x10f
    UInt8 opt_createvectortemps; // 0x110
    UInt8 opt_unpromotetypes; // 0x111
    UInt8 opt_dumptypes; // 0x112
    UInt8 opt_scalarizebitfields; // 0x113
    UInt8 opt_propagatemultiplydivideassignments; // 0x114
    UInt8 opt_moveinvariantsinaddressexpr; // 0x115
    UInt8 opt_convertfpconsts; // 0x116
    UInt8 opt_markcounterloops; // 0x117
    UInt8 opt_markvariablecounterloops; // 0x118
    UInt8 opt_pulloutconstants; // 0x119
    UInt8 opt_defineargorder; // 0x11a
    UInt8 unknown11B; // 0x11b
    UInt8 unknown11C; // 0x11c
    UInt8 unknown11D; // 0x11d
    UInt8 unknown11E; // 0x11e
    UInt8 unknown11F; // 0x11f
    UInt8 unknown120; // 0x120
    UInt8 unknown121; // 0x121
    UInt8 unknown122; // 0x122
    UInt8 unknown123; // 0x123
    UInt8 unknown124; // 0x124
    UInt8 unknown125; // 0x125
    UInt8 unknown126; // 0x126
    UInt8 unknown127; // 0x127
    UInt8 unknown128; // 0x128
    UInt8 unknown129; // 0x129
    UInt8 unknown12A; // 0x12a
    UInt8 unknown12B; // 0x12b
    UInt8 unknown12C; // 0x12c
    UInt8 unknown12D; // 0x12d
    UInt8 opt_branchlessexpensivethandiv; // 0x12e
    UInt8 unknown12F; // 0x12f
    UInt8 unknown130; // 0x130
    UInt8 unknown131; // 0x131
    Boolean unknown132; // 0x132
    Boolean unknown133; // 0x133
    UInt8 unknown134; // 0x134
    UInt8 unknown135; // 0x135
    UInt8 unknown136; // 0x136
    UInt8 unknown137; // 0x137
    UInt8 unknown138; // 0x138
    UInt8 unknown139; // 0x139
    UInt8 unknown13A; // 0x13a
    UInt8 unknown13B; // 0x13b
    UInt32 appltype; // 0x13c
    UInt32 headtype; // 0x140
    UInt32 texttype; // 0x144
    struct HashNameNode* cursegmentname; // 0x148
};
#pragma pack(pop)

struct CMidFunctionEntry {
    CMidFunctionEntry* next;
    void* function;
    void* ctorData;
    void* expression;
};

struct CMidNonGlobalClassEntry {
    CMidNonGlobalClassEntry* next;
    void* value;
};

extern CMidFunctionEntry* cmid_function_entries;
extern CMidNonGlobalClassEntry* cmid_non_global_classes;
extern void CMid_RegisterDummyCtorFunction(void* function, void* data);
extern void CMid_RegisterSingleExprFunction(void* function, void* expression);
extern void CMid_RegisterNonGlobalClass(void* value);

extern Boolean anyerrors;
extern short iscpp_typeequal(Type* a, Type* b);
extern Object* newh_func;
extern CompilerLinkerOptions copts;

struct ParserTryBlock {
    ParserTryBlock* next;
    jmp_buf jmpbuf;
};

extern Boolean CParser_IsVolatile(Type* type, UInt32 qual);
extern Boolean CParser_IsDataObject(Object* obj);
extern Boolean IsTempName(HashNameNode* name);
extern Boolean is_volatile_object(Object* obj);
extern Boolean is_const_object(Object* obj);
extern NameSpaceName* newp_fobj;
extern NameSpaceName* newa_fobj;
extern NameSpaceName* delp_fobj;
extern NameSpaceName* dela_fobj;
extern Boolean CParserIsConstExpr(ENode* expr);
extern Boolean CParserIsVolatileExpr(ENode* expr);
extern SInt32 CParser_GetUniqueID(void);
extern FuncArg* CParser_NewFuncArg(void);
extern SInt16 tk;
extern SInt16 lex(void);
extern HashNameNode* this_name_node;
extern Object* CParser_NewLocalDataObject(DeclInfo* declinfo, Boolean add_to_locals);
extern HashNameNode* CParser_GetUniqueName(void);
extern struct StructMember* ismember(struct TypeStruct* tstruct, HashNameNode* name);
extern short is_typesame(Type* a, Type* b);
extern Object* darr_func;
extern jmp_buf errorreturn;
extern Boolean cprep_nomem_exit;
extern Type stvoid;
extern SInt32 uniqueid;
extern Boolean CParser_TryParamList(SInt32 arg);

extern ParserTryBlock* trychain;

extern Boolean in_func_arglist;
extern Boolean dont_set_references;
extern Boolean cparser_unknown_setup_flag;
extern struct ClassAction* cparser_classactions;
extern struct CallbackAction* callbackactions;
extern struct ParentCleanup* cparser_parentcleanup;
extern FileOffsetInfo cparser_fileoffset;
extern TypeStruct catchinfostruct;
extern TypeStruct ptmstruct;
extern HashNameNode* uniquenamespacename;
extern Boolean unique_name_file_valid;

extern short is_typeequal(Type* a, Type* b);
extern void CParser_SetCFMFlags(Object* object, DeclInfo* declinfo);
extern void CParser_UpdateObject(Object* object, DeclInfo* declinfo);
extern Object* CParser_NewCompilerDefFunctionObject(void);
extern Object* CParser_NewCompilerDefDataObject(void);
extern Object* CParser_NewFunctionObject(DeclInfo* declinfo);
extern Object* CParser_NewGlobalDataObject(DeclInfo* declinfo);
extern Object* CParser_NewObject(DeclInfo* declinfo);
extern Boolean islookaheaddeclaration(void);
extern Boolean isdeclaration(
    UInt8 scanDeclarator, UInt8 requireName, UInt8 isNewExpression, short token
);
extern Boolean is_unsigned(Type* type);
extern Boolean CParser_HasInternalLinkage2(Object* object);
extern Boolean CParser_HasInternalLinkage(Object* object);
extern void CParser_Cleanup(void);
extern UInt32 CParser_GetTypeQualifiers(Type* type, UInt32 qual);
extern UInt32 CParser_GetCVTypeQualifiers(Type* type, UInt32 qual);
extern Type* CParser_GetWCharType(void);
extern Boolean CParser_IsPublicRuntimeObject(Object* object);
extern Object* CParser_NewAliasObject(Object* object, SInt32 offset);
extern Boolean CParser_IsReadOnly(Type* type, UInt32 qual);
extern void appendmember(TypeStruct* tstruct, StructMember* member);
extern Type* CParser_RemoveTopMostQualifiers(Type* type, UInt32* qual);
extern void TypedefDeclInfo(DeclInfo* declinfo, Type* type, UInt32 qual);
extern SInt16 GetPrec(SInt16 token);
extern SInt16 is_memberpointerequal(Type* a, Type* b);
extern Object* CParser_FindDeallocationObject(
    Type* type, FuncArg* args, Boolean isArray, Boolean isGlobal, Boolean* includeSize
);
extern Object* CParser_NewRTFunc(Type* rettype, HashNameNode* name, UInt8 kind, int argcount, ...);
extern void CParser_Setup(void);
extern Boolean CParser_ParseOperatorName(
    short* token, Boolean allowConversion, Boolean parseConversionType
);
extern Boolean CParser_ReInitRuntimeObjects(Boolean is_precompiler);
extern void CParser_SetupRuntimeObjects(void);
extern HashNameNode* CParser_AppendUniqueNameFile(char* prefix);
extern void CParser_SetUniqueID(SInt32 id);
extern HashNameNode* CParser_AppendUniqueName(char* prefix);
extern Type* CParser_GetBoolType(void);
extern Boolean CParser_IsMoreCVQualified(UInt32 a, UInt32 b);
extern Boolean CParser_IsSameOrMoreCVQualified(UInt32 a, UInt32 b);
extern Boolean CParser_IsConst(Type* type, UInt32 qual);
extern Boolean CParser_IsVirtualFunction(Object* obj, TypeClass** tclass, SInt32* index);
extern Boolean is_pascal_object(Object* obj);
extern Boolean is_arglistsame(FuncArg* a, FuncArg* b);
extern SInt16 CParser_GetOperator(ENodeType t);

extern HashNameNode* no_name_node_635BA0;
extern HashNameNode* self_name_node_636D84;
extern HashNameNode* temp_argument_name_636C9C;
extern HashNameNode* vptr_name_node;
extern Object* runtime_object_635AB8;
extern Object* runtime_object_635B10;
extern Object* runtime_object_635B60;
extern Object* runtime_object_635B80;
extern Object* runtime_object_635B94;
extern Object* runtime_object_635F08;
extern Object* runtime_object_635F0C;
extern Object* runtime_object_635F10;
extern Object* runtime_object_636044;
extern Object* runtime_object_63605C;
extern Object* runtime_object_636170;
extern Object* runtime_object_6361AC;
extern Object* runtime_object_6361C0;
extern Object* runtime_object_63628C;
extern Object* runtime_object_6363E8;
extern Object* runtime_object_63641C;
extern Object* runtime_object_636638;
extern Object* runtime_object_636658;
extern Object* runtime_object_63665C;
extern Object* runtime_object_6366DC;
extern Object* runtime_object_6366E4;
extern Object* runtime_object_636810;
extern Object* runtime_object_636840;
extern Object* runtime_object_636948;
extern Object* runtime_object_636A00;
extern Object* runtime_object_636D2C;
extern Object* runtime_object_636D38;
extern Object* runtime_object_636D78;
extern Object* runtime_object_636D94;
extern Object* runtime_object_636D9C;
extern UInt8 unique_name_file_601770[256];

extern void fn_004483B0(Object* obj);
extern Object* runtime_object_6367B0;

#endif
