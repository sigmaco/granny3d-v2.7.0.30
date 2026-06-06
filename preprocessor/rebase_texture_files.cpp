// ========================================================================
// $File: //jeffr/granny/preprocessor/rebase_texture_files.cpp $
// $DateTime: 2007/11/26 12:10:16 $
// $Change: 16607 $
// $Revision: #1 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "rebase_texture_files.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


static CommandRegistrar RegRebaseTextureFiles(eSingleFile_MultiRun,
                                              RebaseTextureFiles,
                                              "RebaseTextureFiles",
                                              "Given a -basepath argument, makes any absolute textures relative to that path, if possible");

static char const*
RebasePath(char const* BasePath,
           char const* TexPath)
{
    size_t const BasePathLen = strlen(BasePath);
    if (_strnicmp(BasePath, TexPath, BasePathLen) == 0)
    {
        // Can rebase this one, but account for any path separator at the beginning, since
        // the basepath might not have included it...
        char const* RetPath = TexPath + BasePathLen;
        while (*RetPath == '\\' || *RetPath == '/')
            ++RetPath;

        return RetPath;
    }
    else
    {
        // Path doesn't match, return unchanged...
        return TexPath;
    }
}


bool RebaseTextureFiles(input_file&     InputFile,
                        key_value_pair* KeyValues,
                        granny_int32x   NumKeyValues,
                        granny_memory_arena*)
{
    granny_file* File = InputFile.GrannyFile;
    granny_file_info* Info = GrannyGetFileInfo(File);
    if (Info == 0)
    {
        fprintf(stderr, "RebaseTextureFiles: unable to obtain a granny_file_info from %s\n", InputFile.Filename);
        return false;
    }

    char const* OutputFile = FindFirstValueForKey(KeyValues, NumKeyValues, "output");
    if (OutputFile == 0)
    {
        fprintf(stderr, "RebaseTextureFiles: must specify an output file with \"-output <filename>\"\n");
        return false;
    }

    char const* BasePath = FindFirstValueForKey(KeyValues, NumKeyValues, "basepath");
    if (BasePath == 0)
    {
        fprintf(stderr, "RebaseTextureFiles: must specify an base path for rebasing with \"-basepath path\"\n");
        return false;
    }

    {for(granny_int32x TexIdx = 0; TexIdx < Info->TextureCount; ++TexIdx)
    {
        if (!Info->Textures[TexIdx])
            continue;
        Info->Textures[TexIdx]->FromFileName =
            RebasePath(BasePath, Info->Textures[TexIdx]->FromFileName);
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
        fprintf(stderr, "RemoveElements: failed to write to %s\n", OutputFile);

    return success;
}
