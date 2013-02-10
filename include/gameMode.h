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

#ifndef _GAME_MODE_H_
#define _GAME_MODE_H_

#include <vector>

/// Represents a game mode (for example Deathmatch)
class GameMode
{
public:

	string id;			// for example "dm"
	string basePath;	// for example "resource/mode/dm"; append extensions as necessary
	string displayName;
	string description;
};

/// Keeps a list of game modes
class GameModeManager
{
public:

	/// Active game mode
	GameMode* actMode;

	/// Clean up
	void exit();

	/// Sets the active mode by id
	void setActMode(const char* id);
	
	/// Call the game mode init script of the acive game mode
	void doGameModeInitialization();
	
	/// Get a game mode by ID
	GameMode* getModeByID(const char* id);
	
	/// Loads modes from an archive
	void loadModesFromArchive(const char* filename, const char* archiveType);

	/// Loads mode from a file
	void loadModeFromFile(const char* file);

	/// List of all game modes
	std::vector<GameMode*> gameModes;
};

extern GameModeManager gameModeManager;

#endif
