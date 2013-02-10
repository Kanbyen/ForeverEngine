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

#ifndef _LEVEL_H_
#define _LEVEL_H_

#include <vector>
#include "point3.h"
#include "levelObject.h"
#include "blockConstants.h"

const int NUM_LEVELS = 3;
const int NUM_LEVEL_LAYERS = 2;
const int LEVEL_LENGTH = 260;
const int MAX_ELEMENTS_PER_BLOCK = 128;

class LevelElement;
class LevelTextureElement;

/// Represents the game world. Provides functions for level generation and callbacks to fill the voxel blocks accordingly. Also creates the objects in the level.
class Level
{
protected:

	struct Building;
	struct DoorOrWindowSpace;

public:
	Level(float scale);
	~Level();

	float scale;

	typedef std::vector<Point3> PositionList;

	/// The level generation functions fill this list with the elements (cubes, spheres) of the level. When a new voxel block is requested, its voxel data is determined using this list
	typedef std::vector<LevelElement*> ElementList;
	typedef ElementList::iterator ElementListItr;
	ElementList elements[NUM_LEVELS][NUM_LEVEL_LAYERS];	// TODO: This list and the equivalent textures are not correctly "push_back" and "clear"ed!

	LevelElement* tempElementList[NUM_LEVEL_LAYERS][MAX_ELEMENTS_PER_BLOCK];
	int tempElementListSize[NUM_LEVEL_LAYERS];

	/// The equivalent to ElementList for textures
	typedef std::vector<LevelTextureElement*> TextureList;
	typedef TextureList::iterator TextureListItr;
	TextureList textures[NUM_LEVELS];

	LevelTextureElement* tempTextureList[MAX_ELEMENTS_PER_BLOCK];
	int tempTextureListSize;

	/// Generate level layout
	void generate();


	// Generate functions for the specific level types
	void generate_Cave(ElementList& entrances, ElementList& exits, int startHeight, int endHeight);
	void generate_City(ElementList& entrances, ElementList& exits, int startHeight, int endHeight);
	void generate_RockField(ElementList& entrances, ElementList& exits, int startHeight, int endHeight);


	// Building generation
	void generateHouse(Building& b, int doorDirs, int texFloor);
	void generateSkyscraper(Building& b, int doorDirs, int texFloor);


	// Generate functions for objects
 	void generateObj_StartingEquipment(Point3 midpoint);
	void generateObj_SmallRoom(const ObjectSpawnArea& area);	// "small room" means: can contain anything
	void generateObj_OpenStreet(const ObjectSpawnArea& area);


	/// Generates entrances of level n+1 from exits of level n
	void createEntrancesFromExits(ElementList& dest, ElementList& src, int n);

	/// Fill the given area with random spheres which keep a minimum distance from each other
	void createRandomSpheresAt(Vector3 min, Vector3 max, int rmin, int rmax, int mindist, int num, bool inversed, ElementList& out, float radiusCap = 1.0f);

	/// Fill the given area with random cubes which keep a minimum distance from each other
	void createRandomCubesAt(Vector3 min, Vector3 max, Vector3 rmin, Vector3 rmax, int mindist, int num, bool inversed, ElementList& out);

	/// Get a random position in the given area which has a minimum distance to other specified positions
	Point3 getRandomFreePosition(Point3 min, Point3 max, PositionList& blockerList, float mindist);

	/// Range must be completely in one level for this function
	int create(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, int blx, int bly, int baseX, int levelNumber, int levelTransMin, int levelTransMax, bool oneLevel, char* buffer);
	/// Level creation function, splits the range if it contains two levels and calls create()
	static int createFunc(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer);
	/// Range must be completely in one level for this function
	void createTex(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, int blx, int bly, int baseX, int levelNumber, char* voxelBuffer, unsigned char* buffer);
	/// Level texturing function
	static void textureFunc(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* voxelBuffer, unsigned char* buffer);

	/// Converts the level-space point to a vector with "real" coordinates which can be used, for example, for object placement
	Vector3 pointToVector(const Point3& point);

	// ### Other level layouts (not used by the game, only in sandbox mode) ###
	static int other_InvCube_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer);
	static int other_Cube_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer);
	static int other_Cave_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer);
	static int other_Terrain_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer);
	static int other_Planes_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer);
	static int other_Perlin_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer);
	static int other_Sphere_VoxelCB(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* buffer);

	static void other_Invert();
	static void other_ApplyTextures(int firstX = 1, int firstY = 1, int firstZ = 1, int lastX = VOLUME_SIDE_LENGTH_IN_VOXELS - 2, int lastY = VOLUME_SIDE_LENGTH_IN_VOXELS - 2, int lastZ = VOLUME_SIDE_LENGTH_IN_VOXELS - 2);
	static void other_autotexCallback(int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ, char* voxelBuffer, unsigned char* buffer);

protected:

	/// Number of level which is currently generated
	int actLevel;
	
	/// Connects two level elements. This class only holds the data; how the connection will be used is up to the level generation function
	class Connection
	{
	public:
		Connection(Vector3 start, Vector3 end);
		Connection(LevelElement* start, LevelElement* end);

		Vector3 start;
		Vector3 end;
		Vector3 midPoint;
		float lengthSq;
	};

	typedef std::vector<Connection*> ConnectionList;
	typedef ConnectionList::iterator ConnectionListItr;

	/// Represents an object generated in the level generation process
	struct LevelObject
	{
		string type;
		Vector3 pos;
	};

	typedef std::vector<LevelObject> ObjectList;
	typedef ObjectList::iterator ObjectListItr;
	/// The level generation functions fill this list with object information. At the end of the level generation, all objects are generated and added to the gameDataStore and this list is cleared
	ObjectList objects;

	/// Information about spaces inside walls where doors or windows will be placed
	struct DoorOrWindowSpace
	{
		Point3 bbmin;
		Point3 bbmax;
		bool enabled;
		bool window;	// true if this is a window space, false if this is a door space
		int minimumWidth;

		bool contains(Point3 p);
		void splitAt(Point3 p, int iDir);
		void placeDoor(int width, int height, int layer);
		void placeWindow(int width, int height, int heightAboveGround, int layer);
	};
	typedef std::vector<DoorOrWindowSpace> DoorOrWindowSpaceList;

	/// Information about buildings
	struct Building
	{
		/// Bounding box
		Point3 bbmin, bbmax;
		/// Bounding box of the floor which is currently being worked on; set this using setActFloor()
		Point3 roombbmin, roombbmax;
		/// List of bounding boxes in which doors must be placed to guarantee accessibility of every room; access: doorSpaces[floor]
		DoorOrWindowSpaceList* doorSpaces;

		/// a 2D map of the building, 0 means free space, 1 means wall, 2 means free space which should remain free; access: bmap[floor][x][y];
		/// bmap[0][0][0] is the voxel at bbmin; size: bmap[numFloors][bsizex][bsizez]
		char*** bmap;

		/// information for splits
		int bsizex, bsizez;
		int posXMin, posXMax;
		int posZMin, posZMax;
		bool xsplitPossible, zsplitPossible;

		/// if multiple floors are present
		int numFloors;
		int actFloor;
		int floorHeight;	// Height of the rooms in a floor; don't forget the roof!

		/// Constructor & Destructor (clears up bmap if non-NULL)
		Building();
		~Building();

		// Building generation helpers

		/// Places a hollow block and inserts the first door space for the entrance
		void generateBuildingBase(int doorDirs, int texFloor, int doorWidth, int windowWidth);
		/// Calculates internal variables
		/// splitFreeSpaceNeeded: Number of free voxels (split point voxel excluded) before / behind door needed for a valid split point position;
		/// determines the minimum size of the resulting rooms:
		void calculateSplitSizes(int splitFreeSpaceNeeded);
		/// Creates the bmap, fills it with the initial room
		void createBMap();
		/// Splits the building up into smaller rooms.
		void splitUp(int numSplitTries, int splitFreeSpaceNeeded, int doorWidth, bool carveCornerAllowed, int carveOutPercentage);
		/// Makes doors from doorSpaces; does not clear the door spaces list
		void createDoorsAndWindows(int doorWidth, int doorHeight, int windowWidth, int windowHeight, int windowHeightAboveGround);

		/// Calculates the volume defined by roombbmin and roombbmax
		int getRoomVolume();
		/// Called by generateBuildingBase. dir is form the BuildingDirection enum
		void getDoorOrWindowSpaceInBuildingSide(int dir, int floor, bool window, int doorWidth, int windowWidth);
		/// Calculate the number of floors which fit into the given bb and adjust the bb to the minimum possible size
		void calcNumFloorsAndAdjustBB(int floorHeight);
		/// Set roombbmin / roombbmax
		void setActFloor(int floor);
	};
	typedef std::vector<Building*> BuildingList;

	enum BuildingDirection
	{
		BPLUSX = 1 << 0,
		BMINUSX = 1 << 1,
		BPLUSZ = 1 << 2,
		BMINUSZ = 1 << 3
	};

	void getLevelElementsInRange(int levelNumber, int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ);
	void getLevelTexturesInRange(int levelNumber, int firstX, int firstY, int firstZ, int lastX, int lastY, int lastZ);
};

extern Level* level;

#endif
