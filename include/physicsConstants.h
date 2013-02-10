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

#ifndef _PHYSICS_CONSTANTS_H_
#define _PHYSICS_CONSTANTS_H_

const float LOGIC_TIME_STEP = 1.f / 60.f;

// collisionMasks for rayTests
const short int RAYTEST_ALL = 1 | 2 | 4 | 8 | 16;
const short int RAYTEST_NO_DEBRIS = 1 | 2 | 4 | 16;	// btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::DebrisFilter

#endif
