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
#include "aiGetNeighbours.h"
#include "object.h"

AIGetNeighbours::AIGetNeighbours(float radius, Object* owner, int excludeTags)
{
	this->radiusSq = radius * radius;
	this->owner = owner;
	this->excludeTags = excludeTags;
}
AIGetNeighbours::~AIGetNeighbours()
{
	neighbourList.clear();
}

void AIGetNeighbours::update(Vector3 pos)
{
	neighbourList.clear();

	for (ObjectManager::iterator it = objectManager.begin(); it != objectManager.end(); it++)
	{
		Object* obj = (*it).second;

		// Don't process the owner itself
		if (obj == owner)
			continue;

		if (obj->tag & excludeTags)
			continue;

		if ((obj->position - pos).squaredLength() <= radiusSq)
			neighbourList.push_back(obj);
	}
}
