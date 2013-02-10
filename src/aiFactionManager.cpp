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
#include "aiFactionManager.h"

AIFactionManager* aiFactionManager;

void AIFactionManager::addFaction(string name, AIAttitude attitudeToSelf)
{
	std::map<string, AIAttitude> newMap;
	newMap.insert(std::map<string, AIAttitude>::value_type(name, attitudeToSelf));

	factionMap.insert(FactionMap::value_type(name, newMap));
}

void AIFactionManager::setAttitude(string ofFaction, string towardsFaction, AIAttitude attitude)
{
	factionMap[ofFaction][towardsFaction] = attitude;
	factionMap[towardsFaction][ofFaction] = attitude;
}
AIAttitude AIFactionManager::getAttitude(string ofFaction, string towardsFaction)
{
	std::map<string, std::map<string, AIAttitude> >::iterator it = factionMap.find(ofFaction);
	FactionMapItr it2 = it->second.find(towardsFaction);
	if (it2 != it->second.end())
		return it2->second;
	else
		return AIATTITUDE_UNKNOWN;
}
