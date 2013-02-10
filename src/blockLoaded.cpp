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

#include "precompiled.h"
#include "blockLoaded.h"
#include "volume.h"

VoxelBlockLoaded::VoxelBlockLoaded(int blockX, int blockY, int blockZ, VoxelVolume* volume, VoxelBlockPersistent* persistent)
	: VoxelBlock(blockX, blockY, blockZ, volume, persistent)
{
	volume->numLoadedBlocks++;
}

VoxelBlockLoaded::~VoxelBlockLoaded()
{
	volume->numLoadedBlocks--;
}

void VoxelBlockLoaded::fill(char value)
{
	memset(data, value, NUM_VOXELS_IN_BLOCK);
}

void VoxelBlockLoaded::fillTex(unsigned char value)
{
	memset(texture, value, NUM_VOXELS_IN_BLOCK);
}
