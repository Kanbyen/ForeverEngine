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

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

/// Stores global game settings which are loaded from settings.xml
class Settings
{
public:
	void setDefaults();
	void tryToLoadFromFile(const char* path);
	void saveToFile();
	void getScreenResolution(int* width, int* height);

	string path;

	// The actual settings
	string gameMode;
	string renderSystem;
	int resolution_Width;
	int resolution_Height;
	bool fullscreen;
	bool vSync;
	bool bloom;
	string nickname;
	string lastServer;
	float mouseSensitivity;
};

extern Settings settings;

#endif
