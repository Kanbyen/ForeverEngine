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
#include "digiSpeElement.h"
#include "digiSpe.h"
#include "tinyxml.h"

const float BORDER_RANGE = 2;
const float DEGREES_TO_RADIANS = (2 * 3.14159f) / 360.0f;

int DigiSpeMeasuringPoint::nextNumber = 0;

// ### DigiSpeExtrusion ###

DigiSpeExtrusion::~DigiSpeExtrusion()
{
	for (int i = 0; i < (int)elements.size(); ++i)
		delete elements[i];
	for (int i = 0; i < (int)auxElements.size(); ++i)
		delete auxElements[i];
}
void DigiSpeExtrusion::getConnection(TiXmlElement* elem)
{
	Connection newConnection;
	
	const char* from = elem->Attribute("from");
	const char* to = elem->Attribute("to");
	if (!from)
	{
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: No 'from' attribute in an extrusion Connection tag!", LML_CRITICAL);
		return;
	}
	if (!to)
	{
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: No 'to' attribute in an extrusion Connection tag!", LML_CRITICAL);
		return;
	}

	newConnection.p1 = digiSpe->getMeasuringPointByName(from);
	newConnection.p2 = digiSpe->getMeasuringPointByName(to);
	
	newConnection.v1 = newConnection.p1->getMidPoint();
	newConnection.v1tov2 = newConnection.p2->getMidPoint() - newConnection.v1;
	newConnection.squaredLength = newConnection.v1tov2.squaredLength();
	
	connections.push_back(newConnection);
}
void DigiSpeExtrusion::set(TiXmlElement* elem)
{
	const char* cTemp;

	cTemp = elem->Attribute("DEACTIVATE");
	if (cTemp)
		deactivated = (cTemp == string("true"));
	else
		deactivated = false;
		
	elem = elem->FirstChildElement();
	while (elem)
	{
		cTemp = elem->Value();
		if (cTemp == string("Point"))
		{
			DigiSpeMeasuringPoint* mp = new DigiSpeMeasuringPoint(elem);
			digiSpe->pointList.push_back(mp);
			if (digiSpe->noDeactivate || !mp->deactivated)
				elements.push_back(mp);
			else
				auxElements.push_back(mp);
		}
		else if (cTemp == string("AuxPoint"))
		{
			DigiSpeMeasuringPoint* mp = new DigiSpeMeasuringPoint(elem);
			digiSpe->pointList.push_back(mp);
			auxElements.push_back(mp);
		}
		else if (cTemp == string("Connection"))
		{
			getConnection(elem);
		}
		else
			LogManager::getSingleton().logMessage("WARNING: DigiSpe: Unknown Tag '" + string(cTemp) + "' inside an Extrusion tag!", LML_CRITICAL);

		elem = elem->NextSiblingElement();
	}
}
void DigiSpeExtrusion::postprocess()
{
	for (int i = 0; i < (int)elements.size(); ++i)
		elements[i]->postprocess();
	for (int i = 0; i < (int)auxElements.size(); ++i)
		auxElements[i]->postprocess();
		
	// Get bounding box
	bbMin = Point3(999999, 999999, 999999);
	bbMax = Point3(-999999, -999999, -999999);
	for (int i = 0; i < (int)elements.size(); ++i)
	{
		DigiSpeMeasuringPoint* mp = elements[i];
		if (mp->bbMax.x > bbMax.x)	bbMax.x = mp->bbMax.x;
		if (mp->bbMax.y > bbMax.y)	bbMax.y = mp->bbMax.y;
		if (mp->bbMax.z > bbMax.z)	bbMax.z = mp->bbMax.z;
		if (mp->bbMin.x < bbMin.x)	bbMin.x = mp->bbMin.x;
		if (mp->bbMin.y < bbMin.y)	bbMin.y = mp->bbMin.y;
		if (mp->bbMin.z < bbMin.z)	bbMin.z = mp->bbMin.z;
	}
}
void DigiSpeExtrusion::postprocess2()
{
	for (int i = 0; i < (int)elements.size(); ++i)
		elements[i]->postprocess2();
	for (int i = 0; i < (int)auxElements.size(); ++i)
		auxElements[i]->postprocess2();
}
void DigiSpeExtrusion::autoImportSamples()
{
	for (int i = 0; i < (int)elements.size(); ++i)
		elements[i]->autoImportSamples();
	for (int i = 0; i < (int)auxElements.size(); ++i)
		auxElements[i]->autoImportSamples();
}
float DigiSpeExtrusion::apply(int x, int y, int z, float in)
{
	// Get the offset from the nearest point on the nearest connection
	float nearestPointDistSq = 9999999.0f;
	float bestLambda = -1;
	Vector3 bestOffset = Vector3::ZERO;
	Connection* bestCon = NULL;
	Vector3 C = Vector3(x, y, z);
	
	for (unsigned int i = 0; i < connections.size(); ++i)
	{
		Connection& con = connections[i];
		Vector3 v1toC = C - con.v1;
		
		float lambda = (v1toC.dotProduct(con.v1tov2)) / con.squaredLength;
		if (lambda < 0)
			lambda = 0;
		else if (lambda > 1)
			lambda = 1;
			
		Vector3 offset = C - (con.v1 + lambda * con.v1tov2);
		float offsetSquaredLength = offset.squaredLength();
		
		if (offsetSquaredLength < nearestPointDistSq)
		{
			nearestPointDistSq = offsetSquaredLength;
			bestLambda = lambda;
			bestOffset = offset;
			bestCon = &con;
		}
	}
	
	// Get radius
	assert(bestLambda != -1);
	float dist = sqrt(nearestPointDistSq);
	float radius;
	#ifdef DIGISPE_RADIUS_CACHE
		if (bestLambda <= 0)
			radius = bestCon->p1->getCachedRadiusForDirection(bestOffset);
		else if (bestLambda >= 1)
			radius = bestCon->p2->getCachedRadiusForDirection(bestOffset);
		else
		{
			float radius1 = bestCon->p1->getCachedRadiusForDirection(bestOffset);
			float radius2 = bestCon->p2->getCachedRadiusForDirection(bestOffset);
			radius = (1 - bestLambda) * radius1 + bestLambda * radius2;
		}
	#else
		if (bestLambda <= 0)
			radius = bestCon->p1->getRadiusForDirection(bestOffset);
		else if (bestLambda >= 1)
			radius = bestCon->p2->getRadiusForDirection(bestOffset);
		else
		{
			float radius1 = bestCon->p1->getRadiusForDirection(bestOffset);
			float radius2 = bestCon->p2->getRadiusForDirection(bestOffset);
			radius = (1 - bestLambda) * radius1 + bestLambda * radius2;
		}
	#endif
	
	// Apply radius
	radius += BORDER_RANGE;
	float value = (radius - dist) / BORDER_RANGE;
	return in - ((value > 0) ? value : 0);
}
	
// ### DigiSpeMeasuringPoint ###

void DigiSpeMeasuringPoint::createSamplePoint(Sample* sample, bool autogenerated)
{
	if (autogenerated)
	{
		// Don't create duplicates!
		for (int i = 0; i < (int)samples.size(); ++i)
		{
			if ((samples[i]->dir * samples[i]->len).distance(sample->dir * sample->len) < 0.001f)
			{
				delete sample;
				return;
			}
		}
	}

	if (sample->flat)
		hasFlatSamplePoint = true;

	if (sample->len > maxradius)
		maxradius = sample->len;
	if (sample->len < minradius)
		minradius = sample->len;

	samples.push_back(sample);

	if (digiSpe->noDeactivate || !deactivated)
	{
		if (digiSpe->showSamples)
		{
			static int debugMeshNumber = 0;

			Entity* entity = sceneMgr->createEntity("DigiSpeDebug" + StringConverter::toString(number) + " (" + StringConverter::toString(debugMeshNumber++) + ")", "ball.mesh");
			entity->setUserAny(Any((Sample*)sample));
			entity->setRenderQueueGroup(RENDER_QUEUE_WORLD_GEOMETRY_2);
			if (autogenerated)
				entity->setMaterialName("CelShadingRed");
			SceneNode* sceneNode = sceneMgr->getRootSceneNode()->createChildSceneNode();
			sceneNode->setPosition((sample->dir * sample->len + midPointF) * 1.5f);
			sceneNode->setScale(Vector3(0.4f / sample->invspread));
			sceneNode->attachObject(entity);
		}
	}
}

DigiSpeMeasuringPoint::Sample* DigiSpeMeasuringPoint::getSamplePoint(TiXmlElement* elem, bool create)
{
	const char* cTemp;
	double dTemp;

	Vector3 sample = getCoords(elem) + sampleRayStartOffset - calcOffset;
	float len = sample.length();

	Sample* newSample = new Sample();
	newSample->pairPartner = NULL;
	newSample->dir = sample.normalisedCopy();
	newSample->len = len;

	cTemp = elem->Attribute("name");
	if (cTemp)
	{
		if ((cTemp[0] != 0) && getSampleByName(cTemp, false))
			LogManager::getSingleton().logMessage("WARNING: DigiSpe: Sample name '" + string(cTemp) + "' used more than once!", LML_CRITICAL);
		newSample->name = cTemp;
	}

	if (elem->Attribute("spread", &dTemp))
		newSample->invspread = 1.0f / dTemp;
	else
		newSample->invspread = 1.0f;

	cTemp = elem->Attribute("mode");
	if (cTemp)
		newSample->flat = (cTemp == string("flat"));
	else
		newSample->flat = false;

	cTemp = elem->Attribute("normal");
	if (cTemp)
		newSample->normal = StringConverter::parseVector3(cTemp).normalisedCopy();
	else if (newSample->flat)
		newSample->normal = -newSample->dir;
	else
		newSample->normal = Vector3::ZERO;
	newSample->planeC = newSample->normal.dotProduct(newSample->dir * newSample->len + midPointF);

	// Create the sample point
	if (create)
		createSamplePoint(newSample);

	// Create connection, if specified
	cTemp = elem->Attribute("connect");
	if (cTemp)
	{
		Sample* partner = prevSample;
		const char* cPartner = elem->Attribute("connectTo");
		if (cPartner)
		{
			partner = getSampleByName(cPartner);
			if (!partner)
				return NULL;
		}
		
		if (cTemp == string("line"))
			createLine(newSample, partner);
		else if (cTemp == string("pair"))
		{
			newSample->pairPartner = partner;
			newSample->connectionCenter = createLine(newSample, partner, true);

			cTemp = elem->Attribute("prevPair");
			if (cTemp)
			{
				Sample* prevPairFirst = getSampleByName(cTemp);
				if (!cTemp)
					return NULL;

				createLine(newSample, prevPairFirst);
				createLine(partner, prevPairFirst->pairPartner);
				createLine(newSample->connectionCenter, prevPairFirst->connectionCenter);
			}
		}
		else
		{
			LogManager::getSingleton().logMessage("ERROR: DigiSpe: Wrong value '" + string(cTemp) + "' specified for a connect attribute!", LML_CRITICAL);
			return NULL;
		}
	}

	prevSample = newSample;
	return newSample;
}
void DigiSpeMeasuringPoint::triangleAdjustN(Line* line, Sample* s)
{
	float dp = line->N.dotProduct(s->dir);
	if (dp < 0)
		line->N = -line->N;
}
void DigiSpeMeasuringPoint::createTriangle(Sample* a, Sample* b, Sample* c, bool addToTriangleA, bool triangulationTriangle)
{
	if (triangulate && !triangulationTriangle)
		return;	// TODO: Account for manual triangles when triangulating

	Triangle* newTriangle = new Triangle();

	newTriangle->a = a;
	newTriangle->b = b;
	newTriangle->c = c;
	createLine(a, b, false, true, &newTriangle->ab, triangulationTriangle);
	createLine(b, c, false, true, &newTriangle->bc, triangulationTriangle);
	createLine(c, a, false, true, &newTriangle->ca, triangulationTriangle);

	// Adjust the line N vectors so that they point in the direction of the remaining triangle corner
	triangleAdjustN(newTriangle->ab, c);
	triangleAdjustN(newTriangle->bc, a);
	triangleAdjustN(newTriangle->ca, b);
	
	// Calculate triangle plane data
	newTriangle->area.redefine(a->dir*a->len, b->dir*b->len, c->dir*c->len);

	// Add the triangle to the list
	triangles.push_back(newTriangle);
	
	if (addToTriangleA)
		a->triangleA.push_back(newTriangle);
}
void DigiSpeMeasuringPoint::getTriangle(TiXmlElement* elem)
{
	const char* a = elem->Attribute("a");
	const char* b = elem->Attribute("b");
	const char* c = elem->Attribute("c");
	if (!a)
	{
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: No 'a' attribute in a Triangle tag!", LML_CRITICAL);
		return;
	}
	if (!b)
	{
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: No 'b' attribute in a Triangle tag!", LML_CRITICAL);
		return;
	}
	if (!c)
	{
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: No 'c' attribute in a Triangle tag!", LML_CRITICAL);
		return;
	}

	Sample* sa = getSampleByName(a, false);
	Sample* sb = getSampleByName(b, false);
	Sample* sc = getSampleByName(c, false);
	
	if (!sa || !sb || !sc)
	{
		// Save triangle data for a second try (when samples from other measuring points have been imported)
		TriangleData data;
		data.a = a;
		data.b = b;
		data.c = c;
		brokenTriangleData.push_back(data);
	}
	else
		createTriangle(sa, sb, sc);
}
DigiSpeMeasuringPoint::Sample* DigiSpeMeasuringPoint::autogenerateSample(Vector3 offset, const char* name)
{
	float len = offset.length();
	if (len == 0)
		return NULL;

	Sample* newSample = new Sample();
	newSample->pairPartner = NULL;
	newSample->dir = offset / len;
	newSample->len = len;
	newSample->name = name;
	newSample->invspread = 1.0f;
	newSample->flat = false;
	newSample->normal = Vector3::ZERO;
	newSample->planeC = 0;
	createSamplePoint(newSample, true);

	return newSample;
}
void DigiSpeMeasuringPoint::getCone(TiXmlElement* elem)
{
	float radius;
	const char* cStr = elem->Attribute("diameter");
	if (cStr)
	{
		istringstream diameterStream(cStr);
		diameterStream >> radius;
		radius *= 0.5f;
		radius *= digiSpe->scale;
	}
	else
	{
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: No 'diameter' attribute in a Cone tag!", LML_CRITICAL);
		return;
	}

	string name = "";
	cStr = elem->Attribute("name");
	if (cStr)
		name = cStr;

	// Create end point
	Vector3 sampleEnd = getCoords(elem, "end") + sampleRayStartOffset - calcOffset;
	autogenerateSample(sampleEnd, ("Cone end of " + name).c_str());

	// Get perpendicular vector
	Vector3 sampleStart = getCoords(elem, "start") + sampleRayStartOffset - calcOffset;
	Vector3 coneDir = sampleEnd - sampleStart;
	Vector3 perp;
	do
	{
		Vector3 rnd = Vector3(Math::RangeRandom(-1, 1), Math::RangeRandom(-1, 1), Math::RangeRandom(-1, 1));
		perp = coneDir.crossProduct(rnd);
	} while (perp == Vector3::ZERO);
	perp = perp.normalisedCopy() * radius;

	// Create two border points
	autogenerateSample(sampleStart + perp, ("Cone side 1 of " + name).c_str());
	autogenerateSample(sampleStart - perp, ("Cone side 2 of " + name).c_str());

	// Get another perpendicular vector
	Vector3 perp2 = perp.crossProduct(coneDir).normalisedCopy() * radius;

	// Create another two border points
	autogenerateSample(sampleStart + perp2, ("Cone side 3 of " + name).c_str());
	autogenerateSample(sampleStart - perp2, ("Cone side 4 of " + name).c_str());
}
DigiSpeMeasuringPoint::Sample* DigiSpeMeasuringPoint::createLine(Sample* s1, Sample* s2, bool indent, bool addToLineTo, Line** out, bool triangulationTriangleLine)
{
	if (!indent && triangulate && !triangulationTriangleLine)
		return NULL;	// TODO: Account for manual lines when triangulating

	assert(s1 && "Sample 1 for line not found!");
	assert(s2 && "Sample 2 for line not found!");

	Vector3 s1pos = s1->dir * s1->len;
	Vector3 s2pos = s2->dir * s2->len;
	float s1s2dist = s1pos.distance(s2pos);

	if (indent)
	{
		// Create helper sample point and connect
		Sample* newSample = new Sample();
		newSample->pairPartner = NULL;
		Vector3 sample = (s1pos + s2pos) / 2.0f;
		float len = sample.length();
		len += s1s2dist * 0.75f;
		newSample->dir = sample.normalisedCopy();
		newSample->len = len;
		newSample->name = "Line from '" + s1->name + "' to '" + s2->name + "'";
		newSample->invspread = 1.0f / (1.0f /*spreadFactor*/ * ((1.0f / s1->invspread) + (1.0f / s2->invspread)) / 2.0f);
		newSample->flat = false;
		newSample->normal = Vector3::ZERO; //(s1->normal + s2->normal).normalisedCopy();
		newSample->planeC = 0; //newSample->normal.dotProduct(newSample->dir * newSample->len + midPointF);
		createSamplePoint(newSample, true);

		createLine(s1, newSample, false);
		createLine(newSample, s2, false);

		return newSample;
	}
	else
	{
		Line* newLine = new Line();
		newLine->s1 = s1;
		newLine->s2 = s2;
		newLine->A = s1pos;
		newLine->B = s2pos;
		newLine->AB = s2pos - s1pos;
		newLine->N = s1pos.crossProduct(s2pos).normalisedCopy();
		lines.push_back(newLine);
		
		if (addToLineTo)
			s1->lineTo.push_back(s2);
		
		if (out)
			*out = newLine;

		return NULL;
	}
}
void DigiSpeMeasuringPoint::getLine(TiXmlElement* elem)
{
	const char* from = elem->Attribute("from");
	const char* to = elem->Attribute("to");
	if (!from)
	{
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: No 'from' attribute in a Line tag!", LML_CRITICAL);
		return;
	}
	if (!to)
	{
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: No 'to' attribute in a Line tag!", LML_CRITICAL);
		return;
	}

	Sample* s1 = getSampleByName(from, false);
	Sample* s2 = getSampleByName(to, false);
	
	if (!s1 || !s2)
	{
		// Save line data for a second try (when samples from other measuring points have been imported)
		LineData data;
		data.from = from;
		data.to = to;
		brokenLineData.push_back(data);
	}
	else
		createLine(s1, s2, false);
}

void DigiSpeMeasuringPoint::doTriangulate()
{
	if (samples.size() < 4)
		return;

	std::list< TriangulationTriangle > result;
	
	// Add first three points to the triangulation, look for a fourth point which is not in the plane specified by the first three points
	Plane firstThreePlane = Plane(samples[0]->dir, samples[1]->dir, samples[2]->dir);
	unsigned int fourthPoint = 0;
	bool ok = false;
	for (unsigned int i = 3; i < samples.size(); ++i)
	{
		if (fabs(firstThreePlane.getDistance(samples[i]->dir)) >= 1e-5f)
		{
			// Found a fitting fourth point
			fourthPoint = i;
			ok = true;
			break;
		}
	}
	
	if (!ok)
	{
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: Triangulation failed, reason: could not find a starting tetrahedron. All sample directions are inside a plane.", LML_CRITICAL);
		return;
	}
	
	// Create starting tetrahedron
	triangulationAddToStartingTetrahedron(samples[0], samples[1], samples[2], samples[fourthPoint], result);
	triangulationAddToStartingTetrahedron(samples[fourthPoint], samples[1], samples[2], samples[0], result);
	triangulationAddToStartingTetrahedron(samples[0], samples[fourthPoint], samples[2], samples[1], result);
	triangulationAddToStartingTetrahedron(samples[0], samples[1], samples[fourthPoint], samples[2], result);
	
	// Add remaining samples to the triangulation
	for (unsigned int i = 3; i < samples.size(); ++i)
	{
		if (i == fourthPoint)
			continue;
		
		triangulationUpdate(samples[i], result);
	}
	
	// Apply triangulation.
	// TODO: Account for existing lines and triangles
	for (std::list< TriangulationTriangle >::iterator it = result.begin(); it != result.end(); ++it)
	{
		TriangulationTriangle* actTri = &(*it);
		
		createTriangle(actTri->a, actTri->b, actTri->c, false, true);
	}
}
void DigiSpeMeasuringPoint::triangulationAddToStartingTetrahedron(Sample* a, Sample* b, Sample* c, Sample* innerDirectionSample, std::list< TriangulationTriangle >& result)
{
	TriangulationTriangle newTri;
	newTri.a = a;
	newTri.b = b;
	newTri.c = c;
	newTri.area.redefine(a->dir, b->dir, c->dir);

	float dp = newTri.area.normal.dotProduct(innerDirectionSample->dir - a->dir);
	assert(dp != 0);
	if (dp < 0)
		newTri.area.normal = -newTri.area.normal;
	
	result.push_back(newTri);
}
void DigiSpeMeasuringPoint::triangulationAddToEdgeList(Sample* a, Sample* b, std::list< TriangulationEdge >& edgeList)
{
	for (std::list< TriangulationEdge >::iterator it = edgeList.begin(); it != edgeList.end(); ++it)
	{
		TriangulationEdge* actEdge = &(*it);
		if ((actEdge->a == a && actEdge->b == b) || (actEdge->b == a && actEdge->a == b))
		{
			// Delete existing entry
			edgeList.erase(it);
			return;
		}
	}
	
	// Add new entry
	TriangulationEdge newEdge;
	newEdge.a = a;
	newEdge.b = b;
	edgeList.push_back(newEdge);
}
void DigiSpeMeasuringPoint::triangulationUpdate(Sample* s, std::list< TriangulationTriangle >& result)
{
	std::list< TriangulationEdge > edgeList;
	
	// Find all triangles facing the new sample and delete them,
	// save edges of deleted triangles in a list (remove edges deleted twice from the list again)
	for (std::list< TriangulationTriangle >::iterator it = result.begin(); it != result.end(); )
	{
		TriangulationTriangle* actTri = &(*it);
		float dp = actTri->area.normal.dotProduct(s->dir - actTri->a->dir);
		if (dp <= 0)	// TODO: Is <= 0 ok, or is < 0 better?!
		{
			// Remove the triangle
			triangulationAddToEdgeList(actTri->a, actTri->b, edgeList);
			triangulationAddToEdgeList(actTri->b, actTri->c, edgeList);
			triangulationAddToEdgeList(actTri->c, actTri->a, edgeList);
			
			std::list< TriangulationTriangle >::iterator nextIt = it;
			++nextIt;
			result.erase(it);
			it = nextIt;
		}
		else
			++it;
	}
	
	if (edgeList.size() < 3)
	{
		Ogre::LogManager::getSingleton().logMessage("DigiSpe: WARNING: Sample " + s->name + " skipped in triangulation because edgeList.size() < 3!");
		return;
	}
	
	// Calculate midpoint of all border corners (all corners wil be counted twice, but that doesn't matter)
	Vector3 innerDirectionSample = Vector3(0.0f);
	for (std::list< TriangulationEdge >::iterator it = edgeList.begin(); it != edgeList.end(); ++it)
	{
		TriangulationEdge* actEdge = &(*it);
		innerDirectionSample += actEdge->a->dir;
		innerDirectionSample += actEdge->b->dir;
	}
	innerDirectionSample *= 1 / (float)(edgeList.size() * 2);
	
	// Connect border edges to new sample
	for (std::list< TriangulationEdge >::iterator it = edgeList.begin(); it != edgeList.end(); ++it)
	{
		TriangulationEdge* actEdge = &(*it);
		
		TriangulationTriangle newTri;
		newTri.a = actEdge->a;
		newTri.b = actEdge->b;
		newTri.c = s;
		newTri.area.redefine(newTri.a->dir, newTri.b->dir, newTri.c->dir);
		
		// Find any other, different sample on the triangulation
		/*Sample* innerDirectionSample = NULL;
		for (std::list< TriangulationTriangle >::iterator it = result.begin(); it != result.end(); ++it)
		{
			TriangulationTriangle* actTri = &(*it);
			if (actTri->a != newTri.a && actTri->a != newTri.b && actTri->a != newTri.c &&
			    fabs(newTri.area.normal.dotProduct(actTri->a->dir - newTri.a->dir)) >= 1e-8f)
			{
				innerDirectionSample = actTri->a;
				break;
			}
			if (actTri->b != newTri.a && actTri->b != newTri.b && actTri->b != newTri.c &&
			    fabs(newTri.area.normal.dotProduct(actTri->b->dir - newTri.a->dir)) >= 1e-8f)
			{
				innerDirectionSample = actTri->b;
				break;
			}
			if (actTri->c != newTri.a && actTri->c != newTri.b && actTri->c != newTri.c &&
			    fabs(newTri.area.normal.dotProduct(actTri->c->dir - newTri.a->dir)) >= 1e-8f)
			{
				innerDirectionSample = actTri->c;
				break;
			}
		}
		assert(innerDirectionSample);
		*/
		
		// Make sure that the plane normal points to the inner side
		float dp = newTri.area.normal.dotProduct(innerDirectionSample - newTri.a->dir);
		if (dp < 0)
			newTri.area.normal = -newTri.area.normal;
		
		result.push_back(newTri);
	}
}

DigiSpeMeasuringPoint::Sample* DigiSpeMeasuringPoint::importSample(DigiSpeMeasuringPoint* fromPoint, const char* name)
{
	Sample* sample = fromPoint->getSampleByName(name);
	if (!sample)
		return NULL;
		
	Vector3 toSample = (fromPoint->midPointF + sample->dir * sample->len) - midPointF;
	float len = toSample.length();
	if (len == 0)
		return NULL;

	Sample* newSample = new Sample();
	newSample->pairPartner = NULL;
	newSample->dir = toSample / len;
	newSample->len = len;
	newSample->name = sample->name;
	newSample->invspread = sample->invspread;
	newSample->flat = sample->flat;
	newSample->normal = sample->normal;
	newSample->planeC = sample->planeC;
	newSample->lineTo = sample->lineTo;
	newSample->triangleA = sample->triangleA;
	createSamplePoint(newSample, false);
	
	return newSample;
}
void DigiSpeMeasuringPoint::postprocess()
{
	char buffer[128];
	const char* cTemp;

	// Process GetSample tags
	std::vector<Sample*> importedSamples;
	
	TiXmlElement* elem = this->elem->FirstChildElement("GetSample");
	bool noFlatSamplePointBefore = (!hasFlatSamplePoint);
	while (elem)
	{
		cTemp = elem->Attribute("fromPoint");
		if (!cTemp)
		{
			LogManager::getSingleton().logMessage("ERROR: DigiSpe: No 'fromPoint' attribute in a GetSample tag!", LML_CRITICAL);
			continue;
		}
		DigiSpeMeasuringPoint* fromPoint = digiSpe->getMeasuringPointByName(cTemp);
		if (!fromPoint)
			continue;
	
		int start, end;
		if (elem->Attribute("start", &start))
		{
			if (!elem->Attribute("end", &end))
			{
				LogManager::getSingleton().logMessage("ERROR: DigiSpe: No 'end' attribute in a GetSample tag!", LML_CRITICAL);
				continue;
			}
		
			if (start > end)
			{
				int iTemp = start;
				start = end;
				end = iTemp;
			}
			
			cTemp = elem->Attribute("mask");
			if (!cTemp)
			{
				LogManager::getSingleton().logMessage("ERROR: DigiSpe: No 'mask' attribute in a GetSample tag!", LML_CRITICAL);
				continue;
			}
			string mask = cTemp;
			
			for (int s = start; s <= end; ++s)
			{
				sprintf(buffer, mask.c_str(), s);
				
				importedSamples.push_back(importSample(fromPoint, buffer));
			}
		}
		else
		{
			cTemp = elem->Attribute("id");
			if (!cTemp)
			{
				LogManager::getSingleton().logMessage("ERROR: DigiSpe: Neither 'start' nor 'id' attribute in a GetSample tag!", LML_CRITICAL);
				continue;
			}
			
			importedSamples.push_back(importSample(fromPoint, cTemp));
		}

		elem = elem->NextSiblingElement("GetSample");
	}
	if (noFlatSamplePointBefore && hasFlatSamplePoint)
		maxradius += 50;
		
	// Check for lines and triangles between imported samples and create them also for this measuring point
	for (unsigned int i = 0; i < importedSamples.size(); ++i)
	{
		Sample* sample = importedSamples[i];
		
		// Lines
		for (unsigned int l = 0; l < sample->lineTo.size(); ++l)
		{
			// Is the connection partner also taken over?
			Sample* connectionPartner = getSampleByName(sample->lineTo[l]->name.c_str(), false);
			if (connectionPartner)
				createLine(sample, connectionPartner, false, false);	// Yes, take over the line
		}
		
		// Triangles
		for (unsigned int t = 0; t < sample->triangleA.size(); ++t)
		{
			// Are the other corners also taken over?
			Triangle* tri = sample->triangleA[t];
			Sample* b = getSampleByName(tri->b->name.c_str(), false);
			if (!b)
				continue;
			Sample* c = getSampleByName(tri->c->name.c_str(), false);
			if (!c)
				continue;
			
			createTriangle(sample, b, c, false);	// Yes, take over the triangle
		}
	}
		
	// Try to create lines where partners were missing (they could have been imported by now)
	for (unsigned int i = 0; i < brokenLineData.size(); ++i)
	{
		Sample* s1 = getSampleByName(brokenLineData[i].from.c_str());
		Sample* s2 = getSampleByName(brokenLineData[i].to.c_str());
		
		if (s1 && s2)
			createLine(s1, s2, false);
	}
	brokenLineData.clear();
	
	// Try to create triangles where partners were missing (they could have been imported by now)
	for (unsigned int i = 0; i < brokenTriangleData.size(); ++i)
	{
		Sample* a = getSampleByName(brokenTriangleData[i].a.c_str());
		Sample* b = getSampleByName(brokenTriangleData[i].b.c_str());
		Sample* c = getSampleByName(brokenTriangleData[i].c.c_str());
		
		if (a && b && c)
			createTriangle(a, b, c);
	}
	brokenTriangleData.clear();
	
	if (triangulate)
		doTriangulate();
	
	
	if (hasFlatSamplePoint)
		maxradius += 50;
	
	// Just to be on the safe side ...
	maxradius += BORDER_RANGE;
	minradius -= BORDER_RANGE;
	
	// Calculate bounding box
	int bbRange = maxradius + BORDER_RANGE;
	bbMin = Point3(midPointF.x - bbRange, midPointF.y - bbRange, midPointF.z - bbRange);
	bbMax = Point3(midPointF.x + bbRange, midPointF.y + bbRange, midPointF.z + bbRange);
}
void DigiSpeMeasuringPoint::autoImportSamples()
{
	if (samples.size() == 0)
		return;	// Don't import samples if this is an aux point
	
	DigiSpe::MeasuringPointList::iterator itend = digiSpe->pointList.end();
	for (DigiSpe::MeasuringPointList::iterator it = digiSpe->pointList.begin(); it != itend; ++it)
	{
		DigiSpeMeasuringPoint* mp = *it;
		if (mp == this)
			continue;
		
		std::vector<Sample*>::iterator sampleEnd = mp->samples.end();
		for (std::vector<Sample*>::iterator sampleIt = mp->samples.begin(); sampleIt != sampleEnd; ++sampleIt)
		{
			Sample* s = *sampleIt;
			
			Vector3 absPos = mp->midPointF + s->len * s->dir;
			Vector3 relPos = absPos - midPointF;
			
			// Get radius
			float radius = getRadiusForDirection(relPos);
			
			if (radius*radius <= relPos.squaredLength())
				continue;	// sample is not contained
			
			// Check if sample is already imported
			if (getSampleByName(s->name.c_str(), false) != NULL)
				continue;
			
			Ogre::LogManager::getSingleton().logMessage("DigiSpe: Point " + name + " auto-imported sample " + s->name);
			
			// Include sample
			samplesToAutoImport.push_back(std::pair<DigiSpeMeasuringPoint*, std::string>(mp, s->name));
		}
	}
}
void DigiSpeMeasuringPoint::postprocess2()
{
	if (samplesToAutoImport.size() > 0)
	{
		for (size_t i = 0; i < samplesToAutoImport.size(); ++i)
			importSample(samplesToAutoImport[i].first, samplesToAutoImport[i].second.c_str());
		samplesToAutoImport.clear();
		
		if (triangulate)
		{
			// Retriangulate
			triangles.clear();
			doTriangulate();
		}	
	}
	
	// Build radius cache. This is the very last thing to do.
	#ifdef DIGISPE_RADIUS_CACHE
		buildRadiusCache();
	#endif
}
void DigiSpeMeasuringPoint::saveOBJ(FILE* file, int& vIndex)
{
	string str;
	str = "g element" + name + "\r\n\r\n";
	fwrite(str.c_str(), str.size(), 1, file);
	
	for (int i = 0; i < (int)triangles.size(); ++i)
	{
		Triangle* t = triangles[i];
		
		Vector3 absA = (t->a->len * t->a->dir) + midPointF;
		str = "v " + StringConverter::toString(absA) + "\r\n";
		
		Vector3 absB = (t->b->len * t->b->dir) + midPointF;
		str += "v " + StringConverter::toString(absB) + "\r\n";
		
		Vector3 absC = (t->c->len * t->c->dir) + midPointF;
		str += "v " + StringConverter::toString(absC) + "\r\n";
		
		Vector3 cross = t->ab->AB.crossProduct(t->bc->AB);
		if (cross.dotProduct(t->a->dir) < 0)
		{
			str += "f " + StringConverter::toString(vIndex) + " ";
			++vIndex;
			str += StringConverter::toString(vIndex) + " ";
			++vIndex;
			str += StringConverter::toString(vIndex) + "\r\n";
			++vIndex;
		}
		else
		{
			vIndex += 2;
			str += "f " + StringConverter::toString(vIndex) + " ";
			--vIndex;
			str += StringConverter::toString(vIndex) + " ";
			--vIndex;
			str += StringConverter::toString(vIndex) + "\r\n";
			vIndex += 3;
		}
		
		fwrite(str.c_str(), str.size(), 1, file);
	}
}
void DigiSpeMeasuringPoint::set(TiXmlElement* elem)
{
	const char* cTemp;

	number = nextNumber++;
	sampleRayStartOffset = Vector3::ZERO;
	this->elem = new TiXmlElement(*elem);

	// Get attributes of <Point>
	cTemp = elem->Attribute("name");
	if (cTemp)
		name = cTemp;
	
	cTemp = elem->Attribute("lenAdjustment");
	if (cTemp)
		lenAdjustment = StringConverter::parseReal(cTemp);
	else
		lenAdjustment = 0;

	calcOffset = Vector3::ZERO;
	midPointF = Vector3::ZERO;
	midPointF = getCoords(elem);
	
	bool hasPointOffset = false;
	cTemp = elem->Attribute(("pointOffset"));
	if (cTemp)
	{
		hasPointOffset = true;
		pointOffset = digiSpe->scale * StringConverter::parseVector3(cTemp);
		if (lenAdjustment != 0 && !pointOffset.isZeroLength())
		{
			float vecLength = pointOffset.length();
			pointOffset *= (vecLength + lenAdjustment) / vecLength;
		}
	}
	else
		pointOffset = Vector3::ZERO;

	cTemp = elem->Attribute("calcOffset");
	if (cTemp)
		calcOffset = digiSpe->scale * StringConverter::parseVector3(cTemp);
	else
		calcOffset = Vector3::ZERO;
	midPointF += calcOffset;
	midPoint = Point3(midPointF.x, midPointF.y, midPointF.z);

	cTemp = elem->Attribute("DEACTIVATE");
	if (cTemp)
		deactivated = (cTemp == string("true"));
	else
		deactivated = false;
	
	cTemp = elem->Attribute("triangulate");
	if (cTemp)
		triangulate = (cTemp == string("true"));
	else
		triangulate = true;

	if (digiSpe->showSamples)
	{
		Entity* entity = sceneMgr->createEntity("DigiSpeDebugMP" + StringConverter::toString(number), "ball.mesh");
		entity->setRenderQueueGroup(RENDER_QUEUE_WORLD_GEOMETRY_2);
		entity->setMaterialName("CelShadingOrange");
		SceneNode* sceneNode = sceneMgr->getRootSceneNode()->createChildSceneNode();
		sceneNode->setPosition(midPointF * 1.5f);
		sceneNode->setScale(Vector3(0.5f));
		sceneNode->attachObject(entity);
	}
	
	// Create pointOffset sample
	if (hasPointOffset)
	{
		Sample* newSample = new Sample();
		newSample->pairPartner = NULL;
		newSample->dir = (-pointOffset).normalisedCopy();
		newSample->len = pointOffset.length();
		newSample->name = "_pointOffset";
		newSample->invspread = 1;
		newSample->flat = false;
		newSample->normal = Vector3::ZERO;
		newSample->planeC = newSample->normal.dotProduct(newSample->dir * newSample->len + midPointF);
		createSamplePoint(newSample, false);			
	}

	// Get sample points
	elem = elem->FirstChildElement();
	maxradius = 0;
	minradius = 99999999;
	hasFlatSamplePoint = false;
	while (elem)
	{
		cTemp = elem->Value();
		if (cTemp == string("Sample"))
			getSamplePoint(elem);
		else if (cTemp == string("SetSampleRayStartOffset"))
			sampleRayStartOffset = StringConverter::parseVector3(elem->Attribute("offset")) * digiSpe->scale;
		else if (cTemp == string("Line"))
			getLine(elem);
		else if (cTemp == string("Triangle"))
			getTriangle(elem);
		else if (cTemp == string("Cone"))
			getCone(elem);
		/*else
			LogManager::getSingleton().logMessage("WARNING: DigiSpe: Unknown tag '" + string(cTemp) + "' in a Point!", LML_CRITICAL);*/

		elem = elem->NextSiblingElement();
	}
}
DigiSpeMeasuringPoint::~DigiSpeMeasuringPoint()
{
	for (int i = 0; i < (int)samples.size(); ++i)
		delete samples[i];
	for (int i = 0; i < (int)lines.size(); ++i)
		delete lines[i];
	for (int i = 0; i < (int)triangles.size(); ++i)
		delete triangles[i];
		
	delete elem;
}

DigiSpeMeasuringPoint::Sample* DigiSpeMeasuringPoint::getSampleByName(const char* name, bool errorIfNotFound)
{
	for (int i = 0; i < (int)samples.size(); ++i)
	{
		if (samples[i]->name == name)
			return samples[i];
	}

	if (errorIfNotFound)
		LogManager::getSingleton().logMessage("ERROR: DigiSpe: Sample point with name " + string(name) + " not found!", LML_CRITICAL);
	return NULL;
}
Vector3 DigiSpeMeasuringPoint::getCoords(TiXmlElement* elem, string prefix)
{
	if (!elem)
		return Vector3::ZERO;

	Vector3 retVal;

	const char* cStr;
	cStr = elem->Attribute((prefix + "len").c_str());
	if (cStr)
	{
		// len[gth], inc[lination] (-90 to 90) and dir[ection] (0 to 360) attributes
		float len;
		istringstream lenStream(cStr);
		lenStream >> len;
		len += lenAdjustment;
		if (len < 0)
			LogManager::getSingleton().logMessage("WARNING: DigiSpe: value for len attribute is < 0!", LML_CRITICAL);

		cStr = elem->Attribute((prefix + "inc").c_str());
		if (!cStr)
		{
			LogManager::getSingleton().logMessage("ERROR: DigiSpe: Sample point with len attribute, but without inc attribute!", LML_CRITICAL);
			return Vector3::ZERO;
		}

		float inc;
		if (cStr == string("prev"))
			inc = prevInc;
		else
		{
			istringstream incStream(cStr);
			if (incStream.fail())
				LogManager::getSingleton().logMessage("WARNING: DigiSpe: value '" + string(cStr) + "' for inc attribute could not be read!", LML_CRITICAL);
			incStream >> inc;
			inc = -inc;	// 90 is top, -90 is bottom
			prevInc = inc;
			if (inc < -90 || inc > 90)
				LogManager::getSingleton().logMessage("WARNING: DigiSpe: value for inc attribute is out of range!", LML_CRITICAL);
		}

		cStr = elem->Attribute((prefix + "dir").c_str());
		if (!cStr)
		{
			LogManager::getSingleton().logMessage("ERROR: DigiSpe: Sample point with len and inc attribute, but without dir attribute!", LML_CRITICAL);
			return Vector3::ZERO;
		}

		float dir;
		if (cStr == string("prev"))
			dir = prevDir;
		else
		{
			istringstream dirStream(cStr);
			if (dirStream.fail())
				LogManager::getSingleton().logMessage("WARNING: DigiSpe: value '" + string(cStr) + "' for dir attribute could not be read!", LML_CRITICAL);
			dirStream >> dir;
			if (dir < 0)
				dir += 360;
			prevDir = dir;
			
			if (dir < 0 || dir > 360)
				LogManager::getSingleton().logMessage("WARNING: DigiSpe: value '" + string(cStr) + "' for dir attribute is out of range!", LML_CRITICAL);
		}

		// Calculate coordinates
		inc += 90;
		inc *= DEGREES_TO_RADIANS;
		dir *= DEGREES_TO_RADIANS;
		retVal = len * Vector3(sinf(inc) * cosf(dir), cosf(inc), sinf(inc) * sinf(dir));

		// Check for pointOffset attribute
		cStr = elem->Attribute((prefix + "pointOffset").c_str());
		if (cStr)
		{
			Vector3 offset = StringConverter::parseVector3(cStr);
			if (lenAdjustment != 0 && !offset.isZeroLength())
			{
				float vecLength = offset.length();
				offset *= (vecLength + lenAdjustment) / vecLength;
			}
			retVal += offset;
		}
	}
	else
	{
		if (elem->Attribute((prefix + "inc").c_str()) || elem->Attribute((prefix + "dir").c_str()))
			LogManager::getSingleton().logMessage("WARNING: DigiSpe: inc or dir attribute in a sample point without len attribute!", LML_CRITICAL);

		// Check for pos[ition] attribute
		cStr = elem->Attribute((prefix + "pos").c_str());
		if (!cStr)
			retVal = Vector3::ZERO;
		else
		{
			Vector3 offset = StringConverter::parseVector3(cStr);
			if (lenAdjustment != 0 && !offset.isZeroLength())
			{
				float vecLength = offset.length();
				offset *= (vecLength + lenAdjustment) / vecLength;
			}
			retVal = offset;
		}
	}

	// Apply scale
	retVal *= digiSpe->scale;

	// Check for relTo attribute
	cStr = elem->Attribute((prefix + "relTo").c_str());
	if (cStr)
	{
		// First, look for a sample with the given name
		Sample* relSample = getSampleByName(cStr, false);
		if (relSample)
			retVal += relSample->dir * relSample->len + calcOffset;	// add calcOffset because it will be subtracted again
		else
		{
			DigiSpeMeasuringPoint* relPoint = digiSpe->getMeasuringPointByName(cStr);
			if (relPoint)
			{
				cStr = elem->Attribute((prefix + "relMode").c_str());
				if (cStr && cStr == string("withoutOffset"))
					retVal = (relPoint->midPointF - relPoint->calcOffset - relPoint->pointOffset + retVal) - (midPointF - calcOffset);
				else
					retVal = (relPoint->midPointF - relPoint->calcOffset + retVal) - (midPointF - calcOffset);
				
			}
			else
				LogManager::getSingleton().logMessage("ERROR: DigiSpe: Point '" + string(cStr) + "', specified for a relTo attribute, not found!", LML_CRITICAL);
		}
	}

	return retVal;
}

#ifdef DIGISPE_RADIUS_CACHE
void DigiSpeMeasuringPoint::buildRadiusCache()
{
	for (int iInc = 0; iInc < INC_CACHE_SIZE; ++iInc)
	{
		float inc = DEGREES_TO_RADIANS * ((90.0f - (iInc / (float)(INC_CACHE_SIZE - 1)) * 180.0f) + 90.0f);	// begin at the bottom
		for (int iDir = 0; iDir < DIR_CACHE_SIZE; ++iDir)
		{
			float dir = DEGREES_TO_RADIANS * (0.0f + (iDir / (float)(DIR_CACHE_SIZE - 1)) * 360.0f);

			// calculate Vector
			Vector3 direction = Vector3(sinf(inc) * cosf(dir), cosf(inc), sinf(inc) * sinf(dir));

			// save calculated radius
			radiusCache[iInc][iDir] = getRadiusForDirection(direction);
		}
	}
}
float DigiSpeMeasuringPoint::getCachedRadiusForDirection(Vector3& direction)
{
	if (direction == Vector3::ZERO)
		return -2 * BORDER_RANGE;	// just to be sure ...

	float inc = (3.14159f / 2.0f) - atan(direction.y / (sqrt(direction.x * direction.x + direction.z * direction.z)));	// [0; pi]
	float dir = atan2(direction.z, direction.x);	// [-pi; pi]
	if (dir < 0)
		dir = (3.14159f * 2.0f) + dir;

	int iInc = (INC_CACHE_SIZE - 1) - (int)(0.5f + (INC_CACHE_SIZE - 1) * inc / (3.14159f));
	int iDir = (int)(0.5f + (DIR_CACHE_SIZE - 1) * dir / (3.14159f * 2.0f));

	assert((iInc >= 0) && (iInc < INC_CACHE_SIZE));
	assert((iDir >= 0) && (iDir < DIR_CACHE_SIZE));

	return radiusCache[iInc][iDir];
}
#endif

float DigiSpeMeasuringPoint::getRadiusForDirection(Vector3& direction)
{
	const int MAX_SAMPLES = 2048;

	float radius = 0;
	float totalWeight = 0;
	float sampleWeight[MAX_SAMPLES];
	float sampleRadius[MAX_SAMPLES];
	float sampleInvSpread[MAX_SAMPLES];
	int numRelevantSamples;
	float smallestAngle = -1;
	float secondSmallestAngle = -1;
	Vector3 normVec = direction.normalisedCopy();
	
	// Check if we are inside a triangle
	for (int i = 0; i < (int)triangles.size(); ++i)
	{
		Triangle* t = triangles[i];
		if (t->ab->N.dotProduct(normVec) < 0)
			continue;
		if (t->bc->N.dotProduct(normVec) < 0)
			continue;
		if (t->ca->N.dotProduct(normVec) < 0)
			continue;
		
		// Yes! Check where the ray intersects the triangle plane
		float intersectionRadius = -t->area.d / (t->area.normal.dotProduct(normVec));
			
		// TEST: smoother interpolation
		/*
		Vector3 point = normVec * intersectionRadius;
		Vector3 x = t->a->dir * t->a->len;
		Vector3 y = t->b->dir * t->b->len;
		Vector3 z = t->c->dir * t->c->len;
		
		float det = x[0]*y[1]*z[2] + y[0]*z[1]*x[2] + z[0]*x[1]*y[2] - z[0]*y[1]*x[2] - z[2]*y[0]*x[1] - z[1]*y[2]*x[0];
		float ma = y[1]*z[2] - y[2]*z[1];
		float mb = z[0]*y[2] - y[0]*z[2];
		float mc = y[0]*z[1] - z[0]*y[1];
		float md = z[1]*x[2] - x[1]*z[2];
		float me = x[0]*z[2] - z[0]*x[2];
		float mf = z[0]*x[1] - x[0]*z[1];
		float mg = x[1]*y[2] - y[1]*x[2];
		float mh = y[0]*x[2] - x[0]*y[2];
		float mi = x[0]*y[1] - y[0]*x[1];
		
		Vector3 coordsInBasis = (1.0f / det) * Vector3(ma*point.x + mb*point.y + mc*point.z, md*point.x + me*point.y + mf*point.z, mg*point.x + mh*point.y + mi*point.z);
		
		float sum = 0;
		float a = coordsInBasis[0];
		float b = coordsInBasis[1];
		float c = coordsInBasis[2];
		a = a*a;
		b = b*b;
		c = c*c;
		sum = a + b + c;
		
		return t->a->len * (a / sum) + t->b->len * (b / sum) + t->c->len * (c / sum);
		*/
		// END TEST
		
		return intersectionRadius;
	}

	// Find relevant samples ...
	numRelevantSamples = 0;
	for (int i = 0; i < (int)samples.size(); ++i)
	{
		float dp = samples[i]->dir.dotProduct(normVec);

		/*float dpbias = 0.766f; // => 40°       //0.8191f;	// => sample influence of 35°
		if (samples[i]->invspread != 1.0f)
		{
			float biaschange = (1.0f - (1.0f / samples[i]->invspread));
			if (biaschange < 0)
				dpbias -= biaschange * (1 - dpbias);
			else	// biaschange > 0
				dpbias -= biaschange * 0.1f;	// NOTE: => highest possible value for spread is ca. 8
		}

		dp = (dp - dpbias) / (1 - dpbias);*/
		if (dp <= 0)
			continue;

		sampleWeight[numRelevantSamples] = dp; // * (1.0f / samples[i]->invspread);
		sampleInvSpread[numRelevantSamples] = samples[i]->invspread;
		if (sampleWeight[numRelevantSamples] > smallestAngle)
		{
			secondSmallestAngle = smallestAngle;
			smallestAngle = sampleWeight[numRelevantSamples];
		}
		else if (sampleWeight[numRelevantSamples] > secondSmallestAngle)
			secondSmallestAngle = sampleWeight[numRelevantSamples];

		if (samples[i]->flat)
		{
			float denominator = samples[i]->normal.dotProduct(normVec);
			if (denominator >= 0)
				continue; //sampleRadius[numRelevantSamples] = 999;	// "very much" because the plane is not intersected by the ray from midpoint to (x,y,z); TODO: Would it be better to skip this sample then (also if distance to plane gets very high?)
			else
				sampleRadius[numRelevantSamples] = min(samples[i]->len + 10.0f, (samples[i]->planeC - samples[i]->normal.dotProduct(midPointF)) / denominator);	// term = lambda
		}
		else
		{
			// Not flat: rounded
			sampleRadius[numRelevantSamples] = samples[i]->len;
		}
		numRelevantSamples++;
	}

	// ... and relevant lines ...
	for (int i = 0; i < (int)lines.size(); ++i)
	{
		// Find closest point on line to this direction, save its direction in linedir and length in linelen
		// Get normal for plane containing M, N and being parallel to this direction
		Vector3 cutnormal = normVec.crossProduct(lines[i]->N);
		// Get lambda for union with the line
		float lambda = - (cutnormal.dotProduct(lines[i]->A)) / (cutnormal.dotProduct(lines[i]->AB));
		Vector3 nearestPoint;
		if (lambda <= 0)
			nearestPoint = lines[i]->A;
		else if (lambda >= 1)
			nearestPoint = lines[i]->B;
		else
			nearestPoint = lines[i]->A + lines[i]->AB * lambda;

		float linelen = nearestPoint.length();
		assert(linelen > 0);
		Vector3 linedir = nearestPoint / linelen;

		// CODE FROM FOR BLOCK ABOVE!
		float dp = linedir.dotProduct(normVec);
		if (dp <= 0)
			continue;

		sampleWeight[numRelevantSamples] = dp; // * (1.0f / samples[i]->invspread);
		sampleInvSpread[numRelevantSamples] = 1.0f; // TODO: spread for lines;   samples[i]->invspread;
		if (sampleWeight[numRelevantSamples] > smallestAngle)
		{
			secondSmallestAngle = smallestAngle;
			smallestAngle = sampleWeight[numRelevantSamples];
		}
		else if (sampleWeight[numRelevantSamples] > secondSmallestAngle)
			secondSmallestAngle = sampleWeight[numRelevantSamples];

		sampleRadius[numRelevantSamples] = linelen;
	
		numRelevantSamples++;
	}

	
	float dpbias = smallestAngle - (1 - smallestAngle) * 4.0f - 0.001f; //secondSmallestAngle - 0.001f;
	dpbias = min(0.999f, max(0.0f, dpbias));
	
	//float dpbias = min(0.999f, max(0.0f, smallestAngle - (1 - smallestAngle) * 4.0f - 0.001f));	// OK
	
	const int MAX_PARAM = 2;
	float powparam = 1 + (MAX_PARAM - 1) * smallestAngle; //(1 - smallestAngle);
	
	for (int i = 0; i < numRelevantSamples; ++i)
	{
		sampleWeight[i] = (sampleWeight[i] - dpbias) / (1 - dpbias) * (1.0f / sampleInvSpread[i]);
		if (sampleWeight[i] < 0)
			sampleWeight[i] = 0;
		else
		{
			if (sampleWeight[i] > 1)
				sampleWeight[i] = 1;
				
			sampleWeight[i] = powf(sampleWeight[i], powparam);
					
			// nothing
					
			//sampleWeight[i] = powf(sampleWeight[i], 4); // OK
		}
		totalWeight += sampleWeight[i];
	}
	
	// ... apply weighted samples.
	for (int i = 0; i < numRelevantSamples; ++i)
		radius += sampleRadius[i] * sampleWeight[i];
	radius /= totalWeight;

	// bias for places where there is no sample nearby
	//radius *= powf(smallestAngle, 3);

	return radius;
}
float DigiSpeMeasuringPoint::apply(int x, int y, int z, float in)
{
	// Check bounds for early-out
	Vector3 vec = Vector3(x, y, z) - midPointF;
	float dist = vec.length();
	if (dist < 0.0001f)
		return -10.0f;
	//if (dist < minradius)		// TODO: doesn't work correctly with triangulation
	//	return -10.0f;
	if (dist > maxradius)
		return in;

	// Get radius
	#ifdef DIGISPE_RADIUS_CACHE
		float radius = getCachedRadiusForDirection(vec);
	#else
		float radius = getRadiusForDirection(vec);
	#endif

	// Apply radius
	radius += BORDER_RANGE;
	float value = (radius - dist) / BORDER_RANGE;
	return in - ((value > 0) ? value : 0);
}

