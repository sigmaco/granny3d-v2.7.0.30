// ========================================================================
// $File: //jeffr/granny/preprocessor/preprocessor.cpp $
// $DateTime: 2007/08/03 14:24:40 $
// $Change: 15658 $
// $Revision: #10 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
//
// Driver for the Granny Preprocessor
//
//   The idea here is to separate the process of parsing and
// validating arguments, loading source data, and dispatching the
// command from actually processing the meshes.  To register a custom
// command, simply create a new function of the PreprocessorCommand
// type, and register it with a unique string in the command table.
//
//   We recommend that you place your new commands in separate source
// files modules, rather than modifying the distribution files in-place,
// since this will ease upgrades if modifications are made to the
// preprocessor.
// =============================================================================
#define _CRT_SECURE_NO_DEPRECATE 1  // MSVC shush

#include "preprocessor.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;


// The current table of commands.  These are created by the CommandRegistrar helper class.
struct command_entry
{
    string       CommandName;
    string       ShortDocString;
    ECommandType CommandType;

    union
    {
        CommandMulti  MultiFn;
        CommandSingle SingleFn;
    };

    // for sorting
    bool operator<(command_entry const& Other) const
    {
        return _stricmp(CommandName.c_str(), Other.CommandName.c_str()) < 0;
    }
};

vector<command_entry>& GlobalCommandList()
{
    static vector<command_entry> CommandList;

    return CommandList;
}



int FindCommandEntry(char const* CommandName)
{
    for (int i = 0; i < int(GlobalCommandList().size()); ++i)
    {
        if (_stricmp(GlobalCommandList()[i].CommandName.c_str(), CommandName) == 0)
        {
            return i;
        }
    }

    return -1;
}




//     // TODO
//     // { "RemoveTrackgroupUnderError" }
//     // { "RemoveUnusedTrackgroups" }
//     // { "RemoveBonesNotAffectingAVert" }
//     // { "GenerateMipMaps" }
//     // { "LimitTextureSize" }
//     // { "SetTextureFormat" }
//     //   PSP
//     //   { "PalettizeTextures" }
//     //   { "CompressedVerts" }


void usage()
{
    printf(" Granny Preprocessor (compiled with Granny v%s)\n"
           " usage:\n"
           "   preprocessor <command> [<input0.gr2> <input1.gr2> ...] [-key value ...]\n"
           "\n"
           "  Where <command> is a supported preprocessor operation, input*.gr2 are\n"
           "  Granny files (any referenced input files must exist).  [-key value]\n"
           "  pairs are passed to the <command> operation as supplimental arguments.\n"
           "\n"
           "  Note that you may use the special command \"ListCommands\" to get a list\n"
           "  of the commands supported by this version of the preprocessor.\n"
           "\n"
           " example:\n"
           "   preprocessor MeshBoneLimit mesh.gr2 -output proc_mesh.gr2 -bonelimit 50\n\n",
           GrannyProductVersion);
}


bool ListCommands(input_file*, granny_int32x,
                  key_value_pair*, granny_int32x,
                  granny_memory_arena*)
{
    size_t max_len = 0;
    {for(int i = 0; i < int(GlobalCommandList().size()); ++i)
    {
        size_t len = GlobalCommandList()[i].CommandName.length();
        if (len > max_len)
            max_len = len;
    }}

    {for(int i = 0; i < int(GlobalCommandList().size()); ++i)
    {
        printf(" %-*s : %s\n",
               (int)max_len,
               GlobalCommandList()[i].CommandName.c_str(),
               GlobalCommandList()[i].ShortDocString.c_str());
    }}

    return true;
}

static CommandRegistrar RegListCommands(eMultiFile,
                                        ListCommands,
                                        "ListCommands",
                                        "Lists all of the commands supported by the preprocessor");

char const* FindFirstValueForKey(key_value_pair* KeyValuePairs,
                                 granny_int32x   NumKeyValuePairs,
                                 char const*     Key)
{
    if (Key == 0)
        return 0;

    {for(granny_int32x i = 0; i < NumKeyValuePairs; ++i)
    {
        if (_stricmp(KeyValuePairs[i].Key, Key) == 0)
            return KeyValuePairs[i].Value;
    }}

    return 0;
}


int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        usage();
        return EXIT_FAILURE;
    }

#ifdef _MACOSX
    // On MacOS X, we need to replace the default file reader creation function with the
    // ansi version that uses fopen/fread, rather than the FSOpen API.
    GrannySetDefaultFileReaderOpenCallback(GrannyGetOSXAnsiFileReaderOpenCallback());
#endif

    // Command string is the first argument
    int CommandIndex = FindCommandEntry(argv[1]);
    if (CommandIndex == -1)
    {
        fprintf(stderr,
                "Misunderstood command: %s.  Use \"preprocessor ListCommands\"\n"
                " to obtain a list of valid command strings\n",
                argv[1]);
        return EXIT_FAILURE;
    }
    command_entry& CommandEntry = GlobalCommandList()[CommandIndex];

    // Parse the input meshes, and the key value pairs.  We're going
    // to allocate too much temp space here, but this way we don't
    // have to grow the arrays.  Maximum number of input meshes is
    // (argc-2), max key/value pairs is (argc-2)/2.
    input_file*  InputFiles     = new input_file[argc - 2];
    granny_int32x NumInputFiles = 0;

    key_value_pair* KeyValuePairs = new key_value_pair[argc - 2];
    granny_int32x   NumKeyValuePairs = 0;

    {for(int i = 2; i < argc; /*explicit increment in the loop*/)
    {
        if (argv[i][0] == '-')
        {
            // Key value pair
            if (i > argc-2)
            {
                // improper k/v, not enough arguments
                fprintf(stderr, "Bad argument list, unmatched key, must be followed by value (%s)\n",
                        argv[i]);
                return EXIT_FAILURE;
            }
            else
            {
                // proper k/v, create a pair object
                KeyValuePairs[NumKeyValuePairs].Key   = new char[strlen(argv[i])]; // ignore the '-'
                KeyValuePairs[NumKeyValuePairs].Value = new char[strlen(argv[i+1]) + 1];

                strcpy(KeyValuePairs[NumKeyValuePairs].Key, argv[i]+1);
                strcpy(KeyValuePairs[NumKeyValuePairs].Value, argv[i+1]);
                ++NumKeyValuePairs;
            }

            i += 2;
        }
        else
        {
            // Input file
            InputFiles[NumInputFiles].GrannyFile = GrannyReadEntireFile(argv[i]);
            InputFiles[NumInputFiles].Filename   = new char[strlen(argv[i]) + 1];
            strcpy(InputFiles[NumInputFiles].Filename, argv[i]);

            if (InputFiles[NumInputFiles].GrannyFile == 0)
            {
                fprintf(stderr, "Unable to load granny file: %s\n", argv[i]);
                return EXIT_FAILURE;
            }

            ++NumInputFiles;
            i += 1;
        }
    }}


    // Dispatch to the specified function...
    bool Success;
    if (CommandEntry.CommandType == eMultiFile)
    {
        granny_memory_arena* TempArena = GrannyNewMemoryArena();
        GrannySetArenaAlignment(TempArena, 16);
        Success = (*CommandEntry.MultiFn)(InputFiles, NumInputFiles,
                                          KeyValuePairs, NumKeyValuePairs,
                                          TempArena);
        GrannyFreeMemoryArena(TempArena);
    }
    else
    {
        assert(CommandEntry.CommandType == eSingleFile_NoMultiRun ||
               CommandEntry.CommandType == eSingleFile_MultiRun);

        if (CommandEntry.CommandType == eSingleFile_NoMultiRun &&
            NumInputFiles != 1)
        {
            fprintf(stderr,
                    "%s is a single file command that doesn't support multiple\n"
                    "invocations.  Please specify only one input file\n",
                    CommandEntry.CommandName.c_str());
            return EXIT_FAILURE;
        }

        Success = true;
        {for(int i = 0; i < NumInputFiles; ++i)
        {
            granny_memory_arena* TempArena = GrannyNewMemoryArena();

            if ((*CommandEntry.SingleFn)(InputFiles[i],
                                         KeyValuePairs,
                                         NumKeyValuePairs,
                                         TempArena) == false)
            {
                GrannyFreeMemoryArena(TempArena);
                fprintf(stderr, "%s failed on input file: %s\n",
                        CommandEntry.CommandName.c_str(),
                        InputFiles[i].Filename);
                Success = false;
                break;
            }

            GrannyFreeMemoryArena(TempArena);
        }}
    }

    return Success ? EXIT_SUCCESS : EXIT_FAILURE;
}


CommandRegistrar::CommandRegistrar(ECommandType CommandType,
                                   CommandMulti NewCommand,
                                   char const*  CommandName,
                                   char const*  ShortDocString)
{
    assert(CommandType == eMultiFile);
    assert(FindCommandEntry(CommandName) == -1);

    command_entry CommandEntry;
    CommandEntry.CommandName = CommandName;
    CommandEntry.ShortDocString = ShortDocString;
    CommandEntry.CommandType = CommandType;
    CommandEntry.MultiFn = NewCommand;
    GlobalCommandList().push_back(CommandEntry);

    // Inefficient to sort this every time, but what the hey
    sort(GlobalCommandList().begin(), GlobalCommandList().end());
}

CommandRegistrar::CommandRegistrar(ECommandType  CommandType,
                                   CommandSingle NewCommand,
                                   char const*   CommandName,
                                   char const*   ShortDocString)
{
    assert(CommandType == eSingleFile_NoMultiRun ||
           CommandType == eSingleFile_MultiRun);
    assert(FindCommandEntry(CommandName) == -1);

    command_entry CommandEntry;
    CommandEntry.CommandName = CommandName;
    CommandEntry.ShortDocString = ShortDocString;
    CommandEntry.CommandType = CommandType;
    CommandEntry.SingleFn = NewCommand;
    GlobalCommandList().push_back(CommandEntry);

    // Inefficient to sort this every time, but what the hey
    sort(GlobalCommandList().begin(), GlobalCommandList().end());
}
