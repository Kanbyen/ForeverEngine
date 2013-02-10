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
#include "componentItem.h"
#include "object.h"
#include "util.h"

const char* itemTypeNames[NUM_ITEMTYPES] = {"Weapons", "Grenades", "Miscellaneous"};

void ComponentItem::exit()
{
	delete this;
}
bool ComponentItem::handleMessage(int msgID, void* data, void* data2)
{
	switch (msgID)
	{
	}

	return true;
}

bool
ComponentItem::
handleGetMessage(int msgID, void* data, void* data2) const
{
  switch(msgID) {
	case COMPMSG_GET_NAME:
		*((string*)data) = creator->displayName;
	return false;
	case COMPMSG_GET_ITEM_COMPONENT:
		*((const ComponentItem**)data) = this;
	return false;
  }
  return true;
}

NamedComponent* ComponentTemplateItem::createInstance(Object* object)
{
	ComponentItem* newItem = new ComponentItem();

	newItem->creator = this;

	return newItem;
}
bool ComponentTemplateItem::init(TiXmlElement* params)
{
	// DisplayName
	TiXmlElement* val = params->FirstChildElement("DisplayName");
	strAttrCpy(displayName, val->Attribute("value"), "");

	// Type
	type = -1;
	val = params->FirstChildElement("Type");
	if (val)
	{
		string str = val->Attribute("value");
		if (str == "weapon")
			type = ITEMTYPE_WEAPON;
		else if (str == "grenade")
			type = ITEMTYPE_GRENADE;
		else if (str == "misc")
			type = ITEMTYPE_MISC;
		else
			LogManager::getSingleton().logMessage(("ERROR: wrong type parameter for a item component: " + str + "!").c_str(), LML_CRITICAL);
	}

	// Holdable
	val = params->FirstChildElement("Holdable");
	if (val)
		holdable = bAttrCpy(val->Attribute("value"), false);
	else
		holdable = false;

	// Takeable
	val = params->FirstChildElement("Takeable");
	if (val)
		takeable = bAttrCpy(val->Attribute("value"), false);
	else
		takeable = false;

	// Weight
	val = params->FirstChildElement("Weight");
	if (val)
		weight = StringConverter::parseReal(val->Attribute("value"));
	else
		weight = 0;
		
	// Importance
	val = params->FirstChildElement("Importance");
	if (val)
		importance = StringConverter::parseReal(val->Attribute("value"));
	else
		importance = 0;

	return true;
}
void ComponentTemplateItem::exit()
{
	delete this;
}
