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

#ifndef _PHYSICS_H_
#define _PHYSICS_H_

#include <list>
#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "btBulletDynamicsCommon.h"
#include "physicsConstants.h"
#include "volume.h"
#include "blockPersistent.h"

/// Standard gravity vector; twice as much as the gravity on earth (-9.81) and the objects still seem a bit slow. Hmmm ...
const btVector3 PHYSICS_GRAVITY = btVector3(0, -20, 0);

/// "Ray result callback" class to find the closest hit object in a ray cast which isn't the specified one
class ClosestNotMe : public btCollisionWorld::ClosestRayResultCallback
{
public:
	ClosestNotMe(btRigidBody* me) : btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
	{
		m_me = me;
	}

	virtual btScalar AddSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		if (rayResult.m_collisionObject == m_me)
			return 1.0;

		return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
	}

	btRigidBody* m_me;
};

class Object;

/// Handles physics engine integration. Pseudo-Singleton, use the global variable "physics".
class Physics
{
friend class ComponentPhysics;
public:

	/// Initializes the physics object
	void init(VoxelVolume* volume);
	/// Deinitializes the physics object
	void exit();
	/// Must be called every frame to update the physics world
	void step(float timeSinceLastFrame);

	/// Adds a dynamic object to the physics world
	void addObject(btRigidBody* body, Object* object, short int collisionGroup = -1);
	/// Removes a dynamic object from the physics world
	void removeObject(btRigidBody* body);

	/// Adds a static object to the physics world
	void addStatic(btRigidBody* body);
	/// Removes a static object from the physics world
	void removeStatic(btRigidBody* body);

	/// Activates all rigid bodies in the box with the given coordinates. Useful for example if the volume was modified inside this box.
	void activateObjectsInBox(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ);

	/// Calls dynamicsWorld->rayTest(...)
	inline void rayTest(const btVector3& rayFromWorld, const btVector3& rayToWorld, btCollisionWorld::RayResultCallback& resultCallback) const
	{
		dynamicsWorld->rayTest(rayFromWorld, rayToWorld, resultCallback);
	}

	/// Gets the Object which owns the specified rigid body or NULL if the body isn't owned by an Object
	inline Object* getObjectFromBody(btRigidBody* body)
	{
		return (Object*)body->getUserPointer();
	}

protected:

	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* broadphase;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

	VoxelVolume* volume;

	std::list<btRigidBody*> objectList;
	typedef std::list<btRigidBody*>::iterator objectListItr;

	/// Bullet's callbacks
	static void internalTickCallback(btDynamicsWorld* world, btScalar timeStep);
	static void internalTickCallbackSendMessages(btRigidBody* co, btRigidBody* co2, btPersistentManifold* contactManifold);
	//static bool contactAddedCallback(btManifoldPoint& cp, const btCollisionObject* colObj0, int partId0, int index0, const btCollisionObject* colObj1, int partId1, int index1);

	/// Creates collision data for the specified block if not already there
	inline void checkShapeCalculation(int x, int y, int z)
	{
		x -= volume->windowXInBlocks;
		y -= volume->windowYInBlocks;
		z -= volume->windowZInBlocks;
		if (!volume->containsBlockRel(x, y, z))
			return;

		VoxelBlockPersistent* block = volume->getPersistentRel(x, y, z);
		if (block->hasSurface())
			block->createCollisionShape();
	}
};

extern Physics physics;

#endif
