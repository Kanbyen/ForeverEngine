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
#include "componentBullet.h"
#include "object.h"
#include "physicsConstants.h"

void ComponentBullet::exit()
{
	delete this;
}
bool ComponentBullet::handleMessage(int msgID, void* data, void* data2)
{
	Vector3 tempVec;
	Quaternion rotation;
	btQuaternion btRotation;
	btTransform trans;

	switch (msgID)
	{
	case COMPMSG_UPDATEPOS:
		tempVec = aim * velocity;
		rigidBody->setLinearVelocity(btVector3(tempVec.x, tempVec.y, tempVec.z));
		rigidBody->setAngularVelocity(btVector3(0, 0, 0));
	break;
	case COMPMSG_UPDATE:
		if (gravity != 0)
		{
			Vector3 vecVelocity = aim * velocity;
			vecVelocity += LOGIC_TIME_STEP * (Vector3(0, gravity, 0) + aim * acceleration);
			aim = vecVelocity.normalisedCopy();
			velocity = vecVelocity.length();
			handleMessage(COMPMSG_SET_AIM, &aim);
		}
		else
			velocity += LOGIC_TIME_STEP * acceleration;
	break;
	case COMPMSG_SET_AIM:
		aim = *((Vector3*)data);
		rotation = Vector3::NEGATIVE_UNIT_Z.getRotationTo(aim, Vector3::UNIT_Y);
		btRotation = btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w);
		rigidBody->getMotionState()->getWorldTransform(trans);
		trans.setRotation(btRotation);
		rigidBody->setCenterOfMassTransform(trans);
	break;
	case COMPMSG_UNLOAD:
		// Bullets are not worth storing. Delete the object
		objectManager.deleteObject(object);
		return false;
	break;
	case COMPMSG_CREATE:
		// Get the rigidbody from the physics component (which should be there)
		if (!object->sendGetMessage(COMPMSG_GET_RIGIDBODY, &rigidBody))
			LogManager::getSingleton().logMessage("ERROR: no physics component in an object with Bullet component!");
		
		// Disable rigid body deactivation
		if (data)
			rigidBody->setSleepingThresholds(0.0, 0.0);

		object->tag |= OBJTAG_BULLET;
	break;
	}

	return true;
}


bool
ComponentBullet::
handleGetMessage(int msgID, void* data, void* data2) const
{
  switch(msgID) {
	case COMPMSG_GET_AIM:
		*((Vector3*)data) = aim;
	return false;
  }
  return true;
}

NamedComponent* ComponentTemplateBullet::createInstance(Object* object)
{
	ComponentBullet* newBullet = new ComponentBullet();

	newBullet->aim = Vector3::NEGATIVE_UNIT_Z;
	newBullet->velocity = velocity;
	newBullet->acceleration = acceleration;
	newBullet->gravity = gravity;

	return newBullet;
}
bool ComponentTemplateBullet::init(TiXmlElement* params)
{
	// Velocity
	TiXmlElement* val = params->FirstChildElement("Velocity");
	if (val)
		velocity = StringConverter::parseReal(val->Attribute("value"));
	else
		velocity = 0.0f;
		
	// Acceleration
	val = params->FirstChildElement("Acceleration");
	if (val)
		acceleration = StringConverter::parseReal(val->Attribute("value"));
	else
		acceleration = 0.0f;
	
	// Gravity
	val = params->FirstChildElement("Gravity");
	if (val)
		gravity = StringConverter::parseReal(val->Attribute("value"));
	else
		gravity = 0.0f;

	return true;
}
void ComponentTemplateBullet::exit()
{
	delete this;
}
