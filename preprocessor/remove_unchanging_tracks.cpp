// ========================================================================
// $File: //jeffr/granny/preprocessor/remove_unchanging_tracks.cpp $
// $DateTime: 2007/07/24 11:51:39 $
// $Change: 15577 $
// $Revision: #2 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "remove_unchanging_tracks.h"
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <math.h>

using namespace std;

static CommandRegistrar RegCombineElements(eMultiFile,
                                           RemoveUnchangingTracks,
                                           "RemoveUnchangingTracks",
                                           "Removes constant tracks from an animation that are the same as the base pose of the model");


granny_real32 SquaredDistanceBetweenN(granny_int32x ElementCount,
                                      granny_real32 const* Vector0,
                                      granny_real32 const* Vector1)
{
    granny_real32 Result = 0.0f;
    while(ElementCount--)
    {
        granny_real32 ElemDist = (*Vector0++ - *Vector1++);
        Result += ElemDist * ElemDist;
    }

    return Result;
}

float GetPosError(granny_transform& One,
                  granny_transform& Two)
{
    return sqrt(SquaredDistanceBetweenN(3, One.Position, Two.Position));
}

float GetOriError(granny_transform& One,
                  granny_transform& Two)
{
    float InnerProd = 0.0f;
    for (int i = 0; i < 4; i++)
        InnerProd += One.Orientation[i] * Two.Orientation[i];

    return ((1.0f - InnerProd) *
            (1.0f - InnerProd));
}

float GetSSError(granny_transform& One,
                 granny_transform& Two)
{
    return sqrt(SquaredDistanceBetweenN(9, (granny_real32*)One.ScaleShear, (granny_real32*)Two.ScaleShear));
}


void CleanTrackGroup(granny_track_group* TrackGroup,
                     granny_model* Model,
                     granny_memory_arena* Arena)
{
    assert(TrackGroup);
    assert(Model);
    assert(Model->Skeleton);

    vector<granny_transform_track> RemainingTracks;
    vector<granny_real32> FinalLODs;
    {for(granny_int32x Idx = 0; Idx < TrackGroup->TransformTrackCount; ++Idx)
    {
        granny_transform_track& Track = TrackGroup->TransformTracks[Idx];
        bool strip = false;
        if (GrannyCurveIsConstantOrIdentity(&Track.OrientationCurve) &&
            GrannyCurveIsConstantOrIdentity(&Track.PositionCurve) &&
            GrannyCurveIsConstantOrIdentity(&Track.ScaleShearCurve))
        {
            // Find the corresponding bone on the model...
            granny_int32x BoneIdx;
            if (GrannyFindBoneByNameLowercase(Model->Skeleton, Track.Name, &BoneIdx))
            {
                granny_bone& Bone = Model->Skeleton->Bones[BoneIdx];

                granny_transform TrackTransform;
                GrannyGetTrackInitialTransform(&Track, &TrackTransform);

                // This is a bit lame, it only uses the exporter defaults.  We need a way
                // to explicitly control the tolerance here..
                float PosError = GetPosError(Bone.LocalTransform, TrackTransform);
                float OriError = GetOriError(Bone.LocalTransform, TrackTransform);
                float SSError  = GetSSError(Bone.LocalTransform, TrackTransform);

                if (PosError < 0.01f &&
                    OriError < GrannyOrientationToleranceFromDegrees(0.1f) &&
                    SSError  < 0.001f)
                {
                    strip = true;
                }
            }
        }

        if (!strip)
        {
            RemainingTracks.push_back(Track);
            if (TrackGroup->TransformLODErrorCount != 0)
                FinalLODs.push_back(TrackGroup->TransformLODErrors[Idx]);
        }
    }}

    if (RemainingTracks.size() != TrackGroup->TransformTrackCount)
    {
        TrackGroup->TransformTrackCount = (granny_int32)RemainingTracks.size();
        TrackGroup->TransformTracks =
            ArenaPushArray(Arena, granny_transform_track, TrackGroup->TransformTrackCount);
        memcpy(TrackGroup->TransformTracks, &RemainingTracks[0],
               sizeof(granny_transform_track) * RemainingTracks.size());

        if (FinalLODs.size())
        {
            assert(FinalLODs.size() == RemainingTracks.size());

            TrackGroup->TransformLODErrorCount = (granny_int32)FinalLODs.size();
            TrackGroup->TransformLODErrors =
                ArenaPushArray(Arena, granny_real32, TrackGroup->TransformLODErrorCount);
            memcpy(TrackGroup->TransformLODErrors, &FinalLODs[0],
                   sizeof(granny_real32) * TrackGroup->TransformLODErrorCount);
        }
    }
}


bool RemoveUnchangingTracks(input_file* InputFiles, granny_int32x NumInputFiles,
                            key_value_pair* KeyValues, granny_int32x NumKeyValues,
                            granny_memory_arena* Arena)
{
    if (NumInputFiles != 2)
    {
        fprintf(stderr, "RemoveUnchangingTracks: only valid for 2 input files\n");
        return false;
    }

    char const* OutputFile = FindFirstValueForKey(KeyValues, NumKeyValues, "output");
    if (OutputFile == 0)
    {
        fprintf(stderr, "RemoveUnchangingTracks: must specify an output file with \"-output <filename>\"\n");
        return false;
    }

    granny_file_info* ModelInfo = GrannyGetFileInfo(InputFiles[0].GrannyFile);
    if (ModelInfo == 0)
    {
        fprintf(stderr, "RemoveUnchangingTracks: unable to obtain a granny_file_info from: %s\n",
                InputFiles[0].Filename);
        return false;
    }

    granny_file* AnimFile = InputFiles[1].GrannyFile;
    granny_file_info* AnimInfo = GrannyGetFileInfo(AnimFile);
    if (AnimInfo == 0)
    {
        fprintf(stderr, "RemoveUnchangingTracks: unable to obtain a granny_file_info from: %s\n",
                InputFiles[1].Filename);
        return false;
    }


    {for(granny_int32x AnimIdx = 0; AnimIdx < AnimInfo->AnimationCount; ++AnimIdx)
    {
        granny_animation* Animation = AnimInfo->Animations[AnimIdx];
        if (!Animation)
            continue;

        {for(granny_int32x TGIdx = 0; TGIdx < Animation->TrackGroupCount; ++TGIdx)
        {
            granny_track_group* TrackGroup = Animation->TrackGroups[TGIdx];
            if (!TrackGroup)
                continue;

            // Find the model for this track group
            bool Cleaned = false;
            {for(granny_int32x ModelIdx = 0; !Cleaned && ModelIdx < ModelInfo->ModelCount; ++ModelIdx)
            {
                granny_model* Model = ModelInfo->Models[ModelIdx];
                if (!Model)
                    continue;

                if (stricmp(Model->Name, TrackGroup->Name) == 0)
                {
                    CleanTrackGroup(TrackGroup, Model, Arena);
                    Cleaned = true;
                }
            }}

            if (!Cleaned)
            {
                fprintf(stderr, "CombineElements: failed to find a model for track_group: %s\n",
                        TrackGroup->Name);
            }
        }}
    }}


    // Write out the modified Info
    granny_int32x FinalSectionCount = AnimFile->SectionCount;
    if (FinalSectionCount < GrannyStandardSectionCount)
        FinalSectionCount = GrannyStandardSectionCount;

    granny_file_builder *Builder = GrannyBeginFile(FinalSectionCount,
                                                   GrannyCurrentGRNStandardTag,
                                                   GrannyGRNFileMV_ThisPlatform,
                                                   GrannyGetTemporaryDirectory(),
                                                   "Prefix");
    granny_file_data_tree_writer *Writer =
        GrannyBeginFileDataTreeWriting(GrannyFileInfoType, AnimInfo,
                                       GrannyStandardDiscardableSection, 0);
    GrannyPreserveObjectFileSections(Writer, AnimFile);
    GrannyWriteDataTreeToFileBuilder(Writer, Builder);
    GrannyEndFileDataTreeWriting(Writer);

    bool success = GrannyEndFile(Builder, OutputFile);
    if (!success)
    {
        fprintf(stderr, "CombineElements: failed to write to %s\n", OutputFile);
    }

    return success;
}
