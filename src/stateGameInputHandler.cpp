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
#include "stateGameInputHandler.h"
#include "stateGame.h"
#include "stateManager.h"
#include "windowManager.h"
#include "settings.h"
#include "object.h"

GameInputHandler gameInputHandler;

void GameInputHandler::update(float timeSinceLastFrame)
{
	rotX = 0;
	rotY = 0;
	translateVector = Vector3::ZERO;
	moveScale = cameraMoveSpeed * timeSinceLastFrame;

	// Handle mouse and keyboard
	handleMouse(timeSinceLastFrame);
	handleKeyboard(timeSinceLastFrame);

	// Move camera
	if (stateManager.getActState() == &game)
		game.moveCamera(rotX, rotY, translateVector);
}

void GameInputHandler::handleMouse(float timeSinceLastFrame)
{
	const OIS::MouseState &ms = inputManager->mouseState;

	static float delay = 0.0f;
	delay -= timeSinceLastFrame;
	
	if (game.getSubState() != SUBST_EDITOR)
	{
		windowManager.showCursor(game.getSubState() == SUBST_GAMESTART);
		return;
	}

	if (!inputManager->isMouseDown(OIS::MB_Middle))
	{
		windowManager.showCursor(true);

		// Edit geometry with left and right clicks
		CEGUI::TabControl* tabWindow = (CEGUI::TabControl*)CEGUI::WindowManager::getSingleton().getWindow("EditorTabControl");
		if (inputManager->processMouse && (tabWindow->getSelectedTabIndex() == 1) && (delay <= 0) && (inputManager->isMouseDown(OIS::MB_Left) || inputManager->isMouseDown(OIS::MB_Right)))
		{
			Ray r = camera->getCameraToViewportRay(CEGUI::MouseCursor::getSingleton().getPosition().d_x / settings.resolution_Width,
														 CEGUI::MouseCursor::getSingleton().getPosition().d_y / settings.resolution_Height);
			Vector3 pos = camera->getDerivedPosition();
			Vector3 dir = r.getDirection();
			dir.normalise();

			game.editLevel(inputManager->isMouseDown(OIS::MB_Left), pos, dir);
			delay = 0.12f;
		}
	}
	else
	{
		windowManager.showCursor(false);

		rotX = Degree(-ms.X.rel * settings.mouseSensitivity);
		rotY = Degree(-ms.Y.rel * settings.mouseSensitivity);
	}
}

bool GameInputHandler::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID btn)
{
	if (game.getSubState() != SUBST_EDITOR && game.getSubState() != SUBST_GAMESTART)
		return true;
	
	CEGUIInputHandler::mousePressed(evt, btn);
	
	if (game.getSubState() != SUBST_EDITOR)
		return true;
	
	CEGUI::TabControl* tabWindow = (CEGUI::TabControl*)CEGUI::WindowManager::getSingleton().getWindow("EditorTabControl");
	if (inputManager->processMouse && (tabWindow->getSelectedTabIndex() == 2))
	{
		if (objectManager.getEditObjects() && objectManager.isEditModeActive())
		{
			if (btn == OIS::MB_Left)
				objectManager.finishEditing();
			else if (btn == OIS::MB_Right)
				objectManager.abortEditing();
		}
		else
		{
			// Select objects
			if (btn == OIS::MB_Left)
				objectManager.selectObjectBelowCursor();
		}
	}
	
	return true;
}
bool GameInputHandler::keyPressed(const OIS::KeyEvent &evt)
{
	CEGUIInputHandler::keyPressed(evt);
	
	handleCommonKeyboardEvents(evt);
	
	if (game.getSubState() != SUBST_EDITOR)
		return true;
	
	OIS::KeyCode ch = evt.key;

	if (ch == OIS::KC_P)
	{
		// Switch to player mode
		game.createPlayer(camera->getDerivedPosition());
	}
	else if (ch == OIS::KC_I)
	{
		// Blend interface in/out
		CEGUI::Window* guiPage = windowManager.getPage("Editor");
		if (guiPage->isVisible())
			guiPage->hide();
		else
			guiPage->show();
	}
	else if (ch == OIS::KC_M)
	{
		// Output camera position and orientation to log
		/*string str = "INFO:   position=\"" + StringConverter::toString(game.cameraNode->getPosition()) + "\"" +
					" yawOrientation=\"" + StringConverter::toString(game.cameraYawNode->getOrientation()) + "\"" +
					" pitchOrientation=\"" + StringConverter::toString(game.cameraPitchNode->getOrientation()) + "\"";
		*/
		string str = "INFO:   position=\"" + StringConverter::toString(game.cameraNode->getPosition()) + "\"" +
		" yaw=\"" + StringConverter::toString(game.cameraYawNode->getOrientation().getYaw(false)) + "\"" +
		" pitch=\"" + StringConverter::toString(game.cameraPitchNode->getOrientation().getPitch(false)) + "\"" +
		" roll=\"" + StringConverter::toString(game.cameraRollNode->getOrientation().getRoll(false)) + "\"";
		
		LogManager::getSingleton().logMessage(str.c_str(), LML_CRITICAL);
	}
	else if (ch == OIS::KC_E)
	{
		// Place objects
		game.spawnObject(CEGUI::EventArgs());
	}
	else if (evt.key == OIS::KC_O)
	{
		// Toggle polygon mode
		camera->setPolygonMode((PolygonMode)((camera->getPolygonMode() % 3) + 1));
		if (camera->getPolygonMode() == PM_SOLID)
			vp->setClearEveryFrame(true, FBT_DEPTH);
		else
			vp->setClearEveryFrame(true, FBT_COLOUR | FBT_DEPTH);
	}
	
	if (objectManager.getEditObjects())
	{
		if (ch == OIS::KC_G)
			objectManager.setEditMode(EDITMODE_GRAB);
		else if (ch == OIS::KC_R)
			objectManager.setEditMode(EDITMODE_ROTATE);
		else if (ch == OIS::KC_T)
			objectManager.setEditMode(EDITMODE_SCALE);
		else if (ch == OIS::KC_X)
			objectManager.setEditDirection(Vector3(1, 0, 0));
		else if (ch == OIS::KC_Y)
			objectManager.setEditDirection(Vector3(0, 1, 0));
		else if (ch == OIS::KC_Z)
			objectManager.setEditDirection(Vector3(0, 0, 1));
	}

	return true;
}

void GameInputHandler::handleKeyboard(float timeSinceLastFrame)
{
	if (guiInputMode)
		return;
	if (game.getSubState() != SUBST_EDITOR)
		return;

	// Set brush size
	float oldBrushSize = game.brushSize;
	if (inputManager->isKeyDown(OIS::KC_1))
		game.brushSize = 2;
	else if (inputManager->isKeyDown(OIS::KC_2))
		game.brushSize = 3;
	else if (inputManager->isKeyDown(OIS::KC_3))
		game.brushSize = 5;
	else if (inputManager->isKeyDown(OIS::KC_4))
		game.brushSize = 7;
	else if (inputManager->isKeyDown(OIS::KC_5))
		game.brushSize = 8;
	else if (inputManager->isKeyDown(OIS::KC_6))
		game.brushSize = 10;
	else if (inputManager->isKeyDown(OIS::KC_7))
		game.brushSize = 12;
	else if (inputManager->isKeyDown(OIS::KC_8))
		game.brushSize = 14;
	else if (inputManager->isKeyDown(OIS::KC_9))
		game.brushSize = 16;

	if (oldBrushSize != game.brushSize)
		game.setNewBrushSize();

	// Move camera upwards along to world's Y-axis.
	if (inputManager->isKeyDown(OIS::KC_SPACE))
		translateVector.y += moveScale;

	// Move camera downwards along to world's Y-axis.
	if (inputManager->isKeyDown(OIS::KC_LCONTROL))
		translateVector.y += -(moveScale);

	// Move camera forward.
	if (inputManager->isKeyDown(OIS::KC_W))
		translateVector.z += -(moveScale);

	// Move camera backward.
	if (inputManager->isKeyDown(OIS::KC_S))
		translateVector.z += moveScale;

	// Move camera left.
	if (inputManager->isKeyDown(OIS::KC_A))
		translateVector.x += -(moveScale);

	// Move camera right.
	if (inputManager->isKeyDown(OIS::KC_D))
		translateVector.x += moveScale;
}

void GameInputHandler::handleCommonKeyboardEvents(const OIS::KeyEvent& evt)
{
	// Exit program
	if (evt.key == OIS::KC_ESCAPE)
	{
		stateManager.changeState(NULL);
		return;
	}

	#ifdef _DEBUG
		if (evt.key == OIS::KC_BACK)
		{
			debugDelay = 0.4f;
			const RenderTarget::FrameStats& stats = window->getStatistics();
			gameConsole->addMessage("Tri count: " + StringConverter::toString(stats.triangleCount));
			gameConsole->addMessage("Batch count: " + StringConverter::toString(stats.batchCount));
		}
	#endif

	// Make screenshots
	static int numScreenShots = 0;
	if (evt.key == OIS::KC_SYSRQ)
	{
		std::ostringstream ss;
		ss << "screenshot_" << ++numScreenShots << ".png";
		window->writeContentsToFile(ss.str());
	}
}
