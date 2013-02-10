project(foreverwar)

cmake_minimum_required(VERSION 2.6)

include(addPCHSupport.cmake)
include(findLua51.cmake)

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wno-deprecated")
set(CMAKE_CXX_FLAGS_RELEASE "-s -O2 -Wall -Wno-deprecated")

#many of these directories are only added so eclipse finds them, too.
IF(CMAKE_HOST_UNIX)
include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
    include
    include/RPC3
    include/DigiSpe
    src
    /usr/include/bullet
    /usr/include/CEGUI
    /usr/include/OIS
    /usr/include/OGRE
    /usr/local/include/bullet
    ${LUA51_INCLUDE_DIR}
    /usr/local/include
    /usr/include
    )
	
#need to find the bullet library files, this is basically a -L flag for gcc
link_directories(
		/usr/lib64
		/usr/lib
		)

ELSE(CMAKE_HOST_UNIX)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
    include
    include/RPC3
    include/DigiSpe
    src
    C:/Libs/lua-5.1.4/src
	C:/Libs/GLUTMingw32/include
	C:/Libs/bullet-2.76/src
	C:/Libs/DirectX-SDK-Feb2010/Include
	C:/Libs/OgreSDK_mingw_v1-7-0/include/OIS
	C:/Libs/OgreSDK_mingw_v1-7-0/include/OGRE
	C:/Libs/OgreSDK_mingw_v1-7-0/boost_1_42
	C:/Libs/luabind-0.9
	C:/Libs/freetype-2.3.12/include
	C:/Libs/pcre-8.02/build
	C:/Libs/CEGUI-0.7.1/cegui/include
    )
	
link_directories(
	C:/Libs/lua-5.1.4/src
	C:/Libs/GLUTMingw32/lib
	C:/Libs/bullet-2.76/build/lib
	C:/Libs/OgreSDK_mingw_v1-7-0/lib/release
	C:/Libs/OgreSDK_mingw_v1-7-0/boost_1_42/lib
	C:/Libs/luabind-0.9/build/Luabind
	C:/Libs/freetype-2.3.12/objs
	C:/Libs/pcre-8.02/build
	C:/Libs/CEGUI-0.7.1/bin
	)
ENDIF(CMAKE_HOST_UNIX)


SET(Libraries
    boost_thread-mt
    CEGUIBase
    CEGUIOgreRenderer
    OIS
    BulletDynamics
    BulletCollision
    LinearMath
    ${LUA51_LIBRARIES}
    luabind
    OgreMain
    enet
    X11
    )

set(Sources
src/aiFactionManager.cpp
src/aiGetNeighbours.cpp
src/aiHandlerBat.cpp
src/aiHandlerBattleSphere.cpp
src/aiHandlerBot.cpp
src/aiMovementController.cpp
src/aiPerception.cpp
src/aiPositionRating.cpp
src/block.cpp
src/blockEmpty.cpp
src/blockFull.cpp
src/blockLoaded.cpp
src/blockPersistent.cpp
src/blockPlaceholder.cpp
src/ceguiInputHandler.cpp
src/component.cpp
src/componentAI.cpp
src/componentBullet.cpp
src/componentCharacterController.cpp
src/componentDamageEffects.cpp
src/componentHealth.cpp
src/componentInventory.cpp
src/componentItem.cpp
src/componentMesh.cpp
src/componentParticle.cpp
src/componentPhysics.cpp
src/componentPlayer.cpp
src/componentPosition.cpp
src/componentTrigger.cpp
src/componentWeapon.cpp
src/componentLight.cpp
src/console.cpp
src/gameMode.cpp
src/input.cpp
src/level.cpp
src/level_other.cpp
src/levelElement.cpp
src/levelTextureElement.cpp
src/luaFunctions.cpp
src/main.cpp
src/marchingCubesTables.cpp
src/noise.cpp
src/object.cpp
src/objectScript.cpp
src/physics.cpp
src/precompiled.cpp
src/resource.h
src/settings.cpp
src/skysphere.cpp
src/stateGame.cpp
src/stateManager.cpp
src/stateMenu.cpp
src/surfacePatch.cpp
src/surfacePatchRenderable.cpp
src/tinystr.cpp
src/tinystr.h
src/tinyxml.cpp
src/tinyxml.h
src/tinyxmlerror.cpp
src/tinyxmlparser.cpp
src/util.cpp
src/volume.cpp
src/volumeLoadThread.cpp
src/weaponControlsInfo.cpp
src/weaponInHand.cpp
src/weaponInventory.cpp
src/windowManager.cpp
src/gameDataStorage.cpp
src/exposeToLua.cpp
src/network.cpp
src/levelObject.cpp
src/stateGameInputHandler.cpp
src/timer.cpp

src/DigiSpe/digiSpe.cpp
src/DigiSpe/digiSpeElement.cpp
src/DigiSpe/digiSpeTour.cpp
    )


SET(CurrentExe "foreverwar")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
ADD_EXECUTABLE(${CurrentExe} ${Sources})

# Precompiled headers this way only work on Linux
IF(CMAKE_HOST_UNIX)
	add_precompiled_header(${CurrentExe}
		 ${PROJECT_SOURCE_DIR}/include/precompiled.h
	)
ENDIF(CMAKE_HOST_UNIX)

TARGET_LINK_LIBRARIES(${CurrentExe} ${Libraries})

