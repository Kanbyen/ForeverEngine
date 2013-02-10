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
#include "blockFull.h"
#include "blockLoaded.h"
#include "volume.h"

VoxelBlockFull::VoxelBlockFull(int blockX, int blockY, int blockZ, VoxelVolume* volume, VoxelBlockPersistent* persistent, VoxelBlockLoaded* texSource)
	: VoxelBlock(blockX, blockY, blockZ, volume, persistent)
{
	if (texSource)
		memcpy(texture, texSource->texture, NUM_VOXELS_IN_BLOCK);
	volume->numFullBlocks++;
}

VoxelBlockFull::~VoxelBlockFull()
{
	volume->numFullBlocks--;
}

void VoxelBlockFull::fill(char value)
{
	if (value < FULL_BLOCK_THRESHOLD)
	{
		VoxelBlockLoaded* newBlock = new VoxelBlockLoaded(position.x, position.y, position.z, volume, persistent);
		newBlock->setObjectsLoaded(objectsLoaded);
		newBlock->setCustomChanges(customChanges);
		newBlock->fill(value);
		memcpy(newBlock->texture, texture, NUM_VOXELS_IN_BLOCK);
		volume->setBlockAbs(position.x, position.y, position.z, newBlock);
	}
}

char VoxelBlockFull::getVoxelAt(int xPosition, int yPosition, int zPosition)
{
	return FULL_BLOCK_VALUE;
}
void VoxelBlockFull::setVoxelAt(int xPosition, int yPosition, int zPosition, const char value)
{
	if (value < FULL_BLOCK_THRESHOLD)
	{
		VoxelBlockLoaded* newBlock = new VoxelBlockLoaded(position.x, position.y, position.z, volume, persistent);
		newBlock->setObjectsLoaded(objectsLoaded);
		newBlock->setCustomChanges(customChanges);
		newBlock->fill(FULL_BLOCK_VALUE);
		newBlock->setVoxelAt(xPosition, yPosition, zPosition, value);
		memcpy(newBlock->texture, texture, NUM_VOXELS_IN_BLOCK);
		volume->setBlockAbs(position.x, position.y, position.z, newBlock);
	}
}
