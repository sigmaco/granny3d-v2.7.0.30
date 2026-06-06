// ========================================================================
// $File: //jeffr/granny/preprocessor/mesh_indices_16bit.cpp $
// $DateTime: 2007/07/24 11:51:39 $
// $Change: 15577 $
// $Revision: #7 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "mesh_indices_16bit.h"
#include <stdio.h>

static CommandRegistrar RegMeshIndices16Bit(eSingleFile_NoMultiRun,
                                            MeshIndices16Bit,
                                            "MeshIndices16Bit",
                                            "Converts Meshes to 16-bit indices where possible (i.e, vert counts of less than 65536)");


bool MeshIndices16Bit(input_file&     InputFile,
                      key_value_pair* KeyValues,
                      granny_int32x   NumKeyValues,
                      granny_memory_arena* TempArena)
{
    char const* OutputFile = FindFirstValueForKey(KeyValues, NumKeyValues, "output");
    if (OutputFile == 0)
    {
        fprintf(stderr, "MeshIndices16Bit: must specify an output file with \"-output <filename>\"\n");
        return false;
    }

    granny_file_info* Info = GrannyGetFileInfo(InputFile.GrannyFile);
    if (Info == 0)
    {
        fprintf(stderr, "MeshIndices16Bit: unable to obtain a granny_file_info from the input file\n");
        return false;
    }

    // Optimize all meshes for the pre/post-transform cache
    {for(granny_int32x i = 0; i < Info->MeshCount; i++)
    {
        granny_mesh* Mesh = Info->Meshes[i];
        if (!Mesh)
            continue;

        if (GrannyGetMeshVertexCount(Mesh) <= (1 << 16))
        {
            // Replace the 32-bit indices with 16-bit indices
            granny_int32x  IndexCount   = GrannyGetMeshIndexCount(Mesh);
            granny_uint16* NewIndices16 =
                ArenaPushArray(TempArena, granny_uint16, IndexCount);

            GrannyCopyMeshIndices(Mesh, 2, NewIndices16);
            Mesh->PrimaryTopology->IndexCount = 0;
            Mesh->PrimaryTopology->Indices = 0;
            Mesh->PrimaryTopology->Index16Count = IndexCount;
            Mesh->PrimaryTopology->Indices16 = NewIndices16;
        }
        else
        {
            // error here potentially?
        }
    }}

    // Write out the modified Info
    granny_file_builder *Builder = GrannyBeginFile(GrannyStandardSectionCount,
                                                   GrannyCurrentGRNStandardTag,
                                                   GrannyGRNFileMV_ThisPlatform,
                                                   GrannyGetTemporaryDirectory(),
                                                   "Prefix");
    granny_file_data_tree_writer *Writer =
        GrannyBeginFileDataTreeWriting(GrannyFileInfoType, Info,
                                       GrannyStandardDiscardableSection, 0);
    GrannyWriteDataTreeToFileBuilder(Writer, Builder);
    GrannyEndFileDataTreeWriting(Writer);

    bool success = GrannyEndFile(Builder, OutputFile);
    if (!success)
    {
        fprintf(stderr, "MeshIndices16Bit: failed to write to %s\n", OutputFile);
    }

    return success;
}

