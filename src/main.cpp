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
#include "surfacePatchRenderable.h"
#include "input.h"
#include "noise.h"
#include "settings.h"
#include "windowManager.h"
#include "stateManager.h"
#include "stateGame.h"
#include "stateMenu.h"
#include "gameMode.h"
#include "component.h"
#include "componentMesh.h"
#include "componentPhysics.h"
#include "componentWeapon.h"
#include "componentPlayer.h"
#include "componentParticle.h"
#include "componentHealth.h"
#include "componentDamageEffects.h"
#include "componentInventory.h"
#include "componentItem.h"
#include "componentAI.h"
#include "componentBullet.h"
#include "componentLight.h"
#include "componentCharacterController.h"
#include "componentTrigger.h"
#include "object.h"
#include "objectScript.h"
#include "skysphere.h"
#include "config.h"
#include "network.h"

#ifdef WIN32
	#include "resource.h"
#endif

Root* ogre;
RenderWindow* window;
SceneManager* sceneMgr;
Camera* camera;
Viewport* vp;

/// Initializes Ogre
void initEngine()
{
	String pluginDir;
	#ifdef WIN32
		pluginDir = "";
	#else
		pluginDir = "./";	// was: /usr/local/lib/OGRE/
	#endif

	ogre = new Root("", "");

	// if on linux, tell the GLX plugin the position of the icon
	#ifndef WIN32
		ResourceGroupManager::getSingleton().addResourceLocation("resource/texture", "FileSystem", "General");
	#endif

	// Load Renderer
	#ifdef _DEBUG
		ogre->loadPlugin(pluginDir + settings.renderSystem + "_d");
	#else
		ogre->loadPlugin(pluginDir + settings.renderSystem);
	#endif
	
	#if (OGRE_VERSION_MAJOR <= 1 && OGRE_VERSION_MINOR <= 6)
		RenderSystemList* renderSystems = NULL;
		RenderSystemList::iterator r_it;
		renderSystems = ogre->getAvailableRenderers();
		r_it = renderSystems->begin();
		ogre->setRenderSystem(*r_it);
	#else
		const RenderSystemList& renderSystems = ogre->getAvailableRenderers();
		RenderSystemList::const_iterator r_it;
		r_it = renderSystems.begin();
		ogre->setRenderSystem(*r_it);
	#endif
	
	ogre->initialise(false);

	// load plugins
	#ifdef _DEBUG
		ogre->loadPlugin(pluginDir + "Plugin_CgProgramManager_d");
		ogre->loadPlugin(pluginDir + "Plugin_OctreeSceneManager_d");
		ogre->loadPlugin(pluginDir + "Plugin_ParticleFX_d");
	#else
		ogre->loadPlugin(pluginDir + "Plugin_CgProgramManager");
		ogre->loadPlugin(pluginDir + "Plugin_OctreeSceneManager");
		ogre->loadPlugin(pluginDir + "Plugin_ParticleFX");
	#endif

	// create a rendering window
	NameValuePairList opts;
	opts["vsync"] = settings.vSync ? "true" : "false";

	// Create the window
	window = ogre->createRenderWindow(windowTitle, settings.resolution_Width, settings.resolution_Height, settings.fullscreen, &opts);

	// Set the window icon and title (Windows specific; the Linux version automatically sets GLX_icon.png as the window icon and the correct title)
	#ifdef WIN32
		HWND hWnd = ::FindWindow("OgreD3D9Wnd", windowTitle);
		if (!::IsWindow(hWnd)) hWnd = ::FindWindow("OgreGLWindow", windowTitle);
		if (::IsWindow(hWnd)) {
			char buf[1024];
			::GetModuleFileName(0, (LPCH)&buf, 1024);
			HINSTANCE instance = ::GetModuleHandle(buf);
			// set the title
			::SetWindowText(hWnd, windowTitle);
			// set the icon
			HICON hIcon = ::LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON1));
			if (hIcon) {
				::SendMessage(hWnd, WM_SETICON, 1, (LPARAM)hIcon);
				::SendMessage(hWnd, WM_SETICON, 0, (LPARAM)hIcon);
			}
		}
	#endif

	// Initialize viewport and camera
	sceneMgr = ogre->createSceneManager(ST_GENERIC);
	camera = sceneMgr->createCamera("camera");

	camera->setNearClipDistance(0.25f);
	camera->setFarClipDistance(2 * 1024.0f);
    vp = window->addViewport(camera);
	vp->setBackgroundColour(ColourValue(0, 0, 0));
	camera->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));

	// Add SurfacePatchRenderable factory
	ogre->addMovableObjectFactory(new SurfacePatchRenderableFactory);

	// Test available shader formats
	LogManager::getSingleton().logMessage("Supported Shader formats:");
	GpuProgramManager::SyntaxCodes c = GpuProgramManager::getSingleton().getSupportedSyntax();
	for (GpuProgramManager::SyntaxCodes::iterator it = c.begin(); it != c.end(); it++)
		LogManager::getSingleton().logMessage(*it);
}

void initResources()
{
	// load the resource locations
	ResourceGroupManager::getSingleton().addResourceLocation("resource/gui", "FileSystem", "GUI");
	ResourceGroupManager::getSingleton().addResourceLocation("resource/overlay", "FileSystem", "GUI");
	ResourceGroupManager::getSingleton().addResourceLocation("resource/material_base", "FileSystem", "Texture_Base");
	ResourceGroupManager::getSingleton().addResourceLocation("resource/material", "FileSystem", "Texture");
	ResourceGroupManager::getSingleton().addResourceLocation("resource/program", "FileSystem", "Texture_Base");
	ResourceGroupManager::getSingleton().addResourceLocation("resource/texture", "FileSystem", "Texture");
	ResourceGroupManager::getSingleton().addResourceLocation("resource/mesh", "FileSystem", "Mesh");
	ResourceGroupManager::getSingleton().addResourceLocation("resource/compositor", "FileSystem", "FX");
	ResourceGroupManager::getSingleton().addResourceLocation("resource/particle", "FileSystem", "FX");

	ResourceGroupManager::getSingleton().initialiseResourceGroup("Texture_Base");
	ResourceGroupManager::getSingleton().initialiseResourceGroup("Texture");
	ResourceGroupManager::getSingleton().initialiseResourceGroup("GUI");
	ResourceGroupManager::getSingleton().initialiseResourceGroup("Mesh");
	ResourceGroupManager::getSingleton().initialiseResourceGroup("FX");
}

void initGUI()
{
	// Initialize window manager and load GUI images
	windowManager.init(ogre);
	CEGUI::ImagesetManager::getSingleton().create("hud_images.imageset");
	
	// Load pages
	windowManager.addPage("Menu", "menu.layout");
	windowManager.addPage("HUD", "hud.layout");
	windowManager.addPage("Editor", "editor.layout");
	windowManager.addPage("GameStart", "gameStart.layout");

	// Hook up GUI event handlers to the window elements
	menu.setup();
}

void initData()
{
	// Init scripting system
	objectScriptManager.init();
	
	// Load game modes
	gameModeManager.loadModesFromArchive("resource/mode", "FileSystem");
	gameModeManager.setActMode(settings.gameMode.c_str());

	// Load the texture atlas
	TextureManager::getSingleton().load("texatlas.tga", "Texture", TEX_TYPE_2D, 5);

	// Create sky sphere mesh
	SkySphere::init();

	// Initialize the perlin noise generator
	Noise::init();

	// Initialize static components of the voxel volume
	VoxelBlockPersistent::initStatic();	// TODO: initStatic or staticInit? ;-)
	VoxelVolume::staticInit();

	// Load component templates
	componentTemplateManager.registerTemplate("Mesh", &createTemplate<ComponentTemplateMesh>);
	componentTemplateManager.registerTemplate("Physics", &createTemplate<ComponentTemplatePhysics>);
	componentTemplateManager.registerTemplate("Weapon", &createTemplate<ComponentTemplateWeapon>);
	componentTemplateManager.registerTemplate("Player", &createTemplate<ComponentTemplatePlayer>);
	componentTemplateManager.registerTemplate("Particle", &createTemplate<ComponentTemplateParticle>);
	componentTemplateManager.registerTemplate("Health", &createTemplate<ComponentTemplateHealth>);
	componentTemplateManager.registerTemplate("DamageEffects", &createTemplate<ComponentTemplateDamageEffects>);
	componentTemplateManager.registerTemplate("Item", &createTemplate<ComponentTemplateItem>);
	componentTemplateManager.registerTemplate("AI", &createTemplate<ComponentTemplateAI>);
	componentTemplateManager.registerTemplate("Bullet", &createTemplate<ComponentTemplateBullet>);
	componentTemplateManager.registerTemplate("Light", &createTemplate<ComponentTemplateLight>);
	componentTemplateManager.registerTemplate("CharacterController", &createTemplate<ComponentTemplateCharacterController>);
	componentTemplateManager.registerTemplate("Trigger", &createTemplate<ComponentTemplateTrigger>);
	componentTemplateManager.registerTemplate("Inventory", &createTemplate<ComponentTemplateInventory>);

	// Load scripts
	objectScriptManager.loadScriptsFromArchive("resource/object", "FileSystem");

	// Load objects
	objectManager.init();
	objectManager.readObjectsFromArchive("resource/object", "FileSystem");
}

void initNetwork()
{
	#ifndef DIGISPE
	network = new Network();
	if (!network->isOk())
		throw Exception(0, "Enet initialization failed.", "initNetwork()");
	#endif
}

void initInput()
{
	bool exclusive = true;
	// TODO!
	#ifndef WIN32
		exclusive = false;
	#endif

	unsigned long hWnd;
	window->getCustomAttribute("WINDOW", &hWnd);
	inputManager = new InputManager(hWnd, exclusive);
	inputManager->setWindowExtents(settings.resolution_Width, settings.resolution_Height);
	if (exclusive)
 		inputManager->centerMouse();
	
	// Capture mouse
	inputManager->capture(0.01f);
	WindowEventUtilities::messagePump();
}

void cleanUp()
{
	#ifndef DIGISPE
		delete network;
	#endif
	
	gameModeManager.exit();

	componentTemplateManager.exit();

	stateManager.changeState(NULL);
	windowManager.exit();
	VoxelBlockPersistent::exitStatic();
	delete inputManager;
	delete ogre;
}

#ifdef WIN32
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main (int argc, char *argv[])
#endif
{
	// Initialize everything
	try
	{
		// Try to load the settings from the XML settings file; otherwise, use the defaults
		settings.tryToLoadFromFile("settings.xml");

		// Initialize Ogre, Resources, GUI and input
		initEngine();
		initResources();
		initGUI();
		initInput();
		
		// Show loading screen
		#ifndef DIGISPE
		OverlayManager::getSingleton().getByName("LoadingOverlay")->show();
		ogre->renderOneFrame();
		#endif
		
		// Initialize  misc. parts of the game
		initData();

		// Change to the correct game state
		#ifdef DIGISPE
			stateManager.changeState(&game);
		#else
			stateManager.changeState(&menu);
		#endif
	}
	catch (Exception e)
	{
		LogManager::getSingleton().logMessage("Error during initialization, program exits. Reason:\r\n" + e.getFullDescription(), LML_CRITICAL);
		return false;
	}

	// Main loop
	while ((stateManager.getActState() != NULL) && (!window->isClosed()))
	{
		WindowEventUtilities::messagePump();
		ogre->renderOneFrame();
	}

	// Clean up
	cleanUp();

	return 0;
}
