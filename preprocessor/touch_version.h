#if !defined(TOUCH_VERSION_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/touch_version.h $
// $DateTime: 2007/04/23 13:13:35 $
// $Change: 14832 $
// $Revision: #1 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// TouchVersion just refreshes the version of the specified file.  Very
// useful for older files, since it allows Granny to load them at runtime
// with no conversion.

bool TouchVersion(input_file&, key_value_pair*, granny_int32x,
                  granny_memory_arena*);

#define TOUCH_VERSION_H
#endif
