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

#include <OgrePrerequisites.h>

class Noise
{
public:
	static inline float interpolatedNoise3D(float x, float y, float z)
	{
		return noise(x/64.0f, y/32.0f, z/64.0f);
	}

	static inline float perlinNoise3D(float x, float y, float z)
	{
		float total = 0.0f;
		float p = 1.0f / 2.0f;		// persistence
		float n = 3 - 1;			// first number: octave count

		for (int i = 0; i <= n; i++)
		{
			float frequency = powf(2, i);
			float amplitude = powf(p, i);

			total += interpolatedNoise3D(x * frequency, y * frequency, z * frequency) * amplitude;
		}

		return total;
	}

   static inline float noise(float x, float y, float z)
   {
	  int X = (int)std::floor(x) & 255,                  // FIND UNIT CUBE THAT
          Y = (int)std::floor(y) & 255,                  // CONTAINS POINT.
          Z = (int)std::floor(z) & 255;
      x -= std::floor(x);                                // FIND RELATIVE X,Y,Z
      y -= std::floor(y);                                // OF POINT IN CUBE.
      z -= std::floor(z);
      float u = fade(x),                                // COMPUTE FADE CURVES
             v = fade(y),                                // FOR EACH OF X,Y,Z.
             w = fade(z);
      int A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z,      // HASH COORDINATES OF
          B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;      // THE 8 CUBE CORNERS,

      return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),  // AND ADD
                                     grad(p[BA  ], x-1, y  , z   )), // BLENDED
                             lerp(u, grad(p[AB  ], x  , y-1, z   ),  // RESULTS
                                     grad(p[BB  ], x-1, y-1, z   ))),// FROM  8
                     lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),  // CORNERS
                                     grad(p[BA+1], x-1, y  , z-1 )), // OF CUBE
                             lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
                                     grad(p[BB+1], x-1, y-1, z-1 ))));
   }

   static inline float fade(float t)
   {
	   return t * t * t * (t * (t * 6 - 15) + 10);
   }

   static inline float lerp(float t, float a, float b)
   {
	   return a + t * (b - a);
   }

   static inline float grad(int hash, float x, float y, float z)
   {
      int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
      float u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
             v = h<4 ? y : h==12||h==14 ? x : z;
      return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
   }

   static int p[512];
   static int permutation[256];

   static void init()
   {
	   for (int i=0; i < 256 ; i++)
		   p[256+i] = p[i] = permutation[i];
   }
};
