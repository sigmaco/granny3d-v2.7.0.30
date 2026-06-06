#if !defined(CLEAN_MATERIALS_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/clean_materials.h $
// $DateTime: 2007/02/23 13:10:39 $
// $Change: 14432 $
// $Revision: #4 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// CleanMaterials runs through the input file, looking for materials that aren't
// referenced by any mesh (or another material, in the case of graph structured exports),
// and removes them from the file.  Since the extended data of materials can get quite
// large, this can be a significant space win for files exported without careful attention
// to which materials are actually in use by the file.
//
// Important!  The filter process doesn't look at the ExtendedData on either the materials
// or the mesh, so if you have files that store material references there, you should
// alter this command to traverse those structures.

bool CleanMaterials(input_file&, key_value_pair*, granny_int32x,
                    granny_memory_arena*);


#define CLEAN_MATERIALS_H
#endif
