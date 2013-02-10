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

#ifndef _AI_HANDLER_BAT_H_
#define _AI_HANDLER_BAT_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "componentAI.h"
#include "aiStateMachine.h"
#include "aiPerception.h"

class JitteredIntervalTimer;
class AIHandlerBat;
class AIMovementController;
class AIGetNeighbours;

/// State used to fight against enemies
class AIStateBatCombat : public AIState<AIHandlerBat>
{
public:
	Vector3 battlePos;
	float targetNotInViewTimer;
	int state;	// 0: fly to target and bite; 1: fly away

	float checkBattlePos(Vector3& testPos, Object* targetObj, bool moveAwayFromNeighbours);
	void getNewBattlePos(bool pickCurrentPosIfNotMoving = false);

	virtual void init();
	virtual void update(float time);
	virtual void exit();
};

/// State used to guard a position
class AIStateBatGuard : public AIState<AIHandlerBat>
{
public:
	Vector3 target;

	void newTarget();

	virtual void init();
	virtual void update(float time);
	virtual void exit();
};

/// AI handler for bats
class AIHandlerBat : public AIHandler
{
public:
	AIPerception* perception;
	JitteredIntervalTimer* perceptionTimer;

	AIGetNeighbours* neighbours;

	AIMovementController* movementController;
	AIStateMachine<AIHandlerBat>* stateMachine;

	float time;

	bool hasTarget;
	int target;
	AIPerceptedObject* targetEntry;

	Vector3 guardPos;
	Vector3 desiredHeading;
	btVector3 angularVelocity;
	
	bool hasAnimFly;

	void getTargetFromList(AIPerception::PerceptionMap& list, bool pickLastPercepted);
	void removeTargetsWithoutAttitudeFromList(AIPerception::PerceptionMap& list, int attitudeToPreserve);
	void doTargetSelection();

	/// Sends a COMPMSG_AI_PERCEPTION_NOTIFICATION to all friendly neighbours
	void notifyNeighboursOfPerceptedAgents();

	virtual void perceptionNotification(AIPerception* msg);
	virtual void attackedBy(Object* obj, float damage);

	virtual void posUpdate(float timeSinceLastFrame);
	virtual void logicUpdate();

	virtual void init();
	virtual void exit();
};

#endif
