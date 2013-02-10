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
#include "settings.h"
#include "tinyxml.h"
#include "util.h"

Settings settings;

void Settings::setDefaults()
{
	gameMode = "dm";
	renderSystem = "RenderSystem_GL";
	getScreenResolution(&resolution_Width, &resolution_Height);
	fullscreen = true;
	vSync = false;
	bloom = false;
	nickname = "Unnamed Player";
	lastServer = "www.TODO.org";
	mouseSensitivity = 0.06;
}

void Settings::tryToLoadFromFile(const char* path)
{
	char buffer[256];
	this->path = path;

	TiXmlDocument doc(path);
	setDefaults();

	if (doc.LoadFile())
	{
		TiXmlHandle docHandle(&doc);

		TiXmlElement* elem = docHandle.FirstChildElement("RenderSystem").ToElement();
		if (elem)
		{
			attrCpy(buffer, elem->Attribute("Value"), "RenderSystem_GL");
			renderSystem = buffer;
		}
		
		elem = docHandle.FirstChildElement("GameMode").ToElement();
		if (elem)
		{
			attrCpy(buffer, elem->Attribute("Value"), "hack");
			gameMode = buffer;
		}

		elem = docHandle.FirstChildElement("Resolution").ToElement();
		if (elem)
		{
			if (!elem->Attribute("Width", &resolution_Width) || !elem->Attribute("Height", &resolution_Height))
				getScreenResolution(&resolution_Width, &resolution_Height);
		}

		elem = docHandle.FirstChildElement("Fullscreen").ToElement();
		if (elem) fullscreen = bAttrCpy(elem->Attribute("Value"), true);

		elem = docHandle.FirstChildElement("VSync").ToElement();
		if (elem) vSync = bAttrCpy(elem->Attribute("Value"), false);

		elem = docHandle.FirstChildElement("Bloom").ToElement();
		if (elem) bloom = bAttrCpy(elem->Attribute("Value"), false);

		elem = docHandle.FirstChildElement("Nickname").ToElement();
		if (elem)
		{
		  // TODO: is attrCpy buffer overflow save??
			attrCpy(buffer, elem->Attribute("Value"), "Unnamed Player");
			nickname = buffer;
		}

		elem = docHandle.FirstChildElement("LastServer").ToElement();
		if (elem)
		{
			attrCpy(buffer, elem->Attribute("Value"), "www.TODO.org");
			lastServer = buffer;
		}

		elem = docHandle.FirstChildElement("MouseSensitivity").ToElement();
		if (elem)
		{
			istringstream i(elem->Attribute("Value"));
			i >> mouseSensitivity;
		}
		
	}

	saveToFile();
}

void Settings::saveToFile()
{
	TiXmlDocument doc;
	TiXmlElement* msg;

 	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);
	
	msg = new TiXmlElement("GameMode");
	msg->SetAttribute("Value", gameMode.c_str());
	doc.LinkEndChild(msg);

	msg = new TiXmlElement("RenderSystem");
	msg->SetAttribute("Value", renderSystem.c_str());
	doc.LinkEndChild(msg);

	msg = new TiXmlElement("Resolution");
	msg->SetAttribute("Width", resolution_Width);
	msg->SetAttribute("Height", resolution_Height);
	doc.LinkEndChild(msg);

	msg = new TiXmlElement("Fullscreen");
	msg->SetAttribute("Value", fullscreen ? "true" : "false");
	doc.LinkEndChild(msg);

	msg = new TiXmlElement("VSync");
	msg->SetAttribute("Value", vSync ? "true" : "false");
	doc.LinkEndChild(msg);

	msg = new TiXmlElement("Bloom");
	msg->SetAttribute("Value", bloom ? "true" : "false");
	doc.LinkEndChild(msg);

	msg = new TiXmlElement("Nickname");
	msg->SetAttribute("Value", nickname.c_str());
	doc.LinkEndChild(msg);
	
	msg = new TiXmlElement("LastServer");
	msg->SetAttribute("Value", lastServer.c_str());
	doc.LinkEndChild(msg);

	msg = new TiXmlElement("MouseSensitivity");
	ostringstream o;
	o << mouseSensitivity;
	msg->SetAttribute("Value", o.str().c_str());
	doc.LinkEndChild(msg);

	doc.SaveFile(path.c_str());
}

void Settings::getScreenResolution(int* width, int* height)
{
#ifdef WIN32
	*width = GetSystemMetrics(SM_CXSCREEN);
	*height = GetSystemMetrics(SM_CYSCREEN);
#else
	Display* display = XOpenDisplay(NULL);
	int screen = DefaultScreen(display);

	*width = DisplayWidth(display, screen);
	*height = DisplayHeight(display, screen);

	XCloseDisplay(display);
#endif
}
