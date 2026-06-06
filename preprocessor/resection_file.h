#if !defined(RESECTION_FILE_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/resection_file.h $
// $DateTime: 2007/02/23 13:10:39 $
// $Change: 14432 $
// $Revision: #3 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// Many of the preprocessor commands create output files with a single section.  This
// simplifies the code to create the processed file, but destroys your ability to
// selectively release components that are only useful at setup time.  (Vertex data,
// texture pixels, etc.)  ResectionFile will restructure the file to have the same default
// layout that the exporter creates.  At present, this is not overridable, though you
// could certainly create a custom version of this command that changes the section
// layout.  For instance, while we create 8 sections by default, you might only care to
// create 2, permanent and temporary.

bool ResectionFile(input_file&, key_value_pair*, granny_int32x,
                   granny_memory_arena*);

#define RESECTION_FILE_H
#endif
