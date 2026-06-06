#if !defined(BONES_PER_MESH_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/bones_per_mesh.h $
// $DateTime: 2007/02/23 13:10:39 $
// $Change: 14432 $
// $Revision: #4 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// BonesPerMesh breaks up any meshes that have more than "-bonelimit
// N" bones in their binding.  This allows you to render models with
// more bones than fit in the constant registers.  (Or the equivalent
// on your platform)

bool BonesPerMesh(input_file&     InputFile,
                  key_value_pair* KeyValues,
                  granny_int32x   NumKeyValues,
                  granny_memory_arena* TempArena);

#define BONES_PER_MESH_H
#endif
