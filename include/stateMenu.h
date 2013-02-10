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

#ifndef _STATE_MENU_H_
#define _STATE_MENU_H_

#include "stateManager.h"
#include "CEGUIWindow.h"
#include "ceguiInputHandler.h"

namespace CEGUI
{
	class System;
	class Window;
}

class State;

class Menu : public FrameListener, public State
{
public:
	void init(State* lastState);
	void exit(State* nextState);

	void setup();

	bool frameStarted(const Ogre::FrameEvent& evt);

	// CEGUI event handlers
	bool MP_Quit_OnClick(const CEGUI::EventArgs &args);
	bool MP_Host_OnClick(const CEGUI::EventArgs &args);
	bool MP_Join_OnClick(const CEGUI::EventArgs &args);
	bool MP_Sandbox_OnClick(const CEGUI::EventArgs &args);
	
	bool MP_Player_Changed(const CEGUI::EventArgs &args);
	bool MP_Server_Changed(const CEGUI::EventArgs &args);

protected:
	
	CEGUI::Editbox* playerEdit;
	CEGUI::Editbox* serverEdit;

	CEGUIInputHandler ceguiInputHandler;
};

extern Menu menu;

#endif
