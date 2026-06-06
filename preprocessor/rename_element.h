#if !defined(RENAME_ELEMENT_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/rename_element.h $
// $DateTime: 2007/06/19 12:58:08 $
// $Change: 15228 $
// $Revision: #1 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// RenameElement accepts only one input file, modifies it, and writes it to the name
// specified by the "output" key/value pair.  The idea is to allow you to rename common
// elements of the granny_file_info structure to allow further CombineElement operations
// to create searchable data files.  For instance, if you export 100 subsequences from a
// Max or Maya animation, each of the animation structures will have a Name field that
// matches the filename of the tool file.  (anims.mb, or something along those lines.)
// This obviously presents a problem if you want to take all of the subsequences, and
// combine them into one large file with 100 Animations.  This command allows you to
// prepare the files such that the elements of the combined array will all have
// meaningful, unique names.  Example:
//
//   preprocessor RenameElement run.gr2  -rename Animations[0] -newname Run  -output run.gr2
//   preprocessor RenameElement walk.gr2 -rename Animations[0] -newname Walk -output walk.gr2
//
// Note that the element names are the same as the member names for GrannyFileInfoType.
// (I know that the [0] index notation can be difficult to pass correctly in unix-derived
// shell, sorry about that.)

bool RenameElement(input_file&,
                   key_value_pair*, granny_int32x,
                   granny_memory_arena*);

#define RENAME_ELEMENT_H
#endif
