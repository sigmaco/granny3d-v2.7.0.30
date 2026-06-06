// ========================================================================
// $File: //jeffr/granny/preprocessor/warn_scale_shear.cpp $
// $DateTime: 2007/02/23 13:10:39 $
// $Change: 14432 $
// $Revision: #5 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "warn_scale_shear.h"
#include <stdio.h>


static CommandRegistrar RegWarnScaleShear(eSingleFile_MultiRun,
                                          WarnScaleShear,
                                          "WarnScaleShear",
                                          "Warns (and returns failure) if any components with non-identity scale/shear are found");

void IssueWarning(char const* Filename,
                  char const* Type,
                  char const* Name,
                  char const* Subname)
{
    fprintf(stderr, "%s: %s \"%s\" (%s) contains non-uniform scale/shear\n",
            Filename, Type, Name, Subname);
}


bool ExamineFileInfo(granny_file_info* Info,
                     char const* Filename)
{
    bool ComponentsValid = false;

    // Loop over all skeletons
    {for(granny_int32x Idx = 0; Idx < Info->SkeletonCount; ++Idx)
    {
        granny_skeleton* Skeleton = Info->Skeletons[Idx];
        if (!Skeleton)  // odd, but allowed
            continue;

        {for(granny_int32x BoneIdx = 0; BoneIdx < Skeleton->BoneCount; ++BoneIdx)
        {
            granny_bone& Bone = Skeleton->Bones[BoneIdx];

            if (Bone.LocalTransform.Flags & GrannyHasScaleShear)
            {
                IssueWarning(Filename, "Skeleton", Skeleton->Name, Bone.Name);
                ComponentsValid = false;
            }
        }}
    }}

    // Loop over all track groups
    {for(granny_int32x Idx = 0; Idx < Info->TrackGroupCount; ++Idx)
    {
        granny_track_group* TrackGroup = Info->TrackGroups[Idx];
        if (!TrackGroup)  // odd, but allowed
            continue;

        {for(granny_int32x XformTrackIdx = 0;
             XformTrackIdx < TrackGroup->TransformTrackCount;
             ++XformTrackIdx)
        {
            granny_transform_track& Track = TrackGroup->TransformTracks[XformTrackIdx];

            if (!GrannyCurveIsIdentity(&Track.ScaleShearCurve))
            {
                IssueWarning(Filename, "TransformTrack", TrackGroup->Name, Track.Name);
                ComponentsValid = false;
            }
        }}
    }}

    return ComponentsValid;
}

bool WarnScaleShear(input_file&     InputFile,
                    key_value_pair* KeyValues,
                    granny_int32x   NumKeyValues,
                    granny_memory_arena*)
{
    bool AllValid = true;

    granny_file_info* Info = GrannyGetFileInfo(InputFile.GrannyFile);
    if (Info == 0)
    {
        fprintf(stderr, "PlatformConvert: unable to obtain a granny_file_info from %s\n", InputFile.Filename);
        return false;
    }

    return ExamineFileInfo(Info, InputFile.Filename);
}
