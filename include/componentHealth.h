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

#ifndef _COMPONENT_HEALTH_H_
#define _COMPONENT_HEALTH_H_

#include "component.h"
#include "objectScript.h"

enum HealthZeroAction
{
	HEALTHZEROACTION_NONE = 0,
	HEALTHZEROACTION_DELETECOMPONENT = 1,
 	HEALTHZEROACTION_DELETEOBJECT = 2,
};

enum DamageType
{
	DAMAGETYPE_COLLISION = 0,
 	DAMAGETYPE_HEAT = 1,
  	DAMAGETYPE_FROST = 2,
   	DAMAGETYPE_ENERGY = 3,

	DAMAGETYPE_COUNT = 4,
};

/// Health Component
/**
	Parameters:<br>
	MaxHealth [required] - the maximum health (parameter: value)<br>
	StartHealth - the health when the component is created<br>
	DamageTaken - script to execute when damaged. WARNING: If this script wait()s, it might not be called every time if it is still running!<br>
	Mode - what should the healthcomp do when the health is depleted. (parameter: value, default: deleteComponent, other values: deleteObject, none)<br>
	Factors - damage factors for the damage types collision, heat, frost and energy
*/
class ComponentHealth : public NamedComponent
{
public:
	float maxHealth;
	float health;
	float oldCollisionStrength, newCollisionStrength;
	float damageFactors[DAMAGETYPE_COUNT];
	ObjectScriptExecutor* script;
	HealthZeroAction mode;

	bool checkHealthDepleted();

	virtual void exit();
	virtual bool handleMessage(int msgID, void* data, void* data2 = NULL);
	virtual bool handleGetMessage(int msgID, void* data, void* data2 = NULL) const;
};

class ComponentTemplateHealth : public ComponentTemplate
{
public:
	float maxHealth;
	float startHealth;
	float damageFactors[DAMAGETYPE_COUNT];
	ObjectScript* script;
	HealthZeroAction mode;

	virtual NamedComponent* createInstance(Object* object);
	virtual bool init(TiXmlElement* params);
	virtual void exit();
};

#endif
