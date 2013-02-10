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

#ifndef _COMPONENT_CHARACTER_CONTROLLER_H_
#define _COMPONENT_CHARACTER_CONTROLLER_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include "btBulletDynamicsCommon.h"
using namespace Ogre;
#include "component.h"

/*const float PLAYER_HEIGHT = 5.0f;
//const float PLAYER_WIDTH = 2.3f;
//const float PLAYER_MASS = 10.0f;

const float PLAYER_GRAVITY_FACTOR = 4.0f;
const float PLAYER_MOVE_SPEED = 21.0f;
const float PLAYER_JUMP_FACTOR = 37.44f;
const float PLAYER_AIR_STEERING_FACTOR = 4.32f;*/
const float PLAYER_JETPACK_FORCE = 100.0f;

class ComponentPhysics;
class ComponentTemplateCharacterController;
class ComponentWeapon;

enum CharacterAnimation
{
	CHARANIM_RUN_FWD = 0,
	CHARANIM_RUN_BWD,
	CHARANIM_RUN_RIGHT,
	CHARANIM_RUN_LEFT,

	CHARANIM_STAND,
};

class ComponentCharacterController : public NamedComponent
{
public:

	ComponentTemplateCharacterController* creator;
	float time;

	// Animation
	Vector3 aimDirection;

	int curAnim;
	float animChangeTime;
	float animTime;
	float timeBase;

	bool hasAnimRunForward;
	bool hasAnimRunBackward;
	bool hasAnimRunRight;
	bool hasAnimRunLeft;
	bool hasAnimStand;

	// Physics
	ComponentPhysics* myPhysics;
	btRigidBody* m_rigidBody;

	btScalar m_halfHeight;

	btVector3 m_raySource;
	btVector3 m_rayTarget;
	btScalar m_rayLambda;

	// Movement
	btScalar m_walkVelocity;

	btScalar m_lastJumpTime;
	bool jumped;		// true if space is down and has lead to a jump
	bool jetpackUsedLastFrame;
	btVector3 m_upVector;

	void step(bool move, Vector3 direction, bool wantsToJump, float timeSinceLastFrame);
	bool canJump(btVector3 velocity) const;
	void jump(float dt, btVector3 velocity);
	bool onGround() const;
	
	/// Weapon in hand
	ComponentWeapon* actWeapon;
	Entity* weaponEntity;
	TagPoint* weaponTagPoint;
	
	/// Wields the specified weapon (which should be from the inventory of the character)
	void selectWeapon(Object* weapon);
	/// Returns if the weapon is ready to fire
	bool isWeaponReady();
	/// Fires the selected weapon. Mode 0 equals left mouse button, mode 1 equals right mouse button
	void fireWeapon(int mode);
	/// Update weapon firing process.
	void updateWeapon(float dt);

	// Component stuff
	virtual void exit();
	virtual bool handleMessage(int msgID, void* data, void* data2 = NULL);
	virtual bool handleGetMessage(int msgID, void* data, void* data2 = NULL) const;
};

class ComponentTemplateCharacterController : public ComponentTemplate
{
public:
	float height;
	float moveSpeed;
	float gravityFactor;
	float jumpFactor;
	float airSteeringFactor;

	virtual NamedComponent* createInstance(Object* object);
	virtual bool init(TiXmlElement* params);
	virtual void exit();
};

#endif
