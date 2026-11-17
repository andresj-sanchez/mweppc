#ifndef MSL_CRTL_H
#define MSL_CRTL_H

typedef struct {
    void* handle;
    char translate;
    char append;
} FileStruct;

#define NUM_HANDLES 256

extern FileStruct* _HandleTable[NUM_HANDLES];
#if MWCCPS2_B198
extern int _HandPtr;
extern int __msl_GetHandle(void);

extern int _CRTStartup(void);
#endif

#endif
