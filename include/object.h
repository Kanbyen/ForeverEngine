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

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <list>
#include <map>
#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include <LinearMath/btQuaternion.h>
using namespace Ogre;
#include "component.h"

class StaticText;
class ComponentMesh;
#ifdef USE_RAKNET
class ComponentNetwork;
class Network;
#endif

#include <string>

enum ObjectTag
{
	OBJTAG_AGENT = 1 << 0,
 	OBJTAG_BULLET = 1 << 1,
};

enum TriggerType
{
	TRIGTYPE_TOUCH = 1 << 0,
};

enum TriggerSubType
{
	TRIGSUBTYPE_WEAPONPICKUP = 1,
};

enum ObjectDeleteState
{
	OBJDELETE_NO = 0,
 	OBJDELETE_YES = 1,
  	OBJDELETE_JUSTREMOVE = 2,
};

const int INVALID_OBJECT_ID = -1;

/// Represents a game object, which is a collection of components (-> component.h)
class Object
{
public:
	/// the name with which this object was created
	string name;

	/// The unique ID of the object with which it can be looked up in the object manager
	int id;

	/// The id of the object which created this object
	int creatorID;

private:
	// TODO: change this to a map or a B* search tree?
	/// List of all components of the object
	ComponentList components;

public:
	const ComponentList& getComponentList() const { return components; };

	/// Add a new component, will be broadcasted over the network if forward is set
	void addComponent(Component* component, bool forward = true);

	/// Remove a component from the list. Warning: you have to delete the component yourself (call its exit method)!
	void removeComponent(Component* component, bool forward = true);

#ifdef USE_RAKNET
	ComponentNetwork* net;
#endif

	/// Quick way to get the mesh component
	ComponentMesh* mesh;

	/// Quick way to get the position
	Vector3 position;

	/// Here, components can enter bit flags from ObjectTag to enable some quick checks
	int tag;

	/// Trigger types the object wants to listen to
	TriggerType listenTo;

	/// If the object should be deleted / removed from the object list in the next delete loop
	ObjectDeleteState deleted;
	
	/// Object anchor scene node (if editObjects is true)
	SceneNode* anchorNode;


	/// Constructor
	Object();

	/// Send a message to all components except the one "exclude" points to. Be careful, the object could delete itself as response - then the function returns false!
	bool sendMessage(int msgID, void* data, void* data2 = NULL, Component* exclude = NULL);

	/// Same as sendMessage, but the caller expects a result to be written to *data. Differences to sendMessage: 1. stops as soon as a component gives a result 2. returns true if a result is there, false otherwise
	bool sendGetMessage(int msgID, void* data, void* data2 = NULL, Component* exclude = NULL) const;

	// Returns the component with the given name or NULL
	Component* getComponent(string name);

	/// Destroys the object and all its components. Caution, deletes itself!
	void exit();
};

class Trigger
{
public:
	/// The object which has caused the trigger or NULL
	Object* source;

	/// The component which has caused the trigger or NULL
	Component* sourceComp;

	/// The center of the trigger area
	Ogre::Vector3 position;

	/// The radius of the trigger area, squared
	float radiusSquared;

	/// The general type of the trigger
	TriggerType type;

	/// The specific type of the trigger
	TriggerSubType subType;

	/// If the trigger should be deleted in the next delete loop
	bool deleted;
};

struct ComponentCreateInfo
{
	/// The component template
	ComponentTemplate* compTemplate;
};

enum EEditMode
{
	EDITMODE_GRAB,
	EDITMODE_ROTATE,
	EDITMODE_SCALE
};

/// Manages all active objects. Inactive objects are handled by the GameDataStorage
class ObjectManager
{
public:
	typedef std::list<ComponentCreateInfo> compCreateList;

	/// Maps object names to lists of functions (+ parameters for the init function) which create the components required for the object
	std::map<string, compCreateList> objectComponentMap;

	typedef std::map<int, Object*> ObjectMap;
	typedef std::map<int, Object*>::iterator ObjectMapItr;

	/// Map of all existing objects
	ObjectMap objectMap;

	/// The id assigned to the next object created (if the id is not in use)
	int newID;

	/// If the object update loop is currently active (if yes, objects cannot be deleted instantly, their deletion is delayed to the object delete loop)
	bool updating;

#ifdef USE_RAKNET
	/// The Network object for synchronization
	Network * net;
#endif

	ObjectManager();

	/// Reads all .object files in the given archive (not recursive)
	void readObjectsFromArchive(const char* filename, const char* archiveType);

	/// Tries to read the object descriptions from the specified file
	void readObjectsFromFile(const char* file);

	/// Adds a new object type
	void addObjectType(TiXmlElement* elem);

	/// Creates an object of the specified type and adds it to the list of active objects
	Object* createObject(const char* name, int creatorID);

	/// Creates an object of the specified type, doesn't add it to the list of active objects
	Object* createInactiveObject(const char* name, int creatorID);

	/// Adds an existing object (probably from the gameObjectStorage) to the object list
	void addExistingObjectToList(Object* object);

	/// Deletes the specified object; if the update loop is running, the action is delayed
	void deleteObject(Object* object);

	/// Removes the specified object from the list, but doesn't delete it; if the update loop is running, the action is delayed
	void removeObjectFromList(Object* object);

	/// Gets the pointer to an object by its id; if that id doesn't exist, it returns NULL
	Object* getObjectByID(int id);
	
	/// Enable or disable object editing
	void setEditObjects(bool edit);
	inline bool getEditObjects() const {return editObjects;}

	/// Sends the update message to all objects, handles triggers, and deletes objects and triggers with the "deleted" flag. Should be called after every physics step.
	void update();
	
	/// Updates other stuff (object anchors) which should also be updated if the simulation is paused
	void updateManager();
	
	// Object selection
	void selectObjectBelowCursor();
	inline Object* getSelectedObject() const {return selObject;}
	void selectObject(Object* newSelection);

	// Object editing
	void setEditMode(EEditMode mode);
	void setEditDirection(Vector3 direction);
	
	bool isEditModeActive();
	void finishEditing();
	void abortEditing();
	

	/// Maps trigger IDs to trigger classes
	std::map<int, Trigger> triggerMap;

	/// ID for the next trigger
	int newTriggerID;

	/// Adds a trigger. Returns the trigger ID with which it can / must be deleted
	int addTrigger(Object* source, Component* sourceComp, TriggerType type, TriggerSubType subType, float radius, Ogre::Vector3 position = Ogre::Vector3(0, 0, 0));

	/// Sets the position of an existing trigger
	void setTriggerPos(int ID, Ogre::Vector3 position);

	/// Deletes a trigger
	void deleteTrigger(int ID);


	/// Initializes the object manager
	void init();
	
	/// Deletes all objects
	void deleteAllObjects();	

	/// Cleans up
	void exit();


	// Make the class behave like a map of ID -> objects
	typedef ObjectMapItr iterator;
	inline const iterator begin()	{ return objectMap.begin(); }
	inline const iterator end()		{ return objectMap.end(); }
	
protected:
	
	bool editObjects;
	int objectAnchorNumber;
	RaySceneQuery* sceneQuery;
	StaticText* tooltip;
	Object* selObject;
	
	bool editModeActive;
	EEditMode editMode;
	Vector3 editDirection;
	float editStartCursorPos;
	Vector3 editBaseVector;
	btQuaternion editBaseQuaternion;
	
	void updateEditing(float modifyAmount);
	float getEditAmount();	/// calculates the edit amount based on the cursor position
	
	void addObject(Object* newObject);
	void removeObject(Object* oldObject);
	Object* getObjectBelowCursor();
	
	void initTooltip();
	void exitTooltip();
};

extern ObjectManager objectManager;

#endif
