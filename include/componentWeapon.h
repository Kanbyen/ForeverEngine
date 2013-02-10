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

#ifndef _COMPONENT_WEAPON_H_
#define _COMPONENT_WEAPON_H_

#include "component.h"
#include "componentPosition.h"

#define WEAPON_PICKUP_DISTANCE 6.2f

// TODO: better place for enum WeaponType?
enum WeaponType
{
	WEAPONTYPE_NORMAL = 0,
 	WEAPONTYPE_GRENADE = 1,
	WEAPONTYPE_TOOL = 2,
};

class ComponentTemplateWeapon;
class ObjectScript;
class ObjectScriptExecutor;

/// Weapon Component
/**
	Parameters:<br>
	Type [required] - in which weapon inventory the weapon will apear (parameter: value, can be normal, grenade or tool)
	Importance [required] - at which position the weapon apperas in the weapon inventory (parameter: value)
 */
class ComponentWeapon : public NamedComponent
{
public:
	string name;
	string shortName;	// for grenades: the name without " grenade" after it
	string description;
	WeaponType type;
	float importance;
	string throwName;	// for grenades: the name of the object to generate when the grenade is thrown

	ComponentPosition holdPos;
	ComponentPosition muzzlePos;
	ObjectScriptExecutor* scriptLMB;
	ObjectScriptExecutor* scriptRMB;

	/// The weapon holder or NULL
	Component* holder;

	ComponentTemplateWeapon* creator;

	/// is the weapon in the world (or in the inventory of an agent)?
	bool inWorld;
	int touchTriggerID;

	void placeInWorld();
	void placeInInvectory();

	virtual void exit();
	virtual bool handleMessage(int msgID, void* data, void* data2 = NULL);
	virtual bool handleGetMessage(int msgID, void* data, void* data2 = NULL) const;
};

/// Functor to compare the weapons by importance to allow them to be sorted in a list
struct componentWeaponImportance : public binary_function<ComponentWeapon*, ComponentWeapon*, bool> {
	bool operator()(ComponentWeapon* x, ComponentWeapon* y) { return x->importance > y->importance; }
};

class ComponentTemplateWeapon : public ComponentTemplate
{
public:
	string name;
	string shortName;
	string description;
	WeaponType type;
	float importance;
	string throwName;

	ComponentPosition holdPos;
	ComponentPosition muzzlePos;
	ObjectScript* scriptLMB;
	ObjectScript* scriptRMB;
	string nameLMB;
	string nameRMB;

	virtual NamedComponent* createInstance(Object* object);
	virtual bool init(TiXmlElement* params);
	virtual void exit();
};

#endif
