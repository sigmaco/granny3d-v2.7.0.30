#if !defined(WARN_SCALE_SHEAR_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/warn_scale_shear.h $
// $DateTime: 2007/02/23 13:10:39 $
// $Change: 14432 $
// $Revision: #4 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// WarnScaleShear scans skeletons and animations, looking for
// non-identity scale/shear.  Since some engines can't render models
// with scale/shear, it can be troublesome if files containing
// constant, or even animated scale slips into the datastream.  When
// found, it will issue a warning idenitifying the component.
//
// Note that WarnScaleShear can accept any number of input files, and
// produces no output, so the -output key is unnecessary.  For multiple files, you simply call as:
//
//  preprocessor WarnScaleShear file1.gr2 file2.gr2 ...
//
// Todo: possible allow constant scale, or scale with no shear based
// on input arguments

bool WarnScaleShear(input_file&, key_value_pair*, granny_int32x,
                    granny_memory_arena*);

#define WARN_SCALE_SHEAR_H
#endif
