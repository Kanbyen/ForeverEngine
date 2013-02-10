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
#include "windowManager.h"
#include "config.h"

WindowManager windowManager;

float WindowManager::getMouseXRel()
{
	return CEGUI::MouseCursor::getSingleton().getPosition().d_x / (float)window->getWidth();
}
float WindowManager::getMouseYRel()
{
	return CEGUI::MouseCursor::getSingleton().getPosition().d_y / (float)window->getHeight();
}
void WindowManager::showCursor(bool show)
{
	if (show)
		CEGUI::MouseCursor::getSingleton().show();
	else
		CEGUI::MouseCursor::getSingleton().hide();
}
void WindowManager::showMouseCursorHack()
{
	// Dummy input to show mouse cursor
	showCursor(true);
	pSystem->injectMouseMove(1, 0);
}

void WindowManager::setPage(const char* name)
{
	pSystem->setGUISheet(getPage(name));
}

void WindowManager::addPage(const char* name, const char* path)
{
	CEGUI::Window* pLayout = CEGUI::WindowManager::getSingleton().loadWindowLayout(path, "", "GUI");
	pageMap.insert(std::map<String, CEGUI::Window*>::value_type(name, pLayout));
}

CEGUI::Window* WindowManager::getPage(const char* name)
{
	std::map<String, CEGUI::Window*>::iterator it = pageMap.find(name);
	if (it == pageMap.end())
		throw Exception(0, "page name not found", "WindowManager");

	return it->second;
}

void WindowManager::init(Root* ogre)
{
	// Create the OgreResourceProvider
	pProvider = new CEGUI::OgreResourceProvider();

	// with a scene manager and window, we can create the GUI renderer
	pGUIRenderer = &CEGUI::OgreRenderer::create(*ogre->getRenderSystem()->getRenderTarget(windowTitle));

	// create the root CEGUI class
	pSystem = &CEGUI::System::create(*pGUIRenderer, pProvider);

	// tell us what is going on (see CEGUI.log in the working directory)
	CEGUI::Logger::getSingleton().setLoggingLevel(CEGUI::Standard);

	// use this CEGUI scheme definition (see CEGUI docs for more)
	CEGUI::SchemeManager::getSingleton().create((CEGUI::utf8*)"Style.scheme", (CEGUI::utf8*)"GUI");

	// show the CEGUI mouse cursor (defined in the look-n-feel)
	pSystem->setDefaultMouseCursor((CEGUI::utf8*)"Style-Imageset", (CEGUI::utf8*)"MouseArrow");

	// use this font for text in the UI
	CEGUI::FontManager::getSingleton().create("FreeSans.font", (CEGUI::utf8*)"GUI");
	pSystem->setDefaultFont((CEGUI::utf8*)"Sans");
	font = &CEGUI::FontManager::getSingleton().get("Sans");
	
	CEGUI::FontManager::getSingleton().create("FreeSansBig.font", (CEGUI::utf8*)"GUI");
	CEGUI::FontManager::getSingleton().create("FreeSansBold.font", (CEGUI::utf8*)"GUI");
}
void WindowManager::exit()
{
	pageMap.clear();
	pSystem->destroy();
	CEGUI::OgreRenderer::destroy(*pGUIRenderer);
	if (pProvider) delete pProvider;
}
