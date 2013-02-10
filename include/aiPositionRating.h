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

#ifndef _AI_POSITION_RATING_H_
#define _AI_POSITION_RATING_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;

class Object;

/// Contains static methods to check if positions are tactically good or bad. All methods with float return values return values (haha) in the range [0; 1]
class AIPositionRating
{
public:

	/// Moves the position away from neighbours, returns the updated position. The maximum offset caused by a single neighbour is 1 (but something like 0.25 is more likely) - set scaling accordingly.
	static Vector3 moveAwayFromNeighbours(Vector3 pos, Object* obj, float radius, float scaling, int steps);

	/// Is there free air at the position (checks the voxel data, not the physics engine)? Returns 0 if the position is out of the volume.
	static float getPositionInAir(Vector3 pos, int radius);

	/// Is the way to the target position free?
	static bool getFreeWayToPos(Vector3 pos, Object* obj);

	/// Is there a free line-of-fire to the target? Also checks if the distance to the target is below shootingRange
	static bool getFreeLineOfFireToTarget(Vector3 pos, Object* obj, Object* target, float shootingRange);
};

#endif
