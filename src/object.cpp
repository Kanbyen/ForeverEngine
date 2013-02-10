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
#include "object.h"
#include "componentMesh.h"

#include "windowManager.h"
#include "util.h"

ObjectManager objectManager;

Object::Object()
{
	mesh = NULL;
	#ifdef USE_RAKNET
	net = NULL;
	#endif
	listenTo = (TriggerType)0;
	deleted = OBJDELETE_NO;
}
/*Component* Object::getComponent(string name)
{
	// TODO? Object::getComponent(string name)
}*/

void Object::addComponent(Component* component, bool forward)
{
#ifdef USE_RAKNET
  if(forward && this->net) {
    if(component->name() != "Player") {
	    net->objAddComponent(component->name().c_str());
    }
  }
#endif
  components.push_back(component);
}

void Object::removeComponent(Component* component, bool forward)
{
#ifdef USE_RAKNET
  if(forward && this->net) {
    net->objRemoveComponent(component->name().c_str());
  }
#endif
	components.remove(component);
}

bool Object::sendMessage(int msgID, void* data, void* data2, Component* exclude)
{
  assert((msgID > COMPMSG_MSGS_BEGIN && msgID < COMPMSG_MSGS_END)
      ||
      (msgID > COMPMSG_AI_MSGS_BEGIN && msgID < COMPMSG_AI_MSGS_END)
      );
	if (deleted == OBJDELETE_YES)
        return false;

	for (ComponentList::iterator it = components.begin(); it != components.end(); it++)
	{
		if ((*it) != exclude)
			if (!(*it)->handleMessage(msgID, data, data2))
				return false;
	}
	return true;
}

bool Object::sendGetMessage(int msgID, void* data, void* data2, Component* exclude) const
{
  assert(msgID > COMPMSG_GET_MSGS_BEGIN && msgID < COMPMSG_GET_MSGS_END);
	for (std::list<Component*>::const_iterator it = components.begin(); it != components.end(); it++)
	{
		if ((*it) != exclude)
			if (!(*it)->handleGetMessage(msgID, data, data2))
				return true;
	}
	return false;
}

Component* Object::getComponent(string name)
{
	for (std::list<Component*>::iterator it = components.begin(); it != components.end(); it++)
	{
		if ((*it)->name() == name)
			return *it;
	}
	return NULL;
}

void Object::exit()
{
	// delete all components
	for (std::list<Component*>::iterator it = components.begin(); it != components.end(); it++)
		(*it)->exit();
	components.clear();

	delete this;
}

ObjectManager::ObjectManager()
#ifdef USE_RAKNET
:net(NULL)
#endif
{
}

int ObjectManager::addTrigger(Object* source, Component* sourceComp, TriggerType type, TriggerSubType subType, float radius, Vector3 position)
{
	int ID;
	Trigger newTrigger;

	newTrigger.position = position;
	newTrigger.type = type;
	newTrigger.subType = subType;
	newTrigger.source = source;
	newTrigger.sourceComp = sourceComp;
	newTrigger.radiusSquared = radius * radius;
	newTrigger.deleted = false;

	// create a new ID and make sure it is not used
	do
	{
		ID = newTriggerID++;
	} while (triggerMap.find(ID) != triggerMap.end());

	triggerMap.insert(std::map<int, Trigger>::value_type(ID, newTrigger));

	return ID;
}
void ObjectManager::setTriggerPos(int ID, Vector3 position)
{
	std::map<int, Trigger>::iterator it = triggerMap.find(ID);
	if (it != triggerMap.end())
		(*it).second.position = position;
}
void ObjectManager::deleteTrigger(int ID)
{
    std::map<int, Trigger>::iterator it = triggerMap.find(ID);
	if (it != triggerMap.end())
		(*it).second.deleted = true;
}

void ObjectManager::addObjectType(TiXmlElement* elem)
{
	string name = elem->Attribute("name");
	compCreateList createList;

	// Check if the name is already used for another object type
	if (objectComponentMap.find(name) != objectComponentMap.end())
		throw Exception(0, "ERROR: Tried to addObjectType() \"" + name + "\" which is already registered in the ObjectManager!", "ObjectManager::addObjectType");

	elem = elem->FirstChildElement();
	while (elem)
	{
		// Add this component template to the creation info of this object
		ComponentCreateInfo newComponentCreateInfo;
		newComponentCreateInfo.compTemplate = componentTemplateManager.createTemplate(elem->Attribute("name"), elem);
		if (!newComponentCreateInfo.compTemplate)
			throw Exception(0, "Error while parsing the object templates!", "ObjectManager::addObjectType");
		createList.push_back(newComponentCreateInfo);

		elem = elem->NextSiblingElement();
	}

	// Add the object type to the list
	objectComponentMap.insert(std::map<string, compCreateList>::value_type(name, createList));

	// NOTE <FOREVER WAR-SPECIFIC> Add the object type to the listbox in the editor
	CEGUI::Listbox* box = (CEGUI::Listbox*)CEGUI::WindowManager::getSingleton().getWindow("SpawnObjectEdit");
	box->addItem(new MyListItem(name, (void*)0));
	box->setItemSelectState((size_t)0, true);
	// </FOREVER WAR-SPECIFIC>
}
void ObjectManager::readObjectsFromFile(const char* file)
{
	TiXmlDocument* doc = new TiXmlDocument(file);
	if (!doc->LoadFile())
		throw Exception(0, "cannot open " + string(file) + " ! Most likely there are syntax errors in the file.", "ObjectManager::readObjectsFromFile");

	TiXmlHandle docHandle(doc);
	docHandle = docHandle.FirstChild("Objects");

	TiXmlElement* element = docHandle.FirstChild().ToElement();
	while (element)
	{
		addObjectType(element);
		element = element->NextSiblingElement();
	}

	delete doc;
}
void ObjectManager::readObjectsFromArchive(const char* filename, const char* archiveType)
{
	Archive* archive = ArchiveManager::getSingleton().load(filename, archiveType);

	FileInfoListPtr infoList = archive->findFileInfo("*.object", false);

	for (int i = 0; i < (int)infoList->size(); i++)
		readObjectsFromFile((string(filename) + "/" + (*infoList)[i].filename).c_str());

	ArchiveManager::getSingleton().unload(archive);
}
Object* ObjectManager::createInactiveObject(const char* name, int creatorID)
{
	// Find the object name in the map
	std::map<string, compCreateList>::iterator it = objectComponentMap.find(name);
	if (it == objectComponentMap.end())
	{
		LogManager::getSingleton().logMessage("ERROR: tried to create object \"" + string(name) + "\" which doesn't exist!", LML_CRITICAL);
		return NULL;
	}

	// Create the Object
	Object* newObject = new Object();
	newObject->name = string(name);
	newObject->tag = (ObjectTag) 0;
	newObject->creatorID = creatorID;
	newObject->position = Vector3::ZERO;	// make sure we don't have a completely invalid position which lets OGRE crash, altough every default value won't make much sense

	// Find a free object ID
	while ((objectMap.find(newID) != objectMap.end()) || (newID == -1))
		newID++;
	newObject->id = newID;
	newID++;

	// Add all components to the object
	compCreateList& createList = it->second;
	for (compCreateList::iterator cit = createList.begin(); cit != createList.end(); cit++)
	{
#ifdef USE_RAKNET
	  if((*cit).compTemplate->name() == "Network" && (net == NULL || net->me == NULL))
	    continue; // DONT create a network component if not connected to anyone
#endif
		Component* newComponent = (*cit).compTemplate->newInstance(newObject);
		if (!newComponent)
		{
			newObject->exit();
			return NULL;
		}
		newComponent->object = newObject;
		newObject->addComponent(newComponent);
	}

	return newObject;
}
void ObjectManager::addObject(Object* newObject)
{
	objectMap.insert(ObjectMap::value_type(newObject->id, newObject));
	
	if (editObjects)
	{
		// Create anchor
		Entity* entity = sceneMgr->createEntity("ObjectAnchor" + StringConverter::toString(objectAnchorNumber++), "smoothCube.mesh");
		entity->setMaterialName("CelShadingBlue");
		entity->setUserAny(Any((Object*)newObject));
		entity->setRenderQueueGroup(RENDER_QUEUE_WORLD_GEOMETRY_2);
		newObject->anchorNode = sceneMgr->getRootSceneNode()->createChildSceneNode();
		newObject->anchorNode->setPosition(newObject->position);
		newObject->anchorNode->setScale(Vector3(0.3f));
		newObject->anchorNode->attachObject(entity);
	}
}
void ObjectManager::removeObject(Object* oldObject)
{
	if (objectMap.erase(oldObject->id) == 0)
		return;	// object not found in list
	
	if (editObjects)
	{
		if (oldObject == selObject)
			selectObject(NULL);
		
		MovableObject* movable = oldObject->anchorNode->getAttachedObject(0);
		oldObject->anchorNode->detachAllObjects();
		sceneMgr->destroyMovableObject(movable);
		sceneMgr->destroySceneNode(oldObject->anchorNode);
	}
}
Object* ObjectManager::createObject(const char* name, int creatorID)
{
	Object* newObject = createInactiveObject(name, creatorID);
	assert(newObject && "Tried to create a non-existing object!");

	// Add the object to the objectMap
	addObject(newObject);

	//LogManager::getSingleton().logMessage("ObjectManager: created " + string(name) + " ID " + StringConverter::toString(newObject->id));

	return newObject;
}
void ObjectManager::addExistingObjectToList(Object* object)
{
	// Find a new free object ID
	while ((objectMap.find(newID) != objectMap.end()) || (newID == -1))
		newID++;
	object->id = newID;
	newID++;

	// Add the object to the objectMap
	addObject(object);

	//LogManager::getSingleton().logMessage("ObjectManager: added existing object OLD_ID " + StringConverter::toString(oldID) + " ID " + StringConverter::toString(object->id));
}
void ObjectManager::deleteObject(Object* object)
{
    // Delayed deletion if we are updating, else we can delete it instantly
    if (updating)
		object->deleted = OBJDELETE_YES;
    else
    {
        //object->sendMessage(COMPMSG_MANUALDELETE, NULL);
		removeObject(object);
        object->exit();

        //LogManager::getSingleton().logMessage("ObjectManager: deleteObject ID " + StringConverter::toString(objectID));
    }
}
void ObjectManager::removeObjectFromList(Object* object)
{
	if (updating)
		object->deleted = OBJDELETE_JUSTREMOVE;
	else
	{
		removeObject(object);

		//LogManager::getSingleton().logMessage("ObjectManager: removeObjectFromList ID " + StringConverter::toString(objectID));
	}
}
Object* ObjectManager::getObjectByID(int id)
{
	ObjectMapItr it = objectMap.find(id);
	if (it == objectMap.end())
		return NULL;
	else
		return (*it).second;
}
void ObjectManager::setEditObjects(bool edit)
{
	editObjects = edit;
	
	if (edit && !tooltip)
		initTooltip();
	
	// TODO: if enabling, create anchors for existing objects; if disabling, delete anchors
}
void ObjectManager::updateManager()
{
	if (editObjects)
	{
		// Update object anchors
	    ObjectMapItr end = objectMap.end();
		for (ObjectMapItr it = objectMap.begin(); it != end; ++it)
		{
			Object* actUpdateObject = (*it).second;
			
			// Reposition object anchor
			actUpdateObject->anchorNode->setPosition(actUpdateObject->position);
			btQuaternion btQ;
			if (actUpdateObject->sendGetMessage(COMPMSG_GET_ORIENTATION, &btQ))
			{
				Quaternion q(btQ.w(), btQ.x(), btQ.y(), btQ.z());
				actUpdateObject->anchorNode->setOrientation(q);
			}
		}
		
		// Get object below cursor
		Object* objBelowCursor = getObjectBelowCursor();
		
		// Update tooltip
		if (objBelowCursor)
		{
			tooltip->setText(objBelowCursor->name.c_str());
			tooltip->setArea(cegui_reldim(windowManager.getMouseXRel()) + cegui_absdim(16), cegui_reldim(windowManager.getMouseYRel()) - cegui_reldim(0.5f) + cegui_absdim(4), cegui_reldim(1), cegui_reldim(1));
		}
		else
		{
			tooltip->setText("");
			tooltip->setArea(cegui_absdim(-1), cegui_absdim(-1), cegui_absdim(0), cegui_absdim(0));
		}
		
		// Update editing
		if (editModeActive)
			updateEditing(getEditAmount());
	}
}
void ObjectManager::update()
{
    // Perform the update loop
    updating = true;
    ObjectMapItr end = objectMap.end();
	for (ObjectMapItr it = objectMap.begin(); it != end; ++it)
	{
		// Is this object already deleted?
		if ((*it).second->deleted != OBJDELETE_NO)
            continue;

		Object* actUpdateObject = (*it).second;

		// Process triggers
		// Does this object want to listen to any triggers?
		if (actUpdateObject->listenTo)
		{
			// Loop through all triggers
			for (std::map<int, Trigger>::iterator tit = triggerMap.begin(); tit != triggerMap.end(); tit++)
			{
				// Is the source the object itself?
				if ((*tit).second.source == actUpdateObject)
					continue;

				// Does the object listen to this type of trigger?
				if (!((*tit).second.type & actUpdateObject->listenTo))
					continue;

				// Range check
				if (((*tit).second.position - actUpdateObject->position).squaredLength() <= (*tit).second.radiusSquared)
				{
					// Notify the object of the trigger
					actUpdateObject->sendMessage(COMPMSG_HANDLETRIGGER, &(*tit).second);
				}
			}
		}

		// Send the UPDATE message
		actUpdateObject->sendMessage(COMPMSG_UPDATE, NULL);
	}
	updating = false;

	// Perform the object delete loop
	for (ObjectMapItr it = objectMap.begin(); it != objectMap.end();)
	{
	    Object* object = (*it).second;

		if (object->deleted != OBJDELETE_NO)
	    {
        // This object should be deleted!
        ObjectMapItr itNext = it;
        itNext++;

			if (object->deleted == OBJDELETE_YES)
			{
				//object->sendMessage(COMPMSG_MANUALDELETE, NULL);
				removeObject(object);
				object->exit();

				//LogManager::getSingleton().logMessage("ObjectManager: delete loop ID " + StringConverter::toString(objectID));
			}
			else if (object->deleted == OBJDELETE_JUSTREMOVE)
			{
				removeObject(object);

				//LogManager::getSingleton().logMessage("ObjectManager: remove loop ID " + StringConverter::toString(objectID));
			}

            it = itNext;
	    }
	    else
            ++it;
	}

	// Perform the trigger delete loop
    for (std::map<int, Trigger>::iterator tit = triggerMap.begin(); tit != triggerMap.end();)
    {
        if ((*tit).second.deleted)
        {
            std::map<int, Trigger>::iterator titNext = tit;
            titNext++;
            triggerMap.erase(tit);
            tit = titNext;
        }
        else
            ++tit;
    }
}
void ObjectManager::init()
{
	newID = 0;
	newTriggerID = 0;
	updating = false;
	editObjects = false;
	objectAnchorNumber = 0;
	tooltip = NULL;
	selObject = NULL;
	editModeActive = false;
}
void ObjectManager::deleteAllObjects()
{
	for (ObjectMapItr it = objectMap.begin(); it != objectMap.end(); it++)
		(*it).second->exit();
	objectMap.clear();
}
void ObjectManager::exit()
{
	deleteAllObjects();

	// delete all object templates
	for (std::map<string, compCreateList>::iterator it = objectComponentMap.begin(); it != objectComponentMap.end(); it++)
	{
		compCreateList& createList = it->second;
		for (compCreateList::iterator cit = createList.begin(); cit != createList.end(); cit++)
			(*cit).compTemplate->exit();
		(*it).second.clear();
	}
	objectComponentMap.clear();
	
	exitTooltip();
}

void ObjectManager::initTooltip()
{
	sceneQuery = sceneMgr->createRayQuery(Ray());
	sceneQuery->setSortByDistance(true);
	//sceneQuery->setQueryMask(~DONT_INCLUDE_IN_QUERY);
	
	tooltip = new StaticText();
	tooltip->create("", cegui_absdim(-1), cegui_absdim(-1), cegui_absdim(-1), cegui_absdim(-1),
					CEGUI::WindowManager::getSingleton().getWindow("EditorRoot"));	
}
void ObjectManager::exitTooltip()
{
	if (!tooltip)
		return;
	
	sceneMgr->destroyQuery(sceneQuery);
	
	tooltip->destroy();
	delete tooltip;
}

void ObjectManager::selectObjectBelowCursor()
{
	selectObject(getObjectBelowCursor());
}
void ObjectManager::selectObject(Object* newSelection)
{
	if (selObject)
	{
		MovableObject* movable = selObject->anchorNode->getAttachedObject(0);
		Entity* entity = (Entity*)movable;
		entity->setMaterialName("CelShadingBlue");
	}
	selObject = newSelection;
	if (selObject)
	{
		MovableObject* movable = selObject->anchorNode->getAttachedObject(0);
		Entity* entity = (Entity*)movable;
		entity->setMaterialName("CelShadingOrange");
		
		CEGUI::WindowManager::getSingleton().getWindow("DeleteObject")->enable();
	}
	else
		CEGUI::WindowManager::getSingleton().getWindow("DeleteObject")->disable();
}
Object* ObjectManager::getObjectBelowCursor()
{
	if (!editObjects)
		return NULL;
	
	float mx = windowManager.getMouseXRel();
	float my = windowManager.getMouseYRel();
	
	if ((mx < 0) || (mx > 1) || (my < 0) || (my > 1))
		return NULL;
	
	// Execute query
	Ray mouseRay = camera->getCameraToViewportRay(mx, my);
	sceneQuery->setRay(mouseRay);
	RaySceneQueryResult &result = sceneQuery->execute();
	if (!result.size())
		return NULL;
	
	// Filter results
	MovableObject* movableObj = NULL;
	RaySceneQueryResult::iterator itr = result.begin();
	do
	{
		if (!itr->movable)
			continue;
		if (itr->movable->getUserAny().isEmpty())
			continue;
		
		movableObj = itr->movable;
		break;
	}
	while (++itr != result.end());
	
	if (!movableObj)
		return NULL;
	
	if (movableObj->getName().compare(0, strlen("ObjectAnchor"), "ObjectAnchor") != 0)
		return NULL;
	
	return any_cast<Object*>(movableObj->getUserAny());
}

void ObjectManager::setEditMode(EEditMode mode)
{
	if (!selObject)
		return;
	
	editModeActive = true;
	editMode = mode;
	editDirection = Vector3(1, 1, 1);
	editStartCursorPos = windowManager.getMouseXRel();
	
	if (mode == EDITMODE_ROTATE)
		selObject->sendGetMessage(COMPMSG_GET_ORIENTATION, &editBaseQuaternion);
	else if (mode == EDITMODE_GRAB)
		editBaseVector = selObject->position;
	else if (mode == EDITMODE_SCALE)
		selObject->sendGetMessage(COMPMSG_GET_SCALE, &editBaseVector);
}
void ObjectManager::setEditDirection(Vector3 direction)
{
	if (!editModeActive)
		return;
	
	editDirection = direction;
	updateEditing(getEditAmount());
}

bool ObjectManager::isEditModeActive()
{
	return editModeActive;
}
void ObjectManager::finishEditing()
{
	updateEditing(getEditAmount());
	editModeActive = false;
}
void ObjectManager::abortEditing()
{
	updateEditing(0);
	editModeActive = false;
}
void ObjectManager::updateEditing(float modifyAmount)
{
	if (!selObject)
	{
		// The object which is being edited has been deleted.
		editModeActive = false;
		return;
	}
	
	if (editMode == EDITMODE_ROTATE)
	{
		btQuaternion rotationQuaternion(btVector3(editDirection.x, editDirection.y, editDirection.z), modifyAmount * 5.0f);
		btQuaternion rotatedQuaternion = editBaseQuaternion * rotationQuaternion;
		selObject->sendMessage(COMPMSG_SET_ORIENTATION, &rotatedQuaternion);
	}
	else if (editMode == EDITMODE_GRAB)
	{
		Vector3 movedPosition = Vector3(editBaseVector + editDirection * (modifyAmount * 10.0f));
		selObject->sendMessage(COMPMSG_SET_POSITION, &movedPosition);
	}
	else if (editMode == EDITMODE_SCALE)
	{
		Vector3 scaledSize = Vector3(editBaseVector + editDirection * (modifyAmount * 3.0f));
		selObject->sendMessage(COMPMSG_SET_SCALE, &scaledSize);
	}
}
float ObjectManager::getEditAmount()
{
	return windowManager.getMouseXRel() - editStartCursorPos;
}
