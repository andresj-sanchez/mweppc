#include "driver/CWPluginsPrivate.h"
#include "driver/MsDos.h"
#include <string.h>

extern "C" {

void fn_004163E0(CWPluginContext context, IDEMessageRef* to, const CWMessageRef* from) {
    ConvertNativePathToFSSpec(context, &from->sourcefile, &to->sourcefile);
    to->linenumber = from->linenumber;
    to->tokenoffset = from->tokenoffset;
    to->tokenlength = from->tokenlength;
    to->selectionoffset = from->selectionoffset;
    to->selectionlength = from->selectionlength;
}

void fn_00416440(CWPluginContext context, const IDEFileInfo* from, CWFileInfo* to) {
    to->filedata = from->filedata;
    to->filedatalength = from->filedatalength;
    to->filedatatype = from->filedatatype;
    to->fileID = from->fileID;
    ConvertFSSpecToNativePath(context, &from->filespec, &to->filespec);
    to->alreadyincluded = from->alreadyincluded;
    to->recordbrowseinfo = from->recordbrowseinfo;
}

void fn_004164A0(CWPluginContext context, const IDEFileInfo* from, CWMacFileInfo* to) {
    to->filedata = from->filedata;
    to->filedatalength = from->filedatalength;
    to->filedatatype = from->filedatatype;
    to->fileID = from->fileID;
    ConvertFSSpecToNativePath(context, &from->filespec, (CWFileSpec*)&to->filespec);
    to->alreadyincluded = from->alreadyincluded;
    to->recordbrowseinfo = from->recordbrowseinfo;
}

void fn_00416500(CWPluginContext context, const IDEProjectFileInfo* from, CWProjectFileInfo* to) {
    ConvertFSSpecToNativePath(context, &from->filespec, &to->filespec);
    ConvertMacTimeToNativeTime(context, from->moddate, &to->moddate);
    to->segment = from->segment;
    to->hasobjectcode = from->hasobjectcode;
    to->hasresources = from->hasresources;
    to->isresourcefile = from->isresourcefile;
    to->weakimport = from->weakimport;
    to->initbefore = from->initbefore;
    to->gendebug = from->gendebug;
    ConvertMacTimeToNativeTime(context, from->objmoddate, &to->objmoddate);
    OS_NameSpecToString(&from->dropinname, to->dropinname, 260);
    to->fileID = from->fileID;
    to->recordbrowseinfo = from->recordbrowseinfo;
    to->filetype = from->filetype;
    to->filecreator = from->filecreator;
    to->hasunitdata = from->hasunitdata;
    to->mergeintooutput = from->mergeintooutput;
    to->unitdatadependencytag = from->unitdatadependencytag;
}

void fn_00416630(
    CWPluginContext context, const IDEProjectFileInfo* from, CWMacProjectFileInfo* to
) {
    ConvertFSSpecToNativePath(context, &from->filespec, (CWFileSpec*)&to->filespec);
    ConvertMacTimeToNativeTime(context, from->moddate, (CWFileTime*)&to->moddate);
    to->segment = from->segment;
    to->hasobjectcode = from->hasobjectcode;
    to->hasresources = from->hasresources;
    to->isresourcefile = from->isresourcefile;
    to->weakimport = from->weakimport;
    to->initbefore = from->initbefore;
    to->gendebug = from->gendebug;
    ConvertMacTimeToNativeTime(context, from->objmoddate, (CWFileTime*)&to->objmoddate);
    OS_NameSpecToString(&from->dropinname, to->dropinname, 260);
    to->fileID = from->fileID;
    to->recordbrowseinfo = from->recordbrowseinfo;
    to->filetype = from->filetype;
    to->filecreator = from->filecreator;
    to->hasunitdata = from->hasunitdata;
    to->mergeintooutput = from->mergeintooutput;
    to->unitdatadependencytag = from->unitdatadependencytag;
}
}
