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

#ifndef _AI_PERCEPTION_H_
#define _AI_PERCEPTION_H_

#include <map>
#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "object.h"

class AIGetNeighbours;

class AIPerceptedObject
{
public:
	// self - explanatory!
	float timeFirstPercepted;
	float timeLastPercepted;

	/// The position where the object was last percepted
	Vector3 lastPerceptedPos;

	/// The velocity of the object as it was last percepted
	Vector3 lastPerceptedVelocity;

	/// Is the object in view / Could the owner of the perception class shoot at the object in a straight line?
	bool inView;

	/// The faction name of the object, if any
	string faction;

	/// Gets the position of the object, in the view of the owner. If the object is in view, this is the real position, if not, this is the position where it was last seen.
	Vector3 getPerceptedPosition(int id);
};

class AIPerception
{
public:
	typedef std::map<int, AIPerceptedObject> PerceptionMap;
	typedef std::map<int, AIPerceptedObject>::iterator PerceptionMapItr;

	/// Constructor; the field of view angle stands just for one side, so if you specify 90° you see everything in front of you!
	AIPerception(ObjectTag tag, float viewRange, Degree fieldOfView, float memorySpan, Object* owner);
	~AIPerception();

	/// Must be called to update the internal timer
	void updateTime(float dt);

	/// Should be called regularly to update the list of percepted objects; also deletes old entries. viewDir must be normalized.
	void updateVision(Vector3 viewDir);

	/// Can be called to make the owner aware of its neighbours
	void updateNeighbours(AIGetNeighbours* neighbours);

	/// Can be called to add the percepted objects of another AIPerception
	void updateByMessage(AIPerception* message);

	/// Call this if the owner can percieve the object in any way. You can optionally specify if the object is shootable if you know it.
	void updatePerceptedObject(Object* object, bool inViewKnown = false, bool inView = false);

	/// Checks if the owner can see the object
	bool viewCheck(Vector3 viewDir, Object* obj);

	/// Gets the AIPerceptedObject entry for the object with the given id or NULL if none exists
	AIPerceptedObject* getPerceptionByID(int objectID);

	/// Returns a list of objects in view
	PerceptionMap getObjectsInView();

	/// Maps object ids to AIPerceptedObject entries
	PerceptionMap perceptionMap;

	/// The object to which this class belongs to
	Object* owner;

	/// The AIPerception class will find objects with this tag
	ObjectTag tag;

	/// The cosine of the field of view angle (to compare it with the dot product of two vectors)
	float fieldOfView;

	/// How long should objects which are not percepted anymore stay in memory?
	float memorySpan;

	/// Squared maximum view distance
	float viewRangeSq;

	// The current time
	float time;

	// Let the class behave like a map of object ID -> AIPerceptedObject
	typedef PerceptionMapItr iterator;
	inline iterator begin()		{ return perceptionMap.begin(); }
	inline iterator end()		{ return perceptionMap.end(); }
	inline int size()			{ return perceptionMap.size(); }
};

#endif
