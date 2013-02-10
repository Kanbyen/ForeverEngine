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
#include "volume.h"
#include "block.h"
#include "blockPersistent.h"
#include "blockEmpty.h"
#include "blockFull.h"
#include "blockLoaded.h"
#include "blockPlaceholder.h"
#include "volumeLoadThread.h"
#include "gameDataStorage.h"
#include "physics.h"

std::list<int> VoxelVolume::relevantBlocks[2];

char VoxelVolume::fullBlockBuffer[BLOCKBUFFER_SIZE];

bool VoxelVolume::update(Vector3 cameraPosition)
{
	bool retVal = false;
	VoxelBlock* newBlock;

	// Handle automatic paging, if enabled
	pagingTarget = cameraPosition / scale;
	if (automaticPaging)
	{
		if (lastPagingPos.squaredDistance(pagingTarget) >= PAGING_RELOAD_DISTANCE_SQ)
		{
			lastPagingPos = pagingTarget;
			int targetX = ((int)(lastPagingPos.x + 0.5f) / BLOCK_SIDE_LENGTH) - VOLUME_RADIUS;
			int targetY = ((int)(lastPagingPos.y + 0.5f) / BLOCK_SIDE_LENGTH) - VOLUME_RADIUS;
			int targetZ = ((int)(lastPagingPos.z + 0.5f) / BLOCK_SIDE_LENGTH) - VOLUME_RADIUS;
			if ((int)(lastPagingPos.x + 0.5f) < 0)	targetX--;
			if ((int)(lastPagingPos.y + 0.5f) < 0)	targetY--;
			if ((int)(lastPagingPos.z + 0.5f) < 0)	targetZ--;
			moveWindow(targetX, targetY, targetZ);
		}
	}

	// Handle geometry changes
	if (volumeLoadThread)
	{
		// Check if the thread has a result
		VoxelBlock* result;
		int numNewBlocks = 0;
		const int MAX_BLOCKS_LOADED_IN_ONE_FRAME = 10;
		while ((numNewBlocks < MAX_BLOCKS_LOADED_IN_ONE_FRAME) && volumeLoadThread->getResult(result))
		{
			numNewBlocks++;
			if (containsBlockAbs(result->position.x, result->position.y, result->position.z))
			{
				changesThisFrame = true;
				setBlockAbs(result->position.x, result->position.y, result->position.z, result);
				result->persistent = getPersistentAbs(result->position.x, result->position.y, result->position.z);
				result->persistent->changeOwner(result->position.x, result->position.y, result->position.z);

				retVal = true;
			}
			else
			{
				delete result;
			}
		}

		if (!changesThisFrame)
			return retVal;
		changesThisFrame = false;
		
		/*ostringstream o;
		o << numNewBlocks;
		Ogre::LogManager::getSingleton().logMessage(string("Blocks loaded: ") + o.str());*/

		// load placeholder blocks and check existing blocks
		for (std::list<int>::iterator it = relevantBlocks[pagingMode].begin(); it != relevantBlocks[pagingMode].end(); it++)
		{
			VoxelBlock* block = blocks[*it];
			if (block->isPlaceholder())
			{
				VoxelBlockPlaceholder* placeholder = (VoxelBlockPlaceholder*)block;
				if (!placeholder->isBeingLoaded)
				{
					placeholder->isBeingLoaded = true;
					volumeLoadThread->load(placeholder->position);
				}
			}
			else
			{
				if (VoxelVolume::checkBlock(block, this, newBlock))
				{
					setBlockAbs(block->position.x, block->position.y, block->position.z, newBlock);
				}
			}
		}
	}
	else
	{
		if (!changesThisFrame)
			return retVal;
		changesThisFrame = false;

		// load placeholder blocks and check if loaded blocks became full or empty
		for (std::list<int>::iterator it = relevantBlocks[(int)pagingMode].begin(); it != relevantBlocks[pagingMode].end(); it++)
		{
			VoxelBlock* block = blocks[*it];
			if (block->isPlaceholder())
			{
				int lastX = BLOCK_SIDE_LENGTH - 1;
				int lastY = BLOCK_SIDE_LENGTH - 1;
				int lastZ = BLOCK_SIDE_LENGTH - 1;
				if (pagingMode == PAGINGMODE_CLOSED)
				{
					if (block->position.x - windowXInBlocks == numBlocksX - 1)	lastX = BLOCK_SIDE_LENGTH - 2;
					if (block->position.y - windowYInBlocks == numBlocksY - 1)	lastY = BLOCK_SIDE_LENGTH - 2;
					if (block->position.z - windowZInBlocks == numBlocksZ - 1)	lastZ = BLOCK_SIDE_LENGTH - 2;
				}

				newBlock = VoxelVolume::loadBlock(block->position, this, pagingMode, block->persistent, blockLoadCB, blockTextureCB, windowXInBlocks, windowYInBlocks, windowZInBlocks, numBlocksX, numBlocksY, numBlocksZ);
				setBlockAbs(block->position.x, block->position.y, block->position.z, newBlock);
				block = newBlock;
			}

			if (VoxelVolume::checkBlock(block, this, newBlock))
			{
				setBlockAbs(block->position.x, block->position.y, block->position.z, newBlock);
			}
		}
	}

	// call update for every voxel block to update the surface data
	for (std::list<int>::iterator it = relevantBlocks[(int)pagingMode].begin(); it != relevantBlocks[pagingMode].end(); it++)
	{
		VoxelBlockPersistent* persistent = blocksPersistent[*it];
		persistent->update();
	}

	// DEBUG: Show the number of full, empty and loaded blocks:
	/*CEGUI::Window* wnd = CEGUI::WindowManager::getSingleton().getWindow("Debug1");
	wnd->setText("empty: " + Ogre::StringConverter::toString(numEmptyBlocks));
	wnd = CEGUI::WindowManager::getSingleton().getWindow("Debug2");
	wnd->setText("full: " + Ogre::StringConverter::toString(numFullBlocks));
	wnd = CEGUI::WindowManager::getSingleton().getWindow("Debug3");
	wnd->setText("loaded: " + Ogre::StringConverter::toString(numLoadedBlocks));*/

	return retVal;
}

void VoxelVolume::moveWindow(int newXInBlocks, int newYInBlocks, int newZInBlocks)
{
	// TODO: get rid of evil hack
  boost::mutex wasted;
  boost::mutex * mutex2use = &wasted;
  if(volumeLoadThread)
    mutex2use = &(volumeLoadThread->inputMut);
	// Lock the input mutex if threaded
	// to get rid of evil hack figure out a value for the second parameter of the constructor
	boost::mutex::scoped_lock lock(*mutex2use, boost::defer_lock);
  if(volumeLoadThread)
    lock.lock();

	// Create new blocks and blocksPersistent arrays
	VoxelBlock** oldBlocks = blocks;
	VoxelBlockPersistent** oldBlocksPersistent = blocksPersistent;

	blocks = new VoxelBlock*[numBlocks];
	memset(blocks, 0, numBlocks * sizeof(VoxelBlock*));
	blocksPersistent = new VoxelBlockPersistent*[numBlocks];
	memset(blocksPersistent, 0, numBlocks * sizeof(VoxelBlockPersistent*));

	changesThisFrame = true;

	int deltaBlocksX = newXInBlocks - windowXInBlocks;
	int deltaBlocksY = newYInBlocks - windowYInBlocks;
	int deltaBlocksZ = newZInBlocks - windowZInBlocks;

	// Fill the new blocks[]: Take over old blocks or create new blocks
	for (int z = 0; z < numBlocksZ; z++)
		for (int y = 0; y < numBlocksY; y++)
			for (int x = 0; x < numBlocksX; x++)
			{
				int newIndex = x + y * numBlocksX + z * numBlocksX * numBlocksY;

				int oldX = x + deltaBlocksX;
				if (oldX < 0 || oldX >= numBlocksX)
				{
					blocks[newIndex] = new VoxelBlockPlaceholder(x + newXInBlocks, y + newYInBlocks, z + newZInBlocks, this, NULL);
					blocksPersistent[newIndex] = NULL;
					continue;
				}
				int oldY = y + deltaBlocksY;
				if (oldY < 0 || oldY >= numBlocksY)
				{
					blocks[newIndex] = new VoxelBlockPlaceholder(x + newXInBlocks, y + newYInBlocks, z + newZInBlocks, this, NULL);
					blocksPersistent[newIndex] = NULL;
					continue;
				}
				int oldZ = z + deltaBlocksZ;
				if (oldZ < 0 || oldZ >= numBlocksZ)
				{
					blocks[newIndex] = new VoxelBlockPlaceholder(x + newXInBlocks, y + newYInBlocks, z + newZInBlocks, this, NULL);
					blocksPersistent[newIndex] = NULL;
					continue;
				}

				// Take the block over from the old dataset
				int oldIndex = oldX + oldY * numBlocksX + oldZ * numBlocksX * numBlocksY;
				blocksPersistent[newIndex] = oldBlocksPersistent[oldIndex];
				blocks[newIndex] = oldBlocks[oldIndex];
				oldBlocksPersistent[oldIndex] = NULL;
				oldBlocks[oldIndex] = NULL;
			}

	// For every new placeholder block: find an old persistent and assign it.
	// At the same time, delete old blocks which didn't get into the new array
	int oldIndex = 0;
	for (int index = 0; index < numBlocks; index++)
	{
		if (!blocksPersistent[index])
		{
			for (; !oldBlocksPersistent[oldIndex]; oldIndex++)
				if (oldBlocks[oldIndex])
					unloadBlock(oldBlocks[oldIndex], oldBlocks[oldIndex]->position.x, oldBlocks[oldIndex]->position.y, oldBlocks[oldIndex]->position.z);
			blocksPersistent[index] = oldBlocksPersistent[oldIndex];
			blocks[index]->persistent = blocksPersistent[index];
			blocksPersistent[index]->changeOwner(blocks[index]->position.x, blocks[index]->position.y, blocks[index]->position.z);
			oldBlocksPersistent[oldIndex] = NULL;
		}
	}

	// Delete remaining old blocks
	for (; oldIndex < numBlocks; oldIndex++)
		if (oldBlocks[oldIndex])
			unloadBlock(oldBlocks[oldIndex], oldBlocks[oldIndex]->position.x, oldBlocks[oldIndex]->position.y, oldBlocks[oldIndex]->position.z);

	delete[] oldBlocks;
	delete[] oldBlocksPersistent;

	windowXInBlocks = newXInBlocks;
	windowYInBlocks = newYInBlocks;
	windowZInBlocks = newZInBlocks;
	windowXInVoxels = windowXInBlocks * BLOCK_SIDE_LENGTH;
	windowYInVoxels = windowYInBlocks * BLOCK_SIDE_LENGTH;
	windowZInVoxels = windowZInBlocks * BLOCK_SIDE_LENGTH;
}
void VoxelVolume::unloadBlock(VoxelBlock* block, int x, int y, int z)
{
	// Call the object unloading function
	block->unloadObjects();

	// If changes were made to this block, the block has to be entered in the gameDataStorage
	if (block->getCustomChanges())
	{
		block->setDataChanged(true);	// this is needed so that the geometry of the block is generated after reloading it
		gameDataStorage.storeBlock(block, x, y, z);
	}
	else
		delete block;
}

void VoxelVolume::setDataChangedRel(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ)
{
	changesThisFrame = true;

	// - and + 1 because changed voxels can affect neighbouring blocks!
	firstX = std::max(0, (firstX - 1));
	firstY = std::max(0, (firstY - 1));
	firstZ = std::max(0, (firstZ - 1));
	lastX = std::min(numVoxelsX - 1, (lastX + 1));
	lastY = std::min(numVoxelsY - 1, (lastY + 1));
	lastZ = std::min(numVoxelsZ - 1, (lastZ + 1));

	// also notify the physics system
	physics.activateObjectsInBox(scale * (firstX + windowXInVoxels), scale * (firstY + windowYInVoxels), scale * (firstZ + windowZInVoxels),
                                 scale * (lastX + windowXInVoxels), scale * (lastY + windowYInVoxels), scale * (lastZ + windowZInVoxels));

	firstX /= BLOCK_SIDE_LENGTH;
	firstY /= BLOCK_SIDE_LENGTH;
	firstZ /= BLOCK_SIDE_LENGTH;
	lastX /= BLOCK_SIDE_LENGTH;
	lastY /= BLOCK_SIDE_LENGTH;
	lastZ /= BLOCK_SIDE_LENGTH;

	for (int x = firstX; x <= lastX; x++)
		for (int y = firstY; y <= lastY; y++)
			for (int z = firstZ; z <= lastZ; z++)
				getBlockRel(x, y, z)->setDataChanged();
}

void VoxelVolume::setDataChangedAbs(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ)
{
	changesThisFrame = true;

	// also notify the physics system
	physics.activateObjectsInBox(scale * (firstX - 1), scale * (firstY - 1), scale * (firstZ - 1), scale * (lastX + 1), scale * (lastY + 1), scale * (lastZ + 1));

	makeRangeRelative(firstX, firstY, firstZ, lastX, lastY, lastZ);

	// - and + 1 because changed voxels can affect neighbouring blocks!
	firstX = std::max(0, (firstX - 1));
	firstY = std::max(0, (firstY - 1));
	firstZ = std::max(0, (firstZ - 1));
	lastX = std::min(numVoxelsX - 1, (lastX + 1));
	lastY = std::min(numVoxelsY - 1, (lastY + 1));
	lastZ = std::min(numVoxelsZ - 1, (lastZ + 1));

	firstX /= BLOCK_SIDE_LENGTH;
	firstY /= BLOCK_SIDE_LENGTH;
	firstZ /= BLOCK_SIDE_LENGTH;
	lastX /= BLOCK_SIDE_LENGTH;
	lastY /= BLOCK_SIDE_LENGTH;
	lastZ /= BLOCK_SIDE_LENGTH;

	for (int x = firstX; x <= lastX; x++)
		for (int y = firstY; y <= lastY; y++)
			for (int z = firstZ; z <= lastZ; z++)
				getBlockRel(x, y, z)->setDataChanged();
}

bool VoxelVolume::getRayIntersection(Vector3 start, Vector3 dir, Ogre::Vector3* out)
{
	dir /= 6 * scale;

	start /= scale;
	start -= Vector3(windowXInVoxels, windowYInVoxels, windowZInVoxels);

	int x = start.x + 0.5;
	int y = start.y + 0.5;
	int z = start.z + 0.5;

	while (true)
	{
		if (!containsPointRel(x, y, z))
			return false;
		if (getVoxelRel(x, y, z) >= 0)
		{
			*out = (Vector3(x, y, z) + Vector3(windowXInVoxels, windowYInVoxels, windowZInVoxels)) * scale;
			return true;
		}

		start += dir;

		x = start.x + 0.5;
		y = start.y + 0.5;
		z = start.z + 0.5;
	}
}

bool VoxelVolume::getRayIntersectionEx(Vector3 start, Vector3 dir, Ogre::Vector3* out, float length)
{
	dir /= 6 * scale;
	length *= 6;

	start /= scale;
	start -= Vector3(windowXInVoxels, windowYInVoxels, windowZInVoxels);

	int x = start.x + 0.5;
	int y = start.y + 0.5;
	int z = start.z + 0.5;

	while (true)
	{
		if (containsPointRel(x, y, z))
		{
			if (getVoxelRel(x, y, z) >= 0)
			{
				*out = (Vector3(x, y, z) + Vector3(windowXInVoxels, windowYInVoxels, windowZInVoxels)) * scale;
				return true;
			}
		}

		start += dir;

		x = start.x + 0.5;
		y = start.y + 0.5;
		z = start.z + 0.5;

		if (length-- <= 0)
			return false;
	}
}

VoxelVolume::VoxelVolume(int numBlocksX, int numBlocksY, int numBlocksZ, float scale, bool threaded, int windowXInVoxels, int windowYInVoxels, int windowZInVoxels)
{
	this->scale = scale;
	this->numBlocksX = numBlocksX;
	numVoxelsX = numBlocksX * BLOCK_SIDE_LENGTH;
	this->numBlocksY = numBlocksY;
	numVoxelsY = numBlocksY * BLOCK_SIDE_LENGTH;
	this->numBlocksZ = numBlocksZ;
	numVoxelsZ = numBlocksZ * BLOCK_SIDE_LENGTH;

	this->windowXInVoxels = windowXInVoxels;
	windowXInBlocks = windowXInVoxels * BLOCK_SIDE_LENGTH;
	this->windowYInVoxels = windowYInVoxels;
	windowYInBlocks = windowYInVoxels * BLOCK_SIDE_LENGTH;
	this->windowZInVoxels = windowZInVoxels;
	windowZInBlocks = windowZInVoxels * BLOCK_SIDE_LENGTH;
	
	blockLoadCB = NULL;
	blockTextureCB = NULL;

	numLoadedBlocks = 0;
	numFullBlocks = 0;
	numEmptyBlocks = 0;

	pagingMode = PAGINGMODE_CLOSED;
	materialName = "TexAtlasMaterial";

	numBlocks = numBlocksX * numBlocksY * numBlocksZ;
	blocks = new VoxelBlock*[numBlocks];
	blocksPersistent = new VoxelBlockPersistent*[numBlocks];

	changesThisFrame = true;

	for (int z = 0; z < numBlocksZ; z++)
		for (int y = 0; y < numBlocksY; y++)
			for (int x = 0; x < numBlocksX; x++)
			{
				int index = x + y * numBlocksX + z * numBlocksX * numBlocksY;
				blocksPersistent[index] = new VoxelBlockPersistent(x, y, z, this);
				blocks[index] = new VoxelBlockPlaceholder(x, y, z, this, blocksPersistent[index]);	// new VoxelBlockEmpty(x, y, z, this, blocksPersistent[index], 255);
			}

	if (threaded)
		volumeLoadThread = new VolumeLoadThread(this);
	else
		volumeLoadThread = NULL;
}

VoxelVolume::~VoxelVolume()
{
	delete volumeLoadThread;

	for (int i = 0; i < numBlocks; ++i)
	{
		if (blocks[i] != NULL)
			delete blocks[i];
		blocks[i] = NULL;
		delete blocksPersistent[i];
	}
	delete[] blocks;
	delete[] blocksPersistent;
}

void VoxelVolume::reset()
{
	changesThisFrame = true;
	for (int z = 0; z < numBlocksZ; z++)
		for (int y = 0; y < numBlocksY; y++)
			for (int x = 0; x < numBlocksX; x++)
			{
				setBlockRel(x, y, z, new VoxelBlockPlaceholder(x + windowXInBlocks, y + windowYInBlocks, z + windowZInBlocks, this, getPersistentRel(x, y, z)));
			}

	if (volumeLoadThread)
		volumeLoadThread->clearTasks();
}

void VoxelVolume::setMaterial(string materialName)
{
	this->materialName = materialName;
}

void VoxelVolume::fill(char value)
{
	for(int z = 0; z < numBlocksZ; z++)
		for(int y = 0; y < numBlocksY; y++)
			for(int x = 0; x < numBlocksX; x++)
				getBlockRel(x, y, z)->fill(value);

	setDataChangedRel(0, 0, 0, numVoxelsX, numVoxelsY, numVoxelsZ);
}

void VoxelVolume::setBlockLoadCallback(blockLoadFunction func)
{
	blockLoadCB = func;
}

void VoxelVolume::setBlockTextureCallback(blockTextureFunction func)
{
	blockTextureCB = func;
}

VoxelBlock* VoxelVolume::loadBlock(Point3 position, VoxelVolume* volume, int pagingMode, VoxelBlockPersistent* persistent, blockLoadFunction loadFunc, blockTextureFunction texFunc, int windowXInBlocks, int windowYInBlocks, int windowZInBlocks, int numBlocksX, int numBlocksY, int numBlocksZ)
{
	const int bx = position.x;
	const int by = position.y;
	const int bz = position.z;
	const int baseX = bx * BLOCK_SIDE_LENGTH;
	const int baseY = by * BLOCK_SIDE_LENGTH;
	const int baseZ = bz * BLOCK_SIDE_LENGTH;

	// Look for a block at this position in the gameDataStorage
	VoxelBlock* storedBlock = gameDataStorage.fetchBlock(bx, by, bz);
	if (storedBlock)
		return storedBlock;

	// If there is no load function, return an empty block
	if (!loadFunc)
		return new VoxelBlockEmpty(bx, by, bz, volume, persistent, 255);

	// Use the callback to fill the buffer with voxel data
	char buffer[BLOCKBUFFER_SIZE];
	int loadFuncRet = loadFunc(baseX - 1, baseY - 1, baseZ - 1,
							   (bx+1) * BLOCK_SIDE_LENGTH, (by+1) * BLOCK_SIDE_LENGTH, (bz+1) * BLOCK_SIDE_LENGTH,
							   buffer);

	// Shrink the range if this is closed mode and the block is at the volume boundary
	int firstX = 0, firstY = 0, firstZ = 0;
	int lastX = BLOCK_SIDE_LENGTH - 1, lastY = BLOCK_SIDE_LENGTH - 1, lastZ = BLOCK_SIDE_LENGTH - 1;
	bool atBoundary = false;
	if (pagingMode == PAGINGMODE_CLOSED)
	{
		if (position.x - windowXInBlocks == 0)	{atBoundary = true; firstX = 1;}
		if (position.y - windowYInBlocks == 0)	{atBoundary = true; firstY = 1;}
		if (position.z - windowZInBlocks == 0)	{atBoundary = true; firstZ = 1;}
		if (position.x - windowXInBlocks == numBlocksX - 1)	{atBoundary = true; lastX = BLOCK_SIDE_LENGTH - 2;}
		if (position.y - windowYInBlocks == numBlocksY - 1)	{atBoundary = true; lastY = BLOCK_SIDE_LENGTH - 2;}
		if (position.z - windowZInBlocks == numBlocksZ - 1)	{atBoundary = true; lastZ = BLOCK_SIDE_LENGTH - 2;}
	}

	char* blockBuffer = buffer;

	// If a block in closed mode is returned as full and it is at the volume boundary, it must become a normal block because the border should be empty!
	if ((loadFuncRet == RETURN_FULL_BLOCK) && atBoundary)
	{
		blockBuffer = fullBlockBuffer;
		loadFuncRet = RETURN_NORMAL_BLOCK;
	}

	// If the block is full or empty, generate texture data
	unsigned char texBuffer[BLOCK_SIDE_LENGTH*BLOCK_SIDE_LENGTH*BLOCK_SIDE_LENGTH];
	if (loadFuncRet != RETURN_EMPTY_BLOCK)
	{
		memset(texBuffer, 255, BLOCK_SIDE_LENGTH*BLOCK_SIDE_LENGTH*BLOCK_SIDE_LENGTH * sizeof(unsigned char));
		if (texFunc)
		{
			texFunc(baseX, baseY, baseZ,
					(bx+1) * BLOCK_SIDE_LENGTH - 1, (by+1) * BLOCK_SIDE_LENGTH - 1, (bz+1) * BLOCK_SIDE_LENGTH - 1,
					(loadFuncRet == RETURN_FULL_BLOCK) ? fullBlockBuffer : buffer, texBuffer);
		}
	}

	switch (loadFuncRet)
	{
	case RETURN_FULL_BLOCK:
	{
		VoxelBlockFull* newBlock = new VoxelBlockFull(bx, by, bz, volume, persistent, NULL);

		// Fill the block with texture data
		for (int x = firstX; x <= lastX; x++)
		{
			for (int y = firstY; y <= lastY; y++)
			{
				for (int z = firstZ; z <= lastZ; z++)
				{
					newBlock->_setTextureAt(x, y, z, texBuffer[x + y * BLOCK_SIDE_LENGTH + z * BLOCK_SIDE_LENGTH * BLOCK_SIDE_LENGTH]);
				}
			}
		}

		return newBlock;
	}
	case RETURN_EMPTY_BLOCK:
	{
		VoxelBlockEmpty* newBlock = new VoxelBlockEmpty(bx, by, bz, volume, persistent, 255);
		return newBlock;
	}
	case RETURN_NORMAL_BLOCK:
	{
		// If this is closed mode, empty the block content first because some parts don't get overwritten by the following step if the block is at the volume boundary
		VoxelBlockLoaded* newBlock = new VoxelBlockLoaded(bx, by, bz, volume, persistent);
		if (pagingMode == PAGINGMODE_CLOSED)
		{
			newBlock->fillTex(255);
			newBlock->fill(-128);
		}

		// Fill the block with data
		for (int x = firstX; x <= lastX; x++)
		{
			for (int y = firstY; y <= lastY; y++)
			{
				for (int z = firstZ; z <= lastZ; z++)
				{
					newBlock->_setVoxelAt(x, y, z, buffer[(x+1) + (y+1) * BLOCKBUFFER_SIDE_LENGTH + (z+1) * BLOCKBUFFER_SIDE_LENGTH * BLOCKBUFFER_SIDE_LENGTH]);
					newBlock->_setTextureAt(x, y, z, texBuffer[x + y * BLOCK_SIDE_LENGTH + z * BLOCK_SIDE_LENGTH * BLOCK_SIDE_LENGTH]);
				}
			}
		}

		return newBlock;
	}
	default:
		assert(!"A block load function returned a wrong value!");
	}
}

bool VoxelVolume::checkBlock(VoxelBlock* block, VoxelVolume* volume, VoxelBlock*& out)
{
	if (!block->innerVoxelsRelevant())
		return false;
	if (!block->getDataChanged())
		return false;

	VoxelBlockLoaded* bl = (VoxelBlockLoaded*)block;
	int v = bl->_getVoxelAt(0, 0, 0);
	if (v <= EMPTY_BLOCK_THRESHOLD)
	{
		// check if the whole block is empty
		// check the corners first to allow early-out
		for (int x = 0; x < BLOCK_SIDE_LENGTH; x += BLOCK_SIDE_LENGTH - 1)
			for (int y = 0; y < BLOCK_SIDE_LENGTH; y += BLOCK_SIDE_LENGTH - 1)
				for (int z = 0; z < BLOCK_SIDE_LENGTH; z += BLOCK_SIDE_LENGTH - 1)
					if (bl->_getVoxelAt(x, y, z) > EMPTY_BLOCK_THRESHOLD)
						return false;

		for (int x = 0; x < BLOCK_SIDE_LENGTH; x++)
			for (int y = 0; y < BLOCK_SIDE_LENGTH; y++)
				for (int z = 0; z < BLOCK_SIDE_LENGTH; z++)
					if (bl->_getVoxelAt(x, y, z) > EMPTY_BLOCK_THRESHOLD)
						return false;

		// This is an empty block!
		bool objectsLoaded = bl->getObjectsLoaded();
		bool customChanges = bl->getCustomChanges();
		VoxelBlockEmpty* newBlock = new VoxelBlockEmpty(block->position.x, block->position.y, block->position.z, volume, block->persistent,
				bl->_getTextureAt(BLOCK_SIDE_LENGTH / 2, BLOCK_SIDE_LENGTH / 2, BLOCK_SIDE_LENGTH / 2));
		newBlock->setObjectsLoaded(objectsLoaded);
		newBlock->setCustomChanges(customChanges);
		out = newBlock;
		return true;
	}
	else if (v >= FULL_BLOCK_THRESHOLD)
	{
		// check if the whole block is full
		// check the corners first to allow early-out
		for (int x = 0; x < BLOCK_SIDE_LENGTH; x += BLOCK_SIDE_LENGTH - 1)
			for (int y = 0; y < BLOCK_SIDE_LENGTH; y += BLOCK_SIDE_LENGTH - 1)
				for (int z = 0; z < BLOCK_SIDE_LENGTH; z += BLOCK_SIDE_LENGTH - 1)
					if (bl->_getVoxelAt(x, y, z) < FULL_BLOCK_THRESHOLD)
						return false;

		for (int x = 0; x < BLOCK_SIDE_LENGTH; x++)
			for (int y = 0; y < BLOCK_SIDE_LENGTH; y++)
				for (int z = 0; z < BLOCK_SIDE_LENGTH; z++)
					if (bl->_getVoxelAt(x, y, z) < FULL_BLOCK_THRESHOLD)
						return false;

		// This is a full block!
		bool objectsLoaded = bl->getObjectsLoaded();
		bool customChanges = bl->getCustomChanges();
		VoxelBlockFull* newBlock = new VoxelBlockFull(block->position.x, block->position.y, block->position.z, volume, block->persistent, bl);
		newBlock->setObjectsLoaded(objectsLoaded);
		newBlock->setCustomChanges(customChanges);
		out = newBlock;
		return true;
	}

	return false;
}

void VoxelVolume::execSphereFunc(Vector3 centre, Real radius, void (*func)(int,int,int,float))
{
	centre /= scale;
	centre -= Vector3(windowXInVoxels, windowYInVoxels, windowZInVoxels);
	radius /= scale;

	int firstX = static_cast<int>(std::floor(centre.x - radius));
	int firstY = static_cast<int>(std::floor(centre.y - radius));
	int firstZ = static_cast<int>(std::floor(centre.z - radius));

	int lastX = static_cast<int>(std::ceil(centre.x + radius));
	int lastY = static_cast<int>(std::ceil(centre.y + radius));
	int lastZ = static_cast<int>(std::ceil(centre.z + radius));

	checkBoundsRel(firstX, firstY, firstZ, lastX, lastY, lastZ);

	for (int z = firstZ; z <= lastZ; z++)
	{
		for (int y = firstY; y <= lastY; y++)
		{
			for (int x = firstX; x <= lastX; x++)
			{
				float value = radius - centre.distance(Vector3(x, y, z));
				if (value > 0)
					func(x, y, z, value);
			}
		}
	}

	setDataChangedRel(firstX, firstY, firstZ, lastX, lastY, lastZ);
}
void VoxelVolume::execCubeFunc(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, void (*func)(int,int,int,float))
{
	firstX /= scale;	lastX /= scale;
	firstY /= scale;	lastY /= scale;
	firstZ /= scale;	lastZ /= scale;
	makeRangeRelative(firstX, firstY, firstZ, lastX, lastY, lastZ);

	checkBoundsRel(firstX, firstY, firstZ, lastX, lastY, lastZ);

	for (int z = firstZ; z <= lastZ; z++)
	{
		for (int y = firstY; y <= lastY; y++)
		{
			for (int x = firstX; x <= lastX; x++)
			{
				func(x, y, z, 10.0f);
			}
		}
	}

	setDataChangedRel(firstX, firstY, firstZ, lastX, lastY, lastZ);
}

void VoxelVolume::execCubeFunc(Vector3 centre, Real radius, void (*func)(int,int,int,float))
{
	execCubeFunc(centre.x - radius, centre.y - radius, centre.z - radius, centre.x + radius, centre.y + radius, centre.z + radius, func);
}

void VoxelVolume::staticInit()
{
	// Full block buffer
	memset(fullBlockBuffer, FULL_BLOCK_VALUE, BLOCKBUFFER_SIZE);

	// PAGINGMODE_CLOSED
	for (int i = 0; i < VOLUME_SIDE_LENGTH*VOLUME_SIDE_LENGTH*VOLUME_SIDE_LENGTH; i++)
		relevantBlocks[(int)PAGINGMODE_CLOSED].push_back(i);

	// PAGINGMODE_OPEN
	Vector3 midPoint = Vector3(VOLUME_SIDE_LENGTH / 2, VOLUME_SIDE_LENGTH / 2, VOLUME_SIDE_LENGTH / 2);
	for (int z = 0; z < VOLUME_SIDE_LENGTH; z++)
	{
		for (int y = 0; y < VOLUME_SIDE_LENGTH; y++)
		{
			for (int x = 0; x < VOLUME_SIDE_LENGTH; x++)
			{
				//if (Vector3(x + 0.5f, y + 0.5f, z + 0.5f).squaredDistance(midPoint) <= VOLUME_RADIUS_SQ + 0.01f)	// TODO!
					relevantBlocks[(int)PAGINGMODE_OPEN].push_back(x + y * VOLUME_SIDE_LENGTH + z * VOLUME_SIDE_LENGTH * VOLUME_SIDE_LENGTH);
			}
		}
	}
}

void VoxelVolume::checkBoundsRel(int& firstX, int& firstY, int& firstZ, int& lastX, int& lastY, int& lastZ)
{
  	firstX = std::max(firstX, 1);
  	firstY = std::max(firstY, 1);
  	firstZ = std::max(firstZ, 1);
	firstX = std::min(firstX, numVoxelsX - 2);
	firstY = std::min(firstY, numVoxelsY - 2);
	firstZ = std::min(firstZ, numVoxelsZ - 2);

	lastX = std::max(lastX, 1);
	lastY = std::max(lastY, 1);
	lastZ = std::max(lastZ, 1);
	lastX = std::min(lastX, numVoxelsX - 2);
  	lastY = std::min(lastY, numVoxelsY - 2);
  	lastZ = std::min(lastZ, numVoxelsZ - 2);
}

void VoxelVolume::makeRangeRelative(int& firstX, int& firstY, int& firstZ, int& lastX, int& lastY, int& lastZ)
{
	firstX -= windowXInVoxels;
	firstY -= windowYInVoxels;
	firstZ -= windowZInVoxels;

	lastX -= windowXInVoxels;
	lastY -= windowYInVoxels;
	lastZ -= windowZInVoxels;
}

void VoxelVolume::makeRangeAbsolute(int& firstX, int& firstY, int& firstZ, int& lastX, int& lastY, int& lastZ)
{
	firstX += windowXInVoxels;
	firstY += windowYInVoxels;
	firstZ += windowZInVoxels;

	lastX += windowXInVoxels;
	lastY += windowYInVoxels;
	lastZ += windowZInVoxels;
}
