#ifndef COMPILER_IRONONREGLOOPACCESSES_H
#define COMPILER_IRONONREGLOOPACCESSES_H

#include "compiler/common.h"

typedef struct NonRegElement {
    void* linear;
    struct NonRegElement* next;
} NonRegElement;

typedef struct NonRegCandidate {
    UInt8 kind;
    UInt8 pad[3];
    void* linear;
    NonRegElement* elements;
    void* address;
    Object* object;
    struct NonRegCandidate* next;
} NonRegCandidate;

extern Boolean (*IRO_NonRegAccessCallback)(IROLinear*, int);

#endif
