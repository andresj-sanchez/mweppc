#include "compiler/CDecl.h"
#include "compiler/CompilerTools.h"

Type* CDecl_NewStructType(SInt32 size, SInt16 align) {
    TypeStruct* tstruct = galloc(sizeof(TypeStruct));
    memclrw(tstruct, sizeof(TypeStruct));

    tstruct->type = TYPESTRUCT;
    tstruct->size = size;
    tstruct->align = align;
    tstruct->stype = STRUCT_TYPE_STRUCT;

    return (Type*)tstruct;
}

Type* CDecl_NewArrayType(Type* type, SInt32 size) {
    TypeArray* tarray = galloc(sizeof(TypeArray));
    memclrw(tarray, sizeof(TypeArray));

    tarray->type = TYPEARRAY;
    tarray->size = size;
    tarray->target = type;
    tarray->nelimit = type->size ? size / type->size : 0;

    return (Type*)tarray;
}

Type* CDecl_NewPointerType(Type* type) {
    TypePointer* tptr = galloc(sizeof(TypePointer));
    memclrw(tptr, sizeof(TypePointer));

    tptr->type = TYPEPOINTER;
    tptr->size = 4;
    tptr->target = type;

    return (Type*)tptr;
}

Type* CDecl_NewRefPointerType(Type* type) {
    TypePointer* tptr = galloc(sizeof(TypePointer));
    memclrw(tptr, sizeof(TypePointer));

    tptr->type = TYPEPOINTER;
    tptr->size = 4;
    tptr->target = type;
    tptr->qual = Q_REFERENCE;

    return (Type*)tptr;
}

TypeTemplDep* CDecl_NewTemplDepType(TypeTemplDepType tdt) {
    TypeTemplDep* t = galloc(sizeof(TypeTemplDep));
    memclrw(t, sizeof(TypeTemplDep));

    t->type = TYPETEMPLATE;
    t->size = 1;
    t->dtype = tdt;

    return t;
}
