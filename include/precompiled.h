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

#include <map>
#include <vector>
#include <list>
#include <set>

#include <Ogre.h>
#include <CEGUI.h>
#include <btBulletDynamicsCommon.h>

extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

extern Ogre::Root *ogre;
extern Ogre::RenderWindow *window;
extern Ogre::SceneManager *sceneMgr;
extern Ogre::Camera *camera;
extern Ogre::Viewport* vp;

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#else
	#include <X11/Xlib.h>
#endif

using namespace std;
using namespace Ogre;
