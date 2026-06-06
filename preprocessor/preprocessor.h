#if !defined(PREPROCESSOR_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/preprocessor.h $
// $DateTime: 2007/02/23 13:10:39 $
// $Change: 14432 $
// $Revision: #7 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "granny.h"

// ------------------------------------------------------------
// Structures to hold arguments to the commands
struct input_file
{
    granny_file* GrannyFile;
    char*        Filename;   // as passed to the command line, may
                             // contain path elements
};

struct key_value_pair
{
    char* Key;
    char* Value;
};

// ------------------------------------------------------------
// Function signature for processing functions
enum ECommandType
{
    eMultiFile,
    eSingleFile_NoMultiRun,
    eSingleFile_MultiRun,
};

typedef bool (*CommandMulti)(input_file*     InputFiles,
                             granny_int32x   NumInputFiles,
                             key_value_pair* KeyValuePairs,
                             granny_int32x   NumKeyValuePairs,
                             granny_memory_arena* TempArena);

typedef bool (*CommandSingle)(input_file&     InputFile,
                              key_value_pair* KeyValuePairs,
                              granny_int32x   NumKeyValuePairs,
                              granny_memory_arena* TempArena);


// ------------------------------------------------------------
// Convenience macros for the memory arena
#define ArenaPushObject(Arena, Type)            \
    ((Type*)GrannyMemoryArenaPush(Arena, sizeof(Type)))

#define ArenaPushArray(Arena, Type, Count) \
    ((Type*)GrannyMemoryArenaPush(Arena, sizeof(Type) * Count))


// ------------------------------------------------------------
// Helper structure to register a preprocessor command.  Use as:
//  static CommandRegistrar RegMyCommand(eSingleFile_MultiRun,  /* or eMultiFile */
//                                       MyCommand,
//                                       "CommandString",
//                                       "ShortDocString");
// at /file/ level, where MyCommand is a pointer to your function.
class CommandRegistrar
{
public:

public:
    CommandRegistrar(ECommandType CommandType,
                     CommandMulti NewCommand,
                     char const*  CommandName,
                     char const*  ShortDocString);

    CommandRegistrar(ECommandType  CommandType,
                     CommandSingle NewCommand,
                     char const*   CommandName,
                     char const*   ShortDocString);

    // disallow standard c++ fns
private:
    CommandRegistrar();
    CommandRegistrar(CommandRegistrar const&);
    void operator=(CommandRegistrar const&);
};


// Utility function for searching K/V pairs.  Returns NULL for missing
// key/value
char const* FindFirstValueForKey(key_value_pair* KeyValuePairs,
                                 granny_int32x   NumKeyValuePairs,
                                 char const*     Key);


// Macosx has strcasecmp rather than stricmp
#ifdef _MACOSX
#define _stricmp strcasecmp
#endif


#define PREPROCESSOR_H
#endif
