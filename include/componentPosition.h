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

#ifndef _COMPONENT_POSITION_H_
#define _COMPONENT_POSITION_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "tinyxml.h"

/// Manages position and rotation of a component
class ComponentPosition
{
public:
	Vector3 position;
	Quaternion quatRotation;
	Vector3 rotation;

	/// Constructor - sets everything to zero / identity
	ComponentPosition();

	/// Constructor - don't use if you use readFrom
	ComponentPosition(float posX, float posY, float posZ, float rotX, float rotY, float rotZ);

	/// Reads the data from a XML file
	void readFrom(TiXmlElement* node);

	/// Applies the transformation to a scene node
	void applyTo(SceneNode* node);

	/// Calculates the quaternion from vecRotation
	void calculateQuaternion();
};

#endif
