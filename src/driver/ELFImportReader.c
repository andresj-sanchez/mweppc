#include "compiler/common.h"
#include "driver/CWParserPluginsContext.h"
#include "compiler/ELF_Endian.h"
#include <stdio.h>
#include <string.h>
#include "driver/CWPluginsPrivate.h"
#include "driver/ELFImportReader.h"
#include "msl/scanf.h"
#include "driver/ELFImportFileIO.h"

int fn_00423FC0(const unsigned char* data) {
    if (data[0] != 0x7f) {
        return 0;
    }
    if (data[1] != 'E') {
        return 0;
    }
    if (data[2] != 'L') {
        return 0;
    }
    if (data[3] != 'F') {
        return 0;
    }
    ELFReader_endian_6001F8 = data[5];
    return 1;
}

unsigned short fn_00424000(const unsigned char* p) {
    if (ELFReader_endian_6001F8 == 1) {
        return (unsigned short)((p[1] << 8) + p[0]);
    }
    return (unsigned short)((p[0] << 8) + p[1]);
}

unsigned int fn_00424030(const unsigned char* p) {
    if (ELFReader_endian_6001F8 == 1) {
        return ((unsigned int)p[3] << 24) + ((unsigned int)p[2] << 16) + (p[1] << 8) + p[0];
    }
    return ((unsigned int)p[0] << 24) + ((unsigned int)p[1] << 16) + (p[2] << 8) + p[3];
}

void fn_00424080(const Elf32_Ehdr* raw, Elf32_Ehdr* out) {
    int i;
    for (i = 0; i < 16; i++) {
        out->e_ident[i] = raw->e_ident[i];
    }
    out->e_type = fn_00424000((const unsigned char*)&raw->e_type);
    out->e_machine = fn_00424000((const unsigned char*)&raw->e_machine);
    out->e_version = fn_00424030((const unsigned char*)&raw->e_version);
    out->e_entry = fn_00424030((const unsigned char*)&raw->e_entry);
    out->e_phoff = fn_00424030((const unsigned char*)&raw->e_phoff);
    out->e_shoff = fn_00424030((const unsigned char*)&raw->e_shoff);
    out->e_flags = fn_00424030((const unsigned char*)&raw->e_flags);
    out->e_ehsize = fn_00424000((const unsigned char*)&raw->e_ehsize);
    out->e_phentsize = fn_00424000((const unsigned char*)&raw->e_phentsize);
    out->e_phnum = fn_00424000((const unsigned char*)&raw->e_phnum);
    out->e_shentsize = fn_00424000((const unsigned char*)&raw->e_shentsize);
    out->e_shnum = fn_00424000((const unsigned char*)&raw->e_shnum);
    out->e_shstrndx = fn_00424000((const unsigned char*)&raw->e_shstrndx);
}

void fn_004241A0(const Elf32_Shdr* raw, Elf32_Shdr* out) {
    out->sh_name = fn_00424030((const unsigned char*)&raw->sh_name);
    out->sh_type = fn_00424030((const unsigned char*)&raw->sh_type);
    out->sh_flags = fn_00424030((const unsigned char*)&raw->sh_flags);
    out->sh_addr = fn_00424030((const unsigned char*)&raw->sh_addr);
    out->sh_offset = fn_00424030((const unsigned char*)&raw->sh_offset);
    out->sh_size = fn_00424030((const unsigned char*)&raw->sh_size);
    out->sh_link = fn_00424030((const unsigned char*)&raw->sh_link);
    out->sh_info = fn_00424030((const unsigned char*)&raw->sh_info);
    out->sh_addralign = fn_00424030((const unsigned char*)&raw->sh_addralign);
    out->sh_entsize = fn_00424030((const unsigned char*)&raw->sh_entsize);
}

int fn_00424230(
    const unsigned char* p, unsigned int* first, unsigned int* second, unsigned int* third
) {
    Elf32_Ehdr header;
    Elf32_Shdr section;
    unsigned int index;
    unsigned int offset;
    unsigned short count;
    unsigned short stride;

    *first = 0;
    *second = 0;
    *third = 0;
    fn_00424080((const Elf32_Ehdr*)p, &header);
    offset = header.e_shoff;
    stride = header.e_shentsize;
    count = header.e_shnum;
    for (index = 1; index < count; index++) {
        fn_004241A0((const Elf32_Shdr*)(p + offset + stride * index), &section);
        if (section.sh_flags & SHF_ALLOC) {
            if (section.sh_type == SHT_PROGBITS) {
                if (section.sh_flags & SHF_EXECINSTR) {
                    *first += section.sh_size;
                } else {
                    *third += section.sh_size;
                }
            } else if (section.sh_type == SHT_NOBITS) {
                *second += section.sh_size;
            }
        }
    }
    return 1;
}

int fn_004242F0(const char* p) {
    return strncmp(p, ELFReader_archiveMagic_5DFA5E, 8) == 0;
}

int fn_00424310(
    const unsigned char* bytes,
    int size,
    unsigned int* first,
    unsigned int* second,
    unsigned int* third
) {
    int offset = 8;
    long memberSize;
    const unsigned char* member;
    unsigned int a, b, c;

    *first = 0;
    *second = 0;
    *third = 0;
    while (offset < size) {
        member = bytes + offset;
        if (strncmp((const char*)member + 0x3a, ELFReader_archiveEnd_5DFA6A, 2) != 0) {
            return 0;
        }
        sscanf((const char*)member + 0x30, ELFReader_archiveFormat_5DFA6E, &memberSize);
        if (fn_00423FC0(member + 0x3c)) {
            if (!fn_00424230(member + 0x3c, &a, &b, &c)) {
                return 0;
            }
            *first += a;
            *second += b;
            *third += c;
        }
        offset += (memberSize + 0x3d) & ~1;
    }
    return 1;
}

void fn_00424400(const void* fileSpec, unsigned int* result) {
    int error;
    CWMemHandle data;
    SInt32 size;
    void* buffer;
    int ok = 0;
    int i;

    for (i = 0; i < 12; i++) {
        result[i] = ELFReader_defaults_6001C8[i];
    }
    error = fn_00423ED0(fileSpec, &data, &size);
    if (error != 0) {
        fn_00423E90(error);
        return;
    }
    if (size != 0) {
        CWLockMemHandle(ELFReader_context_6001FC, data, FALSE, &buffer);
        if (fn_00423FC0(buffer)) {
            ok = fn_00424230(buffer, result + 3, result + 4, result + 5);
        } else if (fn_004242F0(buffer)) {
            ok = fn_00424310(buffer, size, result + 3, result + 4, result + 5);
            CWResizeMemHandle(ELFReader_context_6001FC, data, 8);
        }
        CWUnlockMemHandle(ELFReader_context_6001FC, data);
    }
    if (!ok) {
        fn_00423E90(4);
        CWFreeMemHandle(ELFReader_context_6001FC, data);
        data = 0;
    }
    result[0] = (unsigned int)data;
}
