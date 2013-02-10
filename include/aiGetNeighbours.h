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

#ifndef _AI_GET_NEIGHBOURS_H_
#define _AI_GET_NEIGHBOURS_H_

#include <list>
#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
class Object;

class AIGetNeighbours
{
public:
	typedef std::list<Object*> NeighbourList;
	typedef std::list<Object*>::iterator NeighbourListItr;

	NeighbourList neighbourList;

	AIGetNeighbours(float radius, Object* owner, int excludeTags = 0);
	~AIGetNeighbours();

	void update(Vector3 pos);

	Object* owner;
	float radiusSq;
	int excludeTags;

	// Let the class behave like a list of Object*s
	typedef NeighbourListItr iterator;
	inline iterator begin()		{ return neighbourList.begin(); }
	inline iterator end()		{ return neighbourList.end(); }
	inline int size()			{ return neighbourList.size(); }
};

#endif
