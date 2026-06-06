#if !defined(VERTEX_CACHE_OPTIMIZE_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/vertex_cache_optimize.h $
// $DateTime: 2007/02/23 13:10:39 $
// $Change: 14432 $
// $Revision: #4 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// VertexCacheOptimize lays out the meshes in a render order that is
// efficient across a wide array of vertex cache architectures.  If
// your platform's vertex cache size is known with certainty, you may
// be able to get slightly better performance by replacing this
// algorithm with a custom version.
//
// Note that this will reorder the vertices in order to help out with
// the pretransform cache as well, so any vertex annotations present
// are destroyed by this function.

bool VertexCacheOptimize(input_file&, key_value_pair*, granny_int32x,
                         granny_memory_arena*);

#define VERTEX_CACHE_OPTIMIZE_H
#endif
