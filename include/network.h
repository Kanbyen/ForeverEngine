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

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include "config.h"
#ifndef DIGISPE

#include <vector>
#include <string>

#define PORTRANGE_BEGIN 55500
#define PORTRANGE_END 55510
#define MAX_CLIENTS 16
#define NETWORK_VERSION 1

#define CONNECT_ATTEMT_TIME 5000
#define NUM_CHANNELS 2

#define LOADING_PERCENTAGES_UPDATE_INTERVAL 0.1f

typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;
typedef struct _ENetEvent ENetEvent;

enum ENetworkState
{
	/// Game start screen active
	NETSTATE_GAMESTART,
	/// Game in progress
	NETSTATE_PLAYING,
};

/// Messages sent by the client
enum ENetworkClientMessage
{
	/// The first message sent by the client when joining a game
	/**
		char version
		string nickName
	*/
	NETMSG_CLIENT_JOIN = 1,
	
	/// Message used to synchronize timers
	/**
		// no data
	*/
	NETMSG_CLIENT_PING = 4,
	
	/// Message used to signal that timer is synchronized
	/**
		// no data
	*/
	NETMSG_CLIENT_READY = 5,
	
	/// Game start screen: The loading percentage of the client has changed
	/**
		char percentage
	*/
	NETMSG_CLIENT_LOADING_PERCENTAGE_CHANGED = 2,
	
	/// The client send a chat message
	/**
		string msg
	*/
	NETMSG_CLIENT_CHAT = 3,
};

/// Messages sent by the server
enum ENetworkServerMessage
{
	/// The first message sent by the server when a new client joins a game (after the sever gets NETMSG_CLIENT_JOIN)
	/**
		unsigned char numPlayers	// including the new player
		for every player:
			string nickname
			int score
	*/
	NETMSG_SERVER_JOIN = 1,
	
	/// Message used to synchronize timers
	/**
		bool resend
		float currentTime
	*/
	NETMSG_SERVER_PING = 4,
	
	/// Game start screen: the server tells the clients the new loading percentages of the players
	/**
		for every other player:
			char percentage
	*/
	NETMSG_SERVER_LOADING_PERCENTAGE_CHANGED = 2,
	
	/// The server distributes a chat message to all players (including the player who sent it if that's not the server)
	/**
		unsigned char playerno	// the player who sent the message
		string msg
	*/
	NETMSG_SERVER_CHAT = 3,
};

/// Represents a player in a networked game. Get a list of these using the Network class. Clients get a list of participating players including themselves,
struct NetworkPlayer
{
	/// The player's nickname
	std::string nickname;
	
	/// Can be used for any type of "score", including the level loading percentage if the game is not started yet, or the join state
	/**
		While joining: on server, 0 means nothing special, 1 means that a ping has to be resent after 2 seconds if there is no reply until then
		NETSTATE_GAMESTART: loading percentage
	*/
	int score;
	
	/// The time of the last message which arrived from this client (server only)
	float lastMsgTime;
	
	/// True if the join procedure is complete (server only)
	bool ready;
	
	// TODO: mode (spectator / player)
	
	/// ENet peer. This is only valid if we are the server, so network.isServer() must be true!
	ENetPeer* peer;
};

/// Handles networking using ENet
class Network
{
public:
	
	typedef std::vector< NetworkPlayer* > PlayerList;
	
	/// Initialize the network library
	Network();
	~Network();
	
	
	/// Call this regularly to update the networking
	void update(float dt);
	
	/// Try to create a server to host a game. Returns true on success
	bool host();
	/// Try to create a client to join a game. Returns true on success
	bool join(const char* hostName);
	
	/// Set networking state
	void setState(ENetworkState newState);
	
	/// Disconnect
	void closeConnection();
	
	
	/// Game start screen: the loading percentage has changed
	void loadingPercentageChanged(int newPercentage);
	
	/// Game start screen: all players are ready (server only)
	bool areAllPlayersReady();
	
	/// Send a chat message. playerno is only relevant if this is the server. If playerno is negative, this player is used.
	void sendChatMessage(const std::string& msg, int playerno = -1);
	
	
	/// Get a list of players
	inline const PlayerList& getPlayerList() const {return playerList;}
	
	/// Get this player
	inline NetworkPlayer* getThisPlayer() const {return thisPlayer;}
	
	/// Are we connected to a server or client?
	inline bool isConnected() const {return enetHost != NULL;}
	
	/// Is this a server (as opposed to a client)?
	inline bool isServer() const {return bIsServer;}
	
	/// Is the network library initialized correctly?
	inline bool isOk() const {return ok;}
	
protected:
	
	bool ok;
	float currentTime;
	
	ENetHost* enetHost;
	bool bIsServer;
	
	int bufsiz;
	char buffer[512];
	
	ENetworkState state;
	
	/// If we are a client, this is the server peer
	ENetPeer* server;
	/// List of all players
	PlayerList playerList;
	/// Pointer to this player
	NetworkPlayer* thisPlayer;
	/// List of all players which connected but didn't send the NETMSG_CLIENT_JOIN yet; this is only used for the server
	PlayerList notAuthenticatedPlayers;
	
	
	/// Game start screen: if an update of the player loading percentages must be sent somewhen
	bool loadingPercentagesDirty;
	float loadingPercentagesDirtyTime;
	
	
	void serverUpdate();
	void serverHandleConnect(ENetEvent* event);
	void serverHandleDisconnect(ENetEvent* event);
	bool serverHandleReceive(ENetEvent* event);
	bool hostCustom();	// custom steps to host a game (not enet related)
	
	void serverUpdateLoadingPercentages();
	void serverSendPing(ENetPeer* peer, bool resend);
	
	
	void clientUpdate();
	void clientHandleDisconnect(ENetEvent* event);
	bool clientHandleReceive(ENetEvent* event);
	bool joinCustom();	// custom steps to join a game (not enet related)
	
	void clientSendPing();
	
	
	void displayChatMessage(const std::string& msg, int playerno);
};

extern Network* network;

#endif

#endif // _NETWORK_H_
