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

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include <CEGUI.h>

#ifndef _UTIL_H_
#define _UTIL_H_

// util.h - a collection of useful misc functions

/// Returns the number of lines in the given text
int getNumberOfLines(const char* text);

/// Load data with a replacement if there is no data
void attrCpy(char* dest, const char* src, const char* replacement = NULL);
bool bAttrCpy(const char* src, bool replacement);
void strAttrCpy(string& dest, const char* src, const char* replacement = NULL);

/// Sleeps for the specified number of milliseconds
inline void sleep(int p_milliseconds)
{
	#ifdef WIN32
		Sleep( p_milliseconds );
	#else
		usleep( p_milliseconds * 1000 );
	#endif
}

/// Changes '^' to '\n' in the input string
string make_newlines(string in);

/// SmoothCD function from Game Programming Gems 4
template <typename T> inline T smoothInterpolate(const T &from, const T &to, T &vel, float smoothTime, float dt)
{
	float omega = 2.f/smoothTime;
	float x = omega*dt;
	float exp = 1.f/(1.f+x+0.48f*x*x+0.235f*x*x*x);
	T change = from - to;
	T temp = (vel+omega*change)*dt;
	vel = (vel - omega*temp)*exp;  // Equation 5
	return to + (change+temp)*exp; // Equation 4
}

const float JITTERED_TIMER_INTERVAL = 2.0f / 60.0f;
/// Timer which varies its interval slightly so updates of multiple timers don't occur in the same frame all the time
class JitteredIntervalTimer
{
public:
	JitteredIntervalTimer(float interval);
	bool isReady(float dt);

	float interval;
	float timeUntilNextActivation;
};

enum TextAlignment
{
	TEXTALIGN_LEFT,
 	TEXTALIGN_CENTER,
  	TEXTALIGN_RIGHT,
};

/// Creates CEGUI static texts, optionally with shadow
class StaticText
{
public:
	CEGUI::Window* textWnd;
	CEGUI::Window* shadow;

	StaticText();

	void setAlpha(float alpha);

	CEGUI::UDim getXPosition();
	void setXPosition(CEGUI::UDim x);
	CEGUI::UDim getYPosition();
	void setYPosition(CEGUI::UDim y);

	void setWidth(CEGUI::UDim width);
	void setHeight(CEGUI::UDim height);

	void setArea(const CEGUI::UDim& xpos, const CEGUI::UDim& ypos, const CEGUI::UDim& width, const CEGUI::UDim& height);
	void setText(const char* text);
	inline CEGUI::String getText()
	{
		return textWnd->getText();
	}

	void create(string text, CEGUI::UDim x, CEGUI::UDim y, CEGUI::UDim x2, CEGUI::UDim y2, CEGUI::Window* parent, string color = "FFFFFF", TextAlignment alignment = TEXTALIGN_LEFT, bool hasShadow = true, bool hasBorder = false);
	void destroy();
};

#endif
