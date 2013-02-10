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

#ifndef _BLOCK_H_
#define _BLOCK_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "point3.h"

class VoxelVolume;
class VoxelBlockPersistent;

/// Base class for all blocks which make up a volume
class VoxelBlock
{
public:
	/// Pointer to the volume this block belongs to
	VoxelVolume* volume;

	/// Position of this block measured in blocks from the origin
	Point3 position;

	/// Pointer to the persistent data of this block
	VoxelBlockPersistent* persistent;

	/// Constructor; must be called by every subclass
	VoxelBlock(int blockX, int blockY, int blockZ, VoxelVolume* volume, VoxelBlockPersistent* persistent);
	virtual ~VoxelBlock() {}

	/// Must return if it's possible that a surface can be created in the middle of the block
	virtual bool innerVoxelsRelevant() {return false;}

	/// If this returns true, the block must be loaded if the player comes closer
	virtual bool isPlaceholder() 		{return false;}

	/// Must return an unique ID for the type of block (full / empty / loaded).
	virtual int typeID() = 0;

	// Voxel editing functions
	/// Fill the whole block with the specified value. Warning: if this is a VoxelBlockEmpty or -Full, it might replace itself, so you have to re-fetch the Block pointer!
	virtual void fill(char value) = 0;

	/// Get the value of the voxel at the specified position
	virtual char getVoxelAt(int xPosition, int yPosition, int zPosition) = 0;
	/// Set the voxel to the specified value. Warning: if this is a VoxelBlockEmpty or -Full, it might replace itself, so you have to re-fetch the Block pointer!
	virtual void setVoxelAt(int xPosition, int yPosition, int zPosition, const char value) = 0;

	/// Get the texture of the voxel at the specified position
	virtual unsigned char getTextureAt(int xPosition, int yPosition, int zPosition) = 0;
	/// Set the texture of the voxel at the specified position
	virtual void setTextureAt(int xPosition, int yPosition, int zPosition, const unsigned char value) = 0;

	/// Is called after the geometry for a block has been generated the first time.
	void loadObjects();

	/// Is called before the block is destroyed TODO: This is not called for the existing blocks when the player quits the game. Should this behaviour be changed (for the save/load system)?
	void unloadObjects();

	/// Must be called when using voxel editing functions to ensure that the block will update its geometry data
	inline void setDataChanged(bool changed = true) 		{dataChanged = changed; if (changed) customChanges = true;}
	inline bool getDataChanged()							{return dataChanged;}

	inline void setObjectsLoaded(bool loaded = true) 		{objectsLoaded = loaded;}
	inline bool getObjectsLoaded()							{return objectsLoaded;}

	inline void setCustomChanges(bool changes = true) 		{customChanges = changes;}
	inline bool getCustomChanges()							{return customChanges;}

protected:

	/// Does the surface data need an update because voxels were changed?
	bool dataChanged;

	/// Has the loadObjects() function been called yet?
	bool objectsLoaded;

	/// Were custom changes made to this block? If yes, it must be stored in the gameDataStorage when the players get out of range.
	bool customChanges;
};

#endif
