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
#include "surfacePatchRenderable.h"
#include "block.h"

SurfacePatchRenderable::SurfacePatchRenderable() : SimpleRenderable()
{
	// Set up what we can of the vertex data
	mRenderOp.vertexData = new VertexData();
	mRenderOp.vertexData->vertexStart = 0;
	mRenderOp.vertexData->vertexCount = 0;
	mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;

	// Set up what we can of the index data
	mRenderOp.indexData = new IndexData();
	mRenderOp.useIndexes = true;
	mRenderOp.indexData->indexStart = 0;
	mRenderOp.indexData->indexCount = 0;

	// Set up the vertex declaration
	VertexDeclaration *decl = mRenderOp.vertexData->vertexDeclaration;
	decl->removeAllElements();
	decl->addElement(0, 0, VET_FLOAT3, VES_POSITION);
	decl->addElement(0, 3 * sizeof(float), VET_FLOAT3, VES_NORMAL);
	decl->addElement(0, 6 * sizeof(float), VET_FLOAT3, VES_TEXTURE_COORDINATES);
	decl->addElement(0, 9 * sizeof(float), VET_FLOAT3, VES_TEXTURE_COORDINATES, 1);
}

SurfacePatchRenderable::~SurfacePatchRenderable()
{
	clearGeometry();
	delete mRenderOp.vertexData;
	delete mRenderOp.indexData;
}

void SurfacePatchRenderable::clearGeometry()
{
	mRenderOp.vertexData->vertexBufferBinding->unsetAllBindings();
	mRenderOp.vertexData->vertexCount = 0;
	mRenderOp.indexData->indexCount = 0;
}

void SurfacePatchRenderable::createHardwareBuffers()
{
	VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;

	HardwareVertexBufferSharedPtr vbuf =
		HardwareBufferManager::getSingleton().createVertexBuffer(
		mRenderOp.vertexData->vertexDeclaration->getVertexSize(0),
		mRenderOp.vertexData->vertexCount,
  		HardwareBuffer::HBU_STATIC_WRITE_ONLY,
		false);

	bind->setBinding(0, vbuf);

	HardwareIndexBufferSharedPtr ibuf =
		HardwareBufferManager::getSingleton().createIndexBuffer(
		HardwareIndexBuffer::IT_32BIT, // type of index
		mRenderOp.indexData->indexCount, // number of indexes
  		HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
		false); // no shadow buffer	

	mRenderOp.indexData->indexBuffer = ibuf;	

	Vector3 vaabMin(std::numeric_limits<Real>::max(), std::numeric_limits<Real>::max(), std::numeric_limits<Real>::max());
	Vector3 vaabMax(0.0, 0.0, 0.0);
	
	Real* prPos = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));

	const float textureWeights[3][3] = {{1, 0, 0},
										{0, 1, 0},
  										{0, 0, 1}};
										
	/*const int textureNumberOffsets[3][3] = {{0, 1, 2},
											{-1, 0, 1},
		   									{-2, -1, 0}};*/
	
	for (int v = 0; v < patch->numVertices * 2; v += 2)
	{
		// Position
		*prPos++ = patch->vertices[v].x;
		*prPos++ = patch->vertices[v].y;
		*prPos++ = patch->vertices[v].z;

		// Normal
		*prPos++ = patch->vertices[v+1].x;
		*prPos++ = patch->vertices[v+1].y;
		*prPos++ = patch->vertices[v+1].z;

		// texture weights
		const int curIndex = patch->vertNumbers[v/2];
		const int posInTriangle = curIndex % 3;
		*prPos++ = textureWeights[posInTriangle][0];
		*prPos++ = textureWeights[posInTriangle][1];
		*prPos++ = textureWeights[posInTriangle][2];

		// texture numbers
		for (int i = 0; i < posInTriangle; i++)
			*prPos++ = patch->textures[patch->indices[curIndex - (posInTriangle - i)]];
		*prPos++ = patch->textures[patch->indices[curIndex]];
		for (int i = posInTriangle + 1; i < 3; i++)
			*prPos++ = patch->textures[patch->indices[curIndex + (i - posInTriangle)]];

		// texture numbers
		//*prPos++ = patch->textures[(v/2)/3*3 + 0] + 0.5f;	// 0.5f: number between 0 and 1 as offset because in the shader,
		//*prPos++ = patch->textures[(v/2)/3*3 + 1] + 0.5f;	// floor() is used to determine the texture number as integer and this is
		//*prPos++ = patch->textures[(v/2)/3*3 + 2] + 0.5f;	// much too imprecise if the real texture numbers are passed in!

		// Adjust bounding box ...
		if (patch->vertices[v].x < vaabMin.x)
			vaabMin.x = patch->vertices[v].x;
		if (patch->vertices[v].y < vaabMin.y)
			vaabMin.y = patch->vertices[v].y;
		if (patch->vertices[v].z < vaabMin.z)
			vaabMin.z = patch->vertices[v].z;

		if (patch->vertices[v].x > vaabMax.x)
			vaabMax.x = patch->vertices[v].x;
		if (patch->vertices[v].y > vaabMax.y)
			vaabMax.y = patch->vertices[v].y;
		if (patch->vertices[v].z > vaabMax.z)
			vaabMax.z = patch->vertices[v].z;
	}		

	vbuf->unlock();

	mBox.setExtents(vaabMin, vaabMax);
	
	Ogre::uint* pIdx = static_cast<Ogre::uint*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
	for (int i = 0; i < patch->numIndices; i++)
	{
		*pIdx++ = patch->indices[i];
	}	

	ibuf->unlock();

	// Clean up the surface patch as much as possible
	delete[] patch->textures;
	delete[] patch->vertNumbers;
}

void SurfacePatchRenderable::createCollisionShape()
{
	patch->createCollisionShape(mBox.getMinimum(), mBox.getMaximum());
}

void SurfacePatchRenderable::setGeometry(SurfacePatch* patch, const String& material)
{
	// Initialization stuff
	setMaterial(material);

	mRenderOp.vertexData->vertexCount = patch->numVertices;		
	mRenderOp.indexData->indexCount = patch->numIndices;

	this->patch = patch;
}

Real SurfacePatchRenderable::getSquaredViewDepth(const Camera *cam) const
{
	Vector3 vMin, vMax, vMid, vDist;
	vMin = mBox.getMinimum();
	vMax = mBox.getMaximum();
	vMid = ((vMin - vMax) * 0.5) + vMin;
	vDist = cam->getDerivedPosition() - vMid;

	return vDist.squaredLength();
}

Real SurfacePatchRenderable::getBoundingRadius() const
{
	return Math::Sqrt((std::max)(mBox.getMaximum().squaredLength(), mBox.getMinimum().squaredLength()));
}

const Quaternion &SurfacePatchRenderable::getWorldOrientation() const
{
	return Quaternion::IDENTITY;
}

const Vector3 &SurfacePatchRenderable::getWorldPosition() const
{
	return Vector3::ZERO;
}

const String& SurfacePatchRenderable::getMovableType() const
{
	static String movType = "SurfacePatchRenderable";
    return movType;
}

//-----------------------------------------------------------------------
String SurfacePatchRenderableFactory::FACTORY_TYPE_NAME = "SurfacePatchRenderable";
//-----------------------------------------------------------------------
const String& SurfacePatchRenderableFactory::getType() const
{
	return FACTORY_TYPE_NAME;
}
//-----------------------------------------------------------------------
MovableObject* SurfacePatchRenderableFactory::createInstanceImpl(const String& name, const NameValuePairList* params)
{
	return new SurfacePatchRenderable();
}
//-----------------------------------------------------------------------
void SurfacePatchRenderableFactory::destroyInstance( MovableObject* obj)
{
	delete obj;
}
