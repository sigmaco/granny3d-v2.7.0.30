// ========================================================================
// $File: //jeffr/granny/preprocessor/clean_materials.cpp $
// $DateTime: 2007/07/24 11:51:39 $
// $Change: 15577 $
// $Revision: #10 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "clean_materials.h"
#include <stdio.h>
#include <set>

using namespace std;

static CommandRegistrar RegCleanMaterials(eSingleFile_NoMultiRun,
                                          CleanMaterials,
                                          "CleanMaterials",
                                          "Removes unreferenced materials from the file");

typedef set<granny_material*> MaterialSet;
typedef set<granny_texture*>  TextureSet;


void CleanUnreferencedMaterials(granny_file_info* FileInfo,
                                granny_memory_arena* TempArena)
{
    MaterialSet MeshReferencedMaterials;


    // Loop through all the meshes, checking references
    {for(granny_int32x Idx = 0; Idx < FileInfo->MeshCount; ++Idx)
    {
        granny_mesh* Mesh = FileInfo->Meshes[Idx];

        // and all the material bindings on the mesh
        {for(granny_int32x Binding = 0; Binding < Mesh->MaterialBindingCount; ++Binding)
        {
            if (Mesh->MaterialBindings[Binding].Material != NULL)
                MeshReferencedMaterials.insert(Mesh->MaterialBindings[Binding].Material);
        }}
    }}

    // This is really a suck-tastic way to walk the material tree, but it's easy to verify
    // that it works.
    bool inserted;
    MaterialSet FinalMaterials;
    TextureSet FinalTextures;
    do
    {
        inserted = false;

        {for(MaterialSet::iterator itr = MeshReferencedMaterials.begin();
             itr != MeshReferencedMaterials.end();
             ++itr)
        {
            granny_material* MeshMaterial = *itr;
            if (FinalMaterials.insert(MeshMaterial).second)
            {
                inserted = true;
                if (MeshMaterial->Texture != NULL)
                    FinalTextures.insert(MeshMaterial->Texture);

                {for(granny_int32x Map = 0; Map < MeshMaterial->MapCount; ++Map)
                {
                    granny_material* SubMaterial = MeshMaterial->Maps[Map].Material;
                    FinalMaterials.insert(MeshMaterial->Maps[Map].Material);
                    if (SubMaterial->Texture != NULL)
                        FinalTextures.insert(SubMaterial->Texture);
                }}
            }
        }}

        MeshReferencedMaterials = FinalMaterials;
    } while (inserted);

    // Ok, create the final material array...
    FileInfo->MaterialCount = (granny_int32)FinalMaterials.size();
    if (FileInfo->MaterialCount != 0)
    {
        FileInfo->Materials =
            ArenaPushArray(TempArena, granny_material*, FileInfo->MaterialCount);

        granny_int32x Current = 0;
        {for(MaterialSet::iterator itr = FinalMaterials.begin();
             itr != FinalMaterials.end();
             ++itr)
        {
            FileInfo->Materials[Current++] = *itr;
        }}
    }
    else
    {
        FileInfo->Materials = 0;
    }

    // ...and the final texture array
    FileInfo->TextureCount = (granny_int32)FinalTextures.size();
    if (FileInfo->TextureCount != 0)
    {
        FileInfo->Textures =
            ArenaPushArray(TempArena, granny_texture*, FileInfo->TextureCount);

        granny_int32x Current = 0;
        {for(TextureSet::iterator itr = FinalTextures.begin();
             itr != FinalTextures.end();
             ++itr)
        {
            FileInfo->Textures[Current++] = *itr;
        }}
    }
    else
    {
        FileInfo->Textures = 0;
    }
}

bool CleanMaterials(input_file&     InputFile,
                    key_value_pair* KeyValues,
                    granny_int32x   NumKeyValues,
                    granny_memory_arena* TempArena)
{
    char const* OutputFile = FindFirstValueForKey(KeyValues, NumKeyValues, "output");
    if (OutputFile == 0)
    {
        fprintf(stderr, "CleanMaterials: must specify an output file with \"-output <filename>\"\n");
        return false;
    }

    granny_file* File = InputFile.GrannyFile;
    granny_file_info* FileInfo = GrannyGetFileInfo(File);
    if (FileInfo == 0)
    {
        fprintf(stderr, "CleanMaterials: unable to obtain a granny_file_info from the input model file\n");
        return false;
    }

    // Remove unused materials and textures
    CleanUnreferencedMaterials(FileInfo, TempArena);

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
        GrannyBeginFileDataTreeWriting(GrannyFileInfoType, FileInfo,
                                       GrannyStandardDiscardableSection, 0);
    GrannyPreserveObjectFileSections(Writer, File);

    GrannyWriteDataTreeToFileBuilder(Writer, Builder);
    GrannyEndFileDataTreeWriting(Writer);

    bool success = GrannyEndFile(Builder, OutputFile);
    if (!success)
    {
        fprintf(stderr, "CleanMaterials: failed to write to %s\n", OutputFile);
    }

    return true;
}
