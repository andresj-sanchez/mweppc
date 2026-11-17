#ifndef COMPILER_ENODE_H
#define COMPILER_ENODE_H

#include "compiler/common.h"
#include "compiler/CInt64.h"

typedef enum ENodeOp {
    EPOSTINC = 0x00,
    EPOSTDEC,
    EPREINC,
    EPREDEC,
    EINDIRECT,
    EMONMIN,
    EBINNOT,
    ELOGNOT,
    EFORCELOAD,
    EMUL,
    EMULV,
    EDIV,
    EMODULO,
    EADDV,
    ESUBV,
    EADD,
    ESUB,
    ESHL,
    ESHR,
    ELESS,
    EGREATER,
    ELESSEQU,
    EGREATEREQU,
    EEQU,
    ENOTEQU,
    EAND,
    EXOR,
    EOR,
    ELAND,
    ELOR,
    EASS,
    EMULASS,
    EDIVASS,
    EMODASS,
    EADDASS,
    ESUBASS,
    ESHLASS,
    ESHRASS,
    EANDASS,
    EXORASS,
    EORASS,
    ECOMMA,
    EMIN,
    EMAX,
    EPMODULO,
    EROTL,
    EROTR,
    EBCLR,
    EBTST,
    EBSET,
    ETYPCON,
    EBITFIELD,
    EINTCONST,
    EFLOATCONST,
    ESTRINGCONST,
    ECOND,
    EFUNCCALL,
    EFUNCCALLP,
    EOBJREF,
    ENULLCHECK,
    EPRECOMP,
    ELABEL,
    ESCOPEBEGIN,
    ESCOPEEND,
    EMFPOINTER,
    ETEMP,
    ELOCOBJ,
    EARGOBJ,
    ESETCONST,
    ENEWEXCEPTION,
    ENEWEXCEPTIONARRAY,
    EINITTRYCATCH,
    EOBJACCESS,
    ETEMPLDEP,
    EPOINTERSTAR,
    EDOTSTAR,
    ECTORINIT,
    ESTMT,
    EINSTRUCTION,
    EDEFINE,
    EREUSE,
    EASSBLK,
    ECONDASS
} ENodeType;

#define IRO_MAXEXPR 124

union FloatStorage {
    int words[2];
    double value;
};

struct Float {
    double value;
};

typedef enum TemplDepSubType {
    TDE_PARAM = 0,
    TDE_UNK1,
    TDE_UNK2,
    TDE_UNK3,
    TDE_UNK4,
    TDE_UNK5,
    TDE_UNK6,
    TDE_UNK7,
    TDE_UNK8,
    TDE_UNK9,
    TDE_UNK10,
    TDE_UNK11,
    TDE_UNK12,
    TDE_UNK13,
    TDE_UNK14,
    TDE_UNK15,
    TDE_UNK16,
    TDE_UNK17,
    TDE_UNK18,
    TDE_UNK19,
    TDE_UNK20,
    TDE_UNK21,
    TDE_UNK22,
    TDE_UNK23,
    TDE_UNK24,
    TDE_UNK25
} TemplDepSubType;

struct ENodeList {
    struct ENodeList* next;
    ENode* node;
};

typedef struct ObjAccess {
    union {
        ENode* expression;
        NameSpaceObjectList* list;
    };
    union {
        Operand* cachedValue;
        TypeClass* theclass;
    };
    SInt32 unk8;
    SInt32 unkC;
    SInt32 unk10;
    ENode* expr; /* 0x14 */
    UInt8 unk18;
    UInt8 unk19;
    UInt8 unk1A;
    UInt8 unk1B;
    SInt32 unk1C;
} ObjAccess;

struct TemplParamID;

union ENodeUnion {
    CInt64 intval;
    Float floatval;
    ENode* monadic;
    Object* objref;
    Type* functype;
    struct {
        ENode* left;
        ENode* right;
    } diadic;
    struct {
        ENode* funcref;
        ENodeList* args;
        TypeFunc* functype;
    } funccall;
    struct {
        ENode* cond;
        ENode* expr1;
        ENode* expr2;
    } cond;
    struct {
        ENode* nullcheckexpr;
        ENode* condexpr;
        SInt32 precompid;
    } nullcheck;
    SInt32 precompid;
    CLabel* label;
    struct {
        ENode* accessnode;
        ENode* mfpointer;
    } mfpointer;
    struct {
        Type* type;
        SInt32 uniqueid;
        Boolean needs_dtor;
    } temp;
    struct {
        SInt32 size;
        char* data;
        SInt32 unk8;
    } string;
    struct {
        ENode* initexpr;
        ENode* tryexpr;
    } newexception;
    struct {
        ENode* initexpr;
        ENode* tryexpr;
        ENode* catchexpr;
        ENode* result;
    } itc;
    CtorChain* ctorinit;
    ObjAccess objaccess;
    struct {
        union {
            TemplParamID pid;
            ENode* expr;
            ENodeList* exprlist;
            NameSpaceObjectList* nsoList;
        } u0;
        union {
            ENode* expr;
            ENodeList* args;
            TemplDepArg* arg;
            DepName* name;
            struct {
                UInt8 unk4;
                UInt8 flag;
            } b4;
        } u4;
        ENode* expr8;
        union {
            ENodeList* listC;
            TemplArg* templargs;
        } uC;
        ENodeList* list10;
        ENode* expr14;
        SInt32 unk18;
        UInt8 subtype;
    } templdep;
};

#define ENODE_IS(_enode, _etype) ((_enode)->type == (_etype))
#define ENODE_IS2(_enode, _etype1, _etype2) \
    (ENODE_IS(_enode, (_etype1)) || ENODE_IS(_enode, (_etype2)))
#define ENODE_IS3(_enode, _etype1, _etype2, _etype3) \
    (ENODE_IS2(_enode, (_etype1), (_etype2)) || ENODE_IS(_enode, (_etype3)))

#define ENODE_CASE_MONADIC \
    case EPOSTINC: \
    case EPOSTDEC: \
    case EPREINC: \
    case EPREDEC: \
    case EINDIRECT: \
    case EMONMIN: \
    case EBINNOT: \
    case ELOGNOT: \
    case EFORCELOAD: \
    case ETYPCON: \
    case EBITFIELD

#define ENODE_CASE_DIADIC_ALL \
    case EMUL: \
    case EMULV: \
    case EDIV: \
    case EMODULO: \
    case EADDV: \
    case ESUBV: \
    case EADD: \
    case ESUB: \
    case ESHL: \
    case ESHR: \
    case ELESS: \
    case EGREATER: \
    case ELESSEQU: \
    case EGREATEREQU: \
    case EEQU: \
    case ENOTEQU: \
    case EAND: \
    case EXOR: \
    case EOR: \
    case ELAND: \
    case ELOR: \
    case EASS: \
    case EMULASS: \
    case EDIVASS: \
    case EMODASS: \
    case EADDASS: \
    case ESUBASS: \
    case ESHLASS: \
    case ESHRASS: \
    case EANDASS: \
    case EXORASS: \
    case EORASS: \
    case ECOMMA: \
    case EMIN: \
    case EMAX: \
    case EPMODULO: \
    case EROTL: \
    case EROTR: \
    case EBCLR: \
    case EBTST: \
    case EBSET: \
    case EPOINTERSTAR: \
    case EDOTSTAR

struct ENode {
    UInt8 type;
    UInt8 cost;
    UInt16 flags;
    Boolean ignored;
    Boolean hascall;
    SInt32 unk8;
    Type* rtype;
    SInt32 unk10;
    SInt32 unk14;
    SInt32 unk18;
    SInt32 unk1C;
    ENodeUnion data;
};

enum {
    ENODE_FLAG_CONST = Q_CONST,
    ENODE_FLAG_VOLATILE = Q_VOLATILE,
    ENODE_FLAG_QUALS = Q_CONST | Q_VOLATILE,
    ENODE_FLAG_NONVIRTUAL = 0x80,
    ENODE_FLAG_LOCALID = 0x100,
};

#endif
