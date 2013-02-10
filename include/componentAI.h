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

#ifndef _COMPONENT_AI_H_
#define _COMPONENT_AI_H_

#include "component.h"

class ComponentAI;
class AIPerception;

/// Base class for "AI Handlers" to which the AI component delegates all the work. Important: Derived classes must not use the class constructor / destructor; init() and exit() have to be used. They may be called multiple times in the life of the class.
class AIHandler
{
public:
	ComponentAI* aiComp;

	virtual ~AIHandler() {};

	virtual void perceptionNotification(AIPerception* msg) {}
	virtual void attackedBy(Object* obj, float damage) {}

	virtual void posUpdate(float timeSinceLastFrame) = 0;
	virtual void logicUpdate() = 0;

	virtual void init() = 0;
	virtual void exit() = 0;
};

class ComponentTemplateAI;

/// AI Component
/**
	Parameters:<br>
	Type - one of the AI types (currently only "battle sphere")
 */
class ComponentAI : public NamedComponent
{
public:
	btRigidBody* rigidBody;
	AIHandler* aiHandler;
	bool aiHandlerInitialized;
	ComponentTemplateAI* creator;

	virtual void exit();
	virtual bool handleMessage(int msgID, void* data, void* data2 = NULL);
	virtual bool handleGetMessage(int msgID, void* data, void* data2 = NULL) const;
};

class ComponentTemplateAI : public ComponentTemplate
{
public:
	string displayName;
	string faction;
	string type;

	virtual NamedComponent* createInstance(Object* object);
	virtual bool init(TiXmlElement* params);
	virtual void exit();
};

#endif
