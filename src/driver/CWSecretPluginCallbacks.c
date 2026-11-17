#include "driver/CWSecretPluginCallbacks.h"
#include "driver/CWPlugins.h"
#include "driver/CWPluginsPrivate.h"

typedef CW_CALLBACK_PTR (*cbSecretAttachHandleType)(CWPluginContext, Handle, CWMemHandle*);
typedef CW_CALLBACK_PTR (*cbSecretDetachHandleType)(CWPluginContext, CWMemHandle, Handle*);

static Boolean CWSecretPluginCallbacks_ValidateContext(CWPluginContext context);

CW_CALLBACK CWSecretGetNamedPreferences(
    CWPluginContext context, const char* prefsname, Handle* prefsdata
) {
    CWMemHandle memHandle;
    CWResult res;

    if (!prefsdata) {
        return cwErrInvalidParameter;
    }

    res = CWGetNamedPreferences(context, prefsname, &memHandle);
    if (!res) {
        res = CWSecretDetachHandle(context, memHandle, prefsdata);
    }
    return res;
}

CW_CALLBACK CWSecretDetachHandle(CWPluginContext context, CWMemHandle memHandle, Handle* handle) {
    if (!CWSecretPluginCallbacks_ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    return ((cbSecretDetachHandleType)context->callbacks->cbInternal[1])(
        context, memHandle, handle
    );
}

CW_CALLBACK CWSecretAttachHandle(CWPluginContext context, Handle handle, CWMemHandle* memHandle) {
    if (!CWSecretPluginCallbacks_ValidateContext(context)) {
        return cwErrInvalidParameter;
    }
    return ((cbSecretAttachHandleType)context->callbacks->cbInternal[0])(
        context, handle, memHandle
    );
}

static Boolean CWSecretPluginCallbacks_ValidateContext(CWPluginContext context) {
    return context && context->shellSignature == 'CWIE';
}
