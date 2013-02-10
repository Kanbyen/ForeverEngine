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
#include "util.h"

int getNumberOfLines(const char* text)
{
	int lines = 1;
	
	// Jedes Zeichen durchgehen
	int i;
	for(i = 0; text[i] != 0; ++i)
	{
		if (text[i] == '\n')
			++lines;
	}
	
	if (i == 0)
		lines = 0;
	
	return lines;
}

void strAttrCpy(string& dest, const char* src, const char* replacement)
{
	if (src)
		dest = src;
	else if (replacement)
		dest = replacement;
	else
		throw Exception(0, "required attribute missing in xml file", "strAttrCpy");
}
void attrCpy(char* dest, const char* src, const char* replacement)
{
	if (src)
		strcpy(dest, src);
	else if (replacement)
		strcpy(dest, replacement);
	else
		throw Exception(0, "required attribute missing in xml file", "attrCpy");
}
bool bAttrCpy(const char* src, bool replacement)
{
	if (src)
		return !stricmp(src, "true");
	else
		return replacement;
}

string make_newlines(string in)
{
	size_t i = 0;
	while (i != in.npos)
	{
		i = in.find('^', i);
		if (i != in.npos)
			in[i] = '\n';
	}
	return in;
}

JitteredIntervalTimer::JitteredIntervalTimer(float interval)
{
	this->interval = interval;
	timeUntilNextActivation = 0;
}
bool JitteredIntervalTimer::isReady(float dt)
{
	timeUntilNextActivation -= dt;
	if (timeUntilNextActivation <= 0)
	{
		timeUntilNextActivation += interval + Math::RangeRandom(-JITTERED_TIMER_INTERVAL, JITTERED_TIMER_INTERVAL);

		return true;
	}
	else
		return false;
}

CEGUI::UDim StaticText::getXPosition()
{
	return textWnd->getXPosition();
}
void StaticText::setXPosition(CEGUI::UDim x)
{
	textWnd->setXPosition(x);
	if (shadow)
		shadow->setXPosition(x + cegui_absdim(2));
}
CEGUI::UDim StaticText::getYPosition()
{
	return textWnd->getYPosition();
}
void StaticText::setYPosition(CEGUI::UDim y)
{
	textWnd->setYPosition(y);
	if (shadow)
		shadow->setYPosition(y + cegui_absdim(2));
}
void StaticText::setWidth(CEGUI::UDim width)
{
	textWnd->setWidth(width);
	shadow->setWidth(width);
}
void StaticText::setHeight(CEGUI::UDim height)
{
	textWnd->setHeight(height);
	shadow->setHeight(height);
}
void StaticText::setAlpha(float alpha)
{
	textWnd->setAlpha(alpha);
	if (shadow)
		shadow->setAlpha(alpha);
}
void StaticText::setArea(const CEGUI::UDim& xpos, const CEGUI::UDim& ypos, const CEGUI::UDim& width, const CEGUI::UDim& height)
{
	textWnd->setArea(xpos, ypos, width, height);
	if (shadow)
		shadow->setArea(xpos + cegui_absdim(2), ypos + cegui_absdim(2), width, height);
}
void StaticText::setText(const char* text)
{
	textWnd->setText(text);
	if (shadow)
		shadow->setText(text);
}
void StaticText::create(string text, CEGUI::UDim x, CEGUI::UDim y, CEGUI::UDim x2, CEGUI::UDim y2, CEGUI::Window* parent, string color, TextAlignment alignment, bool hasShadow, bool hasBorder)
{
	// Create shadow
	if (hasShadow)
	{
		shadow = CEGUI::WindowManager::getSingleton().createWindow("Style/StaticText");
		shadow->setText(text);
		shadow->setArea(CEGUI::URect(x + cegui_absdim(2),
									 y + cegui_absdim(2),
									 x2,
		   							 y2));
		shadow->setProperty("FrameEnabled", "false");
		shadow->setProperty("BackgroundEnabled", "false");
		shadow->setProperty("TextColours", "tl:FF000000 tr:FF000000 bl:FF000000 br:FF000000");
		if (alignment == TEXTALIGN_CENTER)
			shadow->setProperty("HorzFormatting", "HorzCentred");
		else if (alignment == TEXTALIGN_RIGHT)
			shadow->setProperty("HorzFormatting", "RightAligned");
		parent->addChildWindow(shadow);
	}
	else
		shadow = NULL;

	// Create foreground text
	textWnd = CEGUI::WindowManager::getSingleton().createWindow("Style/StaticText");
	textWnd->setText(text);
	textWnd->setArea(CEGUI::URect(x,
					   			  y,
								  x2,
  								  y2));
	if (!hasBorder)
	{
		textWnd->setProperty("FrameEnabled", "false");
		textWnd->setProperty("BackgroundEnabled", "false");
	}
	if (alignment == TEXTALIGN_CENTER)
		textWnd->setProperty("HorzFormatting", "HorzCentred");
	else if (alignment == TEXTALIGN_RIGHT)
		textWnd->setProperty("HorzFormatting", "RightAligned");
	textWnd->setProperty("TextColours", "tl:FF" + color + " tr:FF" + color + " bl:FF" + color + " br:FF" + color);
	parent->addChildWindow(textWnd);
}
void StaticText::destroy()
{
	if (textWnd)
	{
		CEGUI::WindowManager::getSingleton().destroyWindow(textWnd);
		textWnd = NULL;
	}
	if (shadow)
	{
		CEGUI::WindowManager::getSingleton().destroyWindow(shadow);
		shadow = NULL;
	}
}
StaticText::StaticText()
{
	shadow = NULL;
	textWnd = NULL;
}
