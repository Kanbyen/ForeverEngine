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
#include "aiHandlerBat.h"
#include "physicsConstants.h"
#include "aiMovementController.h"
#include "aiGetNeighbours.h"
#include "aiPerception.h"
#include "object.h"
#include "componentMesh.h"
#include "util.h"
#include "stateGame.h"
#include "volume.h"
#include "aiFactionManager.h"
#include "aiPositionRating.h"

// TODO: expose these constants in the .object files
const float BAT_PATROL_RADIUS = 15.0f;
const float BAT_COMBAT_RADIUS = 10.0f;
const float	BAT_NO_PROGRESS_TIMEOUT = 0.5f;			// timeout when no progress is made at getting to the desired target position
const float BAT_CHANGE_POS_WHILE_RELOADING_CHANCE = 1.0f;

const float BAT_MAX_FORCE = 50.0f;
const float BAT_MAX_SPEED = 25.0f;
const float MOVING_THRESHOLD = 4.0f;

const float BAT_NEIGHBOUR_RADIUS = 16.0f;
const float BAT_VIEW_RANGE = 90.0f;
const float BAT_FIELD_OF_VIEW = 80.0f;
const float BAT_MEMORY_SPAN = 10.0f;					// how long to remember objects which went out of view
const float BAT_TARGET_NOT_IN_VIEW_THRESHOLD = 0.2f;	// time to stay in combat mode when standing still and the target is out of view

const float BAT_WEIGHT_SEPARATION = 30.0f;
const float BAT_WEIGHT_WALL_AVOIDANCE = 30.0f;
const float BAT_WEIGHT_SEEK = 20.0f;

float AIStateBatCombat::checkBattlePos(Vector3& testPos, Object* targetObj, bool moveAwayFromNeighbours)
{
	float score;

	Object* obj = myOwner->aiComp->object;

	// Move away from neighbours
	if (moveAwayFromNeighbours)
		testPos = AIPositionRating::moveAwayFromNeighbours(testPos, obj, 8.0f, 14.0f, 2);

	// Is the position in the ground?
	score = AIPositionRating::getPositionInAir(testPos, 2);
	if (score < 0.2f)
		return -100.0f;	// a battle position in the ground is not of much use
	else if (score > 0.8f)
		score = 0.8f;	// if there is air, don't respect small nuances - other criteria should weight stronger!

	// Can the current target be shot at from this position?
	if (!AIPositionRating::getFreeLineOfFireToTarget(testPos, obj, targetObj, BAT_VIEW_RANGE))
		score -= 1.2f;

	// Can the agent get to this position in a straigt line?
	if (!AIPositionRating::getFreeWayToPos(testPos, obj))
		score -= 0.4f;

	// Medium bonus for every blocked line-of-fire to other threats
	// TODO

	// Big penalty if the position is in the line-of-fire of a friendly agent
	// TODO: respect line-of-fire of friends

	return score;
}
void AIStateBatCombat::getNewBattlePos(bool pickCurrentPosIfNotMoving)
{
	Vector3 currentPos = myOwner->movementController->currentPos;
	Vector3 testPos;
	Vector3 bestPos = Vector3::ZERO;
	float bestScore = -10000.0f;

	// Do we have a target? If not, the battle sphere stays at its current position
	Object* targetObj = objectManager.getObjectByID(myOwner->target);
	if (!targetObj)
	{
		battlePos = currentPos;
		return;
	}

	// Check 2 random positions and the current position, take a position if the score is positive. If all are negative, take the one with the best score
	for (int i = 0; i < 2; i++)
	{
		testPos = Vector3(Math::RangeRandom(-BAT_COMBAT_RADIUS, BAT_COMBAT_RADIUS),
								Math::RangeRandom(-BAT_COMBAT_RADIUS, BAT_COMBAT_RADIUS),
								Math::RangeRandom(-BAT_COMBAT_RADIUS, BAT_COMBAT_RADIUS));
		testPos += currentPos;

		float score = checkBattlePos(testPos, targetObj, true);
		if (score > 0)
		{
			battlePos = testPos;
			return;
		}
		else if (score >= bestScore)
		{
			bestScore = score;
			bestPos = testPos;
		}
	}

	if (myOwner->movementController->velocity.squaredLength() > 0)
		testPos = currentPos + myOwner->movementController->velocity.normalisedCopy();
	else
		testPos = currentPos;
	float score = checkBattlePos(testPos, targetObj, false);
	if (score > 0)
	{
		battlePos = testPos;
		return;
	}
	else if (score >= bestScore)	// >= so that the current position is taken if all are equally bad
	{
		bestScore = score;
		bestPos = testPos;
	}

	battlePos = bestPos;
}
void AIStateBatCombat::init()
{
	myOwner->movementController->resetMovementTypes();
	myOwner->movementController->wallAvoidanceOn();
	//myOwner->movementController->separationOn();
	myOwner->movementController->seekOn();

	battlePos = myOwner->targetEntry->getPerceptedPosition(myOwner->target);
	myOwner->movementController->setTarget(battlePos);

	targetNotInViewTimer = 0;
	state = 0;
}
void AIStateBatCombat::update(float time)
{
	if (!myOwner->hasTarget)
	{
		myOwner->stateMachine->changeState(new AIStateBatGuard());
		return;
	}
	
	if (state == 0)
	{
		battlePos = myOwner->targetEntry->getPerceptedPosition(myOwner->target);
		myOwner->movementController->updateTarget(battlePos);
	}
	
	// TODO: if bite sucessful, do this:
	/*
		state = 1;
		getNewBattlePos();
		myOwner->movementController->setTarget(battlePos);
		
		// and set a timer to return to state 0, ALSO BELOW
	*/

	if (myOwner->movementController->getMoveToTargetFailed())
	{
		state = 1;
		getNewBattlePos();
		myOwner->movementController->setTarget(battlePos);
		//game.gameConsole->addMessage("new combat target");
		
		// and set a timer to return to state 0
	}

	if (myOwner->movementController->velocity.squaredLength() > MOVING_THRESHOLD)
		targetNotInViewTimer = 0;	// If we are moving to the battlePos, it's not neccesary that we face the target
	else
	{
		// if we are not moving and the target is out of view, advance the targetNotInViewTimer
		if (!myOwner->targetEntry->inView)
		{
			targetNotInViewTimer += time;
			if (targetNotInViewTimer > BAT_TARGET_NOT_IN_VIEW_THRESHOLD)
			{
				myOwner->stateMachine->changeState(new AIStateBatGuard());
				return;
			}
		}
		else
			targetNotInViewTimer = 0;

		// try to face the target
		Vector3 targetPos = myOwner->targetEntry->getPerceptedPosition(myOwner->target);
		Vector3 tempHeading = (targetPos - myOwner->movementController->currentPos).normalisedCopy();
		if (!tempHeading.isZeroLength())
			myOwner->desiredHeading = tempHeading;
	}
}
void AIStateBatCombat::exit()
{
}
void AIStateBatGuard::newTarget()
{
	// Uncomment if you want to test steering behaviours ...
	/*target = Vector3::ZERO;
	myOwner->movementController->setTarget(target);
	return;*/

	// Check for suitable positions (up to 8 times), take one if it is considered good
	for (int i = 0; i < 8; i++)
	{
		target = Vector3(Math::RangeRandom(-BAT_PATROL_RADIUS, BAT_PATROL_RADIUS),
							   Math::RangeRandom(-BAT_PATROL_RADIUS, BAT_PATROL_RADIUS),
							   Math::RangeRandom(-BAT_PATROL_RADIUS, BAT_PATROL_RADIUS));

		if (myOwner->hasTarget)
			target += myOwner->targetEntry->getPerceptedPosition(myOwner->target);
		else
			target += myOwner->guardPos;

		// Move away from neighbours
		target = AIPositionRating::moveAwayFromNeighbours(target, myOwner->aiComp->object, 8.0f, 14.0f, 2);

		// Check if the position is free
		Vector3 scaledTarget = target / game.volumeData->scale;
		if ((!game.volumeData->containsPointAbs(scaledTarget)) ||
			(AIPositionRating::getPositionInAir(target, 2) > 0.7f))
		{
			myOwner->movementController->setTarget(target);
			return;
		}
	}

	// Didn't find a free position, take the current position
	target = myOwner->movementController->currentPos;
	myOwner->movementController->setTarget(target);
}
void AIStateBatGuard::init()
{
	myOwner->movementController->resetMovementTypes();
	myOwner->movementController->wallAvoidanceOn();
	myOwner->movementController->separationOn();
	myOwner->movementController->seekOn();

	newTarget();
	update(0);
}
void AIStateBatGuard::update(float time)
{
	if (myOwner->hasTarget && myOwner->targetEntry->inView)
	{
		myOwner->stateMachine->changeState(new AIStateBatCombat());
		return;
	}

	if (myOwner->movementController->getMoveToTargetFailed())
	{
		//game.gameConsole->addMessage("gave up");
		newTarget();
	}
	else
	{
		if (myOwner->movementController->currentPos.squaredDistance(target) < 3.0f*3.0f)
		{
			//game.gameConsole->addMessage("target reached");
			newTarget();
		}
	}
}
void AIStateBatGuard::exit()
{
}


void AIHandlerBat::init()
{
	time = 0;

	hasTarget = false;
	guardPos = aiComp->object->position;
	desiredHeading = Vector3::NEGATIVE_UNIT_Z;
	angularVelocity = btVector3(0, 0, 0);

	// Get the physics component and disable deactivation on the rigid body
	ComponentPhysics* physComp;
	aiComp->object->sendGetMessage(COMPMSG_GET_PHYSICS_COMPONENT, &physComp);
	aiComp->rigidBody->setActivationState(DISABLE_DEACTIVATION);

	// Initialize class to find neighbours
	neighbours = new AIGetNeighbours(BAT_NEIGHBOUR_RADIUS, aiComp->object, OBJTAG_BULLET);

	// Initialize movement controller
	movementController = new AIMovementController(BAT_MAX_FORCE, BAT_MAX_SPEED, physComp, neighbours);
	movementController->setNoProgressTimeout(BAT_NO_PROGRESS_TIMEOUT);
	movementController->weightSeek = BAT_WEIGHT_SEEK;
	movementController->weightWallAvoidance = BAT_WEIGHT_WALL_AVOIDANCE;
	movementController->weightSeparation = BAT_WEIGHT_SEPARATION;

	// Start with "guard" state
	stateMachine = new AIStateMachine<AIHandlerBat>(this);
	stateMachine->changeState(new AIStateBatGuard());

	// Initialize perception
	perception = new AIPerception(OBJTAG_AGENT, BAT_VIEW_RANGE, Degree(BAT_FIELD_OF_VIEW), BAT_MEMORY_SPAN, aiComp->object);
	perceptionTimer = new JitteredIntervalTimer(1.0f / 3.0f);
	
	// Check if the mesh has a "Fly" animation
	ComponentMesh* mesh = aiComp->object->mesh;
	if (mesh)
	{
		SkeletonInstance* sk = mesh->entity->getSkeleton();

		// Set all bones to "manually controlled"
		if (sk)
		{
			for (int b = 0; b < (int)sk->getNumBones(); ++b)
				sk->getBone(b)->setManuallyControlled(true);

			// TODO: move the hasAnim variable(s) into the template, initialize them upon creation of first instance
			hasAnimFly = sk->hasAnimation("Fly");
		}
	}
}
void AIHandlerBat::notifyNeighboursOfPerceptedAgents()
{
	// Send a COMPMSG_AI_PERCEPTION_NOTIFICATION to all nearby friendly agents
	for (AIGetNeighbours::iterator it = neighbours->begin(); it != neighbours->end(); it++)
	{
		if (!((*it)->tag & OBJTAG_AGENT))
			continue;

		string faction = "";
		(*it)->sendGetMessage(COMPMSG_GET_FACTION, &faction);
		if (aiFactionManager->getAttitude(aiComp->creator->faction, faction) != AIATTITUDE_FRIENDLY)
			continue;

		(*it)->sendMessage(COMPMSG_AI_PERCEPTION_NOTIFICATION, perception);
	}
}
void AIHandlerBat::perceptionNotification(AIPerception* msg)
{
	perception->updateByMessage(msg);
}
void AIHandlerBat::attackedBy(Object* obj, float damage)
{
	perception->updatePerceptedObject(obj);
}
void AIHandlerBat::getTargetFromList(AIPerception::PerceptionMap& list, bool pickLastPercepted)
{
	// Select a target from the given list

	// TODO: move this into AIPerception?

	if (hasTarget)
	{
		// Try to find the old target in the map
		AIPerception::iterator it = list.find(target);
		if (it != list.end())
		{
			// The target is still there, keep it
			return;
		}
	}

	// Pick a new target
	if (pickLastPercepted)
	{
		// Pick the target which was last percepted
		float lastPerceptTime = -1;
		for (AIPerception::iterator it = list.begin(); it != list.end(); it++)
		{
			if ((*it).second.timeLastPercepted > lastPerceptTime)
			{
				lastPerceptTime = (*it).second.timeLastPercepted;
				target = (*it).first;
			}
		}
	}
	else
	{
		// Pick a random target
		int targetNumber = (int) Math::RangeRandom(0, list.size() - 0.0001f);

		AIPerception::iterator it = list.begin();
		advance(it, targetNumber);
		target = (*it).first;
	}

	hasTarget = true;
}
void AIHandlerBat::removeTargetsWithoutAttitudeFromList(AIPerception::PerceptionMap& list, int attitudeToPreserve)
{
	for (AIPerception::iterator it = list.begin(); it != list.end();)
	{
		if (aiFactionManager->getAttitude(aiComp->creator->faction, (*it).second.faction) != attitudeToPreserve)
		{
			AIPerception::iterator itNext = it;
			itNext++;
			list.erase(it);
			it = itNext;
		}
		else
			it++;
	}
}
void AIHandlerBat::doTargetSelection()
{
	AIPerception::PerceptionMap inView = perception->getObjectsInView();
	removeTargetsWithoutAttitudeFromList(inView, AIATTITUDE_HOSTILE);
	if (inView.size())
	{
		getTargetFromList(inView, false);
	}
	else
	{
		// no targets in view
		AIPerception::PerceptionMap all = perception->perceptionMap;
		removeTargetsWithoutAttitudeFromList(all, AIATTITUDE_HOSTILE);
		if (all.size())
		{
			getTargetFromList(all, true);
		}
		else
			hasTarget = false;
	}

	if (hasTarget)
	{
		targetEntry = perception->getPerceptionByID(target);
		//game.gameConsole->addMessage(StringConverter::toString(targetEntry->inView));
	}
}
void AIHandlerBat::posUpdate(float timeSinceLastFrame)
{
	// This is called every time the physics engine is updated; we must ensure that the rigdbody has the right velocities
	btVector3 velocity = movementController->btVelocity;
	/*velocity.normalize();
	velocity *= BAT_MAX_SPEED;*/
	aiComp->rigidBody->setLinearVelocity(velocity);
	aiComp->rigidBody->setAngularVelocity(angularVelocity);
}
void AIHandlerBat::logicUpdate()
{
	time += LOGIC_TIME_STEP;

	// Calculate the current view direction (for perception and for calculating the angularVelocity)
	btQuaternion btCurQuat = aiComp->rigidBody->getOrientation();
	Quaternion curQuat = Quaternion(btCurQuat.w(), btCurQuat.x(), btCurQuat.y(), btCurQuat.z());
	Vector3 curHeading = curQuat * Vector3::NEGATIVE_UNIT_Z;

	// Update neighbours
	neighbours->update(aiComp->object->position);

	// Update vision
	perception->updateTime(LOGIC_TIME_STEP);
	if (perceptionTimer->isReady(LOGIC_TIME_STEP))
	{
		perception->updateVision(curHeading);
		perception->updateNeighbours(neighbours);
		doTargetSelection();
		notifyNeighboursOfPerceptedAgents();
	}

	// Update state machine
	stateMachine->update(LOGIC_TIME_STEP);

	// Update movement controller
	movementController->calculateVelocity(LOGIC_TIME_STEP);

	// Adjust the heading
	desiredHeading = movementController->velocity.normalisedCopy();

	// Calculate the angularVelocity for the heading
	Quaternion destQuat = curHeading.getRotationTo(desiredHeading, Vector3::UNIT_Y);
	float destYaw = destQuat.getYaw(true).valueDegrees();
	float destPitch = destQuat.getPitch(true).valueDegrees();
	float destRoll = destQuat.getRoll(true).valueDegrees();

	angularVelocity = btVector3(destPitch / 20, destYaw / 20, destRoll / 20);

	// Update velocities
	posUpdate(0);
	
	// Update animation
	ComponentMesh* mesh = aiComp->object->mesh;
	if (hasAnimFly && mesh)
	{
		SkeletonInstance* sk = mesh->entity->getSkeleton();
		if (sk)
		{
			sk->reset(true);
	
			Animation* anim = sk->getAnimation("Fly");
			anim->apply(sk, time);
		}
	}
}
void AIHandlerBat::exit()
{
	delete neighbours;
	delete perceptionTimer;
	delete perception;
	delete stateMachine;
	delete movementController;
}
