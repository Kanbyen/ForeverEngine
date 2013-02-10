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
#include "levelTextureElement.h"
#include "level.h"
#include "noise.h"
#include "blockConstants.h"

#define vBuffer(x, y, z) voxelBuffer[((x)+1) + ((y)+1) * (BLOCK_SIDE_LENGTH+2) + ((z)+1) * (BLOCK_SIDE_LENGTH+2) * (BLOCK_SIDE_LENGTH+2)]
#define tBuffer(x, y, z) buffer[(x) + (y) * (BLOCK_SIDE_LENGTH) + (z) * (BLOCK_SIDE_LENGTH) * (BLOCK_SIDE_LENGTH)]

void LevelTextureElementSolidCube::set(Point3 pmin, Point3 pmax, unsigned char texture)
{
	bbMin = pmin;
	bbMax = pmax;
	this->texture = texture;
}

void LevelTextureElementSolidCube::apply(int x1, int y1, int z1, int x2, int y2, int z2, int baseX, int baseY, int baseZ, char* voxelBuffer, unsigned char* buffer)
{
	for (int x = x1; x <= x2; ++x)
		for (int y = y1; y <= y2; ++y)
			for (int z = z1; z <= z2; ++z)
				tBuffer(x, y, z) = texture;
}

void LevelTextureElementAutotexCube::set(Point3 pmin, Point3 pmax, unsigned char top, unsigned char side, unsigned char inner1, unsigned char inner2)
{
	bbMin = pmin;
	bbMax = pmax;
	this->top = top;
	this->side = side;
	this->inner1 = inner1;
	this->inner2 = inner2;
}
void LevelTextureElementAutotexCube::apply(int x1, int y1, int z1, int x2, int y2, int z2, int baseX, int baseY, int baseZ, char* voxelBuffer, unsigned char* buffer)
{
	for (int x = x1; x <= x2; x++)
	{
		for (int y = y1; y <= y2; y++)
		{
			for (int z = z1; z <= z2; z++)
			{
				int n = 0, nrock = 0;
				for (int dx = -1; dx < 2; dx++)
					for (int dy = -1; dy < 2; dy++)
						for (int dz = -1; dz < 2; dz++)
						{
							if (vBuffer(x + dx, y + dy, z + dz) < 0)
								n++;
							else
								nrock++;
						}

				if (n > 0 && nrock > 0)
				{
					// surface voxel
					Vector3 normal = Vector3(vBuffer(x + 1, y    , z    ) -
											vBuffer(x - 1, y    , z    ),
											vBuffer(x    , y - 1, z    ) -
											vBuffer(x    , y + 1, z    ),
											vBuffer(x    , y    , z - 1) -
											vBuffer(x    , y    , z + 1));
					normal.normalise();

					if (normal.y > 0.7f)
						tBuffer(x, y, z) = top;
					else
						tBuffer(x, y, z) = side;
				}
				else
				{
					if (vBuffer(x, y, z) < 0)
					{
						// air voxel; no variation for the sake of speed
						tBuffer(x, y, z) = inner1;
					}
					else
					{
						// ground voxel
						if (inner1 == inner2)
							tBuffer(x, y, z) = inner1;
						else
						{
							float perlin = Noise::perlinNoise3D((baseX+x)*2, (baseY+y)*2, (baseZ+z)*2);
							if (perlin >= 0.0f)
								tBuffer(x, y, z) = inner1;
							else
								tBuffer(x, y, z) = inner2;
						}
					}
				}
			}
		}
	}
}

void LevelTextureElementSolidSphere::set(Point3 midPoint, int radius, unsigned char texture)
{
	this->midPoint = midPoint;
	this->radius = radius;
	this->texture = texture;

	int bbRange = this->radius+1;
	bbMin = Point3(midPoint.x - bbRange, midPoint.y - bbRange, midPoint.z - bbRange);
	bbMax = Point3(midPoint.x + bbRange, midPoint.y + bbRange, midPoint.z + bbRange);
}

int LevelTextureElementSolidSphere::getRadius()
{
	return radius;
}
void LevelTextureElementSolidSphere::apply(int x1, int y1, int z1, int x2, int y2, int z2, int baseX, int baseY, int baseZ, char* voxelBuffer, unsigned char* buffer)
{
	for (int x = x1; x <= x2; ++x)
	{
		for (int y = y1; y <= y2; ++y)
		{
			for (int z = z1; z <= z2; ++z)
			{
				float value = (radius - midPoint.distance(Point3(baseX+x, baseY+y, baseZ+z)));

				if (value > -0.01f)
					tBuffer(x, y, z) = texture;
			}
		}
	}
}
