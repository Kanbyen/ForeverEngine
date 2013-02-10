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
#include "console.h"
#include "windowManager.h"
#include "util.h"

Console::Console(CEGUI::Window* root, float startX, float startY, int maxLines, float messageDuration, float fadeTime, TextAlignment alignment, bool bottomUp, bool hasBorder)
{
	currentTime = 0;
	linesShown = 0;
	this->root = root;
	this->startX = startX;
	this->startY = startY;
	actY = startY;
	this->maxLines = maxLines;
	this->messageDuration = messageDuration;
	this->fadeTime = fadeTime;
	this->hasBorder = hasBorder;
	this->alignment = alignment;
	if (bottomUp)	direction = -1;
	else			direction = 1;
}
void Console::addMessage(string text, float duration, string color)
{
	ConsoleMessage newMsg;

	if (duration < 0)
		duration = messageDuration;

	// Get the line count and horizontal extent of the text
	float textextent = 0;
	int lastextentmeasurepos = 0;
	int numLines = 1;
	
	for (int i = 0; i < (int)text.length(); i++)
		if (text.c_str()[i] == '\n')
		{
			numLines++;
			string line = text.substr(lastextentmeasurepos, i - lastextentmeasurepos + 1);
			float extent = windowManager.getFont()->getTextExtent(line);
			if (extent > textextent)
				textextent = extent;
			lastextentmeasurepos = i + 1;
		}
	
	if (lastextentmeasurepos < (int)text.length())
	{
		float extent = windowManager.getFont()->getTextExtent(text.substr(lastextentmeasurepos, string::npos));
		if (extent > textextent)
			textextent = extent;
	}
	newMsg.numLines = numLines;
	linesShown += numLines;
	float height = numLines * windowManager.getFont()->getLineSpacing() + (hasBorder ? 10 : 0);
	textextent += (hasBorder ? 10 : 0);

	// Respect the maximum line count by deleting the topmost messages until there is enough space
	msgListItr it = msgList.begin();
	int tempLinesShown = linesShown;
	while ((tempLinesShown > maxLines) && (it != msgList.end()))
	{
		tempLinesShown -= (*it).numLines;
		(*it).expireTime = -1.0f;
		it++;
	}
	update(0);

	CEGUI::UDim deltaHeight1;
	CEGUI::UDim deltaHeight2;
	if (direction > 0)
	{
		deltaHeight1 = cegui_absdim(0);
		deltaHeight2 = cegui_absdim(height);
	}
	else
	{
		deltaHeight1 = cegui_absdim(-height);
		deltaHeight2 = cegui_absdim(0);
	}

	if (alignment == TEXTALIGN_LEFT)
	{
		newMsg.wnd.create(text, cegui_reldim(startX), cegui_reldim(startY) + cegui_absdim(actY) + deltaHeight1,
						  cegui_reldim(startX) + cegui_absdim(textextent), cegui_reldim(startY) + cegui_absdim(actY) + deltaHeight2,
						  root, color, alignment, !hasBorder, hasBorder);
	}
	else if (alignment == TEXTALIGN_CENTER)
	{
		newMsg.wnd.create(text, cegui_reldim(startX) - cegui_absdim(textextent / 2), cegui_reldim(startY) + cegui_absdim(actY) + deltaHeight1,
						  cegui_reldim(startX) + cegui_absdim(textextent / 2), cegui_reldim(startY) + cegui_absdim(actY) + deltaHeight2,
						  root, color, alignment, !hasBorder, hasBorder);
	}
	else
	{
		newMsg.wnd.create(text, cegui_reldim(startX) - cegui_absdim(textextent), cegui_reldim(startY) + cegui_absdim(actY) + deltaHeight1,
						  cegui_reldim(startX), cegui_reldim(startY) + cegui_absdim(actY) + deltaHeight2,
						  root, color, alignment, !hasBorder, hasBorder);
	}

	newMsg.setAlpha(0.0f);
	newMsg.creationTime = currentTime;
	newMsg.expireTime = currentTime + duration;

	msgList.push_back(newMsg);
	actY += height * direction;
}
void Console::update(float timepassed)
{
	int newLinesShown = 0;
	float deltaPos = 0;
	currentTime += timepassed;

	// Update all messages
	for (msgListItr it = msgList.begin(); it != msgList.end();)
	{
		if (currentTime >= (*it).expireTime)
		{
			// Line expired, delete it
			actY -= direction * ((*it).numLines * windowManager.getFont()->getLineSpacing() + (hasBorder ? 10 : 0));
			deltaPos -= direction * ((*it).numLines * windowManager.getFont()->getLineSpacing() + (hasBorder ? 10 : 0));
			newLinesShown -= (*it).numLines;
			(*it).wnd.destroy();
			it = msgList.erase(it);
		}
		else
		{
			// Handle alpha
			if (currentTime < (*it).creationTime + fadeTime)
				(*it).setAlpha((currentTime - (*it).creationTime) / fadeTime);
			else if (currentTime > (*it).expireTime - fadeTime)
				(*it).setAlpha(((*it).expireTime - currentTime) / fadeTime);
			else
				(*it).setAlpha(1.0f);

			// Handle Position
			if (newLinesShown)
				(*it).wnd.setYPosition((*it).wnd.getYPosition() + cegui_absdim(deltaPos));
			
			it++;
		}
	}
	linesShown += newLinesShown;
}
Console::~Console()
{
	for (msgListItr it = msgList.begin(); it != msgList.end(); it++)
		(*it).wnd.destroy();

	msgList.clear();
}
