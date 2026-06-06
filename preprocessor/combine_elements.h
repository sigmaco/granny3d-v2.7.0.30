#if !defined(COMBINE_ELEMENTS_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/combine_elements.h $
// $DateTime: 2006/10/26 17:53:54 $
// $Change: 13678 $
// $Revision: #3 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// CombineElements accepts any number of input files, and creates an
// output file that contains all of the elements of the first input
// file, along with those elements from the secondary files specified
// by "combine" key/value pairs.  (This only works for array elements,
// obviously, like "Textures", or "Animations".  We can't combine
// objects or strings like "ExporterInfo".  This is extremely handy
// for coalescing different Granny files that are part of the same
// asset into a single loadable unit.  For instance, if you had a
// character, represented by 3 .gr2 files:
//  - character.gr2 (model, mesh, textures)
//  - run_anim.gr2
//  - walk_anim.gr2
//
// You might want to be able to load all of the character assets at
// once, as combined_char.gr2, so you would execute:
//
//  preprocessor CombineElements character.gr2 run_anim.gr2 walk_anim.gr2 -output combined_char.gr2 -combine Animations
//
// Note that the element names are the same as the member names for
// GrannyFileInfoType.
//
// Also note that this function will happily combine elements that may
// not belong in the same file.  For instance, if a granny_texture
// exists in 0.gr2 and 1.gr2, the combine function will put two copies
// of the texture in the result file if you ask it to -combine
// "Textures".  If you need to add filtering functions to prevent this
// sort of thing (like the texture database commands do), use this
// code as a jumping off point to write your own combination function.
//
// Last note, promise!  Subobject pointers such as the "Mesh" member
// of a granny_model's MeshBinding field won't be updated by any
// combination operations.  So if you combine the models, you should
// probably combine meshes as well.  Again, complicated filtering can
// be added in your own functions.

bool CombineElements(input_file*, granny_int32x, key_value_pair*, granny_int32x,
                     granny_memory_arena*);

#define COMBINE_ELEMENTS_H
#endif
