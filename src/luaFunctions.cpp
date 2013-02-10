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
#include "luaFunctions.h"
#include "stateGame.h"
#include "volume.h"
#include "physics.h"
#include "componentPosition.h"
#include "console.h"
#include "componentHealth.h"
#include "level.h"
#include "digiSpe.h"
#include "gameDataStorage.h"
#include "windowManager.h"

Object* lastHitObject;
Object* luaGetLastHitObject()														{ return lastHitObject; }

void luaGetPosition(Object* object, Vector3* out)									{ *out = object->position; }
bool luaGetHead(Object* object, Vector3* out)										{ return object->sendGetMessage(COMPMSG_GET_HEAD, out); }
bool luaGetAim(Object* object, Vector3* out)										{ return object->sendGetMessage(COMPMSG_GET_AIM, out); }
void luaSetAim(Object* object, Vector3* in)											{ object->sendMessage(COMPMSG_SET_AIM, in); }
bool luaGetMuzzleRel(Object* object, Vector3* out)									{ return object->sendGetMessage(COMPMSG_GET_MUZZLE_REL, out); }
void luaDamageEffect(Object* object, Vector3* position, int type)					{ object->sendMessage(COMPMSG_DAMAGEEFFECT, (void*)position, (void*)&type); }
void luaApplyCentralImpulse(Object* object, Vector3* impulse)						{ object->sendMessage(COMPMSG_APPLYCENTRALIMPULSE, impulse); }
void luaApplyImpulse(Object* object, Vector3* impulse, Vector3* rel_pos)			{ object->sendMessage(COMPMSG_APPLYIMPULSE, impulse, rel_pos); }

void luaSetSavePath(const char* path)
{
	game.setSavePath(path);
}
void luaAddResourcePath(const char* path, const char* locationType, const char* resourceGroup)
{
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(path, locationType, resourceGroup);
}
void luaInitResourceGroup(const char* resourceGroup)
{
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup(resourceGroup);
}
void luaLoadScriptsFrom(const char* path, const char* locationType)
{
	objectScriptManager.loadScriptsFromArchive(path, locationType);
}
void luaLoadObjectsFrom(const char* path, const char* locationType)
{
	objectManager.readObjectsFromArchive(path, locationType);
}

void luaCreateFont(const char* path, const char* resourceGroup)
{
	CEGUI::FontManager::getSingleton().create(path, (CEGUI::utf8*)resourceGroup);
}
void luaAddPage(const char* name, const char* path)
{
	windowManager.addPage(name, path);
}
void luaSetGUIPage(const char* name)
{
	windowManager.setPage(name);
}

void luaSetRunSimulation(bool run)
{
	game.setRunSimulation(run);
}
void luaSetEditObjects(bool edit)
{
	objectManager.setEditObjects(edit);
}

void luaCreatePlayer()
{
	game.createPlayer(game.getCameraPosition());
}
void luaMoveCamera(float rotX, float rotY, Vector3 translateVector)
{
	game.moveCamera(Radian(rotX), Radian(rotY), translateVector);
}

void luaInitLevelGenerator(float levelScale)
{
	level = new Level(levelScale);
	level->generate();
	game.volumeData->setBlockLoadCallback(&Level::createFunc);
	game.volumeData->setBlockTextureCallback(&Level::textureFunc);
}
void luaInitDigiSpe(const char* caveFile, const char* tourFile, bool releaseMode)
{
	// Set up DigiSpe
	digiSpe = new DigiSpe(caveFile, tourFile, releaseMode);
}
void luaSetBlockFunc(const char* name)
{
	std::string strName = name;
	if (strName == "InvCube")
		game.volumeData->setBlockLoadCallback(&Level::other_InvCube_VoxelCB);
	else if (strName == "Null")
		game.volumeData->setBlockLoadCallback(NULL);
	else
		LogManager::getSingleton().logMessage("ERROR: luaSetBlockFunc: invalid parameter!", LML_CRITICAL);
}
void luaSetTextureFunc(const char* name)
{
	std::string strName = name;
	if (strName == "AutoTex")
		game.volumeData->setBlockTextureCallback(&Level::other_autotexCallback);
	else if (strName == "Null")
		game.volumeData->setBlockTextureCallback(NULL);
	else
		LogManager::getSingleton().logMessage("ERROR: luaSetTextureFunc: invalid parameter!", LML_CRITICAL);
}
void luaSetAutotexSettings(int top, int side, int inner1, int inner2)
{
	game.setAutotexSettings(top, side, inner1, inner2);
	// TODO: also set sandbox CEGUI controls?
}

void luaSetVolumeMaterial(string materialName)
{
	game.volumeData->setMaterial(materialName);
}
void luaEnablePaging(bool enable)
{
	game.volumeData->automaticPaging = enable;
	game.volumeData->pagingMode = enable ? PAGINGMODE_OPEN : PAGINGMODE_CLOSED;
	// TODO: also set sandbox CEGUI control?
}
void luaPreloadGeometry(int count)
{
	int loadedBlocks = 0;
	while (loadedBlocks < count)
	{
		loadedBlocks += game.volumeData->update(camera->getDerivedPosition()) ? 1 : 0;
		sleep(2);
	}
}
void luaLoadVolume()
{
	game.volumeData->reset();
	gameDataStorage.clear();
	gameDataStorage.loadBlocks(game.savePath.c_str(), game.volumeData);
}
void luaLoadAll()
{
	game.volumeData->reset();
	gameDataStorage.clear();
	gameDataStorage.loadBlocks(game.savePath.c_str(), game.volumeData);
	gameDataStorage.loadObjects(game.savePath.c_str());
}
Vector3 luaGetVolumeSize()
{
	return Vector3(VOLUME_SIDE_LENGTH_IN_VOXELS * game.volumeData->scale);
}

void luaSetFog(float r, float g, float b, float mindist, float maxdist, bool cutoff)
{
	game.setFog(r, g, b, mindist, maxdist, cutoff);
}

int	luaGetHolderID(Object* weapon)
{
	int id = INVALID_OBJECT_ID;
	weapon->sendGetMessage(COMPMSG_GET_HOLDER_ID, &id);
	return id;
}

bool luaTakeDamage(Object* object, float amount, int type, int aggressorID)
{
	if (!object->sendMessage(COMPMSG_TAKEDAMAGE, (void*)&amount, (void*)&type))
		return false;

	if (aggressorID != INVALID_OBJECT_ID)
		object->sendMessage(COMPMSG_AI_ATTACKEDBY, (void*)&amount, (void*)&aggressorID);
	return true;
}

/// Sums up the values of all voxels in a box with the center at point and the radius boxRadius
int luaCountVoxelValuesAtPoint(Vector3* point, int boxRadius)
{
	int sum = 0;
	int mx = (int)((point->x + 0.5f) / game.volumeData->scale) - game.volumeData->windowXInVoxels;
	int my = (int)((point->y + 0.5f) / game.volumeData->scale) - game.volumeData->windowYInVoxels;
	int mz = (int)((point->z + 0.5f) / game.volumeData->scale) - game.volumeData->windowZInVoxels;

	for (int x = mx - boxRadius; x <= mx + boxRadius; x++)
	{
		for (int y = my - boxRadius; y <= my + boxRadius; y++)
		{
			for (int z = mz - boxRadius; z <= mz + boxRadius; z++)
			{
				if (game.volumeData->containsPointRel(x, y, z))
					sum += game.volumeData->getVoxelRel(x, y, z);
			}
		}
	}

	return sum;
}

void luaObjectsInSphereCallback(Vector3* pos, float range, const char* callback)
{
	float squaredRange = range * range;
	for (ObjectManager::iterator it = objectManager.begin(); it != objectManager.end(); it++)
	{
		float squaredDistance = ((*it).second->position - (*pos)).squaredLength();
		if (squaredDistance <= squaredRange)
			luabind::call_function<void>(objectScriptManager.masterState, callback, pos, squaredDistance, (*it).second, &(*it).second->position);
	}
}

/// createObject(name, position, creatorId)
Object* luaCreateObject(const char* name, Vector3* position, int creatorID)
{
	Object* obj = objectManager.createObject(name, creatorID);
	obj->position = *position;
	obj->sendMessage(COMPMSG_CREATE, (void*)(position));
	return obj;
}

void luaAttachObject(Object* parent, Object* child)
{
	parent->sendMessage(COMPMSG_ATTACH_CHILD, child);
}

/// editVoxelDataSmooth2(pos, positive, radius)
void luaEditVoxelDataSmooth2(Vector3* pos, bool positive, float radius)
{
#ifdef USE_RAKNET
  scheduleEditVoxelDataSmooth2(pos, positive, radius, game.frameNumber+60);
#else
  doEditVoxelDataSmooth2(pos, positive, radius);
#endif
}

#ifdef USE_RAKNET
void scheduleEditVoxelDataSmooth2(Vector3*pos, bool positive, float radius, uint32_t frameNumber, RakNet::RPC3 *rpcFromNetwork)
{
  // TODO: schedule a call to doEditVoxelDataSmooth2
  // currently for testing purposes instant calling of doEdit...
  doEditVoxelDataSmooth2(pos, positive, radius);

  if (rpcFromNetwork==NULL) {
    // call this function on every other client, too
    // TODO: is there a better way to do this? (game.mNetwork->rpc3 is somewhat hacky)
    game.mNetwork->rpc3.CallC("scheduleEditVoxelDataSmooth2", pos, positive, radius, frameNumber, rpcFromNetwork);
  }
}
#endif

void doEditVoxelDataSmooth2(Vector3*pos, bool positive, float radius)
{
	int funcNumber = 0;	// TODO: 1 for box? Is the box shape neccessary?

	if (positive)
		Game::editLevel_CallShapeFunc(funcNumber, *pos, radius, &Game::editLevel_Smooth2_CB_Add);
	else
		Game::editLevel_CallShapeFunc(funcNumber, *pos, radius, &Game::editLevel_Smooth2_CB_Sub);
}


/// editVoxelDataSmooth2(pos, positive, radius)
void luaEditVoxelDataHard(Vector3* pos, bool positive, float radius)
{
#ifdef USE_RAKNET
  scheduleEditVoxelDataHard(pos, positive, radius, game.frameNumber+60);
#else
  doEditVoxelDataHard(pos, positive, radius);
#endif
}

#ifdef USE_RAKNET
void scheduleEditVoxelDataHard(Vector3*pos, bool positive, float radius, uint32_t frameNumber, RakNet::RPC3 *rpcFromNetwork)
{
  // TODO: schedule a call to doEditVoxelDataHard
  // currently for testing purposes instant calling of doEdit...
  doEditVoxelDataHard(pos, positive, radius);

  if (rpcFromNetwork==NULL) {
    // call this function on every other client, too
    // TODO: is there a better way to do this? (game.mNetwork->rpc3 is somewhat hacky)
    game.mNetwork->rpc3.CallC("scheduleEditVoxelDataHard", pos, positive, radius, frameNumber, rpcFromNetwork);
  }
}
#endif

/// editVoxelDataHard(pos, positive, radius)
void doEditVoxelDataHard(Vector3* pos, bool positive, float radius)
{
	int funcNumber = 0;	// TODO: 1 for box? Is the box shape neccessary?

	if (positive)
		Game::editLevel_CallShapeFunc(funcNumber, *pos, radius, &Game::editLevel_Hard_CB_Add);
	else
		Game::editLevel_CallShapeFunc(funcNumber, *pos, radius, &Game::editLevel_Hard_CB_Sub);
}

/// rayTestVoxel(start, dir, length, out)
bool luaRayTestVoxel(Vector3* start, Vector3* dir, float length, Vector3* out)
{
	Vector3 vStart = *start;
	Vector3 vDir = *dir;

	lastHitObject = NULL;	// no chance to hit an object with a voxel ray test

	return game.volumeData->getRayIntersectionEx(vStart, vDir, out, length);
}

/// rayTestPhysics(start, dir, length, out, exclude)
/**
	uses COMPMSG_GET_RIGIDBODY msg on the exclude object to get the rigid body
 */
bool luaRayTestPhysics(Vector3* start, Vector3* dir, float length, Vector3* out, Object* exclude, short int collisionMask)
{
	btRigidBody* body;
	bool bBody = false;

	Vector3 vStart = *start;
	Vector3 vDir = *dir;

	if (exclude)
		bBody = exclude->sendGetMessage(COMPMSG_GET_RIGIDBODY, &body);

	vDir *= length;
	ClosestNotMe rayCallback(bBody ? body : NULL);
	rayCallback.m_closestHitFraction = 1.0;
	rayCallback.m_collisionFilterMask = collisionMask;
	physics.rayTest(btVector3(vStart.x, vStart.y, vStart.z), btVector3(vStart.x + vDir.x, vStart.y + vDir.y, vStart.z + vDir.z), rayCallback);

	if (rayCallback.hasHit())
	{
		*out = vStart + rayCallback.m_closestHitFraction * vDir;
		lastHitObject = physics.getObjectFromBody((btRigidBody*)rayCallback.m_collisionObject);
		return true;
	}
	else
	{
		lastHitObject = NULL;
		return false;
	}
}

/// rayTestCombined(start, dir, length, out, exclude)
/**
	uses COMPMSG_GET_RIGIDBODY msg on the exclude object to get the rigid body
 */
bool luaRayTestCombined(Vector3* start, Vector3* dir, float length, Vector3* out, Object* exclude, short int collisionMask)
{
	// Do the voxel test first
	bool bVoxel = luaRayTestVoxel(start, dir, length, out);
	btRigidBody* body;
	bool bBody = false;

	Vector3 vStart = *start;
	Vector3 vDir = *dir;

	if (exclude)
		bBody = exclude->sendGetMessage(COMPMSG_GET_RIGIDBODY, &body);

	float newLength;
	float oldLength = 0;
	if (bVoxel)
		oldLength = ((*out) - vStart).squaredLength();

	// Now the ray test
	vDir *= length;
	ClosestNotMe rayCallback(bBody ? body : NULL);
	rayCallback.m_closestHitFraction = 1.0;
	rayCallback.m_collisionFilterMask = collisionMask;
	physics.rayTest(btVector3(vStart.x, vStart.y, vStart.z), btVector3(vStart.x + vDir.x, vStart.y + vDir.y, vStart.z + vDir.z), rayCallback);

	lastHitObject = NULL;

	if (!rayCallback.hasHit())
		return bVoxel;	// old result is ok
	else if (!bVoxel)
	{
		*out = vStart + rayCallback.m_closestHitFraction * vDir;
		lastHitObject = physics.getObjectFromBody((btRigidBody*)rayCallback.m_collisionObject);
		return true;
	}
	else
	{
		// both tests returned a result, use the nearer one
		newLength = (rayCallback.m_closestHitFraction * vDir).squaredLength();

		if (newLength < oldLength)
		{
			*out = vStart + rayCallback.m_closestHitFraction * vDir;
			lastHitObject = physics.getObjectFromBody((btRigidBody*)rayCallback.m_collisionObject);
		}
		// else leave it as it is

		return true;
	}
}

/// tests if testObject can be hit from the position start
bool luaRayTestObject(Vector3* start, Object* testObject, Object* exclude, short int collisionMask)
{
	Vector3 toTarget = testObject->position - (*start);
	float toTargetLength = toTarget.length();
	if (toTargetLength != 0.0f)
		toTarget /= toTargetLength;

	Vector3 hitPos;
	if (!luaRayTestCombined(start, &toTarget, toTargetLength, &hitPos, exclude, collisionMask))
		return false;

	Object* hitObj = luaGetLastHitObject();
	if ((!hitObj) || (hitObj != testObject))
		return false;

	return true;
}

/// tests if the straight line from start to target is free
bool luaRayTestFree(Vector3* start, Vector3* target, Object* exclude, short int collisionMask)
{
	Vector3 toTarget = (*target) - (*start);
	float toTargetLength = toTarget.length();
	if (toTargetLength != 0.0f)
		toTarget /= toTargetLength;

	Vector3 hitPos;
	return !luaRayTestCombined(start, &toTarget, toTargetLength, &hitPos, exclude, collisionMask);
}

/// setTargetPosition(object, position)
/**
	used for weapon-in-hand positions
 */
void luaSetTargetPosition(Object* object, ComponentPosition* position)
{
	object->sendMessage(COMPMSG_SETTARGETPOSITION, position);
}

/// addLogMessage(string text)
void luaAddLogMessage(const char* text)
{
	game.gameConsole->addMessage(text);
}

/// wait(float seconds)
void luaWait(float seconds, lua_State* state)
{
	ObjectScriptExecutor* exec = objectScriptManager.getExec(state);
	exec->sleepTime = seconds;
};
