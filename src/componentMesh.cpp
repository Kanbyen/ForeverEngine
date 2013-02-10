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
#include "componentMesh.h"
#include "object.h"

int ComponentTemplateMesh::meshComponentNumber = 0;

void ComponentMesh::exit()
{
	// remove object from scene
	sceneNode->detachAllObjects();
	sceneMgr->destroyEntity(entity);
	sceneMgr->destroySceneNode(sceneNode->getName());
	object->mesh = NULL;
	delete this;
}

bool ComponentMesh::handleMessage(int msgID, void* data, void* data2)
{
	Vector3* vecData;
	btQuaternion quat;

	switch (msgID)
	{
	case COMPMSG_SET_POSITION:
		vecData = (Vector3*)(data);
		object->position = *vecData;
		sceneNode->setPosition(*vecData);
 	break;
	case COMPMSG_SET_ORIENTATION:
	  quat = *((btQuaternion*)data);
	  sceneNode->setOrientation(quat.w(), quat.x(), quat.y(), quat.z());
	break;
	case COMPMSG_SET_SCALE:
		vecData = (Vector3*)(data);
		sceneNode->setScale(*vecData);
	break;
	/*case COMPMSG_ATTACH_TO_NODE:
		if (!hidden)
		{
			sceneNode->detachFromParent();
			((Node*)(data))->addChild(sceneNode);
		}
	break;*/
	case COMPMSG_UNLOAD:
		sceneNode->detachAllObjects();
		hidden = true;
	break;
	case COMPMSG_CREATE:
		hidden = false;	

		// Set the scenenode position accordingly
		if (data)
		{
			sceneNode->setPosition(*(Vector3*)(data));
			object->position = *(Vector3*)(data);
		}

		if (!hidden)
			sceneNode->attachObject(entity);
	break;
	}

	return true;
}

bool ComponentMesh::handleGetMessage(int msgID, void* data, void* data2) const
{
	switch(msgID)
	{
	case COMPMSG_GET_NODE:
		if (!hidden)
		{
			*((Node**)(data)) = sceneNode;
			return false;
		}
	break;
	case COMPMSG_GET_MOVABLE_OBJECT:
		*((MovableObject**)data) = entity;
		return false;
	break;
	case COMPMSG_GET_SCALE:
		*((Vector3*)data) = sceneNode->getScale();
		return false;
	break;
	case COMPMSG_GET_ORIENTATION:
		*((btQuaternion*)data) = btQuaternion(sceneNode->getOrientation().x, sceneNode->getOrientation().y, sceneNode->getOrientation().z, sceneNode->getOrientation().w);
		return false;
	break;
	}
	return true;
}


NamedComponent* ComponentTemplateMesh::createInstance(Object* object)
{
	ComponentMesh* newMesh = new ComponentMesh();

	newMesh->hidden = true;
	newMesh->entity = sceneMgr->createEntity("MeshComp (" + StringConverter::toString(meshComponentNumber++) + ")", file.c_str());
	newMesh->entity->setRenderQueueGroup(RENDER_QUEUE_WORLD_GEOMETRY_2);
	newMesh->sceneNode = sceneMgr->getRootSceneNode()->createChildSceneNode();

	newMesh->sceneNode->setScale(scale);

	if (material.size())
		newMesh->entity->setMaterialName(material);

	// Allow easy access to this component
	object->mesh = newMesh;

	return newMesh;
}
bool ComponentTemplateMesh::init(TiXmlElement* params)
{
	TiXmlElement* val = params->FirstChildElement("File");
	if (!val)
	{
		LogManager::getSingleton().logMessage("ERROR: no file parameter for a mesh component!", LML_CRITICAL);
		return false;
	}
	file = val->Attribute("value");

	val = params->FirstChildElement("Scale");
	if (val)
	{
		string strValue = val->Attribute("value");

		// if it contains spaces, it is a vector
		if (strValue.find(' ') != strValue.npos)
			scale = StringConverter::parseVector3(strValue.c_str());
		else
			scale = Vector3(StringConverter::parseReal(strValue.c_str()));
	}
	else
	{
		scale = Vector3(1.0f);
	}

	val = params->FirstChildElement("Material");
	if (val)
		material = val->Attribute("value");
	else
		material.clear();

	return true;
}
void ComponentTemplateMesh::exit()
{
	delete this;
}
