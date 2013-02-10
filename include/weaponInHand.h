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

#ifndef _WEAPON_IN_HAND_H_
#define _WEAPON_IN_HAND_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "componentPosition.h"

class ComponentWeapon;
class Component;

/// Handles the display and animation of the weapon in hands
class WeaponInHand
{
public:
	// Pointers to active and, if changing, next weapon; both may be NULL
	ComponentWeapon* actWeapon;
	ComponentWeapon* nextWeapon;
	bool isNextWeaponValid;	// nextWeapon might be NULL and valid (for example when throwing a grenade)
	bool weaponChangeStarted;

	/// The current offset of the weapon
	ComponentPosition offset;
	/// The target for the offset; it will be interpolated to this value
	ComponentPosition offsetTo;
	/// The velocity with which the weapon is moving
	Vector3 moveVelocity;
	/// The velocity with which the weapon is rotating
	Vector3 rotVelocity;

	/// The scenenode the weapon is attached to
	SceneNode* scenenode;
	uint8 entityOldRenderQueueGroup;

	/// The weapon holder
	Component* holder;

	/// Starts the process of changing the weapon
	void changeWeapon(ComponentWeapon* changeTo);

	/// Returns if the weapon doesn't fire at the moment and is held in the hand (not half-readied)
	bool isReady();

	/// Starts firing. 0-LMB, 1-RMB
	void fire(int mode);

	/// Should be called every frame
	void update(float dt);
	float time;
	float idleTime;		// if this is above a certain threshold, the offsetTo is set to offset + small random value to introduce some idle movement
	float smoothTime;	// smoothTime for the interpolation of position and rotation

	/// Creates the scene node, etc.
	void init(SceneNode* parentNode, Component* holder);

	/// Cleans up
	void exit();

	WeaponInHand() {scenenode = NULL;}

	static Vector3 WEAPONSTART_BELOW;
	static Vector3 WEAPONSTART_VELOCITY;

	void _attachWeapon();
	void _detachWeapon();
	void _setWeaponStartPosition(Vector3 where);
};

#endif
