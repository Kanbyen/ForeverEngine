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
#include "ceguiInputHandler.h"
#include "windowManager.h"

// MouseListener
bool CEGUIInputHandler::mouseMoved(const OIS::MouseEvent &evt)
{
	if (evt.state.Z.rel)
		windowManager.pSystem->injectMouseWheelChange((evt.state.Z.rel > 0) ? 6 : -6);
	else
		windowManager.pSystem->injectMousePosition(evt.state.X.abs, evt.state.Y.abs);
	
	return true;
}

bool CEGUIInputHandler::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID btn)
{
	CEGUI::MouseButton button = CEGUI::NoButton;

	if (btn == OIS::MB_Left)
		button = CEGUI::LeftButton;
	if (btn == OIS::MB_Middle)
		button = CEGUI::MiddleButton;
	if (btn == OIS::MB_Right)
		button = CEGUI::RightButton;

	bool handledByCEGUI = windowManager.pSystem->injectMouseButtonDown(button);
	if (handledByCEGUI)
		inputManager->processMouse = false;

	return true;
}

bool CEGUIInputHandler::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID btn)
{
	CEGUI::MouseButton button = CEGUI::NoButton;

	if (btn == OIS::MB_Left)
		button = CEGUI::LeftButton;
	if (btn == OIS::MB_Middle)
		button = CEGUI::MiddleButton;
	if (btn == OIS::MB_Right)
		button = CEGUI::RightButton;

	inputManager->processMouse = true;

	windowManager.pSystem->injectMouseButtonUp(button);
	return true;
}

// KeyListener
bool CEGUIInputHandler::keyPressed(const OIS::KeyEvent &evt)
{
	windowManager.pSystem->injectKeyDown(evt.key);
	windowManager.pSystem->injectChar(evt.text);
	return true;
}

bool CEGUIInputHandler::keyReleased(const OIS::KeyEvent &evt)
{
	windowManager.pSystem->injectKeyUp(evt.key);
	return true;
}
