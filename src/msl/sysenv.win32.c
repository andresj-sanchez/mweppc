#define _MSL_USE_INLINE 1
#include <string.h>
#include <stdlib.h>
#include "compiler/common.h"
#include "compiler/win32.h"
#include "msl/ctype_api.h"
#include "msl/sysenv_api.h"

char** _tenviron;
static size_t _environ_length;
static size_t _environ_max;
#define ENVIRON_GROW 16

static int __env_tstricmp(const char* s1, const char* s2) {
    char c1, c2;

    while (1) {
        c1 = toupper(*s1++);
        c2 = toupper(*s2++);
        if (c1 != c2)
            break;
        if (!c1 || c1 == '=')
            return 0;
    }
    if (c1 == '=')
        c1 = 0;
    if (c2 == '=')
        c2 = 0;
    if (c1 == c2)
        return 0;
    return c1 - c2;
}

static void init_environ(char* envblock) {
    char* ptr;

    _environ_length = 0;
    _environ_max = _environ_length + ENVIRON_GROW;
    _tenviron = (char**)malloc((_environ_max) * sizeof(char*));
    _tenviron[0] = 0L;

    ptr = envblock;
    while (*ptr) {
        if (*ptr != '=')
            __msl_putenv(ptr);
        ptr += strlen(ptr) + 1;
    }
}

#define CHUNK(val, sz) ((( (val) + (sz) - 1 ) / (sz)) * (sz))

static void resize_environ(size_t length) {
    size_t oldmax = _environ_max;

    _environ_max = CHUNK(length + 1, ENVIRON_GROW);
    if (_environ_max != oldmax) {
        _tenviron = realloc(_tenviron, sizeof(char*) * _environ_max);
    }
    _environ_length = length;
}

void _SetupEnv(void) {
    char* theEnvStrings;

    theEnvStrings = GetEnvironmentStrings();

    init_environ(theEnvStrings);

    FreeEnvironmentStringsA(theEnvStrings);
}

static char* search_env(const char* inVarName, int* index) {
    char* ptr;
    int theVarNameLength = strlen(inVarName);
    int Min, Max;
    int StrInx = 0;
    int cmp = -1;

    Min = 0;
    Max = _environ_length - 1;
    while (Min <= Max) {
        StrInx = (Min + Max) >> 1;
        ptr = _tenviron[StrInx];

        cmp = __env_tstricmp(inVarName, ptr);
        if (cmp == 0) {
            *index = StrInx;
            return ptr;
        } else if (cmp < 0) {
            Max = StrInx - 1;
        } else {
            Min = StrInx + 1;
        }
    }
    *index = cmp < 0 ? StrInx : StrInx + 1;
    return 0L;
}

char* getenv(const char* inVarName) {
    char* ptr;
    int StrInx = 0;
    int theVarNameLength = strlen(inVarName);

    if ((ptr = search_env(inVarName, &StrInx)))
        return ptr + theVarNameLength + 1;
    else
        return 0L;
}

int __msl_putenv(const char* varSetting) {
    const char* equal_sign = strchr(varSetting, '=');

    if (equal_sign) {
        int var_length = equal_sign - varSetting;
        const char* arg = equal_sign + 1;
        int index;
        int edelta;

        if (search_env(varSetting, &index)) {
            if (!*arg) {
                edelta = -1;
                varSetting = 0L;
            } else {
                edelta = 0;
            }
            free(_tenviron[index]);
        } else if (*arg) {
            edelta = 1;
        } else {
            return 0;
        }

        if (edelta < 0) {
            memmove(&_tenviron[index], &_tenviron[index + 1], (_environ_length - index) * sizeof(char*));
            resize_environ(_environ_length + edelta);
        } else if (edelta > 0) {
            resize_environ(_environ_length + edelta);
            memmove(&_tenviron[index + 1], &_tenviron[index], (_environ_length - index) * sizeof(char*));
        }

        if (varSetting) {
            int len = strlen(varSetting);
            _tenviron[index] = malloc((len + 1) * sizeof(char));
            strcpy(_tenviron[index], varSetting);
        }

        return 0;
    }

    return -1;
}
