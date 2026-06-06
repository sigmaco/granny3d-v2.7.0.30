#if !defined(PLATFORM_CONVERT_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/platform_convert.h $
// $DateTime: 2007/02/23 13:10:39 $
// $Change: 14432 $
// $Revision: #4 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// PlatformConvert accepts only one input file, modifies it, and
// writes it to the name specified by the "output" key/value pair.
// The function looks for -pointer and -endian pairs, writes out the
// corresponding format.  For instance, to convert to
// 32-bit/big-endian format:
//
//   preprocessor PlatformConvert file.gr2 -pointer 32 -endian big -output modified.gr2
//
// The file is now optimized for loading on platforms like X360, PS3,
// and MacPPC.

bool PlatformConvert(input_file&, key_value_pair*, granny_int32x,
                     granny_memory_arena*);

#define PLATFORM_CONVERT_H
#endif
