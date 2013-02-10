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

#ifndef _SURFACE_PATCH_RENDERABLE_H_
#define _SURFACE_PATCH_RENDERABLE_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "surfacePatch.h"

/// Renderable for OGRE
class SurfacePatchRenderable : public SimpleRenderable
{
public:
	SurfacePatch* patch;

	SurfacePatchRenderable();
	~SurfacePatchRenderable();

	void clearGeometry();

	void setGeometry(SurfacePatch* patch, const String& material);
	void createHardwareBuffers();
	void createCollisionShape();

	Real getSquaredViewDepth(const Camera *cam) const;
	Real getBoundingRadius() const;

	virtual const String& getMovableType() const;

protected:
	const Quaternion& getWorldOrientation() const;
	const Vector3& getWorldPosition() const;
};

/// Factory object for creating instances of SurfacePatchRenderable
class SurfacePatchRenderableFactory : public MovableObjectFactory
{
protected:
	MovableObject* createInstanceImpl(const String& name, const NameValuePairList* params);

public:
	SurfacePatchRenderableFactory() {}
	~SurfacePatchRenderableFactory() {}

	static String FACTORY_TYPE_NAME;

	const String& getType() const;
	void destroyInstance(MovableObject* obj);
};

#endif
