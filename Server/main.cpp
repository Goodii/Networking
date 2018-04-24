#include <iostream>
#include <string>

#include <thread>
#include <chrono>

#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include "MessageIdentifiers.h"

void handleNetworkMessages(RakNet::RakPeerInterface*);

enum GameMessages
{
	ID_SERVER_TEXT_MESSAGE = ID_USER_PACKET_ENUM + 1,
};

int main()
{
	const unsigned short PORT = 5456;
	RakNet::RakPeerInterface* pPeerInterface = nullptr;

	//startup the server, and stat it listening to clients
	std::cout << "Starting up the server...\n";

	//initialise the raknet peer interface first
	pPeerInterface = RakNet::RakPeerInterface::GetInstance();

	//create a socket descriptor to describe this connection
	RakNet::SocketDescriptor sd(PORT, 0);

	//now call startup - max of 32 connections, on the assinged port
	pPeerInterface->Startup(32, &sd, 1);
	pPeerInterface->SetMaximumIncomingConnections(32);

	std::thread pingThread(sendClientPing, pPeerInterface);

	handleNetworkMessages(pPeerInterface);

	return 0;
}

void handleNetworkMessages(RakNet::RakPeerInterface* pPeerInterface)
{
	RakNet::Packet* packet = nullptr;

	while (true)
	{
		for (packet = pPeerInterface->Receive(); packet;
			pPeerInterface->DeallocatePacket(packet),
			packet = pPeerInterface->Receive())
		{
			switch (packet->data[0])
			{
			case ID_NEW_INCOMING_CONNECTION:
				std::cout << "A new connection is incoming.\n";
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				std::cout << "A client has disconnected.\n";
				break;
			case ID_CONNECTION_LOST:
				std::cout << "A client has lost connection.\n";
				break;
			case ID_SERVER_TEXT_MESSAGE:
			{
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

				RakNet::RakString str;
				bsIn.Read(str);
				std::cout << str.C_String() << std::endl;
				break;
			}
			default:
				std::cout << "Received a message with an unkown id: " << packet->data[0];
				break;
			}
		}
	}
}

void sendClientPing(RakNet::RakPeerInterface* pPeerInterface)
{
	while (true)
	{
		RakNet::BitStream bs;
		bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_TEXT_MESSAGE);
		bs.Write("Ping!");

		pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}