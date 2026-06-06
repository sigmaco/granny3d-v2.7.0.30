#if !defined(REMOVE_ELEMENTS_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/remove_elements.h $
// $DateTime: 2007/02/23 13:10:39 $
// $Change: 14432 $
// $Revision: #4 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// RemoveElements accepts only one input file, modifies it, and writes
// it to the name specified by the "output" key/value pair.  Each
// key/value pair that matches: "-remove ElementName" will cause the
// named element to be removed from the output GrannyFile.  So for
// instance, you can create a version of a file with the materials and
// textures removed by executing:
//
//   preprocessor RemoveElements file.gr2 -remove Materials -remove Textures -output modified.gr2
//
// Note that the element names are the same as the member names for
// GrannyFileInfoType.

bool RemoveElements(input_file&, key_value_pair*, granny_int32x,
                    granny_memory_arena*);

#define REMOVE_ELEMENTS_H
#endif
