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

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <list>
#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include <CEGUI.h>
using namespace Ogre;
#include "util.h"

struct ConsoleMessage
{
public:
	StaticText wnd;
	float expireTime;
	float creationTime;
	int numLines;

	inline void setAlpha(float alpha)
	{
		wnd.setAlpha(alpha);
	}
};

/// Displays messages on the screen
class Console
{
public:
	/// Parent window for all messages
	CEGUI::Window* root;

	float startX;
	float startY;
	float actY;
	int linesShown;
	int maxLines;
	float messageDuration;
	float fadeTime;
	float currentTime;
	bool hasBorder;
	TextAlignment alignment;
	int direction;

	std::list<ConsoleMessage> msgList;
	typedef std::list<ConsoleMessage>::iterator msgListItr;

	/// Constructor
	Console(CEGUI::Window* root, float startX, float startY, int maxLines, float messageDuration, float fadeTime, TextAlignment alignment = TEXTALIGN_LEFT, bool bottomUp = false, bool hasBorder = false);

	/// Destructor, cleans up
	~Console();

	/// Adds a new message to the console
	void addMessage(string text, float duration = -1.0f, string color = "FFFFFF");

	/// Must be called every frame with the time passed since the last update.
	void update(float timepassed);
};

#endif
