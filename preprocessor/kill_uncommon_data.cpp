// ========================================================================
// $File: //jeffr/granny/preprocessor/kill_uncommon_data.cpp $
// $DateTime: 2007/11/29 14:42:34 $
// $Change: 16656 $
// $Revision: #9 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "kill_uncommon_data.h"
#include <stdio.h>

static CommandRegistrar RegKillUncommonData(eSingleFile_NoMultiRun,
                                            KillUncommonData,
                                            "KillUncommonData",
                                            "Removes data mostly useful in toolchains rather than the runtime to save space");

bool KillUncommonData(input_file&     InputFile,
                      key_value_pair* KeyValues,
                      granny_int32x   NumKeyValues,
                      granny_memory_arena*)
{
    char const* OutputFile = FindFirstValueForKey(KeyValues, NumKeyValues, "output");
    if (OutputFile == 0)
    {
        fprintf(stderr, "KillUncommonData: must specify an output file with \"-output <filename>\"\n");
        return false;
    }

    granny_file* File = InputFile.GrannyFile;
    granny_file_info* Info = GrannyGetFileInfo(File);
    if (Info == 0)
    {
        fprintf(stderr, "KillUncommonData: unable to obtain a granny_file_info from the input file\n");
        return false;
    }

    // Remove exporter details
    Info->ExporterInfo = 0;

    // Remove arttool extended info...
    if (Info->ArtToolInfo != 0)
    {
        Info->ArtToolInfo->ExtendedData.Object = 0;
        Info->ArtToolInfo->ExtendedData.Type = 0;
    }

    // Remove bone extended data
    {for(granny_int32x Skel = 0; Skel < Info->SkeletonCount; ++Skel)
    {
        granny_skeleton* Skeleton = Info->Skeletons[Skel];
        {for(granny_int32x Bone = 0; Bone < Skeleton->BoneCount; ++Bone)
        {
            Skeleton->Bones[Bone].ExtendedData.Type = 0;
            Skeleton->Bones[Bone].ExtendedData.Object = 0;
        }}
    }}

    // Remove vertex annotations and component names
    {for(granny_int32x VData = 0; VData < Info->VertexDataCount; ++VData)
    {
        Info->VertexDatas[VData]->VertexComponentNameCount = 0;
        Info->VertexDatas[VData]->VertexComponentNames = 0;
        Info->VertexDatas[VData]->VertexAnnotationSetCount = 0;
        Info->VertexDatas[VData]->VertexAnnotationSets = 0;
    }}

    // Remove all triangle annotations from the topologies
    {for(granny_int32x Topo = 0; Topo < Info->TriTopologyCount; ++Topo)
    {
        Info->TriTopologies[Topo]->VertexToVertexCount = 0;
        Info->TriTopologies[Topo]->VertexToVertexMap = 0;
        Info->TriTopologies[Topo]->VertexToTriangleCount = 0;
        Info->TriTopologies[Topo]->VertexToTriangleMap = 0;
        Info->TriTopologies[Topo]->SideToNeighborCount = 0;
        Info->TriTopologies[Topo]->SideToNeighborMap = 0;
        Info->TriTopologies[Topo]->BonesForTriangleCount = 0;
        Info->TriTopologies[Topo]->BonesForTriangle = 0;
        Info->TriTopologies[Topo]->TriangleToBoneCount = 0;
        Info->TriTopologies[Topo]->TriangleToBoneIndices = 0;
        Info->TriTopologies[Topo]->TriAnnotationSetCount = 0;
        Info->TriTopologies[Topo]->TriAnnotationSets = 0;
    }}

    // Remove extended data from the meshes
    {for(granny_int32x Mesh = 0; Mesh < Info->MeshCount; ++Mesh)
    {
        Info->Meshes[Mesh]->ExtendedData.Type = 0;
        Info->Meshes[Mesh]->ExtendedData.Object = 0;
    }}

    // Remove extended data from the track_groups
    {for(granny_int32x TrackGroup = 0; TrackGroup < Info->TrackGroupCount; ++TrackGroup)
    {
        Info->TrackGroups[TrackGroup]->ExtendedData.Type = 0;
        Info->TrackGroups[TrackGroup]->ExtendedData.Object = 0;
    }}

    // Remove extended data from the file_info itself
    Info->ExtendedData.Type = 0;
    Info->ExtendedData.Object = 0;

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
        fprintf(stderr, "KillUncommonData: failed to write to %s\n", OutputFile);
    }

    return success;
}
