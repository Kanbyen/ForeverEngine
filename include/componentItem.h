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

#ifndef _COMPONENT_ITEM_H_
#define _COMPONENT_ITEM_H_

#include "component.h"

class ComponentTemplateItem;

enum ItemType
{
	ITEMTYPE_WEAPON = 0,
	ITEMTYPE_GRENADE,
	ITEMTYPE_MISC,
	
	NUM_ITEMTYPES,
};
extern const char* itemTypeNames[NUM_ITEMTYPES];

/// Item Component
/**
	Parameters:<br>
	DisplayName<br>
	Type (currently available: weapon)<br>
	Holdable [default: false] (if the player can hold it with Q)<br>
	Takeable [default: false] (if the player can take it in his inventory)<br>
	Weight [default: 0] (in the inventory, not related to the mass for the physics engine)<br>
 */
class ComponentItem : public NamedComponent
{
public:
	ComponentTemplateItem* creator;

	virtual void exit();
	virtual bool handleMessage(int msgID, void* data, void* data2 = NULL);
	virtual bool handleGetMessage(int msgID, void* data, void* data2 = NULL) const;
};

class ComponentTemplateItem : public ComponentTemplate
{
public:
	string displayName;
	int type;
	int importance;
	bool holdable;
	bool takeable;
	float weight;

	virtual NamedComponent* createInstance(Object* object);
	virtual bool init(TiXmlElement* params);
	virtual void exit();
};

/// Functor to compare items by importance to allow them to be sorted in a multiset
struct ComponentItemImportanceComparer : public binary_function<ComponentItem*, ComponentItem*, bool> {
	bool operator()(ComponentItem* x, ComponentItem* y) { return x->creator->importance > y->creator->importance; }
};

#endif
