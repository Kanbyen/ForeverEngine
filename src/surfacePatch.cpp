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
#include "surfacePatch.h"
#include "physics.h"

SurfacePatch::SurfacePatch(Vector3* vertexData, float* textureData, Ogre::uint* vertexNumbers, int vertexDataSize, Ogre::uint* indexData, int indexDataSize, Vector3 offset, float scale)
{
	this->scale = scale;
	this->offset = offset;
	shapeCreated = false;

	numVertices = vertexDataSize / 2;
	numIndices = indexDataSize;

	// copy vertices
	vertices = new Vector3[vertexDataSize];
	memcpy(vertices, vertexData, vertexDataSize * sizeof(Vector3));

	// copy textures
	textures = new float[numVertices];
	memcpy(textures, textureData, numVertices * sizeof(float));

	// copy vertex numbers
	vertNumbers = new Ogre::uint[numVertices];
	memcpy(vertNumbers, vertexNumbers, numVertices * sizeof(Ogre::uint));

	// copy indices
	indices = new Ogre::uint[indexDataSize];
	memcpy(indices, indexData, indexDataSize * sizeof(Ogre::uint));

	// trimesh creation by copy
	/*btTriangleMesh*         ivArrays;
    ivArrays = new btTriangleMesh();
    unsigned int numFaces = indexDataSize / 3;
    btVector3    vertexPos[3];
	ulong indexNr = 0;
    for (size_t n = 0; n < numFaces; ++n)
    {
        for (unsigned int i = 0; i < 3; ++i)
        {
            vertexPos[i][0] = vertices[indices[indexNr]*2].x;
            vertexPos[i][1] = vertices[indices[indexNr]*2].y;
            vertexPos[i][2] = vertices[indices[indexNr]*2].z;
			indexNr++;
        }
        ivArrays->addTriangle(vertexPos[0], vertexPos[1], vertexPos[2]);
    }*/
}

void SurfacePatch::createCollisionShape(Vector3 aabbMin, Vector3 aabbMax)
{
	if (shapeCreated)
		return;

	shapeCreated = true;

	// create collision shape and rigid body
	ivArrays = new btTriangleIndexVertexArray(numIndices / 3, (int*)indices, 3 * sizeof(int), numVertices, (btScalar*)&vertices[0].x, 2 * sizeof(Vector3));
	shape = new btBvhTriangleMeshShape(ivArrays, true, btVector3(aabbMin.x, aabbMin.y, aabbMin.z), btVector3(aabbMax.x, aabbMax.y, aabbMax.z));

	btTransform trans;
	trans.setIdentity();
	trans.setOrigin(btVector3(offset.x * scale, offset.y * scale, offset.z * scale));
	btVector3 localInertia(0, 0, 0);
	myMotionState = new btDefaultMotionState(trans);
	btRigidBody::btRigidBodyConstructionInfo cInfo(0.f, myMotionState, shape, localInertia);
	body = new btRigidBody(cInfo);
	body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

	physics.addStatic(body);
}

SurfacePatch::~SurfacePatch()
{
	delete[] vertices;
	delete[] indices;

	if (shapeCreated)
	{
		physics.removeStatic(body);
		delete body;
		delete shape;
		delete myMotionState;
		delete ivArrays;
	}
}
