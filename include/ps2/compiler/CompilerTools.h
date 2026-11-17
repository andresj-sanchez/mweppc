#ifndef COMPILER_COMPILERTOOLS_H
#define COMPILER_COMPILERTOOLS_H

#include "compiler/common.h"

typedef void (*heaperror_t)(void);

extern short initheaps(heaperror_t heaperrorproc);
extern heaperror_t getheaperror(void);
extern void setheaperror(heaperror_t heaperrorproc);
extern void locklheap(void);
extern void unlocklheap(void);
extern void* aalloc(UInt32 size);
extern SInt16 getbit(UInt32 value);
extern void memclrw(void* buffer, UInt32 size);
extern void* oalloc(UInt32 size);
extern void* lalloc(UInt32 size);
extern void* galloc(UInt32 size);
extern void* IRO_oalloc(UInt32 size);
extern void* IRO_oalloc2(UInt32 size);
extern void IRO_ofree(void* ptr);
extern void freeoheap(void);

struct GList {
    char** data;
    SInt32 size;
    SInt32 hndlsize;
    SInt32 growsize;
};

struct HashNameNode {
    HashNameNode* next;
    SInt32 id;
    SInt16 hashval;
    char name[2];
};

extern HashNameNode** name_hash_nodes;
extern SInt16 CHash(const char* string);
extern short InitGList(GList* gl, SInt32 size);
extern void FreeGList(GList* gl);
extern void LockGList(GList* gl);
extern void UnlockGList(GList* gl);
extern void ShrinkGList(GList* gl);
extern void AppendGListData(GList* gl, const void* data, SInt32 size);
extern void AppendGListNoData(GList* gl, SInt32 size);
extern void AppendGListByte(GList* gl, SInt8 thebyte);
extern void AppendGListWord(GList* gl, SInt16 theword);
extern void AppendGListLong(GList* gl, SInt32 theword);
extern void AppendGListID(GList* gl, const char* name);
extern void AppendGListName(GList* gl, const char* name);

extern void fn_00446B40(CInt64 value, void* memory);

extern void NameHashExportReset(void);

#endif
