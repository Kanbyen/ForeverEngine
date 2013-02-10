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

#ifndef _STATE_GAME_H_
#define _STATE_GAME_H_

#include "OgrePrerequisites.h"
#include "OgreMath.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "stateManager.h"
#include "blockConstants.h"

class State;
class ComponentPlayer;
class Console;
class VoxelVolume;
class SkySphere;
class Object;

/// TODO
enum EGameSubState
{
	SUBST_EDITOR,
	SUBST_PLAY,
	SUBST_GAMESTART,
	SUBST_RESPAWN
};

/// TODO: => GAME SESSION
#define MAX_STORED_MESSAGES 256
enum EMessageType
{
	MSGTYPE_CHAT,
	MSGTYPE_KILL,	// e.g. "Foo was killed by Bar!"
	MSGTYPE_SCORE,	// e.g. "Blue team scores!"
	MSGTYPE_GAME,	// e.g. "Game ends in 30 seconds!"
};

/// The "game" state. Includes both the sandbox mode and the "real" game where the player controls a character. Pseudo-Singleton: use the global variable game
class Game : public FrameListener, public State
{
friend class GameInputHandler;
friend class Level;
public:

	/// The voxel volume. TODO: Make this a global singleton.
	VoxelVolume* volumeData;

	/// The sky rendering object TODO: => World
	SkySphere* skySphere;

	/// Pointer to the player component or NULL if in sandbox mode
	ComponentPlayer* mPlayer;

	/// Two message logs. gameConsole is on the left side of the screen, descConsole on the right side. Both are only visible when in player mode.
	Console* gameConsole;
	Console* descConsole;
	
	/// Path where the save files will be placed / looked for
	std::string savePath;

	/// @ 60 fps we have enough frame numbers for more than two consecutive years of counting
	unsigned int frameNumber;


	/// Initializes the game state
	void init(State* lastState);
	/// Deinitializes the game state
	void exit(State* nextState);

	/// Execute the newGame script (TODO!)
	void newGame();

	/// Save the current state (TODO!)
	void endGame();

	/// Creates a player object and returns it
	Object* createPlayer(Vector3 position);
	
	/// Enable or pause the game
	inline void setRunSimulation(bool run) {runSimulation = run;}

	/// Moves (rotates and translates) the camera
	void moveCamera(Radian rotX, Radian rotY, Vector3 translateVector);
	/// Sets the position of the camera
	void setCameraPosition(Vector3 pos);
	/// Get the position of the camera
	Vector3 getCameraPosition();
	/// Get the camera forward vector
	Vector3 getCameraForward();
	/// Get the camera right vector
	Vector3 getCameraRight();
	/// Returns the camera roll node
	SceneNode* getCameraRollNode();
	/// Returns the camera node
	SceneNode* getCameraNode();
	/// Set the camera node orientations
	void setCameraNodeOrientations(Quaternion yaw, Quaternion pitch, Quaternion roll);

	/// Send all game objects the COMPMSG_UPDATE message
	void updateObjects(float timeSinceLastFrame);
	/// Updates the sandbox GUI
	void updateGUI(float timeSinceLastFrame);
	
	/// Sets fog parameters
	void setFog(float r, float g, float b, float mindist, float maxdist, bool cutoff);
	/// Updates the fog settings
	void updateFog();

	void setNewVolumeScale();
	void setNewBrushSize();
	void setNewBrushTexture();
	void setNewAutotexSettings();
	void setAutotexSettings(int top, int side, int inner1, int inner2);
	
	/// Set the path where the save files will be placed / looked for
	void setSavePath(const char* path);
	
	void setSubState(EGameSubState subState);
	inline EGameSubState getSubState() const {return subState;}
	void updateGameStart();
	
	// TODO: => GAME SESSION
	void addMessage(const std::string& msg, EMessageType type);
	void clearMessages();
	inline int getNumNewMessagesThisFrame() const {return numNewMessagesThisFrame;}
	/// n: from 0 to messages.size() - 1
	const std::string& getNthLatestMessage(int n);
	void messagesUpdated();

	// Voxel edit methods
	static void editLevel(bool leftMB, Vector3 pos, Vector3 dir);

	static Vector3 lastEditPos;

	static void editLevel_Hard(bool leftMB, Vector3 pos, Vector3 dir, int funcNumber);
	static void editLevel_Hard_CB_Add(int x, int y, int z, float v);
	static void editLevel_Hard_CB_Sub(int x, int y, int z, float v);
	static void editLevel_Smooth(bool leftMB, Vector3 pos, Vector3 dir, int funcNumber);
	static void editLevel_Smooth_CB_Add(int x, int y, int z, float v);
	static void editLevel_Smooth_CB_Sub(int x, int y, int z, float v);
	static void editLevel_Smooth2(bool leftMB, Vector3 pos, Vector3 dir, int funcNumber);
	static void editLevel_Smooth2_CB_Add(int x, int y, int z, float v);
	static void editLevel_Smooth2_CB_Sub(int x, int y, int z, float v);
	static void editLevel_Average(bool leftMB, Vector3 pos, Vector3 dir, int funcNumber);
	static void editLevel_Average_CB(int x, int y, int z, float v);
	static void editLevel_Texturize(bool leftMB, Vector3 pos, Vector3 dir, int funcNumber);
	static void editLevel_Texturize_CB(int x, int y, int z, float v);
	static void editLevel_TexturizeSurface(bool leftMB, Vector3 pos, Vector3 dir, int funcNumber);
	static void editLevel_TexturizeSurface_CB(int x, int y, int z, float v);
	static void editLevel_FloatingRockReduction_CB(int x, int y, int z, float v);

	static void editLevel_CallShapeFunc(int shapeNr, Vector3 pos, Real radius, void (*voxelFunc)(int,int,int,float));

	// CEGUI event handlers
	bool gameStart_ChatKeyDown(const CEGUI::EventArgs &args);
	
	bool click_generate(const CEGUI::EventArgs &args);
	
	bool spawnObject(const CEGUI::EventArgs &args);
	bool deleteObject(const CEGUI::EventArgs &args);
	bool toggleRunSimulation(const CEGUI::EventArgs &args);
	
	void finishedGUIInput(const CEGUI::EventArgs &args);				// sets the new values after they are input
	bool activateGUIInputMode(const CEGUI::EventArgs &args);			// set keyboard input to buffered mode so that CEGUI gets the key input
	bool deactivateGUIInputMode(const CEGUI::EventArgs &args);			// set keyboard input to unbuffered mode so that the game gets the key input
	bool deactivateGUIInputModeWithEnter(const CEGUI::EventArgs &args);	// calls the above method and deactivates the event source if enter is pressed

	bool moveWindowPX(const CEGUI::EventArgs &args);
	bool moveWindowMX(const CEGUI::EventArgs &args);
	bool moveWindowPY(const CEGUI::EventArgs &args);
	bool moveWindowMY(const CEGUI::EventArgs &args);
	bool moveWindowPZ(const CEGUI::EventArgs &args);
	bool moveWindowMZ(const CEGUI::EventArgs &args);

	bool toggleAutomaticPaging(const CEGUI::EventArgs &args);
	bool togglePagingMode(const CEGUI::EventArgs &args);

	bool saveVoxelData(const CEGUI::EventArgs &args);
	bool loadVoxelData(const CEGUI::EventArgs &args);
	bool saveObjectData(const CEGUI::EventArgs &args);
	bool loadObjectData(const CEGUI::EventArgs &args);
	bool loadAllData(const CEGUI::EventArgs &args);

protected:
	
	/// Should the objects be moved, or is the game paused?
	bool runSimulation;

	/// Ogre::SceneNodes for camera display
	SceneNode *cameraNode;
	SceneNode *cameraYawNode;
	SceneNode *cameraPitchNode;
	SceneNode *cameraRollNode;

	Real brushSize;
	Real brushTexture;

	/// Autotex texture numbers (for sandbox mode)
	int autotexTop;
	int autotexSide;
	int autotexInner1;
	int autotexInner2;

	/// Is used to toggle fog display (in sandbox mode)
	bool displayFog;
	ColourValue fogColor;
	float fogMinimumDistance;
	float fogMaximumDistance;
	bool cutoffAfterFogMaxDist;
	
	/// TODO: temporary hack. Use real gamestates instead!
	EGameSubState subState;
	
	/// TODO: => SUBST_GAMESTART:
	int lastLoadingPercentageSent;
	bool gameCanBeStarted;
	
	/// TODO: => GAME SESSION
	std::deque< std::string > messages;
	std::deque< EMessageType > messageTypes;
	int numNewMessagesThisFrame;

	/// Sets up the rendering of the game world
	void createScene();
	/// Sets up the voxel volume
	void createVoxelGeometry();

	/// Ogre's FrameListener callbacks
	bool frameStarted(const FrameEvent& evt);
	bool frameEnded(const FrameEvent& evt);
};

extern Game game;

#endif
