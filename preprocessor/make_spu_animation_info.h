#if !defined(MAKE_SPU_ANIMATION_INFO_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/make_spu_animation_info.h $
// $DateTime: 2007/08/15 15:47:14 $
// $Change: 15741 $
// $Revision: #3 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

bool MakeSPUAnimation(input_file&     InputFile,
                      key_value_pair* KeyValues,
                      granny_int32x   NumKeyValues,
                      granny_memory_arena* Arena);


#define MAKE_SPU_ANIMATION_INFO_H
#endif
