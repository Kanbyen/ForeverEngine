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
#include "digiSpeTour.h"
#include "tinyxml.h"
#include "stateGame.h"
#include "windowManager.h"
#include <util.h>
#include <digiSpe.h>

// ### DigiSpeCameraTrack ###

Radian adaptAngle(Radian a, Radian b)
{
	Radian b2 = Radian(2*M_PI) + b;
	if (b2 - a < a - b)
		return b2;
	
	Radian b3 = b - Radian(2*M_PI);
	if (a - b3 < b - a)
		return b3;
	
	return b;
}

Radian calcAngleVelocity(float t0, Radian r0, float t1, Radian r1, float t2, Radian r2)
{
	r2 = adaptAngle(r1, r2);
	Vector2 toNext = Vector2(t2, r2.valueRadians()) - Vector2(t1, r1.valueRadians());
	
	r1 = adaptAngle(r0, r1);
	Vector2 toPrev = Vector2(t0, r0.valueRadians()) - Vector2(t1, r1.valueRadians());
	
	Vector2 prev = Vector2(t0, r0.valueRadians());
	Vector2 cur = Vector2(t1, r1.valueRadians());
	Vector2 next = Vector2(t2, r2.valueRadians());
	
	Vector2 v = toNext.normalisedCopy() - toPrev.normalisedCopy();
	return 0.05f * Radian(v.normalisedCopy().y);
}

DigiSpeCameraTrack::DigiSpeCameraTrack(TiXmlElement* parent)
{
	duration = 0;
	nextPoint = 1;
	blendOutDuration = -1;
	
	// Load points
	TiXmlElement* e = parent->FirstChildElement("Point");
	bool first = true;
	while (e)
	{
		DigiSpeCameraTrackPoint newPoint;
		
		newPoint.position = DigiSpeTour::getPosition(e);
		newPoint.yawOrientation = DigiSpeTour::getYawOrientation(e);
		newPoint.yaw = newPoint.yawOrientation.getYaw(true);
		newPoint.pitchOrientation = DigiSpeTour::getPitchOrientation(e);
		newPoint.pitch = newPoint.pitchOrientation.getPitch(true);
		newPoint.rollOrientation = DigiSpeTour::getRollOrientation(e);
		newPoint.roll = newPoint.rollOrientation.getRoll(true);
		
		if (first)
			first = false;
		else
		{
			size_t prev = trackVector.size()-1;
			trackVector[prev].distToNext = (newPoint.position - trackVector[prev].position).length() * 0.05f;
			duration += trackVector[prev].distToNext;
		}
		//duration += StringConverter::parseReal(e->Attribute("time"));
		
		newPoint.time = duration;
		
		trackVector.push_back(newPoint);
		
		e = e->NextSiblingElement();
	}
	
	// Calculate node velocities
	for (size_t i = 1; i < trackVector.size()-1; ++i)
	{
		trackVector[i].distToNext = (trackVector[i+1].position - trackVector[i].position).length();
		
		trackVector[i].velocity = (trackVector[i+1].position - trackVector[i].position).normalisedCopy() -
		                          (trackVector[i-1].position - trackVector[i].position).normalisedCopy();
		trackVector[i].velocity.normalise();
		
		trackVector[i].yawVelocity = calcAngleVelocity(trackVector[i-1].time, trackVector[i-1].yaw,
													   trackVector[i].time, trackVector[i].yaw,
													   trackVector[i+1].time, trackVector[i+1].yaw);
		trackVector[i].pitchVelocity = calcAngleVelocity(trackVector[i-1].time, trackVector[i-1].pitch,
													   trackVector[i].time, trackVector[i].pitch,
													   trackVector[i+1].time, trackVector[i+1].pitch);
		trackVector[i].rollVelocity = calcAngleVelocity(trackVector[i-1].time, trackVector[i-1].roll,
													   trackVector[i].time, trackVector[i].roll,
													   trackVector[i+1].time, trackVector[i+1].roll);
	}
	
	trackVector[0].distToNext = (trackVector[1].position - trackVector[0].position).length();
	trackVector[0].velocity = getStartVelocity(0);
	trackVector[0].yawVelocity = Radian(0);
	trackVector[0].pitchVelocity = Radian(0);
	trackVector[0].rollVelocity = Radian(0);
	
	size_t idx = trackVector.size() - 1;
	trackVector[idx].velocity = getEndVelocity(idx);
	trackVector[idx].yawVelocity = Radian(0);
	trackVector[idx].pitchVelocity = Radian(0);
	trackVector[idx].rollVelocity = Radian(0);
	
	// Smooth
	size_t nodeCount = trackVector.size();
	for (int repetitions = 0; repetitions < 3; ++repetitions)
	{
		Vector3 newVel;
		Vector3 oldVel = getStartVelocity(0);
		for (size_t i = 1; i<nodeCount-1; i++)
		{
			// Equation 12
		    newVel = getEndVelocity(i)*trackVector[i].distToNext + getStartVelocity(i)*trackVector[i-1].distToNext;
			newVel /= (trackVector[i-1].distToNext + trackVector[i].distToNext);
			trackVector[i-1].velocity = oldVel;
			oldVel = newVel;
		}
	    trackVector[nodeCount-1].velocity = getEndVelocity(nodeCount-1);
		trackVector[nodeCount-2].velocity = oldVel;
	}
}
DigiSpeCameraTrack::~DigiSpeCameraTrack()
{
}

void DigiSpeCameraTrack::startAt(float t)
{
	currentTime = t;
	nextPoint = 1;
}
bool DigiSpeCameraTrack::isFinished()
{
	return currentTime >= duration;
}
void DigiSpeCameraTrack::setBlendOutDuration(float blendOutDuration)
{
	this->blendOutDuration = blendOutDuration;
}

Vector3 getPositionOnCubic(const Vector3& startPos, const Vector3& startVel, const Vector3& endPos, const Vector3& endVel, float t)
{
	float t2 = t*t;
	float t3 = t2*t;
	
	Vector3 f1 = 2*startPos - 2*endPos + startVel + endVel;
	Vector3 f2 = -3*startPos + 3*endPos - 2*startVel - endVel;
	Vector3 f3 = startVel;
	Vector3 f4 = startPos;
	
	return t3*f1 + t2*f2 + t*f3 + f4;
}
Vector3 DigiSpeCameraTrack::getStartVelocity(size_t i)
{
	Vector3 temp = 3 * (trackVector[i+1].position - trackVector[i].position).normalisedCopy();
	return (temp - trackVector[i+1].velocity) * 0.5f;
}
Vector3 DigiSpeCameraTrack::getEndVelocity(size_t i)
{
	Vector3 temp = 3 * (trackVector[i].position - trackVector[i-1].position).normalisedCopy();
	return (temp - trackVector[i-1].velocity) * 0.5f;
}

Radian interpolateRotation(Radian a, Radian av, Radian b, Radian bv, float factor)
{
	b = adaptAngle(a, b);
	//return a * (1 - factor) + b * factor;
	
	// rotation(t) = q*t^3 + r*t^2 + s*t + a
	Radian q = av + bv - 2*b + 2*a;
	Radian r = 3*b - 2*av - bv - 3*a;
	Radian s = av;
	
	return ((q * factor + r) * factor + s) * factor + a;
}

void DigiSpeCameraTrack::update(float dt)
{
	currentTime += dt;
	
	if (isFinished())
	{
		CEGUI::WindowManager::getSingleton().getWindow("DigiSpeBlendRect")->setAlpha(1);
		return;
	}
	
	// Get current points
	while (trackVector[nextPoint].time < currentTime)
		++nextPoint;
	
	DigiSpeCameraTrackPoint& p1 = trackVector[nextPoint-1];
	DigiSpeCameraTrackPoint& p2 = trackVector[nextPoint];
	
	float timeDistance = p2.time - p1.time;
	float factor = (currentTime - p1.time) / timeDistance;
	
	Vector3 startVel = p1.velocity * p1.distToNext;
	Vector3 endVel = p2.velocity * p1.distToNext;
	Vector3 pos = getPositionOnCubic(p1.position, startVel, p2.position, endVel, factor);
	
	// TODO: replace this
	/*factor = -2*factor*factor*factor + 3*factor*factor;
	Quaternion yaw = Quaternion::Slerp(factor, p1.yawOrientation, p2.yawOrientation);
	Quaternion pitch = Quaternion::Slerp(factor, p1.pitchOrientation, p2.pitchOrientation);
	Quaternion roll = Quaternion::Slerp(factor, p1.rollOrientation, p2.rollOrientation);*/
	
	Radian yaw = interpolateRotation(p1.yaw, p1.yawVelocity * p1.distToNext, p2.yaw, p2.yawVelocity * p1.distToNext, factor);
	Radian pitch = interpolateRotation(p1.pitch, p1.pitchVelocity * p1.distToNext, p2.pitch, p2.pitchVelocity * p1.distToNext, factor);
	Radian roll = interpolateRotation(p1.roll, p1.rollVelocity * p1.distToNext, p2.roll, p2.rollVelocity * p1.distToNext, factor);
	
	Quaternion qYaw(yaw, Vector3::UNIT_Y);
	Quaternion qPitch(pitch, Vector3::UNIT_X);
	Quaternion qRoll(roll, Vector3::UNIT_Z);
	
	DigiSpeTour::setCamera(pos, qYaw, qPitch, qRoll);
	
	// Blend out
	if (duration - currentTime < blendOutDuration)
	{
		CEGUI::Window* blendRect = CEGUI::WindowManager::getSingleton().getWindow("DigiSpeBlendRect");
		
		float factor = std::min(1.0f, 1.0f - ((duration - currentTime) / blendOutDuration));
		blendRect->setVisible(true);
		blendRect->setAlpha(factor);
	}
}

// ### DigiSpeTour ###

DigiSpeTour::DigiSpeTour(const char* file)
{
	ok = true;
	enteredCave = false;
	cameraTrackRunning = false;
	timePassed = 0.0f;
	
	// Get window handles
	pointName = CEGUI::WindowManager::getSingleton().getWindow("DigiSpeTourPointName");
	pointNameShadow = CEGUI::WindowManager::getSingleton().getWindow("DigiSpeTourPointNameShadow");
	pointDesc = CEGUI::WindowManager::getSingleton().getWindow("DigiSpeTourPointDesc");
	pointDescShadow = CEGUI::WindowManager::getSingleton().getWindow("DigiSpeTourPointDescShadow");
	photo = CEGUI::WindowManager::getSingleton().getWindow("DigiSpePhoto");
	camIcon = CEGUI::WindowManager::getSingleton().getWindow("DigiSpeCamera");
	
	blendRect = CEGUI::WindowManager::getSingleton().getWindow("DigiSpeBlendRect");
	
	// Get fonts
	pointFont = pointName->getFont();
	pointDescFont = pointDesc->getFont();

	// Load tour
	TiXmlDocument doc(file);
	if (!doc.LoadFile())
	{
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: Cannot open tour file (wrong path / invalid XML)!", LML_CRITICAL);
		ok = false;
		return;
	}

	TiXmlHandle docHandle(&doc);
	TiXmlElement* tourElem = docHandle.FirstChildElement().ToElement();
	if (!tourElem)
	{
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: Invalid tour file (no XML element)!", LML_CRITICAL);
		ok = false;
		return;
	}

	if (tourElem->Value() != string("Tour"))
	{
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: Invalid tour file (does not begin with a Tour tag)!", LML_CRITICAL);
		ok = false;
		return;
	}
	
	// Load camera track
	TiXmlElement* cameraTrackElem = tourElem->FirstChildElement("CameraTrack");
	if (cameraTrackElem)
	{
		camTrack = new DigiSpeCameraTrack(cameraTrackElem);
		camTrack->setBlendOutDuration(0.4f);
	}
	else
	{
		camTrack = NULL;
		CEGUI::WindowManager::getSingleton().getWindow("DigiSpeCameraTrack")->setVisible(false);
	}
	
	// Load guided tour
	TiXmlElement* guidedTourElem = tourElem->FirstChildElement("GuidedTour");
	
	TiXmlElement* e = guidedTourElem->FirstChildElement("Start");
	if (e)
	{
		start.position = getPosition(e);
		start.yawOrientation = getYawOrientation(e);
		start.pitchOrientation = getPitchOrientation(e);
		
		setCamera(start.position, start.yawOrientation, start.pitchOrientation);
	}
	
	e = guidedTourElem->FirstChildElement("Point");
	while (e)
	{
		DigiSpeTourPoint newPoint;
		
		newPoint.position = getPosition(e);
		newPoint.yawOrientation = getYawOrientation(e);
		newPoint.pitchOrientation = getPitchOrientation(e);
		
		const char* cStr = e->Attribute("name");
		if (cStr)
			newPoint.name = cStr;
		
		newPoint.photo.setNull();
		cStr = e->Attribute("photo");
		if (cStr)
		{
			string photoPath = string(cStr);
			newPoint.photo = Ogre::TextureManager::getSingleton().load(cStr, "DigiSpe", TEX_TYPE_2D, 0);
			
			if (newPoint.photo.isNull())
			{
				LogManager::getSingleton().logMessage("ERROR: DigiSpe: Could not find photo: " + string(cStr) + " !", LML_CRITICAL);
				ok = false;
				return;
			}
			
			CEGUI::Texture* cTex = &windowManager.getRenderer()->createTexture(newPoint.photo);
			CEGUI::Imageset* imageSet = &CEGUI::ImagesetManager::getSingleton().create(cStr, *cTex);
			imageSet->defineImage(cStr, CEGUI::Point(0.0f, 0.0f),CEGUI::Size(cTex->getSize().d_width, cTex->getSize().d_height), CEGUI::Point(0.0f,0.0f));
			
			newPoint.photoName = CEGUI::PropertyHelper::imageToString(&imageSet->getImage(cStr)).c_str();
		}
		
		cStr = e->Attribute("photoFullscreen");
		newPoint.photoFullscreen = (cStr && cStr == string("true"));
		
		cStr = e->GetText();
		if (cStr)
		{
			size_t len = strlen(cStr);
			char* str = new char[len+1];
			strcpy(str, cStr);
			for (size_t c = 0; c < len; ++c)
			{
				if (str[c] == '^')
					str[c] = '\n';
			}
			newPoint.desc = str;
			delete[] str;
		}
			
		points.push_back(newPoint);
		e = e->NextSiblingElement();
	}
	
	actPoint = -1;
	transition = false;
	
	photoActive = false;
	photoAlpha = 0.0f;
}
DigiSpeTour::~DigiSpeTour()
{
	delete camTrack;
}

void DigiSpeTour::startCameraTrack()
{
	assert(camTrack);
	cameraTrackRunning = true;
	camTrack->startAt(0);
}

void DigiSpeTour::enterCave()
{
	if (transition)
		return;
	
	startTransition(start, points[0]);
	actPoint = 0;
	enteredCave = true;
}
void DigiSpeTour::move(int step)
{
	if (transition)
		return;

	int oldPoint = actPoint;
	actPoint += step;
	
	// Can't go before first or after last point
	if (actPoint < 0)
		actPoint = 0;
	else if (actPoint >= (int)points.size())
		actPoint = (int)points.size() - 1;
	if (actPoint == oldPoint)
		return;
	
	startTransition(points[oldPoint], points[actPoint]);
}
void DigiSpeTour::togglePhoto()
{
	if (transition)
		return;
	
	photoActive = !photoActive;
}

void DigiSpeTour::startTransition(DigiSpeTourPoint& p1, DigiSpeTourPoint& p2)
{
	transition = true;
	transitionDetailsChanged = false;
	transitionTime = 0;
	transitionDuration = 2.8f;	// Seems to be ok to set this to a constant value, regardless of the length
	this->p1 = &p1;
	this->p2 = &p2;
	
	photoActive = false;
}

void DigiSpeTour::updateCameraTrack(float dt)
{
	camTrack->update(dt);
	if (camTrack->isFinished())
	{
		cameraTrackRunning = false;
		setCamera(start.position, start.yawOrientation, start.pitchOrientation);
		
		CEGUI::WindowManager::getSingleton().getWindow("DigiSpeTour")->hide();
		CEGUI::WindowManager::getSingleton().getWindow("DigiSpeStart")->show();
		digiSpe->startBlendIn();
	}
}
void DigiSpeTour::updateGuidedTour(float dt)
{
	// Calculate transition
	transitionTime += dt;
	if (transition && transitionTime >= transitionDuration)
	{
		setCamera(p2->position, p2->yawOrientation, p2->pitchOrientation);
		transition = false;
	}
	else if (transition)
	{
		float factor = transitionTime / transitionDuration;
		factor = -2*factor*factor*factor + 3*factor*factor;
		
		Vector3 pos = p1->position + (p2->position - p1->position) * factor;
		Quaternion yaw = Quaternion::Slerp(factor, p1->yawOrientation, p2->yawOrientation);
		Quaternion pitch = Quaternion::Slerp(factor, p1->pitchOrientation, p2->pitchOrientation);
		
		setCamera(pos, yaw, pitch);
	}
	
	float detailsAlpha;
	if (transitionTime / transitionDuration < 0.5f)
	{
		detailsAlpha = max(0.0f, 1.0f - (transitionTime / 0.4f));
	}
	else
	{
		if (!transitionDetailsChanged)
		{
			pointName->setText(p2->name);
			pointNameShadow->setText(p2->name);
			pointDesc->setText(p2->desc);
			pointDescShadow->setText(p2->desc);
			
			int numLines = getNumberOfLines(p2->desc.c_str());
			float descHeight = numLines * pointDescFont->getFontHeight(); //numLines * fontBold->getBaseline() + (numLines - 1) * fontBold->getLineSpacing();
			float nameHeight = pointFont->getFontHeight();
			
			const int cameraSize = 64;
			float cameraWidth;
			if (p2->photo.isNull())
			{
				photo->setArea(cegui_absdim(0), cegui_absdim(0), cegui_absdim(0), cegui_absdim(0));
				camIcon->setArea(cegui_absdim(0), cegui_absdim(0), cegui_absdim(0), cegui_absdim(0));
				
				cameraWidth = 0;
			}
			else
			{
				if (p2->photoFullscreen)
				{
					float photoAspectRatio = p2->photo->getWidth() / (float)p2->photo->getHeight();
					float screenAspectRatio = (float)vp->getActualWidth() / (float)vp->getActualHeight();
					
					if (photoAspectRatio > screenAspectRatio)
					{
						float scale = screenAspectRatio / photoAspectRatio;
						photo->setArea(cegui_reldim(0), cegui_reldim((1 - scale)/2), cegui_reldim(1), cegui_reldim(scale));
					}
					else
					{
						float scale = photoAspectRatio / screenAspectRatio;
						photo->setArea(cegui_reldim((1 - scale)/2), cegui_reldim(0), cegui_reldim(scale), cegui_reldim(1));
					}
				}
				else
					photo->setArea(cegui_absdim(10), cegui_absdim(10), cegui_absdim(p2->photo->getWidth()), cegui_absdim(p2->photo->getHeight()));
				photo->setProperty("Image", p2->photoName.c_str());
				
				cameraWidth = cameraSize+10;
				float cameraHeight = std::max((float)cameraSize, descHeight);
				camIcon->setArea(cegui_absdim(10), cegui_reldim(1) + cegui_absdim(-(cameraHeight - cameraSize)/2 - cameraSize - 10), cegui_absdim(cameraSize), cegui_absdim(cameraSize));
			}
			
			const int shadow_offset = 2;
			pointDesc->setArea(cegui_absdim(10 + cameraWidth), cegui_reldim(1) + cegui_absdim(-5 - descHeight), cegui_reldim(1) + cegui_absdim(-240 - cameraWidth), cegui_absdim(descHeight));
			pointDescShadow->setArea(pointDesc->getXPosition() + cegui_absdim(shadow_offset), pointDesc->getYPosition() + cegui_absdim(shadow_offset), pointDesc->getWidth(), pointDesc->getHeight());
			pointName->setArea(cegui_absdim(10 + cameraWidth), cegui_reldim(1) + cegui_absdim(-5 - descHeight - 5 - nameHeight), cegui_reldim(1) + cegui_absdim(-240 - cameraWidth), cegui_absdim(nameHeight));
			pointNameShadow->setArea(pointName->getXPosition() + cegui_absdim(shadow_offset), pointName->getYPosition() + cegui_absdim(shadow_offset), pointName->getWidth(), pointName->getHeight());
			
			transitionDetailsChanged = true;
		}
		
		detailsAlpha = max(0.0f, min(1.0f, 1.0f - ((transitionDuration - transitionTime) / 0.4f)));
	}
	
	photoAlpha += 2.0f * (photoActive ? 1 : -1) * dt;
	photoAlpha = std::min(1.0f, std::max(0.0f, photoAlpha));
	
	pointName->setAlpha(detailsAlpha);
	pointNameShadow->setAlpha(detailsAlpha);
	pointDesc->setAlpha(detailsAlpha);
	pointDescShadow->setAlpha(detailsAlpha);
	photo->setAlpha(std::min(detailsAlpha, photoAlpha));
	camIcon->setAlpha(detailsAlpha);
}
void DigiSpeTour::update(float dt)
{
	timePassed += dt;
	
	if (enteredCave)
		updateGuidedTour(dt);
	else if (cameraTrackRunning)
		updateCameraTrack(dt);
}

void DigiSpeTour::setCamera(Vector3 pos, Quaternion yaw, Quaternion pitch, Quaternion roll)
{
	game.setCameraPosition(pos);
	game.setCameraNodeOrientations(yaw, pitch, roll);
}

Vector3 DigiSpeTour::getPosition(TiXmlElement* e)
{
	return StringConverter::parseVector3(e->Attribute("position"));
}
Quaternion DigiSpeTour::getYawOrientation(TiXmlElement* e)
{
	const char* yawOrientationStr = e->Attribute("yawOrientation");
	if (yawOrientationStr)
		return StringConverter::parseQuaternion(yawOrientationStr);
	
	const char* yawStr = e->Attribute("yaw");
	if (yawStr)
		return Quaternion(StringConverter::parseAngle(yawStr), Vector3::UNIT_Y);
	
	return Quaternion::IDENTITY;
}
Quaternion DigiSpeTour::getPitchOrientation(TiXmlElement* e)
{
	const char* pitchOrientationStr = e->Attribute("pitchOrientation");
	if (pitchOrientationStr)
		return StringConverter::parseQuaternion(pitchOrientationStr);
	
	const char* pitchStr = e->Attribute("pitch");
	if (pitchStr)
		return Quaternion(StringConverter::parseAngle(pitchStr), Vector3::UNIT_X);
	
	return Quaternion::IDENTITY;
}
Quaternion DigiSpeTour::getRollOrientation(TiXmlElement* e)
{
	const char* rollOrientationStr = e->Attribute("rollOrientation");
	if (rollOrientationStr)
		return StringConverter::parseQuaternion(rollOrientationStr);
	
	const char* rollStr = e->Attribute("roll");
	if (rollStr)
		return Quaternion(StringConverter::parseAngle(rollStr), Vector3::UNIT_Z);
	
	return Quaternion::IDENTITY;
}
