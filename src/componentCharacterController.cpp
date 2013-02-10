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
#include "OgreTagPoint.h"
#include "componentCharacterController.h"
#include "physics.h"
#include "object.h"
#include "componentPhysics.h"
#include "componentMesh.h"
#include "componentWeapon.h"
#include "objectScript.h"

const char* animStringRunForward = "RunForward";
const char* animStringRunBackward = "RunBackward";
const char* animStringRunRight = "RunRight";
const char* animStringRunLeft = "RunLeft";
const char* animStringStand = "Stand";

const float weaponAttachScale = 1.5f;	// TODO!

void ComponentCharacterController::step(bool move, Vector3 direction, bool wantsToJump, float timeSinceLastFrame)
{
	time += timeSinceLastFrame;

	// Do the raytest to determine if the player is standing or not
	btTransform xform;
	m_rigidBody->getMotionState()->getWorldTransform(xform);
	btVector3 down = -xform.getBasis()[1];
	down.normalize();

	m_upVector = xform.getBasis()[1];
	m_upVector.normalize();

	m_raySource = xform.getOrigin();

	m_rayTarget = m_raySource + down * m_halfHeight * btScalar(2.0f);

	ClosestNotMe rayCallback(m_rigidBody);
	rayCallback.m_closestHitFraction = 1.0;
	physics.rayTest(m_raySource, m_rayTarget, rayCallback);
	if (rayCallback.hasHit())
		m_rayLambda = rayCallback.m_closestHitFraction;
	else
		m_rayLambda = 1.0;

	// Do the actual movement
	m_lastJumpTime += LOGIC_TIME_STEP;

	// Apply Gravity. Don't do this in bullet because some control over the body is lost if this is done:
	// even if the velocity is set to zero, the body slides downwards slowly!
	m_rigidBody->applyCentralImpulse(PHYSICS_GRAVITY * creator->gravityFactor * 0.8f * (btScalar(1.0) / m_rigidBody->getInvMass()) * timeSinceLastFrame);

	// Get transform and velocity
	m_rigidBody->getMotionState()->getWorldTransform(xform);
	btVector3 linearVelocity = m_rigidBody->getLinearVelocity();

	//CEGUI::Window* wnd = CEGUI::WindowManager::getSingleton().getWindow("Debug1"); // If you want to debug ...

	btVector3 velocity = linearVelocity;
	if (!move && onGround() && (linearVelocity.y() <= 3.0f))
	{
		// Stop when on the ground and not being moved by the player
		// TODO: Fall damage if the velocity was high before!
		velocity = btVector3(0, 0, 0);

		//wnd->setText("Stable on ground");
	}
	else if (!move && onGround())
	{
		velocity = linearVelocity;

		//wnd->setText("Instable on ground");
	}
	else	// if walking ...
	{
		// calculate walk direction
		direction.y = 0;
		btVector3 walkDirection = btVector3(direction.x, direction.y, direction.z);
		walkDirection.normalize();
		btScalar walkSpeed = m_walkVelocity;

		if (onGround())
		{
			velocity = walkDirection * walkSpeed;	// TODO: add some small gravity so that the body stays on the ground?

			// Don't hold the player on the ground when jumping
			if (m_lastJumpTime <= 0.5f)
				velocity.setY(linearVelocity.getY());

			//wnd->setText("Walking on ground");
		}
		else
		{
			btVector3 speedXZ = linearVelocity;
			speedXZ.setY(0);
			if (move)
			{
				speedXZ += creator->airSteeringFactor * walkDirection * walkSpeed * timeSinceLastFrame * 0.6f;
				if (speedXZ.length2() > (walkDirection * walkSpeed).length2())
				{
					speedXZ.normalize();
					speedXZ *= (walkDirection * walkSpeed).length();
				}
			}
			velocity = speedXZ;
			velocity.setY(linearVelocity.y());

			//wnd->setText("Flying");
		}
	}

	m_rigidBody->setLinearVelocity(velocity);

	m_rigidBody->getMotionState()->setWorldTransform(xform);
	//m_rigidBody->setCenterOfMassTransform(xform);			// This seems to introduce framerate dependency!

	if (wantsToJump)
		jump(timeSinceLastFrame, velocity);
	else
	{
		jumped = false;
		jetpackUsedLastFrame = false;
	}

	// Animation
	if (move)
	{
		// Set orientation
		Vector3 horizontalAim = aimDirection;
		horizontalAim.y = 0;
		horizontalAim.normalise();
		Quaternion meshRotation = Vector3::NEGATIVE_UNIT_Z.getRotationTo(horizontalAim, Vector3::UNIT_Y);
		btQuaternion btMeshRotation = btQuaternion(meshRotation.x, meshRotation.y, meshRotation.z, meshRotation.w);
		object->sendMessage(COMPMSG_SET_ORIENTATION, &btMeshRotation, NULL);

		// Determine animation to play
		int newAnim;
		if (!move)
			newAnim = CHARANIM_STAND;
		else
		{
			direction.normalise();

			float dp = direction.dotProduct(aimDirection);
			if (dp >= 0.707106781f)	// 45°
				newAnim = CHARANIM_RUN_FWD;
			else if (dp <= -0.707106781f)
				newAnim = CHARANIM_RUN_BWD;
			else
			{
				Vector3 right = aimDirection.crossProduct(Vector3::UNIT_Y);
				dp = direction.dotProduct(right);
				if (dp >= 0)
					newAnim = CHARANIM_RUN_RIGHT;
				else
					newAnim = CHARANIM_RUN_LEFT;
			}
		}

		// Start / stop animation
		if (newAnim != curAnim)
		{
			curAnim = newAnim;
			animChangeTime = time;
			timeBase = animTime;
		}

		// Play animation; TODO: Only on logic update
		if (object->mesh)
		{
			SkeletonInstance* sk = object->mesh->entity->getSkeleton();
			sk->reset(true);

			Animation* anim = NULL;
			if (curAnim == CHARANIM_RUN_FWD)
			{
				anim = sk->getAnimation(animStringRunForward);
				animTime = timeBase + (time - animChangeTime);
			}
			else if (curAnim == CHARANIM_RUN_RIGHT)
			{
				anim = sk->getAnimation(animStringRunRight);
				animTime = timeBase + (time - animChangeTime);
			}
			else if (curAnim == CHARANIM_RUN_LEFT)
			{
				anim = sk->getAnimation(animStringRunLeft);
				animTime = timeBase + (time - animChangeTime);
			}
			else if (curAnim == CHARANIM_RUN_BWD)
			{
				if (hasAnimRunBackward)
				{
					anim = sk->getAnimation(animStringRunBackward);
					animTime = timeBase + (time - animChangeTime);
				}
				else
				{
					anim = sk->getAnimation(animStringRunForward);
					animTime = timeBase - (time - animChangeTime);
				}
			}

			if (anim)
				anim->apply(sk, animTime);

			float fangle = asinf(aimDirection.y) * 90;
			//float fangle = sinf(time * 1.0f) * 90;
			Degree smallangle = Degree(fangle * 0.3f);
			Degree bigangle = Degree(fangle * 0.7f);

			Bone* b;
			b = sk->getBone("Head");
			b->pitch(smallangle, Ogre::Node::TS_WORLD);
			b = sk->getBone("UpArm.L");
			b->pitch(smallangle * 1.5f, Ogre::Node::TS_WORLD);
			b = sk->getBone("LoArm.L");
			b->pitch(bigangle * 1.5f, Ogre::Node::TS_WORLD);
			b = sk->getBone("UpArm.R");
			b->pitch(smallangle, Ogre::Node::TS_WORLD);
			b = sk->getBone("LoArm.R");
			b->pitch(bigangle, Ogre::Node::TS_WORLD);
		}
	}
}

bool ComponentCharacterController::canJump(btVector3 velocity) const
{
	if (velocity.getY() > 1.0f)
		return false;	// Don't let the player accelerate further if he is already flying upwards
	else
		return m_rayLambda < 0.9f;
}

void ComponentCharacterController::jump(float dt, btVector3 velocity)
{
	if (jetpackUsedLastFrame || (!canJump(velocity) && !jumped && m_lastJumpTime > 0.2f))
	{
		// use jetpack	TODO!
		/*jetpackUsedLastFrame = true;
		btVector3 linearVelocity = m_rigidBody->getLinearVelocity();
		linearVelocity.setY(linearVelocity.getY() + PLAYER_JETPACK_FORCE * dt);
		m_rigidBody->setLinearVelocity(linearVelocity);*/
	}
	else if (canJump(velocity) && !jumped && m_lastJumpTime > 0.2f)
	{
		btTransform xform;
		m_rigidBody->getMotionState()->getWorldTransform(xform);
		btVector3 up = xform.getBasis()[1];
		up.normalize();

		btScalar magnitude = (btScalar(1.0) / m_rigidBody->getInvMass()) * creator->jumpFactor * 0.85f;

		/*btVector3 linearVelocity = m_rigidBody->getLinearVelocity();
		linearVelocity.setY(magnitude);
		m_rigidBody->setLinearVelocity(linearVelocity);*/

		m_rigidBody->applyCentralImpulse(up * magnitude);

		m_lastJumpTime = 0.0f;
		jumped = true;
	}
}
bool ComponentCharacterController::onGround() const
{
	return m_rayLambda < 0.7f;
}

void ComponentCharacterController::selectWeapon(Object* weapon)
{
	actWeapon = (ComponentWeapon*)weapon->getComponent("Weapon");
	assert(actWeapon);
	
	actWeapon->holder = this;

	// Setup display
	if (object->mesh)
	{
		weaponEntity = weapon->mesh->entity;	// TODO: What if the entity is unloaded when the weapon is placed in an inventory?
		Entity* ent = object->mesh->entity;
		
		weaponTagPoint = ent->attachObjectToBone("Item.R", weaponEntity, Quaternion(Degree(-90), Vector3(1, 0, 0)) * Quaternion(Degree(180), Vector3(0, 1, 0)));
		weaponTagPoint->setScale(Vector3(weaponAttachScale));
	}
	else
	{
		// TODO!
	}
}
bool ComponentCharacterController::isWeaponReady()
{
	if (actWeapon)
	{
		if (actWeapon->scriptLMB)
			if (actWeapon->scriptLMB->isRunning())
				return false;
		
		if (actWeapon->scriptRMB)
			if (actWeapon->scriptRMB->isRunning())
				return false;
	}
	
	return true;
}
void ComponentCharacterController::fireWeapon(int mode)
{
	if (!isWeaponReady())
		return;
	
	if (mode == 0)
	{
		if (actWeapon->scriptLMB)
			actWeapon->scriptLMB->run();
	}
	else if (mode == 1)
	{
		if (actWeapon->scriptRMB)
			actWeapon->scriptRMB->run();
	}
}
void ComponentCharacterController::updateWeapon(float dt)
{
	// Execute scripts
	if (actWeapon)
	{
		if (actWeapon->scriptLMB)
			actWeapon->scriptLMB->update(dt);
		if (actWeapon->scriptRMB)
			actWeapon->scriptRMB->update(dt);
	}
}

void ComponentCharacterController::exit()
{
	delete this;
}
bool ComponentCharacterController::handleMessage(int msgID, void* data, void* data2)
{
	btVector3 tempbtVector;
	Object* obj;

	switch (msgID)
	{
	case COMPMSG_UPDATE:
		updateWeapon(LOGIC_TIME_STEP);
	break;
	case COMPMSG_SET_AIM:
		aimDirection = *((Vector3*)data);
		aimDirection.normalise();
	break;
	case COMPMSG_ATTACH_CHILD_TO_WEAPON:
	{
		if (!actWeapon)
			break;
			
		obj = (Object*)data;
		MovableObject* childMovableObject;
		Quaternion childOrientation;
		obj->sendGetMessage(COMPMSG_GET_MOVABLE_OBJECT, &childMovableObject);
		obj->sendGetMessage(COMPMSG_GET_ORIENTATION, &childOrientation);
		
		Quaternion attachRotation = Quaternion(Degree(-90), Vector3(1, 0, 0)) * Quaternion(Degree(180), Vector3(0, 1, 0));
		
		#if (OGRE_VERSION_MAJOR <= 1 && OGRE_VERSION_MINOR <= 6)
			childMovableObject->detatchFromParent();	// "detaTch" is a typo in OGRE!
		#else
			childMovableObject->detachFromParent();
		#endif
		if (object->mesh)
			object->mesh->entity->attachObjectToBone("Item.R", childMovableObject, childOrientation * attachRotation, attachRotation * (obj->position * weaponAttachScale));
		else
			; // TODO!
	break;
	}
	case COMPMSG_CREATE:
	{
		btTransform transform;

		// If the object was created the first time (not reloaded) ...
		if (data)
		{
			// Get our physics component
			if (!object->sendGetMessage(COMPMSG_GET_PHYSICS_COMPONENT, &myPhysics))
				LogManager::getSingleton().logMessage("WARNING: no physics component in an object with character controller component!");
			m_rigidBody = myPhysics->body;

			// Set rigid body options
			m_rigidBody->setSleepingThresholds(0.0, 0.0);
			m_rigidBody->setAngularFactor(0.0);
			m_rigidBody->setGravity(btVector3(0, 0, 0));

			// Set the position
			Vector3* position = (Vector3*)(data);
			object->position = *position;
			transform.setIdentity();
			transform.setOrigin(btVector3(position->x, position->y - ((3.0f * creator->height) / 8.0f), position->z));
			m_rigidBody->getMotionState()->setWorldTransform(transform);
			m_rigidBody->setCenterOfMassTransform(transform);

			// We *must* set the up vector here, otherwise it might be uninitialized the next time the camera is set if this is the player object (if no physics update occurred in-between) -> crash, symptom: AxisAlignedBox::setExtents assert failure
			m_upVector = transform.getBasis()[1];
			m_upVector.normalize();

			// Mesh animation initialization
			if (object->mesh)
			{
				aimDirection = Vector3::ZERO;

				curAnim = CHARANIM_STAND;
				animChangeTime = 0;
				animTime = 60*60*10;
				timeBase = 60*60*10;

				SkeletonInstance* sk = object->mesh->entity->getSkeleton();

				// Set all bones to "manually controlled"
				if (sk)
				{
					for (int b = 0; b < (int)sk->getNumBones(); ++b)
						sk->getBone(b)->setManuallyControlled(true);

					// TODO: move the hasAnim variables into ComponentTemplateCharacterController, initialize them upon creation of first instance
					hasAnimRunForward = sk->hasAnimation(animStringRunForward);
					hasAnimRunBackward = sk->hasAnimation(animStringRunBackward);
					hasAnimRunRight = sk->hasAnimation(animStringRunRight);
					hasAnimRunLeft = sk->hasAnimation(animStringRunLeft);
					hasAnimStand = sk->hasAnimation(animStringStand);
				}
			}
			
			// Weapon in hand initialization
			actWeapon = NULL;
		}
	}
	break;
	}

	return true;
}
bool ComponentCharacterController::handleGetMessage(int msgID, void* data, void* data2) const
{
	btTransform trans;
	btVector3 tempbtVector;
	switch(msgID)
	{
	case COMPMSG_GET_HEAD:
		m_rigidBody->getMotionState()->getWorldTransform(trans);
		//btQuaternion quat = mPlayer->m_rigidBody->getOrientation();

		/*cameraYawNode->setOrientation(0, 0, 1, 0);
		cameraPitchNode->setOrientation(0, 0, 1, 0);
		camera->setOrientation(Quaternion(quat.w(), quat.x(), quat.y(), quat.z()));*/
		tempbtVector = trans.getOrigin();
		tempbtVector += m_upVector * ((2.0f * creator->height) / 8.0f);	// Place camera at 3 / 4 player height (there has to be some space above the head because else the weapon in hand will be able to get into geometry)
		*((Vector3*)data) = Vector3(tempbtVector.getX(), tempbtVector.getY(), tempbtVector.getZ());
	return false;
	case COMPMSG_GET_AIM:
		*((Vector3*)data) = aimDirection;
	return false;
	case COMPMSG_GET_CHARACTER_CONTROLLER:
		*((const ComponentCharacterController**)data) = this;
	return false;
	}
	return true;
}

NamedComponent* ComponentTemplateCharacterController::createInstance(Object* object)
{
	ComponentCharacterController* newController = new ComponentCharacterController();

	newController->creator = this;

	newController->m_halfHeight = height / 2;
	newController->m_walkVelocity = moveSpeed;

	newController->jumped = false;
	newController->jetpackUsedLastFrame = false;
	newController->m_lastJumpTime = 10.0f;
	
	newController->aimDirection = Vector3::NEGATIVE_UNIT_Z;

	newController->time = 0;

	return newController;
}
bool ComponentTemplateCharacterController::init(TiXmlElement* params)
{
	TiXmlElement* val;

	// Height
	val = params->FirstChildElement("Height");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no Height parameter for a character controller component!", LML_CRITICAL);
		return false;
	}
	else
		height = StringConverter::parseReal(val->Attribute("value"));

	// MoveSpeed
	val = params->FirstChildElement("MoveSpeed");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no MoveSpeed parameter for a character controller component!", LML_CRITICAL);
		return false;
	}
	else
		moveSpeed = StringConverter::parseReal(val->Attribute("value"));

	// GravityFactor
	val = params->FirstChildElement("GravityFactor");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no GravityFactor parameter for a character controller component!", LML_CRITICAL);
		return false;
	}
	else
		gravityFactor = StringConverter::parseReal(val->Attribute("value"));

	// JumpFactor
	val = params->FirstChildElement("JumpFactor");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no JumpFactor parameter for a character controller component!", LML_CRITICAL);
		return false;
	}
	else
		jumpFactor = StringConverter::parseReal(val->Attribute("value"));

	// AirSteeringFactor
	val = params->FirstChildElement("AirSteeringFactor");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no AirSteeringFactor parameter for a character controller component!", LML_CRITICAL);
		return false;
	}
	else
		airSteeringFactor = StringConverter::parseReal(val->Attribute("value"));

	return true;
}
void ComponentTemplateCharacterController::exit()
{
	delete this;
}
