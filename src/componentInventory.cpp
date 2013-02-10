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
#include "componentInventory.h"
#include "object.h"

void ComponentInventory::exit()
{
	// Delete all object which are in the inventory
	for (int i = 0; i < NUM_ITEMTYPES; ++i)
	{
		for (ObjectsItr itr = objects[i].begin(); itr != objects[i].end(); ++itr)
		{
			objectManager.deleteObject((*itr)->object);
		}
	}
	
	delete this;
}
bool ComponentInventory::handleMessage(int msgID, void* data, void* data2)
{
	switch (msgID)
	{
	case COMPMSG_CREATE:
		if (data)
		{
			// Create the default items and add them to the new inventory
			for (unsigned int i = 0; i < creator->defaultItems.size(); ++i)
			{
				Object* obj = objectManager.createInactiveObject(creator->defaultItems[i].name.c_str(), INVALID_OBJECT_ID);
				obj->id = INVALID_OBJECT_ID;
				
				ComponentItem* compItem;
				if (!obj->sendGetMessage(COMPMSG_GET_ITEM_COMPONENT, &compItem))
					LogManager::getSingleton().logMessage(("ERROR: an inventory component has an object specified as default item which has no item component: " + creator->defaultItems[i].name + "!").c_str(), LML_CRITICAL);
				
				objects[compItem->creator->type].insert(compItem);
			}
		}
	break;
	}

	return true;
}

bool ComponentInventory::handleGetMessage(int msgID, void* data, void* data2) const
{
	switch (msgID)
	{
	case COMPMSG_GET_INVENTORY:
		*((const ComponentInventory**)data) = this;
	return false;
	}
	return true;
}

NamedComponent* ComponentTemplateInventory::createInstance(Object* object)
{
	ComponentInventory* newInventory = new ComponentInventory();

	newInventory->creator = this;

	return newInventory;
}
bool ComponentTemplateInventory::init(TiXmlElement* params)
{
	TiXmlElement* val;
	ItemToBePlacedInInventory item;
	
	// Create list of default items
	val = params->FirstChildElement("Item");
	while (val)
	{
		item.name = val->Attribute("name");
		defaultItems.push_back(item);
	
		val = val->NextSiblingElement("Item");
	}

	return true;
}
void ComponentTemplateInventory::exit()
{
	delete this;
}
