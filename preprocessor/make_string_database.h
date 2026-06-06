#if !defined(MAKE_STRING_DATABASE_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/make_string_database.h $
// $DateTime: 2007/03/05 11:02:38 $
// $Change: 14500 $
// $Revision: #1 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "preprocessor.h"

// These two commands facilitate the use of granny_string_database objects to factor out
// common strings in multiple Granny files.  This is designed to be used as a two-pass
// lookup, so the procedure is:
//
// for all Files
//  preprocessor ExtractStrings <File> -database foo.gsd
//
// for all Files
//  preprocessor RemapStrings <File> -database foo.gsd -output <RemappedFile>
//
// ExtractStrings can accept any number of files in the <File> argument, so you can
// simplify this by passing multiple files to that command if you desire.  Because
// RemapStrings accepts an -output argument, rather than an argument that specifys a
// filename transformation, it accepts only a single file, and must be run once per input
// file.
//
// You should read the "File IO String Replacement" tutorial for more details on the
// specifics of using .gr2 files with externally stored strings, but the basic idea is
// that ExtractStrings builds up an object of type granny_string_database, and stores it
// in Granny's file format.  Once you've loaded the database file, you obtain a pointer to
// that object with GrannyGetStringDatabase(file*), which is analogous to
// GrannyGetFileInfo().  That database is then specified in the string replacement
// callback, which allows Granny's file loader to replace strings pointers in the remapped
// files with the appropriate entries in the shared database.
//
// Note that nothing prevents you from having multiple string databases for different
// groups of files in your application.  You simply need to ensure that the correct
// database is specified during the load step, and that the database granny_file* is not
// deleted while other granny_file*'s exist that depend on those strings.

bool ExtractStrings(input_file*, granny_int32x,
                    key_value_pair*, granny_int32x,
                    granny_memory_arena*);

bool RemapStrings(input_file&,
                  key_value_pair*, granny_int32x,
                  granny_memory_arena*);


#define MAKE_STRING_DATABASE_H
#endif
