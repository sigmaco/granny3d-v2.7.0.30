// ========================================================================
// $File: //jeffr/granny/preprocessor/combine_elements.cpp $
// $DateTime: 2007/07/24 11:51:39 $
// $Change: 15577 $
// $Revision: #6 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "combine_elements.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static CommandRegistrar RegCombineElements(eMultiFile,
                                           CombineElements,
                                           "CombineElements",
                                           "Combines root level elements of the granny_file_info");

bool CombineObjectElement(granny_file_info* BaseObject,
                          granny_file_info** Secondaries,
                          granny_int32x SecondaryCount,
                          char const* ElementName,
                          granny_memory_arena* TempArena);


bool CombineElements(input_file*     InputFiles,
                     granny_int32x   NumInputFiles,
                     key_value_pair* KeyValues,
                     granny_int32x   NumKeyValues,
                     granny_memory_arena* TempArena)
{
    if (NumInputFiles < 2)
    {
        fprintf(stderr, "CombineElements: only valid for more than one input file\n");
        return false;
    }

    char const* OutputFile = FindFirstValueForKey(KeyValues, NumKeyValues, "output");
    if (OutputFile == 0)
    {
        fprintf(stderr, "CombineElements: must specify an output file with \"-output <filename>\"\n");
        return false;
    }

    granny_file_info* Info = GrannyGetFileInfo(InputFiles[0].GrannyFile);
    if (Info == 0)
    {
        fprintf(stderr, "CombineElements: unable to obtain a granny_file_info from the input file\n");
        return false;
    }

    granny_file_info** Secondaries = new granny_file_info*[NumInputFiles];

    {for(granny_int32x File = 1; File < NumInputFiles; ++File)
    {
        Secondaries[File-1] = GrannyGetFileInfo(InputFiles[File].GrannyFile);
        if (Secondaries[File-1] == 0)
        {
            fprintf(stderr, "CombineElements: unable to obtain a granny_file_info from: %s\n",
                    InputFiles[File].Filename);
            return false;
        }
    }}

    {for(granny_int32x i = 0; i < NumKeyValues; ++i)
    {
        if (_stricmp(KeyValues[i].Key, "combine") == 0)
        {
            if (CombineObjectElement(Info, Secondaries,
                                     NumInputFiles - 1, KeyValues[i].Value,
                                     TempArena) == false)
            {
                fprintf(stderr, "CombineElements: failed to combine element: %s\n",
                        KeyValues[i].Value);
                return false;
            }
        }
    }}


    delete [] Secondaries;

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
        fprintf(stderr, "CombineElements: failed to write to %s\n", OutputFile);
    }

    return success;
}


// Handy little utilities for dealing with ArrayOfRef members
#define AOR_ExtractCountAndPtr(Count, Pointer, Memory)      \
    do {                                                    \
        Count   = *((granny_int32*)(Memory));               \
        Pointer = *((void**)((granny_uint8*)Memory + 4));   \
    } while (false)

#define AOR_SetCountAndPtr(Count, Pointer, Memory)          \
    do {                                                    \
        *((granny_int32*)(Memory)) = Count;                 \
        *((void**)((granny_uint8*)(Memory) + 4)) = Pointer; \
    } while (false)

bool CombineArrayOfRefMember(granny_file_info* BaseObject,
                             granny_file_info** Secondaries,
                             granny_int32x SecondaryCount,
                             granny_int32x DataOffset,
                             granny_memory_arena* TempArena)
{
    granny_int32 TotalObjects;
    void* Ignored;

    // Extract the current count from the BaseObject
    AOR_ExtractCountAndPtr(TotalObjects, Ignored,
                           ((granny_uint8*)BaseObject) + DataOffset);

    {for(granny_int32x Idx = 0; Idx < SecondaryCount; ++Idx)
    {
        granny_int32 Count;
        AOR_ExtractCountAndPtr(Count, Ignored,
                               ((granny_uint8*)(Secondaries[Idx])) + DataOffset);

        TotalObjects += Count;
    }}

    if (TotalObjects == 0)
    {
        // No objects, including in the base object, we're done...
        return true;
    }

    void** NewReferences = ArenaPushArray(TempArena, void*, TotalObjects);
    granny_int32x CurrentRef = 0;

    // Recopy the members from the base
    {
        granny_int32 Count;
        void* ArrayLocation;
        AOR_ExtractCountAndPtr(Count, ArrayLocation,
                               ((granny_uint8*)BaseObject) + DataOffset);

        void** Array = (void**)ArrayLocation;
        {for(granny_int32x Idx  = 0; Idx < Count; ++Idx)
        {
            NewReferences[CurrentRef++] = Array[Idx];
        }}
    }

    // And the new members from the secondaries...
    {for(granny_int32x Idx = 0; Idx < SecondaryCount; ++Idx)
    {
        granny_int32 Count;
        void* ArrayLocation;
        AOR_ExtractCountAndPtr(Count, ArrayLocation,
                               ((granny_uint8*)Secondaries[Idx]) + DataOffset);

        void** Array = (void**)ArrayLocation;
        {for(granny_int32x Idx  = 0; Idx < Count; ++Idx)
        {
            NewReferences[CurrentRef++] = Array[Idx];
        }}
    }}

    AOR_SetCountAndPtr(TotalObjects, NewReferences,
                       ((granny_uint8*)BaseObject) + DataOffset);

    return true;
}

bool CombineRefToVarArrayMember(granny_file_info* BaseObject,
                                granny_file_info** Secondaries,
                                granny_int32x SecondaryCount,
                                granny_int32x DataOffset,
                                granny_memory_arena* TempArena)
{
    // We don't actually handle these yet.  There aren't any in the
    // default granny_file_info structure.  Straightforward to add,
    // though it will be required that the variant types for all the
    // objects match.
    return false;
}


// Actually does the work of finding the specified element, and
// combining it.
bool CombineObjectElement(granny_file_info* BaseObject,
                          granny_file_info** Secondaries,
                          granny_int32x SecondaryCount,
                          char const* ElementName,
                          granny_memory_arena* TempArena)
{
    granny_int32x Offset = 0;

    granny_data_type_definition* CurrentType = GrannyFileInfoType;
    while (CurrentType->Type != GrannyEndMember)
    {
        if (_stricmp(CurrentType->Name, ElementName) == 0)
        {
            switch (CurrentType->Type)
            {
                case GrannyArrayOfReferencesMember:
                {
                    return CombineArrayOfRefMember(BaseObject,
                                                   Secondaries, SecondaryCount,
                                                   Offset, TempArena);
                } break;

                case GrannyReferenceToVariantArrayMember:
                {
                    return CombineRefToVarArrayMember(BaseObject,
                                                      Secondaries, SecondaryCount,
                                                      Offset, TempArena);
                } break;

                // We can't remove members of these types
                case GrannyReferenceToArrayMember:
                case GrannyReferenceMember:
                case GrannyStringMember:
                case GrannyVariantReferenceMember:
                case GrannyInlineMember:
                case GrannyUnsupportedMemberType_Remove:
                case GrannyTransformMember:
                case GrannyReal32Member:
                case GrannyInt8Member:
                case GrannyUInt8Member:
                case GrannyBinormalInt8Member:
                case GrannyNormalUInt8Member:
                case GrannyInt16Member:
                case GrannyUInt16Member:
                case GrannyBinormalInt16Member:
                case GrannyNormalUInt16Member:
                case GrannyInt32Member:
                case GrannyUInt32Member:
                    return false;

                    // Bad Type value...
                default:
                    assert(false);
                    return false;
            }


            return true;
        }

        // Advance in the object, and the type array
        Offset += GrannyGetMemberTypeSize(CurrentType);
        ++CurrentType;
    }

    return false;
}
