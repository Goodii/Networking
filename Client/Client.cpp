#include "Client.h"
#include "Gizmos.h"
#include "Input.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include "GameMessages.h"
#include <sstream>
#include <imgui.h>

using glm::vec3;
using glm::vec4;
using glm::mat4;
using aie::Gizmos;
using namespace std;
using namespace ImGui;

Client::Client() {

}

Client::~Client() {
}

bool Client::startup() {
	
	setBackgroundColour(0.25f, 0.25f, 0.25f);

	// initialise gizmo primitive counts
	Gizmos::create(10000, 10000, 10000, 10000);

	// create simple camera transforms
	m_viewMatrix = glm::lookAt(vec3(10), vec3(0), vec3(0, 1, 0));
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f,
										  getWindowWidth() / (float)getWindowHeight(),
										  0.1f, 1000.f);

	m_gameObject.position = glm::vec3(0, 0, 0);
	m_gameObject.colour = glm::vec4(1, 0, 1, 1);


	handleNetworkConnection();

	return true;
}

void Client::shutdown() {

	Gizmos::destroy();
}

void Client::update(float deltaTime) {

	// query time since application started
	float time = getTime();

	// wipe the gizmos clean for this frame
	Gizmos::clear();	
	
	handleNetworkMessages();

	// quit if we press escape
	aie::Input* input = aie::Input::getInstance();

	if (input->isKeyDown(aie::INPUT_KEY_RIGHT))
	{
		m_gameObject.position.x += 10.f * deltaTime;
		m_gameObject.position.z -= 10.f * deltaTime;
		sendClientData();
	}
	if (input->isKeyDown(aie::INPUT_KEY_LEFT))
	{
		m_gameObject.position.x -= 10.f * deltaTime;
		m_gameObject.position.z += 10.f * deltaTime;
		sendClientData();
	}
	if (input->isKeyDown(aie::INPUT_KEY_UP))
	{
		m_gameObject.position.x -= 10.f * deltaTime;
		m_gameObject.position.z -= 10.f * deltaTime;
		sendClientData();
	}
	if (input->isKeyDown(aie::INPUT_KEY_DOWN))
	{
		m_gameObject.position.x += 10.f * deltaTime;
		m_gameObject.position.z += 10.f * deltaTime;
		sendClientData();
	}

	if (input->wasKeyPressed(aie::INPUT_KEY_ENTER))
	{
		std::string msg;
		getline(cin, msg);

		
	}

	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();
}

void Client::draw() {

	// wipe the screen to the background colour
	clearScreen();

	// update perspective in case window resized
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f,
										  getWindowWidth() / (float)getWindowHeight(),
										  0.1f, 1000.f);

	Gizmos::addSphere(m_gameObject.position, 1.f, 32, 32, m_gameObject.colour);

	for (auto& otherClient : m_otherClientGameObjects) 
	{ 
		Gizmos::addSphere(otherClient.second.position, 1.0f, 32, 32, otherClient.second.colour); 
	}

	Gizmos::draw(m_projectionMatrix * m_viewMatrix);
}

void Client::handleNetworkConnection()
{
	//initialise the raknet peer interface first
	m_pPeerInterface = RakNet::RakPeerInterface::GetInstance();
	initialiseClientConnection();
}

void Client::initialiseClientConnection()
{
	//Create a socket descriptor to describe this connection
	//no data needed, as we will be connecting to a server
	RakNet::SocketDescriptor sd;

	//Now call startup - max of 1 connection (to the server)
	m_pPeerInterface->Startup(1, &sd, 1);

	cout << "Connecting to server at: " << IP << endl;

	//call connect to attempt to connect to the given server
	RakNet::ConnectionAttemptResult res = m_pPeerInterface->Connect(IP, PORT, nullptr, 0);

	//finaly, check to see if we connected, and if not, throw an error
	if (res != RakNet::CONNECTION_ATTEMPT_STARTED)
	{
		cout << "Unable to start connection, Error number: " << res << endl;
	}
}

void Client::handleNetworkMessages()
{
	RakNet::Packet* packet;

	for (packet = m_pPeerInterface->Receive(); packet;
		m_pPeerInterface->DeallocatePacket(packet),
		packet = m_pPeerInterface->Receive())
	{
		switch (packet->data[0])
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			cout << "Another client has disconnected.\n";
			break;
		case ID_REMOTE_CONNECTION_LOST:
			cout << "Another client has lost connection.\n";
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			cout << "Another client has connected.\n";
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			cout << "Our connection request has been accepted.\n";
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			cout << "The server is full.\n";
			break;
		case ID_CONNECTION_LOST:
			cout << "Connection lost.\n";
			break;
		case ID_SERVER_SET_CLIENT_ID:
			onSetClientIDPacket(packet);
			break;
		case ID_CLIENT_CLIENT_DATA:
			onReceivedClientDataPacket(packet);
			break;
		default:
			cout << "Received a message with an unkown id: " << packet->data[0] << endl;
			break;
		}
	}
}

void Client::onSetClientIDPacket(RakNet::Packet* packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	bsIn.Read(m_clientID);

	cout << "Set my client ID to: " << m_clientID << endl;
}

void Client::sendClientData()
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_CLIENT_CLIENT_DATA);
	bs.Write(m_clientID);
	bs.Write((char*)&m_gameObject, sizeof(GameObject));

	m_pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
		RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Client::onReceivedClientDataPacket(RakNet::Packet* packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

	int clientID;
	bsIn.Read(clientID);

	//if clientID does not match our ID, update our client GameObject information
	if (clientID != m_clientID)
	{
		GameObject clientData;
		bsIn.Read((char*)&clientData, sizeof(GameObject));

		m_otherClientGameObjects[clientID] = clientData;

		//output GameObject information to console
		cout << clientData.position.x << " : " << clientData.position.z << endl;
	}
}

