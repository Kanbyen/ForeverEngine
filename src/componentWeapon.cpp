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
#include "componentWeapon.h"
#include "componentMesh.h"
#include "object.h"
#include "physicsConstants.h"
#include "util.h"
#include "objectScript.h"

void ComponentWeapon::placeInWorld()
{
	// TODO: obsolete this, just send COMPMSG_CREATE instead
	inWorld = true;
	touchTriggerID = objectManager.addTrigger(object, this, TRIGTYPE_TOUCH, TRIGSUBTYPE_WEAPONPICKUP, WEAPON_PICKUP_DISTANCE);
}
void ComponentWeapon::placeInInvectory()
{
	// TODO: obsolete this, just send COMPMSG_UNLOAD instead
	inWorld = false;
	object->sendMessage(COMPMSG_UNLOAD, NULL, NULL);
}
void ComponentWeapon::exit()
{
	if (inWorld)
		objectManager.deleteTrigger(touchTriggerID);

	if (scriptLMB)
		delete scriptLMB;
	if (scriptRMB)
		delete scriptRMB;

	delete this;
}
bool ComponentWeapon::handleMessage(int msgID, void* data, void* data2)
{
	btTransform transform;

	switch (msgID)
	{
	case COMPMSG_UPDATE:
		if (inWorld && object->mesh)
			objectManager.setTriggerPos(touchTriggerID, object->mesh->sceneNode->getPosition());
	break;
	case COMPMSG_ATTACH_CHILD:
		if (holder)
			holder->handleMessage(COMPMSG_ATTACH_CHILD_TO_WEAPON, data, data2);
	break;
	case COMPMSG_SETTARGETPOSITION:
		if (holder)
			return holder->handleMessage(msgID, data);
	break;
	case COMPMSG_UNLOAD:
		inWorld = false;
		objectManager.deleteTrigger(touchTriggerID);
	break;
	case COMPMSG_CREATE:
		inWorld = true;
		
		// Add trigger, if in world
		if (inWorld)
			placeInWorld();

		// Set trigger position
		if (inWorld && object->mesh)
			objectManager.setTriggerPos(touchTriggerID, object->mesh->sceneNode->getPosition());
	break;
	}

	return true;
}

bool ComponentWeapon::handleGetMessage(int msgID, void* data, void* data2) const
{
	switch(msgID)
	{
	case COMPMSG_GET_HEAD:
	case COMPMSG_GET_AIM:
	case COMPMSG_GET_RIGIDBODY:
		if (holder)
			return holder->handleGetMessage(msgID, data);
		break;
	case COMPMSG_GET_MUZZLE_REL:
		*((Vector3*)(data)) = muzzlePos.position;
	return false;
	case COMPMSG_GET_HOLDER_ID:
		if (holder)
		{
			*((int*)data) = holder->object->id;
			return false;
		}
	break;
	}
	return true;
}


NamedComponent* ComponentTemplateWeapon::createInstance(Object* object)
{
	ComponentWeapon* newWeapon = new ComponentWeapon();

	newWeapon->name = name;
	newWeapon->shortName = shortName;
	newWeapon->description = description;
	newWeapon->importance = importance;
	newWeapon->type = type;
	newWeapon->throwName = throwName;

	newWeapon->holdPos = holdPos;
	newWeapon->muzzlePos = muzzlePos;
	if (scriptLMB)
		newWeapon->scriptLMB = new ObjectScriptExecutor(scriptLMB, object);
	else
		newWeapon->scriptLMB = NULL;
	if (scriptRMB)
		newWeapon->scriptRMB = new ObjectScriptExecutor(scriptRMB, object);
	else
		newWeapon->scriptRMB = NULL;

	newWeapon->holder = NULL;
	newWeapon->inWorld = false;
	newWeapon->creator = this;

	newWeapon->object = object;	// need to do this now because placeInWorld needs the object variable

	return newWeapon;
}
bool ComponentTemplateWeapon::init(TiXmlElement* params)
{
	// Importance
	TiXmlElement* val = params->FirstChildElement("Importance");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no importance parameter for a weapon component!", LML_CRITICAL);
		return false;
	}
	else
	{
		importance = StringConverter::parseReal(val->Attribute("value"));
	}

	// Type
	val = params->FirstChildElement("Type");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no type parameter for a weapon component!", LML_CRITICAL);
		return false;
	}
	else
	{
		string t = val->Attribute("value");
		if (t == "normal")
			type = WEAPONTYPE_NORMAL;
		else if (t == "grenade")
			type = WEAPONTYPE_GRENADE;
		else if (t == "tool")
			type = WEAPONTYPE_TOOL;
		else
		{
			LogManager::getSingleton().logMessage("ERROR: unknown value \"" + t + "\" as the type parameter for a weapon component!", LML_CRITICAL);
			return false;
		}
	}

	// Description
	val = params->FirstChildElement("Description");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no description parameter for a weapon component!", LML_CRITICAL);
		return false;
	}

	shortName = val->Attribute("name");
	name = shortName;
	if (type == WEAPONTYPE_GRENADE)
		name += " grenade";
	description = make_newlines(val->Attribute("value"));

	// HoldPos
	val = params->FirstChildElement("HoldPos");
	if (val)
		holdPos.readFrom(val);

	// MuzzlePos
	val = params->FirstChildElement("MuzzlePos");
	if (val)
		muzzlePos.readFrom(val);

	// Scripts
	val = params->FirstChildElement("LMB");
	if (val)
	{
		scriptLMB = new ObjectScript(val);
		const char* cName = val->Attribute("name");
		if (cName)
			nameLMB = cName;
		else
			nameLMB = "";
	}
	else
		scriptLMB = NULL;

	val = params->FirstChildElement("RMB");
	if (val)
	{
		scriptRMB = new ObjectScript(val);
		const char* cName = val->Attribute("name");
		if (cName)
			nameRMB = cName;
		else
			nameRMB = "";
	}
	else
		scriptRMB = NULL;

	throwName = "";
	val = params->FirstChildElement("Throw");
	if (val)
		throwName = val->Attribute("object");

	return true;
}
void ComponentTemplateWeapon::exit()
{
	if (scriptLMB)
		delete scriptLMB;
	if (scriptRMB)
		delete scriptRMB;
	delete this;
}
