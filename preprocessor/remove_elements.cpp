// ========================================================================
// $File: //jeffr/granny/preprocessor/remove_elements.cpp $
// $DateTime: 2007/10/29 21:53:04 $
// $Change: 16393 $
// $Revision: #10 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "remove_elements.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <algorithm>

using namespace std;
static CommandRegistrar RegRemoveElements(eSingleFile_NoMultiRun,
                                          RemoveElements,
                                          "RemoveElements",
                                          "Removes granny_file_info members specified by \"-remove Name\" k/v pairs");

// Actually does the work of finding the specified element, and killing it.  Because of
// the way the granny file type works, we can just 0 it out, and call it good.  Note that
// to ensure that the object is REALLY gone, we need to recursively search the object
// hierarchy.  To simplify the code, we'll do that in another function, and simply return
// the pointers we've removed here.
bool RemoveObjectElement(void* Object,
                         granny_data_type_definition* Type,
                         char const* ElementName,
                         vector<void*>& RemovedObjects)
{
    assert(RemovedObjects.size() == 0);

    granny_uint8* CurrObj = (granny_uint8*)Object;
    granny_data_type_definition* CurrentType = Type;

    while (CurrentType->Type != GrannyEndMember)
    {
        if (_stricmp(CurrentType->Name, ElementName) == 0)
        {
            switch (CurrentType->Type)
            {
                case GrannyStringMember:
                {
                    // Simple pointer, just null it and return success
                    void** pPointer = (void**)CurrObj;
                    *pPointer = NULL;
                    return true;
                } break;

                case GrannyReferenceMember:
                {
                    // A simple pointer, but we do have to return the object to the cleaning routine
                    void** pPointer = (void**)CurrObj;
                    RemovedObjects.push_back(*pPointer);
                    *pPointer = NULL;
                    return true;
                } break;

                case GrannyReferenceToArrayMember:
                {
                    // 32-bit count followed by a pointer.  0 them both.  No returned
                    // pointers.
                    granny_int32* pCount = (granny_int32x*)(CurrObj);
                    void** pPointer      = (void**)(CurrObj + 4);
                    *pCount   = 0;
                    *pPointer = NULL;

                    return true;
                } break;

                case GrannyArrayOfReferencesMember:
                {
                    // 32-bit count followed by a pointer.  0 them both, return the values
                    // contained
                    granny_int32* pCount = (granny_int32x*)(CurrObj);
                    void*** pPointer     = (void***)(CurrObj + 4);

                    {for(granny_int32 Idx = 0; Idx < *pCount; ++Idx)
                    {
                        if ((*pPointer)[Idx] != NULL)
                            RemovedObjects.push_back((*pPointer)[Idx]);
                    }}
                    *pCount   = 0;
                    *pPointer = NULL;

                    return true;
                } break;

                case GrannyVariantReferenceMember:
                {
                    // These are 2 pointers.  0 them both.  Note that in the case of
                    // variants and their types, we don't have to return values to the
                    // tree walker.
                    void** pPointer0 = (void**)(CurrObj + 0);
                    void** pPointer1 = (void**)(CurrObj + sizeof(void*));
                    *pPointer0 = NULL;
                    *pPointer1 = NULL;

                    return true;
                } break;

                case GrannyReferenceToVariantArrayMember:
                {
                    // pointer/count/pointer.  Note that in the case of variants and their
                    // types, we don't have to return values to the tree walker.
                    void** pPointer0      = (void**)(CurrObj + 0);
                    granny_int32x* pCount = (granny_int32x*)(CurrObj + sizeof(void*));
                    void** pPointer1      = (void**)(CurrObj + sizeof(void*) + 4);
                    *pPointer0 = NULL;
                    *pCount    = 0;
                    *pPointer1 = NULL;
                    return true;
                } break;

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
                    // We can't remove members of these types
                    return false;

                    // Bad Type value...
                default:
                    assert(false);
                    return false;
            }


            return true;
        }

        // Advance in the object, and the type array
        CurrObj += GrannyGetMemberTypeSize(CurrentType);
        ++CurrentType;
    }

    return false;
}


bool RecursivelyCleanPointer(void* Object,
                             granny_data_type_definition* Type,
                             void* ValueToRemove,
                             granny_memory_arena* Arena,
                             vector<void*>& VisitedStack)
{
    assert(ValueToRemove != NULL);

    // Walk this object
    {
        granny_uint8* CurrObj = (granny_uint8*)Object;
        granny_data_type_definition* CurrentType = Type;

        while (CurrentType->Type != GrannyEndMember)
        {
            switch (CurrentType->Type)
            {
                case GrannyReferenceMember:
                {
                    void** pPointer = (void**)CurrObj;
                    if (*pPointer == ValueToRemove)
                        *pPointer = NULL;
                } break;

                case GrannyArrayOfReferencesMember:
                {
                    // 32-bit count followed by a pointer.  0 them both, return the values
                    // contained
                    granny_int32* pCount = (granny_int32x*)(CurrObj);
                    void*** pPointer      = (void***)(CurrObj + 4);

                    // This is a little weird for a find loop, but we want to handle the case
                    // that multiple pointers in the array could be the object we're looking
                    // for.
                    int NotTheValue = 0;
                    {for(granny_int32 Idx = 0; Idx < *pCount; ++Idx)
                    {
                        if ((*pPointer)[Idx] != ValueToRemove)
                            ++NotTheValue;
                    }}

                    if (NotTheValue == 0)
                    {
                        *pCount = 0;
                        *pPointer = NULL;
                    }
                    if (NotTheValue != *pCount)
                    {
                        void** NewArray = ArenaPushArray(Arena, void*, NotTheValue);
                        if (NewArray == NULL)
                            return false;

                        int CurrVal = 0;
                        {for(granny_int32 Idx = 0; Idx < *pCount; ++Idx)
                        {
                            if ((*pPointer)[Idx] != ValueToRemove)
                                NewArray[CurrVal++] = (*pPointer)[Idx];
                        }}
                        assert(CurrVal == NotTheValue);

                        *pCount = NotTheValue;
                        *pPointer = NewArray;
                    }
                } break;

                default:
                    // Ignore
                    break;
            }

            // Advance in the object, and the type array
            CurrObj += GrannyGetMemberTypeSize(CurrentType);
            ++CurrentType;
        }
    }

    // Recurse through SubObjects
    {
        granny_uint8* CurrObj = (granny_uint8*)Object;
        granny_data_type_definition* CurrentType = Type;

        while (CurrentType->Type != GrannyEndMember)
        {
            switch (CurrentType->Type)
            {
                case GrannyReferenceMember:
                {
                    void** pPointer = (void**)CurrObj;
                    if (*pPointer != NULL)
                    {
                        if (find(VisitedStack.begin(), VisitedStack.end(), *pPointer) == VisitedStack.end())
                        {
                            VisitedStack.push_back(*pPointer);
                            if (RecursivelyCleanPointer(*pPointer,
                                                        CurrentType->ReferenceType,
                                                        ValueToRemove,
                                                        Arena, VisitedStack) == false)
                            {
                                return false;
                            }
                            VisitedStack.pop_back();
                        }
                    }
                } break;

                case GrannyArrayOfReferencesMember:
                {
                    granny_int32* pCount = (granny_int32x*)(CurrObj);
                    void*** pPointer     = (void***)(CurrObj + 4);

                    if (*pCount != 0)
                    {
                        assert(*pPointer != NULL);

                        {for(granny_int32 Idx = 0; Idx < *pCount; ++Idx)
                        {
                            if (find(VisitedStack.begin(), VisitedStack.end(), (*pPointer)[Idx]) == VisitedStack.end())
                            {
                                VisitedStack.push_back((*pPointer)[Idx]);
                                if (RecursivelyCleanPointer((*pPointer)[Idx],
                                                            CurrentType->ReferenceType,
                                                            ValueToRemove,
                                                            Arena, VisitedStack) == false)
                                {
                                    return false;
                                }
                                VisitedStack.pop_back();
                            }
                        }}
                    }
                } break;

                case GrannyVariantReferenceMember:
                {
                    void** pType   = (void**)(CurrObj);
                    void** pObject = (void**)(CurrObj + 4);

                    if (*pType != 0 && *pObject != 0)
                    {
                        if (find(VisitedStack.begin(), VisitedStack.end(), *pObject) == VisitedStack.end())
                        {
                            VisitedStack.push_back(*pObject);
                            if (RecursivelyCleanPointer(*pObject,
                                                        (granny_data_type_definition*)*pType,
                                                        ValueToRemove,
                                                        Arena, VisitedStack) == false)
                            {
                                return false;
                            }
                            VisitedStack.pop_back();
                        }
                    }
                } break;

                // Ignore
                default:
                    break;
            }

            // Advance in the object, and the type array
            CurrObj += GrannyGetMemberTypeSize(CurrentType);
            ++CurrentType;
        }
    }


    return true;
}


bool RemoveElements(input_file&     InputFile,
                    key_value_pair* KeyValues,
                    granny_int32x   NumKeyValues,
                    granny_memory_arena* Arena)
{
    char const* OutputFile = FindFirstValueForKey(KeyValues, NumKeyValues, "output");
    if (OutputFile == 0)
    {
        fprintf(stderr, "RemoveElements: must specify an output file with \"-output <filename>\"\n");
        return false;
    }

    granny_file* File = InputFile.GrannyFile;
    granny_file_info* Info = GrannyGetFileInfo(File);
    if (Info == 0)
    {
        fprintf(stderr, "RemoveElements: unable to obtain a granny_file_info from the input file\n");
        return false;
    }

    {for(granny_int32x i = 0; i < NumKeyValues; ++i)
    {
        if (_stricmp(KeyValues[i].Key, "remove") == 0)
        {
            vector<void*> RemovedObjects;
            if (RemoveObjectElement(Info, GrannyFileInfoType,
                                    KeyValues[i].Value,
                                    RemovedObjects) == false)
            {
                fprintf(stderr,
                        "RemoveElements: failed to remove element: %s\n",
                        KeyValues[i].Value);
                return false;
            }

            {for(granny_int32x j = 0; j < int(RemovedObjects.size()); ++j)
            {
                vector<void*> VisitedStack;
                if (RecursivelyCleanPointer(Info, GrannyFileInfoType,
                                            RemovedObjects[j], Arena,
                                            VisitedStack) == false)
                {
                    fprintf(stderr,
                            "RemoveElements: failed to recursively clean for element: %s\n",
                            KeyValues[i].Value);
                    return false;
                }
            }}
        }
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
    {
        fprintf(stderr, "RemoveElements: failed to write to %s\n", OutputFile);
    }

    return success;
}
