#ifndef COMPILER_SCOPES_H
#define COMPILER_SCOPES_H

#include "compiler/common.h"

struct NameSpace {
    NameSpace* parent;
    HashNameNode* name;
    struct NameSpaceList* usings;
    TypeClass* theclass;
    NameSpaceName* tparams;
    union {
        NameSpaceName** hash;
        NameSpaceName* list;
    } data;
    UInt32 names;
    Boolean is_hash;
    Boolean is_global;
    Boolean is_unnamed;
    Boolean is_templ;
};

struct NameSpaceList {
    NameSpaceList* next;
    NameSpace* nspace;
};

struct NameSpaceObjectList {
    NameSpaceObjectList* next;
    Object* object;
};

struct NameSpaceName {
    NameSpaceName* next;
    HashNameNode* name;
    NameSpaceObjectList first;
};

extern NameSpace* cscope_root;
extern NameSpaceName* CScope_FindNameSpaceName(NameSpace* nspace, HashNameNode* name);
extern NameSpace* cscope_current;

#endif
