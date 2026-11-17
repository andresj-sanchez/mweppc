#ifndef DRIVER_CWPLUGINERRORS_H
#define DRIVER_CWPLUGINERRORS_H

enum {
    cwNoErr,
    cwErrUserCanceled,
    cwErrRequestFailed,
    cwErrInvalidParameter,
    cwErrInvalidCallback,
    cwErrInvalidMPCallback,
    cwErrOSError,
    cwErrOutOfMemory,
    cwErrFileNotFound,
    cwErrUnknownFile,
    cwErrSilent,
    cwErrCantSetAttribute,
    cwErrStringBufferOverflow,
    cwErrDirectoryNotFound,
    cwErrLastCommonError = 512,

    cwErrUnknownSegment,
    cwErrSBMNotFound,
    cwErrObjectFileNotStored,
    cwErrLicenseCheckFailed,
    cwErrFileSpecNotSpecified,
    cwErrFileSpecInvalid,
    cwErrLastCompilerLinkerError = 1024
};

#endif
