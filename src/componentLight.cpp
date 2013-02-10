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
#include "componentLight.h"
#include "object.h"

int ComponentTemplateLight::lightComponentNumber = 0;

void ComponentLight::exit()
{
	// remove object from scene
	sceneNode->detachAllObjects();
	sceneMgr->destroyLight(light);
	sceneMgr->destroySceneNode(sceneNode->getName());

	delete this;
}

bool ComponentLight::handleMessage(int msgID, void* data, void* data2)
{
	Vector3* vecData;
	
	switch (msgID)
	{
	case COMPMSG_SET_POSITION:
		vecData = (Vector3*)(data);
		object->position = *vecData;
		sceneNode->setPosition(*vecData);
 	break;
	/*case COMPMSG_ATTACH_TO_NODE:
		sceneNode->detachFromParent();
		((Node*)(data))->addChild(sceneNode);
	break;*/
	case COMPMSG_UNLOAD:
		sceneNode->detachAllObjects();
	break;
	case COMPMSG_CREATE:
		// Set the scenenode position accordingly
		if (data)
		{
			sceneNode->setPosition(*(Vector3*)(data));
			object->position = *(Vector3*)(data);
		}

		sceneNode->attachObject(light);
	break;
	}

	return true;
}

bool ComponentLight::handleGetMessage(int msgID, void* data, void* data2) const
{
	switch(msgID)
	{
	case COMPMSG_GET_NODE:
		*((Node**)(data)) = sceneNode;
		return false;
	break;
	case COMPMSG_GET_MOVABLE_OBJECT:
		*((MovableObject**)data) = light;
	break;
	}
	return true;
}


NamedComponent* ComponentTemplateLight::createInstance(Object* object)
{
	ComponentLight* newLight = new ComponentLight();

	newLight->light = sceneMgr->createLight("LightComp (" + StringConverter::toString(lightComponentNumber++) + ")");
	newLight->light->setType(Light::LT_POINT);
	newLight->light->setDiffuseColour(color);
	newLight->light->setAttenuation(radius, 1.0f, 0, (1.0f / 0.08f - 1) / (radius*radius));	// 0.08f: attenuation value at distance <radius> from light
	newLight->sceneNode = sceneMgr->getRootSceneNode()->createChildSceneNode();

	return newLight;
}
bool ComponentTemplateLight::init(TiXmlElement* params)
{
	TiXmlElement* val = params->FirstChildElement("Radius");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no radius parameter for a light component!", LML_CRITICAL);
		return false;
	}
	radius = StringConverter::parseReal(val->Attribute("value"));

	val = params->FirstChildElement("Color");
	if (val)
	{
		string strValue = val->Attribute("value");
		Vector3 tempVec;

		// if it contains spaces, it is a vector
		if (strValue.find(' ') != strValue.npos)
			tempVec = StringConverter::parseVector3(strValue.c_str());
		else
			tempVec = Vector3(StringConverter::parseReal(strValue.c_str()));

		color = ColourValue(tempVec.x, tempVec.y, tempVec.z);
	}
	else
	{
		color = ColourValue(1.0f);
	}

	return true;
}
void ComponentTemplateLight::exit()
{
	delete this;
}
