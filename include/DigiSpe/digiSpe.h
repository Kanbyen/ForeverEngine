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

#ifndef _DIGISPE_H_
#define _DIGISPE_H_

#include <vector>
#include "digiSpeElement.h"

class DigiSpeTour;
class StaticText;
namespace Ogre
{
	class RaySceneQuery;
};
namespace CEGUI
{
	class Window;
}

/// Class for the visualization of caves
class DigiSpe
{
public:

	// <Cave> tag options
	bool showSamples;
	bool noDeactivate;
	float scale;

	/// Measuring points are entered in this list
	typedef std::vector<DigiSpeElement*> ElementList;
	typedef ElementList::iterator ElementListItr;
	ElementList elements;
	ElementList auxElements;
	
	typedef std::vector<DigiSpeMeasuringPoint*> MeasuringPointList;
	MeasuringPointList pointList;

	
	DigiSpe(const char* caveFile, const char* tourFile, bool releaseMode);
	~DigiSpe();


	void update(float dt);
	void startBlendIn();
	
	DigiSpeMeasuringPoint::Sample* getSampleUnderCursor();
	DigiSpeMeasuringPoint* getMeasuringPointByName(const char* name, bool errorIfNotFound = true);

	static int voxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer);

protected:

	bool ready;
	float currentTime;
	
	// Display handling of about text
	bool aboutShown;
	float aboutStartTime;
	float aboutEndTime;

	// Scene queries
	RaySceneQuery* sceneQuery;
	StaticText* tooltip;
	void initTooltip();
	void exitTooltip();
	
	// GUI
	float blendInTime;
	
	CEGUI::Window* blendRect;
	CEGUI::Window* aboutText;
	
	void setupGUI();
	bool Button_Enter(const CEGUI::EventArgs &args);
	bool Button_CameraTrack(const CEGUI::EventArgs &args);
	bool Button_AboutProgram(const CEGUI::EventArgs &args);
	bool Button_BlendRect(const CEGUI::EventArgs &args);
	
	bool Button_Next(const CEGUI::EventArgs &args);
	bool Button_Previous(const CEGUI::EventArgs &args);
	bool Button_Quit(const CEGUI::EventArgs &args);
	bool Button_Camera(const CEGUI::EventArgs &args);
	
	// Tour
	DigiSpeTour* tour;
};

extern DigiSpe* digiSpe;

#endif
