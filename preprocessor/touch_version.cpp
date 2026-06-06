// ========================================================================
// $File: //jeffr/granny/preprocessor/touch_version.cpp $
// $DateTime: 2007/07/24 11:51:39 $
// $Change: 15577 $
// $Revision: #2 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "touch_version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static CommandRegistrar RegTouchVersion(eSingleFile_NoMultiRun,
                                        TouchVersion,
                                        "TouchVersion",
                                        "Updates the specified file to the current file format.");

bool TouchVersion(input_file&      InputFile,
                   key_value_pair* KeyValues,
                   granny_int32x   NumKeyValues,
                   granny_memory_arena*)
{
    char const* OutputFile = FindFirstValueForKey(KeyValues, NumKeyValues, "output");
    if (OutputFile == 0)
    {
        fprintf(stderr, "TouchVersion: must specify an output file with \"-output <filename>\"\n");
        return false;
    }

    granny_file* File = InputFile.GrannyFile;
    granny_file_info* Info = GrannyGetFileInfo(File);
    if (Info == 0)
    {
        fprintf(stderr, "TouchVersion: unable to obtain a granny_file_info from the input file\n");
        return false;
    }

    // Write out the converted Info
    granny_int32x FinalSectionCount = File->SectionCount;
    if (FinalSectionCount < GrannyStandardSectionCount)
        FinalSectionCount = GrannyStandardSectionCount;

    granny_file_builder *Builder = GrannyBeginFile(FinalSectionCount,
                                                   GrannyCurrentGRNStandardTag,
                                                   GrannyGRNFileMV_ThisPlatform,
                                                   GrannyGetTemporaryDirectory(),
                                                   "Prefix");
    granny_file_data_tree_writer *Writer =
        GrannyBeginFileDataTreeWriting(GrannyFileInfoType, Info,
                                       GrannyStandardDiscardableSection, 0);
    GrannyPreserveFileSectionFormats(Builder, File);
    GrannyPreserveObjectFileSections(Writer, File);

    GrannyWriteDataTreeToFileBuilder(Writer, Builder);
    GrannyEndFileDataTreeWriting(Writer);

    bool success = GrannyEndFile(Builder, OutputFile);
    if (!success)
    {
        fprintf(stderr, "TouchVersion: failed to write to %s\n", OutputFile);
    }

    return success;
}
