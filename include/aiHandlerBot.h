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

#ifndef _AI_HANDLER_BOT_H_
#define _AI_HANDLER_BOT_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "componentAI.h"
#include "aiStateMachine.h"
#include "aiPerception.h"

class JitteredIntervalTimer;
class AIMovementController;
class ComponentCharacterController;
class ComponentInventory;

/// AI handler for bots
class AIHandlerBot : public AIHandler
{
public:
	AIPerception* perception;
	JitteredIntervalTimer* perceptionTimer;
	AIGetNeighbours* neighbours;
	ComponentCharacterController* myCharacterController;
	ComponentInventory* myInventory;
	
	float time;

	Vector3 aimDirection;
	Vector3 moveDirection;
	
	void selectWeapon();

	virtual void perceptionNotification(AIPerception* msg);
	virtual void attackedBy(Object* obj, float damage);

	virtual void posUpdate(float timeSinceLastFrame);
	virtual void logicUpdate();

	virtual void init();
	virtual void exit();
};

#endif
