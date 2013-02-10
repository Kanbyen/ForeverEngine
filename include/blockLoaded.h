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

#ifndef _BLOCK_LOADED_H_
#define _BLOCK_LOADED_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "blockConstants.h"
#include "block.h"

class VoxelVolume;

/// A fully loaded voxel block with voxel and texture data
class VoxelBlockLoaded : public VoxelBlock
{
public:
	/// Voxel values
	signed char data[NUM_VOXELS_IN_BLOCK];
	/// Texture values
	unsigned char texture[NUM_VOXELS_IN_BLOCK];

	/// Constructor
	VoxelBlockLoaded(int blockX, int blockY, int blockZ, VoxelVolume* volume, VoxelBlockPersistent* persistent);
	~VoxelBlockLoaded();

	virtual bool innerVoxelsRelevant() {return true;}
	virtual int typeID() {return 0;}

	// Voxel editing functions
	virtual void fill(char value);
	void fillTex(unsigned char value);

	virtual char getVoxelAt(int xPosition, int yPosition, int zPosition)								{return _getVoxelAt(xPosition, yPosition, zPosition);}
	virtual void setVoxelAt(int xPosition, int yPosition, int zPosition, char value)					{_setVoxelAt(xPosition, yPosition, zPosition, value);}

	virtual unsigned char getTextureAt(int xPosition, int yPosition, int zPosition)						{return _getTextureAt(xPosition, yPosition, zPosition);}
	virtual void setTextureAt(int xPosition, int yPosition, int zPosition, const unsigned char value)	{_setTextureAt(xPosition, yPosition, zPosition, value);}

	// Fast inline versions, should be preferred if you know that you're working with a VoxelBlockLoaded
	inline char _getVoxelAt(int xPosition, int yPosition, int zPosition);
	inline void _setVoxelAt(int xPosition, int yPosition, int zPosition, char value);

	inline unsigned char _getTextureAt(int xPosition, int yPosition, int zPosition);
	inline void _setTextureAt(int xPosition, int yPosition, int zPosition, const unsigned char value);
};

char VoxelBlockLoaded::_getVoxelAt(int xPosition, int yPosition, int zPosition)
{
	return data[xPosition +
			yPosition * BLOCK_SIDE_LENGTH +
			zPosition * BLOCK_SIDE_LENGTH * BLOCK_SIDE_LENGTH];
}

void VoxelBlockLoaded::_setVoxelAt(int xPosition, int yPosition, int zPosition, char value)
{
	if (value == 0)
		value--;
	data[xPosition +
			yPosition * BLOCK_SIDE_LENGTH +
			zPosition * BLOCK_SIDE_LENGTH * BLOCK_SIDE_LENGTH] = value;
}

unsigned char VoxelBlockLoaded::_getTextureAt(int xPosition, int yPosition, int zPosition)
{
	return texture[xPosition +
			yPosition * BLOCK_SIDE_LENGTH +
			zPosition * BLOCK_SIDE_LENGTH * BLOCK_SIDE_LENGTH];
}

void VoxelBlockLoaded::_setTextureAt(int xPosition, int yPosition, int zPosition, const unsigned char value)
{
	texture[xPosition +
			yPosition * BLOCK_SIDE_LENGTH +
			zPosition * BLOCK_SIDE_LENGTH * BLOCK_SIDE_LENGTH] = value;
}

#endif
