#include "driver/CWParserPluginsContext.h"
#include "driver/COSToolsCLT.h"
#include "driver/ELFImportReader.h"

#pragma export on
CWResult __stdcall ELFImport_main(CWPluginContext context) {
    CWFileSpec fileSpec;
    CWObjectData object;
    SInt32 request;
    SInt32 fileNumber;
    CWResult result = 0;

    ELFReader_context_6001FC = context;
    CWGetPluginRequest(context, &request);
    switch (request) {
        case -2:
            CWSetCWPluginAPI(context, 2);
            break;
        case -1:
            break;
        case 0:
            COS_Init(context);
            CWGetMainFileSpec(context, &fileSpec);
            CWGetMainFileNumber(context, &fileNumber);
            fn_00424400(&fileSpec, (unsigned int*)&object);
            if (object.objectdata == NULL) {
                result = 2;
            } else {
                result = CWStoreObjectData(context, fileNumber, &object);
            }
            result = CWDonePluginRequest(context, result);
            break;
    }
    return result;
}
#pragma export reset
