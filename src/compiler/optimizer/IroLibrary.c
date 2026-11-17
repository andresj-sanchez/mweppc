#include "compiler/common.h"
#include <string.h>
#include "compiler/IroLibrary.h"
#include "msl/string.h"

Boolean IRO_ErrnoAccountedFor(void) {
    return IRO_errnoFlag && strlen(IRO_errnoString) != 0;
}
