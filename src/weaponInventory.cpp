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
#include "weaponInventory.h"
#include "componentPlayer.h"
#include "windowManager.h"
#include "object.h"
#include "util.h"
#include "stateGame.h"		// TODO: <REFACTOR> better remove this and pass mPlayer to this class

void WeaponInventory::displayList()
{
	if (listShown)
		return;
	timeSinceLastChange = 0;
	listShown = true;
}
void WeaponInventory::hideList()
{
	if (!listShown)
		return;
	timeSinceLastChange = 0;
	listShown = false;
}
void WeaponInventory::createList()
{
	listDirty = false;
	deleteList();
	displayCreated = true;

	float offsetY = 0;
	int i = 0;
	weaponDisplay.resize(weaponSet.size());
	for (weaponSetItr it = weaponSet.begin(); it != weaponSet.end(); it++)
	{
		CEGUI::UDim x = cegui_reldim(startX) + ((it == activeWeapon) ? cegui_absdim(10) : cegui_absdim(0));
		
		weaponDisplay[i].create((*it)->name, x, cegui_reldim(startY) + cegui_absdim(offsetY),
						 x + cegui_absdim(windowManager.getFont()->getTextExtent((*it)->name)),
						 cegui_reldim(startY) + cegui_absdim(offsetY) + cegui_absdim(windowManager.getFont()->getLineSpacing()),
						 root, (it == activeWeapon) ? DESCRIPTION_MESSAGE_COLOR : "DDDDDD");

		offsetY += windowManager.getFont()->getLineSpacing();
		i++;
	}

	string weaponDisplayInfoText = "";
	if (type != WEAPONTYPE_GRENADE)
	{
		weaponDisplayInfoText += "grenade: ";
		weaponDisplayInfoText += game.mPlayer->getActShiftWeapon() ? game.mPlayer->getActShiftWeapon()->shortName : "none";
	}
	else
	{
		weaponDisplayInfoText += game.mPlayer->isUsingATool ? "tool: " : "weapon: ";
		weaponDisplayInfoText += game.mPlayer->getActLMBWeapon() ? game.mPlayer->getActLMBWeapon()->name : "none";
	}
	weaponDisplayInfoText += "\n";
	if (type == WEAPONTYPE_GRENADE)
		weaponDisplayInfoText += "grenade: ";
	else if (type == WEAPONTYPE_NORMAL)
		weaponDisplayInfoText += "weapon: ";
	else if (type == WEAPONTYPE_TOOL)
		weaponDisplayInfoText += "tool: ";
	
	weaponDisplayInfo.create(weaponDisplayInfoText,
							 cegui_reldim(startX) - cegui_absdim(10),
							 cegui_reldim(startY) - cegui_absdim(windowManager.getFont()->getLineSpacing() * 2),
							 cegui_reldim(startX) - cegui_absdim(10) + cegui_absdim(windowManager.getFont()->getTextExtent(weaponDisplayInfoText)),
							 cegui_reldim(startY) - cegui_absdim(windowManager.getFont()->getLineSpacing() * 0),
							 root, "FFFFFF");

	updateListAlpha();
}
void WeaponInventory::deleteList()
{
	if (!displayCreated)
		return;
	
	for (int i = 0; i < (int)weaponDisplay.size(); i++)
		weaponDisplay[i].destroy();
	weaponDisplayInfo.destroy();
	weaponDisplay.clear();

	displayCreated = false;
}
void WeaponInventory::updateListAlpha()
{
	if (!displayCreated)
		return;
	
	float alpha;
	if (timeSinceLastChange < fadeTime)
	{
		if (listShown)
			alpha = timeSinceLastChange / fadeTime;
		else
			alpha = (fadeTime - timeSinceLastChange) / fadeTime;
	}
	else
	{
		if (listShown)
			alpha = 1.0f;
		else
			alpha = 0.0f;
	}

	for (int i = 0; i < (int)weaponDisplay.size(); i++)
		weaponDisplay[i].setAlpha(alpha);
	weaponDisplayInfo.setAlpha(alpha);
}
bool WeaponInventory::addWeapon(ComponentWeapon* weapon)
{
	// Is the weapon already in the inventory?
	for (weaponSetItr it = weaponSet.begin(); it != weaponSet.end(); it++)
	{
		if (weapon->name == (*it)->name)
			return true;
	}

	listDirty = true;	// TODO: This was at the top of this function because even if the weapon is already in the inventory, the ammo could have changed -> the list has to be regenerated

	weaponSet.insert(weapon);

	// Was the set empty? Then set the first item as active item
	if (weaponSet.size() == 1)
	{
		activeWeapon = weaponSet.begin();
	}
	
	return false;
}
void WeaponInventory::removeWeapon(ComponentWeapon* weapon)
{
	listDirty = true;
	weaponSet.erase(weapon);
}
void WeaponInventory::update(float timepassed)
{
	timeSinceLastChange += timepassed;

	if (listDirty)
		createList();

	updateListAlpha();
}
void WeaponInventory::init(CEGUI::Window* root, float startX, float startY, float fadeTime, WeaponType type)
{
	listShown = false;
	listDirty = false;
	displayCreated = false;
	timeSinceLastChange = 0;
	
	this->root = root;
	this->startX = startX;
	this->startY = startY;
	this->fadeTime = fadeTime;
	this->type = type;
}
void WeaponInventory::deleteObjects()
{
	for (weaponSetItr it = weaponSet.begin(); it != weaponSet.end(); it++)
		(*it)->object->exit(); // The objects are not in the object manager, so don't do this: objectManager.deleteObject((*it)->object);
	weaponSet.clear();
}
void WeaponInventory::exit()
{
	deleteObjects();

	deleteList();
}
