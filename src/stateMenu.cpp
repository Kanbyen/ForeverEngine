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
#include "stateManager.h"
#include "stateMenu.h"
#include "windowManager.h"
#include "stateGame.h"
#include "settings.h"
#include "network.h"

Menu menu;

#define event_subscribe(window, eventName, method, className) { \
	wnd = CEGUI::WindowManager::getSingleton().getWindow(window); \
	wnd->subscribeEvent(eventName, CEGUI::Event::Subscriber(method, className)); \
	}

#define edit_subscribe(window) { \
	wnd = CEGUI::WindowManager::getSingleton().getWindow(window); \
	wnd->subscribeEvent(CEGUI::Editbox::EventActivated, CEGUI::Event::Subscriber(&Game::activateGUIInputMode, &game)); \
	wnd->subscribeEvent(CEGUI::Editbox::EventDeactivated, CEGUI::Event::Subscriber(&Game::deactivateGUIInputMode, &game)); \
	wnd->subscribeEvent(CEGUI::Editbox::EventKeyDown, CEGUI::Event::Subscriber(&Game::deactivateGUIInputModeWithEnter, &game)); \
	}

bool Menu::frameStarted(const FrameEvent& evt)
{
	inputManager->capture(evt.timeSinceLastFrame);

	const OIS::MouseState &ms = inputManager->mouseState;
	CEGUI::MouseCursor::getSingleton().setPosition(CEGUI::Point(ms.X.abs, ms.Y.abs));

	return true;
}

void Menu::setup()
{
	CEGUI::Listbox* box;
	CEGUI::Window* wnd;

	// Main Menu
	event_subscribe("MP_Quit", CEGUI::PushButton::EventClicked, &Menu::MP_Quit_OnClick, this);
	event_subscribe("MP_Host", CEGUI::PushButton::EventClicked, &Menu::MP_Host_OnClick, this);
	event_subscribe("MP_Join", CEGUI::PushButton::EventClicked, &Menu::MP_Join_OnClick, this);
	event_subscribe("MP_Sandbox", CEGUI::PushButton::EventClicked, &Menu::MP_Sandbox_OnClick, this);
	
	event_subscribe("MP_Player_Edit", CEGUI::Editbox::EventKeyUp, &Menu::MP_Player_Changed, this);
	event_subscribe("MP_Server_Edit", CEGUI::Editbox::EventKeyUp, &Menu::MP_Server_Changed, this);
	
	// Game Start
	event_subscribe("GameStartChat", CEGUI::Editbox::EventKeyDown, &Game::gameStart_ChatKeyDown, &game);

	// Editor
	event_subscribe("Generate_Cube", CEGUI::PushButton::EventClicked, &Game::click_generate, &game);
	event_subscribe("Generate_Sphere", CEGUI::PushButton::EventClicked, &Game::click_generate, &game);
	event_subscribe("Generate_PerlinNoise", CEGUI::PushButton::EventClicked, &Game::click_generate, &game);
	event_subscribe("Generate_Planes", CEGUI::PushButton::EventClicked, &Game::click_generate, &game);
	event_subscribe("Generate_Terrain", CEGUI::PushButton::EventClicked, &Game::click_generate, &game);
	event_subscribe("Generate_PerlinNoiseCave", CEGUI::PushButton::EventClicked, &Game::click_generate, &game);
	event_subscribe("Generate_Invert", CEGUI::PushButton::EventClicked, &Game::click_generate, &game);
	event_subscribe("Generate_ForeverWar", CEGUI::PushButton::EventClicked, &Game::click_generate, &game);
	event_subscribe("Generate_ForeverWarMini", CEGUI::PushButton::EventClicked, &Game::click_generate, &game);
	
	event_subscribe("SpawnObject", CEGUI::PushButton::EventClicked, &Game::spawnObject, &game);
	event_subscribe("DeleteObject", CEGUI::PushButton::EventClicked, &Game::deleteObject, &game);
	event_subscribe("RunSimulation", CEGUI::Checkbox::EventCheckStateChanged, &Game::toggleRunSimulation, &game);
	
	event_subscribe("MoveWindowPX", CEGUI::PushButton::EventClicked, &Game::moveWindowPX, &game);
	event_subscribe("MoveWindowMX", CEGUI::PushButton::EventClicked, &Game::moveWindowMX, &game);
	event_subscribe("MoveWindowPY", CEGUI::PushButton::EventClicked, &Game::moveWindowPY, &game);
	event_subscribe("MoveWindowMY", CEGUI::PushButton::EventClicked, &Game::moveWindowMY, &game);
	event_subscribe("MoveWindowPZ", CEGUI::PushButton::EventClicked, &Game::moveWindowPZ, &game);
	event_subscribe("MoveWindowMZ", CEGUI::PushButton::EventClicked, &Game::moveWindowMZ, &game);
	event_subscribe("AutomaticPaging", CEGUI::Checkbox::EventCheckStateChanged, &Game::toggleAutomaticPaging, &game);
	event_subscribe("PagingModeOpen", CEGUI::RadioButton::EventSelectStateChanged, &Game::togglePagingMode, &game);
	event_subscribe("PagingModeClosed", CEGUI::RadioButton::EventSelectStateChanged, &Game::togglePagingMode, &game);
	
	event_subscribe("Save_Voxel_Data", CEGUI::PushButton::EventClicked, &Game::saveVoxelData, &game);
	event_subscribe("Load_Voxel_Data", CEGUI::PushButton::EventClicked, &Game::loadVoxelData, &game);
	event_subscribe("Save_Object_Data", CEGUI::PushButton::EventClicked, &Game::saveObjectData, &game);
	event_subscribe("Load_Object_Data", CEGUI::PushButton::EventClicked, &Game::loadObjectData, &game);
	event_subscribe("Load_All_Data", CEGUI::PushButton::EventClicked, &Game::loadAllData, &game);
	
	edit_subscribe("BrushSizeEdit");
	edit_subscribe("BrushTextureEdit");
	edit_subscribe("AutotexTopEdit");
	edit_subscribe("AutotexSideEdit");
	edit_subscribe("AutotexInner1Edit");
	edit_subscribe("AutotexInner2Edit");
	edit_subscribe("VolumeScaleEdit");

	box = (CEGUI::Listbox*)CEGUI::WindowManager::getSingleton().getWindow("BrushShapeEdit");
	box->addItem(new MyListItem("sphere", (void*)0));
	box->addItem(new MyListItem("box", (void*)1));
	box->setItemSelectState((size_t)0, true);

	box = (CEGUI::Listbox*)CEGUI::WindowManager::getSingleton().getWindow("BrushOpEdit");
	box->addItem(new MyListItem("hard", (void*)&Game::editLevel_Hard));
	box->addItem(new MyListItem("smooth", (void*)&Game::editLevel_Smooth));
	box->addItem(new MyListItem("smooth (2)", (void*)&Game::editLevel_Smooth2));
	box->addItem(new MyListItem("average", (void*)&Game::editLevel_Average));
	box->addItem(new MyListItem("texturize", (void*)&Game::editLevel_Texturize));
	box->addItem(new MyListItem("texturize surface", (void*)&Game::editLevel_TexturizeSurface));
	box->setItemSelectState((size_t)0, true);
}

void Menu::init(State* lastState)
{
	OverlayManager::getSingleton().getByName("LoadingOverlay")->hide();
	
	// Register ourself as frame listener
	ogre->addFrameListener(&menu);
	// Register correct input handler
	inputManager->setInputHandler(&ceguiInputHandler);

	// Set GUI page
	windowManager.setPage("Menu");

	// Dummy input to show mouse cursor
	windowManager.showMouseCursorHack();
	
	// Get widgets, set text
	playerEdit = (CEGUI::Editbox*)CEGUI::WindowManager::getSingleton().getWindow("MP_Player_Edit");
	serverEdit = (CEGUI::Editbox*)CEGUI::WindowManager::getSingleton().getWindow("MP_Server_Edit");
	
	playerEdit->setText(settings.nickname);
	serverEdit->setText(settings.lastServer);
}

void Menu::exit(State* nextState)
{
	ogre->removeFrameListener(&menu);
}

bool Menu::MP_Quit_OnClick(const CEGUI::EventArgs &args)
{
	stateManager.changeState(NULL);
	return true;
}
bool Menu::MP_Host_OnClick(const CEGUI::EventArgs &args)
{
	#ifndef DIGISPE
	delete network;	// TODO: do this when exiting the game!
	network = new Network();
	if (!network->isOk())
	{
		delete network;
		return true;	// TODO: display error
	}
	if (!network->host())
	{
		delete network;
		return true;	// TODO: display error
	}
	
	settings.gameMode = "dm";
	stateManager.changeState(&game);
	game.setSubState(SUBST_GAMESTART);
	#endif
	
	return true;
}
bool Menu::MP_Join_OnClick(const CEGUI::EventArgs &args)
{
	#ifndef DIGISPE
	delete network;	// TODO: do this when exiting the game!
	network = new Network();
	if (!network->isOk())
	{
		delete network;
		return true;	// TODO: display error
	}
	if (!network->join(settings.lastServer.c_str()))
	{
		delete network;
		return true;	// TODO: display error
	}
	
	settings.gameMode = "dm";
	stateManager.changeState(&game);
	game.setSubState(SUBST_GAMESTART);
	#endif
	
	return true;
}
bool Menu::MP_Sandbox_OnClick(const CEGUI::EventArgs &args)
{
	settings.gameMode = "sandbox";
	stateManager.changeState(&game);
	return true;
}
bool Menu::MP_Player_Changed(const CEGUI::EventArgs &args)
{
	settings.nickname = playerEdit->getText().c_str();
	return true;
}
bool Menu::MP_Server_Changed(const CEGUI::EventArgs &args)
{
	settings.lastServer = serverEdit->getText().c_str();
	return true;
}
