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
#include "componentTrigger.h"
#include "physicsConstants.h"

void ComponentTrigger::exit()
{
	if (script)
		delete script;
	delete this;
}
bool ComponentTrigger::finishedAction()
{
	// Watch out for the end of the script
	if ((creator->type == SCRIPTTRIGGER_TIMER && actTime >= creator->time && !script) || (scriptRunning && !script->isRunning()))
	{
		if (mode == ACTIONMODE_DELETECOMPONENT)
		{
			object->removeComponent(this);
			exit();
			return false;
		}
		else if (mode == ACTIONMODE_DELETEOBJECT)
		{
			objectManager.deleteObject(object);
			return false;
		}
		else	// if (mode == ACTIONMODE_REPEAT)
		{
			scriptRunning = false;
			actTime -= creator->time;
		}
	}

	return true;
}
bool ComponentTrigger::handleMessage(int msgID, void* data, void* data2)
{
	Vector3* vecData;
	
	switch (msgID)
	{
	case COMPMSG_UPDATE:
		actTime += LOGIC_TIME_STEP;
		
		// Update script
		if (script && script->isRunning())
			script->update(LOGIC_TIME_STEP);
		
		// Watch out for the end of the script
		if (!finishedAction())
			return false;
		
		// If timer, start script if ready
		if (creator->type == SCRIPTTRIGGER_TIMER && actTime >= creator->time)
		{
			// Execute script
			if (!scriptRunning && !script->isRunning())
			{
				scriptRunning = true;
				script->run();
			}
		}
	break;
	case COMPMSG_COLLIDED:
		// Execute script
		if (creator->type == SCRIPTTRIGGER_COLLISION && script && !script->isRunning())
		{
			scriptRunning = true;
			script->run((Object*)data2);

			if (!finishedAction())
				return false;
		}
	break;
	case COMPMSG_SET_POSITION:
		vecData = (Vector3*)(data);
		object->position = *vecData;
	break;
	
	// TODO: IMPLEMENT type SCRIPTTRIGGER_ACTIVATION!
	}

	return true;
}

bool
ComponentTrigger::
handleGetMessage(int msgID, void* data, void* data2) const
{
  switch(msgID) {
  }
  return true;
}

NamedComponent* ComponentTemplateTrigger::createInstance(Object* object)
{
	ComponentTrigger* newTrigger = new ComponentTrigger();

	newTrigger->creator = this;
	newTrigger->actTime = 0;
	newTrigger->scriptRunning = false;
	newTrigger->mode = mode;

	if (script)
		newTrigger->script = new ObjectScriptExecutor(script, object);
	else
		newTrigger->script = NULL;

	return newTrigger;
}
bool ComponentTemplateTrigger::init(TiXmlElement* params)
{
	TiXmlElement* val;
	const char* cTemp;
	
	// Type
	val = params->FirstChildElement("Type");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no type parameter for a trigger component!", LML_CRITICAL);
		return false;
	}
	else
	{
		cTemp = val->Attribute("value");
		if (cTemp == string("Timer"))
			type = SCRIPTTRIGGER_TIMER;
		else if (cTemp == string("Collision"))
			type = SCRIPTTRIGGER_COLLISION;
		else if (cTemp == string("Activation"))
			type = SCRIPTTRIGGER_ACTIVATION;
		else
		{
			LogManager::getSingleton().logMessage(("ERROR: wrong type parameter for a trigger component: " + string(cTemp) + "!").c_str(), LML_CRITICAL);
			return false;
		}
	}

	if (type == SCRIPTTRIGGER_TIMER)
	{
		// Time
		val = params->FirstChildElement("Time");
		if (!val)
		{
			LogManager::getSingleton().logMessage("ERROR: no time parameter for a trigger component of type Timer!", LML_CRITICAL);
			return false;
		}
		else
			time = StringConverter::parseReal(val->Attribute("value"));
	}

	// Script
	val = params->FirstChildElement("Script");
	if (!val)
		script = NULL;
	else
		script = new ObjectScript(val);

	// Mode
	mode = ACTIONMODE_REPEAT;	// set default
	val = params->FirstChildElement("Mode");
	if (val)
	{
		string strVal = val->Attribute("value");
		if (strVal == "repeat")
			mode = ACTIONMODE_REPEAT;
		else if (strVal == "deleteComponent")
			mode = ACTIONMODE_DELETECOMPONENT;
		else if (strVal == "deleteObject")
			mode = ACTIONMODE_DELETEOBJECT;
		else
		{
			LogManager::getSingleton().logMessage("ERROR: wrong value for a mode parameter in a trigger component: \"" + strVal + "\"!", LML_CRITICAL);
			return false;
		}
	}

	// Sanity check
	if (type == SCRIPTTRIGGER_TIMER && mode == ACTIONMODE_REPEAT && !script)
		LogManager::getSingleton().logMessage("WARNING: trigger component without script is in repeat mode! Only the other modes make sense!");
	else if ((type == SCRIPTTRIGGER_ACTIVATION || type == SCRIPTTRIGGER_COLLISION) && !script)
		LogManager::getSingleton().logMessage("WARNING: trigger component of type activation or collision without script!");

	return true;
}
void ComponentTemplateTrigger::exit()
{
	if (script)
		delete script;
	delete this;
}
