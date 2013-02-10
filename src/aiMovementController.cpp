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
#include "aiMovementController.h"
#include "componentPhysics.h"
#include "object.h"
#include "luaFunctions.h"
#include "stateGame.h"
#include "aiGetNeighbours.h"

AIMovementController::AIMovementController(float maxForce, float maxSpeed, ComponentPhysics* physComp, AIGetNeighbours* neighbours)
{
	movementTypes = AIMOVEMENT_NONE;
	resultingForce = Vector3::ZERO;
	manualForce = Vector3::ZERO;
	currentPos = physComp->object->position;
	velocity = Vector3::ZERO;
	btVelocity = btVector3(0, 0, 0);
	
	arriveDeceleration = 0.3f;
	feelerLength = STD_FEELER_LENGTH;
	separationMaximumDistance = 8.0f;
	
	weightArrive = 1.0f;
	weightSeek = 1.0f;
	weightWallAvoidance = 1.0f;
	weightSeparation = 1.0f;

	this->maxForce = maxForce;
	this->maxSpeed = maxSpeed;
	this->physComp = physComp;
	this->neighbours = neighbours;

	setTarget(Vector3::ZERO);
}

void AIMovementController::setTarget(Vector3 target)
{
	this->target = target;
	closestDistToTargetSq = 9999999.0f;	// very much
	noProgressTimer = 0.0f;
	moveToTargetFailed = false;
}
void AIMovementController::updateTarget(Vector3 target)
{
	this->target = target;
}

bool AIMovementController::accumulateForce(Vector3& sum, Vector3 force)
{
	// calculate how much steering force the vehicle has used so far
	float magnitudeSoFar = sum.length();

	// calculate how much steering force remains to be used by this vehicle
	float magnitudeRemaining = maxForce - magnitudeSoFar;

	// return false if there is no more force left to use
	if (magnitudeRemaining <= 0.0) return false;

	// calculate the magnitude of the force we want to add
	float magnitudeToAdd = force.length();

	// if the magnitude of the sum of ForceToAdd and the running total
	// does not exceed the maximum force available to this vehicle, just
	// add together. Otherwise add as much of the ForceToAdd vector as is
	// possible without going over the max.
	if (magnitudeToAdd < magnitudeRemaining)
		sum += force;
	else
		sum += (force.normalisedCopy() * magnitudeRemaining);

	return true;
}

Vector3 AIMovementController::calculateVelocity(float dt)
{
	Vector3 force = calculateForce();
	
	// We assume a mass of 1, so acceleration == force / mass == force
	velocity += force * dt;
	float velocityLength = velocity.length();
	if (velocityLength > maxSpeed)
	{
		velocity /= velocityLength;
		velocity *= maxSpeed;
	}
	btVelocity = btVector3(velocity.x, velocity.y, velocity.z);

	// check the success of the agent at getting to the target
	if (movementTypes & (AIMOVEMENT_ARRIVE | AIMOVEMENT_SEEK))
	{
		float distToTargetSq = (currentPos - target).squaredLength();
		if (distToTargetSq > 0.01f)
		{
			if (distToTargetSq < closestDistToTargetSq)
			{
				noProgressTimer = 0;
				closestDistToTargetSq = distToTargetSq;
			}
			else
				noProgressTimer += dt;

			if (noProgressTimer > noProgressTimeout)
				moveToTargetFailed = true;
		}
		else
		{
			moveToTargetFailed = false;
			noProgressTimer = 0;
		}
	}

	return velocity;
}
Vector3 AIMovementController::calculateForce()
{
	Vector3 force;
	resultingForce = Vector3::ZERO;

	// Get current position and velocity from the physics component
	currentPos = physComp->object->position;
	//btVelocity = physComp->body->getLinearVelocity();
	//velocity = Vector3(btVelocity.x(), btVelocity.y(), btVelocity.z());
	
	// Apply steering behaviours (and manual force)
	if (!accumulateForce(resultingForce, manualForce)) return resultingForce;

	if (movementTypes & AIMOVEMENT_WALL_AVOIDANCE)
	{
		force = wallAvoidance() * weightWallAvoidance;
		if (!accumulateForce(resultingForce, force)) return resultingForce;
	}

	if (movementTypes & AIMOVEMENT_SEPARATION)
	{
		force = separation(neighbours) * weightSeparation;
		if (!accumulateForce(resultingForce, force)) return resultingForce;
	}

	if (movementTypes & AIMOVEMENT_SEEK)
	{
		force = seek(target) * weightSeek;
		if (!accumulateForce(resultingForce, force)) return resultingForce;
	}
	
	if (movementTypes & AIMOVEMENT_ARRIVE)
	{
		force = arrive(target, arriveDeceleration) * weightArrive;
		if (!accumulateForce(resultingForce, force)) return resultingForce;
	}

	return resultingForce;
}

/* ......................................................
				BEGIN BEHAVIOR DECLARATIONS
.......................................................*/

Vector3 AIMovementController::seek(Vector3 targetPos)
{
	Vector3 desiredVelocity = targetPos - currentPos;
	desiredVelocity.normalise();
	desiredVelocity *= maxSpeed;

	return (desiredVelocity - velocity);
}

Vector3 AIMovementController::arrive(Vector3 targetPos, float deceleration)
{
	Vector3 toTarget = targetPos - currentPos;

	// calculate the distance to the target
	float dist = toTarget.length();

	if (dist > 0)
	{
		// calculate the speed required to reach the target given the desired deceleration
		float speed = dist / deceleration;

		// make sure the velocity does not exceed the max
		speed = min(speed, maxSpeed);

		// from here proceed just like Seek except we don't need to normalize
		// the toTarget vector because we have already gone to the trouble
		// of calculating its length: dist.
		Vector3 desiredVelocity = (toTarget / dist) * speed;

		return (desiredVelocity - velocity);
	}

	return Vector3::ZERO;
}

Vector3 AIMovementController::wallAvoidance()
{
	Vector3 force = Vector3::ZERO;
	const Vector3 feelers[5] = {Vector3(0, 0, -1),
									  Vector3(0.707106781, 0, -0.707106781),
									  Vector3(-0.707106781, 0, -0.707106781),
									  Vector3(0, 0.707106781, -0.707106781),
									  Vector3(0, -0.707106781, -0.707106781)};

	btQuaternion btCurQuat = physComp->body->getOrientation();
	Quaternion curQuat = Quaternion(btCurQuat.w(), btCurQuat.x(), btCurQuat.y(), btCurQuat.z());
									  
	// For each feeler ...
	for (int i = 0; i < 5; i++)
	{
		Vector3 transformedFeeler = curQuat * feelers[i];
		Vector3 hitPos;
		if (luaRayTestVoxel(&currentPos, &transformedFeeler, feelerLength, &hitPos))
		{
			// Calculate volume normal at hit Position
			int x = (int)((hitPos.x + 0.5f) / game.volumeData->scale);
			int y = (int)((hitPos.y + 0.5f) / game.volumeData->scale);
			int z = (int)((hitPos.z + 0.5f) / game.volumeData->scale);
			Vector3 normal = Vector3(game.volumeData->getVoxelAt(x - 1, y    , z    ) -
									 game.volumeData->getVoxelAt(x + 1, y    , z    ),
									 game.volumeData->getVoxelAt(x    , y - 1, z    ) -
									 game.volumeData->getVoxelAt(x    , y + 1, z    ),
									 game.volumeData->getVoxelAt(x    , y    , z - 1) -
									 game.volumeData->getVoxelAt(x    , y    , z + 1)).normalisedCopy();
			force += (normal) / (hitPos - currentPos).length();
		}
	}

	return force;
}

Vector3 AIMovementController::separation(AIGetNeighbours* neighbours)
{
	Vector3 force = Vector3::ZERO;
	void* unsused;

	for (AIGetNeighbours::iterator it = neighbours->begin(); it != neighbours->end(); it++)
	{
		// Only separate if the other object has a physics component
		if (!(*it)->sendGetMessage(COMPMSG_GET_PHYSICS_COMPONENT, &unsused))
			continue;
		
		Vector3 toAgent = currentPos - (*it)->position;
		float toAgentLength = toAgent.length();
		if ((toAgentLength > separationMaximumDistance) || (toAgentLength == 0.0f))
			continue;
		force += (toAgent / toAgentLength) / toAgentLength;
	}
	
	return force;
}
