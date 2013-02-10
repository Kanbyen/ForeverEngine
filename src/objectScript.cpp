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
#include "objectScript.h"
#include "exposeToLua.h"
#include "stateGame.h"
#include "console.h"

ObjectScriptManager objectScriptManager;

void ObjectScriptManager::init()
{
	// Create the "master state", associate it with luabind and open auxiliary libraries
	masterState = luaL_newstate();
	luabind::open(masterState);
	luaL_openlibs(masterState);

	// Register OGRE and game engine classes and functions
	bindOgreVector3(masterState);
	bindOgreColourValue(masterState);
	bindGameEngine(masterState);
}
void ObjectScriptManager::exit()
{
	if (stateMap.size())
		assert(!"WARNING: stateMap has still entries in ObjectScriptManager::exit()!");
	
	lua_close(masterState);
}
void ObjectScriptManager::loadScriptsFromArchive(const char* filename, const char* archiveType)
{
	Archive* archive = ArchiveManager::getSingleton().load(filename, archiveType);

	FileInfoListPtr infoList = archive->findFileInfo("*.lua", false);

	for (int i = 0; i < (int)infoList->size(); i++)
		loadScriptsFromFile((string(filename) + "/" + (*infoList)[i].filename).c_str());

	ArchiveManager::getSingleton().unload(archive);
}
void ObjectScriptManager::loadScriptsFromFile(const char* file)
{
	luaL_dofile(masterState, file);
}
void ObjectScriptManager::addToStateMap(ObjectScriptExecutor* exec)
{
	stateMap.insert(std::map<lua_State*, ObjectScriptExecutor*>::value_type(exec->threadState, exec));

	// Key:		pointer to exec
	// Value:	thread
	lua_pushlightuserdata(masterState, exec);	// use the pointer to the exec object as key for the coroutine object
	lua_insert(masterState, 1);					// switch the two elements in the stack
	lua_settable(masterState, LUA_REGISTRYINDEX);
}
void ObjectScriptManager::removeFromStateMap(ObjectScriptExecutor* exec)
{
	stateMap.erase(exec->threadState);

	// use the following code again. While commented out, no garbage collection of lua threads will take place, but there are no lua-related crashes.
	lua_pushlightuserdata(masterState, exec);
	lua_pushnil(masterState);
	lua_settable(masterState, LUA_REGISTRYINDEX);
}
ObjectScriptExecutor* ObjectScriptManager::getExec(lua_State* state)
{
	stateMapItr it = stateMap.find(state);
	if (it != stateMap.end())
		return it->second;
	else
		return NULL;
}

ObjectScript::ObjectScript(TiXmlElement* params)
{
	functionName = "";
	if (params->Attribute("script"))
		functionName = params->Attribute("script");
}
ObjectScript::~ObjectScript()
{
}


ObjectScriptExecutor::ObjectScriptExecutor(ObjectScript* script, Object* object)
{
	this->script = script;
	this->object = object;

	threadState = lua_newthread(objectScriptManager.masterState);
	objectScriptManager.addToStateMap(this);

	stop();
}
ObjectScriptExecutor::~ObjectScriptExecutor()
{
	objectScriptManager.removeFromStateMap(this);
}
void ObjectScriptExecutor::stop()
{
	running = false;
	time = 0;
	sleepTime = 0;
}
void ObjectScriptExecutor::run()
{
	if (script->functionName == "")
		return;

	try {
		if (!running)
		{
			running = true;
			luabind::resume_function<void>(threadState, script->functionName.c_str(), object);
		}
		else
		{
			luabind::resume<void>(threadState);
		}
	} catch (luabind::error&) {
		const char* msg;

		msg = lua_tostring(threadState, -1);
		if (msg == NULL)
			msg = "(error without message)";
		lua_pop(threadState, 1);

		LogManager::getSingleton().logMessage("SCRIPT ERROR:");
		LogManager::getSingleton().logMessage(msg);

		game.gameConsole->addMessage("SCRIPT ERROR:", -1.0f, "FF5555");
		game.gameConsole->addMessage(msg, -1.0f, "FF5555");
	}

	if (sleepTime <= 0)
		stop();
}
void ObjectScriptExecutor::update(float dt)
{
	if (running)
	{
		time += dt;

		if (sleepTime > 0)
		{
			sleepTime -= dt;
			if (sleepTime > 0)
				return;

			sleepTime = 0;
		}

		run();
	}
}
