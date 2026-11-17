#include "driver/DropInCompilerLinkerContext.h"
#include "driver/CLPluginAPI.h"
#include <stdlib.h>
#include <string.h>

#define GetContext DropInCompilerLinkerPrivate_GetContext

extern "C" {

static CWCompilerLinkerContext* GetContext(CWPluginContext context) {
    if (context &&
        (context->pluginType == CWDROPINCOMPILERTYPE ||
         context->pluginType == CWDROPINLINKERTYPE)) {
        return static_cast<CWCompilerLinkerContext*>(context);
    } else {
        return 0;
    }
}

CW_CALLBACK CWIsPrecompiling(CWPluginContext context, Boolean* isPrecompiling) {
    CWCompilerLinkerContext* cl;
    if (!isPrecompiling) {
        return cwErrInvalidParameter;
    }
    if (!(cl = GetContext(context))) {
        return cwErrInvalidCallback;
    }
    *isPrecompiling = cl->precompile;
    return cwNoErr;
}

CW_CALLBACK CWIsAutoPrecompiling(CWPluginContext context, Boolean* isAutoPrecompiling) {
    CWCompilerLinkerContext* cl;
    if (!isAutoPrecompiling) {
        return cwErrInvalidParameter;
    }
    if (!(cl = GetContext(context))) {
        return cwErrInvalidCallback;
    }
    *isAutoPrecompiling = cl->autoprecompile;
    return cwNoErr;
}

CW_CALLBACK CWIsPreprocessing(CWPluginContext context, Boolean* isPreprocessing) {
    CWCompilerLinkerContext* cl;
    if (!isPreprocessing) {
        return cwErrInvalidParameter;
    }
    if (!(cl = GetContext(context))) {
        return cwErrInvalidCallback;
    }
    *isPreprocessing = cl->preprocess;
    return cwNoErr;
}

CW_CALLBACK CWIsGeneratingDebugInfo(CWPluginContext context, Boolean* isGenerating) {
    CWCompilerLinkerContext* cl;
    if (!isGenerating) {
        return cwErrInvalidParameter;
    }
    if (!(cl = GetContext(context))) {
        return cwErrInvalidCallback;
    }
    *isGenerating = cl->debuginfo;
    return cwNoErr;
}

CW_CALLBACK CWIsCachingPrecompiledHeaders(CWPluginContext context, Boolean* isCaching) {
    CWCompilerLinkerContext* cl;
    if (!isCaching) {
        return cwErrInvalidParameter;
    }
    if (!(cl = GetContext(context))) {
        return cwErrInvalidCallback;
    }
    *isCaching = cl->cachingPCH;
    return cwNoErr;
}

CW_CALLBACK CWGetBrowseOptions(CWPluginContext context, CWBrowseOptions* browseOptions) {
    CWCompilerLinkerContext* cl;
    if (!browseOptions) {
        return cwErrInvalidParameter;
    }
    if (!(cl = GetContext(context))) {
        return cwErrInvalidCallback;
    }
    *browseOptions = cl->browseoptions;
    return cwNoErr;
}
}

extern "C" {

CW_CALLBACK CWGetMainFileNumber(CWPluginContext context, SInt32* fileNumber) {
    CWCompilerLinkerContext* cl;
    if (!fileNumber) {
        return cwErrInvalidParameter;
    }
    if (!(cl = GetContext(context))) {
        return cwErrInvalidCallback;
    }
    *fileNumber = cl->whichfile;
    return cwNoErr;
}

CW_CALLBACK CWGetMainFileID(CWPluginContext context, short* fileID) {
    CWCompilerLinkerContext* cl;
    if (!fileID) {
        return cwErrInvalidParameter;
    }
    if (!(cl = GetContext(context))) {
        return cwErrInvalidCallback;
    }
    *fileID = cl->fileID;
    return cwNoErr;
}

CW_CALLBACK CWGetMainFileSpec(CWPluginContext context, CWFileSpec* fileSpec) {
    CWCompilerLinkerContext* cl;
    if (!fileSpec) {
        return cwErrInvalidParameter;
    }
    if (!(cl = GetContext(context))) {
        return cwErrInvalidCallback;
    }
    ConvertFSSpecToNativePath(cl, &cl->sourcefile, fileSpec);
    return cwNoErr;
}

CW_CALLBACK CWGetMainFileText(CWPluginContext context, const char** text, SInt32* textLength) {
    CWCompilerLinkerContext* cl;
    if (!text) {
        return cwErrInvalidParameter;
    }
    if (!textLength) {
        return cwErrInvalidParameter;
    }
    if (!(cl = GetContext(context))) {
        return cwErrInvalidCallback;
    }
    *text = cl->sourcetext;
    *textLength = cl->sourcetextsize;
    return cwNoErr;
}

CW_CALLBACK CWDisplayLines(CWPluginContext context, SInt32 nlines) {
    CWCompilerLinkerContext* cl;
    if (!(cl = GetContext(context))) {
        return cwErrInvalidCallback;
    }
    return cl->callbacks->cbDisplayLines(cl, nlines);
}

CW_CALLBACK CWGetPrecompiledHeaderSpec(
    CWPluginContext context, CWFileSpec* pchspec, const char* target
) {
    CWCompilerLinkerContext* cl;
    OSSpec spec;
    CWResult result;

    if (!pchspec) {
        return cwErrInvalidParameter;
    }
    if (!(cl = GetContext(context))) {
        return cwErrInvalidCallback;
    }
    ConvertNativePathToFSSpec(cl, pchspec, &spec);
    result = cl->callbacks->cbGetPrecompiledHeaderSpec(cl, &spec, target);
    ConvertFSSpecToNativePath(cl, &spec, pchspec);
    return result;
}
}
