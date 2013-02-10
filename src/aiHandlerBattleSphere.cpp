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
#include "aiHandlerBattleSphere.h"
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
const float BATTLE_SPHERE_PATROL_RADIUS = 5.0f;
const float BATTLE_SPHERE_COMBAT_RADIUS = 4.5f;
const float	BATTLE_SPHERE_NO_PROGRESS_TIMEOUT = 1.3f;			// timeout when no progress is made at getting to the desired target position
const float BATTLE_SPHERE_CHANGE_POS_WHILE_RELOADING_CHANCE = 1.0f;

const float BATTLE_SPHERE_MAX_FORCE = 30.0f;
const float BATTLE_SPHERE_MAX_SPEED = 15.0f;

const float MOVING_THRESHOLD = 4.0f;

const int BATTLE_SPHERE_SHOTS_UNTIL_RELOAD = 10;
const float BATTLE_SPHERE_RELOAD_TIME = 2.0f;
const float BATTLE_SPHERE_FIRE_DELAY = 0.13f;

const char* BATTLE_SPHERE_BULLET = "energy bullet";
const float BATTLE_SPHERE_FIRE_ANGLE_THRESHOLD = 0.92f;

const float BATTLE_SPHERE_NEIGHBOUR_RADIUS = 16.0f;
const float BATTLE_SPHERE_VIEW_RANGE = 90.0f;
const float BATTLE_SPHERE_FIELD_OF_VIEW = 80.0f;
const float BATTLE_SPHERE_MEMORY_SPAN = 15.0f;					// how long to remember objects which went out of view
const float BATTLE_SPHERE_TARGET_NOT_IN_VIEW_THRESHOLD = 0.2f;	// time to stay in combat mode when standing still and the target is out of view

const float BATTLE_SPHERE_WEIGHT_SEPARATION = 60.0f;
const float BATTLE_SPHERE_WEIGHT_WALL_AVOIDANCE = 40.0f;
const float BATTLE_SPHERE_WEIGHT_ARRIVE = 20.0f;
const float BATTLE_SPHERE_ARRIVE_DECELERATION = 0.3f;

// Animation string(s)
const char* animStringFly = "Fly";

float AIStateBattleSphereCombat::checkBattlePos(Vector3& testPos, Object* targetObj, bool moveAwayFromNeighbours)
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
	if (!AIPositionRating::getFreeLineOfFireToTarget(testPos, obj, targetObj, BATTLE_SPHERE_VIEW_RANGE))
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
void AIStateBattleSphereCombat::getNewBattlePos(bool pickCurrentPosIfNotMoving)
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

	// If we are already (almost) standing still and the current position is ok (flag is set), simply stay there
	if (pickCurrentPosIfNotMoving && (myOwner->movementController->velocity.squaredLength() <= MOVING_THRESHOLD))
	{
		battlePos = currentPos;
		return;
	}

	// Check 2 random positions and the current position, take a position if the score is positive. If all are negative, take the one with the best score
	for (int i = 0; i < 2; i++)
	{
		testPos = Vector3(Math::RangeRandom(-BATTLE_SPHERE_COMBAT_RADIUS, BATTLE_SPHERE_COMBAT_RADIUS),
								Math::RangeRandom(-BATTLE_SPHERE_COMBAT_RADIUS, BATTLE_SPHERE_COMBAT_RADIUS),
								Math::RangeRandom(-BATTLE_SPHERE_COMBAT_RADIUS, BATTLE_SPHERE_COMBAT_RADIUS));
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
void AIStateBattleSphereCombat::init()
{
	myOwner->movementController->resetMovementTypes();
	myOwner->movementController->wallAvoidanceOn();
	myOwner->movementController->separationOn();
	myOwner->movementController->arriveOn();

	getNewBattlePos(true);
	myOwner->movementController->setTarget(battlePos);

	targetNotInViewTimer = 0;
}
void AIStateBattleSphereCombat::update(float time)
{
	if (!myOwner->hasTarget)
	{
		myOwner->stateMachine->changeState(new AIStateBattleSphereGuard());
		return;
	}

	if (myOwner->movementController->getMoveToTargetFailed())
	{
		getNewBattlePos();
		myOwner->movementController->setTarget(battlePos);
		//game.gameConsole->addMessage("new combat target");
	}

	if (myOwner->movementController->velocity.squaredLength() > MOVING_THRESHOLD)
		targetNotInViewTimer = 0;	// If we are moving to the battlePos, it's not neccesary that we face the target
	else
	{
		// if we are not moving and the target is out of view, advance the targetNotInViewTimer
		if (!myOwner->targetEntry->inView)
		{
			targetNotInViewTimer += time;
			if (targetNotInViewTimer > BATTLE_SPHERE_TARGET_NOT_IN_VIEW_THRESHOLD)
			{
				myOwner->stateMachine->changeState(new AIStateBattleSphereGuard());
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

		// check if we can shoot
		if ((myOwner->targetEntry->inView) && (myOwner->fireDelay < 0) && (myOwner->shotsUntilReload > 0))
		{
			btQuaternion btCurQuat = myOwner->aiComp->rigidBody->getOrientation();
			Quaternion curQuat = Quaternion(btCurQuat.w(), btCurQuat.x(), btCurQuat.y(), btCurQuat.z());
			Vector3 curHeading = curQuat * Vector3::NEGATIVE_UNIT_Z;
			Vector3 toTarget = (targetPos - myOwner->movementController->currentPos).normalisedCopy();

			if (curHeading.dotProduct(toTarget) > BATTLE_SPHERE_FIRE_ANGLE_THRESHOLD)
			{
				// Ready to fire!
				myOwner->shotsUntilReload--;
				myOwner->fireDelay = BATTLE_SPHERE_FIRE_DELAY;

				// Create a bullet ...
				Vector3 position = myOwner->movementController->currentPos;
				Object* thisObj = myOwner->aiComp->object;
				position += curHeading * (thisObj->mesh->sceneNode->getScale().x + 0.3f);

				Object* obj = objectManager.createObject(BATTLE_SPHERE_BULLET, myOwner->aiComp->object->id);
				obj->sendMessage(COMPMSG_CREATE, (void*)&position);
				obj->sendMessage(COMPMSG_SET_AIM, (void*)&toTarget);

				if (myOwner->shotsUntilReload == 0)
				{
					// We need to reload, so there is a chance to change position
					if (Math::RangeRandom(0, 1) < BATTLE_SPHERE_CHANGE_POS_WHILE_RELOADING_CHANCE)
					{
						getNewBattlePos();
						myOwner->movementController->setTarget(battlePos);
					}
				}
			}
		}
	}
}
void AIStateBattleSphereCombat::exit()
{
}

void AIStateBattleSphereGuard::newTarget()
{
	timeUntilTargetChange = Math::RangeRandom(1.0f, 5.0f);

	// Uncomment if you want to test steering behaviours ...
	//target = Vector3::ZERO;
	//return;

	// Check for suitable positions (up to 8 times), take one if it is considered good
	for (int i = 0; i < 8; i++)
	{
		target = Vector3(Math::RangeRandom(-BATTLE_SPHERE_PATROL_RADIUS, BATTLE_SPHERE_PATROL_RADIUS),
							   Math::RangeRandom(-BATTLE_SPHERE_PATROL_RADIUS, BATTLE_SPHERE_PATROL_RADIUS),
							   Math::RangeRandom(-BATTLE_SPHERE_PATROL_RADIUS, BATTLE_SPHERE_PATROL_RADIUS));

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
void AIStateBattleSphereGuard::init()
{
	myOwner->movementController->resetMovementTypes();
	myOwner->movementController->wallAvoidanceOn();
	myOwner->movementController->separationOn();
	myOwner->movementController->arriveOn();

	timeUntilTargetChange = 0;
	update(0);
}
void AIStateBattleSphereGuard::update(float time)
{
	if (myOwner->hasTarget && myOwner->targetEntry->inView)
	{
		myOwner->stateMachine->changeState(new AIStateBattleSphereCombat());
		return;
	}

	if (myOwner->movementController->getMoveToTargetFailed())
	{
		newTarget();
		//game.gameConsole->addMessage("new guard target");
	}
	else
	{
		timeUntilTargetChange -= time;
		if (timeUntilTargetChange <= 0)
		{
			newTarget();
			//game.gameConsole->addMessage("new guard target");
		}
	}

	if (myOwner->hasTarget && (myOwner->movementController->velocity.squaredLength() <= MOVING_THRESHOLD))
	{
		// try to face the target
		Vector3 lookAtPos = myOwner->targetEntry->getPerceptedPosition(myOwner->target) + (myOwner->targetEntry->lastPerceptedVelocity.normalisedCopy() * 30.0f);
		Vector3 tempHeading = (lookAtPos - myOwner->movementController->currentPos).normalisedCopy();
		if (!tempHeading.isZeroLength())
			myOwner->desiredHeading = tempHeading;
	}
}
void AIStateBattleSphereGuard::exit()
{
}


void AIHandlerBattleSphere::init()
{
	time = 0;

	hasTarget = false;
	guardPos = aiComp->object->position;
	desiredHeading = Vector3::NEGATIVE_UNIT_Z;
	angularVelocity = btVector3(0, 0, 0);

	fireDelay = 0;
	shotsUntilReload = BATTLE_SPHERE_SHOTS_UNTIL_RELOAD;

	// Get the physics component and disable deactivation on the rigid body
	ComponentPhysics* physComp;
	aiComp->object->sendGetMessage(COMPMSG_GET_PHYSICS_COMPONENT, &physComp);
	aiComp->rigidBody->setActivationState(DISABLE_DEACTIVATION);

	// Initialize class to find neighbours
	neighbours = new AIGetNeighbours(BATTLE_SPHERE_NEIGHBOUR_RADIUS, aiComp->object, OBJTAG_BULLET);

	// Initialize movement controller
	movementController = new AIMovementController(BATTLE_SPHERE_MAX_FORCE, BATTLE_SPHERE_MAX_SPEED, physComp, neighbours);
	movementController->setNoProgressTimeout(BATTLE_SPHERE_NO_PROGRESS_TIMEOUT);
	movementController->arriveDeceleration = BATTLE_SPHERE_ARRIVE_DECELERATION;
	movementController->weightArrive = BATTLE_SPHERE_WEIGHT_ARRIVE;
	movementController->weightWallAvoidance = BATTLE_SPHERE_WEIGHT_WALL_AVOIDANCE;
	movementController->weightSeparation = BATTLE_SPHERE_WEIGHT_SEPARATION;

	// Start with "guard" state
	stateMachine = new AIStateMachine<AIHandlerBattleSphere>(this);
	stateMachine->changeState(new AIStateBattleSphereGuard());

	// Initialize perception
	perception = new AIPerception(OBJTAG_AGENT, BATTLE_SPHERE_VIEW_RANGE, Degree(BATTLE_SPHERE_FIELD_OF_VIEW), BATTLE_SPHERE_MEMORY_SPAN, aiComp->object);
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
			hasAnimFly = sk->hasAnimation(animStringFly);
		}
	}
}
void AIHandlerBattleSphere::notifyNeighboursOfPerceptedAgents()
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
void AIHandlerBattleSphere::perceptionNotification(AIPerception* msg)
{
	perception->updateByMessage(msg);
}
void AIHandlerBattleSphere::attackedBy(Object* obj, float damage)
{
	perception->updatePerceptedObject(obj);
}
void AIHandlerBattleSphere::getTargetFromList(AIPerception::PerceptionMap& list, bool pickLastPercepted)
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
void AIHandlerBattleSphere::removeTargetsWithoutAttitudeFromList(AIPerception::PerceptionMap& list, int attitudeToPreserve)
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
void AIHandlerBattleSphere::doTargetSelection()
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
void AIHandlerBattleSphere::posUpdate(float timeSinceLastFrame)
{
	// This is called every time the physics engine is updated; we must ensure that the rigdbody has the right velocities
	aiComp->rigidBody->setLinearVelocity(movementController->btVelocity);
	aiComp->rigidBody->setAngularVelocity(angularVelocity);
}
void AIHandlerBattleSphere::logicUpdate()
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

	// Update firing variables
	fireDelay -= LOGIC_TIME_STEP;
	if (fireDelay < BATTLE_SPHERE_FIRE_DELAY - BATTLE_SPHERE_RELOAD_TIME)
		shotsUntilReload = BATTLE_SPHERE_SHOTS_UNTIL_RELOAD;

	// Update state machine
	stateMachine->update(LOGIC_TIME_STEP);

	// Update movement controller
	movementController->calculateVelocity(LOGIC_TIME_STEP);

	// Adjust the heading if the agent is moving
	if (movementController->velocity.squaredLength() > MOVING_THRESHOLD)
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
	
			Animation* anim = sk->getAnimation(animStringFly);
			anim->apply(sk, time);
		}
	}
}
void AIHandlerBattleSphere::exit()
{
	delete neighbours;
	delete perceptionTimer;
	delete perception;
	delete stateMachine;
	delete movementController;
}
