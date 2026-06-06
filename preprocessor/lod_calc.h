#if !defined(LOD_CALC_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/lod_calc.h $
// $DateTime: 2006/10/26 17:53:54 $
// $Change: 13678 $
// $Revision: #3 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// LODCalc computes track_group LOD values for animations, given a
// model to bind to.  This is a fairly time consuming process, since
// the animation must be sampled many, many times, and the bounding
// boxes of the verts affected by the bones transformed for each
// possible combination of bone removal.  The form for the command is:
//
//  preprocessor LODCalc <ModelFile>.gr2 <AnimFile>.gr2 -output AnimOutput.gr2
//
// For now, this command processes only one animation at a time, but
// it would be easy to modify to support any number of animations.
// Note that we assume that ModelFile and AnimFile are in the same
// coordinate system, we do /not/ GrannyTransformFile on input.

bool LODCalc(input_file*, granny_int32x, key_value_pair*, granny_int32x,
             granny_memory_arena*);

#define LOD_CALC_H
#endif
