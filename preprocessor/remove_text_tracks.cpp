// ========================================================================
// $File: //jeffr/granny/preprocessor/remove_text_tracks.cpp $
// $DateTime: 2007/11/29 14:42:34 $
// $Change: 16656 $
// $Revision: #1 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "remove_text_tracks.h"
#include <stdio.h>

static CommandRegistrar RegRemoveTextTracks(eSingleFile_NoMultiRun,
                                            RemoveTextTracks,
                                            "RemoveTextTracks",
                                            "Removes any text tracks contained in the file");

bool RemoveTextTracks(input_file&     InputFile,
                        key_value_pair* KeyValues,
                        granny_int32x   NumKeyValues,
                        granny_memory_arena*)
{
    char const* OutputFile = FindFirstValueForKey(KeyValues, NumKeyValues, "output");
    if (OutputFile == 0)
    {
        fprintf(stderr, "RemoveTextTracks: must specify an output file with \"-output <filename>\"\n");
        return false;
    }

    granny_file* File = InputFile.GrannyFile;
    granny_file_info* Info = GrannyGetFileInfo(File);
    if (Info == 0)
    {
        fprintf(stderr, "RemoveTextTracks: unable to obtain a granny_file_info from the input file\n");
        return false;
    }

    // Remove all text tracks from each track group
    {for(granny_int32x TGIdx = 0; TGIdx < Info->TrackGroupCount; ++TGIdx)
    {
        granny_track_group* TrackGroup = Info->TrackGroups[TGIdx];
        TrackGroup->TextTrackCount = 0;
        TrackGroup->TextTracks = 0;
    }}

    // Write out the modified Info
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
    GrannyPreserveObjectFileSections(Writer, File);

    GrannyWriteDataTreeToFileBuilder(Writer, Builder);
    GrannyEndFileDataTreeWriting(Writer);

    bool success = GrannyEndFile(Builder, OutputFile);
    if (!success)
    {
        fprintf(stderr, "RemoveTextTracks: failed to write to %s\n", OutputFile);
    }

    return success;
}
