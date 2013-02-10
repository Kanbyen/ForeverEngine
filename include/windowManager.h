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

#ifndef _WINDOW_MANAGER_H_
#define _WINDOW_MANAGER_H_

#include <map>
#include "OgrePrerequisites.h"
#include <CEGUI.h>
using namespace std;
using namespace Ogre;
#include <RendererModules/Ogre/CEGUIOgreRenderer.h>
#include <RendererModules/Ogre/CEGUIOgreResourceProvider.h>

/// Manages the display of CEGUI pages. Pseudo-Singleton: use the global variable windowManager
class WindowManager
{
friend class CEGUIInputHandler;
public:

	/// Initializes CEGUI
	void init(Root* ogre);
	/// Deinitializes CEGUI
	void exit();

	/// Registers a GUI page
	void addPage(const char* name, const char* path);
	/// Sets the active GUI page
	void setPage(const char* name);
	/// Get a GUI page by its name
	CEGUI::Window* getPage(const char* name);

	/// Get the relative mouse cursor position
	float getMouseXRel();
	/// Get the relative mouse cursor position
	float getMouseYRel();
	/// Shows or hides the CEGUI cursor
	void showCursor(bool show);
	/// Evil hack. Do not use (hahaha).
	void showMouseCursorHack();
	
	CEGUI::OgreRenderer* getRenderer() const {return pGUIRenderer;}

	/// Returns the font used for all windows
	inline CEGUI::Font* getFont() {return font;};

protected:

	/// The font used for all windows
	CEGUI::Font* font;

	std::map<Ogre::String, CEGUI::Window*> pageMap;

	CEGUI::OgreRenderer* pGUIRenderer;
	CEGUI::OgreResourceProvider* pProvider;
	CEGUI::System* pSystem;
};

/// Custom class for list items to support suctom colors
class MyListItem : public CEGUI::ListboxTextItem
{
public:
	MyListItem(const CEGUI::String& text, void* data = NULL) : ListboxTextItem(text, 0, data)
	{
		setSelectionBrushImage("Style-Imageset", "GenericBrush");
		setSelectionColours(CEGUI::colour(229 / 256.0f, 135 / 256.0f, 43 / 256.0f));
	}
};

extern WindowManager windowManager;

#endif
