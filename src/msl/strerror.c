#include <string.h>
#include <stdio.h>
#include "msl/strerror.h"

char strerror_message_7[] __attribute__((aligned(4))) = "Argument list too long";
char strerror_message_13[] __attribute__((aligned(4))) = "Permission denied";
char strerror_message_11[] __attribute__((aligned(4))) = "Resource temporarily unavailable";
char strerror_message_9[] __attribute__((aligned(4))) = "Bad file descriptor";
char strerror_message_16[] __attribute__((aligned(4))) = "Device busy";
char strerror_message_10[] __attribute__((aligned(4))) = "No child processes";
char strerror_message_35[] __attribute__((aligned(4))) = "Resource deadlock avoided";
char strerror_message_33[] __attribute__((aligned(4))) = "Numerical argument out of domain";
char strerror_message_17[] __attribute__((aligned(4))) = "File exists";
char strerror_message_14[] __attribute__((aligned(4))) = "Bad address";
char strerror_message_27[] __attribute__((aligned(4))) = "File too large";
char strerror_message_40[] __attribute__((aligned(4))) = "File Position Error";
char strerror_message_88[] __attribute__((aligned(4))) = "Wide character encoding error";
char strerror_message_4[] __attribute__((aligned(4))) = "Interrupted system call";
char strerror_message_22[] __attribute__((aligned(4))) = "Invalid argument";
char strerror_message_5[] __attribute__((aligned(4))) = "Input/output error";
char strerror_message_21[] __attribute__((aligned(4))) = "Is a directory";
char strerror_message_24[] __attribute__((aligned(4))) = "Too many open files";
char strerror_message_31[] __attribute__((aligned(4))) = "Too many links";
char strerror_message_36[] __attribute__((aligned(4))) = "File name too long";
char strerror_message_23[] __attribute__((aligned(4))) = "Too many open files in system";
char strerror_message_19[] __attribute__((aligned(4))) = "Operation not supported by device";
char strerror_message_2[] __attribute__((aligned(4))) = "No such file or directory";
char strerror_message_0[] __attribute__((aligned(4))) = "No error detected";
char strerror_message_8[] __attribute__((aligned(4))) = "Exec format error";
char strerror_message_77[] __attribute__((aligned(4))) = "No locks available";
char strerror_message_12[] __attribute__((aligned(4))) = "Cannot allocate memory";
char strerror_message_28[] __attribute__((aligned(4))) = "No space left on device";
char strerror_message_38[] __attribute__((aligned(4))) = "Function not implemented";
char strerror_message_20[] __attribute__((aligned(4))) = "Not a directory";
char strerror_message_39[] __attribute__((aligned(4))) = "Directory not empty";
char strerror_message_25[] __attribute__((aligned(4))) = "Inappropriate ioctl for device";
char strerror_message_6[] __attribute__((aligned(4))) = "Device not configured";
char strerror_message_1[] __attribute__((aligned(4))) = "Operation not permitted";
char strerror_message_32[] __attribute__((aligned(4))) = "Broken pipe";
char strerror_message_34[] __attribute__((aligned(4))) = "Result too large";
char strerror_message_30[] __attribute__((aligned(4))) = "Read-only file system";
char strerror_message_26[] __attribute__((aligned(4))) = "Signal error";
char strerror_message_29[] __attribute__((aligned(4))) = "Illegal seek";
char strerror_message_3[] __attribute__((aligned(4))) = "No such process";
char strerror_message_99[] __attribute__((aligned(4))) = "Unknown error";
char strerror_message_18[] __attribute__((aligned(4))) = "Cross-device link";

char strerror_unknown_format[] __attribute__((aligned(4))) = "Unknown Error (%d)";

char __strerror_buffer[256];

char* strerror(int errnum) {
    return __strerror(errnum, __strerror_buffer);
}

char* __strerror(int errnum, char* buffer) {
    switch (errnum) {
    case 7:
        strcpy(buffer, strerror_message_7);
        break;
    case 13:
        strcpy(buffer, strerror_message_13);
        break;
    case 11:
        strcpy(buffer, strerror_message_11);
        break;
    case 9:
        strcpy(buffer, strerror_message_9);
        break;
    case 16:
        strcpy(buffer, strerror_message_16);
        break;
    case 10:
        strcpy(buffer, strerror_message_10);
        break;
    case 35:
        strcpy(buffer, strerror_message_35);
        break;
    case 33:
        strcpy(buffer, strerror_message_33);
        break;
    case 17:
        strcpy(buffer, strerror_message_17);
        break;
    case 14:
        strcpy(buffer, strerror_message_14);
        break;
    case 27:
        strcpy(buffer, strerror_message_27);
        break;
    case 40:
        strcpy(buffer, strerror_message_40);
        break;
    case 88:
        strcpy(buffer, strerror_message_88);
        break;
    case 4:
        strcpy(buffer, strerror_message_4);
        break;
    case 22:
        strcpy(buffer, strerror_message_22);
        break;
    case 5:
        strcpy(buffer, strerror_message_5);
        break;
    case 21:
        strcpy(buffer, strerror_message_21);
        break;
    case 24:
        strcpy(buffer, strerror_message_24);
        break;
    case 31:
        strcpy(buffer, strerror_message_31);
        break;
    case 36:
        strcpy(buffer, strerror_message_36);
        break;
    case 23:
        strcpy(buffer, strerror_message_23);
        break;
    case 19:
        strcpy(buffer, strerror_message_19);
        break;
    case 2:
        strcpy(buffer, strerror_message_2);
        break;
    case 0:
        strcpy(buffer, strerror_message_0);
        break;
    case 8:
        strcpy(buffer, strerror_message_8);
        break;
    case 77:
        strcpy(buffer, strerror_message_77);
        break;
    case 12:
        strcpy(buffer, strerror_message_12);
        break;
    case 28:
        strcpy(buffer, strerror_message_28);
        break;
    case 38:
        strcpy(buffer, strerror_message_38);
        break;
    case 20:
        strcpy(buffer, strerror_message_20);
        break;
    case 39:
        strcpy(buffer, strerror_message_39);
        break;
    case 25:
        strcpy(buffer, strerror_message_25);
        break;
    case 6:
        strcpy(buffer, strerror_message_6);
        break;
    case 1:
        strcpy(buffer, strerror_message_1);
        break;
    case 32:
        strcpy(buffer, strerror_message_32);
        break;
    case 34:
        strcpy(buffer, strerror_message_34);
        break;
    case 30:
        strcpy(buffer, strerror_message_30);
        break;
    case 26:
        strcpy(buffer, strerror_message_26);
        break;
    case 29:
        strcpy(buffer, strerror_message_29);
        break;
    case 3:
        strcpy(buffer, strerror_message_3);
        break;
    case 99:
        strcpy(buffer, strerror_message_99);
        break;
    case 18:
        strcpy(buffer, strerror_message_18);
        break;
    default:
        sprintf(buffer, strerror_unknown_format, errnum);
        break;
    }
    return buffer;
}
