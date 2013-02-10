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

#ifndef _SURFACE_PATCH_H_
#define _SURFACE_PATCH_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "btBulletDynamicsCommon.h"

class SurfacePatch
{
public:
	// Data
	float scale;
	Vector3 offset;

	int numVertices;
	Vector3* vertices;
	float* textures;
	Ogre::uint* vertNumbers;

	int numIndices;
	Ogre::uint* indices;

	// Physics data
	bool shapeCreated;
	btCollisionShape* shape;
	btTriangleIndexVertexArray* ivArrays;
	btDefaultMotionState* myMotionState;
	btRigidBody* body;

	SurfacePatch(Vector3* vertexData, float* textureData, Ogre::uint* vertexNumbers, int vertexDataSize, Ogre::uint* indexData, int indexDataSize, Vector3 offset, float scale);
	~SurfacePatch();

	void createCollisionShape(Vector3 aabbMin, Vector3 aabbMax);
};

#endif
