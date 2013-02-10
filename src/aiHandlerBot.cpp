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
#include "aiHandlerBot.h"
#include "aiGetNeighbours.h"
#include "aiPerception.h"
#include "object.h"
#include "aiFactionManager.h"
#include "physicsConstants.h"
#include "util.h"
#include "componentCharacterController.h"
#include "componentInventory.h"
#include "physics.h"

const float BOT_NEIGHBOUR_RADIUS = 16.0f;
const float BOT_VIEW_RANGE = 90.0f;
const float BOT_FIELD_OF_VIEW = 80.0f;
const float BOT_MEMORY_SPAN = 15.0f;

void AIHandlerBot::selectWeapon()
{
	// begin() gives us the iterator to the most important weapon
	ComponentInventory::ObjectsItr itr = myInventory->begin(ITEMTYPE_WEAPON);
	Object* weapon = (*itr)->object;
	myCharacterController->selectWeapon(weapon);
}

void AIHandlerBot::perceptionNotification(AIPerception* msg)
{
	perception->updateByMessage(msg);
}
void AIHandlerBot::attackedBy(Object* obj, float damage)
{
	perception->updatePerceptedObject(obj);
}
void AIHandlerBot::posUpdate(float timeSinceLastFrame)
{
	const bool move = moveDirection != Vector3::ZERO;
	const bool wantsToJump = false;
	myCharacterController->step(move, moveDirection, wantsToJump, timeSinceLastFrame);
}
void AIHandlerBot::init()
{
	// Get the physics component and disable deactivation on the rigid body
	ComponentPhysics* physComp;
	aiComp->object->sendGetMessage(COMPMSG_GET_PHYSICS_COMPONENT, &physComp);
	aiComp->rigidBody->setActivationState(DISABLE_DEACTIVATION);

	// Initialize class to find neighbours
	neighbours = new AIGetNeighbours(BOT_NEIGHBOUR_RADIUS, aiComp->object, OBJTAG_BULLET);

	// Initialize perception
	perception = new AIPerception(OBJTAG_AGENT, BOT_VIEW_RANGE, Degree(BOT_FIELD_OF_VIEW), BOT_MEMORY_SPAN, aiComp->object);
	perceptionTimer = new JitteredIntervalTimer(1.0f / 3.0f);

	// Get our character controller
	if (!aiComp->object->sendGetMessage(COMPMSG_GET_CHARACTER_CONTROLLER, &myCharacterController))
		LogManager::getSingleton().logMessage("WARNING: no character controller component in an object with AIHandlerBot!");
	
	// Get our inventory
	if (!aiComp->object->sendGetMessage(COMPMSG_GET_INVENTORY, &myInventory))
		LogManager::getSingleton().logMessage("WARNING: no inventory component in an object with AIHandlerBot!");
	
	// Use the best weapon which is currently in our inventory
	selectWeapon();

	time = 0;
	aimDirection = Vector3(1, 0, 0);
	moveDirection = Vector3::ZERO;
}
void AIHandlerBot::logicUpdate()
{
	time += LOGIC_TIME_STEP;

	// Movement
	// TODO!
	aimDirection = Vector3(sinf(0.5f * time), sinf(0.5f * time), cosf(0.5f * time)).normalisedCopy();
	moveDirection = Vector3(sinf(time), 0, cosf(time)).normalisedCopy();
	aiComp->object->sendMessage(COMPMSG_SET_AIM, &aimDirection, NULL, aiComp);
	
	// Weapon firing
	myCharacterController->fireWeapon(0);

	// Update neighbours
	neighbours->update(aiComp->object->position);

	// Update vision
	perception->updateTime(LOGIC_TIME_STEP);
	if (perceptionTimer->isReady(LOGIC_TIME_STEP))
	{
		perception->updateVision(aimDirection);
		perception->updateNeighbours(neighbours);
		//doTargetSelection();
		//notifyNeighboursOfPerceptedAgents();
	}

	// Update velocities
	posUpdate(0);
}
void AIHandlerBot::exit()
{
	delete neighbours;
	delete perceptionTimer;
	delete perception;
}
