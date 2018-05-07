#pragma once

#include "Application.h"
#include <glm/mat4x4.hpp>

#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include <unordered_map>

struct UserData
{
	char username[32];
	char message[80];
};

struct GameObject
{
	glm::vec3 position;
	glm::vec4 colour;
};

class Client : public aie::Application {
public:

	Client();
	virtual ~Client();

	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime);
	virtual void draw();

	//initialise connection
	void handleNetworkConnection();
	void initialiseClientConnection();

	//handle incoming packets
	void handleNetworkMessages();

	void onSetClientIDPacket(RakNet::Packet * packet);

	void sendClientData();

	void onReceivedClientDataPacket(RakNet::Packet * packet);

protected:

	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;

	int m_clientID;
	UserData m_user;
	GameObject m_gameObject;

	std::unordered_map<int, GameObject> m_otherClientGameObjects;

	RakNet::RakPeerInterface* m_pPeerInterface;

	const char* IP = "127.0.0.1";
	const unsigned short PORT = 5456;
};