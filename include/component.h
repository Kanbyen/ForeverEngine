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

#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#include <map>
#include <list>
using namespace std;
#include "tinyxml.h"
#include "componentMessages.h"

/// What some components should do after finishing execution
enum ActionMode
{
	ACTIONMODE_REPEAT = 0,
	ACTIONMODE_DELETECOMPONENT = 1,
	ACTIONMODE_DELETEOBJECT = 2,
};

class Object;

/// Base class for all components
class Component
{
public:
	Object* object;

	/// Must free all allocated memory and delete itself
	virtual void exit() = 0;

	/// Must return a uniqe name... use NamedComponent for easy handling of this
  virtual const string& name() = 0;

	/// Must process the message; for the details of the return value semantic, see the detailed description
	/**
		For normal messages: Component must return false if it has deleted itself or the trigger which caused the message ("it doesn't want to live on")
	*/
	virtual bool handleMessage(int msgID, void* data, void* data2 = NULL) = 0;

	/// Must process the message; for the details of the return value semantic, see the detailed description
	/**
		For _GET_ messages: return true if the search for the correct component should continue, false if the result was passed in and the search should be stopped
	*/
	virtual bool handleGetMessage(int msgID, void* data, void* data2 = NULL) const = 0;
};

typedef std::list<Component*> ComponentList;

class ComponentTemplate;

template<class T>
ComponentTemplate * createTemplate(string name);

class NamedComponent;

/// Creates Components. Derived classes store all information needed for this process.
class ComponentTemplate
{
  template<class T>
	  friend ComponentTemplate * createTemplate(const string& name);
protected:
  string _name;
public:

  const string& name() const { return _name; }

	/// Must initialize the component template by retrieving the needed parameters from the xml file
	virtual bool init(TiXmlElement* params) = 0;

	/// Must clean up everything and delete itself
	virtual void exit() = 0;

	/// Must create an instance of this template
	virtual NamedComponent* createInstance(Object* object) = 0;

	Component * newInstance(Object* object);
};

class NamedComponent : public Component
{
  friend class ComponentTemplate;
protected:
  ComponentTemplate const * creator;
public:
  virtual const string& name() { return creator->name(); } // TODO: will any subclasses ever need this to be virtual?
};

typedef ComponentTemplate* (*ComponentTemplateCreateFunc)(const string& name);

/// Creates component templates for object templates
class ComponentTemplateManager
{
public:
	std::map<string, ComponentTemplateCreateFunc> compMap;

	/// Every component type must be registered with this method
	void registerTemplate(string name, ComponentTemplateCreateFunc createFunc);

	/// Retrieves the creation function for the component of the specified type
	ComponentTemplate* createTemplate(const char* name, TiXmlElement* params);

	/// Cleans up
	void exit();
};

template<class T>
ComponentTemplate * createTemplate(const string& name) {
  ComponentTemplate * ret = new T;
  ret->_name = name;
  return ret;
}


extern ComponentTemplateManager componentTemplateManager;

#endif
