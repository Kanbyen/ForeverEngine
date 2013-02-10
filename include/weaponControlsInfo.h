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

#ifndef _WEAPON_CONTROLS_INFO_H_
#define _WEAPON_CONTROLS_INFO_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "util.h"

class StaticText;
class ComponentWeapon;

class WeaponControlsInfo
{
public:
	float currentTime;
	float setTime;
	float expireTime;

	StaticText text[2];

	void set(ComponentWeapon* weapon);

	void init();
	void update(float time);
	void exit();
};

#endif
