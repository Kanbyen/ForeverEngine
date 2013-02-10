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

#ifndef _AI_FACTION_MANAGER_H_
#define _AI_FACTION_MANAGER_H_

#include <map>

enum AIAttitude
{
	AIATTITUDE_UNKNOWN = 0,
	AIATTITUDE_FRIENDLY = 1,
 	AIATTITUDE_HOSTILE = 2,
};

class AIFactionManager
{
public:
	typedef std::map<string, std::map<string, AIAttitude> > FactionMap;
	typedef std::map<string, AIAttitude>::iterator FactionMapItr;

	FactionMap factionMap;

	void addFaction(string name, AIAttitude attitudeToSelf);

	void setAttitude(string ofFaction, string towardsFaction, AIAttitude attitude);
	AIAttitude getAttitude(string ofFaction, string towardsFaction);
};

extern AIFactionManager* aiFactionManager;

#endif
