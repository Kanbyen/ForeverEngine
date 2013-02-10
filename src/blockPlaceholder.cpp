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
#include "blockPlaceholder.h"

VoxelBlockPlaceholder::VoxelBlockPlaceholder(int blockX, int blockY, int blockZ, VoxelVolume* volume, VoxelBlockPersistent* persistent)
	: VoxelBlock(blockX, blockY, blockZ, volume, persistent)
{
	dataChanged = false;
	isBeingLoaded = false;
}

VoxelBlockPlaceholder::~VoxelBlockPlaceholder()
{
}

void VoxelBlockPlaceholder::fill(char value)
{
}

char VoxelBlockPlaceholder::getVoxelAt(int xPosition, int yPosition, int zPosition)
{
	return FULL_BLOCK_VALUE;
}
void VoxelBlockPlaceholder::setVoxelAt(int xPosition, int yPosition, int zPosition, const char value)
{
}

unsigned char VoxelBlockPlaceholder::getTextureAt(int xPosition, int yPosition, int zPosition)
{
	return 255;
}
void VoxelBlockPlaceholder::setTextureAt(int xPosition, int yPosition, int zPosition, const unsigned char value)
{
}
