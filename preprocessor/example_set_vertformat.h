#if !defined(EXAMPLE_SET_VERTFORMAT_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/example_set_vertformat.h $
// $DateTime: 2007/02/23 13:10:39 $
// $Change: 14432 $
// $Revision: #4 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// Converts any mesh that is skinned to the granny_pwnt3232_vertex
// format.  Rigid meshes are not affected.  This command takes a
// single input file, and requires that the "output" key/value pair be
// present.  At the moment, there are no other recognized parameters.
//
// To use:
//  preprocessor ExampleSetVertFormat file.gr2 -output modified.gr2

bool ExampleSetVertFormat(input_file&, key_value_pair*, granny_int32x,
                          granny_memory_arena*);

#define EXAMPLE_SET_VERTFORMAT_H
#endif
