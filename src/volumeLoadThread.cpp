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
#include "volumeLoadThread.h"
#include "volume.h"
#include "block.h"
#include "gameDataStorage.h"

VolumeLoadThreadFunctor::VolumeLoadThreadFunctor(VolumeLoadThread* t)
{
	this->t = t;
}

/// Main function called by the new thread
void VolumeLoadThreadFunctor::operator()()
{
	Point3 actCoords;
	VoxelBlock* block;
	VoxelBlock* result;
	VoxelVolume* volume;
	blockLoadFunction loadFunc;
	blockTextureFunction texFunc;
	int pagingMode;
	int windowXInBlocks, windowYInBlocks, windowZInBlocks;
	int numBlocksX, numBlocksY, numBlocksZ;
	
	while (!t->endThread)
	{
		// Wait for new blocks to load
		{
			boost::mutex::scoped_lock lock(t->inputMut);
			//boost::unique_lock<boost::mutex> lock(t->inputMut);

			while (true)
			{
				while (!t->input.size())
				{
					t->inputCond.wait(lock);
					if (t->endThread)
						return;
				}

				// Select the next block to load: the block which is closest to the pagingTarget of the volume
				float bestDist = 999999999.0f;
				std::list<Point3>::iterator bestIt;
				for (std::list<Point3>::iterator it = t->input.begin(); it != t->input.end(); it++)
				{
					Vector3 blockPos = Vector3(it->x, it->y, it->z);
					blockPos *= BLOCK_SIDE_LENGTH;
					blockPos += Vector3(BLOCK_SIDE_LENGTH / 2);
					float dist = t->volume->pagingTarget.squaredDistance(blockPos);
					
					if (dist < bestDist)
					{
						bestDist = dist;
						bestIt = it;
					}
				}
				
				actCoords = *bestIt;
				t->input.erase(bestIt);
				
				// If the block is no longer in the volume, discard it
				if (t->volume->containsBlockAbs(actCoords.x, actCoords.y, actCoords.z))
				{
					// If the block is not a placeholder, discard it
					VoxelBlock* testBlock = t->volume->getBlockAbs(actCoords.x, actCoords.y, actCoords.z);
					if (testBlock->isPlaceholder())
						break;	// load the block
				}
			};

			// get all neccessary information
			volume = t->volume;
			pagingMode = volume->pagingMode;
			loadFunc = volume->blockLoadCB;
			texFunc = volume->blockTextureCB;
			windowXInBlocks = volume->windowXInBlocks;
			windowYInBlocks = volume->windowYInBlocks;
			windowZInBlocks = volume->windowZInBlocks;
			numBlocksX = volume->numBlocksX;
			numBlocksY = volume->numBlocksY;
			numBlocksZ = volume->numBlocksZ;
		}

		// Load the block at actCoords
		block = VoxelVolume::loadBlock(actCoords, volume, pagingMode, NULL, loadFunc, texFunc, windowXInBlocks, windowYInBlocks, windowZInBlocks, numBlocksX, numBlocksY, numBlocksZ);

		if (VoxelVolume::checkBlock(block, volume, result))
			delete block;
		else
			result = block;
		
		// Put the block in the output list
		{
			boost::mutex::scoped_lock lock(t->outputMut);
			//boost::lock_guard<boost::mutex> lock(t->outputMut);
			t->output.push_back(result);
		}
	}
}

bool VolumeLoadThread::getResult(VoxelBlock*& out)
{
	{
		boost::mutex::scoped_lock lock(outputMut);
		//boost::lock_guard<boost::mutex> lock(outputMut);
		if (!output.size())
			return false;

		out = *output.begin();
		output.erase(output.begin());
		return true;
	}
}

void VolumeLoadThread::load(Point3 position)
{
	{
		boost::mutex::scoped_lock lock(inputMut);
		//boost::lock_guard<boost::mutex> lock(inputMut);

		// Don't duplicate block requests
		for (std::list<Point3>::iterator it = input.begin(); it != input.end(); it++)
		{
			if ((it->x == position.x) && (it->y == position.y) && (it->z == position.z))
				return;
		}
		
		input.push_back(position);
	}
	inputCond.notify_one();
}

void VolumeLoadThread::clearTasks()
{
	// Stop the thread
	{
		boost::mutex::scoped_lock lock(inputMut);
		//boost::lock_guard<boost::mutex> lock(inputMut);

		endThread = true;
	}
	inputCond.notify_one();
	thread->join();

	// Clear everything
	for (std::list<VoxelBlock*>::iterator it = output.begin(); it != output.end(); it++)
		delete *it;
	output.clear();
	input.clear();

	// Restart the thread
	endThread = false;
	thread = new boost::thread(*functor);
}

VolumeLoadThread::VolumeLoadThread(VoxelVolume* volume)
{
	this->volume = volume;
	endThread = false;

	functor = new VolumeLoadThreadFunctor(this);
	thread = new boost::thread(*functor);
}
VolumeLoadThread::~VolumeLoadThread()
{
	{
		boost::mutex::scoped_lock lock(inputMut);
		//boost::lock_guard<boost::mutex> lock(inputMut);

		endThread = true;
	}
	inputCond.notify_one();
	thread->join();

	for (std::list<VoxelBlock*>::iterator it = output.begin(); it != output.end(); it++)
		delete *it;
	
	delete thread;
	delete functor;
}
