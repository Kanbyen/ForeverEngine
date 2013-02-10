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

#ifndef _COMPONENT_INVENTORY_H_
#define _COMPONENT_INVENTORY_H_

#include <vector>
#include <set>
#include "component.h"
#include "componentItem.h"

class Object;
class ComponentTemplateInventory;

struct ItemToBePlacedInInventory
{
	string name;
	// TODO: percentage chance to be generated
};

class ComponentInventory : public NamedComponent
{
public:

	ComponentTemplateInventory* creator;

	/// Objects in the inventory, grouped by type (see enum ItemType in componentItem.h)
	std::multiset< ComponentItem*, ComponentItemImportanceComparer > objects[NUM_ITEMTYPES];
	typedef std::multiset< ComponentItem*, ComponentItemImportanceComparer >::iterator ObjectsItr;
	
	// Functions to look up objects
	inline ObjectsItr begin(int itemType) {return objects[itemType].begin();}
	inline ObjectsItr end(int itemType) {return objects[itemType].end();}

	virtual void exit();
	virtual bool handleMessage(int msgID, void* data, void* data2 = NULL);
	virtual bool handleGetMessage(int msgID, void* data, void* data2 = NULL) const;
};

class ComponentTemplateInventory : public ComponentTemplate
{
public:

	/// List of items which are in the inventory by default
	std::vector< ItemToBePlacedInInventory > defaultItems;

	virtual NamedComponent* createInstance(Object* object);
	virtual bool init(TiXmlElement* params);
	virtual void exit();
};

#endif
