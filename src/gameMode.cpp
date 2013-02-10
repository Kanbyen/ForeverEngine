#include "precompiled.h"
#include "gameMode.h"
#include "objectScript.h"
#include "tinyxml.h"

GameModeManager gameModeManager;

void GameModeManager::exit()
{
	for (unsigned int i = 0; i < gameModes.size(); ++i)
		delete gameModes[i];
	gameModes.clear();
}

void GameModeManager::setActMode(const char* id)
{
	actMode = getModeByID(id);
	if (!actMode)
	{
		LogManager::getSingleton().logMessage(("ERROR: Tried to set non-existing game mode with id: " + string(id)).c_str());
		return;
	}
}

void GameModeManager::doGameModeInitialization()
{
	// TODO: don't load lua file if already loaded
	string luaFile = actMode->basePath + ".lua";
	objectScriptManager.loadScriptsFromFile(luaFile.c_str());
	
	try {
		luabind::call_function<void>(objectScriptManager.masterState, "initMode");
	} catch (luabind::error&) {
		const char* msg;
		
		msg = lua_tostring(objectScriptManager.masterState, -1);
		if (msg == NULL)
			msg = "(error without message)";
		lua_pop(objectScriptManager.masterState, 1);
		
		LogManager::getSingleton().logMessage("SCRIPT ERROR:");
		LogManager::getSingleton().logMessage(msg);
	}
}

GameMode* GameModeManager::getModeByID(const char* id)
{
	for (unsigned int i = 0; i < gameModes.size(); ++i)
		if (gameModes[i]->id == id)
			return gameModes[i];
	
	return NULL;
}

void GameModeManager::loadModesFromArchive(const char* filename, const char* archiveType)
{
	Archive* archive = ArchiveManager::getSingleton().load(filename, archiveType);

	FileInfoListPtr infoList = archive->findFileInfo("*.mode", false);

	for (int i = 0; i < (int)infoList->size(); i++)
		loadModeFromFile((string(filename) + "/" + (*infoList)[i].filename).c_str());

	ArchiveManager::getSingleton().unload(archive);
}
void GameModeManager::loadModeFromFile(const char* file)
{
	TiXmlDocument doc(file);
	if (!doc.LoadFile())
	{
		LogManager::getSingleton().logMessage(("ERROR: Could not load game mode file (invalid path or invalid XML): " + string(file)).c_str());
		return;
	}
	
	TiXmlHandle docHandle(&doc);
	TiXmlElement* element = docHandle.FirstChildElement("Mode").ToElement();
	
	GameMode* newMode = new GameMode();
	newMode->basePath = file;
	int dotPos = newMode->basePath.rfind('.');
	newMode->basePath = newMode->basePath.substr(0, dotPos);
	
	int slashPos = newMode->basePath.rfind('/');
	newMode->id = newMode->basePath.substr(slashPos + 1);
	
	TiXmlElement* elem = element->FirstChildElement("DisplayName");
	newMode->displayName = elem->GetText();
	
	elem = element->FirstChildElement("Description");
	newMode->description = elem->GetText();
	
	gameModes.push_back(newMode);
}
