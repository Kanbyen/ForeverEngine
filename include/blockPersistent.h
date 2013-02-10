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

#ifndef _BLOCK_PERSISTENT_H_
#define _BLOCK_PERSISTENT_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "marchingCubesTables.h"
#include "surfacePatch.h"
#include "surfacePatchRenderable.h"
#include "blockConstants.h"
#include "point3.h"

// Helper variables for getMeshData and getVoxelData
extern Ogre::uint* tempIndexField;
extern Vector3* tempVertexData;
extern Ogre::uint* tempVertexNumber;
extern Ogre::uint* tempVertexTextures;	// for each vertex: which texture information does the shader get (the same for the whole triangle)?
extern float* tempTextureData;
extern Ogre::uint tempVertexDataSize;
extern Ogre::uint* tempIndexData;
extern Ogre::uint tempIndexDataSize;

class VoxelVolume;
class VoxelBlock;

/// Holds reusable data for voxel blocks
class VoxelBlockPersistent
{
public:
	/// Pointer to the volume this block belongs to
	VoxelVolume* volume;

	/// Offset from the origin of the volume in blocks
	Point3 position;

	// Rendering stuff
	Vector3 positionVec;
	SceneNode* sceneNode;
	SurfacePatch* surface;
	SurfacePatchRenderable* surfaceRenderable;

	/// Constructor - takes the block's position offset from the volume origin
	VoxelBlockPersistent(int x, int y, int z, VoxelVolume* volume);
	~VoxelBlockPersistent();

	/// Must be called when a new voxel block owns this object with the new block's (absolute) position
	void changeOwner(int x, int y, int z);

	/// Is called in VoxelVolume::updateGeometry(). The surface geometry is updated here if the data has changed
	void update();

	/// Call on a loaded block to create the collision data
	void createCollisionShape() 		{if (!surface->shapeCreated) surfaceRenderable->createCollisionShape();}

	/// Returns if the block has surface data. Can only be true for loaded blocks.
	bool hasSurface()					{return (surface && surfaceRenderable);}

	// Geometry generation functions
	void getMeshData(int lastX, int lastY, int lastZ);
	void getMeshDataInner(int lastX, int lastY, int lastZ, VoxelBlock* block);

	inline void applyMarchingCubes(Ogre::uchar& iCubeIndex, float* factorlist, Vector3* poslist1, Vector3* poslist2, int* indexFieldIndex, Vector3* vertlist, unsigned char* texlist, char v000, char v100, char v010, char v110, char v001, char v101, char v011, char v111, unsigned char t000, unsigned char t100, unsigned char t010, unsigned char t110, unsigned char t001, unsigned char t101, unsigned char t011, unsigned char t111, int x, int y, int z);
	inline Vector3 calcNormal(Vector3 pos1, Vector3 pos2, float factor);
	inline Vector3 vertexInterp(Vector3 start, Vector3 end, float factor);
	inline void getVertexData(int i, int texture, Ogre::uint vertexTextures);
	inline void getVertexDataNoLimits(int i, int texture, Ogre::uint vertexTextures);
	inline void getTriangleData(int i0, int i1, int i2);
	inline void getVoxelData(char v000, char v100, char v010, char v110, char v001, char v101, char v011, char v111, unsigned char t000, unsigned char t100, unsigned char t010, unsigned char t110, unsigned char t001, unsigned char t101, unsigned char t011, unsigned char t111, int x, int y, int z);

	// Helper variables for triangle generation
	static int indexFieldIndex[12];
	static Vector3 vertlist[12];
	static unsigned char texlist[12];
	static Vector3 poslist[2][12];
	static float factorlist[12];
	
	static void initStatic();
	static void exitStatic();
};

#include "blockPersistent.inl"

#endif
