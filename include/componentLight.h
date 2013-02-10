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

#ifndef _COMPONENT_LIGHT_H_
#define _COMPONENT_LIGHT_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "component.h"

/// Light Component
/**
	Currently only point lights are supported.
	Parameters:<br>
	Radius [required] - the radius of the point light (parameter: value)<br>
	Color - the light color (parameters: r, g, b)
*/
class ComponentLight : public NamedComponent
{
public:
	SceneNode* sceneNode;
	Light* light;

	virtual void exit();
	virtual bool handleMessage(int msgID, void* data, void* data2 = NULL);
	virtual bool handleGetMessage(int msgID, void* data, void* data2 = NULL) const;
};

class ComponentTemplateLight : public ComponentTemplate
{
public:
	static int lightComponentNumber;

	float radius;
	ColourValue color;

	virtual NamedComponent* createInstance(Object* object);
	virtual bool init(TiXmlElement* params);
	virtual void exit();
};

#endif
