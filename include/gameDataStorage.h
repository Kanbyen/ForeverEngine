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

#ifndef _GAME_DATA_STORAGE_H_
#define _GAME_DATA_STORAGE_H_

#include <map>
#include <vector>
#include <string>
#include <boost/thread.hpp>

using namespace std;

class VoxelVolume;
class VoxelBlock;
class Object;

/// Stores data of one voxel block which is currently unloaded, including all objects which are in this block TODO: block and objects could be separated for more performance; access has to be synchronized between volume load thread <-> main thread for the blocks, but objects are only accessed by the main thread
struct BlockStorage
{
	/// The stored voxel block at this position (with custom modifications) or NULL if no custom changes were made to the block and it can thus be regenerated from the level data
	VoxelBlock* block;
	/// List of temporarily unloaded objects in this block. These objects are not handled by the object manager.
	std::vector<Object*> objects;
};

/// Singleton class which stores objects and voxel blocks which are temporarily unloaded until a player comes close to them again. Will also provide functions to store them to disk / load them.
class GameDataStorage
{
public:

	/// Frees all allocated memory
	void clear();

	/// Stores the given block
	void storeBlock(VoxelBlock* block, int x, int y, int z);
	/// Returns the block and removes it from the storage or returns NULL if there is no stored block at the specified position
	VoxelBlock* fetchBlock(int x, int y, int z);

	/// Saves all modified blocks in the directory specified by basePath (for example "save/"). Returns true if successful
	bool saveModifiedBlocks(const char* basePath, VoxelVolume* volume);
	/// Loads all blocks which are saved in the specified directory. Returns true if successful
	bool loadBlocks(const char* basePath, VoxelVolume* volume);
	
	/// Saves all objects in the directory specified by basePath (for example "save/"). Returns true if successful
	bool saveObjects(const char* basePath);
	/// Loads all objects which are saved in the specified directory. Returns true if successful
	bool loadObjects(const char* basePath);

	/// Removes the objects in the given vector from the object manager, sends them COMPMSG_UNLOAD and stores them
	void storeObjectVector(std::vector<Object*>& objects, int x, int y, int z);
	/// Like storeObjectVector, but stores only one object
	void storeObject(Object* object, int x, int y, int z);
	/// Variation which determines the block from the object's position attribute
	void storeObject(Object* object);
	/// Sends the objects which are stored for the block with the given coordinates COMPMSG_CREATE (with first parameter NULL) and enters them into the object manager. This activates them again.
	void fetchObjects(int x, int y, int z);
	
protected:

	/// Access: storage[x][y][z]
	std::map< int, std::map< int, std::map< int, BlockStorage> > > storage;

	boost::mutex access;

	/// Removes the object from the object manager and sends COMPMSG_UNLOAD to it. If the object is not worth storing, it deletes itself and the function returns false
	bool prepareObjectForStorage(Object* obj);
	/// Creates the object if only the name and position is stored
	Object* createNewObject(Object* obj);
	/// Called by fetchObjects for every object
	void reactivateObject(Object* obj);

	/// Saves a single block into the specified directory
	void saveBlock(const char* basePath, VoxelBlock* block, FILE* blockIndex);
	/// Appends a single object to the specified file
	void saveObject(Object* object, FILE* file);
	/// Loads a single object from the file
	void loadObject(FILE* file, int version);

	/// Returns the specified block storage or NULL if it doesn't exist
	BlockStorage* getStorage(int x, int y, int z);
	/// Returns the specified block storage, creates it if it doesn't exist
	BlockStorage* getOrCreateStorage(int x, int y, int z);
	/// If no data is stored for the given position, but a BlockStorage object exists, the object is removed
	void deleteStorageIfPossible(int x, int y, int z);
};

extern GameDataStorage gameDataStorage;

#endif
