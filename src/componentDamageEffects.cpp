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
#include "componentDamageEffects.h"
#include "object.h"

void ComponentDamageEffects::exit()
{
	delete this;
}
bool ComponentDamageEffects::handleMessage(int msgID, void* data, void* data2)
{
	switch (msgID)
	{
	case COMPMSG_DAMAGEEFFECT:
		if (!creator->damageEffects[*((int*)data2)].size())
			break;

		Object* obj = objectManager.createObject(creator->damageEffects[*((int*)data2)].c_str(), object->id);
		obj->position = *((Vector3*)data);
		obj->sendMessage(COMPMSG_CREATE, (void*)(data));
	break;
	}

	return true;
}

bool
ComponentDamageEffects::
handleGetMessage(int msgID, void* data, void* data2) const
{
  switch(msgID) {
  }
  return true;
}

NamedComponent* ComponentTemplateDamageEffects::createInstance(Object* object)
{
	ComponentDamageEffects* newDamageEffects = new ComponentDamageEffects();

	newDamageEffects->creator = this;

	return newDamageEffects;
}
bool ComponentTemplateDamageEffects::init(TiXmlElement* params)
{
	// Collision
	TiXmlElement* val = params->FirstChildElement("Collision");
	if (val)
		damageEffects[DAMAGETYPE_COLLISION] = val->Attribute("name");
	else
		damageEffects[DAMAGETYPE_COLLISION] = "sparkle small yellow";

	// Heat
	val = params->FirstChildElement("Heat");
	if (val)
		damageEffects[DAMAGETYPE_HEAT] = val->Attribute("name");
	else
		damageEffects[DAMAGETYPE_HEAT] = "";

	// Frost
	val = params->FirstChildElement("Frost");
	if (val)
		damageEffects[DAMAGETYPE_FROST] = val->Attribute("name");
	else
		damageEffects[DAMAGETYPE_FROST] = "";

	// Energy
	val = params->FirstChildElement("Energy");
	if (val)
		damageEffects[DAMAGETYPE_ENERGY] = val->Attribute("name");
	else
		damageEffects[DAMAGETYPE_ENERGY] = "";

	return true;
}
void ComponentTemplateDamageEffects::exit()
{
	delete this;
}
