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
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>

#include "exposeToLua.h"
#include "luaFunctions.h"
#include "componentPosition.h"
#include "object.h"

// Some helpful macros for defining constants (sort of) in Lua. Similar to this code:
// object g = globals(L);
// object table = g["class"];
// table["constant"] = class::constant;

#define LUA_CONST_START( class ) { object g = globals(state); object table = g[#class];
#define LUA_CONST( class, name ) table[#name] = class::name
#define LUA_CONST_END }

void bindGameEngine(lua_State* state)
{
	using namespace luabind;

	// ComponentPosition
	module(state)
	[
		class_<ComponentPosition>( "ComponentPosition" )
			.def_readwrite( "position", &ComponentPosition::position )
			.def_readwrite( "rotation", &ComponentPosition::rotation )
			.def(constructor<>())
			.def(constructor<float, float, float, float, float, float>())
			.def("calculateQuaternion", &ComponentPosition::calculateQuaternion)
	];

	// Object
	module(state)
	[
		class_<Object>( "Object" )
			.def_readwrite( "position", &Object::position )
			.def_readwrite( "id", &Object::id )
			.def_readwrite( "creatorID", &Object::creatorID )
	];

	// lua functions
	module(state)
	[
		def("setSavePath", luaSetSavePath),
		def("addResourcePath", luaAddResourcePath),
		def("initResourceGroup", luaInitResourceGroup),
		def("loadScriptsFrom", luaLoadScriptsFrom),
		def("loadObjectsFrom", luaLoadObjectsFrom),
		
		def("createFont", luaCreateFont),
		def("addPage", luaAddPage),
		def("setGUIPage", luaSetGUIPage),
		
		def("setRunSimulation", luaSetRunSimulation),
		def("setEditObjects", luaSetEditObjects),
	
		def("createPlayer", luaCreatePlayer),
		def("moveCamera", luaMoveCamera),
		
		def("initLevelGenerator", luaInitLevelGenerator),
		def("initDigiSpe", luaInitDigiSpe),
		def("setBlockFunc", luaSetBlockFunc),
		def("setTextureFunc", luaSetTextureFunc),
		def("setAutotexSettings", luaSetAutotexSettings),
		
		def("setVolumeMaterial", luaSetVolumeMaterial),
		def("enablePaging", luaEnablePaging),
		def("preloadGeometry", luaPreloadGeometry),
		def("loadVolume", luaLoadVolume),
		def("loadAll", luaLoadAll),
		def("getVolumeSize", luaGetVolumeSize),
		
		def("setFog", luaSetFog),
	
		def("addLogMessage", luaAddLogMessage),
		def("wait", luaWait, yield),
		def("setTargetPosition", luaSetTargetPosition),
		def("rayTestVoxel", luaRayTestVoxel),
		def("rayTestPhysics", luaRayTestPhysics),
		def("rayTestCombined", luaRayTestCombined),
		def("rayTestObject", luaRayTestObject),
		def("rayTestFree", luaRayTestFree),
		def("createObject", luaCreateObject),
		def("attachObject", luaAttachObject),
		def("takeDamage", luaTakeDamage),
		def("damageEffect", luaDamageEffect),
		def("applyCentralImpulse", luaApplyCentralImpulse),
		def("applyImpulse", luaApplyImpulse),
		def("objectsInSphereCallback", luaObjectsInSphereCallback),
		def("luaCountVoxelValuesAtPoint", luaCountVoxelValuesAtPoint),
		def("getHolderID", luaGetHolderID),

		def("getPosition", luaGetPosition),
		def("getHead", luaGetHead),
		def("getAim", luaGetAim),
		def("setAim", luaSetAim),
		def("getMuzzleRel", luaGetMuzzleRel),
		def("getLastHitObject", luaGetLastHitObject),

		def("editVoxelDataHard", luaEditVoxelDataHard),
		def("editVoxelDataSmooth2", luaEditVoxelDataSmooth2)
	];

	lastHitObject = NULL;
}

void bindOgreVector3(lua_State* state)
{
	using namespace luabind;
	using namespace Ogre;

	module(state)
	[
		class_<Vector3>( "Vector3" )
		.def_readwrite( "x", &Vector3::x )
		.def_readwrite( "y", &Vector3::y )
		.def_readwrite( "z", &Vector3::z )
		.def(constructor<>())
		.def(constructor<Vector3&>())
		.def(constructor<Real, Real, Real>())
		.def(constructor<Real>())
		.def("absDotProduct", &Vector3::absDotProduct)
		.def("crossProduct", &Vector3::crossProduct )
		.def("directionEquals", &Vector3::directionEquals )
		.def("distance", &Vector3::distance )
		.def("dotProduct", &Vector3::dotProduct )
		.def("getRotationTo", &Vector3::getRotationTo )
		.def("isZeroLength", &Vector3::isZeroLength )
		.def("length", &Vector3::length )
		.def("makeCeil", &Vector3::makeCeil )
		.def("makeFloor", &Vector3::makeFloor )
		.def("midPoint", &Vector3::midPoint )
		.def("normalise", &Vector3::normalise )
		.def("nornaliseCopy", &Vector3::normalisedCopy )
		.def("perpendicular", &Vector3::perpendicular )
		.def("positionCloses", &Vector3::positionCloses )
		.def("positionEquals", &Vector3::positionEquals )
			//.def("ptr", &Vector3::ptr )
		.def("randomDeviant", &Vector3::randomDeviant )
		.def("reflect", &Vector3::reflect )
		.def("squaredDistance", &Vector3::squaredDistance )
		.def("squaredLength", &Vector3::squaredLength )

			// Operators

		.def( self + other<Vector3>() )
		.def( self - other<Vector3>() )
		.def( self * other<Vector3>() )
		.def( self * Real() )
		.def( Real() * self )
		.def(tostring(self))
	];

	LUA_CONST_START( Vector3 )
		LUA_CONST( Vector3, ZERO);
		LUA_CONST( Vector3, UNIT_X);
		LUA_CONST( Vector3, UNIT_Y);
		LUA_CONST( Vector3, UNIT_Z);
		LUA_CONST( Vector3, NEGATIVE_UNIT_X);
		LUA_CONST( Vector3, NEGATIVE_UNIT_Y);
		LUA_CONST( Vector3, NEGATIVE_UNIT_Z);
		LUA_CONST( Vector3, UNIT_SCALE);
	LUA_CONST_END;
}

void bindOgreColourValue(lua_State* state)
{
	using namespace luabind;
	using namespace Ogre;

	module(state)
	[
		class_<ColourValue>("ColourValue")
		.def(constructor<>())
		.def(constructor<Real, Real, Real, Real>())
		.def(constructor<Real, Real, Real>())
		.def(tostring(self))
		.def_readwrite( "r", &ColourValue::r)
		.def_readwrite( "g", &ColourValue::g )
		.def_readwrite( "b", &ColourValue::b )
		.def_readwrite( "a", &ColourValue::a )
		.def( "saturate", &ColourValue::saturate )

			// Operators

		.def( self + other<ColourValue>() )
		.def( self - other<ColourValue>() )
		.def( self * other<ColourValue>() )
		.def( self * Real() )
		.def( self / Real() )
	];

	LUA_CONST_START( ColourValue )
		LUA_CONST( ColourValue, ZERO);
		LUA_CONST( ColourValue, Black);
		LUA_CONST( ColourValue, White);
		LUA_CONST( ColourValue, Red);
		LUA_CONST( ColourValue, Green);
		LUA_CONST( ColourValue, Blue);
	LUA_CONST_END;
}
