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
#include "componentHealth.h"
#include "physicsConstants.h"

void ComponentHealth::exit()
{
	if (script)
		delete script;
	delete this;
}
bool ComponentHealth::checkHealthDepleted()
{
	if (health <= 0)
	{
		// Watch out for the end of the script
		if (!script || !script->isRunning())
		{
			if (mode == HEALTHZEROACTION_DELETECOMPONENT)
			{
				object->removeComponent(this);
				exit();
				return false;
			}
			else if (mode == HEALTHZEROACTION_DELETEOBJECT)
			{
				objectManager.deleteObject(object);
				return false;
			}
		}
	}

	return true;
}
bool ComponentHealth::handleMessage(int msgID, void* data, void* data2)
{
	float oldHealth;

	switch (msgID)
	{
	case COMPMSG_UPDATE:
		if (script && script->isRunning())
			script->update(LOGIC_TIME_STEP);

		if (!checkHealthDepleted())
			return false;

		if (newCollisionStrength + oldCollisionStrength > 90.0f)
		{
			int damageType = (int)DAMAGETYPE_COLLISION;
			float damage = (newCollisionStrength + oldCollisionStrength) * 0.15f;
			if (!handleMessage(COMPMSG_TAKEDAMAGE, &damage, &damageType))
				return false;
			newCollisionStrength = 0;
		}
		oldCollisionStrength = newCollisionStrength;
		newCollisionStrength = 0;
	break;
	case COMPMSG_COLLIDED:
		newCollisionStrength += *((float*)data);
	break;
	case COMPMSG_TAKEDAMAGE:
		if (health <= 0)	// no recovery if dead
			break;

		oldHealth = health;
		health -= *((float*)data) * damageFactors[*((int*)data2)];

		// Execute script
		if (script && (health < oldHealth) && !script->isRunning())
			script->run(oldHealth, health);

		// Execute action when health depleted
		if (!checkHealthDepleted())
			return false;
	break;
	}

	return true;
}

bool
ComponentHealth::
handleGetMessage(int msgID, void* data, void* data2) const
{
  switch(msgID) {
	case COMPMSG_GET_HEALTH:		*((float*)data) = health;		return false;
	case COMPMSG_GET_MAX_HEALTH:	*((float*)data) = maxHealth;	return false;
  }
  return true;
}

NamedComponent* ComponentTemplateHealth::createInstance(Object* object)
{
	ComponentHealth* newHealth = new ComponentHealth();

	newHealth->maxHealth = maxHealth;
	newHealth->health = startHealth;
	for (int i = 0; i < DAMAGETYPE_COUNT; i++)
		newHealth->damageFactors[i] = damageFactors[i];
	newHealth->mode = mode;

	if (script)
		newHealth->script = new ObjectScriptExecutor(script, object);
	else
		newHealth->script = NULL;

	newHealth->newCollisionStrength = 0;
	newHealth->oldCollisionStrength = 0;

	return newHealth;
}
bool ComponentTemplateHealth::init(TiXmlElement* params)
{
	// MaxHealth
	TiXmlElement* val = params->FirstChildElement("MaxHealth");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no MaxHealth parameter for a health component!", LML_CRITICAL);
		return false;
	}
	else
		maxHealth = StringConverter::parseReal(val->Attribute("value"));

	// StartHealth
	val = params->FirstChildElement("StartHealth");
	if (val)
		startHealth = StringConverter::parseReal(val->Attribute("value"));
	else
		startHealth = maxHealth;

	// Factors
	for (int i = 0; i < DAMAGETYPE_COUNT; i++)
		damageFactors[i] = 0;

	val = params->FirstChildElement("Factors");
	if (val)
	{
		const char* strVal = val->Attribute("collision");
		if (strVal)
			damageFactors[DAMAGETYPE_COLLISION] = StringConverter::parseReal(strVal);
		strVal = val->Attribute("heat");
		if (strVal)
			damageFactors[DAMAGETYPE_HEAT] = StringConverter::parseReal(strVal);
		strVal = val->Attribute("frost");
		if (strVal)
			damageFactors[DAMAGETYPE_FROST] = StringConverter::parseReal(strVal);
		strVal = val->Attribute("energy");
		if (strVal)
			damageFactors[DAMAGETYPE_ENERGY] = StringConverter::parseReal(strVal);
	}

	// Script
	val = params->FirstChildElement("DamageTaken");
	if (!val)
		script = NULL;
	else
		script = new ObjectScript(val);

	// Mode
	val = params->FirstChildElement("Mode");
	if (val)
	{
		string strVal = val->Attribute("value");
		if (strVal == "none")
			mode = HEALTHZEROACTION_NONE;
		else if (strVal == "deleteComponent")
			mode = HEALTHZEROACTION_DELETECOMPONENT;
		else if (strVal == "deleteObject")
			mode = HEALTHZEROACTION_DELETEOBJECT;
		else
		{
			LogManager::getSingleton().logMessage("ERROR: wrong value for a mode parameter in a health component: \"" + strVal + "\"!", LML_CRITICAL);
			return false;
		}
	}

	return true;
}
void ComponentTemplateHealth::exit()
{
	if (script)
		delete script;
	delete this;
}
