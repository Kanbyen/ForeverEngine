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
#include "componentAI.h"
#include "aiHandlerBattleSphere.h"
#include "aiHandlerBot.h"
#include "aiHandlerBat.h"
#include "object.h"
#include "util.h"

void ComponentAI::exit()
{
	if (aiHandlerInitialized)
		aiHandler->exit();
	delete aiHandler;
	delete this;
}
bool ComponentAI::handleMessage(int msgID, void* data, void* data2)
{
	Object* obj;

	switch (msgID)
	{
	case COMPMSG_UPDATEPOS:
		aiHandler->posUpdate(*((float*)data));
	break;
	case COMPMSG_UPDATE:
		aiHandler->logicUpdate();
	break;
	case COMPMSG_AI_PERCEPTION_NOTIFICATION:
		aiHandler->perceptionNotification((AIPerception*)data);
	break;
	case COMPMSG_AI_ATTACKEDBY:
		obj = objectManager.getObjectByID(*((int*)data2));
		if (obj)
			aiHandler->attackedBy(obj, *((float*)data));
	break;
	case COMPMSG_UNLOAD:
		aiHandler->exit();
		aiHandlerInitialized = false;
	break;
	case COMPMSG_CREATE:
		// Get the rigidbody from the physics component (which should be there)
		if (!object->sendGetMessage(COMPMSG_GET_RIGIDBODY, &rigidBody))
			LogManager::getSingleton().logMessage("WARNING: no physics component in an object with AI component!");

		aiHandler->init();
		aiHandlerInitialized = true;

		// Mark this object as agent so it can easily be told apart from uninteresting objects by the vision system
		object->tag |= OBJTAG_AGENT;
	break;
	}

	return true;
}

bool
ComponentAI::
handleGetMessage(int msgID, void* data, void* data2) const
{
  switch(msgID) {
	case COMPMSG_GET_NAME:
		*((string*)data) = creator->displayName;
	return false;
	case COMPMSG_GET_FACTION:
		*((string*)data) = creator->faction;
	return false;
  }
  return true;
}

NamedComponent* ComponentTemplateAI::createInstance(Object* object)
{
	ComponentAI* newAI = new ComponentAI();

	newAI->creator = this;

	if (type == "battle sphere")
		newAI->aiHandler = new AIHandlerBattleSphere();
	else if (type == "bot")
		newAI->aiHandler = new AIHandlerBot();
	else if (type == "bat")
		newAI->aiHandler = new AIHandlerBat();
	else
		throw Exception(0, "Invalid Type parameter in an AI component: " + type + "!", "ComponentTemplateAI::createInstance");
	newAI->aiHandlerInitialized = false;

	newAI->aiHandler->aiComp = newAI;

	return newAI;
}
bool ComponentTemplateAI::init(TiXmlElement* params)
{
	// DisplayName
	TiXmlElement* val = params->FirstChildElement("DisplayName");
	strAttrCpy(displayName, val->Attribute("value"), "");

	// Faction
	val = params->FirstChildElement("Faction");
	strAttrCpy(faction, val->Attribute("value"), "");

	// Type
	val = params->FirstChildElement("Type");
	strAttrCpy(type, val->Attribute("value"), "");

	return true;
}
void ComponentTemplateAI::exit()
{
	delete this;
}
