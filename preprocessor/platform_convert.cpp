// ========================================================================
// $File: //jeffr/granny/preprocessor/platform_convert.cpp $
// $DateTime: 2007/11/14 18:07:24 $
// $Change: 16557 $
// $Revision: #11 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "platform_convert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static CommandRegistrar RegPlatformConvert(eSingleFile_NoMultiRun,
                                           PlatformConvert,
                                           "PlatformConvert",
                                           "Converts the file to the specified platform.  K/V: -pointer [32|64] -endian [little|big]");

 
GRANNY_CALLBACK(granny_uint32) IdentityMapping(void*, char const* String)
{
    // 64-bit problems here...
    return (granny_uint32)(granny_intaddrx)String;
}

bool PlatformConvert(input_file&     InputFile,
                     key_value_pair* KeyValues,
                     granny_int32x   NumKeyValues,
                     granny_memory_arena*)
{
    char const* OutputFile = FindFirstValueForKey(KeyValues, NumKeyValues, "output");
    if (OutputFile == 0)
    {
        fprintf(stderr, "PlatformConvert: must specify an output file with \"-output <filename>\"\n");
        return false;
    }

    granny_file* File      = InputFile.GrannyFile;
    granny_file_info* Info = GrannyGetFileInfo(InputFile.GrannyFile);
    if (Info == 0)
    {
        fprintf(stderr, "PlatformConvert: unable to obtain a granny_file_info from the input file\n");
        return false;
    }

    // Default to 32le
    bool PreserveStringMapping = false;
    granny_int32x PointerSize = 32;
    bool LittleEndian         = true;

    char const* Preserve = FindFirstValueForKey(KeyValues, NumKeyValues, "remapped");
    if (Preserve)
    {
        if (stricmp(Preserve, "true") == 0)
        {
            PreserveStringMapping = true;
        }
        else if (stricmp(Preserve, "false") != 0)
        {
            fprintf(stderr, "PlatformConvert: key for -remapped invalid, only \"true\"|\"false\" recognized\n");
        }
    }

    char const* PointerValue = FindFirstValueForKey(KeyValues, NumKeyValues, "pointer");
    if (PointerValue != 0)
    {
        PointerSize = atoi(PointerValue);
    }

    char const* EndianValue = FindFirstValueForKey(KeyValues, NumKeyValues, "endian");
    if (EndianValue != 0)
    {
        if (_stricmp(EndianValue, "big") == 0)
            LittleEndian = false;
        else if (_stricmp(EndianValue, "little") == 0)
            LittleEndian = true;
        else
        {
            fprintf(stderr, "PlatformConvert: unable to parse endian key, must be \"big\" or \"little\"\n");
            return false;
        }
    }

    granny_grn_file_magic_value MagicValue;
    if (GrannyGetMagicValueForPlatform(PointerSize, LittleEndian, &MagicValue) == false)
    {
        fprintf(stderr, "PlatformConvert: unable to find platform (%dbit, %s-endian)\n",
                PointerSize, LittleEndian ? "little" : "big");
        return false;
    }

    granny_int32x FinalSectionCount = File->SectionCount;
    if (FinalSectionCount < GrannyStandardSectionCount)
        FinalSectionCount = GrannyStandardSectionCount;

    // Write out the modified Info
    granny_file_builder *Builder =
        GrannyBeginFile(FinalSectionCount,
                        GrannyCurrentGRNStandardTag,
                        &MagicValue,
                        GrannyGetTemporaryDirectory(),
                        "Prefix");

    granny_file_data_tree_writer *Writer =
        GrannyBeginFileDataTreeWriting(GrannyFileInfoType, Info,
                                       GrannyStandardDiscardableSection, 0);
    GrannyPreserveObjectFileSections(Writer, File);
    if (PreserveStringMapping)
    {
        GrannySetFileStringDatabaseCRC(Builder, File->Header->StringDatabaseCRC);
        GrannySetFileWriterStringCallback(Writer, IdentityMapping, 0);
    }

    GrannyWriteDataTreeToFileBuilder(Writer, Builder);
    GrannyEndFileDataTreeWriting(Writer);

    bool success = GrannyEndFile(Builder, OutputFile);
    if (!success)
    {
        fprintf(stderr, "PlatformConvert: failed to write to %s\n", OutputFile);
    }

    return success;
}
