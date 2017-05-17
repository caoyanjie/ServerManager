#include "NATPunchthroghFacilitator.h"

#include "RakNet/RakPeerInterface.h"
#include "RakNet/NatPunchthroughServer.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"

#include <thread>

RakNetServer::NATPunchthroghFacilitator *RakNetServer::NATPunchthroghFacilitator::m_instance = nullptr;
RakNetServer::NATPunchthroghFacilitator::NATPunchthroghFacilitator() 
	: m_isWorking(false),
	NAT_PUNCHGHROUGH_FACILITATOR_PORT(50002)
{
}


RakNetServer::NATPunchthroghFacilitator::~NATPunchthroghFacilitator()
{
}

RakNetServer::NATPunchthroghFacilitator* RakNetServer::NATPunchthroghFacilitator::GetInstance()
{
	if (nullptr == m_instance)
	{
		m_instance = new RakNetServer::NATPunchthroghFacilitator;
	}
	return m_instance;
}

void RakNetServer::NATPunchthroghFacilitator::start()
{
	if (m_isWorking)
	{
		return;
	}
	m_isWorking = true;
	std::thread NATPunchthroughServer(&RakNetServer::NATPunchthroghFacilitator::init, this);
	NATPunchthroughServer.detach();
}

void RakNetServer::NATPunchthroghFacilitator::init()
{
	m_natPunchthroughFacilitator = RakNet::RakPeerInterface::GetInstance();
	RakNet::StartupResult result = m_natPunchthroughFacilitator->Startup(8096, &RakNet::SocketDescriptor(NAT_PUNCHGHROUGH_FACILITATOR_PORT, 0), 1);
	switch (result)
	{
	case RakNet::StartupResult::COULD_NOT_GENERATE_GUID:
		break;
	case RakNet::StartupResult::FAILED_TO_CREATE_NETWORK_THREAD:
		//printf("创建网络通讯线程失败！\n");
		return;
	case RakNet::StartupResult::INVALID_MAX_CONNECTIONS:
		//printf("不合法的最大网络连接数！\n");
		return;
	case RakNet::StartupResult::INVALID_SOCKET_DESCRIPTORS:
		break;
	case RakNet::StartupResult::PORT_CANNOT_BE_ZERO:
		//printf("端口不能为0！\n");
		return;
	case RakNet::StartupResult::RAKNET_ALREADY_STARTED:
		break;
	case RakNet::StartupResult::RAKNET_STARTED:
		//printf("监听成功于 %s\n\n", m_natPunchthroughFacilitator->GetMyBoundAddress().ToString(true));
		break;
	case RakNet::StartupResult::SOCKET_FAILED_TEST_SEND:
		break;
	case RakNet::StartupResult::SOCKET_FAILED_TO_BIND:
		break;
	case RakNet::StartupResult::SOCKET_FAMILY_NOT_SUPPORTED:
		break;
	case RakNet::StartupResult::SOCKET_PORT_ALREADY_IN_USE:
		return;
		break;
	case RakNet::StartupResult::STARTUP_OTHER_FAILURE:
		break;
	default:
		break;
	}
	m_natPunchthroughFacilitator->SetTimeoutTime(5000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
	m_natPunchthroughFacilitator->SetMaximumIncomingConnections(8096);
	RakNet::NatPunchthroughServer *natPunchthroughPlugin = new RakNet::NatPunchthroughServer;
	m_natPunchthroughFacilitator->AttachPlugin(natPunchthroughPlugin);

	RakNet::Packet *packet;
	while (true)
	{
		for (packet = m_natPunchthroughFacilitator->Receive(); packet; m_natPunchthroughFacilitator->DeallocatePacket(packet), packet = m_natPunchthroughFacilitator->Receive())
		{
			switch (packet->data[0])
			{
			case ID_NEW_INCOMING_CONNECTION:
			{
				// printf("一个语音客户端连入");
				break;
			}
			case ID_CONNECTION_LOST:
				removeClient(packet->guid);
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				removeClient(packet->guid);
				break;
			case ID_USER_PACKET_ENUM + 1:
			{
				RakNet::BitStream in(packet->data, packet->length, false);
				in.IgnoreBytes(sizeof(RakNet::MessageID));

				RakNet::RakString roomID;
				in.Read(roomID);
				std::string requestRoomID = roomID.C_String();

				bool isExists = false;
				for (unsigned i = 0; i < m_voiceRoomList.size(); ++i)
				{
					if (m_voiceRoomList.at(i).roomID == requestRoomID)
					{
						RakNet::BitStream stream;
						stream.Write((RakNet::MessageID)(ID_USER_PACKET_ENUM + 1));
						stream.Write((unsigned short)(m_voiceRoomList.at(i).clientsIDList.size()));

						for (unsigned short j = 0; j < m_voiceRoomList.at(i).clientsIDList.size(); ++j)
						{
							stream.Write(m_voiceRoomList.at(i).clientsIDList.at(j));
						}
						m_natPunchthroughFacilitator->Send(&stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->guid, false);

						m_voiceRoomList.at(i).clientsIDList.push_back(packet->guid);
						isExists = true;
					}
				}
				
				// 如果房间不存在，创建房间
				if (!isExists)
				{
					VoiceRoom voiceRoom;
					voiceRoom.roomID = requestRoomID;
					voiceRoom.clientsIDList.push_back(packet->guid);
					m_voiceRoomList.push_back(voiceRoom);
				}

				//const unsigned short clientSize = m_natPunchthroughFacilitator->NumberOfConnections();
				//if (clientSize == 1)
				//{
				//	break;
				//}

				//RakNet::BitStream stream;
				//stream.Write((RakNet::MessageID)(ID_USER_PACKET_ENUM + 1));

				//stream.Write((unsigned short)(clientSize-1));

				//for (unsigned short i = 0; i < clientSize; ++i)
				//{
				//	const RakNet::RakNetGUID guid = m_natPunchthroughFacilitator->GetGUIDFromIndex(i);
				//	if (guid != packet->guid)
				//	{
				//		stream.Write(guid);
				//	}
				//}

				//m_natPunchthroughFacilitator->Send(&stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->guid, false);
				break;
			}
			default:
				break;
			}
		}
	}
}

void RakNetServer::NATPunchthroghFacilitator::removeClient(const RakNet::RakNetGUID clientID)
{
	for (auto room = m_voiceRoomList.begin(); room != m_voiceRoomList.end(); ++room)
	{
		for (auto client = room->clientsIDList.begin(); client != room->clientsIDList.end(); ++client)
		{
			if (clientID == *client)
			{
				room->clientsIDList.erase(client);
				if (room->clientsIDList.size() == 0)
				{
					m_voiceRoomList.erase(room);
				}
				return;
			}
		}
	}
}
