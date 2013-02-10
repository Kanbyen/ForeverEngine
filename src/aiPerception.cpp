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
#include "aiPerception.h"
#include "luaFunctions.h"
#include "physicsConstants.h"
#include "aiGetNeighbours.h"

Vector3 AIPerceptedObject::getPerceptedPosition(int id)
{
	if (inView)
	{
		Object* obj = objectManager.getObjectByID(id);
		if (obj)
			return obj->position;
		else
			return lastPerceptedPos;	// the object was deleted in the meantime!
	}
	else
		return lastPerceptedPos;
}

AIPerception::AIPerception(ObjectTag tag, float viewRange, Degree fieldOfView, float memorySpan, Object* owner)
{
	time = 0;
	this->tag = tag;
	this->fieldOfView = Math::Cos(fieldOfView.valueRadians(), false);
	this->memorySpan = memorySpan;
	this->owner = owner;
	this->viewRangeSq = viewRange * viewRange;
}
AIPerception::~AIPerception()
{
	perceptionMap.clear();
}

void AIPerception::updatePerceptedObject(Object* object, bool inViewKnown, bool inView)
{
	PerceptionMapItr it = perceptionMap.find(object->id);
	if (it != perceptionMap.end())
	{
		// Update an existing entry
		(*it).second.timeLastPercepted = time;
		(*it).second.lastPerceptedPos = object->position;
		object->sendGetMessage(COMPMSG_GET_VELOCITY, &(*it).second.lastPerceptedVelocity);
		(*it).second.faction = "";
		object->sendGetMessage(COMPMSG_GET_FACTION, &(*it).second.faction);

		if (inViewKnown)
			(*it).second.inView = inView;
	}
	else
	{
		// Create a new entry
		AIPerceptedObject newObject;
		newObject.timeFirstPercepted = time;
		newObject.timeLastPercepted = time;
		newObject.lastPerceptedPos = object->position;
		object->sendGetMessage(COMPMSG_GET_VELOCITY, &newObject.lastPerceptedVelocity);
		newObject.inView = inViewKnown ? inView : false;
		newObject.faction = "";
		object->sendGetMessage(COMPMSG_GET_FACTION, &newObject.faction);
		perceptionMap.insert(PerceptionMap::value_type(object->id, newObject));
	}
}

bool AIPerception::viewCheck(Vector3 viewDir, Object* obj)
{
	// Is the object in the view range?
	Vector3 toObject = obj->position - owner->position;
	float distToObjectSq = toObject.squaredLength();
	if (distToObjectSq > viewRangeSq)
		return false;

	// Is the object in the field of view?
	float distToObject = sqrt(distToObjectSq);
	if (distToObject != 0.0f)
		toObject /= distToObject;
	if (viewDir.dotProduct(toObject) < fieldOfView)
		return false;

	// Do a ray test as the final step
	Vector3 hitPos;
	if (!luaRayTestCombined(&owner->position, &toObject, distToObject, &hitPos, owner, RAYTEST_NO_DEBRIS))
		return false;
		
	Object* hitObj = luaGetLastHitObject();
	if ((!hitObj) || (hitObj != obj))
		return false;

	return true;
}

void AIPerception::updateTime(float dt)
{
	time += dt;
}

void AIPerception::updateVision(Vector3 viewDir)
{
	// Update memory records
	for (PerceptionMapItr it = perceptionMap.begin(); it != perceptionMap.end(); )
	{
		if (time - (*it).second.timeLastPercepted > memorySpan)
		{
			// delete the memory entry
			PerceptionMapItr itNext = it;
			itNext++;
			perceptionMap.erase(it);
			it = itNext;
		}
		else
		{
			// check if the object still exists and if it is in view
			Object* obj = objectManager.getObjectByID((*it).first);
			if (!obj)
			{
				// delete the object from the agent's memory if it is in view - the agent saw how it was deleted. Otherwise, keep it
				if ((*it).second.inView)
				{
					PerceptionMapItr itNext = it;
					itNext++;
					perceptionMap.erase(it);
					it = itNext;
				}
				else
					it++;
			}
			else
			{
				(*it).second.inView = false;	// Just set inView to false because "Handle vision" follows which will set it to true again if it is in view
				it++;
			}
		}
	}
	
	// Handle vision
	for (ObjectManager::iterator it = objectManager.begin(); it != objectManager.end(); it++)
	{
		Object* obj = (*it).second;

		// Don't process the owner itself
		if (obj == owner)
			continue;

		// Is the object of the right type?
		if (!(obj->tag & tag))
			continue;

		// Check field of view and do a ray test
		if (!viewCheck(viewDir, obj))
			continue;
		
		// The owner of this class can see the object!
		updatePerceptedObject(obj, true, true);
	}
}

void AIPerception::updateNeighbours(AIGetNeighbours* neighbours)
{
	for (AIGetNeighbours::iterator it = neighbours->begin(); it != neighbours->end(); it++)
	{
		if (!((*it)->tag & tag))
			continue;

		updatePerceptedObject(*it);
	}
}

void AIPerception::updateByMessage(AIPerception* message)
{
	// If the other AIPerception has objects in view, update only with those objects, else update with all objects
	// This should ensure that objects in view get priority
	PerceptionMapItr it;
	bool takeOutOfView = false;
	
	for (it = message->begin(); it != message->end(); it++)
	{
		if ((*it).second.inView)
			break;
	}

	if (it == message->end())
		takeOutOfView = true;

	for (it = message->begin(); it != message->end(); it++)
	{
		if ((!takeOutOfView) && (!(*it).second.inView))
			continue;

		if ((*it).first == owner->id)
			continue;

		Object* obj = objectManager.getObjectByID((*it).first);	// this also prevents multiple agents reporting just memories to each other which are thus never forgotten
		if (!obj)
			continue;

		if (!(obj->tag & tag))
			continue;

		updatePerceptedObject(obj);
	}
}

AIPerception::PerceptionMap AIPerception::getObjectsInView()
{
	PerceptionMap inViewMap;

	for (PerceptionMapItr it = perceptionMap.begin(); it != perceptionMap.end(); it++)
	{
		if ((*it).second.inView)
			inViewMap.insert(PerceptionMap::value_type((*it).first, (*it).second));
	}

	return inViewMap;
}

AIPerceptedObject* AIPerception::getPerceptionByID(int objectID)
{
	PerceptionMapItr it = perceptionMap.find(objectID);
	if (it == perceptionMap.end())
		return NULL;
	else
		return &(*it).second;
}
