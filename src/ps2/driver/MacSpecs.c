#include "driver/MacSpecs.h"
#include "compiler/win32.h"
#include "driver/Generic.h"
#include "driver/MsDos.h"
#include "driver/oslib.h"
#include "driver/StringUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct DirNode {
    char* name;
    unsigned int dirID;
    union {
        struct DirNode* parent;
        struct VolNode* volume;
    } p;
    struct DirNode* list;
    struct DirNode* next;
} DirNode;

typedef struct VolNode {
    UInt16 vRefNum;
    DirNode root;
    struct VolNode* next;
} VolNode;

static unsigned int lastDirID = 3;
static unsigned int lastVolRef = 2;
static VolNode* vols;
static DirNode** dirIDMap[256];
static unsigned int dirMapRows;
static char stvol[64];
static char stdir[260];
static char stpath[260];
static char stname[256];

static int AddDirMapEntry(DirNode* node);
static VolNode* FindOrAddVolRef(VolNode** list, char* name);
static DirNode* FindDirMapEntry(unsigned int dirID);
static DirNode* FindOrAddDirRef(DirNode* parent, char* name);
static int FindOrAdd(const OSPathSpec* path, unsigned int* vRefNum, unsigned int* dirID);
static void StepVolDir(unsigned int* vRefNum, unsigned int* dirID, DirNode** node);
static int ResolvePath(const OSPathSpec* path, unsigned int* vRefNum, unsigned int* dirID);
static int ResolveVolDir(unsigned int vRefNum, unsigned int dirID, char* vol, char* dir);

int __stdcall OS_GetRsrcOSSpec(const OSSpec* spec, OSSpec* rspec, Boolean create) {
    char dbuffer[260];
    int err;

    OS_PathSpecToString(&spec->path, dbuffer, sizeof(dbuffer));
    err = OS_MakeSpec2(dbuffer, "RESOURCE.FRK", rspec);
    if (err) {
        return err;
    }

    err = OS_Status(rspec);
    if (err) {
        if (create) {
            err = OS_Mkdir(rspec);
            if (err) {
                return err;
            }
            SetFileAttributesA(
                OS_SpecToString(rspec, STSbuf, sizeof(STSbuf)), FILE_ATTRIBUTE_HIDDEN
            );
        } else {
            return err;
        }
        err = OS_MakeSpec2(dbuffer, "RESOURCE.FRK", rspec);
        if (err) {
            return err;
        }
    } else {
        if (OS_IsFile(rspec)) {
            return ERROR_DIRECTORY;
        }
    }

    if ((err = OS_MakeNameSpec(
             OS_NameSpecToString(&spec->name, STSbuf, sizeof(STSbuf)), &rspec->name
         )) != 0) {
        return err;
    }

    return 0;
}

int __stdcall OS_FSSpec_To_OSSpec(const FSSpec* fss, OSSpec* spec) {
    int err;

    err = OS_VolDir_To_OSPathSpec(fss->vRefNum, fss->parID, &spec->path);
    if (err) {
        return err;
    }

    p2cstrcpy(stname, fss->name);
    return OS_MakeNameSpec(stname, &spec->name);
}

int __stdcall OS_VolDir_To_OSPathSpec(SInt16 vRefNum, SInt32 dirID, OSPathSpec* spec) {
    int err;
    int vlen;
    int dlen;

    if (!vRefNum || !dirID) {
        err = OS_GetCWD(spec);
        if (err) {
            return err;
        }
    } else {
        if (!ResolveVolDir(vRefNum, dirID, stvol, stdir)) {
            return ERROR_PATH_NOT_FOUND;
        }

        vlen = strlen(stvol);
        dlen = strlen(stdir);
        if (vlen + dlen < 260) {
            memcpy(spec->s, stvol, vlen);
            memcpy(spec->s + vlen, stdir, dlen + 1);
        }
    }

    return 0;
}

int __stdcall OS_OSSpec_To_FSSpec(const OSSpec* spec, FSSpec* fss) {
    int err;
    SInt16 vRefNum;
    SInt32 dirID;
    char* name;

    err = OS_OSPathSpec_To_VolDir(&spec->path, &vRefNum, &dirID);
    fss->vRefNum = vRefNum;
    fss->parID = dirID;
    if (err) {
        return err;
    }

    name = OS_NameSpecToString(&spec->name, stname, sizeof(fss->name));
    if (!name) {
        return ERROR_BUFFER_OVERFLOW;
    }

    c2pstrcpy(fss->name, stname);
    return 0;
}

int __stdcall OS_OSPathSpec_To_VolDir(const OSPathSpec* spec, SInt16* vRefNum, SInt32* dirID) {
    unsigned int vol;
    unsigned int dir;

    if (ResolvePath(spec, &vol, &dir)) {
        *vRefNum = vol;
        *dirID = dir;
        return 0;
    } else {
        *vRefNum = 0;
        *dirID = 0;
        return ERROR_PATH_NOT_FOUND;
    }
}

static int ResolveVolDir(unsigned int vRefNum, unsigned int dirID, char* vol, char* dir) {
    DirNode* nodes[256];
    DirNode* cur;
    char* name;
    int idx;
    int len;
    char* dptr;

    vRefNum = -vRefNum;
    idx = 0;
    do {
        StepVolDir(&vRefNum, &dirID, &cur);
        if (cur) {
            nodes[idx++] = cur;
        }
    } while (cur);

    if (idx) {
        strcpy(vol, nodes[--idx]->name);
    } else {
        vol[0] = 0;
        dir[0] = 0;
        return FALSE;
    }

    dptr = dir;
    *(dptr++) = OS_PATHSEP;
    while (idx--) {
        name = nodes[idx]->name;
        len = strlen(name);
        if ((dptr - dir) + len + 1 > 260) {
            *dptr = 0;
            return FALSE;
        } else {
            memcpy(dptr, nodes[idx]->name, len);
            dptr += len;
            *(dptr++) = OS_PATHSEP;
        }
    }

    *dptr = 0;
    return TRUE;
}

static int ResolvePath(const OSPathSpec* path, unsigned int* vRefNum, unsigned int* dirID) {
    if (FindOrAdd(path, vRefNum, dirID)) {
        *vRefNum = -*vRefNum;
        return TRUE;
    } else {
        *vRefNum = 0;
        *dirID = 0;
        return FALSE;
    }
}

static void StepVolDir(unsigned int* vRefNum, unsigned int* dirID, DirNode** node) {
    VolNode* step;

    if (*vRefNum == 1) {
        *node = 0;
    } else if (*dirID == 2) {
        step = vols;
        while (step && step->vRefNum != *vRefNum) {
            step = step->next;
        }
        if (step) {
            *node = &step->root;
            *vRefNum = 1;
            *dirID = 1;
        } else {
            *node = 0;
        }
    } else {
        *node = FindDirMapEntry(*dirID);
        if (*node) {
            *dirID = (*node)->p.parent->dirID;
        }
    }
}

static int FindOrAdd(const OSPathSpec* path, unsigned int* vRefNum, unsigned int* dirID) {
    VolNode* vol;
    DirNode* level;
    char pb[260];
    char voln[64];
    char pathbuf[260];
    char* ptr;
    char* st;
    char ch;

    if (!OS_PathSpecToString(path, pathbuf, 260)) {
        return FALSE;
    }
    if (!pathbuf[0]) {
        return FALSE;
    }

    ptr = OS_GetDirPtr(pathbuf);
    strncpy(voln, pathbuf, ptr - pathbuf);
    voln[ptr - pathbuf] = 0;
    strcpy(pb, ptr);

    vol = FindOrAddVolRef(&vols, voln);
    if (!vol) {
        return FALSE;
    }

    *vRefNum = vol->vRefNum;
    level = &vol->root;
    ptr = &pb[1];
    OS_ASSERT(267, *pb == OS_PATHSEP);

    while (*ptr) {
        st = ptr;
        while ((ch = *ptr) && ch != OS_PATHSEP) {
            ++ptr;
        }

        if (ch == OS_PATHSEP) {
            *ptr++ = 0;
        }

        level = FindOrAddDirRef(level, st);
        if (!level) {
            return FALSE;
        }
    }

    *dirID = level->dirID;
    return TRUE;
}

static DirNode* FindOrAddDirRef(DirNode* parent, char* name) {
    int sz;
    DirNode* dn;
    DirNode** list;

    list = &parent->list;
    while (*list) {
        if (OS_EqualPath(name, (*list)->name)) {
            return *list;
        }
        list = &(*list)->next;
    }

    if (lastDirID >= 0x10000) {
        return 0;
    }

    sz = strlen(name) + 1;

    dn = malloc(sizeof(DirNode));
    if (!dn) {
        return 0;
    }
    dn->name = malloc(sz);
    if (!dn->name) {
        return 0;
    }
    strcpy(dn->name, name);

    dn->list = 0;
    dn->next = 0;
    dn->dirID = lastDirID++;

    if (!AddDirMapEntry(dn)) {
        return 0;
    }

    dn->p.parent = parent;
    *list = dn;
    return dn;
}

static DirNode* FindDirMapEntry(unsigned int dirID) {
    unsigned int row;
    unsigned int col;

    row = dirID >> 8;
    col = dirID & 0xFF;
    OS_ASSERT(166, dirID != 2);

    if (row >= dirMapRows) {
        return 0;
    } else {
        return dirIDMap[row][col];
    }
}

static VolNode* FindOrAddVolRef(VolNode** list, char* name) {
    int sz;
    VolNode* vn;

    while (*list) {
        if (OS_EqualPath(name, (*list)->root.name)) {
            return *list;
        }
        list = &(*list)->next;
    }

    if (lastVolRef >= 256) {
        return 0;
    }

    vn = malloc(sizeof(VolNode));
    if (!vn) {
        return 0;
    }

    vn->next = 0;
    vn->vRefNum = lastVolRef++;

    sz = strlen(name) + 1;
    vn->root.name = malloc(sz);
    if (!vn->root.name) {
        return 0;
    }

    strcpy(vn->root.name, name);
    vn->root.dirID = 2;
    vn->root.p.volume = vn;
    vn->root.list = 0;
    vn->root.next = 0;
    *list = vn;
    return vn;
}

static int AddDirMapEntry(DirNode* node) {
    unsigned int row;
    unsigned int col;

    row = node->dirID >> 8;
    col = node->dirID & 0xFF;

    while (row >= dirMapRows) {
        if (row >= 256) {
            fprintf(stderr, "Fatal error:  too many directories referenced, out of memory\n");
            return FALSE;
        }

        dirIDMap[row] = calloc(sizeof(DirNode*), 256);
        if (!dirIDMap[row]) {
            return FALSE;
        }

        dirMapRows++;
    }

    dirIDMap[row][col] = node;
    return TRUE;
}
