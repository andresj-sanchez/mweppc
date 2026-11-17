#ifndef COMPILER_COMMON_H
#define COMPILER_COMMON_H

typedef unsigned char UInt8;
typedef signed char SInt8;
typedef unsigned short UInt16;
typedef signed short SInt16;
typedef unsigned long UInt32;
typedef signed long SInt32;
typedef unsigned char Boolean;

typedef char* Ptr;
typedef Ptr* Handle;
typedef unsigned char* StringPtr;
typedef const unsigned char* ConstStringPtr;
#ifdef __MWERKS__
typedef short VarArgShort;
#else
typedef int VarArgShort;
#endif

#define TRUE 1
#define FALSE 0
#ifndef NULL
#define NULL 0
#endif

typedef struct Type Type;
typedef struct TypeIntegral TypeIntegral;
typedef struct TypePointer TypePointer;
typedef struct TypeBitfield TypeBitfield;
typedef struct TypeEnum TypeEnum;
typedef struct TypeStruct TypeStruct;
typedef struct TypeMemberPointer TypeMemberPointer;
typedef struct TypeMemberFunc TypeMemberFunc;
typedef struct TypeFunc TypeFunc;
typedef struct ENode ENode;
typedef struct IntrinsicOpRecord IntrinsicOpRecord;
typedef struct CInt64 CInt64;
typedef struct Object Object;
typedef struct PCodeArg PCodeArg;
typedef struct IGNode IGNode;
typedef struct IGEdge IGEdge;
typedef struct HashNameNode HashNameNode;
typedef struct GCCNameInfo GCCNameInfo;
typedef struct GCCAsmOperand GCCAsmOperand;
typedef struct GCCAsmConstraint GCCAsmConstraint;
typedef struct GCCNameEntry GCCNameEntry;
typedef struct GCCValueEntry GCCValueEntry;
typedef struct GCCStaticEntry GCCStaticEntry;
typedef struct GCCStaticInfo GCCStaticInfo;
typedef struct CompilerLinkerOptions CompilerLinkerOptions;
typedef struct ObjectList ObjectList;
typedef struct IROLinear IROLinear;
typedef struct IRONode IRONode;
typedef struct IROExpr IROExpr;
typedef struct IROList IROList;
typedef struct IROListNode IROListNode;
typedef struct OpcodeInfo OpcodeInfo;
typedef struct Operand Operand;
typedef struct MulDivCacheEntry MulDivCacheEntry;
typedef struct CLabel CLabel;
typedef struct PCodeLabel PCodeLabel;
typedef struct PCodeBlock PCodeBlock;
typedef struct CodeGenFunction CodeGenFunction;
typedef struct PCode PCode;
typedef struct SchedulerDGNode SchedulerDGNode;
typedef struct SchedulerDGSuccessor SchedulerDGSuccessor;
typedef struct SchedulerDGNodeList SchedulerDGNodeList;
typedef struct PCLink PCLink;
typedef struct NameSpace NameSpace;
typedef struct NameSpaceName NameSpaceName;
typedef struct NameSpaceObjectList NameSpaceObjectList;
typedef struct NameSpaceList NameSpaceList;
typedef struct TypeClass TypeClass;
typedef struct TypeArray TypeArray;
typedef struct TypeTemplDep TypeTemplDep;
typedef struct TemplArg TemplArg;
typedef struct TemplFuncInstance TemplFuncInstance;
typedef struct TemplDepArg TemplDepArg;
typedef struct DepName DepName;
typedef struct CPrepFileInfo CPrepFileInfo;
typedef struct CtorChain CtorChain;
typedef struct DeclBlock DeclBlock;

typedef struct TemplStack TemplStack;
typedef struct Alias Alias;
typedef struct AliasMember AliasMember;
typedef struct ArgMatch ArgMatch;
typedef struct IAEffect IAEffect;
typedef struct IAMnemonic IAMnemonic;
typedef struct IAEffects IAEffects;
typedef struct BClassList BClassList;
typedef struct BE_SymNode BE_SymNode;
typedef struct Conversion Conversion;
typedef struct BitVector BitVector;
typedef struct BitVectorRange BitVectorRange;
typedef struct CompressedBitRun CompressedBitRun;
typedef struct CompressedBitVector CompressedBitVector;
typedef struct FreeCompressedBitVector FreeCompressedBitVector;
typedef struct BufferUpdate BufferUpdate;
typedef struct TStreamElement TStreamElement;
typedef struct ClassFriend ClassFriend;
typedef struct DeclInfo DeclInfo;
typedef struct ExceptionAction ExceptionAction;
typedef struct ExceptionActionNode ExceptionActionNode;
typedef struct ExceptionActionDAGNode ExceptionActionDAGNode;
typedef struct ExceptionReference ExceptionReference;
typedef struct ExceptionOutputRef ExceptionOutputRef;
typedef struct CGList CGList;
typedef struct CI_SwitchCase CI_SwitchCase;
typedef struct CI_FuncData CI_FuncData;
typedef struct CI_Var CI_Var;
typedef struct BasicInductionVar BasicInductionVar;
typedef struct CNameNode CNameNode;
typedef struct OLinkList OLinkList;
typedef struct ParserTryBlock ParserTryBlock;
typedef struct CMidFunctionEntry CMidFunctionEntry;
typedef struct CMidNonGlobalClassEntry CMidNonGlobalClassEntry;
typedef struct ObjType ObjType;
typedef struct ObjTypeTag ObjTypeTag;
typedef struct ObjMemberVarPath ObjMemberVarPath;
typedef struct CI_Action CI_Action;
typedef struct CompilerLinkerParamBlk CompilerLinkerParamBlk;
typedef struct NameResult NameResult;
typedef struct CScopeSave CScopeSave;
typedef struct Candidate Candidate;
typedef struct ClassLayout ClassLayout;
typedef struct ClassList ClassList;
typedef struct TemplPartialSpec TemplPartialSpec;
typedef struct PropInfo PropInfo;
typedef struct InductionVar InductionVar;
typedef struct UseOrDef UseOrDef;
typedef struct RegUseOrDef RegUseOrDef;
typedef struct ObjectUseDef ObjectUseDef;
typedef struct InstrList InstrList;
typedef struct BlockList BlockList;
typedef struct ComparisonValues ComparisonValues;
typedef struct ConIteratorList ConIteratorList;
typedef struct ConIterator ConIterator;
typedef struct ConversionIterator ConversionIterator;
typedef struct DWInfo DWInfo;
typedef struct DefArgCtorInfo DefArgCtorInfo;
typedef struct DwarfFixup DwarfFixup;
typedef struct DwarfFunctionState DwarfFunctionState;
typedef struct DwarfLocationOperand DwarfLocationOperand;
typedef struct DwarfNode DwarfNode;
typedef struct DwarfPendingRecord DwarfPendingRecord;
typedef struct DwarfRef DwarfRef;
typedef struct DwarfStateList DwarfStateList;
typedef struct DwarfSymbol DwarfSymbol;
typedef struct ENodeList ENodeList;
typedef struct TemplateAction TemplateAction;
typedef struct EMemberInfo EMemberInfo;
typedef struct ExceptSpecList ExceptSpecList;
typedef struct EntryPoint EntryPoint;
typedef struct ExceptionTableHeader ExceptionTableHeader;
typedef struct FileOffsetInfo FileOffsetInfo;
typedef struct Float Float;
typedef struct FuncArg FuncArg;
typedef struct BrowseClassHeader BrowseClassHeader;
typedef struct GList GList;
typedef struct SwitchInfo SwitchInfo;
typedef struct IROAddrRecord IROAddrRecord;
typedef struct IRODef IRODef;
typedef struct IROElmList IROElmList;
typedef struct IROLoop IROLoop;
typedef struct IROLoopInd IROLoopInd;
typedef struct IROUse IROUse;
typedef struct CI_Statement CI_Statement;
typedef struct IARegister IARegister;
typedef struct CI_Switch CI_Switch;
typedef struct InlineXRef InlineXRef;
typedef struct InterruptGenerationRecord InterruptGenerationRecord;
typedef struct InterruptList InterruptList;
typedef struct CScopeNSIterator CScopeNSIterator;
typedef struct Loop Loop;
typedef struct Macro Macro;
typedef struct TokenStack TokenStack;
typedef struct FuncMatchArgs FuncMatchArgs;
typedef struct BigDeclInfo BigDeclInfo;
typedef struct PackedDeclInfo PackedDeclInfo;
typedef struct ObjBase ObjBase;
typedef struct ObjCCategory ObjCCategory;
typedef struct ObjCInfo ObjCInfo;
typedef struct ObjCMethod ObjCMethod;
typedef struct ObjCMethodArg ObjCMethodArg;
typedef struct ObjCMethodList ObjCMethodList;
typedef struct ObjCNamedArg ObjCNamedArg;
typedef struct ObjCProtocol ObjCProtocol;
typedef struct ObjCProtocolList ObjCProtocolList;
typedef struct ObjCSelector ObjCSelector;
typedef struct ObjEnumConst ObjEnumConst;
typedef struct ObjGenRelocation ObjGenRelocation;
typedef struct ObjGenCodeRelocation ObjGenCodeRelocation;
typedef struct ObjGenScopeEntry ObjGenScopeEntry;
typedef struct ObjGenRegisterLocation ObjGenRegisterLocation;
typedef struct ObjGenSection ObjGenSection;
typedef struct ObjGenSymbolLink ObjGenSymbolLink;
typedef struct ObjMemberVar ObjMemberVar;
typedef struct ObjNameSpace ObjNameSpace;
typedef struct InitExpr InitExpr;
typedef struct PendingObject PendingObject;
typedef struct TemplParam TemplParam;
typedef struct TemplateFriend TemplateFriend;
typedef struct TemplateMember TemplateMember;
typedef struct TokenStream TokenStream;
typedef struct QualNode QualNode;
typedef struct IROAssign IROAssign;
typedef struct SOMInfo SOMInfo;
typedef struct NameSpaceLookupList NameSpaceLookupList;
typedef struct CScopeObjectIterator CScopeObjectIterator;
typedef struct SectionAttributeNode SectionAttributeNode;
typedef struct SectionRec SectionRec;
typedef struct SectionSymbolAttributes SectionSymbolAttributes;
typedef struct Statement Statement;
typedef struct DeclThing DeclThing;
typedef struct StructMember StructMember;
typedef struct SwitchCase SwitchCase;
typedef struct CodeLabelList CodeLabelList;
typedef struct TemplParamID TemplParamID;
typedef struct PSpecList PSpecList;
typedef struct TypeDeduce TypeDeduce;
typedef struct TemplateFunction TemplateFunction;
typedef struct DefAction DefAction;
typedef struct DeduceInfo DeduceInfo;
typedef struct ObjectTemplated ObjectTemplated;
typedef struct TemplateScopeState TemplateScopeState;
typedef struct TemplClass TemplClass;
typedef struct TemplClassInst TemplClassInst;
typedef struct TypeObjCID TypeObjCID;
typedef struct TypeList TypeList;
typedef struct VClassList VClassList;
typedef struct VTable VTable;
typedef struct VarInfo VarInfo;
typedef struct ObjRegInfo ObjRegInfo;
typedef struct RegInfo RegInfo;
typedef struct RegisterMaskRecord RegisterMaskRecord;
typedef struct RegisterDebugShortRecord RegisterDebugShortRecord;
typedef struct RegisterDebugLongRecord RegisterDebugLongRecord;
typedef struct RegisterDebugSymbolRecord RegisterDebugSymbolRecord;
typedef struct VarRecord VarRecord;
typedef struct WeirdOperand WeirdOperand;
typedef union ENodeUnion ENodeUnion;
typedef struct ValueLabel ValueLabel;
typedef struct AvailableValue AvailableValue;
typedef struct RegValue RegValue;
typedef struct State State;
typedef struct StackedValue StackedValue;
typedef union FloatStorage FloatStorage;
typedef union MWVector128 MWVector128;

enum {
    Q_CONST = 1,
    Q_IMPLICIT_WEAK = 0x20000,
    Q_IS_TEMPLATED = 0x400000,
    Q_WEAK = 0x40000,
    Q_VOLATILE = 2,
    Q_ASM = 4,
    Q_PASCAL = 8,
    Q_INLINE = 0x10,
    Q_REFERENCE = 0x20,
    Q_MUTABLE = 0x80,
    Q_MANGLE_NAME = 0x80000
};

typedef enum TemplParamType {
    TPT_TYPE = 0,
    TPT_NONTYPE,
    TPT_TEMPLATE
} TemplParamType;

struct TemplParamID {
    UInt16 index;
    UInt8 nindex;
    UInt8 type;
};

#ifndef CERROR_FILE
#define CERROR_FILE __FILE__
#endif
#define CError_ASSERT(line, cond) \
    do { \
        if (!(cond)) \
            CError_Internal(CERROR_FILE, line); \
    } while (0)
#define CError_FATAL(line) CError_Internal(CERROR_FILE, line)
#define IRO_ASSERT(line, cond) ((void)0)

#define FITS_IN_SHORT(value) ((value) == ((short)(value)))
#define FITS_IN_USHORT(value) ((value) == ((unsigned short)(value)))
#define HIGH_PART(value) ((short)(((value) >> 16) + (((value) >> 15) & 1)))
#define LOW_PART(value) ((short)(value))

typedef enum AccessType {
    ACCESSPUBLIC,
    ACCESSPRIVATE,
    ACCESSPROTECTED,
    ACCESSNONE
} AccessType;

#endif
