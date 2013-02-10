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

#include "blockConstants.h"

inline VoxelBlock* VoxelVolume::getBlockAbs(int x, int y, int z)
{
	x -= windowXInBlocks;
	y -= windowYInBlocks;
	z -= windowZInBlocks;
	return getBlockRel(x, y, z);
}

inline void VoxelVolume::setBlockAbs(int x, int y, int z, VoxelBlock* newBlock)
{
	x -= windowXInBlocks;
	y -= windowYInBlocks;
	z -= windowZInBlocks;
	setBlockRel(x, y, z, newBlock);
}

inline VoxelBlock* VoxelVolume::getBlockRel(int x, int y, int z)
{
#ifdef _DEBUG
	if ((int)(x + y * numBlocksX + z * numBlocksX * numBlocksY) >= numBlocks)
		throw new Ogre::Exception(0, "getBlockAt() with wrong Parameters (result too big)!", "getBlockAt");
	if (x + y * numBlocksX + z * numBlocksX * numBlocksY < 0)
		throw new Ogre::Exception(0, "getBlockAt() with wrong Parameters (result negative)!", "getBlockAt");
#endif
	return blocks[x + y * numBlocksX + z * numBlocksX * numBlocksY];
}

inline void VoxelVolume::setBlockRel(int x, int y, int z, VoxelBlock* newBlock)
{
#ifdef _DEBUG
	if ((int)(blockX + blockY * numBlocksX + blockZ * numBlocksX * numBlocksY) >= numBlocks)
		throw new Ogre::Exception(0, "setBlockAt() with wrong Parameters (result too big)!", "setBlockAt");
	if (blockX + blockY * numBlocksX + blockZ * numBlocksX * numBlocksY < 0)
		throw new Ogre::Exception(0, "setBlockAt() with wrong Parameters (result negative)!", "setBlockAt");
#endif
	int index = x + y * numBlocksX + z * numBlocksX * numBlocksY;
	delete blocks[index];
	blocks[index] = newBlock;
}

inline VoxelBlockPersistent* VoxelVolume::getPersistentAbs(int x, int y, int z)
{
	x -= windowXInBlocks;
	y -= windowYInBlocks;
	z -= windowZInBlocks;
	return getPersistentRel(x, y, z);
}

inline VoxelBlockPersistent* VoxelVolume::getPersistentRel(int blockX, int blockY, int blockZ)
{
#ifdef _DEBUG
	if ((int)(blockX + blockY * numBlocksX + blockZ * numBlocksX * numBlocksY) >= numBlocks)
		throw new Ogre::Exception(0, "getPersistentAt() with wrong Parameters (result too big)!", "getPersistentAt");
	if (blockX + blockY * numBlocksX + blockZ * numBlocksX * numBlocksY < 0)
		throw new Ogre::Exception(0, "getPersistentAt() with wrong Parameters (result negative)!", "getPersistentAt");
#endif
	return blocksPersistent[blockX + blockY * numBlocksX + blockZ * numBlocksX * numBlocksY];
}

inline char VoxelVolume::getVoxelAt(Vector3& pos) const
{
	return getVoxelAt(pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f);
}

inline char VoxelVolume::getVoxelAt(int xPosition, int yPosition, int zPosition) const
{
	xPosition -= windowXInVoxels;
	yPosition -= windowYInVoxels;
	zPosition -= windowZInVoxels;
	
	return getVoxelRel(xPosition, yPosition, zPosition);
}

inline char VoxelVolume::getVoxelRel(int xPosition, int yPosition, int zPosition) const
{
	int blockX = xPosition / BLOCK_SIDE_LENGTH;
	int blockY = yPosition / BLOCK_SIDE_LENGTH;
	int blockZ = zPosition / BLOCK_SIDE_LENGTH;

	int xOffset = xPosition - (blockX * BLOCK_SIDE_LENGTH);
	int yOffset = yPosition - (blockY * BLOCK_SIDE_LENGTH);
	int zOffset = zPosition - (blockZ * BLOCK_SIDE_LENGTH);

	VoxelBlock* block = blocks[blockX + blockY * numBlocksX + blockZ * numBlocksX * numBlocksY];
	return block->getVoxelAt(xOffset, yOffset, zOffset);
}

inline void VoxelVolume::setVoxelAt(int xPosition, int yPosition, int zPosition, const char value)
{
	xPosition -= windowXInVoxels;
	yPosition -= windowYInVoxels;
	zPosition -= windowZInVoxels;
	
	setVoxelRel(xPosition, yPosition, zPosition, value);
}

inline void VoxelVolume::setVoxelRel(int xPosition, int yPosition, int zPosition, const char value)
{
	int blockX = xPosition / BLOCK_SIDE_LENGTH;
	int blockY = yPosition / BLOCK_SIDE_LENGTH;
	int blockZ = zPosition / BLOCK_SIDE_LENGTH;

	int xOffset = xPosition - (blockX * BLOCK_SIDE_LENGTH);
	int yOffset = yPosition - (blockY * BLOCK_SIDE_LENGTH);
	int zOffset = zPosition - (blockZ * BLOCK_SIDE_LENGTH);

	VoxelBlock* block = blocks[blockX + blockY * numBlocksX + blockZ * numBlocksX * numBlocksY];
	block->setVoxelAt(xOffset, yOffset, zOffset, value);
}

inline unsigned char VoxelVolume::getTextureAt(int xPosition, int yPosition, int zPosition) const
{
	xPosition -= windowXInVoxels;
	yPosition -= windowYInVoxels;
	zPosition -= windowZInVoxels;
	
	return getTextureRel(xPosition, yPosition, zPosition);
}

inline unsigned char VoxelVolume::getTextureRel(int xPosition, int yPosition, int zPosition) const
{
	int blockX = xPosition / BLOCK_SIDE_LENGTH;
	int blockY = yPosition / BLOCK_SIDE_LENGTH;
	int blockZ = zPosition / BLOCK_SIDE_LENGTH;

	int xOffset = xPosition - (blockX * BLOCK_SIDE_LENGTH);
	int yOffset = yPosition - (blockY * BLOCK_SIDE_LENGTH);
	int zOffset = zPosition - (blockZ * BLOCK_SIDE_LENGTH);

	VoxelBlock* block = blocks[blockX + blockY * numBlocksX + blockZ * numBlocksX * numBlocksY];
	return block->getTextureAt(xOffset, yOffset, zOffset);
}

inline void VoxelVolume::setTextureAt(int xPosition, int yPosition, int zPosition, const unsigned char value)
{
	xPosition -= windowXInVoxels;
	yPosition -= windowYInVoxels;
	zPosition -= windowZInVoxels;
	
	setTextureRel(xPosition, yPosition, zPosition, value);
}

inline void VoxelVolume::setTextureRel(int xPosition, int yPosition, int zPosition, const unsigned char value)
{
	int blockX = xPosition / BLOCK_SIDE_LENGTH;
	int blockY = yPosition / BLOCK_SIDE_LENGTH;
	int blockZ = zPosition / BLOCK_SIDE_LENGTH;

	int xOffset = xPosition - (blockX * BLOCK_SIDE_LENGTH);
	int yOffset = yPosition - (blockY * BLOCK_SIDE_LENGTH);
	int zOffset = zPosition - (blockZ * BLOCK_SIDE_LENGTH);

	VoxelBlock* block = blocks[blockX + blockY * numBlocksX + blockZ * numBlocksX * numBlocksY];
	block->setTextureAt(xOffset, yOffset, zOffset, value);
}

inline bool VoxelVolume::containsPointAbs(Vector3& pos)
{
	return containsPointAbs(pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f);
}

inline bool VoxelVolume::containsPointAbs(int x, int y, int z)
{
	x -= windowXInVoxels;
	y -= windowYInVoxels;
	z -= windowZInVoxels;
	
	return containsPointRel(x, y, z);
}

inline bool VoxelVolume::isValidBlockAbs(int x, int y, int z)
{
	x -= windowXInBlocks;
	y -= windowYInBlocks;
	z -= windowZInBlocks;

	if (!containsBlockRel(x, y, z))
		return false;

	if (!getBlockRel(x, y, z)->getObjectsLoaded())
		return false;

	return true;
}

inline bool VoxelVolume::containsPointRel(int x, int y, int z)
{
	return (((x > -1) && x < numVoxelsX) &&
			((y > -1) && y < numVoxelsY) &&
			((z > -1) && z < numVoxelsZ));
}

inline bool VoxelVolume::containsBlockAbs(int x, int y, int z)
{
	x -= windowXInBlocks;
	y -= windowYInBlocks;
	z -= windowZInBlocks;
	
	return containsBlockRel(x, y, z);
}

inline bool VoxelVolume::containsBlockRel(int x, int y, int z)
{
	return (((x > -1) && x < numBlocksX) &&
			((y > -1) && y < numBlocksY) &&
			((z > -1) && z < numBlocksZ));
}
