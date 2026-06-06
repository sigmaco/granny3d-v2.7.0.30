#if !defined(TRANSFORM_FILE_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/transform_file.h $
// $DateTime: 2007/02/23 13:10:39 $
// $Change: 14432 $
// $Revision: #4 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// TransformFile uses GrannyComputeBasisTransform and GrannyTransform
// file to move the input file into a specified coordinate system.  By
// default, if you call this with no arguments, the coordinate system
// used is 1 unit per meter, with Y-up, X-right, and Z-back.  You can
// control the coordinate system with the following arguments.  (The
// default coordinate system is shown.)
//
// -unitsPerMeter 1
// -up Y
// -right X
// -back Z
//
// Valid options for the up/right/back arguments are:
//  - X      = (1, 0, 0)
//  - Y      = (0, 1, 0)
//  - Z      = (0, 0, 1)
//  - NegX   = (-1, 0, 0)
//  - NegY   = (0, -1, 0)
//  - NegZ   = (0, 0, -1)
//
// TransformFile will remove the ArtToolInfo structure from the output
// file, since presumably it's no longer required.  We don't currently
// support moving a file's origin, but this would be straightforward
// to add to the command.

bool TransformFile(input_file&, key_value_pair*, granny_int32x,
                   granny_memory_arena*);

#define TRANSFORM_FILE_H
#endif
