bin_PROGRAMS = foreverwar

# set the include path found by configure
INCLUDES = $(all_includes) -I$(top_srcdir)/include -I/usr/include/lua50 \
	-I/usr/local/include -I/usr/local/include/CEGUI -I/usr/local/include/OIS -I/usr/local/include/bullet

# the library search path.
foreverwar_LDFLAGS = $(all_libraries) -L/usr/local/lib -lCEGUIBase \
	-lCEGUIOgreRenderer -lOIS -lbulletdynamics -lbulletcollision -lbulletmath -llua50 -lluabind \
	/usr/lib/libboost_thread.a

LDADD = @OGRE_LIBS@
AM_CXXFLAGS = @OGRE_CFLAGS@ -Wall

foreverwar_LDADD = /usr/local/lib/libOgreMain.so

foreverwar_SOURCES = aiFactionManager.cpp aiGetNeighbours.cpp \
	aiHandlerBattleSphere.cpp aiMovementController.cpp aiPerception.cpp aiPositionRating.cpp \
	block.cpp blockEmpty.cpp blockFull.cpp blockLoaded.cpp blockPersistent.cpp \
	blockPlaceholder.cpp component.cpp componentAI.cpp componentBullet.cpp \
	componentCollisionListener.cpp componentDamageEffects.cpp componentHealth.cpp componentItem.cpp \
	componentMesh.cpp componentParticle.cpp componentPhysics.cpp componentPlayer.cpp \
	componentPosition.cpp componentTimer.cpp componentWeapon.cpp console.cpp input.cpp level.cpp \
	levelElement.cpp luaFunctions.cpp main.cpp marchingCubesTables.cpp noise.cpp object.cpp \
	objectScript.cpp physics.cpp precompiled.cpp settings.cpp stateGame.cpp stateManager.cpp \
	stateMenu.cpp surfacePatch.cpp surfacePatchRenderable.cpp tinystr.cpp tinyxml.cpp \
	tinyxmlerror.cpp tinyxmlparser.cpp util.cpp volume.cpp volumeLoadThread.cpp \
	weaponControlsInfo.cpp weaponInHand.cpp weaponInventory.cpp windowManager.cpp


BUILT_SOURCES = $(top_srcdir)/include/precompiled.h.gch
%.h.gch: %.h
	$(CXXCOMPILE) -c $<
