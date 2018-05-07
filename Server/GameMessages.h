#pragma once

#include "MessageIdentifiers.h"
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include <thread>

enum GameMessages
{
	ID_SERVER_TEXT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	ID_SERVER_SET_CLIENT_ID,
	ID_CLIENT_CLIENT_DATA,
};

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