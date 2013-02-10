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
#include "blockPersistent.h"
#include "blockLoaded.h"
#include "volume.h"
#include "digiSpe.h"

Ogre::uint* tempIndexField;

Vector3* tempVertexData = NULL;		// this is initialized in VoxelBlockPersistent::initStatic(). Having it on the stack like the other arrays lead to crashes when using gdb.
Ogre::uint* tempVertexNumber;
Ogre::uint* tempVertexTextures;
float* tempTextureData;
Ogre::uint tempVertexDataSize;

Ogre::uint* tempIndexData;
Ogre::uint tempIndexDataSize;

int VoxelBlockPersistent::indexFieldIndex[12];
Vector3 VoxelBlockPersistent::vertlist[12];
unsigned char VoxelBlockPersistent::texlist[12];
Vector3 VoxelBlockPersistent::poslist[2][12];
float VoxelBlockPersistent::factorlist[12];

void VoxelBlockPersistent::initStatic()
{
	tempIndexField = new Ogre::uint[TEMP_INDEX_FIELD_SIZE];
	tempVertexData = new Vector3[NUM_VOXELS_IN_BLOCK * 15 * 3];
	tempVertexNumber = new Ogre::uint[NUM_VOXELS_IN_BLOCK * 15 * 3];
	tempVertexTextures = new Ogre::uint[NUM_VOXELS_IN_BLOCK * 15 * 3];
	tempTextureData = new float[NUM_VOXELS_IN_BLOCK * 15 * 3];
	tempIndexData = new Ogre::uint[NUM_VOXELS_IN_BLOCK * 15 * 3];
}
void VoxelBlockPersistent::exitStatic()
{
	delete tempIndexField;
	delete tempVertexData;
	delete tempVertexNumber;
	delete tempVertexTextures;
	delete tempTextureData;
	delete tempIndexData;
}

VoxelBlockPersistent::VoxelBlockPersistent(int x, int y, int z, VoxelVolume* volume)
{
	this->volume = volume;
	surface = NULL;

	sceneNode = sceneMgr->getRootSceneNode()->createChildSceneNode("VoxelBlock(" + StringConverter::toString(x) + "," + StringConverter::toString(y) + "," + StringConverter::toString(z) + ")", positionVec);
	surfaceRenderable = (SurfacePatchRenderable*)sceneMgr->createMovableObject("SurfacePatchRenderable(" + StringConverter::toString(x) + "," + StringConverter::toString(y) + "," + StringConverter::toString(z) + ")", "SurfacePatchRenderable");
	sceneNode->attachObject(surfaceRenderable);
}
VoxelBlockPersistent::~VoxelBlockPersistent()
{
	sceneMgr->destroyMovableObject("SurfacePatchRenderable(" + StringConverter::toString(position.x) + "," + StringConverter::toString(position.y) + "," + StringConverter::toString(position.z) + ")", "SurfacePatchRenderable");
	sceneMgr->destroySceneNode(sceneNode->getName());
	delete surface;
}

void VoxelBlockPersistent::changeOwner(int x, int y, int z)
{
	position = Point3(x, y, z);
	positionVec = Vector3(x * BLOCK_SIDE_LENGTH, y * BLOCK_SIDE_LENGTH, z * BLOCK_SIDE_LENGTH);

	surfaceRenderable->clearGeometry();

	delete surface;
	surface = NULL;
}

void VoxelBlockPersistent::update()
{
	// Get the Block
	VoxelBlock* myBlock = volume->getBlockAbs(position.x, position.y, position.z);

	if (!myBlock->getDataChanged())
		return;
	
	if (myBlock->isPlaceholder())
	{
		myBlock->setDataChanged(false);
		return;
	}

	int myTypeID = myBlock->typeID();
	bool otherTypeID = myBlock->innerVoxelsRelevant();

	// Only update if this block is in the center of a 3x3 group of loaded blocks
	for (int dx = -1; dx < 2; dx++)
	{
		for (int dy = -1; dy < 2; dy++)
		{
			for (int dz = -1; dz < 2; dz++)
			{
				if (volume->pagingMode == PAGINGMODE_OPEN)
				{
					// Don't update boundary blocks in open mode
					if ((position.x + dx - volume->windowXInBlocks < 0) ||
						(position.x + dx - volume->windowXInBlocks >= volume->numBlocksX))
						return;
					if ((position.y + dy - volume->windowYInBlocks < 0) ||
						(position.y + dy - volume->windowYInBlocks >= volume->numBlocksY))
						return;
					if ((position.z + dz - volume->windowZInBlocks < 0) ||
						(position.z + dz - volume->windowZInBlocks >= volume->numBlocksZ))
						return;
				}
				else
				{
					if ((position.x + dx - volume->windowXInBlocks < 0) ||
						(position.x + dx - volume->windowXInBlocks >= volume->numBlocksX))
						continue;
					if ((position.y + dy - volume->windowYInBlocks < 0) ||
						(position.y + dy - volume->windowYInBlocks >= volume->numBlocksY))
						continue;
					if ((position.z + dz - volume->windowZInBlocks < 0) ||
						(position.z + dz - volume->windowZInBlocks >= volume->numBlocksZ))
						continue;
				}

				VoxelBlock* block = volume->getBlockAbs(position.x + dx, position.y + dy, position.z + dz);
				if (block->isPlaceholder())
					return;
				if (!otherTypeID && (block->typeID() != myTypeID))
					otherTypeID = true;
			}
		}
	}

	myBlock->setDataChanged(false);
	
	// Recreate geometry for this block ...
	sceneNode->setPosition(positionVec * volume->scale);
	surfaceRenderable->clearGeometry();

	if (otherTypeID)
	{
		int lastX = BLOCK_SIDE_LENGTH - 1;
		int lastY = BLOCK_SIDE_LENGTH - 1;
		int lastZ = BLOCK_SIDE_LENGTH - 1;

		if (volume->pagingMode == PAGINGMODE_CLOSED)
		{
			if (position.x - volume->windowXInBlocks == volume->numBlocksX - 1)	lastX--;
			if (position.y - volume->windowYInBlocks == volume->numBlocksY - 1) lastY--;
			if (position.z - volume->windowZInBlocks == volume->numBlocksZ - 1) lastZ--;
		}

		// Apply marching cubes
		getMeshData(lastX, lastY, lastZ);
		if (myBlock->innerVoxelsRelevant())
			getMeshDataInner(lastX, lastY, lastZ, myBlock);
	}
	else
		tempVertexDataSize = 0;

	// delete old surface data
	if (surface)
	{
		delete surface;
		surface = NULL;
	}

	if (tempVertexDataSize <= 0)
	{
		myBlock->loadObjects();
		return;
	}

	// Copy data into a SurfacePatch where it will also be accessible for the physics engine and update the renderable
	surface = new SurfacePatch(tempVertexData, tempTextureData, tempVertexNumber, tempVertexDataSize, tempIndexData, tempIndexDataSize, positionVec, volume->scale);
	surfaceRenderable->setGeometry(surface, volume->materialName.c_str());
	surfaceRenderable->createHardwareBuffers();

	sceneNode->needUpdate();

	myBlock->loadObjects();
}

void VoxelBlockPersistent::getMeshData(int lastX, int lastY, int lastZ)
{
	memset(tempIndexField, 255, TEMP_INDEX_FIELD_SIZE * sizeof(Ogre::uint));
	tempVertexDataSize = 0;
	tempIndexDataSize = 0;

	Point3 relPos = Point3((position.x - volume->windowXInBlocks) * BLOCK_SIDE_LENGTH, (position.y - volume->windowYInBlocks) * BLOCK_SIDE_LENGTH, (position.z - volume->windowZInBlocks) * BLOCK_SIDE_LENGTH);

	// For every outer voxel ...
	// handle +Y area
	for(int z = 0; z <= lastZ - 1; z++)
	{
		const int y = lastY;
		for(int x = 0; x <= lastX - 1; x++)
		{
			const char v000 = volume->getVoxelRel(relPos.x + x  , relPos.y + y  , relPos.z + z  );
			const char v100 = volume->getVoxelRel(relPos.x + x+1, relPos.y + y  , relPos.z + z  );
			const char v010 = volume->getVoxelRel(relPos.x + x  , relPos.y + y+1, relPos.z + z  );
			const char v110 = volume->getVoxelRel(relPos.x + x+1, relPos.y + y+1, relPos.z + z  );
			const char v001 = volume->getVoxelRel(relPos.x + x  , relPos.y + y  , relPos.z + z+1);
			const char v101 = volume->getVoxelRel(relPos.x + x+1, relPos.y + y  , relPos.z + z+1);
			const char v011 = volume->getVoxelRel(relPos.x + x  , relPos.y + y+1, relPos.z + z+1);
			const char v111 = volume->getVoxelRel(relPos.x + x+1, relPos.y + y+1, relPos.z + z+1);
			const unsigned char t000 = volume->getTextureRel(relPos.x + x  , relPos.y + y  , relPos.z + z  );
			const unsigned char t100 = volume->getTextureRel(relPos.x + x+1, relPos.y + y  , relPos.z + z  );
			const unsigned char t010 = volume->getTextureRel(relPos.x + x  , relPos.y + y+1, relPos.z + z  );
			const unsigned char t110 = volume->getTextureRel(relPos.x + x+1, relPos.y + y+1, relPos.z + z  );
			const unsigned char t001 = volume->getTextureRel(relPos.x + x  , relPos.y + y  , relPos.z + z+1);
			const unsigned char t101 = volume->getTextureRel(relPos.x + x+1, relPos.y + y  , relPos.z + z+1);
			const unsigned char t011 = volume->getTextureRel(relPos.x + x  , relPos.y + y+1, relPos.z + z+1);
			const unsigned char t111 = volume->getTextureRel(relPos.x + x+1, relPos.y + y+1, relPos.z + z+1);

			getVoxelData(v000, v100, v010, v110, v001, v101, v011, v111, t000, t100, t010, t110, t001, t101, t011, t111, x, y, z);
		}
	}

	// handle +X area
	for(int z = 0; z <= lastZ - 1; z++)
	{
		const int x = lastX;
		for(int y = 0; y <= lastY; y++)
		{
			const char v000 = volume->getVoxelRel(relPos.x + x  , relPos.y + y  , relPos.z + z  );
			const char v100 = volume->getVoxelRel(relPos.x + x+1, relPos.y + y  , relPos.z + z  );
			const char v010 = volume->getVoxelRel(relPos.x + x  , relPos.y + y+1, relPos.z + z  );
			const char v110 = volume->getVoxelRel(relPos.x + x+1, relPos.y + y+1, relPos.z + z  );
			const char v001 = volume->getVoxelRel(relPos.x + x  , relPos.y + y  , relPos.z + z+1);
			const char v101 = volume->getVoxelRel(relPos.x + x+1, relPos.y + y  , relPos.z + z+1);
			const char v011 = volume->getVoxelRel(relPos.x + x  , relPos.y + y+1, relPos.z + z+1);
			const char v111 = volume->getVoxelRel(relPos.x + x+1, relPos.y + y+1, relPos.z + z+1);
			const unsigned char t000 = volume->getTextureRel(relPos.x + x  , relPos.y + y  , relPos.z + z  );
			const unsigned char t100 = volume->getTextureRel(relPos.x + x+1, relPos.y + y  , relPos.z + z  );
			const unsigned char t010 = volume->getTextureRel(relPos.x + x  , relPos.y + y+1, relPos.z + z  );
			const unsigned char t110 = volume->getTextureRel(relPos.x + x+1, relPos.y + y+1, relPos.z + z  );
			const unsigned char t001 = volume->getTextureRel(relPos.x + x  , relPos.y + y  , relPos.z + z+1);
			const unsigned char t101 = volume->getTextureRel(relPos.x + x+1, relPos.y + y  , relPos.z + z+1);
			const unsigned char t011 = volume->getTextureRel(relPos.x + x  , relPos.y + y+1, relPos.z + z+1);
			const unsigned char t111 = volume->getTextureRel(relPos.x + x+1, relPos.y + y+1, relPos.z + z+1);

			getVoxelData(v000, v100, v010, v110, v001, v101, v011, v111, t000, t100, t010, t110, t001, t101, t011, t111, x, y, z);
		}
	}

	// handle +Z area
	for(int x = 0; x <= lastX; x++)
	{
		const int z = lastZ;
		for(int y = 0; y <= lastY; y++)
		{
			const char v000 = volume->getVoxelRel(relPos.x + x  , relPos.y + y  , relPos.z + z  );
			const char v100 = volume->getVoxelRel(relPos.x + x+1, relPos.y + y  , relPos.z + z  );
			const char v010 = volume->getVoxelRel(relPos.x + x  , relPos.y + y+1, relPos.z + z  );
			const char v110 = volume->getVoxelRel(relPos.x + x+1, relPos.y + y+1, relPos.z + z  );
			const char v001 = volume->getVoxelRel(relPos.x + x  , relPos.y + y  , relPos.z + z+1);
			const char v101 = volume->getVoxelRel(relPos.x + x+1, relPos.y + y  , relPos.z + z+1);
			const char v011 = volume->getVoxelRel(relPos.x + x  , relPos.y + y+1, relPos.z + z+1);
			const char v111 = volume->getVoxelRel(relPos.x + x+1, relPos.y + y+1, relPos.z + z+1);
			const unsigned char t000 = volume->getTextureRel(relPos.x + x  , relPos.y + y  , relPos.z + z  );
			const unsigned char t100 = volume->getTextureRel(relPos.x + x+1, relPos.y + y  , relPos.z + z  );
			const unsigned char t010 = volume->getTextureRel(relPos.x + x  , relPos.y + y+1, relPos.z + z  );
			const unsigned char t110 = volume->getTextureRel(relPos.x + x+1, relPos.y + y+1, relPos.z + z  );
			const unsigned char t001 = volume->getTextureRel(relPos.x + x  , relPos.y + y  , relPos.z + z+1);
			const unsigned char t101 = volume->getTextureRel(relPos.x + x+1, relPos.y + y  , relPos.z + z+1);
			const unsigned char t011 = volume->getTextureRel(relPos.x + x  , relPos.y + y+1, relPos.z + z+1);
			const unsigned char t111 = volume->getTextureRel(relPos.x + x+1, relPos.y + y+1, relPos.z + z+1);

			getVoxelData(v000, v100, v010, v110, v001, v101, v011, v111, t000, t100, t010, t110, t001, t101, t011, t111, x, y, z);
		}
	}
}

void VoxelBlockPersistent::getMeshDataInner(int lastX, int lastY, int lastZ, VoxelBlock* block)
{
	VoxelBlockLoaded* bl = (VoxelBlockLoaded*)block;
	
	// For every inner voxel ...
	for(int z = 0; z <= lastZ - 1; z++)
	{
		for(int y = 0; y <= lastY - 1; y++)
		{
			for(int x = 0; x <= lastX - 1; x++)
			{
				const char v000 = bl->_getVoxelAt(x  ,y  ,z  );
				const char v100 = bl->_getVoxelAt(x+1,y  ,z  );
				const char v010 = bl->_getVoxelAt(x  ,y+1,z  );
				const char v110 = bl->_getVoxelAt(x+1,y+1,z  );
				const char v001 = bl->_getVoxelAt(x  ,y  ,z+1);
				const char v101 = bl->_getVoxelAt(x+1,y  ,z+1);
				const char v011 = bl->_getVoxelAt(x  ,y+1,z+1);
				const char v111 = bl->_getVoxelAt(x+1,y+1,z+1);
				const unsigned char t000 = bl->_getTextureAt(x  ,y  ,z  );
				const unsigned char t100 = bl->_getTextureAt(x+1,y  ,z  );
				const unsigned char t010 = bl->_getTextureAt(x  ,y+1,z  );
				const unsigned char t110 = bl->_getTextureAt(x+1,y+1,z  );
				const unsigned char t001 = bl->_getTextureAt(x  ,y  ,z+1);
				const unsigned char t101 = bl->_getTextureAt(x+1,y  ,z+1);
				const unsigned char t011 = bl->_getTextureAt(x  ,y+1,z+1);
				const unsigned char t111 = bl->_getTextureAt(x+1,y+1,z+1);

				getVoxelData(v000, v100, v010, v110, v001, v101, v011, v111, t000, t100, t010, t110, t001, t101, t011, t111, x, y, z);
			}
		}
	}
}
