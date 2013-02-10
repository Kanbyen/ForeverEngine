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
#include "componentParticle.h"
#include "object.h"

int ComponentTemplateParticle::particleComponentNumber = 0;

void ComponentParticle::exit()
{
	// remove object from scene
	sceneNode->detachAllObjects();
	sceneMgr->destroyParticleSystem(particle);
	sceneMgr->destroySceneNode(sceneNode->getName());

	delete this;
}
bool ComponentParticle::handleMessage(int msgID, void* data, void* data2)
{
	Vector3 pos;

	switch (msgID)
	{
	case COMPMSG_UPDATE:
		//object->position = sceneNode->_getDerivedPosition();
		sceneNode->setPosition(object->position);
	break;
	case COMPMSG_UNLOAD:
		// Detach the particle system from the scene node
		sceneNode->detachAllObjects();
	break;
	case COMPMSG_CREATE:
		// Set the scenenode position accordingly
		if (data)
		{
			sceneNode->setPosition(*(Vector3*)(data));
			object->position = *(Vector3*)(data);
		}
		sceneNode->attachObject(particle);
	break;
	}

	return true;
}

bool ComponentParticle::handleGetMessage(int msgID, void* data, void* data2) const
{
	switch(msgID)
	{
	case COMPMSG_GET_NODE:
		*((Node**)(data)) = sceneNode;
	return false;
	case COMPMSG_GET_MOVABLE_OBJECT:
		*((MovableObject**)data) = particle;
	break;
	}
	return true;
}

NamedComponent* ComponentTemplateParticle::createInstance(Object* object)
{
	ComponentParticle* newParticle = new ComponentParticle();

	newParticle->sceneNode = sceneMgr->getRootSceneNode()->createChildSceneNode();	// TODO: create it as a child of the mesh component node, if possible and specified
	newParticle->particle = sceneMgr->createParticleSystem("ParticleComp (" + StringConverter::toString(particleComponentNumber++) + ")", name);
	newParticle->particle->setRenderQueueGroup(RENDER_QUEUE_9);

	return newParticle;
}
bool ComponentTemplateParticle::init(TiXmlElement* params)
{
	// Name
	TiXmlElement* val = params->FirstChildElement("Particle");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no particle name parameter for a particle component!", LML_CRITICAL);
		return false;
	}
	else
	{
		name = val->Attribute("value");
	}

	return true;
}
void ComponentTemplateParticle::exit()
{
	delete this;
}
