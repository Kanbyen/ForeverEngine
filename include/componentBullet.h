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

#ifndef _COMPONENT_BULLET_H_
#define _COMPONENT_BULLET_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include "btBulletDynamicsCommon.h"
using namespace Ogre;
#include "component.h"

/// Bullet Component
/**
	Parameters:<br>
	Velocity - no comment needed
 */
class ComponentBullet : public NamedComponent
{
public:
	float velocity;
	float acceleration;
	btRigidBody* rigidBody;
	Vector3 aim;
	float gravity;

	virtual void exit();
	virtual bool handleMessage(int msgID, void* data, void* data2 = NULL);
	virtual bool handleGetMessage(int msgID, void* data, void* data2 = NULL) const;
};

class ComponentTemplateBullet : public ComponentTemplate
{
public:
	float velocity;
	float acceleration;
	float gravity;

	virtual NamedComponent* createInstance(Object* object);
	virtual bool init(TiXmlElement* params);
	virtual void exit();
};

#endif
