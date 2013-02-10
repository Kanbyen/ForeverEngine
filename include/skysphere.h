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

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;

#ifndef _SKY_SPHERE_H_
#define _SKY_SPHERE_H_

/// Displays the sky sphere
class SkySphere
{
public:
    static Entity* entity;
    static int nextSceneNodeNumber;

    /// Creates the sphere mesh
    static void init();

    SceneNode* sceneNode;

    /// Creates a new skysphere object (shown by default)
    SkySphere(const char* material);
    ~SkySphere();

    /// Show or hide the sphere
    void setVisible(bool visible);

    /// Set position and parameters
    void update(Camera* cam, float radius);
};

#endif
