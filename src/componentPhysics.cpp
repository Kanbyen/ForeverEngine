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
#include "componentPhysics.h"
#include "componentMesh.h"
#include "object.h"
#include "gameDataStorage.h"

void ComponentPhysics::exit()
{
	delete body->getMotionState();
	physics.removeObject(body);
	delete body;
	delete this;
}
bool ComponentPhysics::handleMessage(int msgID, void* data, void* data2)
{
	btTransform transform;
	Vector3* vecData;
	Vector3* vecData2;
	btQuaternion quat;
	Vector3 position;
	btVector3 tempbtVector;

	switch (msgID)
	{
	case COMPMSG_UPDATEPOS:
		body->getMotionState()->getWorldTransform(transform);
		position = Vector3(transform.getOrigin().getX(), transform.getOrigin().getY(), transform.getOrigin().getZ());
		quat = body->getOrientation();

		// Set the object position and rotation
		object->sendMessage(COMPMSG_SET_POSITION, &position, NULL, this);
		object->sendMessage(COMPMSG_SET_ORIENTATION, &quat, NULL, this);
		
		/*if(object->mesh) {
			quat = body->getOrientation();
			object->mesh->sceneNode->setOrientation(quat.w(), quat.x(), quat.y(), quat.z());
			object->mesh->sceneNode->setPosition(position);
		}*/
		object->position = position;
	break;
	case COMPMSG_UPDATE:
	{
		body->getMotionState()->getWorldTransform(transform);
		tempbtVector = transform.getOrigin();
		tempbtVector /= physics.volume->scale;	// NOTE: This step is important! Don't copy the 3 lines below without it!
		int x = tempbtVector.x() / BLOCK_SIDE_LENGTH; if (tempbtVector.x() < 0) x--;
		int y = tempbtVector.y() / BLOCK_SIDE_LENGTH; if (tempbtVector.y() < 0) y--;
		int z = tempbtVector.z() / BLOCK_SIDE_LENGTH; if (tempbtVector.z() < 0) z--;
		if (!physics.volume->isValidBlockAbs(x, y, z))
		{
			// The object this component belongs to is out of the world. Store or delete the object.
			gameDataStorage.storeObject(object, x, y, z);
			return false;
		}
	}
	break;
	case COMPMSG_SET_POSITION:
		vecData = (Vector3*)(data);
		object->position = *vecData;
		body->getWorldTransform().setOrigin(btVector3(vecData->x, vecData->y, vecData->z));
 	break;
	case COMPMSG_SET_VELOCITY:
		vecData = (Vector3*)(data);
		body->setLinearVelocity(btVector3(vecData->x, vecData->y, vecData->z));
 	break;
	case COMPMSG_SET_ORIENTATION:
	  quat = *((btQuaternion*)data);
	  body->getWorldTransform().setRotation(quat);
	  body->setAngularVelocity(btVector3(0,0,0));
	break;
	case COMPMSG_APPLYCENTRALIMPULSE:
		vecData = (Vector3*)data;
		body->activate();
		body->applyCentralImpulse(btVector3(vecData->x, vecData->y, vecData->z));
	break;
	case COMPMSG_APPLYIMPULSE:
		vecData = (Vector3*)data;
		vecData2 = (Vector3*)data2;
		body->activate();
		body->applyImpulse(btVector3(vecData->x, vecData->y, vecData->z), btVector3(vecData2->x, vecData2->y, vecData2->z));
	break;
	case COMPMSG_UNLOAD:
		physics.removeObject(body);
	break;
	case COMPMSG_CREATE:
	{
		// Place the object in the world
		physics.addObject(body, object, creator->collisionGroup);
		body->setGravity(creator->gravity);

		// Set the position accordingly
		if (data)
		{
			// this is a newly created object, set the transform for the first time
			Vector3* position = (Vector3*)(data);
			object->position = *position;
			transform.setIdentity();
			transform.setOrigin(btVector3(position->x, position->y, position->z));
			body->getMotionState()->setWorldTransform(transform);
			body->setCenterOfMassTransform(transform);
		}

	}
	break;
	}

	return true;
}

bool
ComponentPhysics::
handleGetMessage(int msgID, void* data, void* data2) const
{
  btVector3 tempbtVector;
  switch(msgID) {
		case COMPMSG_GET_VELOCITY:
			tempbtVector = body->getLinearVelocity();
			*((Vector3*)data) = Vector3(tempbtVector.x(), tempbtVector.y(), tempbtVector.z());
		return false;
		case COMPMSG_GET_ORIENTATION:
			*((btQuaternion*)data) = body->getOrientation();
		return false;
		case COMPMSG_GET_RIGIDBODY:
			*((btRigidBody**)data) = body;
		return false;
		case COMPMSG_GET_PHYSICS_COMPONENT:
			*((const ComponentPhysics**)data) = this;
		return false;
  }
  return true;
}

NamedComponent* ComponentTemplatePhysics::createInstance(Object* object)
{
	AxisAlignedBox aabox;
	btTransform startTransform;
	startTransform.setIdentity();

	ComponentPhysics* newPhysics = new ComponentPhysics();

	// TODO: this will crash if object has no mesh -> fix
	// Create the collision shape if not already done
	if (!btShape)
	{
		switch (shape)
		{
		case COLSHAPE_BOUNDINGBOX:
			aabox = object->mesh->entity->getBoundingBox();
			radius = (aabox.getMaximum() - aabox.getMinimum()) / 2;
			radius *= object->mesh->sceneNode->getScale();
		case COLSHAPE_BOX:
			radius *= radiusFactor;
			btShape = new btBoxShape(btVector3(radius.x, radius.y, radius.z));
		break;

		case COLSHAPE_BOUNDINGSPHERE:
			radius.x = object->mesh->entity->getBoundingRadius();
			radius *= object->mesh->sceneNode->getScale();
		case COLSHAPE_SPHERE:
			radius.x *= radiusFactor.x;
			btShape = new btSphereShape(btScalar(radius.x));
		break;

		case COLSHAPE_CYLINDER:
			if (object->mesh)
				radius *= object->mesh->sceneNode->getScale();
			radius *= radiusFactor;
			btShape = new btCylinderShape(btVector3(radius.x, radius.y, radius.z));
		break;

		case COLSHAPE_MULTISPHERE:
			btShape = new btMultiSphereShape(/*btVector3(radius.x, radius.y, radius.z), Parameter inertiaHalfExtents was removed from bullet*/ &spherePositions[0], &sphereRadii[0], spherePositions.size());
		break;
		}
	}

	// Rigidbody is dynamic if and only if mass is non zero, otherwise static
	btScalar btMass(mass);
	bool isDynamic = (btMass != 0.f);
	btVector3 localInertia(0,0,0);
	if (isDynamic)
		btShape->calculateLocalInertia(mass, localInertia);

	// Using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(btMass, myMotionState, btShape, localInertia);
	newPhysics->body = new btRigidBody(rbInfo);

	newPhysics->body->setFriction(1.5);	// TODO: Make adjustable in object files

	newPhysics->creator = this;

	return newPhysics;
}
bool ComponentTemplatePhysics::init(TiXmlElement* params)
{
	// Shape
	TiXmlElement* val = params->FirstChildElement("Shape");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no shape parameter for a physics component!", LML_CRITICAL);
		return false;
	}
	btShape = NULL;
	string strShape = val->Attribute("value");
	if (strShape == "BoundingSphere")
		shape = COLSHAPE_BOUNDINGSPHERE;
	else if (strShape == "BoundingBox")
		shape = COLSHAPE_BOUNDINGBOX;
	else if (strShape == "Sphere")
		shape = COLSHAPE_SPHERE;
	else if (strShape == "Box")
		shape = COLSHAPE_BOX;
	else if (strShape == "Cylinder")
		shape = COLSHAPE_CYLINDER;
	else if (strShape == "MultiSphere")
	{
		shape = COLSHAPE_MULTISPHERE;

		const char* str = val->Attribute("inertiaHalfExtents");
		if (!str)
		{
			LogManager::getSingleton().logMessage("ERROR: MultiSphere-shape in physics component without inertiaHalfExtents attribute!", LML_CRITICAL);
			return false;
		}
		radius = StringConverter::parseVector3(str);

		val = val->FirstChildElement();
		if (!val)
		{
			LogManager::getSingleton().logMessage("ERROR: MultiSphere-shape in physics component with no sphere specified!", LML_CRITICAL);
			return false;
		}

		while (val)
		{
			str = val->Attribute("position");
			if (!str)
			{
				LogManager::getSingleton().logMessage("ERROR: No 'position' attribute for a spere in a multiSphere-shape in s physics component!", LML_CRITICAL);
				return false;
			}
			Vector3 tempVector = StringConverter::parseVector3(str);
			btVector3 position = btVector3(tempVector.x, tempVector.y, tempVector.z);
			spherePositions.push_back(position);

			str = val->Attribute("radius");
			if (!str)
			{
				LogManager::getSingleton().logMessage("ERROR: No 'radius' attribute for a spere in a multiSphere-shape in s physics component!", LML_CRITICAL);
				return false;
			}
			float sphereRadius = StringConverter::parseReal(str);
			sphereRadii.push_back(sphereRadius);

			val = val->NextSiblingElement();
		}
	}
	else
	{
		LogManager::getSingleton().logMessage("ERROR: shape for physics component not regonized: " + strShape + " !", LML_CRITICAL);
		return false;
	}

	// CollisionGroup
	val = params->FirstChildElement("CollisionGroup");
	if (val)
	{
		string strVal = val->Attribute("value");
		if (strVal == "bullet")
			collisionGroup = btBroadphaseProxy::DebrisFilter;
		else
		{
			LogManager::getSingleton().logMessage("ERROR: CollisionGroup for physics component not regonized: " + strVal + " !", LML_CRITICAL);
			return false;
		}
	}
	else
		collisionGroup = btBroadphaseProxy::DefaultFilter;

	// Mass
	val = params->FirstChildElement("Mass");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no mass parameter for a physics component!", LML_CRITICAL);
		return false;
	}
	else
	{
		mass = StringConverter::parseReal(val->Attribute("value"));
	}

	// Radius factor
	radiusFactor = Vector3(1.0f);
	val = params->FirstChildElement("Radius");
	if (val)
	{
		const char* value = val->Attribute("factor");
		if (value)
		{
			string strValue = value;

			// if it contains spaces, it is a vector
			if (strValue.find(' ') != strValue.npos)
				radiusFactor = StringConverter::parseVector3(value);
			else
				radiusFactor = Vector3(StringConverter::parseReal(value));
		}

		value = val->Attribute("value");
		if (value)
		{
			string strValue = value;

			// if it contains spaces, it is a vector
			if (strValue.find(' ') != strValue.npos)
				radius = StringConverter::parseVector3(value);
			else
				radius = Vector3(StringConverter::parseReal(value));
		}
	}

	// Gravity
	gravity = PHYSICS_GRAVITY;
	val = params->FirstChildElement("Gravity");
	if (val)
	{
		Vector3 tempVec3 = StringConverter::parseVector3(val->Attribute("value"));
		gravity = btVector3(tempVec3.x, tempVec3.y, tempVec3.z);
	}

	return true;
}
void ComponentTemplatePhysics::exit()
{
	if (btShape)
		delete btShape;
	delete this;
}
