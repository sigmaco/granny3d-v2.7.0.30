#if !defined(REMOVE_UNCHANGING_TRACKS_H)
// ========================================================================
// $File: //jeffr/granny/preprocessor/remove_unchanging_tracks.h $
// $DateTime: 2007/07/05 15:59:23 $
// $Change: 15424 $
// $Revision: #1 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#include "preprocessor.h"

// RemoveUnchangingTracks simulates the behavior of turning off the (removed) exporter
// option "Store Unchanging Tracks".  This has become a preprocessor command rather than a
// builtin option since the exporter doesn't have enough information to properly determine
// which tracks are "unchanging" versus "unchanging and the same as the base pose".  Only
// the latter tracks can be safely removed without changing the meaning of the animation.
//
// Use the command as follows.  We'll assume that the models for the animations are stored
// in model.gr2, with the animation to be altered in anim.gr2.
//
//  preprocessor RemoveUnchangingTracks model.gr2 anim.gr2 -output stripped_anim.gr2

bool RemoveUnchangingTracks(input_file*, granny_int32x,
                            key_value_pair*, granny_int32x,
                            granny_memory_arena*);

#define REMOVE_UNCHANGING_TRACKS_H
#endif
