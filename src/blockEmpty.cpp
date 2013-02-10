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
#include "blockEmpty.h"
#include "blockLoaded.h"
#include "volume.h"

VoxelBlockEmpty::VoxelBlockEmpty(int blockX, int blockY, int blockZ, VoxelVolume* volume, VoxelBlockPersistent* persistent, float texture)
	: VoxelBlock(blockX, blockY, blockZ, volume, persistent)
{
	this->texture = 255;	// TODO: Remove texture variable?
	//this->texture = texture;

	volume->numEmptyBlocks++;
}

VoxelBlockEmpty::~VoxelBlockEmpty()
{
	volume->numEmptyBlocks--;
}

void VoxelBlockEmpty::fill(char value)
{
	if (value > EMPTY_BLOCK_THRESHOLD)
	{
		VoxelBlockLoaded* newBlock = new VoxelBlockLoaded(position.x, position.y, position.z, volume, persistent);
		newBlock->setObjectsLoaded(objectsLoaded);
		newBlock->setCustomChanges(customChanges);
		newBlock->fill(value);
		newBlock->fillTex(texture);
		volume->setBlockAbs(position.x, position.y, position.z, newBlock);
	}
}

char VoxelBlockEmpty::getVoxelAt(int xPosition, int yPosition, int zPosition)
{
	return EMPTY_BLOCK_VALUE;
}
void VoxelBlockEmpty::setVoxelAt(int xPosition, int yPosition, int zPosition, const char value)
{
	if (value > EMPTY_BLOCK_THRESHOLD)
	{
		VoxelBlockLoaded* newBlock = new VoxelBlockLoaded(position.x, position.y, position.z, volume, persistent);
		newBlock->setObjectsLoaded(objectsLoaded);
		newBlock->setCustomChanges(customChanges);
		newBlock->fill(EMPTY_BLOCK_VALUE);
		newBlock->setVoxelAt(xPosition, yPosition, zPosition, value);
		newBlock->fillTex(texture);
		volume->setBlockAbs(position.x, position.y, position.z, newBlock);
	}
}

unsigned char VoxelBlockEmpty::getTextureAt(int xPosition, int yPosition, int zPosition)
{
	return texture;
}
void VoxelBlockEmpty::setTextureAt(int xPosition, int yPosition, int zPosition, const unsigned char value)
{
	return;
}
