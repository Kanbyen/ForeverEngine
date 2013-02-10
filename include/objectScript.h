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

#ifndef _OBJECT_SCRIPT_H_
#define _OBJECT_SCRIPT_H_

#include <luabind/luabind.hpp>

#include <map>
#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "object.h"
#include "tinyxml.h"

class ObjectScriptExecutor;

class ObjectScriptManager
{
public:
	lua_State* masterState;
	std::map<lua_State*, ObjectScriptExecutor*> stateMap;
	typedef std::map<lua_State*, ObjectScriptExecutor*>::iterator stateMapItr;

	/// Initializes the scripting language and registers the game functions for use by the scripts
	void init();

	/// Cleans up
	void exit();

	/// Loads scripts from an archive
	void loadScriptsFromArchive(const char* filename, const char* archiveType);

	/// Loads scripts from a file
	void loadScriptsFromFile(const char* file);

	void addToStateMap(ObjectScriptExecutor* exec);
	void removeFromStateMap(ObjectScriptExecutor* exec);
	ObjectScriptExecutor* getExec(lua_State* state);
};

extern ObjectScriptManager objectScriptManager;

/// A script associated to an object
class ObjectScript
{
public:
	/// Name of the script function to call
	string functionName;

	/// Constructor, takes the first node of the script as input
	ObjectScript(TiXmlElement* params);
	~ObjectScript();
};

/// Executes an ObjectScript. Needed because multiple executions of a script at the same time should be possible.
class ObjectScriptExecutor
{
public:
	bool running;
	float time;
	ObjectScript* script;
	lua_State* threadState;

	/// Time the execution of the script has to wait
	float sleepTime;

	/// Parent object of this script
	Object* object;

	/// Constructor
	ObjectScriptExecutor(ObjectScript* script, Object* object);

	/// Destructor
	~ObjectScriptExecutor();

	/// Starts the execution of the script. If the script has a duration (calls wait()), regular calls to update are also neccessary!
	void run();

	/// Starts the execution of the script and passes 2 additional parameters to it. See also run()
	template <class T1, class T2> void run(T1 param1, T2 param2)
	{
		if (script->functionName == "")
			return;

		try {
			if (!running)
			{
				running = true;
				luabind::resume_function<void>(threadState, script->functionName.c_str(), object, param1, param2);
			}
		} catch (luabind::error&) {
			const char* msg;

			msg = lua_tostring(threadState, -1);
			if (msg == NULL)
				msg = "(error without message)";
			lua_pop(threadState, 1);

			Ogre::LogManager::getSingleton().logMessage("SCRIPT ERROR:");
			Ogre::LogManager::getSingleton().logMessage(msg);

			//game.gameConsole->addMessage("SCRIPT ERROR:", -1.0f, "FF5555");
			//game.gameConsole->addMessage(msg, -1.0f, "FF5555");
		}

		if (sleepTime <= 0)
			stop();
	}

	/// Starts the execution of the script and passes 1 additional parameter to it. See also run()
	template <class T1> void run(T1 param1)
	{
		if (script->functionName == "")
			return;

		try {
			if (!running)
			{
				running = true;
				luabind::resume_function<void>(threadState, script->functionName.c_str(), object, param1);
			}
		} catch (luabind::error&) {
			const char* msg;

			msg = lua_tostring(threadState, -1);
			if (msg == NULL)
				msg = "(error without message)";
			lua_pop(threadState, 1);

			Ogre::LogManager::getSingleton().logMessage("SCRIPT ERROR:");
			Ogre::LogManager::getSingleton().logMessage(msg);

			//game.gameConsole->addMessage("SCRIPT ERROR:", -1.0f, "FF5555");
			//game.gameConsole->addMessage(msg, -1.0f, "FF5555");
		}

		if (sleepTime <= 0)
			stop();
	}

	/// Stops the script execution
	void stop();

	/// Should be called every logic update; see run()
	void update(float dt);

	/// self-explanatory
	inline bool isRunning() {return running;}
};

#endif
