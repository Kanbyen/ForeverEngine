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

#ifndef _COMPONENT_PLAYER_H_
#define _COMPONENT_PLAYER_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "component.h"
#include "weaponControlsInfo.h"
#include "weaponInventory.h"
#include "weaponInHand.h"
#include "util.h"
#include "input.h"

const float DESCRIPTION_MESSAGE_DURATION = 15.0f;
const string DESCRIPTION_MESSAGE_COLOR = "ffcd1e";
const float HINT_MESSAGE_DURATION = 15.0f;
const string HINT_MESSAGE_COLOR = "ff9d4a";

const float ITEM_PICKUP_DISTANCE = 7.0f;
const float ITEM_PICKUP_DISTANCE_SQ = ITEM_PICKUP_DISTANCE * ITEM_PICKUP_DISTANCE;

const int TEXT_ENTRY_LINE_MAX_LEN = 127;

class ComponentItem;
class ComponentCharacterController;

/// Player Component
/**
	Parameters: none
 */
class ComponentPlayer : public NamedComponent
{
public:

	// Weapon inventory
	WeaponControlsInfo weaponControlsInfo;						// Displays information about the weapon controls in the middle of the screen
	WeaponInHand weaponInHand;									// The weapon currently held
	WeaponInventory weapons[3];									// The array is indexed by WeaponComponent::type
	int actWeaponInventory;										// Index to weapons[]
	bool isUsingATool;											// Does the player hold a tool or a normal weapon in his hands?
	float inventoryShowTime;									// After this time is elapsed, the current weapon inventory will be hidden
	int weaponPickedUp;											// How many weapons the player has picked up since the game was started. Useful for showing help texts.
	bool grenadePickedUp;
	bool toolPickedUp;
	
	bool dontRespawn;											// True if the user presses P to exit player mode and switch to the editor

	void showWeaponInventory(int number, bool showDesc = true);	// Shows the weapon inventory of the specified number, or the current inventory if -1 is passed as number; if showDesc is true, the weapon description will be shown in the descConsole
	ComponentWeapon* getActLMBWeapon();							// Returns the currently selected weapon or tool (or NULL)
	ComponentWeapon* getActShiftWeapon();						// Returns the currently selected grenade (or NULL)
	bool throwGrenade();										// Returns if a grenade was thrown

	// Item pickup
	ComponentItem*	targetItem;									// If the targeted item is in pickup range, this points to it

	/// The crosshair overlay
	Overlay* crosshair;

	// The overlay shown when the player is damaged
	static Ogre::ManualObject* playerDamagedOverlay;
	static Ogre::SceneNode* playerDamagedSceneNode;
	static Ogre::MaterialPtr playerDamagedMaterialPtr;
	static float playerDamagedBlend;
	static void createPlayerDamagedOverlay();
	static void destroyPlayerDamagedOverlay();
	static void handlePlayerDamagedOverlay(float dt);

	// Character controller
	ComponentCharacterController* myCharacterController;
	void playerStep(bool move, Vector3 direction, bool wantsToJump, float timeSinceLastFrame);
	
	// Text entry line (to chat with other players or possibly enter commands)
	char textEntryLineText[TEXT_ENTRY_LINE_MAX_LEN + 1];
	int textEntryLineCursor;
	StaticText textEntryLine;
	void showTextEntryLine();
	void hideTextEntryLine();
	void updateTextEntryLine();
	void textEntryLineEnter();
	void textEntryLineKeyEvent(const OIS::KeyEvent& evt);

	// Misc
	StaticText targetName;
	static void addHint(string text);							// static so that ComponentTemplatePlayer can call it

	// Component stuff
	virtual void exit();
	virtual bool handleMessage(int msgID, void* data, void* data2 = NULL);
	virtual bool handleGetMessage(int msgID, void* data, void* data2 = NULL) const;
};

class ComponentTemplatePlayer : public ComponentTemplate
{
public:

	virtual NamedComponent* createInstance(Object* object);
	virtual bool init(TiXmlElement* params);
	virtual void exit();
};

/// Input handler for player control. Pseudo-Singleton: use the global variable playerInputHandler
class PlayerInputHandler : public InputHandler
{
public:

	virtual void update(float timeSinceLastFrame);

	virtual bool mouseMoved(const OIS::MouseEvent& evt);
	virtual bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID btn);

	virtual bool keyPressed(const OIS::KeyEvent& evt);

protected:

	void handleMouse(float timeSinceLastFrame);
	void handleKeyboard(float timeSinceLastFrame);

	Radian rotX, rotY;
	
	bool textEntryLineActive;
};

extern PlayerInputHandler playerInputHandler;

#endif
