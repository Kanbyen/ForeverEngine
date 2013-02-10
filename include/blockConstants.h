/*****************************************************************************************

Forever War - a NetHack-like FPS

Copyright (C) 2008 Thomas Schöps

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program;
if not, see <http://www.gnu.org/licenses/>.

*****************************************************************************************/

#ifndef _BLOCK_CONSTANTS_H_
#define _BLOCK_CONSTANTS_H_

#include "config.h"

/// BLOCK constants
#if defined( DIGISPE )
const int BLOCK_SIDE_LENGTH = 32;
#else
const int BLOCK_SIDE_LENGTH = 16;
#endif
const int BLOCK_SIDE_LENGTH_PLUS_1 = BLOCK_SIDE_LENGTH + 1;
const int NUM_VOXELS_IN_BLOCK = BLOCK_SIDE_LENGTH * BLOCK_SIDE_LENGTH * BLOCK_SIDE_LENGTH;
const int TEMP_INDEX_FIELD_SIZE = BLOCK_SIDE_LENGTH_PLUS_1 * BLOCK_SIDE_LENGTH_PLUS_1 * BLOCK_SIDE_LENGTH_PLUS_1 * 8;

/// VOLUME constants
const int VOLUME_SIDE_LENGTH = 18;	// was: 22
const int VOLUME_SIDE_LENGTH_IN_VOXELS = VOLUME_SIDE_LENGTH * BLOCK_SIDE_LENGTH;
const int VOLUME_RADIUS = (VOLUME_SIDE_LENGTH - 1) / 2;
const int VOLUME_RADIUS_SQ = VOLUME_RADIUS * VOLUME_RADIUS;
const int VOLUME_NUM_BLOCKS = VOLUME_SIDE_LENGTH * VOLUME_SIDE_LENGTH * VOLUME_SIDE_LENGTH;

/// Return values for the voxel load function
const int RETURN_NORMAL_BLOCK = 0;
const int RETURN_FULL_BLOCK = 1;
const int RETURN_EMPTY_BLOCK = 2;
/// Buffer size for block loading
const int BLOCKBUFFER_SIDE_LENGTH = BLOCK_SIDE_LENGTH+2;
const int BLOCKBUFFER_SIZE = BLOCKBUFFER_SIDE_LENGTH * BLOCKBUFFER_SIDE_LENGTH * BLOCKBUFFER_SIDE_LENGTH;

/// How far the camera has to travel from the last point where the volume window was set until the volume window is set again
const float PAGING_RELOAD_DISTANCE = 14.0f;
const float PAGING_RELOAD_DISTANCE_SQ = PAGING_RELOAD_DISTANCE * PAGING_RELOAD_DISTANCE;

/// All voxels in an empty block have this value
const char EMPTY_BLOCK_VALUE = -100;

/// If all voxels of a block and the neighbouring voxels in +X, +Y, +Z direction are less or equal to this value, the block is considered empty
const char EMPTY_BLOCK_THRESHOLD = -64;

/// All voxels in an full block have this value
const char FULL_BLOCK_VALUE = 100;

/// If all voxels of a block and the neighbouring voxels in +X, +Y, +Z direction are greater or equal to this value, the block is considered full
const char FULL_BLOCK_THRESHOLD = 64;

#endif
