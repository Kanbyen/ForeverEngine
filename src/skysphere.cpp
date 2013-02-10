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
#include "skysphere.h"

Entity* SkySphere::entity;
int SkySphere::nextSceneNodeNumber = 0;

const char* SKY_SPHERE_MESH_NAME = "SkySphere";
const int SKY_SPHERE_RINGS = 10;
const int SKY_SPHERE_SEGMENTS = 18;

void SkySphere::init()
{
    entity = NULL;

    const float radius = 1;
    const int nRings = SKY_SPHERE_RINGS;
    const int nSegments = SKY_SPHERE_SEGMENTS;

	MeshPtr pSphere = MeshManager::getSingleton().createManual(SKY_SPHERE_MESH_NAME, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	SubMesh *pSphereVertex = pSphere->createSubMesh();
	//pSphere->sharedVertexData = new VertexData();
	//pSphereVertex->useSharedVertices = true;
	pSphereVertex->vertexData = new VertexData();
	pSphereVertex->useSharedVertices = false;

	VertexData* vertexData = pSphereVertex->vertexData; //pSphere->sharedVertexData;
	IndexData* indexData = pSphereVertex->indexData;

    // define the vertex format
	VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
	size_t currOffset = 0;

	// positions
	vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION);
	currOffset += VertexElement::getTypeSize(VET_FLOAT3);

    // colors
    vertexDecl->addElement(0, currOffset, VET_FLOAT1, VES_TEXTURE_COORDINATES);
    currOffset += VertexElement::getTypeSize(VET_FLOAT1);

	// allocate the vertex buffer
	vertexData->vertexCount = (nRings + 1) * (nSegments+1);
	HardwareVertexBufferSharedPtr vBuf = HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(0), vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
	VertexBufferBinding* binding = vertexData->vertexBufferBinding;
	binding->setBinding(0, vBuf);
	float* pVertex = static_cast<float*>(vBuf->lock(HardwareBuffer::HBL_DISCARD));

	// allocate index buffer
	indexData->indexCount = 6 * nRings * (nSegments + 1);
	indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
	HardwareIndexBufferSharedPtr iBuf = indexData->indexBuffer;
	unsigned short* pIndices = static_cast<unsigned short*>(iBuf->lock(HardwareBuffer::HBL_DISCARD));

	float fDeltaRingAngle = (Math::PI / nRings);
	float fDeltaSegAngle = (2 * Math::PI / nSegments);
	unsigned short wVerticeIndex = 0 ;

	// Generate the group of rings for the sphere
	for( int ring = 0; ring <= nRings; ring++ ) {
		float r0 = radius * sinf (ring * fDeltaRingAngle);
		float y0 = radius * cosf (ring * fDeltaRingAngle);

		// Generate the group of segments for the current ring
		for(int seg = 0; seg <= nSegments; seg++) {
			float x0 = r0 * sinf(seg * fDeltaSegAngle);
			float z0 = r0 * cosf(seg * fDeltaSegAngle);

			// Add one vertex to the strip which makes up the sphere
			*pVertex++ = x0;
			*pVertex++ = y0;
			*pVertex++ = z0;

            Vector3 vNormal = Vector3(x0, y0, z0).normalisedCopy();

            // Calculate color
            const Vector3 dirToLight = Vector3(0.0f, 1.0f, 0.0f); // from the left: Vector3(0.0f, 0.957826285f, -0.287347886f);
			const float texSize = 512.0f;
            float factor = (dirToLight.dotProduct(vNormal) * 0.5f + 0.5f) * (texSize-2)/texSize + 1.0f/texSize;
            *pVertex++ = factor;

			if (ring != nRings)
			{
				// each vertex (except the last) has six indices pointing to it
				*pIndices++ = wVerticeIndex + nSegments + 1;
				*pIndices++ = wVerticeIndex;
				*pIndices++ = wVerticeIndex + nSegments;
				*pIndices++ = wVerticeIndex + nSegments + 1;
				*pIndices++ = wVerticeIndex + 1;
				*pIndices++ = wVerticeIndex;
				wVerticeIndex ++;
			}
		}; // end for seg
	} // end for ring

	// Unlock
	vBuf->unlock();
	iBuf->unlock();

    // Set bounds
	pSphere->_setBounds( AxisAlignedBox( Vector3(-radius, -radius, -radius), Vector3(radius, radius, radius) ), false );
	pSphere->_setBoundingSphereRadius(radius);
	// This line makes clear the mesh is loaded (avoids memory leaks)
	pSphere->load();
}

SkySphere::SkySphere(const char* material)
{
    entity = sceneMgr->createEntity("SkySphereEnt " + StringConverter::toString(nextSceneNodeNumber), SKY_SPHERE_MESH_NAME);
    entity->setMaterialName(material);
    entity->setRenderQueueGroup(RENDER_QUEUE_WORLD_GEOMETRY_2 - 1);
	entity->setCastShadows(false);
    sceneNode = sceneMgr->getRootSceneNode()->createChildSceneNode("SkySphereNode " + StringConverter::toString(nextSceneNodeNumber++));
	sceneNode->attachObject(entity);
}

SkySphere::~SkySphere()
{
    sceneNode->detachAllObjects();
    sceneMgr->destroyEntity(entity);
    sceneMgr->destroySceneNode(sceneNode->getName());
}

void SkySphere::setVisible(bool visible)
{
    sceneNode->setVisible(visible);
}

void SkySphere::update(Camera* cam, float radius)
{
    sceneNode->setPosition(cam->getDerivedPosition());
    sceneNode->setScale(radius, radius, radius);
}
