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

#ifndef _LEVEL_OBJECT_H_
#define _LEVEL_OBJECT_H_

#include "point3.h"

/// Represents an object spawn area during level generation
class ObjectSpawnArea
{
public:
	virtual ~ObjectSpawnArea() {};
	virtual Vector3 generateAirPosition() const = 0;
	virtual Vector3 generateGroundPosition() const = 0;
};

/// A cube-shaped spawn area
class ObjectSpawnAreaCube : public ObjectSpawnArea
{
public:
	ObjectSpawnAreaCube(Point3 bbmin, Point3 bbmax);

	Vector3 generateAirPosition() const;
	Vector3 generateGroundPosition() const;

protected:
	Vector3 bbmin;
	Vector3 bbmax;
};

#endif
