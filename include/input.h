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

#ifndef _INPUT_H_
#define _INPUT_H_

#include "OISEvents.h"
#include "OISInputManager.h"
#include "OISMouse.h"
#include "OISKeyboard.h"
#include "OISJoyStick.h"

namespace CEGUI
{
	class System;
}

class InputHandler : public OIS::MouseListener, public OIS::KeyListener, public OIS::JoyStickListener
{
public:

	/// Can be used to query some input states directly
	virtual void update(float timeSinceLastFrame) {};

	// MouseListener
	virtual bool mouseMoved(const OIS::MouseEvent &evt) {return true;};
	virtual bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID btn) {return true;};
	virtual bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID btn) {return true;};

	// KeyListener
	virtual bool keyPressed(const OIS::KeyEvent &evt) {return true;};
	virtual bool keyReleased(const OIS::KeyEvent &evt) {return true;};

	// JoyStickListener
	virtual bool buttonPressed(const OIS::JoyStickEvent &evt, int index) {return true;};
	virtual bool buttonReleased(const OIS::JoyStickEvent &evt, int index) {return true;};
	virtual bool axisMoved(const OIS::JoyStickEvent &evt, int index) {return true;};
	virtual bool povMoved(const OIS::JoyStickEvent &evt, int index) {return true;};
};

/// Deals with OIS. Sends all input to the current InputHandler. Pseudo-Singleton: use the global variable inputManager
class InputManager
{
public:

	/// Mouse state information, can be accessed by input handlers
	OIS::MouseState mouseState;

	/// If false, input handlers should not process mouseMoved and mouseReleased events
	bool processMouse;

	/// Initializes OIS in the given window
	InputManager(unsigned long hWnd, bool exclusive);
	/// Deinitializes OIS
	~InputManager();

	/// Sets a new input handler
	void setInputHandler(InputHandler* newHandler);

	/// Centers the mouse in the window
	void centerMouse();
	/// Sets a new window size
	void setWindowExtents(int width, int height);
	/// Gets new input data. Must be called every frame.
	void capture(float timeSinceLastFrame);

	/// Returns if the specified key is currently pressed
	inline bool isKeyDown(OIS::KeyCode key)
	{
		return mKeyboard->isKeyDown(key);
	}

	/// Returns if the specified mouse button is currently pressed
	inline bool isMouseDown(OIS::MouseButtonID button)
	{
		return mouseState.buttonDown(button);
	}
	
protected:

	OIS::InputManager *m_ois;
	OIS::Mouse *mMouse;
	OIS::Keyboard *mKeyboard;
	unsigned long m_hWnd;

	InputHandler* handler;
};

extern InputManager* inputManager;

#endif
