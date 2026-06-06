// ========================================================================
// $File: //jeffr/granny/preprocessor/make_string_database.cpp $
// $DateTime: 2007/09/18 17:22:59 $
// $Change: 16008 $
// $Revision: #5 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "make_string_database.h"
#include <set>
#include <string>
#include <assert.h>

using namespace std;

struct remap_context
{
    granny_string_database* Database;
    bool                    MissingStringFound;

    remap_context(granny_string_database* Database_)
      : Database(Database_), MissingStringFound(false) {}
};

granny_uint32 GatherStringsCallback(void *Data, char const *String);
granny_uint32 RemapStringsCallback(void *Data, char const *String);

// Register the commands
static CommandRegistrar RegExtractStrings(eMultiFile, ExtractStrings, "ExtractStrings",
                                          "Creates or updates a string_database with the input files' strings (use -database to specify)");

static CommandRegistrar RegRemapStrings(eSingleFile_NoMultiRun, RemapStrings, "RemapStrings",
                                        "Remaps the strings in the given file to the input string_database (use -database to specify)");


bool ExtractStrings(input_file*     InputFiles,
                    granny_int32x   NumInputFiles,
                    key_value_pair* KeyValues,
                    granny_int32x   NumKeyValues,
                    granny_memory_arena* TempArena)
{
    char const* DatabaseName = FindFirstValueForKey(KeyValues, NumKeyValues, "database");
    if (DatabaseName == 0)
    {
        fprintf(stderr, "ExtractStrings: must specify a database with \"-database <filename>\"\n");
        return false;
    }

    set<string> FinalStrings;

    // Always insert the empty string...
    FinalStrings.insert("");

    // Pull the currents strings out of the input database
    granny_file* PrevDatabaseFile = GrannyReadEntireFile(DatabaseName);
    if (PrevDatabaseFile != NULL)
    {
        granny_string_database* PrevDatabase = GrannyGetStringDatabase(PrevDatabaseFile);
        if (PrevDatabase == NULL)
        {
            {for(granny_int32 i = 0; i < PrevDatabase->StringCount; ++i)
            {
                char const* ThisString = PrevDatabase->Strings[i];
                assert(ThisString != NULL);

                FinalStrings.insert(ThisString);
            }}
        }

        GrannyFreeFile(PrevDatabaseFile);
    }

    // Walk the files to extract new strings...
    {for(granny_int32x FileIdx = 0; FileIdx < NumInputFiles; ++FileIdx)
    {
        granny_file_info* FileInfo = GrannyGetFileInfo(InputFiles[FileIdx].GrannyFile);

        granny_file_writer* DiscardWriter =
            GrannyCreateMemoryFileWriter(__FILE__, __LINE__, 1 << 16);

        // Pretend to write out the file, really we're just gathering strings...
        granny_file_builder *Builder = GrannyBeginFile(GrannyStandardSectionCount, GrannyCurrentGRNStandardTag,
                                                       GrannyGRNFileMV_ThisPlatform,
                                                       GrannyGetTemporaryDirectory(),
                                                       "Prefix");
        granny_file_data_tree_writer *Writer =
            GrannyBeginFileDataTreeWriting(GrannyFileInfoType, FileInfo, GrannyStandardDiscardableSection, 0);
        GrannySetFileWriterStringCallback(Writer, GatherStringsCallback,
                                          (void*)&FinalStrings);

        GrannyWriteDataTreeToFileBuilder(Writer, Builder);
        GrannyEndFileDataTreeWriting(Writer);
        GrannyAbortFile(Builder);
    }}


    // Create the final string_database object
    granny_string_database StringDatabase;
    {
        GrannyMakeEmptyDataTypeObject(GrannyStringDatabaseType, &StringDatabase);

        StringDatabase.StringCount = granny_int32(FinalStrings.size());
        StringDatabase.Strings = ArenaPushArray(TempArena, char*,
                                                StringDatabase.StringCount);
        GrannyBeginCRC32(&StringDatabase.DatabaseCRC);

        granny_int32x Current = 0;
        {for (set<string>::iterator itr = FinalStrings.begin();
              itr != FinalStrings.end();
              ++itr)
        {
            StringDatabase.Strings[Current++] = (char*)itr->c_str();

            GrannyAddToCRC32(&StringDatabase.DatabaseCRC,
                             (granny_uint32)itr->length(),
                             itr->c_str());
        }}
        assert(Current == StringDatabase.StringCount);

        // End the crc, and prevent "0" from slipping in.  We collapse that CRC to "1"
        GrannyEndCRC32(&StringDatabase.DatabaseCRC);
        if (StringDatabase.DatabaseCRC == 0)
            StringDatabase.DatabaseCRC = 1;
    }

    // Write out the resulting database...
    granny_file_builder *Builder = GrannyBeginFile(GrannyStandardSectionCount, GrannyCurrentGRNStandardTag,
                                                   GrannyGRNFileMV_ThisPlatform,
                                                   GrannyGetTemporaryDirectory(),
                                                   "Prefix");
    granny_file_data_tree_writer *Writer =
        GrannyBeginFileDataTreeWriting(GrannyStringDatabaseType, &StringDatabase, GrannyStandardDiscardableSection, 0);
    GrannyWriteDataTreeToFileBuilder(Writer, Builder);
    GrannyEndFileDataTreeWriting(Writer);

    bool success = GrannyEndFile(Builder, DatabaseName);
    if (!success)
    {
        fprintf(stderr, "ExtractStrings: failed to write database to %s\n", DatabaseName);
    }

    return success;
}


bool RemapStrings(input_file& InputFile,
                  key_value_pair* KeyValues,
                  granny_int32x   NumKeyValues,
                  granny_memory_arena* TempArena)
{
    char const* DatabaseName = FindFirstValueForKey(KeyValues, NumKeyValues, "database");
    char const* OutputFile = FindFirstValueForKey(KeyValues, NumKeyValues, "output");
    {
        if (DatabaseName == 0)
        {
            fprintf(stderr, "RemapStrings: must specify a database with \"-database <filename>\"\n");
            return false;
        }

        if (OutputFile == 0)
        {
            fprintf(stderr, "RemapStrings: must specify an output with \"-output <filename>\"\n");
            return false;
        }
    }

    // Get the string database
    granny_file* DatabaseFile = GrannyReadEntireFile(DatabaseName);
    if (DatabaseFile == NULL)
    {
        fprintf(stderr, "RemapStrings: couldn't open %s for reading\n", DatabaseName);
        return false;
    }

    granny_string_database* Database = GrannyGetStringDatabase(DatabaseFile);
    {
        if (Database == NULL)
        {
            fprintf(stderr, "RemapStrings: couldn't get a granny_string_database from %s\n", DatabaseName);
            return false;
        }

        if (Database->StringCount == 0)
        {
            fprintf(stderr, "RemapStrings: input Database doesn't appear to be valid (no strings)\n", DatabaseName);
            return false;
        }
        {for(granny_int32 Idx = 2; Idx < Database->StringCount; ++Idx)
        {
            if (strcmp(Database->Strings[Idx-1], Database->Strings[Idx]) >= 0)
            {
                fprintf(stderr, "RemapStrings: input Database doesn't appear to be valid (unsorted entries)\n", DatabaseName);
                return false;
            }
        }}
    }


    // Read the input file...
    granny_file_info* Info = GrannyGetFileInfo(InputFile.GrannyFile);
    if (Info == NULL)
    {
        fprintf(stderr, "RemapStrings: couldn't get a granny_file_info from the input\n");
        return false;
    }


    // Ok.  Now we need to rewrite the input file, preserving sectioning, and remapping
    // the strings as we find them.
    remap_context RemapContext(Database);

    granny_int32x FinalSectionCount = InputFile.GrannyFile->SectionCount;
    if (FinalSectionCount < GrannyStandardSectionCount)
        FinalSectionCount = GrannyStandardSectionCount;

    granny_file_builder *Builder = GrannyBeginFile(FinalSectionCount,
                                                   GrannyCurrentGRNStandardTag,
                                                   InputFile.GrannyFile->SourceMagicValue,
                                                   GrannyGetTemporaryDirectory(),
                                                   "Prefix");
    GrannySetFileStringDatabaseCRC(Builder, Database->DatabaseCRC);

    granny_file_data_tree_writer *Writer =
        GrannyBeginFileDataTreeWriting(GrannyFileInfoType, Info,
                                       GrannyStandardDiscardableSection, 0);
    GrannySetFileWriterStringCallback(Writer, RemapStringsCallback,
                                      (void*)&RemapContext);
    GrannyPreserveFileSectionFormats(Builder, InputFile.GrannyFile);
    GrannyPreserveObjectFileSections(Writer, InputFile.GrannyFile);

    GrannyWriteDataTreeToFileBuilder(Writer, Builder);
    GrannyEndFileDataTreeWriting(Writer);

    bool success = GrannyEndFile(Builder, OutputFile);
    if (success == false)
    {
        fprintf(stderr, "RemapStrings: failed to write to %s\n", OutputFile);
    }
    else if (RemapContext.MissingStringFound)
    {
        fprintf(stderr, "RemapStrings: MISSING STRINGS found in %s, was this file included in the database build?\n", OutputFile);
    }

    return true;
}


// ======================================================================
// Callbacks for gathering and remapping strings...
//
granny_uint32 GatherStringsCallback(void *Data, char const *String)
{
    set<string>* FinalStrings = reinterpret_cast< set<string>* >(Data);

    if (String != NULL)
        FinalStrings->insert(String);
    else
        FinalStrings->insert("");

    // For the gather callback, we don't actually care about the string returned...
    return 0;
}

granny_uint32 RemapStringsCallback(void *Data, char const *String)
{
    remap_context* RemapContext = reinterpret_cast<remap_context*>(Data);

    // Remap nulls to the empty string...
    if (String == NULL)
        String = "";

    // Braindead linear search for right now.  We do ensure the database is
    // sorted, so this could be replaced with a binsearch in the future
    // if we need a speedup.
    {for (granny_int32 i = 0; i < RemapContext->Database->StringCount; ++i)
    {
        if (strcmp(RemapContext->Database->Strings[i], String) == 0)
            return granny_uint32(i);
    }}

    // Oh, very bad.  We don't have that string in our table...
    RemapContext->MissingStringFound = true;
    return 0;
}


