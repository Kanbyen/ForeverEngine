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

#ifndef _DIGISPE_TOUR_H_
#define _DIGISPE_TOUR_H_

#include <vector>
#include <string>
#include <map>

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include "OgreQuaternion.h"

namespace CEGUI
{
	class Window;
	class Font;
}
class TiXmlElement;

class DigiSpeCameraTrack
{
public:
	
	DigiSpeCameraTrack(TiXmlElement* parent);
	~DigiSpeCameraTrack();
	
	void startAt(float t);
	void update(float dt);
	bool isFinished();
	
	void setBlendOutDuration(float blendOutDuration);
	
protected:
	
	struct DigiSpeCameraTrackPoint
	{
		float time;
		float distToNext;
		Vector3 position;
		Vector3 velocity;
		Quaternion yawOrientation;
		Quaternion pitchOrientation;
		Quaternion rollOrientation;
		Radian yaw, yawVelocity;
		Radian pitch, pitchVelocity;
		Radian roll, rollVelocity;
	};
	
	typedef std::vector< DigiSpeCameraTrackPoint > TrackVector;
	TrackVector trackVector;
	
	float currentTime;
	float duration;
	float blendOutDuration;
	
	int nextPoint;
	
	Vector3 getStartVelocity(size_t i);
	Vector3 getEndVelocity(size_t i);
};

/// Allows the user to view a guided cave tour
class DigiSpeTour
{
public:

	struct DigiSpeTourPoint
	{
		Vector3 position;
		Quaternion yawOrientation;
		Quaternion pitchOrientation;
		
		string name;
		string desc;
		
		Ogre::TexturePtr photo;
		std::string photoName;
		bool photoFullscreen;
	};

	// Lifetime
	DigiSpeTour(const char* file);
	void update(float dt);
	~DigiSpeTour();
	
	// Camera track
	void startCameraTrack();
	
	// Guided tour control
	void enterCave();
	void move(int step);
	void togglePhoto();
	
	// Check if loading of the tour file was successful
	inline bool isOk()		{return ok;}
	
	
	static void setCamera(Vector3 pos, Quaternion yaw, Quaternion pitch, Quaternion roll = Quaternion::IDENTITY);
	
	static Vector3 getPosition(TiXmlElement* e);
	static Quaternion getYawOrientation(TiXmlElement* e);
	static Quaternion getPitchOrientation(TiXmlElement* e);
	static Quaternion getRollOrientation(TiXmlElement* e);
	
protected:
	
	// State
	bool ok;
	bool cameraTrackRunning;
	bool enteredCave;
	float timePassed;
	

	
	// CameraTrack
	DigiSpeCameraTrack* camTrack;
	
	// Points
	int actPoint;
	std::vector< DigiSpeTourPoint > points;
	DigiSpeTourPoint start;
	
	// Transitions
	bool transition;
	float transitionTime;
	float transitionDuration;
	bool transitionDetailsChanged;	// are the details (text, photos, ...) of the next point loaded yet?
	DigiSpeTourPoint* p1;
	DigiSpeTourPoint* p2;
	
	// Photos
	bool photoActive;
	float photoAlpha;
	
	// GUI
	CEGUI::Font* pointFont;
	CEGUI::Font* pointDescFont;
	
	CEGUI::Window* pointName;
	CEGUI::Window* pointNameShadow;
	CEGUI::Window* pointDesc;
	CEGUI::Window* pointDescShadow;
	CEGUI::Window* photo;
	CEGUI::Window* camIcon;
	
	CEGUI::Window* blendRect;
	
	void updateGuidedTour(float dt);
	void updateCameraTrack(float dt);
	
	void startTransition(DigiSpeTourPoint& p1, DigiSpeTourPoint& p2);
};

#endif
