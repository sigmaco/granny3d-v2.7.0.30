#if !defined(SET_COMPRESSION_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/set_compression.h $
// $DateTime: 2007/02/23 13:10:39 $
// $Change: 14432 $
// $Revision: #5 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// Sets the compression parameters for the file.  To ensure that the
// file is compressed with Oodle, use:
//  preprocessor Compress file.gr2 -output modified.gr2
//
// To decompress, use
//  preprocessor Decompress file.gr2 -output modified.gr2
//
// Note that unlike most of the commands, which output in the native platform of the
// preprocessor executable, the compression functions don't change the platform of the
// input file.  I.e, if the file is optimized for 64-bit big-endian platforms, the
// compressed file will be as well.  This is usually going to be the last item in a
// preprocessor script so we don't want to disturb the intended target.
//
// We are also very careful to maintain the object section placement, so you can call
// these commands after "ResectionFile", or a similar command, and the sectioning will be
// the same as the source file.

bool OodleCompress(input_file&, key_value_pair*, granny_int32x,
                   granny_memory_arena*);
bool Decompress(input_file&, key_value_pair*, granny_int32x,
                granny_memory_arena*);

#define SET_COMPRESSION_H
#endif
