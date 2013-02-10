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
#include "component.h"

ComponentTemplateManager componentTemplateManager;

Component *
ComponentTemplate::
newInstance(Object* object)
{
  NamedComponent* ret = this->createInstance(object);
  ret->creator = this;
  return ret;
}

void ComponentTemplateManager::registerTemplate(string name, ComponentTemplateCreateFunc createFunc)
{
	compMap.insert(std::map<string, ComponentTemplateCreateFunc>::value_type(name, createFunc));
}

ComponentTemplate* ComponentTemplateManager::createTemplate(const char* name, TiXmlElement* params)
{
	std::map<string, ComponentTemplateCreateFunc>::iterator it = compMap.find(name);
	if (it == compMap.end())
	{
		LogManager::getSingleton().logMessage("ERROR: tried to create component of type \"" + string(name) + "\" which doesn't exist!", LML_CRITICAL);
		return NULL;
	}

	ComponentTemplate* newTemplate = (*it).second(name);
	if (!newTemplate->init(params))
		return NULL;
	return newTemplate;
}

void ComponentTemplateManager::exit()
{
	compMap.clear();
}
