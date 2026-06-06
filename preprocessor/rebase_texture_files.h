#if !defined(REBASE_TEXTURE_FILES_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/rebase_texture_files.h $
// $DateTime: 2007/11/26 12:10:16 $
// $Change: 16607 $
// $Revision: #1 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "preprocessor.h"

// RebaseTextureFiles looks at the image names specified in the granny_texture, and tries
// to make that path relative to the supplied -basepath
//
// Todo: .. paths?

bool RebaseTextureFiles(input_file&, key_value_pair*, granny_int32x,
                        granny_memory_arena*);

#define REBASE_TEXTURE_FILES_H
#endif
