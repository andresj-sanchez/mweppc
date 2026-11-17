#if MWCCPS2_B198
#include "driver/cc-eabi-ppc-mw.h"
#include "driver/cc-eabi-ppc.h"
#include "driver/CWPlugins.h"
#include "driver/oslib.h"
#include "driver/libimp-eabi-ppc.h"

void GetStaticTarget(OSType* cpu, OSType* os) {
    *cpu = 'mips';
    *os = 'psx2';
}
#else
#include "driver/cc-eabi-ppc-mw.h"
#include "driver/cc-eabi-ppc.h"
#include "driver/CWPlugins.h"
#include "driver/libimp-eabi-ppc.h"
#include "driver/oslib.h"

void GetStaticTarget(OSType* cpu, OSType* os) {
#if MWCCPS2_B52
    *cpu = 'mips';
    *os = 'psx2';
#else
    *cpu = 'ePPC';
    *os = 'EABI';
#endif
}
#endif

void GetStaticPluginType(OSType* language, OSType* plugintype) {
    *language = Lang_Any;
    *plugintype = CWDROPINCOMPILERTYPE;
}

void GetStaticParserPluginType(OSType* style) {
    *style = 'Seep';
}
#if !MWCCPS2_B198

int RegisterStaticTargetPlugins(void) {
    return RegisterStaticCompilerPlugin() && RegisterStaticLibImporterPlugin();
}

int RegisterStaticTargetResources(void) {
    return RegisterCompilerResources() && RegisterLibImporterResources();
}
#endif
