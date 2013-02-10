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
#include "aiPositionRating.h"
#include "aiGetNeighbours.h"
#include "physicsConstants.h"
#include "luaFunctions.h"
#include "stateGame.h"
#include "volume.h"
#include "object.h"

Vector3 AIPositionRating::moveAwayFromNeighbours(Vector3 pos, Object* obj, float radius, float scaling, int steps)
{
	AIGetNeighbours neighbours(radius, obj, OBJTAG_BULLET);
	Vector3 result = pos;

	for (int i = 0; i < steps; i++)
	{
		Vector3 force = Vector3::ZERO;
		neighbours.update(result);
		for (AIGetNeighbours::iterator it = neighbours.begin(); it != neighbours.end(); it++)
		{
			Vector3 toAgent = result - (*it)->position;
			float toAgentLength = toAgent.length();
			if (toAgentLength == 0.0f)
				continue;

			if (toAgentLength < 1.0f)
				force += (toAgent / toAgentLength);
			else
				force += (toAgent / toAgentLength) / toAgentLength;
		}
		result += force * scaling;
	}

	return result;
}

float AIPositionRating::getPositionInAir(Vector3 pos, int radius)
{
	Vector3 scaledPos = pos / game.volumeData->scale;
	if (!game.volumeData->containsPointAbs(scaledPos))
		return 0;
	
	float score = -luaCountVoxelValuesAtPoint(&pos, radius);
	int numVoxelsChecked = (1 + 2 * radius) * (1 + 2 * radius) * (1 + 2 * radius);
	return score / (128 * numVoxelsChecked);
}

bool AIPositionRating::getFreeWayToPos(Vector3 pos, Object* obj)
{
	return luaRayTestFree(&obj->position, &pos, obj, RAYTEST_NO_DEBRIS);
}

bool AIPositionRating::getFreeLineOfFireToTarget(Vector3 pos, Object* obj, Object* target, float shootingRange)
{
	Vector3 toTarget = target->position - pos;
	if (toTarget.squaredLength() > shootingRange*shootingRange)
		return false;
	return luaRayTestObject(&pos, target, obj, RAYTEST_NO_DEBRIS);
}
