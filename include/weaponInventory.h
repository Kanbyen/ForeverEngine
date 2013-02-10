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

#ifndef _WEAPON_INVENTORY_H_
#define _WEAPON_INVENTORY_H_

#include <vector>
#include <set>
#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "util.h"
#include "componentWeapon.h"

const float WEAPONLIST_DISPLAYTIME = 6.0f;

/// Manages lists of weapons and displays them if neccessary
class WeaponInventory
{
public:
	/// Parent window + start position of all windows created by this class
	CEGUI::Window* root;
	float startX;
	float startY;
	WeaponType type;

	bool listShown;
	bool listDirty;
	float timeSinceLastChange;
	float fadeTime;

	/// Sorted list of weapon components
	std::multiset<ComponentWeapon*, componentWeaponImportance > weaponSet;
	typedef std::multiset<ComponentWeapon*, componentWeaponImportance >::iterator weaponSetItr;

	/// The visible weapon list, generated from the list above
	std::vector<StaticText> weaponDisplay;

	/// Is the weaponDisplay vector already generated?
	bool displayCreated;

	/// The information text above the weapon list
	StaticText weaponDisplayInfo;

	/// The currently active weapon
	weaponSetItr activeWeapon;

	/// Shows the weapon list
	void displayList();

	/// Hides the weapon list
	void hideList();

	/// (Re)creates the visible weapon list
	void createList();

	/// Deletes the visible weapon list
	void deleteList();

	/// Sets the visibility of the list
	void updateListAlpha();

	/// Returns if there is a weapon in this inventory
	inline bool empty()
	{
		return weaponSet.empty();
	}

	/// Adds a weapon to the inventory. Returns true if it already exists.
	bool addWeapon(ComponentWeapon* weapon);

	/// Remove a weapon from the list. Does not destroy the removed component / object!
	void removeWeapon(ComponentWeapon* weapon);

	/// Initializes the weapon inventory
	void init(CEGUI::Window* root, float startX, float startY, float fadeTime, WeaponType type);

	/// Should be called every frame
	void update(float timepassed);

	/// Deletes al contained objects
	void deleteObjects();

	/// Cleans up
	void exit();
};

#endif
