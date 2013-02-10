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
#include "gameDataStorage.h"
#include "block.h"
#include "blockConstants.h"
#include "blockFull.h"
#include "blockEmpty.h"
#include "blockLoaded.h"
#include "object.h"
#include "stateGame.h"	// TODO: <REFACTOR>: This is only needed because of game.volumeData->scale!
#include "volume.h"

GameDataStorage gameDataStorage;

void GameDataStorage::clear()
{
	for (std::map< int, std::map< int, std::map< int, BlockStorage> > >::iterator itx = storage.begin(); itx != storage.end(); ++itx)
	{
		for (std::map< int, std::map< int, BlockStorage> >::iterator ity = (*itx).second.begin(); ity != (*itx).second.end(); ++ity)
		{
			for (std::map< int, BlockStorage>::iterator itz = (*ity).second.begin(); itz != (*ity).second.end(); ++itz)
			{
				BlockStorage& bs = (*itz).second;
				if (bs.block)
				{
					delete bs.block;
					bs.block = NULL;
				}
				for (std::vector<Object*>::iterator ito = bs.objects.begin(); ito != bs.objects.end(); ++ito)
					(*ito)->exit();
				bs.objects.clear();
			}
		}
	}
}

void GameDataStorage::storeBlock(VoxelBlock* block, int x, int y, int z)
{
	boost::mutex::scoped_lock lock(access);

	//LogManager::getSingleton().logMessage("storeBlock " + StringConverter::toString(x) + " " + StringConverter::toString(y) + " " + StringConverter::toString(z));
	
	BlockStorage* bs = getOrCreateStorage(x, y, z);
	assert(!bs->block && "The game attempts to store a block, but a block with the same coordinates is already stored!");

	bs->block = block;

	block->setObjectsLoaded(false);
}
VoxelBlock* GameDataStorage::fetchBlock(int x, int y, int z)
{
	boost::mutex::scoped_lock lock(access);
	
	BlockStorage* bs = getStorage(x, y, z);
	if (!bs)
		return NULL;

	//LogManager::getSingleton().logMessage("fetchBlock " + StringConverter::toString(x) + " " + StringConverter::toString(y) + " " + StringConverter::toString(z));

	VoxelBlock* block = bs->block;
	bs->block = NULL;
	return block;
}

bool GameDataStorage::saveModifiedBlocks(const char* basePath, VoxelVolume* volume)
{
	string indexPath = basePath + string("index");
	FILE* blockIndex = fopen(indexPath.c_str(), "wb");
	if (!blockIndex)
		return false;

	// Save blocks from storage
	{
		boost::mutex::scoped_lock lock(access);

		for (std::map< int, std::map< int, std::map< int, BlockStorage> > >::iterator itx = storage.begin(); itx != storage.end(); ++itx)
		{
			for (std::map< int, std::map< int, BlockStorage> >::iterator ity = (*itx).second.begin(); ity != (*itx).second.end(); ++ity)
			{
				for (std::map< int, BlockStorage>::iterator itz = (*ity).second.begin(); itz != (*ity).second.end(); ++itz)
				{
					BlockStorage& bs = (*itz).second;
					if (bs.block)
						saveBlock(basePath, bs.block, blockIndex);
				}
			}
		}
	}

	// Save blocks from volume
	for (int z = 0; z < volume->numBlocksZ; z++)
	{
		for (int y = 0; y < volume->numBlocksY; y++)
		{
			for (int x = 0; x < volume->numBlocksX; x++)
			{
				int index = x + y * volume->numBlocksX + z * volume->numBlocksX * volume->numBlocksY;
				VoxelBlock* block = volume->blocks[index];
				if (block->getCustomChanges())
					saveBlock(basePath, block, blockIndex);
			}
		}
	}

	// Save block from volumeLoadThread output queue; wait a bit to ensure that the load thread is not currently processing a block with custom changes
	// TODO

	fclose(blockIndex);
	return true;
}
bool GameDataStorage::loadBlocks(const char* basePath, VoxelVolume* volume)
{
	string indexPath = basePath + string("index");
	FILE* blockIndex = fopen(indexPath.c_str(), "rb");
	if (!blockIndex)
		return false;

	int numBlocks;
	fseek(blockIndex, 0, SEEK_END);
	numBlocks = ftell(blockIndex) / (sizeof(int) * 3 + sizeof(char));
	fseek(blockIndex, 0, SEEK_SET);

	for (int i = 0; i < numBlocks; ++i)
	{
		char typeID;
		int x, y, z;
		fread(&x, sizeof(int), 1, blockIndex);
		fread(&y, sizeof(int), 1, blockIndex);
		fread(&z, sizeof(int), 1, blockIndex);
		fread(&typeID, sizeof(char), 1, blockIndex);

		FILE* blockFile = NULL;
		if (typeID != 1)
		{
			ostringstream filename;
			filename << basePath << x << " " << y << " " << z;
			blockFile = fopen(filename.str().c_str(), "rb");
			if (!blockFile)
				continue;
		}

		VoxelBlock* block;
		if (typeID == 1)
			block = new VoxelBlockEmpty(x, y, z, volume, NULL, 0);
		else if (typeID == 2)
		{
			VoxelBlockFull* fullBlock = new VoxelBlockFull(x, y, z, volume, NULL, NULL);
			fread(&fullBlock->texture, sizeof(unsigned char), NUM_VOXELS_IN_BLOCK, blockFile);
			block = fullBlock;
		}
		else
		{
			VoxelBlockLoaded* loadedBlock = new VoxelBlockLoaded(x, y, z, volume, NULL);
			fread(&loadedBlock->texture, sizeof(unsigned char), NUM_VOXELS_IN_BLOCK, blockFile);
			fread(&loadedBlock->data, sizeof(signed char), NUM_VOXELS_IN_BLOCK, blockFile);
			block = loadedBlock;
		}

		if (typeID != 1)
			fclose(blockFile);

		block->setCustomChanges(true);
		storeBlock(block, x, y, z);
	}

	fclose(blockIndex);
	return true;
}
void GameDataStorage::saveBlock(const char* basePath, VoxelBlock* block, FILE* blockIndex)
{
	assert(block->typeID() != 3);	// don't save placeholder blocks!

	// Write blockIndex
	fwrite(&block->position.x, sizeof(int), 1, blockIndex);
	fwrite(&block->position.y, sizeof(int), 1, blockIndex);
	fwrite(&block->position.z, sizeof(int), 1, blockIndex);

	const char typeID = block->typeID();
	fwrite(&typeID, sizeof(char), 1, blockIndex);
	
	if (block->typeID() == 1)	// empty block
		return;

	// Create block file
	ostringstream filename;
	filename << basePath << block->position.x << " " << block->position.y << " " << block->position.z;

	FILE* blockFile = fopen(filename.str().c_str(), "wb");
	if (!blockFile)
		return;

	// Write texture data and voxel data for loaded blocks
	if (block->typeID() == 2)
	{
		// full block
		VoxelBlockFull* fullBlock = (VoxelBlockFull*)block;
		fwrite(&fullBlock->texture, sizeof(unsigned char), NUM_VOXELS_IN_BLOCK, blockFile);
	}
	else
	{
		// normal block
		VoxelBlockLoaded* loadedBlock = (VoxelBlockLoaded*)block;
		fwrite(&loadedBlock->texture, sizeof(unsigned char), NUM_VOXELS_IN_BLOCK, blockFile);
		fwrite(&loadedBlock->data, sizeof(signed char), NUM_VOXELS_IN_BLOCK, blockFile);
	}

	fclose(blockFile);
}

bool GameDataStorage::saveObjects(const char* basePath)
{
	string filePath = basePath + string("objects");
	FILE* file = fopen(filePath.c_str(), "wb");
	if (!file)
		return false;
	
	const char* formatString = "FWOBJ";
	fwrite(formatString, sizeof(char), 5, file);
	int version = 1;
	fwrite(&version, sizeof(int), 1, file);
	
	// Save objects from storage
	{
		boost::mutex::scoped_lock lock(access);
		
		for (std::map< int, std::map< int, std::map< int, BlockStorage> > >::iterator itx = storage.begin(); itx != storage.end(); ++itx)
		{
			for (std::map< int, std::map< int, BlockStorage> >::iterator ity = (*itx).second.begin(); ity != (*itx).second.end(); ++ity)
			{
				for (std::map< int, BlockStorage>::iterator itz = (*ity).second.begin(); itz != (*ity).second.end(); ++itz)
				{
					BlockStorage& bs = (*itz).second;
					for (std::vector<Object*>::iterator it = bs.objects.begin(); it != bs.objects.end(); ++it)
						saveObject(*it, file);
				}
			}
		}
	}
	
	// Save currently active objects
	for (ObjectManager::iterator it = objectManager.begin(); it != objectManager.end(); ++it)
		saveObject((*it).second, file);
	
	fclose(file);
	return true;
}
bool GameDataStorage::loadObjects(const char* basePath)
{
	string filePath = basePath + string("objects");
	FILE* file = fopen(filePath.c_str(), "rb");
	if (!file)
		return false;
	
	char formatString[6];
	int version;
	
	fread(formatString, sizeof(char), 5, file);
	formatString[5] = 0;
	if (strcmp(formatString, "FWOBJ") != 0)
	{
		version = 0;
		fseek(file, 0, SEEK_SET);
	}
	else
		fread(&version, sizeof(int), 1, file);
	
	while (!feof(file))
	{
		// Load an object
		loadObject(file, version);
	}
	
	fclose(file);
	return true;
}
void GameDataStorage::loadObject(FILE* file, int version)
{
	char buffer[256];
	bool objCreated = false;
	
	// Load name
	int nameLen;
	if (fread(&nameLen, sizeof(int), 1, file) < 1)
		return;
	fread(buffer, sizeof(char), nameLen, file);
	buffer[nameLen] = 0;
	
	// Load position
	Vector3 pos;
	fread(&pos.x, sizeof(float), 1, file);
	fread(&pos.y, sizeof(float), 1, file);
	fread(&pos.z, sizeof(float), 1, file);
	
	// Create object
	Object* obj = new Object();
	obj->name = buffer;
	obj->id = INVALID_OBJECT_ID;	// so the gameDataStorage knows that this object has to be created first
	obj->position = pos;
	
	if (version >= 1)
	{
		char rotationAndScale;
		fread(&rotationAndScale, sizeof(char), 1, file);
		
		if (rotationAndScale & 1<<0)
		{
			float w, x, y, z;
			fread(&w, sizeof(float), 1, file);
			fread(&x, sizeof(float), 1, file);
			fread(&y, sizeof(float), 1, file);
			fread(&z, sizeof(float), 1, file);
			btQuaternion q(w, x, y, z);
			
			if (!objCreated)
			{
				obj = createNewObject(obj);
				objCreated = true;
			}
			obj->sendMessage(COMPMSG_SET_ORIENTATION, &q);
		}
		if (rotationAndScale & 1<<1)
		{
			Vector3 scale;
			fread(&scale.x, sizeof(float), 1, file);
			fread(&scale.y, sizeof(float), 1, file);
			fread(&scale.z, sizeof(float), 1, file);
			
			if (!objCreated)
			{
				obj = createNewObject(obj);
				objCreated = true;
			}
			obj->sendMessage(COMPMSG_SET_SCALE, &scale);
		}
	}
	
	// Store object
	gameDataStorage.storeObject(obj);
}
void GameDataStorage::saveObject(Object* object, FILE* file)
{
	// Write name
	int nameLen = object->name.size();
	fwrite(&nameLen, sizeof(int), 1, file);
	fwrite(object->name.c_str(), sizeof(char), nameLen, file);
	
	// Write position
	fwrite(&object->position.x, sizeof(float), 1, file);
	fwrite(&object->position.y, sizeof(float), 1, file);
	fwrite(&object->position.z, sizeof(float), 1, file);
	
	char rotationAndScale = 0;
	btQuaternion rotation;
	Vector3 scale;
	if (object->sendGetMessage(COMPMSG_GET_ORIENTATION, &rotation))
		rotationAndScale |= 1<<0;
	if (object->sendGetMessage(COMPMSG_GET_SCALE, &scale))
		rotationAndScale |= 1<<1;
	
	fwrite(&rotationAndScale, sizeof(char), 1, file);
	
	if (rotationAndScale & 1<<0)
	{
		fwrite(&rotation.w(), sizeof(float), 1, file);
		fwrite(&rotation.x(), sizeof(float), 1, file);
		fwrite(&rotation.y(), sizeof(float), 1, file);
		fwrite(&rotation.z(), sizeof(float), 1, file);
	}
	if (rotationAndScale & 1<<1)
	{
		fwrite(&scale.x, sizeof(float), 1, file);
		fwrite(&scale.y, sizeof(float), 1, file);
		fwrite(&scale.z, sizeof(float), 1, file);
	}
}

void GameDataStorage::storeObjectVector(std::vector<Object*>& objects, int x, int y, int z)
{
	// Prepare the objects
	for (std::vector<Object*>::iterator it = objects.begin(); it != objects.end(); )
	{
		if (!prepareObjectForStorage(*it))
			it = objects.erase(it);
		else
			++it;
	}

	// Store the objects
	boost::mutex::scoped_lock lock(access);
	BlockStorage* bs = getOrCreateStorage(x, y, z);
	bs->objects.insert(bs->objects.end(), objects.begin(), objects.end());
}
void GameDataStorage::storeObject(Object* object, int x, int y, int z)
{
	if (!prepareObjectForStorage(object))
		return;

	// Store it
	boost::mutex::scoped_lock lock(access);
	BlockStorage* bs = getOrCreateStorage(x, y, z);
	bs->objects.push_back(object);
}
void GameDataStorage::storeObject(Object* object)
{
	int x = object->position.x / (BLOCK_SIDE_LENGTH * game.volumeData->scale); if (object->position.x < 0) x--;
	int y = object->position.y / (BLOCK_SIDE_LENGTH * game.volumeData->scale); if (object->position.y < 0) y--;
	int z = object->position.z / (BLOCK_SIDE_LENGTH * game.volumeData->scale); if (object->position.z < 0) z--;
	storeObject(object, x, y, z);
}
void GameDataStorage::fetchObjects(int x, int y, int z)
{
	std::vector<Object*> objects;

	// Fetch the list of objects from the storage
	{
		boost::mutex::scoped_lock lock(access);
		
		BlockStorage* bs = getStorage(x, y, z);
		if (!bs)
			return;

		objects = bs->objects;
		bs->objects.clear();

		if (!bs->block)
			deleteStorageIfPossible(x, y, z);
	}

	// Reactivate all objects
	for (std::vector<Object*>::iterator it = objects.begin(); it != objects.end(); ++it)
		reactivateObject(*it);
}

bool GameDataStorage::prepareObjectForStorage(Object* obj)
{
	if (obj->id == INVALID_OBJECT_ID)
		return true;	// The object has not been sent the create msg yet, so there is nothing to do
	
	if (!obj->sendMessage(COMPMSG_UNLOAD, NULL))
		return false;

	objectManager.removeObjectFromList(obj);
	return true;
}
Object* GameDataStorage::createNewObject(Object* obj)
{
	Object* placeholder = obj;
	
	obj = objectManager.createInactiveObject(placeholder->name.c_str(), INVALID_OBJECT_ID);
	obj->position = placeholder->position;
	
	delete placeholder;
	
	// The create msg has to be sent the first time, with position parameter
	if (!obj->sendMessage(COMPMSG_CREATE, &obj->position))
		return NULL;	// I can't imagine a reason for the object to delete itself as reaction to CREATE, but safe is safe
	
	return obj;
}
void GameDataStorage::reactivateObject(Object* obj)
{
	obj->deleted = OBJDELETE_NO;

	if (obj->id == INVALID_OBJECT_ID)
	{
		// This is a new object, create it
		obj = createNewObject(obj);
	}
	else
	{
		// This is a re-activated object. Use NULL as paramter for the create msg
		if (!obj->sendMessage(COMPMSG_CREATE, NULL))
			return;
	}

	// Re-inject the object into the manager
	objectManager.addExistingObjectToList(obj);
}

BlockStorage* GameDataStorage::getStorage(int x, int y, int z)
{
	std::map< int, std::map< int, std::map< int, BlockStorage> > >::iterator itx = storage.find(x);
	if (itx == storage.end())	return NULL;

	std::map< int, std::map< int, BlockStorage> >::iterator ity = (*itx).second.find(y);
	if (ity == (*itx).second.end())	return NULL;

	std::map< int, BlockStorage>::iterator itz = (*ity).second.find(z);
	if (itz == (*ity).second.end())	return NULL;

	return &(*itz).second;
}
BlockStorage* GameDataStorage::getOrCreateStorage(int x, int y, int z)
{
	std::map< int, std::map< int, std::map< int, BlockStorage> > >::iterator itx = storage.find(x);
	if (itx == storage.end())
	{
		std::map< int, std::map< int, BlockStorage> > storagey;
		itx = storage.insert(std::map< int, std::map< int, std::map< int, BlockStorage> > >::value_type(x, storagey)).first;
	}

	std::map< int, std::map< int, BlockStorage> >::iterator ity = (*itx).second.find(y);
	if (ity == (*itx).second.end())
	{
		std::map< int, BlockStorage> storagez;
		ity = (*itx).second.insert(std::map< int, std::map< int, BlockStorage> >::value_type(y, storagez)).first;
	}

	std::map< int, BlockStorage>::iterator itz = (*ity).second.find(z);
	if (itz == (*ity).second.end())
	{
		BlockStorage newStorage;
		newStorage.block = NULL;
		itz = (*ity).second.insert(std::map< int, BlockStorage>::value_type(z, newStorage)).first;
	}

	return &(*itz).second;
}
void GameDataStorage::deleteStorageIfPossible(int x, int y, int z)
{
	std::map< int, std::map< int, std::map< int, BlockStorage> > >::iterator itx = storage.find(x);
	if (itx == storage.end())	return;

	std::map< int, std::map< int, BlockStorage> >::iterator ity = (*itx).second.find(y);
	if (ity == (*itx).second.end())	return;

	std::map< int, BlockStorage>::iterator itz = (*ity).second.find(z);
	if (itz == (*ity).second.end())	return;

	BlockStorage* bs = &(*itz).second;
	if (bs->block || (bs->objects.size() > 0))
		return;

	// Delete it
	(*ity).second.erase(itz);
	if ((*ity).second.size() > 0)
		return;

	(*itx).second.erase(ity);
	if ((*itx).second.size() > 0)
		return;

	storage.erase(itx);
}
