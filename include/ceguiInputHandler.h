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

#ifndef _CEGUI_INPUT_HANDLER_H_
#define _CEGUI_INPUT_HANDLER_H_

#include "input.h"

/// Handles CEGUI related input events. These function should be called by other input handlers which need to deal with CEGUI input. Recommended: inherit from this class
class CEGUIInputHandler : public InputHandler
{
public:

	// MouseListener
	virtual bool mouseMoved(const OIS::MouseEvent &evt);
	virtual bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID btn);
	virtual bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID btn);

	// KeyListener
	virtual bool keyPressed(const OIS::KeyEvent &evt);
	virtual bool keyReleased(const OIS::KeyEvent &evt);
};

#endif
