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

#ifndef _AI_MOVEMENT_CONTROLLER_H_
#define _AI_MOVEMENT_CONTROLLER_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
using namespace Ogre;

const float STD_FEELER_LENGTH = 5.0f;

enum AIMovementType
{
	AIMOVEMENT_NONE               = 0x00000,
 	AIMOVEMENT_SEEK               = 0x00002,
	//flee               = 0x00004,
 	AIMOVEMENT_ARRIVE             = 0x00008,
	/*wander             = 0x00010,
	cohesion           = 0x00020,*/
 	AIMOVEMENT_SEPARATION         = 0x00040,
	/*alignment         = 0x00080,
	obstacle_avoidance = 0x00100,*/
	AIMOVEMENT_WALL_AVOIDANCE     = 0x00200,
	/*follow_path        = 0x00400,
	pursuit            = 0x00800,
	evade              = 0x01000,
	interpose          = 0x02000,
	hide               = 0x04000,
	flock              = 0x08000,
	offset_pursuit     = 0x10000,*/
};

class ComponentPhysics;
class AIGetNeighbours;

/// Handles the movement of an agent based on multiple "steering behaviours" which can be turned on / off
class AIMovementController
{
public:
	/// Maximum speed
	float				maxSpeed;
	/// Maximum force
	float				maxForce;

	/// Manual force set by other AI parts
	Vector3		manualForce;
	/// The calculated force with which the agent should move
	Vector3		resultingForce;

	/// Target for behaviours like seek, arrive, ...
	Vector3		target;
	/// Current position of the agent
	Vector3		currentPos;
	/// Current velocity of the agent
	Vector3		velocity;
	btVector3			btVelocity;

	/// Which behaviours are turned on?
	int					movementTypes;

	/// How fast the agent should decelerate in arrive mode
	float				arriveDeceleration;

	/// Length of the feelers for wall avoidance
	float				feelerLength;

	/// The maximum distance for which separation takes effect
	float				separationMaximumDistance;

	/// The neighbour list for separation etc.
	AIGetNeighbours*	neighbours;

	/// The physics component of this agent
	ComponentPhysics*	physComp;


	/// Constructor
	AIMovementController(float maxForce, float maxSpeed, ComponentPhysics* physComp, AIGetNeighbours* neighbours = NULL);

	/// Sets a new target, resets the variables related to measuring the success at getting to the target
	void			setTarget(Vector3 target);
	
	/// Updates the target position (in contrast to setTarget() this method does not reset the success variables)
	void			updateTarget(Vector3 target);

	/// Calculates the updated velocity of the agent.
	Vector3	calculateVelocity(float dt);

	/// Calculates and sums the steering forces from any active behaviors. Usually, you don't need to call this - use calculateVelocity instead
	Vector3	calculateForce();

	/// Turns off all steering behaviours
	void			resetMovementTypes()				{movementTypes = AIMOVEMENT_NONE;}

	/// Sets the time after which, if no progress is made at getting to the target, getMoveToTargetFailed() returns true
	void			setNoProgressTimeout(float timeout)	{noProgressTimeout = timeout;}

	/// Returns if the agent failed to get to the target. Set the timeout with setNoProgressTimeout()
	bool			getMoveToTargetFailed()				{return moveToTargetFailed;}


	/* ......................................................
	              BEGIN BEHAVIOR DECLARATIONS
	.......................................................*/

	/// This behavior moves the agent towards a target position
	Vector3	seek(Vector3 targetPos);
	void			seekOn()	{movementTypes |= AIMOVEMENT_SEEK;}
	void			seekOff()	{movementTypes &= ~AIMOVEMENT_SEEK;}
	float			weightSeek;

	/// This behavior is similar to seek but it attempts to arrive
  	/// at the target position with a zero velocity
	Vector3	arrive(Vector3 targetPos, float deceleration);
	void			arriveOn()	{movementTypes |= AIMOVEMENT_ARRIVE;}
	void			arriveOff()	{movementTypes &= ~AIMOVEMENT_ARRIVE;}
	float			weightArrive;

	/// This returns a steering force which will keep the agent away from any
	/// walls it may encounter
	Vector3	wallAvoidance();
	void			wallAvoidanceOn()	{movementTypes |= AIMOVEMENT_WALL_AVOIDANCE;}
	void			wallAvoidanceOff()	{movementTypes &= ~AIMOVEMENT_WALL_AVOIDANCE;}
	float			weightWallAvoidance;

	/// This creates a force which separates the agent from other objects
	Vector3	separation(AIGetNeighbours* neighbours);
	void			separationOn()	{movementTypes |= AIMOVEMENT_SEPARATION;}
	void			separationOff()	{movementTypes &= ~AIMOVEMENT_SEPARATION;}
	float			weightSeparation;

protected:

	// Variables to measure the success at getting to the target
	float				noProgressTimer;		// how long the agent didn't get closer to its target
	float				noProgressTimeout;		// how fast should the agent give up and signal moveToTargetFailed?
	float				closestDistToTargetSq;	// what was the closest (squared) distance of the agent to the target?
	bool				moveToTargetFailed;		// did the agent fail at getting to the target?

	/// Adds force to the sum if the limit isn't reached yet
	bool      		accumulateForce(Vector3& sum, Vector3 force);
};

#endif
