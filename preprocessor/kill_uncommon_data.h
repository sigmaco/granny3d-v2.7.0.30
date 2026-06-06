#if !defined(KILL_UNCOMMON_DATA_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/kill_uncommon_data.h $
// $DateTime: 2007/02/23 13:10:39 $
// $Change: 14432 $
// $Revision: #4 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// KillUncommonData removes data like the vertex annotations, extended
// data on the bones, etc that is usually useful in toolchains, but
// that you don't need for your runtime.  Note that we're very "broad
// brush" here, if you need some of the data to hang around, just
// modify the function to keep the bits you need

bool KillUncommonData(input_file&, key_value_pair*, granny_int32x,
                      granny_memory_arena*);

#define KILL_UNCOMMON_DATA_H
#endif
