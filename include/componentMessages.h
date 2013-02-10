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

#ifndef _COMPONENT_MESSAGES_H_
#define _COMPONENT_MESSAGES_H_

enum ComponentMessages					// sent ...														parameter type(s) (*)	parameter semantic
{
	COMPMSG_MSGS_BEGIN,
	COMPMSG_CREATE,						// after an object is created to place it in the world			Ogre::Vector3			position; can also be NULL, then the object was already in the world at some time and is now re-inserted; it should use its old position
	COMPMSG_UNLOAD,						// when an object gets out of the world and is stored in the gameDataStorage. If the object is not worth it (e.g. a bullet), it can delete itself as response to this message. Otherwise, it should get into a state where a CREATE msg can re-activate the object.
	COMPMSG_UPDATE,						// after every logic step										-
	COMPMSG_UPDATEPOS,					// after every physics step										float					time since last frame
	//COMPMSG_HIDE,						// when an object should hide itself - TODO: obsolete, use 	COMPMSG_UNLOAD!		-
	//COMPMSG_SHOW,						// when an object should show itself - TODO: obsolete, use 	COMPMSG_CREATE!		Ogre::Vector3			position
	//COMPMSG_MANUALDELETE,				// when an object is deleted manually (not by the "garbage collecting" at exit())
	COMPMSG_HANDLETRIGGER,				// when an object is notified of a trigger						Trigger					trigger which caused the message
	COMPMSG_COLLIDED,					// when an object collided with something						float + Object			strength of the collision + collision partner
	COMPMSG_APPLYCENTRALIMPULSE,		// to apply a central impule to the rigid body					Ogre::Vector3			impulse direction and magnitude
	COMPMSG_APPLYIMPULSE,				// to apply an impulse to the rigid body						2 x Ogre::Vector3		impulse + relative position
	COMPMSG_SETTARGETPOSITION,			// to set the target position of an object						ComponentPosition
	COMPMSG_SET_AIM,					// to set the "aim"												Ogre::Vector3			the new aim direction
	//COMPMSG_ATTACH_TO_NODE,			// to attach all scene nodes of an object to the given node		Ogre::Node				the node to attach the object to
	COMPMSG_ATTACH_CHILD,				// to attach a child object										Object					the child object
	COMPMSG_ATTACH_CHILD_TO_WEAPON,		// to attach a child object to the weapon of this object		Object					the child object
	COMPMSG_TAKEDAMAGE,					// to let the object take damage								float + int				amount of damage + damage type
	COMPMSG_DAMAGEEFFECT,				// to let the object display a damage effect					int	+ Vector3			damage type + effect position
	COMPMSG_SET_POSITION,				// to set the position 											Ogre::Vector3
	COMPMSG_SET_VELOCITY,				// to set the velocity              							Ogre::Vector3
	COMPMSG_SET_ORIENTATION,			// to set the Orientation										btQuaternion
	COMPMSG_SET_SCALE,					// to set the scale												Ogre::Vector3
	COMPMSG_MSGS_END,

	COMPMSG_GET_MSGS_BEGIN,
	COMPMSG_GET_HEAD,					// to get the "head"											Ogre::Vector3
	COMPMSG_GET_AIM,					// to get the "aim"												Ogre::Vector3
	COMPMSG_GET_RIGIDBODY,				// to get the rigidbody											btRigidBody*
	COMPMSG_GET_MOVABLE_OBJECT,			// to get the movable object									Ogre::MovableObject*
	COMPMSG_GET_MUZZLE_REL,				// to get the muzzle position (relative to obj center)			Ogre::Vector3
	COMPMSG_GET_NODE,					// to get the node												Ogre::Node*
	COMPMSG_GET_HEALTH,					// to get the health											float
	COMPMSG_GET_MAX_HEALTH,				// to get the maximum health									float
	COMPMSG_GET_NAME,					// to get the (item, actor, ...) name							string
	COMPMSG_GET_FACTION,				// to get the faction name										string
	COMPMSG_GET_VELOCITY,				// to get the velocity											Ogre::Vector3
	COMPMSG_GET_HOLDER_ID,				// to get the id of the (weapon) holder							int
	COMPMSG_GET_INVENTORY,				// to get the ComponentInventory								ComponentInventory*
	COMPMSG_GET_ORIENTATION,			// to get the Orientation										btQuaternion
	COMPMSG_GET_SCALE,					// to get the scale												Ogre::Vector3
	// Messages to get various components - TODO: obsolete. Use object->getComponent(const char* name) instead!
	COMPMSG_GET_ITEM_COMPONENT,			// to get the ComponentItem										ComponentItem*
	COMPMSG_GET_PHYSICS_COMPONENT,		// to get the ComponentPhysics									ComponentPhysics*
	COMPMSG_GET_CHARACTER_CONTROLLER,	// to get the ComponentCharacterController						ComponentCharacterController*
	COMPMSG_GET_MSGS_END,

	COMPMSG_AI_MSGS_BEGIN,
	COMPMSG_AI_PERCEPTION_NOTIFICATION,	// to notify neighbours of percepted agents (help me!)			AIPerception
	COMPMSG_AI_ATTACKEDBY,				// to notify an agent that it was attacked by another one		float + int				amount of damage + object id of aggressor
	COMPMSG_AI_MSGS_END,
};

#endif
