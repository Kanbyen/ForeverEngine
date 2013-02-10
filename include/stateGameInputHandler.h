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

#ifndef _STATE_GAME_INPUT_HANDLER_H_
#define _STATE_GAME_INPUT_HANDLER_H_

#include "OgreVector3.h"
using namespace Ogre;
#include "input.h"
#include "ceguiInputHandler.h"

const float cameraMoveSpeed = 25;

/// Input Handler for the "Sandbox" mode. Pseudo-Singleton: use the global variable gameInputHandler
class GameInputHandler : public CEGUIInputHandler
{
public:

	/// If yes, don't process keyboard events because the user enters something in an input field
	bool guiInputMode;

	virtual void update(float timeSinceLastFrame);

	/// Handle keyboard events which are posible as player and in sandbox mode
	static void handleCommonKeyboardEvents(const OIS::KeyEvent& evt);

protected:

	void handleMouse(float timeSinceLastFrame);
	void handleKeyboard(float timeSinceLastFrame);

    virtual bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID btn);
	virtual bool keyPressed(const OIS::KeyEvent &evt);

	/// Camera movement
	Vector3 translateVector;
	Radian rotX, rotY;
	float moveScale;
};

extern GameInputHandler gameInputHandler;

#endif
