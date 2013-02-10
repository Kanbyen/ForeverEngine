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
#include "levelElement.h"
#include "level.h"
#include "noise.h"

// ### LevelElement ###
Point3 LevelElement::getMidPoint()
{
    return midPointOrig;
}

// ### LevelElementCube ###
void LevelElementCube::set(Point3 pmin, Point3 pmax)
{
	midPointOrig = (pmin + pmax) / 2;
	midPoint = midPointOrig * level->scale;

	bbMin = pmin * level->scale;
	bbMax = pmax * level->scale;
}
void LevelElementCube::set(Point3 midPoint, int xRadius, int yRadius, int zRadius)
{
    this->midPointOrig = midPoint;
	midPoint *= level->scale;
	xRadius *= level->scale;
	yRadius *= level->scale;
	zRadius *= level->scale;
	this->midPoint = midPoint;

	bbMin = Point3(midPoint.x - xRadius, midPoint.y - yRadius, midPoint.z - zRadius);
	bbMax = Point3(midPoint.x + xRadius, midPoint.y + yRadius, midPoint.z + zRadius);
}

float LevelElementCube::apply(int x, int y, int z, float in)
{
	if ((x <= bbMin.x) || (x >= bbMax.x))
		return in + 1.01f;
	if ((y <= bbMin.y) || (y >= bbMax.y))
		return in + 1.01f;
	if ((z <= bbMin.z) || (z >= bbMax.z))
		return in + 1.01f;

	return in + 4.0f;
}

// ### LevelElementInvCube ###
float LevelElementInvCube::apply(int x, int y, int z, float in)
{
	if ((x <= bbMin.x) || (x >= bbMax.x))
		return in - 1.01f;
	if ((y <= bbMin.y) || (y >= bbMax.y))
		return in - 1.01f;
	if ((z <= bbMin.z) || (z >= bbMax.z))
		return in - 1.01f;

	return in - 4.0f;
}

// ### LevelElementSolidCube ###
float LevelElementSolidCube::apply(int x, int y, int z, float in)
{
	return in + 4.0f;
}

// ### LevelElementInvSolidCube ###
float LevelElementInvSolidCube::apply(int x, int y, int z, float in)
{
	return in - 4.0f;
}

// ### LevelElementSphere ###
void LevelElementSphere::set(Point3 midPoint, int radius)
{
    midPointOrig = midPoint;
    radiusOrig = radius;
	midPoint *= level->scale;
	radius *= level->scale;
	this->midPoint = midPoint;
	this->midPointF = Vector3(midPoint.x, midPoint.y, midPoint.z);
	this->radius = 4 + radius;

	int bbRange = this->radius;
	bbMin = Point3(midPoint.x - bbRange, midPoint.y - bbRange, midPoint.z - bbRange);
	bbMax = Point3(midPoint.x + bbRange, midPoint.y + bbRange, midPoint.z + bbRange);
}
int LevelElementSphere::getRadius()
{
    return radiusOrig;
}
float LevelElementSphere::apply(int x, int y, int z, float in)
{
	float value = (radius - midPointF.distance(Vector3(x, y, z))) / 4;
	return in + ((value > 0) ? value : 0);
}

// ### LevelElementInvSphere ###
float LevelElementInvSphere::apply(int x, int y, int z, float in)
{
	float value = (radius - midPointF.distance(Vector3(x, y, z))) / 4;
	return in - ((value > 0) ? value : 0);
}

// ### LevelElementSharpSphere ###
void LevelElementSharpSphere::setSharp(Point3 midPoint, int radius)
{
    midPointOrig = midPoint;
    radiusOrig = radius;
	midPoint *= level->scale;
	radius *= level->scale;
	this->midPoint = midPoint;
	this->midPointF = Vector3(midPoint.x, midPoint.y, midPoint.z);
	this->radius = 1 + radius;

	int bbRange = this->radius;
	bbMin = Point3(midPoint.x - bbRange, midPoint.y - bbRange, midPoint.z - bbRange);
	bbMax = Point3(midPoint.x + bbRange, midPoint.y + bbRange, midPoint.z + bbRange);
}
float LevelElementSharpSphere::apply(int x, int y, int z, float in)
{
	float value = (radius - midPointF.distance(Vector3(x, y, z)));
	return in + ((value > 0) ? value : 0);
}

// ### LevelElementFieldGround ###
float LevelElementFieldGround::apply(int x, int y, int z, float in)
{
	const int GROUND_HEIGHT = 15;

	float offset = 0;
	offset += min(1.0f, max(-1.0f, (bbMax.y - GROUND_HEIGHT - y) * 0.015f));
	offset += min(0.0f, (bbMax.z - 10 - z) * 0.1f);
	offset += min(0.0f, (-(bbMin.z + 10) + z) * 0.1f);

	return max(-4.0f, min(4.0f, Noise::perlinNoise3D(x, y, z) + offset));
}
int LevelElementFieldGround::getHeightAboveGround(int x, int y, int z)
{
	float v = apply(x, y, z, 0);
	if (v < 0)
	{
		// The position is in the air, move down and look for ground
		for (int i = 1; i < 300; ++i)
		{
			if (apply(x, y + i, z, 0) >= 0)
				return i - 1;
		}
	}
	else
	{
		// The position is in the ground, move up and look for air
		for (int i = 1; i < 300; ++i)
		{
			if (apply(x, y - i, z, 0) < 0)
				return i;
		}
	}

	// Error - couldn't find ground / air
	return 0;
}
