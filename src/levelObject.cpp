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
#include "levelObject.h"
#include "level.h"
#include "stateGame.h"
#include "volume.h"

ObjectSpawnAreaCube::ObjectSpawnAreaCube(Point3 bbmin, Point3 bbmax)
{
	this->bbmin = level->pointToVector(bbmin);
	this->bbmax = level->pointToVector(bbmax);
}

Vector3 ObjectSpawnAreaCube::generateAirPosition() const
{
	return Vector3(Math::RangeRandom(bbmin.x, bbmax.x), Math::RangeRandom(bbmin.y, bbmax.y), Math::RangeRandom(bbmin.z, bbmax.z));
}
Vector3 ObjectSpawnAreaCube::generateGroundPosition() const
{
	// TODO: replace 5 with object bb half height
	// TODO: unify air and ground position; look up in object generation properties
	return Vector3(Math::RangeRandom(bbmin.x, bbmax.x), bbmin.y + 5, Math::RangeRandom(bbmin.z, bbmax.z));
}
