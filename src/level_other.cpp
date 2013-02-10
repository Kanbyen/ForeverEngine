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
#include "level.h"
#include "noise.h"
#include "stateGame.h"	// TODO - only for voxelData!
#include "volume.h"

#define setVoxelBuffer(x, y, z, v) buffer[((x) - firstX) + ((y) - firstY) * blx + ((z) - firstZ) * blx * bly] = (v)
#define vBuffer(x, y, z) voxelBuffer[((x)+1) + ((y)+1) * (BLOCK_SIDE_LENGTH+2) + ((z)+1) * (BLOCK_SIDE_LENGTH+2) * (BLOCK_SIDE_LENGTH+2)]
#define tBuffer(x, y, z) buffer[(x) + (y) * (BLOCK_SIDE_LENGTH) + (z) * (BLOCK_SIDE_LENGTH) * (BLOCK_SIDE_LENGTH)]

int Level::other_InvCube_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer)
{
	const int blx = lastX - firstX + 1;
	const int bly = lastY - firstY + 1;
	
	for (int x = firstX; x <= lastX; x++)
	{
		for (int y = firstY; y <= lastY; y++)
		{
			for (int z = firstZ; z <= lastZ; z++)
			{
				const int blockHalfSize = (VOLUME_SIDE_LENGTH_IN_VOXELS / 5);
				const int blockmin = (VOLUME_SIDE_LENGTH_IN_VOXELS / 2) - blockHalfSize;
				const int blockmax = (VOLUME_SIDE_LENGTH_IN_VOXELS / 2) + blockHalfSize;
				if (x >= blockmin && y >= blockmax && z >= blockmin && x <= blockmax && /*y <= blockmax &&*/ z <= blockmax)
					setVoxelBuffer(x, y, z, -128);
				else
					setVoxelBuffer(x, y, z, 127);
			}
		}
	}
	
	return RETURN_NORMAL_BLOCK;
}

int Level::other_Cube_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer)
{
	const int blx = lastX - firstX + 1;
	const int bly = lastY - firstY + 1;

	for (int x = firstX; x <= lastX; x++)
	{
		for (int y = firstY; y <= lastY; y++)
		{
			for (int z = firstZ; z <= lastZ; z++)
			{
				/*int mx = abs((x - 4) % 16);
				int my = abs((y - 4) % 16);
				int mz = abs((z - 4) % 16);
				if ((mx < 8) && (my < 8) && (mz < 8))
					setVoxelBuffer(x, y, z, 127);
				else
					setVoxelBuffer(x, y, z, -128);*/

				/*int mx = x % BLOCK_SIDE_LENGTH;
				int my = y % BLOCK_SIDE_LENGTH;
				int mz = z % BLOCK_SIDE_LENGTH;
				const int blockmin = (BLOCK_SIDE_LENGTH * 1) / 3;
				const int blockmax = (BLOCK_SIDE_LENGTH * 2) / 3;
				if ((mx >= blockmin) && (my >= blockmin) && (mz >= blockmin) && (mx <= blockmax) && (my <= blockmax) && (mz <= blockmax))
					setVoxelBuffer(x, y, z, 127);
				else
					setVoxelBuffer(x, y, z, -128);*/
				
				const int blockmin = (VOLUME_SIDE_LENGTH_IN_VOXELS / 2) - 3;
				const int blockmax = (VOLUME_SIDE_LENGTH_IN_VOXELS / 2) + 3;
				if (x >= blockmin && y >= blockmin && z >= blockmin && x <= blockmax && y <= blockmax && z <= blockmax)
					setVoxelBuffer(x, y, z, -128);
				else
					setVoxelBuffer(x, y, z, 127);
			}
		}
	}

	return RETURN_NORMAL_BLOCK;
}

int Level::other_Cave_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer)
{
	const int blx = lastX - firstX + 1;
	const int bly = lastY - firstY + 1;

	const float SPHERE_RADIUS = 80.0f;

	for (int x = firstX; x <= lastX; x++)
	{
		for (int y = firstY; y <= lastY; y++)
		{
			for (int z = firstZ; z <= lastZ; z++)
			{
				float dx = x - SPHERE_RADIUS;
				float dy = y - SPHERE_RADIUS;
				float dz = z - SPHERE_RADIUS;

				float dist = sqrt(dx*dx + dy*dy + dz*dz);
				dist = dist - SPHERE_RADIUS;
				dist /= 10.0f;
				dist = std::max(0.0f, dist);

				float value = ((Noise::perlinNoise3D(x, y, z) + dist) * 256.0f) * 6.0f;

				value = std::min((float)value, (float)127);
				value = std::max((float)value, (float)-128);
				setVoxelBuffer(x, y, z, (char)value);
			}
		}
	}

	return RETURN_NORMAL_BLOCK;
}

int Level::other_Terrain_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer)
{
	const int blx = lastX - firstX + 1;
	const int bly = lastY - firstY + 1;

	for (int x = firstX; x <= lastX; x++)
	{
		for (int z = firstZ; z <= lastZ; z++)
		{
			//float height = 60.0f + perlinNoise3D(x, 0, z) * 40.0f;
			float perlin = (Noise::perlinNoise3D(x, 0, z) + 1.0f) / 1.55f;
			//perlin *= perlin;
			perlin = powf(perlin, 3.0f);
			float height = 30.0f + perlin * 46.0f;

			for (int y = firstY; y <= lastY; y++)
			{
				float value = (height - y) * 64.0f;
				value = std::min((float)value, (float)127);
				value = std::max((float)value, (float)-128);
				setVoxelBuffer(x, y, z, (char)value);
			}
		}
	}

	return RETURN_NORMAL_BLOCK;
}

int Level::other_Planes_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer)
{
	const int blx = lastX - firstX + 1;
	const int bly = lastY - firstY + 1;

	for (int y = firstY; y <= lastY; y++)
	{
		float sineOffset = sinf(y / 15.0f) * 0.5f;
		for (int x = firstX; x <= lastX; x++)
		{
			for (int z = firstZ; z <= lastZ; z++)
			{
				float value = ((Noise::perlinNoise3D(x, y, z) + sineOffset) * 256.0f - 64.0f) * 6.0f;

				value = std::min((float)value, (float)std::numeric_limits<signed char>::max());
				value = std::max((float)value, (float)std::numeric_limits<signed char>::min());
				setVoxelBuffer(x, y, z, (char)value);
			}
		}
	}

	return RETURN_NORMAL_BLOCK;
}

int Level::other_Perlin_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer)
{
	const int blx = lastX - firstX + 1;
	const int bly = lastY - firstY + 1;

	for (int x = firstX; x <= lastX; x++)
	{
		for (int y = firstY; y <= lastY; y++)
		{
			for (int z = firstZ; z <= lastZ; z++)
			{
				float value = (Noise::perlinNoise3D(x, y, z) * 256.0f - 64.0f) * 6.0f;

				value = std::min((float)value, (float)127);
				value = std::max((float)value, (float)-128);
				setVoxelBuffer(x, y, z, (char)value);
			}
		}
	}

	return RETURN_NORMAL_BLOCK;
}

int Level::other_Sphere_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer)
{
	const int blx = lastX - firstX + 1;
	const int bly = lastY - firstY + 1;

	for (int x = firstX; x <= lastX; x++)
	{
		for (int y = firstY; y <= lastY; y++)
		{
			for (int z = firstZ; z <= lastZ; z++)
			{
				int mx = (x / 40 * 40) + 20;
				int my = (y / 50 * 50) + 25;
				int mz = (z / 40 * 40) + 20;
				Vector3 centre = Vector3(mx, my, mz);

				float v = 15 - (centre - Vector3(x, y, z)).length();
				char data = std::max(-128, std::min(127, (int)(v * 127)));
				setVoxelBuffer(x, y, z, data);
			}
		}
	}

	return RETURN_NORMAL_BLOCK;
}

void Level::other_Invert()
{
	for(int z = 1; z <= VOLUME_SIDE_LENGTH_IN_VOXELS - 2; z++)
	{
		for(int y = 1; y <= VOLUME_SIDE_LENGTH_IN_VOXELS - 2; y++)
		{
			for(int x = 1; x <= VOLUME_SIDE_LENGTH_IN_VOXELS - 2; x++)
			{
				char data = game.volumeData->getVoxelRel(x, y, z);
				// Don't invert the minimum value as it is because the result would overflow
				if (data == std::numeric_limits<signed char>::min())
					data++;
				game.volumeData->setVoxelRel(x, y, z, -data);
			}
		}
	}

	game.volumeData->setDataChangedRel(1, 1, 1, VOLUME_SIDE_LENGTH_IN_VOXELS - 2, VOLUME_SIDE_LENGTH_IN_VOXELS - 2, VOLUME_SIDE_LENGTH_IN_VOXELS - 2);
}

void Level::other_autotexCallback(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* voxelBuffer, unsigned char* buffer)
{
	for (int x = 0; x <= lastX-firstX; x++)
	{
		for (int y = 0; y <= lastY-firstY; y++)
		{
			for (int z = 0; z <= lastZ-firstZ; z++)
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
						tBuffer(x, y, z) = game.autotexTop;
					else
						tBuffer(x, y, z) = game.autotexSide;
				}
				else
				{
					if (vBuffer(x, y, z) < 0)
					{
						// air voxel; no variation for the sake of speed
						tBuffer(x, y, z) = game.autotexInner1;
					}
					else
					{
						// ground voxel
						float perlin = Noise::perlinNoise3D((firstX+x)*2, (firstY+y)*2, (firstZ+z)*2);
						if (perlin >= 0.0f)
							tBuffer(x, y, z) = game.autotexInner1;
						else
							tBuffer(x, y, z) = game.autotexInner2;
					}
				}
			}
		}
	}
}

void Level::other_ApplyTextures(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ)
{
	game.volumeData->makeRangeRelative(firstX, firstY, firstZ, lastX, lastY, lastZ);
	game.volumeData->checkBoundsRel(firstX, firstY, firstZ, lastX, lastY, lastZ);

	for(int z = firstZ; z <= lastZ; z++)
	{
		for(int y = firstY; y <= lastY; y++)
		{
			for(int x = firstX; x <= lastX; x++)
			{
				// Method based on surfaces and normals
				int n = 0, nrock = 0;
				for (int dx = -1; dx < 2; dx++)
					for (int dy = -1; dy < 2; dy++)
						for (int dz = -1; dz < 2; dz++)
						{
							if (game.volumeData->getVoxelRel(x + dx, y + dy, z + dz) < 0)
								n++;
							else
								nrock++;
						}

				if (n > 0 && nrock > 0)
				{
					// surface voxel
					Vector3 normal = Vector3(game.volumeData->getVoxelRel(x + 1, y    , z    ) -
											 game.volumeData->getVoxelRel(x - 1, y    , z    ),
											 game.volumeData->getVoxelRel(x    , y - 1, z    ) -
											 game.volumeData->getVoxelRel(x    , y + 1, z    ),
											 game.volumeData->getVoxelRel(x    , y    , z - 1) -
											 game.volumeData->getVoxelRel(x    , y    , z + 1));
					normal.normalise();

					if (normal.y > 0.7f)
						game.volumeData->setTextureRel(x, y, z, game.autotexTop);
					else
						game.volumeData->setTextureRel(x, y, z, game.autotexSide);
				}
				else
				{
					if (game.volumeData->getVoxelRel(x, y, z) < 0)
					{
						// air voxel; no variation for the sake of speed
						game.volumeData->setTextureRel(x, y, z, game.autotexInner1);
					}
					else
					{
						// ground voxel
						float perlin = Noise::perlinNoise3D(x*2, y*2, z*2);
						if (perlin >= 0.0f)
							game.volumeData->setTextureRel(x, y, z, game.autotexInner1);
						else
							game.volumeData->setTextureRel(x, y, z, game.autotexInner2);
					}
				}
			}
		}
	}
}
