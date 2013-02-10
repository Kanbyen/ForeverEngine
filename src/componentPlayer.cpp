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
#include "componentPlayer.h"
#include "componentWeapon.h"
#include "componentItem.h"
#include "componentMesh.h"
#include "componentCharacterController.h"
#include "object.h"
#include "console.h"
#include "stateGame.h"
#include "windowManager.h"
#include "luaFunctions.h"
#include "settings.h"
#include "stateGameInputHandler.h"
#include "physicsConstants.h"
#include "objectScript.h"
#include "network.h"

ManualObject* ComponentPlayer::playerDamagedOverlay;
SceneNode* ComponentPlayer::playerDamagedSceneNode;
MaterialPtr ComponentPlayer::playerDamagedMaterialPtr;
float ComponentPlayer::playerDamagedBlend;

PlayerInputHandler playerInputHandler;

float PLAYER_DAMAGED_OVERLAY_FADE_SPEED = 2.5f;

void ComponentPlayer::showWeaponInventory(int number, bool showDesc)
{
	// Do nothing if the target inventory is empty
	if (number == -1)
	{
		if (weapons[actWeaponInventory].empty())
			return;
	}
	else
	{
		if (weapons[number].empty())
			return;
	}

	// Hide the currently displayed inventory
	if ((actWeaponInventory != number) && (number != -1))
	{
		weapons[actWeaponInventory].hideList();
		actWeaponInventory = number;
	}

	// Show the new inventory
	if (!weapons[actWeaponInventory].empty())
	{
		inventoryShowTime = WEAPONLIST_DISPLAYTIME;
		weapons[actWeaponInventory].displayList();

		// Also tell the WeaponInHand class if the current weapon should be displayed
		if (actWeaponInventory != WEAPONTYPE_GRENADE)
			weaponInHand.changeWeapon(*(weapons[actWeaponInventory].activeWeapon));

		// Show the weapon description
		//if (showDesc)
		//	game.descConsole->addMessage((*(weapons[actWeaponInventory].activeWeapon))->description, DESCRIPTION_MESSAGE_DURATION, DESCRIPTION_MESSAGE_COLOR);

		// Show the weapon controls info
		weaponControlsInfo.set((*(weapons[actWeaponInventory].activeWeapon)));

		isUsingATool = (number == WEAPONTYPE_TOOL);
	}
}
ComponentWeapon* ComponentPlayer::getActLMBWeapon()
{
	const int index = isUsingATool ? WEAPONTYPE_TOOL : WEAPONTYPE_NORMAL;
	if (weapons[index].empty())
		return NULL;
	else
		return *(weapons[index].activeWeapon);
}
ComponentWeapon* ComponentPlayer::getActShiftWeapon()
{
	const int index = WEAPONTYPE_GRENADE;
	if (weapons[index].empty())
		return NULL;
	else
		return *(weapons[index].activeWeapon);
}
bool ComponentPlayer::throwGrenade()
{
	ComponentWeapon* grenade = getActShiftWeapon();
	if (!grenade)
		return false;

	Vector3 spawnDirection = camera->getDerivedDirection();
	Vector3 spawnPosition = camera->getDerivedPosition() + spawnDirection * (myCharacterController->creator->height * 1.3f);
	spawnDirection *= 30;

	Object* obj = objectManager.createObject(grenade->throwName.c_str(), object->id);
	if (!obj)
		return false;

	obj->sendMessage(COMPMSG_CREATE, &spawnPosition);
	obj->sendMessage(COMPMSG_APPLYCENTRALIMPULSE, &spawnDirection);

	return true;
}
void ComponentPlayer::addHint(string text)
{
	game.gameConsole->addMessage(text, HINT_MESSAGE_DURATION, HINT_MESSAGE_COLOR);
}

void ComponentPlayer::exit()
{
	weaponInHand.exit();

	for (int i = 0; i < 3; i++)
		weapons[i].deleteObjects();

	destroyPlayerDamagedOverlay();

	targetName.destroy();

	weaponControlsInfo.exit();

	for (int i = 0; i < 3; i++)
		weapons[i].exit();

	// Notify the Game class
	game.mPlayer = NULL;
	if (dontRespawn)
		game.setSubState(SUBST_EDITOR);
	else
		game.setSubState(SUBST_RESPAWN);

	// Show HUD
	windowManager.setPage("Editor");
	crosshair->hide();

	// Register game input handler
	inputManager->setInputHandler(&gameInputHandler);

	delete this;
}
bool ComponentPlayer::handleMessage(int msgID, void* data, void* data2)
{
	int weaponType;
	Trigger* trigger;
	btVector3 tempbtVector;

	switch (msgID)
	{
	case COMPMSG_UPDATEPOS:
		// Handle camera
		object->sendGetMessage(COMPMSG_GET_HEAD, &object->position, NULL, this);
		game.setCameraPosition(object->position);
	break;
	case COMPMSG_UPDATE:
	{
		// Handle "player damaged" overlay
		handlePlayerDamagedOverlay(LOGIC_TIME_STEP);

		// Handle weapon inventories
		weaponControlsInfo.update(LOGIC_TIME_STEP);

		weaponInHand.update(LOGIC_TIME_STEP);

		for (int i = 0; i < 3; i++)
			weapons[i].update(LOGIC_TIME_STEP);

		inventoryShowTime -= LOGIC_TIME_STEP;
		if (inventoryShowTime < 0)
			weapons[actWeaponInventory].hideList();

		// Handle item target and target display
		string targetText = "";

		Vector3 head;
		object->sendGetMessage(COMPMSG_GET_HEAD, &head, NULL, this);
		Vector3 aim;
		handleGetMessage(COMPMSG_GET_AIM, &aim);
		Vector3 hitPos;
		Object* obj = NULL;

		targetItem = NULL;

		if (luaRayTestCombined(&head, &aim, 60, &hitPos, object, RAYTEST_NO_DEBRIS))
		{
			obj = luaGetLastHitObject();
			if (obj)
				obj->sendGetMessage(COMPMSG_GET_NAME, &targetText);
		}

		if (targetText.size())
		{
			if ((hitPos - head).squaredLength() <= ITEM_PICKUP_DISTANCE_SQ)
				obj->sendGetMessage(COMPMSG_GET_ITEM_COMPONENT, &targetItem);

			targetName.setText(targetText.c_str());
			targetName.setAlpha(1);
			if (targetItem && (targetItem->creator->holdable || targetItem->creator->takeable))
				targetName.textWnd->setProperty("TextColours", "tl:FFffc018 tr:FFffc018 bl:FFffc018 br:FFffc018");
			else
				targetName.textWnd->setProperty("TextColours", "tl:FFFFFFFF tr:FFFFFFFF bl:FFFFFFFF br:FFFFFFFF");

			float textExtent = windowManager.getFont()->getTextExtent(targetText) + 16;	// 16: border size
			targetName.setXPosition(cegui_reldim(0.5) + cegui_absdim(-(textExtent / 2)));
			targetName.setWidth(cegui_absdim(textExtent));
		}
		else
		{
			targetName.setAlpha(0);
		}

		// Handle health display
		float fTemp;
		object->sendGetMessage(COMPMSG_GET_HEALTH, &fTemp);
		int health = (int)(fTemp);
		if (fTemp > 0 && health == 0)
			health = 1;
		object->sendGetMessage(COMPMSG_GET_MAX_HEALTH, &fTemp);
		int maxHealth = (int)(fTemp);
		if (fTemp > 0 && maxHealth == 0)
			maxHealth = 1;

		CEGUI::Window* wnd = CEGUI::WindowManager::getSingleton().getWindow("HealthText");
		wnd->setText(StringConverter::toString(health)); // + " / " + StringConverter::toString(maxHealth));
	}
	break;
	case COMPMSG_HANDLETRIGGER:
		trigger = (Trigger*)data;
		if (trigger->type & TRIGTYPE_TOUCH)
		{
			switch (trigger->subType)
			{
			case TRIGSUBTYPE_WEAPONPICKUP:
				ComponentWeapon* weapon = (ComponentWeapon*)trigger->sourceComp;
				// Add the weapon into the correct inventory
				weaponType = (int)weapon->type;
				if (weapons[weaponType].addWeapon(weapon))
				{
					// Destroy the weapon object because there is already an equal one in the weaponInventory
					//objectManager.deleteObject(trigger->source);
				  return true; // we already got that weapon, someone else can handle this trigger (ammo-pickup?)
				}
				else
				{
					// Remove the object from the world, but don't delete it - it is kept in the inventory
					weapon->placeInInvectory();
					objectManager.removeObjectFromList(weapon->object);
				}

				game.gameConsole->addMessage("You picked up a " + weapon->name + "!");

				// Show the weapon description
				game.gameConsole->addMessage(weapon->description, DESCRIPTION_MESSAGE_DURATION, DESCRIPTION_MESSAGE_COLOR);


				// Show message(s)
				if (weaponPickedUp == 0)
					addHint("Use weapons and tools with [LMB] and [RMB],\nselect weapons with the mouse wheel.\nHelp for each weapon is displayed\nwhen selecting it.");
				else if (weaponPickedUp == 999)
					addHint("Congratulations to your 1000th weapon pickup in this game!");
				else if (weaponPickedUp == 1499)
					addHint("Incredible! This is your 1500th weapon pickup in this game!!");
				else if (weaponPickedUp == 1999)
					addHint("UNBELIEVABLE! This is your 2000th weapon pickup in this game!!!");
				else if (weaponPickedUp == 9999)
					addHint("Something went wrong. The counter tells me that\nthis is your 10000th weapon pickup in this game.\nAre you viewing this with a hex editor?!");
				weaponPickedUp++;

				if (!grenadePickedUp && weaponType == WEAPONTYPE_GRENADE)
				{
					grenadePickedUp = true;
					addHint("Use [G] to switch between grenade and normal weapon selection.\nThrow grenades with [Shift].");
				}
				if (!toolPickedUp && weaponType == WEAPONTYPE_TOOL)
				{
					toolPickedUp = true;
					addHint("Use [T] to switch between tool and normal weapon selection.");
				}

				// Show the weapon inventory TODO: To show or not to show, that is the question
				if (weapons[actWeaponInventory].empty())
					showWeaponInventory(weaponType, false);
				else
					showWeaponInventory(-1, false);

				return false;	// we have deleted the trigger which caused the event, so we must return false!
			break;
			}
		}
	break;
	case COMPMSG_ATTACH_CHILD_TO_WEAPON:
	{
		Object* obj = (Object*)data;
		Node* childNode;
		obj->sendGetMessage(COMPMSG_GET_NODE, &childNode);
		childNode->getParent()->removeChild(childNode);
		weaponInHand.scenenode->addChild(childNode);
		
		obj->position = childNode->_getDerivedPosition();	// TODO: is this one-time position setting necessary?
	}
	break;
	case COMPMSG_SETTARGETPOSITION:
		weaponInHand.offsetTo = *(ComponentPosition*)data;
	break;
	case COMPMSG_TAKEDAMAGE:
		playerDamagedBlend = 1.0f;
	break;
	case COMPMSG_UNLOAD:
		// Temporarily store the player because he is too far away from the player?! Doesn't make sense.
		assert(!"WARNING: The player object got the COMPMSG_UNLOAD message! Something is wrong!");
		objectManager.deleteObject(object);	// We delete the object - the player dies.
		return false;
	break;
	case COMPMSG_CREATE:
		// Mark this object as agent (like objects with AI component) so it can easily be told apart from uninteresting objects by the vision system
		object->tag |= OBJTAG_AGENT;

		// Get our character controller
		if (!object->sendGetMessage(COMPMSG_GET_CHARACTER_CONTROLLER, &myCharacterController))
			LogManager::getSingleton().logMessage("WARNING: no character controller component in a player object!");
	break;
	}

	return true;
}


bool ComponentPlayer::handleGetMessage(int msgID, void* data, void* data2) const
{
	btVector3 tempbtVector;
	switch(msgID)
	{
		case COMPMSG_GET_HEAD:
			return myCharacterController->handleGetMessage(COMPMSG_GET_HEAD, data, data2);
		case COMPMSG_GET_AIM:
			*((Vector3*)data) = camera->getDerivedDirection();
		return false;
		case COMPMSG_GET_FACTION:
			*((string*)data) = "Player";
		return false;
	}
	return true;
}

void ComponentPlayer::createPlayerDamagedOverlay()
{
	const float PLAYER_DAMAGED_OVERLAY_INSET = 0.4f;
	const float PLAYER_DAMAGED_OVERLAY_RED = 1.0f;

	// Create the material
	MaterialPtr materialPtr = MaterialManager::getSingleton().getByName("PlayerDamagedOverlayMaterial");
	if (materialPtr.isNull())
	{
		playerDamagedMaterialPtr = MaterialManager::getSingleton().create("PlayerDamagedOverlayMaterial", "General");
		playerDamagedMaterialPtr->getTechnique(0)->getPass(0)->setDepthCheckEnabled(false);
		playerDamagedMaterialPtr->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
		playerDamagedMaterialPtr->getTechnique(0)->getPass(0)->setLightingEnabled(false);
		playerDamagedMaterialPtr->getTechnique(0)->getPass(0)->setSceneBlending(SBT_TRANSPARENT_ALPHA);

		playerDamagedMaterialPtr->getTechnique(0)->getPass(0)->createTextureUnitState();
		playerDamagedMaterialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setBlank();
	}
	else
		playerDamagedMaterialPtr = materialPtr;

	// Create the "player damaged" overlay
	playerDamagedOverlay = sceneMgr->createManualObject("PlayerDamagedOverlay");

	// Use identity view/projection matrices
	playerDamagedOverlay->setUseIdentityProjection(true);
	playerDamagedOverlay->setUseIdentityView(true);

	playerDamagedOverlay->begin("PlayerDamagedOverlayMaterial", RenderOperation::OT_TRIANGLE_LIST);

	playerDamagedOverlay->position(-1, -1, 0.0);
	playerDamagedOverlay->colour(PLAYER_DAMAGED_OVERLAY_RED, 0, 0, 1);
	playerDamagedOverlay->position(-1 + PLAYER_DAMAGED_OVERLAY_INSET, -1 + PLAYER_DAMAGED_OVERLAY_INSET, 0.0);
	playerDamagedOverlay->colour(PLAYER_DAMAGED_OVERLAY_RED, 0, 0, 0);

	playerDamagedOverlay->position( 1, -1, 0.0);
	playerDamagedOverlay->colour(PLAYER_DAMAGED_OVERLAY_RED, 0, 0, 1);
	playerDamagedOverlay->position( 1 - PLAYER_DAMAGED_OVERLAY_INSET, -1 + PLAYER_DAMAGED_OVERLAY_INSET, 0.0);
	playerDamagedOverlay->colour(PLAYER_DAMAGED_OVERLAY_RED, 0, 0, 0);

	playerDamagedOverlay->position( 1,  1, 0.0);
	playerDamagedOverlay->colour(PLAYER_DAMAGED_OVERLAY_RED, 0, 0, 1);
	playerDamagedOverlay->position( 1 - PLAYER_DAMAGED_OVERLAY_INSET,  1 - PLAYER_DAMAGED_OVERLAY_INSET, 0.0);
	playerDamagedOverlay->colour(PLAYER_DAMAGED_OVERLAY_RED, 0, 0, 0);

	playerDamagedOverlay->position(-1,  1, 0.0);
	playerDamagedOverlay->colour(PLAYER_DAMAGED_OVERLAY_RED, 0, 0, 1);
	playerDamagedOverlay->position(-1 + PLAYER_DAMAGED_OVERLAY_INSET,  1 - PLAYER_DAMAGED_OVERLAY_INSET, 0.0);
	playerDamagedOverlay->colour(PLAYER_DAMAGED_OVERLAY_RED, 0, 0, 0);

	for (int i = 0; i < 8; i += 2)
	{
		playerDamagedOverlay->index(i + 0);
		playerDamagedOverlay->index((i + 2) % 8);
		playerDamagedOverlay->index(i + 1);

		playerDamagedOverlay->index(i + 1);
		playerDamagedOverlay->index((i + 2) % 8);
		playerDamagedOverlay->index((i + 3) % 8);
	}

	playerDamagedOverlay->end();

	// Use infinite AAB to always stay visible
	AxisAlignedBox aabInf;
	aabInf.setInfinite();
	playerDamagedOverlay->setBoundingBox(aabInf);

	// Render just before overlays
	playerDamagedOverlay->setRenderQueueGroup(RENDER_QUEUE_OVERLAY - 1);
	playerDamagedOverlay->setVisible(false);

	// Attach to scene
	playerDamagedSceneNode = sceneMgr->getRootSceneNode()->createChildSceneNode();
	playerDamagedSceneNode->attachObject(playerDamagedOverlay);
}
void ComponentPlayer::destroyPlayerDamagedOverlay()
{
	playerDamagedSceneNode->detachAllObjects();
	sceneMgr->destroySceneNode(playerDamagedSceneNode->getName());
	sceneMgr->destroyManualObject(playerDamagedOverlay);
	playerDamagedMaterialPtr.setNull();
}
void ComponentPlayer::handlePlayerDamagedOverlay(float dt)
{
	if (playerDamagedBlend <= 0)
		playerDamagedOverlay->setVisible(false);
	else
	{
		playerDamagedBlend -= dt * PLAYER_DAMAGED_OVERLAY_FADE_SPEED;

		playerDamagedOverlay->setVisible(true);
		playerDamagedMaterialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(LBX_MODULATE, LBS_CURRENT, LBS_MANUAL, 0, playerDamagedBlend);
	}
}

void ComponentPlayer::playerStep(bool move, Vector3 direction, bool wantsToJump, float timeSinceLastFrame)
{
	// Call the corresponding character controller method
	myCharacterController->step(move, direction, wantsToJump, timeSinceLastFrame);
}

void ComponentPlayer::textEntryLineEnter()
{
	if (strncmp(textEntryLineText, "lua:", 4) == 0)
	{
		// Execute entered lua code
		game.gameConsole->addMessage(&textEntryLineText[4], -1.0f, "8679ff");
		
		if (luaL_dostring(objectScriptManager.masterState, &textEntryLineText[4]) != 0)
		{
			const char* msg;
	
			msg = lua_tostring(objectScriptManager.masterState, -1);
			if (msg == NULL)
				msg = "(error without message)";
			lua_pop(objectScriptManager.masterState, 1);
	
			LogManager::getSingleton().logMessage("SCRIPT ERROR:");
			LogManager::getSingleton().logMessage(msg);
	
			game.gameConsole->addMessage("SCRIPT ERROR:", -1.0f, "FF5555");
			game.gameConsole->addMessage(msg, -1.0f, "FF5555");
		}
	}
	else
	{
		// Send chat message to other players
		string message = settings.nickname + ": " + textEntryLineText;
		
		game.gameConsole->addMessage(message.c_str(), -1.0f, "d524d7");
		
		// TODO: Send the message
	}
	
	memset(textEntryLineText, 0, TEXT_ENTRY_LINE_MAX_LEN + 1);
	textEntryLineCursor = 0;
}
void ComponentPlayer::updateTextEntryLine()
{
	string displayText = "> ";
	displayText += textEntryLineText;
	textEntryLine.setText(displayText.c_str());
}
void ComponentPlayer::showTextEntryLine()
{
	// TODO: Better way to determine a good y position
	textEntryLine.create("", cegui_reldim(0.03f), cegui_reldim(0.63f) + cegui_absdim(8 * windowManager.getFont()->getLineSpacing()),
						 cegui_reldim(0.55f), cegui_reldim(0.63f) + cegui_absdim(9 * windowManager.getFont()->getLineSpacing()),
						 CEGUI::WindowManager::getSingleton().getWindow("HUDRoot"), "fdda19");
	updateTextEntryLine();
}
void ComponentPlayer::hideTextEntryLine()
{
	textEntryLine.destroy();
}
void ComponentPlayer::textEntryLineKeyEvent(const OIS::KeyEvent& evt)
{
	if (evt.key == OIS::KC_BACK)
	{
		if (textEntryLineCursor > 0)
		{
			--textEntryLineCursor;
			textEntryLineText[textEntryLineCursor] = 0;
		}
		
		updateTextEntryLine();
	}
	else if (evt.key == OIS::KC_LSHIFT || evt.key == OIS::KC_RSHIFT)
	{
		// do nothing
	}
	else
	{
		if (textEntryLineCursor < TEXT_ENTRY_LINE_MAX_LEN)
		{
			textEntryLineText[textEntryLineCursor] = (char)evt.text;
			++textEntryLineCursor;
		}
		
		updateTextEntryLine();
	}
}

// ### ComponentTemplatePlayer ###

NamedComponent* ComponentTemplatePlayer::createInstance(Object* object)
{
	if (game.mPlayer)
		return NULL;

	ComponentPlayer* newPlayer = new ComponentPlayer();

	object->listenTo = TRIGTYPE_TOUCH;

	newPlayer->weaponInHand.init(game.getCameraRollNode(), newPlayer);
	for (int i = 0; i < 3; i++)
		newPlayer->weapons[i].init(CEGUI::WindowManager::getSingleton().getWindow("HUDRoot"), 0.05f, 0.15f, 0.3f, (WeaponType)i);
	newPlayer->actWeaponInventory = 0;
	newPlayer->inventoryShowTime = -1;
	newPlayer->weaponPickedUp = 0;
	newPlayer->grenadePickedUp = false;
	newPlayer->toolPickedUp = false;
	newPlayer->isUsingATool = false;

	newPlayer->weaponControlsInfo.init();

	newPlayer->targetItem = NULL;
	newPlayer->targetName.create("", cegui_reldim(0.5f), cegui_reldim(0.5f) + cegui_absdim(26), cegui_reldim(0.5f), cegui_reldim(0.5f) + cegui_absdim(66), CEGUI::WindowManager::getSingleton().getWindow("HUDRoot"), "FFFFFF", TEXTALIGN_CENTER);

	newPlayer->textEntryLineCursor = 0;
	memset(newPlayer->textEntryLineText, 0, TEXT_ENTRY_LINE_MAX_LEN + 1);
	
	newPlayer->dontRespawn = false;

	// Enter the player component in the Game class
	game.mPlayer = newPlayer;
	game.setSubState(SUBST_PLAY);

	// Create the "player damaged" overlay
	ComponentPlayer::playerDamagedBlend = 0;
	ComponentPlayer::createPlayerDamagedOverlay();

	// Show HUD
	windowManager.showCursor(false);
	windowManager.setPage("HUD");
	// Show crosshair
	newPlayer->crosshair = OverlayManager::getSingleton().getByName("IngameOverlay");
	newPlayer->crosshair->show();

	// Set input handler
	inputManager->setInputHandler(&playerInputHandler);

	// Show a welcome message	TODO: to show or not to show?
	//game.gameConsole->addMessage("Hello Stranger, welcome to the world of war!\nUse [Esc] to quit, [W], [A], [S], [D] to move and [Space] to jump!", HINT_MESSAGE_DURATION, HINT_MESSAGE_COLOR);

	return newPlayer;
}
bool ComponentTemplatePlayer::init(TiXmlElement* params)
{
	return true;
}
void ComponentTemplatePlayer::exit()
{
	delete this;
}

// ### PlayerInputHandler ###

void PlayerInputHandler::update(float timeSinceLastFrame)
{
	rotX = 0;
	rotY = 0;

	// Handle mouse and keyboard
	handleMouse(timeSinceLastFrame);
	handleKeyboard(timeSinceLastFrame);

	if (!game.mPlayer)
		return;

	// Move player
	Vector3 forward = game.getCameraForward();
	if (!textEntryLineActive)
	{
		// Get movement direction
		Vector3 movement = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 right = game.getCameraRight();
		if (inputManager->isKeyDown(OIS::KC_W))	movement -= forward;
		if (inputManager->isKeyDown(OIS::KC_A))	movement -= right;
		if (inputManager->isKeyDown(OIS::KC_S))	movement += forward;
		if (inputManager->isKeyDown(OIS::KC_D))	movement += right;
	
		// Move
		game.mPlayer->playerStep(movement != Vector3(0.0f, 0.0f, 0.0f), movement, inputManager->isKeyDown(OIS::KC_SPACE), timeSinceLastFrame);
	}
	else
	{
		game.mPlayer->playerStep(false, Vector3::ZERO, false, timeSinceLastFrame);
	}

	game.moveCamera(rotX, rotY, Vector3::ZERO);
	game.mPlayer->object->sendMessage(COMPMSG_SET_AIM, &forward, NULL, game.mPlayer);
}

void PlayerInputHandler::handleMouse(float timeSinceLastFrame)
{
	const OIS::MouseState &ms = inputManager->mouseState;

	// Look around
	rotX = Degree(-ms.X.rel * settings.mouseSensitivity);
	rotY = Degree(-ms.Y.rel * settings.mouseSensitivity);

	// Fire weapons if mouse buttons held
	if (ms.buttonDown(OIS::MB_Left))
		game.mPlayer->weaponInHand.fire(0);
	if (ms.buttonDown(OIS::MB_Right))
		game.mPlayer->weaponInHand.fire(1);
}

bool PlayerInputHandler::mouseMoved(const OIS::MouseEvent &evt)
{
	int change = evt.state.Z.rel;
	if (change)
	{
		// TODO: <REFACTOR> very ugly stuff here
		ComponentPlayer* player = game.mPlayer;
		if (player->weapons[player->actWeaponInventory].weaponSet.size())
		{
			if (change >= 100)
			{
				if (player->weapons[player->actWeaponInventory].activeWeapon == player->weapons[player->actWeaponInventory].weaponSet.begin())
				{
					player->weapons[player->actWeaponInventory].activeWeapon = player->weapons[player->actWeaponInventory].weaponSet.begin();
					advance(player->weapons[player->actWeaponInventory].activeWeapon, player->weapons[player->actWeaponInventory].weaponSet.size() - 1);
				}
				else
					player->weapons[player->actWeaponInventory].activeWeapon--;
			}
			else if (change <= -100)
			{
				WeaponInventory::weaponSetItr it = player->weapons[player->actWeaponInventory].activeWeapon;
				it++;
				if (it == player->weapons[player->actWeaponInventory].weaponSet.end())
					player->weapons[player->actWeaponInventory].activeWeapon = player->weapons[player->actWeaponInventory].weaponSet.begin();
				else
					player->weapons[player->actWeaponInventory].activeWeapon++;
			}

			for (int i = 0; i < 3; i++)
				player->weapons[i].createList();
			player->showWeaponInventory(-1);
		}
	}

	return true;
}

bool PlayerInputHandler::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID btn)
{
	// Fire weapons if mouse buttons pressed
	if (btn == OIS::MB_Left)
		game.mPlayer->weaponInHand.fire(0);
	if (btn == OIS::MB_Right)
		game.mPlayer->weaponInHand.fire(1);

	return true;
}

bool PlayerInputHandler::keyPressed(const OIS::KeyEvent &evt)
{
	OIS::KeyCode ch = evt.key;
	ComponentPlayer* player = game.mPlayer;
	
	// Text entry line. If active, this has priority over all other keyboard presses, so it must come first
	if (!textEntryLineActive)
	{
		if (ch == OIS::KC_Y)
		{
			game.mPlayer->showTextEntryLine();
			textEntryLineActive = true;
			return true;
		}
	}
	else
	{
		// Enter text, close line with Esc, confirm with Enter
		if (ch == OIS::KC_ESCAPE)
		{
			game.mPlayer->hideTextEntryLine();
			textEntryLineActive = false;
		}
		else if (ch == OIS::KC_RETURN)
		{
			game.mPlayer->textEntryLineEnter();
			game.mPlayer->hideTextEntryLine();
			textEntryLineActive = false;
		}
		else
			game.mPlayer->textEntryLineKeyEvent(evt);
		
		return true;
	}
	
	GameInputHandler::handleCommonKeyboardEvents(evt);
	
	// Switch to sandbox mode
	if (ch == OIS::KC_P)
	{
		if (game.mPlayer)
		{
			game.mPlayer->dontRespawn = true;
			objectManager.deleteObject(game.mPlayer->object);
		}
		return true;
	}

	// Select weapon inventory
	switch (player->actWeaponInventory)
	{
	case WEAPONTYPE_NORMAL:
		if (ch == OIS::KC_T)
			player->showWeaponInventory(WEAPONTYPE_TOOL);
		else if (ch == OIS::KC_G)
			player->showWeaponInventory(WEAPONTYPE_GRENADE);
	break;
	case WEAPONTYPE_TOOL:
		if (ch == OIS::KC_T)
			player->showWeaponInventory(WEAPONTYPE_NORMAL);
		else if (ch == OIS::KC_G)
			player->showWeaponInventory(WEAPONTYPE_GRENADE);
	break;
	case WEAPONTYPE_GRENADE:
		if (ch == OIS::KC_T)
			player->showWeaponInventory(WEAPONTYPE_TOOL);
		else if (ch == OIS::KC_G)
			player->showWeaponInventory(WEAPONTYPE_NORMAL);
	break;
	}

	// Throw grenades
	if (ch == OIS::KC_LSHIFT)
		player->throwGrenade();

	return true;
}

void PlayerInputHandler::handleKeyboard(float timeSinceLastFrame)
{
	// Show Networkplayer list
	#ifdef USE_RAKNET
  CEGUI::FrameWindow * Userlist_Box = (CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Userlist_Box");
  Userlist_Box->setVisible(inputManager->isKeyDown(OIS::KC_TAB));

	static float userlist_update_delay = 0;
	userlist_update_delay -= timeSinceLastFrame;
	if (userlist_update_delay <= 0)
	{
	  CEGUI::Listbox* Userlist = (CEGUI::Listbox*)CEGUI::WindowManager::getSingleton().getWindow("Userlist");
	  Userlist->resetList();
	  // If there's no me there's no others
	  if(game.mNetwork->me) {
		  Userlist->addItem(new MyListItem("Clients: " + boost::lexical_cast<string>(game.mNetwork->users.size() + 1)));
			Userlist->addItem(new MyListItem(game.mNetwork->me->nick + " <--", (void*)0));
		  for (Userlist::iterator i = game.mNetwork->users.begin(); i!= game.mNetwork->users.end(); i++) {
			  string name = (*i).second->nick;
				Userlist->addItem(new MyListItem(name, (void*)0));
		  }
	  } else {
		  Userlist->addItem(new MyListItem("No Clients"));
	  }
		userlist_update_delay = 0.5f;
	}
	#endif
}
