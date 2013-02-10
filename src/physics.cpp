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
#include "physics.h"
#include "stateGame.h"			// for world size and to update the player
#include "componentPlayer.h"	// see above
#include "object.h"				// to call objectManager.update() and for the objectList

Physics physics;

const float PHYSICS_MAXIMUM_VELOCITY = 50.0f;
const float PHYSICS_MAXIMUM_VELOCITY_SQ = PHYSICS_MAXIMUM_VELOCITY * PHYSICS_MAXIMUM_VELOCITY;

void Physics::internalTickCallbackSendMessages(btRigidBody* co, btRigidBody* co2, btPersistentManifold* contactManifold)
{
	if (co->isStaticObject() || co->isKinematicObject())
		return;

	int numContacts = contactManifold->getNumContacts();
	for (int j = 0; j < numContacts; j++)
	{
		btManifoldPoint& pt = contactManifold->getContactPoint(j);

		if (co->getUserPointer())
		{
			// Notify obj that it collided with obj2
			Object* obj = (Object*)co->getUserPointer();
			Object* obj2 = (Object*)co2->getUserPointer();
			float collisionStrength = pt.m_appliedImpulse * co->getInvMass();
			collisionStrength = 0;	// TODO! Find a better way to calculate collision strength!
			obj->sendMessage(COMPMSG_COLLIDED, &collisionStrength, obj2);
		}

		//if (pt.m_appliedImpulse != 0)
		//	game.gameConsole->addMessage(StringConverter::toString(pt.m_appliedImpulse * bdA->getInvMass()));
	}
}
void Physics::internalTickCallback(btDynamicsWorld* world, btScalar timeStep)
{
	// iterate over all btManifoldPoints to get information about collisions
	int numManifolds = physics.dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold = physics.dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);

		// Two rigid bodies collided, notify the objects they belong to
		btRigidBody* obA = static_cast<btRigidBody*>(contactManifold->getBody0());
		btRigidBody* obB = static_cast<btRigidBody*>(contactManifold->getBody1());

		internalTickCallbackSendMessages(obA, obB, contactManifold);
		internalTickCallbackSendMessages(obB, obA, contactManifold);
	}

	// cap the maximum velocity for rigid bodies
	for (objectListItr it = physics.objectList.begin(); it != physics.objectList.end(); it++)
	{
		if ((*it)->getLinearVelocity().length2() > PHYSICS_MAXIMUM_VELOCITY_SQ)
		{
			btVector3 newVelocity = ((*it)->getLinearVelocity() / (*it)->getLinearVelocity().length()) * PHYSICS_MAXIMUM_VELOCITY;
			(*it)->setLinearVelocity(newVelocity);
		}
	}
}

/// Method using contact callbacks - doesn't work properly
/*extern ContactAddedCallback		gContactAddedCallback;
//#include "console.h"
bool Physics::contactAddedCallback(btManifoldPoint& cp, const btCollisionObject* colObj0, int partId0, int index0, const btCollisionObject* colObj1, int partId1, int index1)
{
	//if (cp.m_appliedImpulse != 0)
	//	game.gameConsole->addMessage(StringConverter::toString(cp.m_appliedImpulse));

	// this return value is currently ignored, but to be on the safe side: return false if you don't calculate friction
	return false;
}*/

void Physics::addObject(btRigidBody* body, Object* object, short int collisionGroup)
{
	body->setUserPointer(object);

	//body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);	// needed for contact callbacks
	if (collisionGroup != -1)
		dynamicsWorld->addRigidBody(body, collisionGroup, btBroadphaseProxy::AllFilter);
	else
		dynamicsWorld->addRigidBody(body);
	objectList.push_back(body);
}
void Physics::removeObject(btRigidBody* body)
{
	dynamicsWorld->removeRigidBody(body);
	objectList.remove(body);
}

void Physics::addStatic(btRigidBody* body)
{
	body->setUserPointer(NULL);
	dynamicsWorld->addRigidBody(body);
}
void Physics::removeStatic(btRigidBody* body)
{
	dynamicsWorld->removeRigidBody(body);
}

void Physics::step(float timeSinceLastFrame)
{
	// Check if physics data of a block has to be created
	for (objectListItr it = objectList.begin(); it != objectList.end(); it++)
	{
		if (!(*it)->isActive())
			continue;

		btVector3 min, max, temp;
		(*it)->getAabb(min, max);
		min /= volume->scale;
		max /= volume->scale;

		int firstX = min.x() / BLOCK_SIDE_LENGTH; if (min.x() < 0) firstX--;
		int firstY = min.y() / BLOCK_SIDE_LENGTH; if (min.y() < 0) firstY--;
		int firstZ = min.z() / BLOCK_SIDE_LENGTH; if (min.z() < 0) firstZ--;
		int lastX = max.x() / BLOCK_SIDE_LENGTH; if (max.x() < 0) lastX--;
		int lastY = max.y() / BLOCK_SIDE_LENGTH; if (max.y() < 0) lastY--;
		int lastZ = max.z() / BLOCK_SIDE_LENGTH; if (max.z() < 0) lastZ--;
		for (int x = firstX; x <= lastX; x++)
			for (int y = firstY; y <= lastY; y++)
				for (int z = firstZ; z <= lastZ; z++)
					checkShapeCalculation(x, y, z);
	}

	// Let the physics engine do the simulation (or just interpolation)
	objectManager.updating = true;	// TODO! That's a test!
	dynamicsWorld->stepSimulation(timeSinceLastFrame, 8, 1.0f/60.0f);

	// Let the objects with physics components update their position
	for (objectListItr it = objectList.begin(); it != objectList.end(); it++)
	{
		Object* obj = (Object*)((*it)->getUserPointer());
		if (!obj)
			continue;

		obj->sendMessage(COMPMSG_UPDATEPOS, &timeSinceLastFrame);
	}
	objectManager.updating = false;	// TODO!
}

void Physics::init(VoxelVolume* volume)
{
	this->volume = volume;

	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	broadphase = new btDbvtBroadphase();

	solver = new btSequentialImpulseConstraintSolver();

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	dynamicsWorld->setGravity(PHYSICS_GRAVITY);

	//gContactAddedCallback = contactAddedCallback;
	dynamicsWorld->setInternalTickCallback(internalTickCallback);
}

void Physics::activateObjectsInBox(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ)
{
	AxisAlignedBox box1(firstX, firstY, firstZ, lastX, lastY, lastZ);
	btVector3 min, max;
	for (objectListItr it = objectList.begin(); it != objectList.end(); it++)
	{
		(*it)->getAabb(min, max);
		AxisAlignedBox box2(min.getX(), min.getY(), min.getZ(), max.getX(), max.getY(), max.getZ());

		if (box1.intersects(box2))
			(*it)->activate();
	}
}

void Physics::exit()
{
	int i;

	if (dynamicsWorld->getNumCollisionObjects())
		LogManager::getSingleton().logMessage("WARNING: Exiting physics, but there are still " + StringConverter::toString(dynamicsWorld->getNumCollisionObjects()) + " bodies in the dynamicsWorld!");

	// remove the rigidbodies from the dynamics world and delete them
	for (i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject( obj );
		delete obj;
	}

	objectList.clear();

	delete dynamicsWorld;
	delete solver;
	delete broadphase;
	delete dispatcher;
	delete collisionConfiguration;
}
