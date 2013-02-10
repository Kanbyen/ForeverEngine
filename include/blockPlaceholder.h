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

#ifndef _BLOCK_PLACEHOLDER_H_
#define _BLOCK_PLACEHOLDER_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "block.h"
#include "blockConstants.h"

/// Is a placeholder for another subclass of VoxelBlock until the other block is loaded. Doesn't allow any changes, returns BLOCK_FULL_VALUE for all voxels so that AI agents don't try to fly into these blocks.
class VoxelBlockPlaceholder : public VoxelBlock
{
public:
	bool isBeingLoaded;

	/// Contructor
	VoxelBlockPlaceholder(int blockX, int blockY, int blockZ, VoxelVolume* volume, VoxelBlockPersistent* persistent);
	~VoxelBlockPlaceholder();

	virtual bool isPlaceholder() {return true;}
	virtual int typeID() {return 3;}

	// Voxel editing functions
	virtual void fill(char value);

	virtual char getVoxelAt(int xPosition, int yPosition, int zPosition);
	virtual void setVoxelAt(int xPosition, int yPosition, int zPosition, const char value);

	virtual unsigned char getTextureAt(int xPosition, int yPosition, int zPosition);
	virtual void setTextureAt(int xPosition, int yPosition, int zPosition, const unsigned char value);
};

#endif
