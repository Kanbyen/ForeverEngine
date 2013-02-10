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

#ifndef _COMPONENT_DAMAGE_EFFECTS_H_
#define _COMPONENT_DAMAGE_EFFECTS_H_

#include "component.h"
#include "componentHealth.h"

class ComponentTemplateDamageEffects;

/// DamageEffects Component
/**
	Parameters:<br>
	Collision [default: sparkle small yellow] (parameter: name) - the name of the effect to produce when damaged by the "collision" damage type; same for the following damage types:<br>
	Heat [default: none]<br>
	Frost [default: none]<br>
	Energy [default: none]
 */
class ComponentDamageEffects : public NamedComponent
{
public:
	ComponentTemplateDamageEffects* creator;

	virtual void exit();
	virtual bool handleMessage(int msgID, void* data, void* data2);
	virtual bool handleGetMessage(int msgID, void* data, void* data2 = NULL) const;
};

class ComponentTemplateDamageEffects : public ComponentTemplate
{
public:
	string damageEffects[DAMAGETYPE_COUNT];

	virtual NamedComponent* createInstance(Object* object);
	virtual bool init(TiXmlElement* params);
	virtual void exit();
};

#endif
