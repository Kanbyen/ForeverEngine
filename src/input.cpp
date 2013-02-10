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
#include "input.h"
#include "windowManager.h"
#include "stateManager.h"
#include "stateGame.h"
#include "CEGUISystem.h"

InputManager* inputManager;

InputManager::InputManager(unsigned long hWnd, bool exclusive)
	: handler(NULL)
{
	OIS::ParamList pl;
	pl.insert(OIS::ParamList::value_type("WINDOW", StringConverter::toString(hWnd)));

	if (!exclusive)
	{
		//Setup OIS in non-exclusive mode
		#if defined OIS_WIN32_PLATFORM
			pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND")));
			pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
			pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
			pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
		#elif defined OIS_LINUX_PLATFORM
			pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
			//pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
			pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
			pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
		#endif
	}

	m_hWnd = hWnd;
	m_ois = OIS::InputManager::createInputSystem(pl);
	mMouse = static_cast<OIS::Mouse*>(m_ois->createInputObject(OIS::OISMouse, true));
	mMouse->setBuffered(true);
	
	mKeyboard = static_cast<OIS::Keyboard*>(m_ois->createInputObject(OIS::OISKeyboard, true));
	mKeyboard->setBuffered(true);

	processMouse = true;
}

InputManager::~InputManager()
{
    m_ois->destroyInputObject(mMouse);
    m_ois->destroyInputObject(mKeyboard);
	OIS::InputManager::destroyInputSystem(m_ois);
}

void InputManager::setInputHandler(InputHandler* newHandler)
{
	handler = newHandler;
	if (handler)
	{
		mMouse->setEventCallback(handler);
		mKeyboard->setEventCallback(handler);
	}
}

void InputManager::centerMouse()
{
	OIS::MouseState& ms = (OIS::MouseState&)mMouse->getMouseState();
	ms.X.abs = ms.width / 2;
	ms.Y.abs = ms.height / 2;
}

void InputManager::capture(float timeSinceLastFrame)
{
	mMouse->capture();
	mKeyboard->capture();

	mouseState = mMouse->getMouseState();

	if (handler)
		handler->update(timeSinceLastFrame);
}

void InputManager::setWindowExtents(int width, int height)
{
	// Set Mouse Region.. if window resizes, we should alter this to reflect as well
	const OIS::MouseState& ms = mMouse->getMouseState();
	ms.width = width;
	ms.height = height;
}
