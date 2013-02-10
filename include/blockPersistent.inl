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

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "volume.h"

Vector3 VoxelBlockPersistent::calcNormal(Vector3 pos1, Vector3 pos2, float factor)
{
	Vector3 absToRel = Vector3(volume->windowXInVoxels, volume->windowYInVoxels, volume->windowZInVoxels);
	pos1 -= absToRel;
	pos2 -= absToRel;

	Vector3 result, result2;
	float weight1 = 1, weight2 = 1;
	{
		int baseX = positionVec.x + pos1.x + 0.5f;
		int baseY = positionVec.y + pos1.y + 0.5f;
		int baseZ = positionVec.z + pos1.z + 0.5f;

		if (baseX <= 0)
			result = Vector3(-1.0f, 0.0f, 0.0f);
		else if (baseY <= 0)
			result = Vector3(0.0f, -1.0f, 0.0f);
		else if (baseZ <= 0)
			result = Vector3(0.0f, 0.0f, -1.0f);
		else if (baseX >= volume->numVoxelsX - 1)
			result = Vector3(1.0f, 0.0f, 0.0f);
		else if (baseY >= volume->numVoxelsY - 1)
			result = Vector3(0.0f, 1.0f, 0.0f);
		else if (baseZ >= volume->numVoxelsZ - 1)
			result = Vector3(0.0f, 0.0f, 1.0f);
		else
		{
			result = Vector3(volume->getVoxelRel(baseX - 1, baseY    , baseZ    ) -
					volume->getVoxelRel(baseX + 1, baseY    , baseZ    ),
									   volume->getVoxelRel(baseX    , baseY - 1, baseZ    ) -
											   volume->getVoxelRel(baseX    , baseY + 1, baseZ    ),
											   volume->getVoxelRel(baseX    , baseY    , baseZ - 1) -
													   volume->getVoxelRel(baseX    , baseY    , baseZ + 1));
			weight1 = result.length();
			if (weight1 > 1e-7)
				result /= weight1;
			else
				weight1 = 0;
		}
		//weight1 = abs(volume->getVoxelRel(baseX, baseY, baseZ));
	}
	{
		int baseX = positionVec.x + pos2.x + 0.5f;
		int baseY = positionVec.y + pos2.y + 0.5f;
		int baseZ = positionVec.z + pos2.z + 0.5f;

		if (baseX <= 0)
			result2 = Vector3(-1.0f, 0.0f, 0.0f);
		else if (baseY <= 0)
			result2 = Vector3(0.0f, -1.0f, 0.0f);
		else if (baseZ <= 0)
			result2 = Vector3(0.0f, 0.0f, -1.0f);
		else if (baseX >= volume->numVoxelsX - 1)
			result2 = Vector3(1.0f, 0.0f, 0.0f);
		else if (baseY >= volume->numVoxelsY - 1)
			result2 = Vector3(0.0f, 1.0f, 0.0f);
		else if (baseZ >= volume->numVoxelsZ - 1)
			result2 = Vector3(0.0f, 0.0f, 1.0f);
		else
		{
			result2 = Vector3(volume->getVoxelRel(baseX - 1, baseY    , baseZ    ) -
					volume->getVoxelRel(baseX + 1, baseY    , baseZ    ),
									   volume->getVoxelRel(baseX    , baseY - 1, baseZ    ) -
											   volume->getVoxelRel(baseX    , baseY + 1, baseZ    ),
											   volume->getVoxelRel(baseX    , baseY    , baseZ - 1) -
													   volume->getVoxelRel(baseX    , baseY    , baseZ + 1)) * factor;
			weight2 = result2.length();
			if (weight2 > 1e-7)
				result2 /= weight2;
			else
				weight2 = 0;
		}
		//weight2 = abs(volume->getVoxelRel(baseX, baseY, baseZ));
	}

	float weightsum = ((1 - factor) * weight1) + (factor * weight2);
	//assert(weightsum != 0);
	Vector3 result3 = (result * ((1 - factor) * weight1 / weightsum)) + (result2 * (factor * weight2 / weightsum));

	//Vector3 result3 = (result * (1 - factor)) + (result2 * (factor));
	//result3.normalise();
	//assert(result3 != Vector3::ZERO);
	return result3;
}

Vector3 VoxelBlockPersistent::vertexInterp(Vector3 start, Vector3 end, float factor)
{
	return start + (factor * (end - start));
}

void VoxelBlockPersistent::getVertexDataNoLimits(int i, int texture, Ogre::uint vertexTextures)
{
	Ogre::uint tempIndexFieldValue = tempIndexField[indexFieldIndex[i]];
	if ((tempIndexFieldValue < (Ogre::uint)-1) && (vertexTextures == tempVertexTextures[tempIndexFieldValue]))
	{
		tempIndexData[tempIndexDataSize++] = tempIndexFieldValue;	// There is already a suited vertex at this place, just index it
	}
	else
	{	// Create a new vertex
		const Ogre::ulong newIndex = tempVertexDataSize / 2;

		tempVertexNumber[tempVertexDataSize / 2] = tempIndexDataSize;
		tempTextureData[tempVertexDataSize / 2] = texture + 0.5f;
		tempVertexTextures[tempVertexDataSize / 2] = vertexTextures;

		tempVertexData[tempVertexDataSize++] = vertlist[i];
		if (tempIndexFieldValue < (Ogre::uint)-1)	// take the normal from an existing vertex if possible
			tempVertexData[tempVertexDataSize++] = tempVertexData[tempIndexFieldValue*2 + 1];
		else
			tempVertexData[tempVertexDataSize++] = calcNormal(poslist[0][i], poslist[1][i], factorlist[i]);

		tempIndexField[indexFieldIndex[i]] = newIndex;
		tempIndexData[tempIndexDataSize++] = newIndex;
	}
}

void VoxelBlockPersistent::getVertexData(int i, int texture, Ogre::uint vertexTextures)
{
	const Ogre::ulong newIndex = tempVertexDataSize / 2;

	tempTextureData[tempVertexDataSize / 2] = texture + 0.5f;
	tempVertexNumber[tempVertexDataSize / 2] = tempIndexDataSize;
	tempVertexTextures[tempVertexDataSize / 2] = vertexTextures;

	tempVertexData[tempVertexDataSize++] = vertlist[i];

	Ogre::uint tempIndexFieldValue = tempIndexField[indexFieldIndex[i]];
	if (tempIndexFieldValue < (Ogre::uint)-1)
	{
		// There is already a vertex at this place, take the normal from it
		tempVertexData[tempVertexDataSize++] = tempVertexData[tempIndexFieldValue*2 + 1];
	}
	else
	{
		tempVertexData[tempVertexDataSize++] = calcNormal(poslist[0][i], poslist[1][i], factorlist[i]);
		tempIndexField[indexFieldIndex[i]] = newIndex;
	}

	tempIndexData[tempIndexDataSize++] = newIndex;
}

void VoxelBlockPersistent::getTriangleData(int i0, int i1, int i2)
{
	Ogre::uint vertexTextures = (int)texlist[i0] + ((int)texlist[i1] << 8) + ((int)texlist[i2] << 16);
	if (texlist[i0] == texlist[i1] && texlist[i0] == texlist[i2])
	{
		getVertexDataNoLimits(i0, texlist[i0], vertexTextures);
		getVertexDataNoLimits(i1, texlist[i1], vertexTextures);
		getVertexDataNoLimits(i2, texlist[i2], vertexTextures);
	}
	else
	{
		getVertexData(i0, texlist[i0], vertexTextures);
		getVertexData(i1, texlist[i1], vertexTextures);
		getVertexData(i2, texlist[i2], vertexTextures);
	}
}

void VoxelBlockPersistent::getVoxelData(char v000, char v100, char v010, char v110, char v001, char v101, char v011, char v111, unsigned char t000, unsigned char t100, unsigned char t010, unsigned char t110, unsigned char t001, unsigned char t101, unsigned char t011, unsigned char t111, int x, int y, int z)
{
	uchar iCubeIndex;
	applyMarchingCubes(iCubeIndex, factorlist, poslist[0], poslist[1], indexFieldIndex, vertlist, texlist, v000, v100, v010, v110, v001, v101, v011, v111, t000, t100, t010, t110, t001, t101, t011, t111, x, y, z);

	for (int i = 0; triTable[iCubeIndex][i] != -1; i += 3)
		getTriangleData(triTable[iCubeIndex][i  ], triTable[iCubeIndex][i+1], triTable[iCubeIndex][i+2]);
}

#define insertMarchingCubesVertex(no, start, end, xStart, yStart, zStart, xEnd, yEnd, zEnd, xOffset, yOffset, zOffset) {\
	factorlist[no] = v##start / (float)(v##start - v##end);\
	vertlist[no] = volume->scale * vertexInterp(Vector3(xStart, yStart, zStart), Vector3(xEnd, yEnd, zEnd), factorlist[no]);\
	indexFieldIndex[no] = (((x<<1)+xOffset) + ((y<<1)+yOffset) * (BLOCK_SIDE_LENGTH_PLUS_1<<1) + ((z<<1)+zOffset) * (BLOCK_SIDE_LENGTH_PLUS_1<<1) * (BLOCK_SIDE_LENGTH_PLUS_1<<1));\
	texlist[no] = std::min(t##start, t##end);\
	poslist1[no] = Vector3(xStart, yStart, zStart);\
	poslist2[no] = Vector3(xEnd, yEnd, zEnd);\
	}

void VoxelBlockPersistent::applyMarchingCubes(Ogre::uchar& iCubeIndex, float* factorlist, Vector3* poslist1, Vector3* poslist2, int* indexFieldIndex, Vector3* vertlist, unsigned char* texlist, char v000, char v100, char v010, char v110, char v001, char v101, char v011, char v111, unsigned char t000, unsigned char t100, unsigned char t010, unsigned char t110, unsigned char t001, unsigned char t101, unsigned char t011, unsigned char t111, int x, int y, int z)
{
	// Determine the index into the edge table which
	// tells us which vertices are inside of the surface
	iCubeIndex = 0;

	if (v000 < 0) iCubeIndex |= 1;		// 0
	if (v100 < 0) iCubeIndex |= 2;		// 1
	if (v110 < 0) iCubeIndex |= 4;		// 2
	if (v010 < 0) iCubeIndex |= 8;		// 3
	if (v001 < 0) iCubeIndex |= 16;		// 4
	if (v101 < 0) iCubeIndex |= 32;		// 5
	if (v111 < 0) iCubeIndex |= 64;		// 6
	if (v011 < 0) iCubeIndex |= 128;	// 7

	// Cube is entirely in/out of the surface
	if (edgeTable[iCubeIndex] == 0)
		return;

	// Find the vertices where the surface intersects the cube
	if (edgeTable[iCubeIndex] & 1)		// 0 - 1
		insertMarchingCubesVertex(0, 000, 100, x, y, z, x+1, y, z, 1, 0, 0);
	if (edgeTable[iCubeIndex] & 2)		// 1 - 2
		insertMarchingCubesVertex(1, 100, 110, x+1, y, z, x+1, y+1, z, 2, 1, 0);
	if (edgeTable[iCubeIndex] & 4)		// 2 - 3
		insertMarchingCubesVertex(2, 110, 010, x+1, y+1, z, x, y+1, z, 1, 2, 0);
	if (edgeTable[iCubeIndex] & 8)		// 3 - 0
		insertMarchingCubesVertex(3, 010, 000, x, y+1, z, x, y, z, 0, 1, 0);
	if (edgeTable[iCubeIndex] & 16)		// 4 - 5
		insertMarchingCubesVertex(4, 001, 101, x, y, z+1, x+1, y, z+1, 1, 0, 2);
	if (edgeTable[iCubeIndex] & 32)		// 5 - 6
		insertMarchingCubesVertex(5, 101, 111, x+1, y, z+1, x+1, y+1, z+1, 2, 1, 2);
	if (edgeTable[iCubeIndex] & 64)		// 6 - 7
		insertMarchingCubesVertex(6, 111, 011, x+1, y+1, z+1, x, y+1, z+1, 1, 2, 2);
	if (edgeTable[iCubeIndex] & 128)	// 7 - 4
		insertMarchingCubesVertex(7, 011, 001, x, y+1, z+1, x, y, z+1, 0, 1, 2);
	if (edgeTable[iCubeIndex] & 256)	// 0 - 4
		insertMarchingCubesVertex(8, 000, 001, x, y, z, x, y, z+1, 0, 0, 1);
	if (edgeTable[iCubeIndex] & 512)	// 1 - 5
		insertMarchingCubesVertex(9, 100, 101, x+1, y, z, x+1, y, z+1, 2, 0, 1);
	if (edgeTable[iCubeIndex] & 1024)	// 2 - 6
		insertMarchingCubesVertex(10, 110, 111, x+1, y+1, z, x+1, y+1, z+1, 2, 2, 1);
	if (edgeTable[iCubeIndex] & 2048)	// 3 - 7
		insertMarchingCubesVertex(11, 010, 011, x, y+1, z, x, y+1, z+1, 0, 2, 1);
}

