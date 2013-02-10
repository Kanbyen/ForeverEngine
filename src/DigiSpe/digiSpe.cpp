/*****************************************************************************************

Forever War - a NetHack-like FPS

Copyright (C) 2008 Thomas Sch�ps

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
#include "digiSpe.h"
#include "digiSpeElement.h"
#include "digiSpeTour.h"
#include "blockConstants.h"
#include "tinyxml.h"
#include "util.h"
#include "windowManager.h"
#include "stateGame.h"
#include "volume.h"
#include "level.h"
#include "windowManager.h"	// To get the mouse coordinates to show the tooltip

DigiSpe* digiSpe = NULL;

DigiSpe::DigiSpe(const char* caveFile, const char* tourFile, bool releaseMode)
{
	assert(!digiSpe && "Two digiSpe objects created at the same time!");
	digiSpe = this;
	
	double dTemp;
	
	ready = false;
	currentTime = 0;
	
	scale = 7;	// configurabe by the "voxelDistance" attribute of the Cave tag
	
	// Create and initialize our resource group
	Ogre::ResourceGroupManager::getSingleton().createResourceGroup("DigiSpe");
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("DigiSpe");
	
	// Show GUI Page
	if (releaseMode)
		windowManager.setPage("DigiSpe");
	
	// Init GUI
	blendRect = CEGUI::WindowManager::getSingleton().getWindow("DigiSpeBlendRect");
	aboutText = CEGUI::WindowManager::getSingleton().getWindow("DigiSpeAboutText");
	
	aboutShown = false;
	startBlendIn();

	// Load XML file
	if (!releaseMode)
	{
		TiXmlDocument doc(caveFile);
		if (!doc.LoadFile())
		{
			LogManager::getSingleton().logMessage("ERROR: DigiSpe: Cannot open cave file (wrong path / invalid XML)!", LML_CRITICAL);
			return;
		}
	
		TiXmlHandle docHandle(&doc);
		TiXmlElement* elem = docHandle.FirstChildElement().ToElement();
		if (!elem)
		{
			LogManager::getSingleton().logMessage("ERROR: DigiSpe: Invalid cave file (no XML element)!", LML_CRITICAL);
			return;
		}
	
		if (elem->Value() != string("Cave"))
		{
			LogManager::getSingleton().logMessage("ERROR: DigiSpe: Invalid cave file (does not begin with a Cave tag)!", LML_CRITICAL);
			return;
		}
	
		if (elem->Attribute("voxelDistance", &dTemp))
			scale = 1.0f / ((float)dTemp);
			
		const char* cStr = elem->Attribute("showSamples");
		showSamples = (cStr && (cStr == string("true")));
	
		cStr = elem->Attribute("noDeactivate");
		noDeactivate = (cStr && (cStr == string("true")));
	
		// Read all elements
		elem = elem->FirstChildElement();
		while (elem)
		{
			if (elem->Value() == string("Point"))
			{
				DigiSpeMeasuringPoint* mp = new DigiSpeMeasuringPoint(elem);
				pointList.push_back(mp);
				if (noDeactivate || !mp->deactivated)
					elements.push_back(mp);
				else
					auxElements.push_back(mp);
			}
			else if (elem->Value() == string("AuxPoint"))
			{
				DigiSpeMeasuringPoint* mp = new DigiSpeMeasuringPoint(elem);
				pointList.push_back(mp);
				auxElements.push_back(mp);
			}
			else if (elem->Value() == string("Extrusion"))
			{
				DigiSpeExtrusion* ext = new DigiSpeExtrusion(elem);
				if (noDeactivate || !ext->deactivated)
					elements.push_back(ext);
				else
					auxElements.push_back(ext);
			}
			else
				LogManager::getSingleton().logMessage("WARNING: DigiSpe: Unknown Tag '" + string(elem->Value()) + "' inside the Cave tag!", LML_CRITICAL);
	
			elem = elem->NextSiblingElement();
		}
	
		// Do postprocessing
		for (int i = 0; i < (int)elements.size(); ++i)
			elements[i]->postprocess();
		for (int i = 0; i < (int)auxElements.size(); ++i)
			auxElements[i]->postprocess();
		
		for (int i = 0; i < (int)elements.size(); ++i)
			elements[i]->autoImportSamples();
		for (int i = 0; i < (int)auxElements.size(); ++i)
			auxElements[i]->autoImportSamples();
		
		for (int i = 0; i < (int)elements.size(); ++i)
			elements[i]->postprocess2();
		for (int i = 0; i < (int)auxElements.size(); ++i)
			auxElements[i]->postprocess2();
		
		// Save OBJ
		const bool saveOBJ = true;
		if (saveOBJ)
		{
			FILE* objFile = fopen("cave.obj", "wb");
			string header = "# Cave generated by DigiSpe\r\n\r\n";
			fwrite(header.c_str(), header.size(), 1, objFile);
			
			int vIndex = 1;
			for (int i = 0; i < (int)elements.size(); ++i)
				elements[i]->saveOBJ(objFile, vIndex);
			
			fclose(objFile);
		}
	}
	else
	{
		showSamples = false;
		noDeactivate = true;
		
		setupGUI();
	}
	
	if (showSamples)
		initTooltip();
	
	// Set block load callbacks
	game.volumeData->setBlockLoadCallback(&DigiSpe::voxelCB);
	game.volumeData->setBlockTextureCallback(NULL); //&Level::other_autotexCallback);
	
	// Load tour
	tour = NULL;
	if (tourFile[0] != 0)
	{
		tour = new DigiSpeTour(tourFile);
		if (!tour->isOk())
		{
			delete tour;
			tour = NULL;
		}
	}
	
	ready = true;
}
DigiSpe::~DigiSpe()
{
	if (!ready)
		return;
		
	delete tour;

	for (int i = 0; i < (int)elements.size(); ++i)
		delete elements[i];
	for (int i = 0; i < (int)auxElements.size(); ++i)
		delete auxElements[i];	
	digiSpe = NULL;

	if (showSamples)
		exitTooltip();
}

DigiSpeMeasuringPoint* DigiSpe::getMeasuringPointByName(const char* name, bool errorIfNotFound)
{
	for (int i = 0; i < (int)pointList.size(); ++i)
	{
		if (pointList[i]->name == name)
			return pointList[i];
	}

	if (errorIfNotFound)
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: Measuring point with name " + string(name) + " not found!", LML_CRITICAL);
	return NULL;
}

int DigiSpe::voxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer)
{
	const int blx = lastX - firstX + 1;
	const int bly = lastY - firstY + 1;

	float fBuffer[BLOCK_SIDE_LENGTH+2][BLOCK_SIDE_LENGTH+2][BLOCK_SIDE_LENGTH+2];
	for (int x = 0; x <= lastX - firstX; ++x)
		for (int y = 0; y <= lastY - firstY; ++y)
			for (int z = 0; z <= lastZ - firstZ; ++z)
				fBuffer[x][y][z] = 1.0f;

	bool elementApplied = false;
	for (ElementListItr it = digiSpe->elements.begin(); it != digiSpe->elements.end(); ++it)
	{
		if ((lastX < (*it)->bbMin.x) || (firstX > (*it)->bbMax.x) || (lastY < (*it)->bbMin.y) || (firstY > (*it)->bbMax.y) || (lastZ < (*it)->bbMin.z) || (firstZ > (*it)->bbMax.z))
			continue;

		for (int x = max(firstX, (*it)->bbMin.x); x <= min(lastX, (*it)->bbMax.x); ++x)
			for (int y = max(firstY, (*it)->bbMin.y); y <= min(lastY, (*it)->bbMax.y); ++y)
				for (int z = max(firstZ, (*it)->bbMin.z); z <= min(lastZ, (*it)->bbMax.z); ++z)
					fBuffer[x-firstX][y-firstY][z-firstZ] = (*it)->apply(x, y, z, fBuffer[x-firstX][y-firstY][z-firstZ]);
		elementApplied = true;
	}

	if (!elementApplied)
		return RETURN_FULL_BLOCK;

	for (int x = 0; x <= lastX - firstX; ++x)
	{
		for (int y = 0; y <= lastY - firstY; ++y)
		{
			for (int z = 0; z <= lastZ - firstZ; ++z)
			{
				int value = fBuffer[x][y][z] * 127;

				// clamp value to [-128; 127]
				if (value > 127)		value = 127;
				else if (value < -128)	value = -128;

				buffer[(x) + (y) * blx + (z) * blx * bly] = (char)(value);
			}
		}
	}

	return RETURN_NORMAL_BLOCK;
}

DigiSpeMeasuringPoint::Sample* DigiSpe::getSampleUnderCursor()
{
	if (showSamples)
	{
		double mx = windowManager.getMouseXRel();
		double my = windowManager.getMouseYRel();

		if ((mx < 0) || (mx > 1) || (my < 0) || (my > 1))
			return NULL;

		// Execute query
		Ray mouseRay = camera->getCameraToViewportRay(mx, my);
		sceneQuery->setRay(mouseRay);
		RaySceneQueryResult &result = sceneQuery->execute();
		if (!result.size())
			return NULL;

		// Filter results
		MovableObject* movableObj = NULL;
		RaySceneQueryResult::iterator itr = result.begin();
		do
		{
			if (!itr->movable)
				continue;
			if (itr->movable->getUserAny().isEmpty())
				continue;

			movableObj = itr->movable;
			break;
		}
		while (++itr != result.end());

		if (!movableObj)
			return NULL;
		
		if (movableObj->getName().compare(0, strlen("DigiSpeDebug"), "DigiSpeDebug") != 0)
			return NULL;

		return any_cast<DigiSpeMeasuringPoint::Sample*>(movableObj->getUserAny());
	}
	else
		return NULL;
}
void DigiSpe::startBlendIn()
{
	blendInTime = currentTime;
	blendRect->setVisible(true);
}
void DigiSpe::update(float dt)
{
	if (!ready)
		return;
	
	currentTime += dt;

	// Update sample tooltip
	if (showSamples)
	{
		DigiSpeMeasuringPoint::Sample* sample = getSampleUnderCursor();
		if (sample)
		{
			tooltip->setText(("Name: " + sample->name).c_str());	//  + "\nSpread: " + StringConverter::toString(1.0f / sample->invspread)
			tooltip->setArea(cegui_reldim(windowManager.getMouseXRel()) + cegui_absdim(16), cegui_reldim(windowManager.getMouseYRel()) - cegui_reldim(0.5f) + cegui_absdim(4), cegui_reldim(1), cegui_reldim(1));
		}
		else
		{
			tooltip->setText("");
			tooltip->setArea(cegui_absdim(-1), cegui_absdim(-1), cegui_absdim(0), cegui_absdim(0));
		}
	}
	
	// Show / hide about text
	if (aboutShown)
	{
		if (aboutEndTime < 0)
		{
			// Show about text
			float factor = std::min(1.0f, (currentTime - aboutStartTime) / 0.4f);
			blendRect->setAlpha(0.4f * factor);
			aboutText->setAlpha(factor);
		}
		else if (currentTime < aboutEndTime + 0.4f)
		{
			// Hide about text
			float factor = std::min(1.0f, (currentTime - aboutEndTime) / 0.4f);
			blendRect->setAlpha(0.4f * (1 - factor));
			aboutText->setAlpha(1 - factor);
		}
		else
		{
			// End transition
			blendRect->hide();
			aboutShown = false;
		}
	}
	else if (blendInTime >= 0)
	{
		// Blend in
		float factor = 1 - ((currentTime - blendInTime) / 0.4f);
		if (factor > 0)
			blendRect->setAlpha(factor);
		else
		{
			if (blendRect->isVisible())
				blendRect->setVisible(false);
		}
	}
	
	// Update tour
	if (tour)
		tour->update(dt);
}

void DigiSpe::initTooltip()
{
    sceneQuery = sceneMgr->createRayQuery(Ray());
    sceneQuery->setSortByDistance(true);
	//sceneQuery->setQueryMask(~DONT_INCLUDE_IN_QUERY);

    tooltip = new StaticText();
	tooltip->create("", cegui_absdim(-1), cegui_absdim(-1), cegui_absdim(-1), cegui_absdim(-1),
					CEGUI::WindowManager::getSingleton().getWindow("EditorRoot"));
}
void DigiSpe::exitTooltip()
{
	sceneMgr->destroyQuery(sceneQuery);

	tooltip->destroy();
	delete tooltip;
}

#define event_subscribe(window, eventName, method, className) { \
	btn = (CEGUI::PushButton *)CEGUI::WindowManager::getSingleton().getWindow(window); \
	btn->subscribeEvent(eventName, CEGUI::Event::Subscriber(method, className)); \
	}

void DigiSpe::setupGUI()
{
	CEGUI::PushButton* btn;
	
	event_subscribe("DigiSpeEnter", CEGUI::PushButton::EventClicked, &DigiSpe::Button_Enter, this);
	event_subscribe("DigiSpeCameraTrack", CEGUI::PushButton::EventClicked, &DigiSpe::Button_CameraTrack, this);
	event_subscribe("DigiSpeAboutProgram", CEGUI::PushButton::EventClicked, &DigiSpe::Button_AboutProgram, this);
	event_subscribe("DigiSpeBlendRect", CEGUI::Window::EventMouseButtonDown, &DigiSpe::Button_BlendRect, this);
	event_subscribe("DigiSpeAboutText", CEGUI::Window::EventMouseButtonDown, &DigiSpe::Button_BlendRect, this);
	
	event_subscribe("DigiSpeNext", CEGUI::PushButton::EventClicked, &DigiSpe::Button_Next, this);
	event_subscribe("DigiSpePrevious", CEGUI::PushButton::EventClicked, &DigiSpe::Button_Previous, this);
	event_subscribe("DigiSpeQuit", CEGUI::PushButton::EventClicked, &DigiSpe::Button_Quit, this);
	event_subscribe("DigiSpeCamera", CEGUI::Window::EventMouseClick, &DigiSpe::Button_Camera, this);
}
bool DigiSpe::Button_Enter(const CEGUI::EventArgs &args)
{
	CEGUI::WindowManager::getSingleton().getWindow("DigiSpeStart")->hide();
	CEGUI::WindowManager::getSingleton().getWindow("DigiSpeTour")->show();
	
	if (tour)
		tour->enterCave();
	
	return true;
}
bool DigiSpe::Button_CameraTrack(const CEGUI::EventArgs &args)
{
	CEGUI::WindowManager::getSingleton().getWindow("DigiSpeStart")->hide();
	
	if (tour)
		tour->startCameraTrack();
	
	return true;
}
bool DigiSpe::Button_AboutProgram(const CEGUI::EventArgs &args)
{
	aboutText->setText("�ber das Programm\r\n\r\nDigiSpe wurde von Thomas Sch�ps und Peter Meier entwickelt ...\r\n\r\nTODO: finalen Text und Lizenzbestimmungen\r\nder verwendeten Bibliotheken + GPL hinzuf�gen");
	aboutText->show();
	
	blendRect->show();
	blendRect->setAlpha(0);
	
	aboutShown = true;
	aboutStartTime = currentTime;
	aboutEndTime = -1;
	
	return true;
}
bool DigiSpe::Button_BlendRect(const CEGUI::EventArgs &args)
{
	if (aboutShown && aboutEndTime < 0)
		aboutEndTime = currentTime;
	
	return true;
}
bool DigiSpe::Button_Next(const CEGUI::EventArgs &args)
{
	if (tour)
		tour->move(1);
	return true;
}
bool DigiSpe::Button_Previous(const CEGUI::EventArgs &args)
{
	if (tour)
		tour->move(-1);
	return true;
}
bool DigiSpe::Button_Quit(const CEGUI::EventArgs &args)
{
	stateManager.changeState(NULL);
	return true;
}
bool DigiSpe::Button_Camera(const CEGUI::EventArgs &args)
{
	if (tour)
		tour->togglePhoto();
	return true;
}
