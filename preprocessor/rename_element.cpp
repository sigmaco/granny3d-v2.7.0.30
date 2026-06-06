// ========================================================================
// $File: //jeffr/granny/preprocessor/rename_element.cpp $
// $DateTime: 2007/07/24 11:51:39 $
// $Change: 15577 $
// $Revision: #2 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "rename_element.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <vector>
#include <algorithm>

using namespace std;
static CommandRegistrar RegRenameElement(eSingleFile_NoMultiRun,
                                         RenameElement,
                                         "RenameElement",
                                         "Renames granny_file_info members specified by \"-rename Name[idx]\"");

bool ExtractElementIdx(char const* ElementAndIdx,
                       char* OutputElement,
                       granny_int32x* OutputIndex)
{
    assert(ElementAndIdx);
    assert(OutputElement);
    assert(OutputIndex);

    const char* LeftBracket  = strchr(ElementAndIdx, '[');
    const char* RightBracket = strchr(ElementAndIdx, ']');

    if (!LeftBracket || !RightBracket ||
        RightBracket < LeftBracket || RightBracket == LeftBracket+1)
    {
        printf("RenameElement: Invalid array notation: %s\n", ElementAndIdx);
        return false;
    }

    char* End = std::copy(ElementAndIdx, LeftBracket, OutputElement);
    *End = '\0';

    bool AllNumbers = true;
    for (char const* Test = LeftBracket + 1; Test < RightBracket; ++Test)
    {
        if (!isdigit(*Test))
            AllNumbers = false;
    }

    if (!AllNumbers)
    {
        printf("RenameElement: Invalid array notation: %s\n", ElementAndIdx);
        return false;
    }

    *OutputIndex = atoi(LeftBracket+1);
    return true;
}

bool DoElementRename(granny_file_info* Info,
                     char const* ElementName,
                     granny_int32x Index,
                     char const* NewName)
{
    if (Index < 0)
    {
        fprintf(stderr, "RenameElements: invalid index (%d)\n", Index);
        return false;
    }

    // Suxors, but there aren't really that many elements to rename at the root level...
    if (_stricmp(ElementName, "materials") == 0)
    {
        if (Index >= Info->MaterialCount)
        {
            fprintf(stderr, "RenameElements: invalid index (%d, only %d materials present)\n",
                    Index, Info->MaterialCount);
            return false;
        }

        if (Info->Materials[Index])
            Info->Materials[Index]->Name = NewName;
    }
    else if (_stricmp(ElementName, "animations") == 0)
    {
        if (Index >= Info->AnimationCount)
        {
            fprintf(stderr, "RenameElements: invalid index (%d, only %d animations present)\n",
                    Index, Info->AnimationCount);
            return false;
        }

        if (Info->Animations[Index])
            Info->Animations[Index]->Name = NewName;
    }
    else if (_stricmp(ElementName, "meshes") == 0)
    {
        if (Index >= Info->MeshCount)
        {
            fprintf(stderr, "RenameElements: invalid index (%d, only %d meshes present)\n",
                    Index, Info->MeshCount);
            return false;
        }

        if (Info->Meshes[Index])
            Info->Meshes[Index]->Name = NewName;
    }
    else if (_stricmp(ElementName, "models") == 0)
    {
        if (Index >= Info->ModelCount)
        {
            fprintf(stderr, "RenameElements: invalid index (%d, only %d materials present)\n",
                    Index, Info->ModelCount);
            return false;
        }

        if (Info->Models[Index])
            Info->Models[Index]->Name = NewName;
    }

    return true;
}

bool RenameElement(input_file&     InputFile,
                   key_value_pair* KeyValues,
                   granny_int32x   NumKeyValues,
                   granny_memory_arena* Arena)
{
    char const* OutputFile = FindFirstValueForKey(KeyValues, NumKeyValues, "output");
    if (OutputFile == 0)
    {
        fprintf(stderr, "RenameElements: must specify an output file with \"-output <filename>\"\n");
        return false;
    }

    char const* TargetName = FindFirstValueForKey(KeyValues, NumKeyValues, "newname");
    if (OutputFile == 0)
    {
        fprintf(stderr, "RenameElements: must specify a Name value with \"-newname <value>\"\n");
        return false;
    }

    granny_file* File = InputFile.GrannyFile;
    granny_file_info* Info = GrannyGetFileInfo(File);
    if (Info == 0)
    {
        fprintf(stderr, "RenameElements: unable to obtain a granny_file_info from the input file\n");
        return false;
    }

    bool FoundAny = false;
    {for (granny_int32x Key = 0; Key < NumKeyValues; ++Key)
    {
        if (_stricmp(KeyValues[Key].Key, "rename") == 0)
        {
            char ElementBuffer[512];
            granny_int32x Index;
            if (ExtractElementIdx(KeyValues[Key].Value, ElementBuffer, &Index))
            {
                FoundAny = true;

                if (!DoElementRename(Info, ElementBuffer, Index, TargetName))
                {
                    fprintf(stderr, "RenameElements: Failed to rename: %s[%d] to %s\n",
                            ElementBuffer, Index, TargetName);
                    return false;
                }
            }
        }
    }}

    if (FoundAny)
    {
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
    else
    {
        fprintf(stderr, "RenameElements: unable to find any valid \"-rename\" keys\n");
        return false;
    }
}

