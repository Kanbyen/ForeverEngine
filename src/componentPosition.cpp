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
#include "componentPosition.h"

ComponentPosition::ComponentPosition()
{
	position = Vector3::ZERO;
	quatRotation = Quaternion::IDENTITY;
	rotation = Vector3::ZERO;
}
ComponentPosition::ComponentPosition(float posX, float posY, float posZ, float rotX, float rotY, float rotZ)
{
	position = Vector3(posX, posY, posZ);
	rotation = Vector3(rotX, rotY, rotZ);
	calculateQuaternion();
}
void ComponentPosition::readFrom(TiXmlElement* node)
{
	const char* str = node->Attribute("position");
	if (str)
		position = StringConverter::parseVector3(str);
	else
	{
		LogManager::getSingleton().logMessage("WARNING: no position parameter for a ComponentPosition!");
	}
	
	str = node->Attribute("rotation");
	if (str)
	{
		rotation = StringConverter::parseVector3(str);
		calculateQuaternion();
	}
	//else
	//	LogManager::getSingleton().logMessage("WARNING: no rotation parameter for a ComponentPosition!");
}
void ComponentPosition::calculateQuaternion()
{
	quatRotation = Quaternion(Degree(rotation.x), Vector3::UNIT_X);
	quatRotation = Quaternion(Degree(rotation.y), Vector3::UNIT_Y) * quatRotation;
	quatRotation = Quaternion(Degree(rotation.z), Vector3::UNIT_Z) * quatRotation;
}
void ComponentPosition::applyTo(SceneNode* node)
{
	node->setPosition(position);
	node->setOrientation(quatRotation);
}
