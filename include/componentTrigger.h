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

#ifndef _COMPONENT_TRIGGER_H_
#define _COMPONENT_TRIGGER_H_

#include "component.h"
#include "objectScript.h"

class ComponentTemplateTrigger;

enum ScriptTriggerType
{
	SCRIPTTRIGGER_COLLISION = 0,	// script is executed when the object collides with something
	SCRIPTTRIGGER_TIMER,			// script is executed based on a timer
	SCRIPTTRIGGER_ACTIVATION,		// script is executed when someone activates the object
};

class ComponentTrigger : public NamedComponent
{
public:

	ObjectScriptExecutor* script;
	ComponentTemplateTrigger* creator;
	
	// Timer variables
	float actTime;
	
	// State variables
	bool scriptRunning;
	ActionMode mode;
	
	bool finishedAction();

	virtual void exit();
	virtual bool handleMessage(int msgID, void* data, void* data2 = NULL);
	virtual bool handleGetMessage(int msgID, void* data, void* data2 = NULL) const;
};

class ComponentTemplateTrigger : public ComponentTemplate
{
public:
	int type;	// of enum ScriptTriggerType
	
	ObjectScript* script;

	// Timer variables
	float time;
	
	/// What to do after the script has finished?
	ActionMode mode;

	virtual NamedComponent* createInstance(Object* object);
	virtual bool init(TiXmlElement* params);
	virtual void exit();
};

#endif
