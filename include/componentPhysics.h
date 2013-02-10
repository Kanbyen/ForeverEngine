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

#ifndef _COMPONENT_PHYSICS_H_
#define _COMPONENT_PHYSICS_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include "btBulletDynamicsCommon.h"
using namespace Ogre;
#include "component.h"
#include "physics.h"

class ComponentTemplatePhysics;

/// Physics Component
/**
	Parameters:<br>
	Shape [required] - Collision shape (parameter: value), one of the following: BoundingSphere, BoundingBox, Sphere, Box, Cylinder, MultiSphere<br>
	Mass [required] - the mass of the collision shape (parameter: value)<br>
	Radius - the radius of the shape (parameters: value, factor)
 */
class ComponentPhysics : public NamedComponent
{
public:
	btRigidBody* body;
	ComponentTemplatePhysics* creator;

	virtual void exit();
	virtual bool handleMessage(int msgID, void* data, void* data2 = NULL);
	virtual bool handleGetMessage(int msgID, void* data, void* data2 = NULL) const;
};

enum CollisionShape
{
	COLSHAPE_SPHERE = 0,
 	COLSHAPE_BOX,
  	COLSHAPE_BOUNDINGSPHERE,
   	COLSHAPE_BOUNDINGBOX,
	COLSHAPE_CYLINDER,
	COLSHAPE_MULTISPHERE,
};

class ComponentTemplatePhysics : public ComponentTemplate
{
public:
	Vector3 radius;
	Vector3 radiusFactor;
	btVector3 gravity;
	float mass;
	CollisionShape shape;
	btCollisionShape* btShape;
	short int collisionGroup;

	/// For Multi-Sphere shape
	std::vector< btVector3 > spherePositions;
	std::vector< float > sphereRadii;

	virtual NamedComponent* createInstance(Object* object);
	virtual bool init(TiXmlElement* params);
	virtual void exit();
};

#endif
