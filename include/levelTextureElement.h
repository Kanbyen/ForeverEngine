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

#ifndef _LEVEL_TEXTURE_ELEMENT_H_
#define _LEVEL_TEXTURE_ELEMENT_H_

#include "point3.h"

/// Pure virtual base class for level texture elements
class LevelTextureElement
{
public:
	virtual ~LevelTextureElement() {}

	Point3 bbMin;
	Point3 bbMax;

	virtual void apply(int x1, int y1, int z1, int x2, int y2, int z2, int baseX, int baseY, int baseZ, char* voxelBuffer, unsigned char* buffer) = 0;
};

/// A cube filled with a specific texture
class LevelTextureElementSolidCube : public LevelTextureElement
{
public:
	inline LevelTextureElementSolidCube() {};
	inline LevelTextureElementSolidCube(Point3 pmin, Point3 pmax, unsigned char texture) {set(pmin, pmax, texture);};

	void set(Point3 pmin, Point3 pmax, unsigned char texture);
	virtual void apply(int x1, int y1, int z1, int x2, int y2, int z2, int baseX, int baseY, int baseZ, char* voxelBuffer, unsigned char* buffer);

protected:
	unsigned char texture;
};

/// An autotex-filled cube
class LevelTextureElementAutotexCube : public LevelTextureElement
{
public:
	inline LevelTextureElementAutotexCube() {};
	inline LevelTextureElementAutotexCube(Point3 pmin, Point3 pmax, unsigned char top, unsigned char side, unsigned char inner1, unsigned char inner2) {set(pmin, pmax, top, side, inner1, inner2);};

	void set(Point3 pmin, Point3 pmax, unsigned char top, unsigned char side, unsigned char inner1, unsigned char inner2);
	virtual void apply(int x1, int y1, int z1, int x2, int y2, int z2, int baseX, int baseY, int baseZ, char* voxelBuffer, unsigned char* buffer);

protected:
	unsigned char top;
	unsigned char side;
	unsigned char inner1;
	unsigned char inner2;
};

/// A sphere filled with a specific texture
class LevelTextureElementSolidSphere : public LevelTextureElement
{
public:
	inline LevelTextureElementSolidSphere() {};
	inline LevelTextureElementSolidSphere(Point3 midPoint, int radius, unsigned char texture) {set(midPoint, radius, texture);};

	void set(Point3 midPoint, int radius, unsigned char texture);

    int getRadius();
	virtual void apply(int x1, int y1, int z1, int x2, int y2, int z2, int baseX, int baseY, int baseZ, char* voxelBuffer, unsigned char* buffer);

protected:
	Point3 midPoint;
	int radius;
	unsigned char texture;
};

#endif
