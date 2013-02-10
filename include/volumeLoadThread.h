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

#ifndef _VOLUME_LOAD_THREAD_H_
#define _VOLUME_LOAD_THREAD_H_

#include <list>
#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;
#include "point3.h"

#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>

class VoxelVolume;
class VoxelBlock;
class VolumeLoadThreadFunctor;

/// Handles the loading of new volume blocks in a thread
class VolumeLoadThread
{
public:
	boost::thread* thread;
	VolumeLoadThreadFunctor* functor;
	VoxelVolume* volume;

	std::list<Point3> input;
	boost::condition_variable_any inputCond;
	//boost::condition_variable inputCond;
	boost::mutex inputMut;

	std::list<VoxelBlock*> output;
	boost::mutex outputMut;

	// Shared variables
	/// Set to true if the thread should terminate
	bool endThread;

	/// Constructor, creates the thread
	VolumeLoadThread(VoxelVolume* volume);
	~VolumeLoadThread();

	/// Tell the thread to load the block located at position
	void load(Point3 position);

	/// Get a result if there is one - returns true if a result has been written to out
	bool getResult(VoxelBlock*& out);

	/// Clear the task list
	void clearTasks();
};

class VolumeLoadThreadFunctor
{
public:
	VolumeLoadThread* t;

	VolumeLoadThreadFunctor(VolumeLoadThread* t);
	void operator() ();
};

#endif
