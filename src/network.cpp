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
#include "network.h"

#ifndef DIGISPE

#include <enet/enet.h>
#include "settings.h"
#include "timer.h"

#include "stateGame.h"	// TODO: replace this with GAME SESSION

Network* network = NULL;

Network::Network()
{
	ok = false;
	currentTime = 0;
	enetHost = NULL;
	
	if (enet_initialize () != 0)
	{
		LogManager::getSingleton().logMessage("An error occurred while initializing ENet.", LML_CRITICAL);
		return;
	}
	
	ok = true;
}
Network::~Network()
{
	if (!ok)
		return;
	
	for (size_t i = 0; i < notAuthenticatedPlayers.size(); ++i)
		delete notAuthenticatedPlayers[i];
	for (size_t i = 0; i < playerList.size(); ++i)
		delete playerList[i];
	
	closeConnection();
	enet_deinitialize();
}

void Network::update(float dt)
{
	if (!isOk())
		return;
	if (!isConnected())
		return;
	
	currentTime += dt;
	
	if (isServer())
		serverUpdate();
	else
		clientUpdate();
}

bool Network::hostCustom()
{
	// Add ourself to the player list
	thisPlayer = new NetworkPlayer();
	thisPlayer->peer = NULL;
	thisPlayer->nickname = settings.nickname;
	thisPlayer->score = 0;
	thisPlayer->lastMsgTime = -1;
	thisPlayer->ready = true;
	playerList.push_back(thisPlayer);
	
	return true;
}
void Network::serverHandleConnect(ENetEvent* event)
{
	NetworkPlayer* newPlayer = new NetworkPlayer();
	
	newPlayer->peer = event->peer;
	event->peer->data = newPlayer;
	
	notAuthenticatedPlayers.push_back(newPlayer);
	
	// printf ("A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);
}
void Network::serverHandleDisconnect(ENetEvent* event)
{
	NetworkPlayer* player = (NetworkPlayer*)event->peer->data;
	event->peer->data = NULL;
	
	bool found = false;
	for (size_t i = 0; i < notAuthenticatedPlayers.size(); ++i)
	{
		if (notAuthenticatedPlayers[i] == player)
		{
			notAuthenticatedPlayers.erase(notAuthenticatedPlayers.begin() + i);
			delete player;
			return;
		}
	}
	for (size_t i = 0; i < playerList.size(); ++i)
	{
		if (playerList[i] == player)
		{
			playerList.erase(playerList.begin() + i);
			found = true;
			break;
		}
	}
	assert(found && "Tried to disconnect unknown player");
	
	// Remove player object if there
	// TODO
	
	// Send disconnect to all other players
	// TODO
	
	delete player;
}
bool Network::serverHandleReceive(ENetEvent* event)
{
	ENetPacket* p = event->packet;
	int len = (int)p->dataLength;
	unsigned char* d = (unsigned char*)p->data;
	NetworkPlayer* player = (NetworkPlayer*)event->peer->data;
	
	player->lastMsgTime = currentTime;
	
	if (len < 1)
		return false;
	
	unsigned char msg = d[0];
	if (msg == NETMSG_CLIENT_PING)
	{
		serverSendPing(player->peer, false);
		
		// This signals that we have to resend the ping in case there is no reply within 2 seconds
		player->score = 1;
	}
	else if (msg == NETMSG_CLIENT_READY)
	{
		if (state != NETSTATE_GAMESTART)
			return false;
		
		player->ready = true;
		player->score = 0;
	}
	else if (msg == NETMSG_CLIENT_JOIN)
	{
		if (state != NETSTATE_GAMESTART)
			return false;	// TODO: Let the player join when the next round begins
		if (len < 3)
			return false;
		char version = d[1];
		if (version != NETWORK_VERSION)
			return false;
		
		unsigned char nameLen = d[2];
		if (len < 3 + nameLen)
			return false;
		
		bool found = false;
		for (size_t i = 0; i < notAuthenticatedPlayers.size(); ++i)
		{
			if (notAuthenticatedPlayers[i] == player)
			{
				notAuthenticatedPlayers.erase(notAuthenticatedPlayers.begin() + i);
				found = true;
				break;
			}
		}
		if (!found)
			return false;
		
		// Add new player to list
		memcpy(buffer, &d[3], nameLen);
		buffer[nameLen] = 0;
		
		player->nickname = buffer;
		player->score = 0;
		player->ready = false;
		player->lastMsgTime = currentTime;
		
		playerList.push_back(player);
		
		// Respond with list of players (including the new player)
		bufsiz = 0;
		buffer[bufsiz++] = (char)NETMSG_SERVER_JOIN;
		buffer[bufsiz++] = (unsigned char)playerList.size();
		for (size_t i = 0; i < playerList.size(); ++i)
		{
			buffer[bufsiz++] = (char)playerList[i]->nickname.size();
			memcpy(&buffer[bufsiz], playerList[i]->nickname.c_str(), buffer[bufsiz-1]);
			bufsiz += (int)buffer[bufsiz-1];
			
			memcpy(&buffer[bufsiz], &playerList[i]->score, sizeof(int));
			bufsiz += sizeof(int);
		}
		
		ENetPacket* packet = enet_packet_create(buffer, bufsiz, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(player->peer, 0, packet);
		enet_host_flush(enetHost);
		
		// TODO: Send new player to other players
	}
	else if (msg == NETMSG_CLIENT_LOADING_PERCENTAGE_CHANGED)
	{
		if (state != NETSTATE_GAMESTART)
			return true;
		if (len != 2)
			return false;
		if (d[1] > 100)
			return false;
		
		player->score = d[1];
		if (!loadingPercentagesDirty)
		{
			loadingPercentagesDirty = true;
			loadingPercentagesDirtyTime = currentTime;
		}
	}
	else if (msg == NETMSG_CLIENT_CHAT)
	{
		if (len < 3)
			return false;
		
		unsigned char msglen = d[1];
		if (msglen + 2 > len)
			return false;
		
		memcpy(buffer, &d[2], msglen);
		buffer[msglen] = 0;
		
		for (size_t i = 0; i < playerList.size(); ++i)
		{
			if (playerList[i] == player)
			{
				sendChatMessage(buffer, i);
				return true;
			}
		}
	}
	
	return true;
}
void Network::serverUpdate()
{
	ENetEvent event;
	
	// Handle events
	while (enet_host_service(enetHost, &event, 0) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			serverHandleConnect(&event);
		break;
		case ENET_EVENT_TYPE_RECEIVE:
			if (!serverHandleReceive(&event))
				Ogre::LogManager::getSingleton().logMessage("Network::serverUpdate: Invalid packet received. TODO: disconnect client!");
			enet_packet_destroy(event.packet);
		break;
		case ENET_EVENT_TYPE_DISCONNECT:
			serverHandleDisconnect(&event);
		break;
		default:;
		}
	}
	
	if (state == NETSTATE_GAMESTART)
	{
		if (loadingPercentagesDirty && currentTime - loadingPercentagesDirtyTime >= LOADING_PERCENTAGES_UPDATE_INTERVAL)
			serverUpdateLoadingPercentages();
		
		// Check if we have to resend a ping
		for (size_t i = 1; i < playerList.size(); ++i)
		{
			if (!playerList[i]->ready && playerList[i]->score == 1 && (currentTime - playerList[i]->lastMsgTime > 2.0f))
			{
				serverSendPing(playerList[i]->peer, true);
				playerList[i]->lastMsgTime = currentTime;
			}
		}
	}
}
void Network::serverUpdateLoadingPercentages()
{
	// Send loading percentages of other players to every player
	for (size_t i = 1; i < playerList.size(); ++i)
	{
		if (!playerList[i]->ready)
			continue;
		
		bufsiz = 0;
		buffer[bufsiz++] = (char)NETMSG_SERVER_LOADING_PERCENTAGE_CHANGED;
		for (size_t k = 0; k < playerList.size(); ++k)
		{
			if (k == i)	continue;
			if (playerList[k]->ready)
				buffer[bufsiz++] = playerList[k]->score;
			else
				buffer[bufsiz++] = 0;
		}
		
		ENetPacket* packet = enet_packet_create(buffer, bufsiz, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(playerList[i]->peer, 0, packet);
	}
}
void Network::serverSendPing(ENetPeer* peer, bool resend)
{
	bufsiz = 0;
	buffer[bufsiz++] = (char)NETMSG_SERVER_PING;
	buffer[bufsiz++] = resend;
	memcpy(&buffer[bufsiz], &currentTime, sizeof(float));
	bufsiz += 4;
	
	ENetPacket* packet = enet_packet_create(buffer, bufsiz, 0);
	enet_peer_send(peer, 1, packet);
	enet_host_flush(enetHost);
}

void Network::clientHandleDisconnect(ENetEvent* event)
{
	enet_host_destroy(enetHost);
	enetHost = NULL;
}
bool Network::clientHandleReceive(ENetEvent* event)
{
	ENetPacket* p = event->packet;
	int len = (int)p->dataLength;
	unsigned char* d = (unsigned char*)p->data;
	//NetworkPlayer* player = (NetworkPlayer*)event->peer->data;
	
	if (len < 1)
		return false;
	
	unsigned char msg = d[0];
	if (msg == NETMSG_SERVER_LOADING_PERCENTAGE_CHANGED)
	{
		if (state != NETSTATE_GAMESTART)
			return true;
		if (len != (int)playerList.size())
			return false;
		
		int cursor = 1;
		for (size_t i = 0; i < playerList.size(); ++i)
		{
			if (playerList[i] == getThisPlayer())
				continue;
			playerList[i]->score = d[cursor];
			++cursor;
		}
	}
	else if (msg == NETMSG_SERVER_CHAT)
	{
		if (len < 4)
			return false;
		
		unsigned char playerno = d[1];
		
		unsigned char msglen = d[2];
		if (msglen + 2 > len)
			return false;
		
		memcpy(buffer, &d[3], msglen);
		buffer[msglen] = 0;
		
		displayChatMessage(buffer, playerno);
	}
	
	return true;
}
void Network::clientUpdate()
{
	ENetEvent event;
	
	while (enet_host_service(enetHost, &event, 0) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
			if (!clientHandleReceive(&event))
				Ogre::LogManager::getSingleton().logMessage("Network::clientUpdate: Invalid packet received. TODO: disconnect!");
			enet_packet_destroy(event.packet);
		break;
		case ENET_EVENT_TYPE_DISCONNECT:
			clientHandleDisconnect(&event);
		break;
		default:;
		}
	}
}
bool Network::joinCustom()
{
	// Send NETMSG_CLIENT_JOIN
	bufsiz = 0;
	buffer[bufsiz++] = (char)NETMSG_CLIENT_JOIN;
	buffer[bufsiz++] = (char)NETWORK_VERSION;
	buffer[bufsiz++] = (unsigned char)settings.nickname.length();
	memcpy(&buffer[bufsiz], settings.nickname.c_str(), buffer[bufsiz-1]);
	bufsiz += (int)buffer[bufsiz-1];
	
	ENetPacket* packet = enet_packet_create(buffer, bufsiz, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(server, 0, packet);
	enet_host_flush(enetHost);
	
	// Wait for reply
	ENetEvent event;
	bool success = false;
	while (!success && enet_host_service(enetHost, &event, 3000) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
			if (event.packet->dataLength >= 3 && event.packet->data[0] == NETMSG_SERVER_JOIN)
			{
				// Got the correct response
				int cursor = 2;
				unsigned char numOtherPlayers = event.packet->data[1];
				for (unsigned char i = 0; i < numOtherPlayers; ++i)
				{
					NetworkPlayer* newPlayer = new NetworkPlayer();
					
					unsigned char nameLen = event.packet->data[cursor];
					if ((int)event.packet->dataLength <= cursor + nameLen + 4)
						return false;
					memcpy(buffer, &event.packet->data[cursor+1], nameLen);
					buffer[nameLen] = 0;
					cursor += nameLen + 1;
					
					newPlayer->peer = NULL;
					newPlayer->nickname = buffer;
					newPlayer->score = *((int*)(&event.packet->data[cursor]));
					cursor += 4;
					
					playerList.push_back(newPlayer);
				}
				
				// We are always the last player in the list because the server added us with push_back()
				thisPlayer = playerList[playerList.size() - 1];
				
				success = true;
			}
			
			enet_packet_destroy(event.packet);
		break;
		case ENET_EVENT_TYPE_DISCONNECT:
			return false;
		break;
		default:;
		}
	}
	
	if (!success)
		return false;
	
	// Sync timers
	MyTimer timer;
	float pingSendTime;
	float delta = 0;
	int numDeltas = 0;
	const int numDeltasRequired = 4;
	
	while (numDeltas < numDeltasRequired)
	{
		// Send a ping
		clientSendPing();
		pingSendTime = timer.getTime();
			
		// Wait for reply, resend if no reply in 2 seconds
		success = false;
		bool resend = false;	// Was the client OR server ping resent? If so, do not use it for syncing
		while (!success)
		{
			while (!success && enet_host_service(enetHost, &event, 2000) > 0)
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_RECEIVE:
					if (event.packet->dataLength == 6 && event.packet->data[0] == NETMSG_SERVER_PING)
					{
						if ((bool)event.packet->data[1] == true)
							resend = true;
						
						if (!resend)
						{
							float myTime = timer.getTime();
							float ping = 0.5f * (myTime - pingSendTime);
							float serverTime = *((float*)&event.packet->data[2]) + ping;
							
							delta += serverTime - myTime;
							++numDeltas;
						}
						
						success = true;
					}
					break;
					
				case ENET_EVENT_TYPE_DISCONNECT:
					return false;
					break;
					
				default:;
				}
			}
			
			if (!success)
			{
				// No reply in 2 seconds - resend
				resend = true;
				
				clientSendPing();
				pingSendTime = timer.getTime();
			}
		}
	}
	
	delta /= numDeltas;
	currentTime = timer.getTime() + delta;
	
	// Send NETMSG_CLIENT_READY
	bufsiz = 0;
	buffer[bufsiz++] = (char)NETMSG_CLIENT_READY;
	
	packet = enet_packet_create(buffer, bufsiz, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(server, 0, packet);
	enet_host_flush(enetHost);
	
	return true;
}

void Network::clientSendPing()
{
	bufsiz = 0;
	buffer[bufsiz++] = (char)NETMSG_CLIENT_PING;
	
	ENetPacket* packet = enet_packet_create(buffer, bufsiz, 0);
	enet_peer_send(server, 1, packet);
	enet_host_flush(enetHost);
}

bool Network::host()
{
	bIsServer = true;
	
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = PORTRANGE_BEGIN;	// TODO: Try out a port range?
	
	enetHost = enet_host_create(&address, MAX_CLIENTS, 0, 0);
	if (!enetHost)
		return false;
	
	return hostCustom();					   
}
bool Network::join(const char* hostName)
{
	bIsServer = false;
	
	enetHost = enet_host_create (NULL, 1, 0, 0);
	if (!enetHost)
	{
		Ogre::LogManager::getSingleton().logMessage("Network::join: Could not create the client.");
		return false;
	}

	ENetAddress address;
	ENetEvent event;
	
	enet_address_set_host(&address, hostName);	// TODO: use enet_address_set_host?
	address.port = PORTRANGE_BEGIN;	// TODO: Try out a port range?
	
	// Initiate the connection
	server = enet_host_connect(enetHost, &address, NUM_CHANNELS);    
	
	if (!server)
	{
		Ogre::LogManager::getSingleton().logMessage("Network::join: No available peers for initiating an ENet connection.");
		return false;
	}
    
    // Wait for the connection attempt to succeed
	if (enet_host_service(enetHost, &event, CONNECT_ATTEMT_TIME) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
	{
		if (joinCustom())
			return true;
		else
		{
			enet_peer_reset(server);
			Ogre::LogManager::getSingleton().logMessage("Network::join: Server didn't respond correctly.");
			return false;
		}
	}
    else
	{
		// Either the 5 seconds are up or a disconnect event was received.
		// Reset the peer in the event the 5 seconds had run out without any significant event.
		enet_peer_reset(server);
		
		Ogre::LogManager::getSingleton().logMessage("Network::join: Connection failed.");
		return false;
	}				   
}
void Network::setState(ENetworkState newState)
{
	state = newState;
	
	if (state == NETSTATE_GAMESTART)
	{
		loadingPercentagesDirty = false;
	}
}
void Network::closeConnection()
{
	if (isConnected())
	{
		if (isServer())
		{
			// TODO
		}
		else
		{
			ENetEvent event;
			enet_peer_disconnect(server, 0);
			
			// Allow up to 3 seconds for the disconnect to succeed and drop any received packets
			bool success = false;
			while (!success && enet_host_service(enetHost, &event, 3000) > 0)
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_RECEIVE:
					enet_packet_destroy(event.packet);
				break;
				case ENET_EVENT_TYPE_DISCONNECT:
					success = true;
				break;
				default:;
				}
			}
		    
		    // The disconnect attempt didn't succeed yet. Force the connection down.
			if (!success)
				enet_peer_reset(server);
		}
	}
	
	if (enetHost)
	{
		enet_host_destroy(enetHost);
		enetHost = NULL;
	}
}

void Network::loadingPercentageChanged(int newPercentage)
{
	thisPlayer->score = newPercentage;
	
	if (isServer())
	{
		loadingPercentagesDirty = true;
		loadingPercentagesDirtyTime = currentTime;
	}
	else
	{
		// Send information to server instantly, send reliably if value is 100%
		bufsiz = 0;
		buffer[bufsiz++] = (char)NETMSG_CLIENT_LOADING_PERCENTAGE_CHANGED;
		buffer[bufsiz++] = (char)newPercentage;
		
		ENetPacket* packet = enet_packet_create(buffer, bufsiz, (newPercentage == 100) ? ENET_PACKET_FLAG_RELIABLE : 0);
		enet_peer_send(server, (newPercentage == 100) ? 0 : 1, packet);
	}
}
bool Network::areAllPlayersReady()
{
	for (size_t i = 0; i < playerList.size(); ++i)
	{
		if (!playerList[i]->ready || playerList[i]->score != 100)
			return false;
	}
	
	return true;
}
void Network::sendChatMessage(const std::string& msg, int playerno)
{
	if (playerno >= static_cast<int>(playerList.size()))
	{
		Ogre::LogManager::getSingleton().logMessage("Network::sendChatMessage: invalid playerno!");
		return;
	}
	
	bufsiz = 0;
	buffer[bufsiz++] = (isServer() ? (char)NETMSG_SERVER_CHAT : (char)NETMSG_CLIENT_CHAT);
	if (isServer())
	{
		if (playerno < 0)
		{
			for (size_t i = 0; i < playerList.size(); ++i)
			{
				if (playerList[i] == getThisPlayer())
				{
					playerno = i;
					break;
				}
			}
		}
		buffer[bufsiz++] = playerno;
	}
	buffer[bufsiz++] = (unsigned char)msg.size();
	memcpy(&buffer[bufsiz], msg.c_str(), buffer[bufsiz-1]);
	bufsiz += (int)buffer[bufsiz-1];
	
	if (isServer())
	{
		// Display msg and distribute it
		displayChatMessage(msg, playerno);
		
		for (size_t i = 1; i < playerList.size(); ++i)
		{
			ENetPacket* packet = enet_packet_create(buffer, bufsiz, ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(playerList[i]->peer, 0, packet);
		}
	}
	else
	{
		// Send msg to server
		ENetPacket* packet = enet_packet_create(buffer, bufsiz, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(server, 0, packet);
	}
}

void Network::displayChatMessage(const std::string& msg, int playerno)
{
	game.addMessage(playerList[playerno]->nickname + ": " + msg, MSGTYPE_CHAT);
}

#endif
