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

#ifndef _LEVEL_ELEMENT_H_
#define _LEVEL_ELEMENT_H_

#include "point3.h"

/// Pure virtual base class for level elements
class LevelElement
{
public:
	virtual ~LevelElement() {}

	Point3 bbMin;
	Point3 bbMax;
	Point3 midPoint;
	Point3 midPointOrig;

    Point3 getMidPoint();
	virtual float apply(int x, int y, int z, float in) = 0;
};

/// Cube with sharp borders (visible at the edges). If the borders are important (if the cube will be made hollow), better use LevelElementSolidCube
/**
	Other variants: LevelElement[Inv][Solid]Cube
*/
class LevelElementCube : public LevelElement
{
public:
	inline LevelElementCube() {};
	inline LevelElementCube(Point3 midPoint, int xRadius, int yRadius, int zRadius) {set(midPoint, xRadius, yRadius, zRadius);};
	inline LevelElementCube(Point3 pmin, Point3 pmax) {set(pmin, pmax);};

	/// Only to take the load off the constructors, handy for derived classes
	void set(Point3 midPoint, int xRadius, int yRadius, int zRadius);
	void set(Point3 pmin, Point3 pmax);

	virtual float apply(int x, int y, int z, float in);
};

/// Cube-shaped hole with solid borders (see LevelElementCube)
class LevelElementInvCube : public LevelElementCube
{
public:
	inline LevelElementInvCube(Point3 midPoint, int xRadius, int yRadius, int zRadius) {set(midPoint, xRadius, yRadius, zRadius);};
	inline LevelElementInvCube(Point3 pmin, Point3 pmax) {set(pmin, pmax);};

	virtual float apply(int x, int y, int z, float in);
};

/// Cube with round edges (advantage: edges are solid, thus behave normal in contrast to the sharp edges of LevelElementCube)
class LevelElementSolidCube : public LevelElementCube
{
public:
	inline LevelElementSolidCube(Point3 midPoint, int xRadius, int yRadius, int zRadius) {set(midPoint, xRadius, yRadius, zRadius);};
	inline LevelElementSolidCube(Point3 pmin, Point3 pmax) {set(pmin, pmax);};

	virtual float apply(int x, int y, int z, float in);
};

/// Cube-shaped hole with round edges (see LevelElementSolidCube)
class LevelElementInvSolidCube : public LevelElementCube
{
public:
	inline LevelElementInvSolidCube(Point3 midPoint, int xRadius, int yRadius, int zRadius) {set(midPoint, xRadius, yRadius, zRadius);};
	inline LevelElementInvSolidCube(Point3 pmin, Point3 pmax) {set(pmin, pmax);};

	virtual float apply(int x, int y, int z, float in);
};

/// Sphere
class LevelElementSphere : public LevelElement
{
public:
	inline LevelElementSphere() {};
	inline LevelElementSphere(Point3 midPoint, int radius) {set(midPoint, radius);};

	void set(Point3 midPoint, int radius);

	int radius;

    int getRadius();
	virtual float apply(int x, int y, int z, float in);

protected:
	int radiusOrig;
	Vector3 midPointF;
};

/// Sphere-shaped hole
class LevelElementInvSphere : public LevelElementSphere
{
public:
	inline LevelElementInvSphere(Point3 midPoint, int radius) {set(midPoint, radius);};

	virtual float apply(int x, int y, int z, float in);
};

/// Non-metaball-like sphere
class LevelElementSharpSphere : public LevelElementSphere
{
public:
	inline LevelElementSharpSphere(Point3 midPoint, int radius) {setSharp(midPoint, radius);};

	void setSharp(Point3 midPoint, int radius);

	virtual float apply(int x, int y, int z, float in);
};

/// Ground for "field" levels. The ground height is ca. GROUND_HEIGHT units below bbMax.y (see code of apply()) but varies greatly.
class LevelElementFieldGround : public LevelElementCube
{
public:
	inline LevelElementFieldGround(Point3 pmin, Point3 pmax) {set(pmin, pmax);};

	/// Returns the height of the specified position above the ground; if the position is in the ground, the value will be negative
	int getHeightAboveGround(int x, int y, int z);
	int getHeightAboveGround(Point3 point) {return getHeightAboveGround(point.x, point.y, point.z);};

	virtual float apply(int x, int y, int z, float in);
};

#endif
