#include "compiler/ScanFloatX86.h"
#include "msl/threadlocaldata.h"
#include "msl/strtold.h"

UInt32 ScanFloat(void* input, double* destination, UInt8* status) {
    SInt32 local;
    errno = 0;
    *destination = strtod((const char*)input, (char**)&local);
    *status = errno != 0;
    return local;
}
