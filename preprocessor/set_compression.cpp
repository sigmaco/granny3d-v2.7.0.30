// ========================================================================
// $File: //jeffr/granny/preprocessor/set_compression.cpp $
// $DateTime: 2007/07/24 11:51:39 $
// $Change: 15577 $
// $Revision: #10 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "set_compression.h"
#include <stdio.h>

static CommandRegistrar RegCompress(eSingleFile_NoMultiRun, OodleCompress, "Compress",
                                    "Compresses the input file using Oodle1");
static CommandRegistrar RegDecompress(eSingleFile_NoMultiRun, Decompress, "Decompress",
                                      "Removes any compression on the input file");

bool CompressionDriver(input_file&     InputFile,
                       key_value_pair* KeyValues,
                       granny_int32x   NumKeyValues,
                       granny_compression_type CompressionType)
{
    char const* OutputFile = FindFirstValueForKey(KeyValues, NumKeyValues, "output");
    if (OutputFile == 0)
    {
        fprintf(stderr, "[De]Compress: must specify an output file with \"-output <filename>\"\n");
        return false;
    }

    granny_variant variant;
    GrannyGetDataTreeFromFile(InputFile.GrannyFile, &variant);

    // Write out the converted Info
    granny_int32x FinalSectionCount = InputFile.GrannyFile->SectionCount;
    if (FinalSectionCount < GrannyStandardSectionCount)
        FinalSectionCount = GrannyStandardSectionCount;
    granny_file_builder *Builder = GrannyBeginFile(FinalSectionCount,
                                                   InputFile.GrannyFile->Header->TypeTag,
                                                   InputFile.GrannyFile->SourceMagicValue,
                                                   GrannyGetTemporaryDirectory(),
                                                   "Prefix");
    {for (int SectionIdx = 0; SectionIdx < FinalSectionCount; ++SectionIdx)
    {
        granny_grn_section *SectionArray =
            GrannyGetGRNSectionArray(InputFile.GrannyFile->Header);

        GrannySetFileSectionFormat(Builder,
                                   SectionIdx,
                                   CompressionType,
                                   SectionArray[SectionIdx].InternalAlignment);
    }}

    granny_file_data_tree_writer *Writer =
        GrannyBeginFileDataTreeWriting(variant.Type, variant.Object,
                                       GrannyStandardDiscardableSection, 0);
    GrannyPreserveObjectFileSections(Writer, InputFile.GrannyFile);
    GrannyWriteDataTreeToFileBuilder(Writer, Builder);
    GrannyEndFileDataTreeWriting(Writer);

    bool success = GrannyEndFile(Builder, OutputFile);
    if (!success)
    {
        fprintf(stderr, "CompressionDriver: failed to write to %s\n", OutputFile);
    }

    return success;
}

bool Decompress(input_file&     InputFile,
                key_value_pair* KeyValues,
                granny_int32x   NumKeyValues,
                granny_memory_arena*)
{
    return CompressionDriver(InputFile, KeyValues, NumKeyValues, GrannyNoCompression);
}


bool OodleCompress(input_file&     InputFile,
                   key_value_pair* KeyValues,
                   granny_int32x   NumKeyValues,
                   granny_memory_arena*)
{
    return CompressionDriver(InputFile, KeyValues, NumKeyValues, GrannyOodle1Compression);
}

