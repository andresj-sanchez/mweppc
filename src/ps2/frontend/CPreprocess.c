#include "compiler/CBrowse.h"
#include "compiler/common.h"
#include "compiler/CompilerTools.h"
#include "compiler/CMangler.h"
#include "compiler/CPrep.h"
#include "compiler/CPreprocess.h"
#include <stdio.h>
#include <string.h>
#include "compiler/CParser.h"
#include "driver/COSToolsCLT.h"

void CPrep_PreprocessDumpNewLine(void) {
    if (copts.line_prepdump && pplist.data && filesp >= 0) {
        AppendGListData(&pplist, unk5E95F7, 2);
    }
}

void CPrep_PreprocessDumpFileInfo(Boolean newLine) {
    char buf[512];
    SInt32 len;
    char* name;

    if (pplist.data && filesp >= 0) {
        if (pplist.size > 0 &&
            ((nlflag && newLine) ||
             (*pplist.data)[pplist.size - 1] != unk5E95F7[strlen("\r\n") - 1])) {
            AppendGListName(&pplist, unk5E95F7);
        }
        if (copts.line_prepdump) {
            len = sprintf(buf, CPrep_LineMode_5E9985, unk636D28);
        } else {
            len = sprintf(buf, CPrep_CommentMode_5E9991, unk636D28);
        }
        AppendGListData(&pplist, buf, len);
        if (copts.fullpath_prepdump) {
            if (unk6361DC->filename) {
                AppendGListData(&pplist, unk6361DC->filename, strlen(unk6361DC->filename));
            } else {
                COS_FileGetPathName(buf, &unk6361DC->textfile, NULL);
                AppendGListData(&pplist, buf, strlen(buf));
            }
        } else {
            if (unk6361DC->filename) {
                name = COS_PathGetFileName(unk6361DC->filename);
                AppendGListData(&pplist, name, strlen(name));
            } else {
                COS_FileGetFileSpecInfo(unk6361DC, NULL, NULL, buf, sizeof(buf));
                AppendGListData(&pplist, buf, strlen(buf));
            }
        }
        len = sprintf(buf, CPrep_StackDepth_5E99A0, (int)filesp);
        AppendGListData(&pplist, buf, len);
        if (copts.line_prepdump && newLine) {
            CPrep_PreprocessDumpNewLine();
        }
        nlflag = 1;
    }
}
