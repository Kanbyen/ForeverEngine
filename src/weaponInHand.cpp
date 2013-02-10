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
#include "weaponInHand.h"
#include "componentWeapon.h"
#include "componentMesh.h"
#include "object.h"
#include "physicsConstants.h"
#include "util.h"
#include "objectScript.h"

Vector3 WeaponInHand::WEAPONSTART_BELOW = Vector3(0, -1.2f, 0);
Vector3 WeaponInHand::WEAPONSTART_VELOCITY = Vector3(0, 1.0f, 0);

void WeaponInHand::_attachWeapon()
{
	if (!actWeapon)
		return;
	if (!actWeapon->object->mesh)
		return;

	// Attach entity to scenenode
	Entity* entity = actWeapon->object->mesh->entity;
	scenenode->attachObject(entity);

	// Change the render queue group
	entityOldRenderQueueGroup = entity->getRenderQueueGroup();
	entity->setRenderQueueGroup(RENDER_QUEUE_8);

	// Set holder
	actWeapon->holder = holder;
}
void WeaponInHand::_detachWeapon()
{
	scenenode->detachAllObjects();
	
	if (!actWeapon)
		return;
	if (!actWeapon->object->mesh)
		return;

	// Restore old render queue group
	Entity* entity = actWeapon->object->mesh->entity;
	entity->setRenderQueueGroup(entityOldRenderQueueGroup);

	// Remove holder
	actWeapon->holder = NULL;
}
void WeaponInHand::_setWeaponStartPosition(Vector3 where)
{
	if (!actWeapon)
		return;
	
	offset = actWeapon->holdPos;
	offset.position += where;
	offsetTo = actWeapon->holdPos;
	offset.applyTo(scenenode);
	moveVelocity = WEAPONSTART_VELOCITY;
	rotVelocity = Vector3::ZERO;
	smoothTime = 0.12f;
}

void WeaponInHand::changeWeapon(ComponentWeapon* changeTo)
{
	if (!actWeapon)
	{
		// This is the first weapon the player has
		actWeapon = changeTo;
		_attachWeapon();
		_setWeaponStartPosition(WEAPONSTART_BELOW);
	}
	else if (changeTo == actWeapon)
	{
		// Player changes back to active weapon. If he wanted to switch to another weapon in the meantime, stop this
		nextWeapon = NULL;
		offsetTo = actWeapon->holdPos;
		isNextWeaponValid = false;
	}
	else
	{
		// Change the weapon
		nextWeapon = changeTo;
		weaponChangeStarted = false;
		isNextWeaponValid = true;
	}
}
bool WeaponInHand::isReady()
{
	if (weaponChangeStarted)
		return false;
	if (actWeapon)
	{
		if (actWeapon->scriptLMB)
			if (actWeapon->scriptLMB->isRunning())
				return false;
		
		if (actWeapon->scriptRMB)
			if (actWeapon->scriptRMB->isRunning())
				return false;
	}
	if ((offsetTo.position - offset.position).squaredLength() > 0.01f)
		return false;

	return true;
}
void WeaponInHand::fire(int mode)
{
	if (!actWeapon)
		return;
	if (!isReady())
		return;

	if (mode == 0)
	{
		if (actWeapon->scriptLMB)
			actWeapon->scriptLMB->run();
		smoothTime = 0.12f;
	}
	else if (mode == 1)
	{
		if (actWeapon->scriptRMB)
			actWeapon->scriptRMB->run();
		smoothTime = 0.12f;
	}
}
void WeaponInHand::update(float dt)
{
	time += dt;

	// Idle movement
	const float IDLE_THRESHOLD = 1.3f;
	const float IDLE_THRESHOLD_VARIANCE = 0.3f;
	if (actWeapon && isReady())
	{
		idleTime += dt;
		if (idleTime >= IDLE_THRESHOLD)
		{
			smoothTime = 1.0f;
			idleTime -= IDLE_THRESHOLD + Math::RangeRandom(-IDLE_THRESHOLD_VARIANCE, IDLE_THRESHOLD_VARIANCE);
			offsetTo.position = actWeapon->holdPos.position + Vector3(Math::RangeRandom(-0.007f, 0.007f), Math::RangeRandom(-0.005f, 0.005f), Math::RangeRandom(-0.005f, 0.005f));
			offsetTo.rotation = actWeapon->holdPos.rotation + Vector3(Math::RangeRandom(-0.3f, 0.3f), Math::RangeRandom(-0.3f, 0.3f), Math::RangeRandom(-0.4f, 0.4f));
		}
	}
	else
		idleTime = 0.9f;

	// Interpolate the offset
	if (actWeapon)
	{
		offset.position = smoothInterpolate(offset.position, offsetTo.position, moveVelocity, smoothTime, LOGIC_TIME_STEP);
		offset.rotation = smoothInterpolate(offset.rotation, offsetTo.rotation, rotVelocity, smoothTime, LOGIC_TIME_STEP);
		offset.calculateQuaternion();
		offset.applyTo(scenenode);
	}

	// Change to a new weapon?
	if (isNextWeaponValid)
	{
		if (weaponChangeStarted)
		{
			if ((offsetTo.position - offset.position).squaredLength() < 0.01f)
			{
				_detachWeapon();
				actWeapon = nextWeapon;
				nextWeapon = NULL;
				isNextWeaponValid = false;
				weaponChangeStarted = false;
				_attachWeapon();
				_setWeaponStartPosition(WEAPONSTART_BELOW);
			}
		}
		else
		{
			if (isReady())
			{
				weaponChangeStarted = true;
				offsetTo = actWeapon->holdPos;
				offsetTo.position += WEAPONSTART_BELOW;
				smoothTime = 0.12f;
			}
		}
	}

	// Execute scripts
	if (actWeapon)
	{
		if (actWeapon->scriptLMB)
			actWeapon->scriptLMB->update(dt);
		if (actWeapon->scriptRMB)
			actWeapon->scriptRMB->update(dt);
	}
}
void WeaponInHand::init(SceneNode* parentNode, Component* holder)
{
	time = 0.0f;
	idleTime = 0.0f;
	smoothTime = 0.12f;
	actWeapon = NULL;
	nextWeapon = NULL;
	isNextWeaponValid = false;
	weaponChangeStarted = false;

	moveVelocity = Vector3::ZERO;
	rotVelocity = Vector3::ZERO;
	
	scenenode = parentNode->createChildSceneNode();
	this->holder = holder;
}
void WeaponInHand::exit()
{
	if (!scenenode)
		return;
	
	_detachWeapon();
	sceneMgr->destroySceneNode(scenenode->getName());
	scenenode = NULL;
}
