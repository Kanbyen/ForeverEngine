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
#include "block.h"
#include "volume.h"
#include "blockPersistent.h"
#include "object.h"
#include "gameDataStorage.h"

VoxelBlock::VoxelBlock(int blockX, int blockY, int blockZ, VoxelVolume* volume, VoxelBlockPersistent* persistent)
	: position(blockX, blockY, blockZ)
{
	this->volume = volume;
	this->persistent = persistent;
	if (persistent)
		persistent->changeOwner(blockX, blockY, blockZ);
	dataChanged = true;
	objectsLoaded = false;
	customChanges = false;
}

void VoxelBlock::loadObjects()
{
	if (objectsLoaded)
		return;
	objectsLoaded = true;

	gameDataStorage.fetchObjects(position.x, position.y, position.z);
	
	// (Lots of) Debug output:
	//LogManager::getSingleton().logMessage("loadObjects " + StringConverter::toString(position.x) + " " + StringConverter::toString(position.y) + " " + StringConverter::toString(position.z));
}

void VoxelBlock::unloadObjects()
{
	// Create a vector of all objects in this block
	const float scaledBlockLength = BLOCK_SIDE_LENGTH * volume->scale;
	std::vector<Object*> vec;
	for (ObjectManager::iterator it = objectManager.begin(); it != objectManager.end(); ++it)
	{
		Vector3& pos = (*it).second->position;
		if (pos.x < position.x * scaledBlockLength || pos.y < position.y * scaledBlockLength || pos.z < position.z * scaledBlockLength ||
		    pos.x >= (position.x+1) * scaledBlockLength || pos.y >= (position.y+1) * scaledBlockLength || pos.z >= (position.z+1) * scaledBlockLength)
			continue;
		vec.push_back((*it).second);
	}

	if (vec.size())
		gameDataStorage.storeObjectVector(vec, position.x, position.y, position.z);

	// (Lots of) Debug output:
	//LogManager::getSingleton().logMessage("UNloadObjects " + StringConverter::toString(position.x) + " " + StringConverter::toString(position.y) + " " + StringConverter::toString(position.z));
}
