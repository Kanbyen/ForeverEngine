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

#ifndef _LUA_FUNCTIONS_H_
#define _LUA_FUNCTIONS_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include <luabind/object.hpp>

class Object;
class ComponentPosition;

/// The object hit by the last ray test or NULL / nil if no object or the ground was hit
extern Object* lastHitObject;

// GetX / SetX functions
void luaGetPosition(Object* object, Ogre::Vector3* out);
bool luaGetHead(Object* object, Ogre::Vector3* out);
bool luaGetAim(Object* object, Ogre::Vector3* out);
void luaSetAim(Object* object, Ogre::Vector3* in);
bool luaGetMuzzleRel(Object* object, Ogre::Vector3* out);
int	luaGetHolderID(Object* weapon);
Object* luaGetLastHitObject();

// System
/// Sets the path where the save files will be placed / looked for
void luaSetSavePath(const char* path);
/// Adds the specified resource location. This enables Ogre to find resources there
void luaAddResourcePath(const char* path, const char* locationType, const char* resourceGroup);
/// Initializes the specified resource group
void luaInitResourceGroup(const char* resourceGroup);
/// Loads all scripts (*.lua) from the directory
void luaLoadScriptsFrom(const char* path, const char* locationType);
/// Loads all objects (*.object) from the directory
void luaLoadObjectsFrom(const char* path, const char* locationType);

// GUI
/// Loads a font
void luaCreateFont(const char* path, const char* resourceGroup);
/// Adds a GUI page
void luaAddPage(const char* name, const char* path);
/// Sets the current GUI page
void luaSetGUIPage(const char* name);

// Game
/// Enables or pauses the simulation
void luaSetRunSimulation(bool run);
/// Enables or disables object edit mode
void luaSetEditObjects(bool edit);

// Player
/// Creates the player object at the current position
void luaCreatePlayer();
/// Moves the camera (angles in radians)
void luaMoveCamera(float rotX, float rotY, Vector3 translateVector);

// Level generation
/// Initializes the Forever War level generator
void luaInitLevelGenerator(float levelScale);
/// Initializes DigiSpe
void luaInitDigiSpe(const char* caveFile, const char* tourFile, bool releaseMode);
/// Supported functions: InvCube, Null
void luaSetBlockFunc(const char* name);
/// Supported functions: AutoTex, Null
void luaSetTextureFunc(const char* name);
/// Set autotexturize parameters
void luaSetAutotexSettings(int top, int side, int inner1, int inner2);

// Voxel volume
/// Sets the material name
void luaSetVolumeMaterial(string materialName);
/// Enables / disables paging
void luaEnablePaging(bool enable);
/// Preload the specified number of blocks around the camera position; works only if block loading is threaded, otherwise the whole volume will be loaded!
void luaPreloadGeometry(int count);
/// Load the saved volume
void luaLoadVolume();
/// Load the saved volume and objects
void luaLoadAll();
/// Get the volume size in world units
Vector3 luaGetVolumeSize();

// Environment
/// Set fog parameters. Min- and maxdist are multiplied with the volume radius, so their range is 0 to 1.
void luaSetFog(float r, float g, float b, float mindist, float maxdist, bool cutoff);

// Damage
void luaDamageEffect(Object* object, Ogre::Vector3* position, int type);
bool luaTakeDamage(Object* object, float amount, int type, int aggressorID);

// Impulses
void luaApplyCentralImpulse(Object* object, Ogre::Vector3* impulse);
void luaApplyImpulse(Object* object, Ogre::Vector3* impulse, Ogre::Vector3* rel_pos);

/// Sums up the values of all voxels in a box with the center at point and the radius boxRadius
int luaCountVoxelValuesAtPoint(Ogre::Vector3* point, int boxRadius);

/// Calls a callback (a lua function) for every object in the specified sphere
void luaObjectsInSphereCallback(Ogre::Vector3* pos, float range, const char* callback);

/// createObject(name, position, creatorID)
Object* luaCreateObject(const char* name, Ogre::Vector3* position, int creatorID);

void luaAttachObject(Object* parent, Object* child);

/// editVoxelDataSmooth2(pos, positive, radius)
void luaEditVoxelDataSmooth2(Ogre::Vector3* pos, bool positive, float radius);

#ifdef USE_RAKNET
/// delay actual action to synchronize over network
void scheduleEditVoxelDataSmooth2(Vector3*pos, bool positive, float radius, unsigned int frameNumber, RakNet::RPC3 *rpcFromNetwork = NULL);
#endif

/// actually  do something
void doEditVoxelDataSmooth2(Ogre::Vector3* pos, bool positive, float radius);

/// editVoxelDataHard(pos, positive, radius)
void luaEditVoxelDataHard(Ogre::Vector3* pos, bool positive, float radius);

#ifdef USE_RAKNET
/// delay actual action to synchronize over network
void scheduleEditVoxelDataHard(Vector3*pos, bool positive, float radius, unsigned int frameNumber, RakNet::RPC3 *rpcFromNetwork = NULL);
#endif

/// actually  do something
void doEditVoxelDataHard(Ogre::Vector3* pos, bool positive, float radius);

/// rayTestVoxel(start, dir, length, out)
bool luaRayTestVoxel(Ogre::Vector3* start, Ogre::Vector3* dir, float length, Ogre::Vector3* out);

/// rayTestPhysics(start, dir, length, out, exclude)
/**
	uses COMPMSG_GET_RIGIDBODY msg on the exclude object to get the rigid body
 */
bool luaRayTestPhysics(Ogre::Vector3* start, Ogre::Vector3* dir, float length, Ogre::Vector3* out, Object* exclude, short int collisionMask);

/// rayTestCombined(start, dir, length, out, exclude)
/**
	uses COMPMSG_GET_RIGIDBODY msg on the exclude object to get the rigid body
*/
bool luaRayTestCombined(Ogre::Vector3* start, Ogre::Vector3* dir, float length, Ogre::Vector3* out, Object* exclude, short int collisionMask);

/// tests if testObject can be hit from the position start
bool luaRayTestObject(Ogre::Vector3* start, Object* testObject, Object* exclude, short int collisionMask);

/// tests if the straight line from start to target is free
bool luaRayTestFree(Ogre::Vector3* start, Ogre::Vector3* target, Object* exclude, short int collisionMask);

/// setTargetPosition(object, position)
/**
	used for weapon-in-hand positions
*/
void luaSetTargetPosition(Object* object, ComponentPosition* position);

/// addLogMessage(string text)
void luaAddLogMessage(const char* text);

/// wait(float seconds)
void luaWait(float seconds, lua_State* state);

#endif
