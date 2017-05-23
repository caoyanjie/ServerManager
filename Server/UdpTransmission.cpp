#include "PubHeader.h"

#include "UdpTransmission.h"
#include "FileTransfer.h"
#include "NATPunchthroghFacilitator.h"

// Thread to write log
#include <thread>

// RakNet headers 
// {{{
#include "RakNet/RakNetTypes.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakAssert.h"
#include "RakNet/RakSleep.h" //}}}

// RapidJson headers 
// {{{
#include "RapidJson/stringbuffer.h"
#include "RapidJson/writer.h"
#include "RapidJson/reader.h" //}}}

#include <sstream>
#include <iostream>

RakNetServer::UdpTransmission::UdpTransmission() :
	// {{{
	SERVER_PORT(50000),
	SERVER_MONITOR_CLIENT_PORT(50001),
	MAX_CLIENT(30),
    CONNECT_PASSWORD("helloitisme")
{
    m_listeningThreadActive = true;
    m_monitorThreadActive = true;
    m_broadcastThradActive = true;

	// 记录日志子线程
	m_writeLog = RakNetServer::WriteLog::GetInstance();
	m_writeLog->pushLog("日志线程启动成功！\t\t%s\n\n", m_writeLog->getCurrentTime().c_str());

	// 处理数据子线程
	m_dataProcess = new RakNetServer::DataProcess();
	m_writeLog->pushLog("\n处理数据子线程开启成功！\t%s\n", m_writeLog->getCurrentTime().c_str());

	// 广播数据子线程
	// std::thread broadcastDataSubThread([this]() {this->broadcast(); });
	std::thread broadcastDataSubThread(&UdpTransmission::broadcast, this);
	broadcastDataSubThread.detach();
	m_writeLog->pushLog("\n转发数据子线程正在待命......\t%s\n", m_writeLog->getCurrentTime().c_str());
	
	// 获得 RakNet 实例
	m_server = RakNet::RakPeerInterface::GetInstance();
	m_writeLog->pushLog("\n获取 RakNet 实例成功！\t\t%s\n", m_writeLog->getCurrentTime().c_str());

	connect(m_writeLog, SIGNAL(sglPrintString(QString)), this, SIGNAL(sglPrintString(QString)));
    connect(m_dataProcess, SIGNAL(sglCreateNewRoom(QString, QString, QString)), this, SIGNAL(sglCreateNewRoom(QString, QString, QString)));
	connect(m_dataProcess, SIGNAL(sglJoinRoom(QString, QString, QString)), this, SIGNAL(sglJoinRoom(QString, QString, QString)));
	connect(m_dataProcess, SIGNAL(sglLeaveRoom(QString, QString, QString)), this, SIGNAL(sglLeaveRoom(QString, QString, QString)));
} //}}}

RakNetServer::UdpTransmission::~UdpTransmission()
{ //{{{
    m_writeLog->pushLog("\n关闭服务器\t\t\t%s\n", m_writeLog->getCurrentTime().c_str());
    m_writeLog->pushLog("\n析构 UdpTransmission......\n");

    delete m_dataProcess;
    m_dataProcess = nullptr;

    m_broadcastThradActive = false;
    m_listeningThreadActive = false;
    m_monitorThreadActive = false;

    m_monitorThreadLock.lock();
    m_serverClientViewport->Shutdown(200);
    RakNet::RakPeerInterface::DestroyInstance(m_serverClientViewport);
    m_monitorThreadLock.unlock();

    std::lock_guard<std::mutex> lock(m_serverLock);
    m_server->Shutdown(300);
    RakNet::RakPeerInterface::DestroyInstance(m_server);

    m_writeLog->pushLog("\n完成 UdpTransmission 析构\n");
    delete m_writeLog;
    m_writeLog = nullptr;
} //}}}

// 启动服务器
void RakNetServer::UdpTransmission::start()
{ //{{{
	// 监听
    std::thread msgThread(&UdpTransmission::listening, this);
    msgThread.detach();

    std::thread monitorThread(&UdpTransmission::listeningClientViewport, this);
    monitorThread.detach();

	RakNetServer::NATPunchthroghFacilitator::GetInstance()->start();
} //}}}

// 关闭服务器
void RakNetServer::UdpTransmission::close()
{
	m_dataProcess->close();
	m_monitorThreadActive = false;
	m_monitorThreadLock.lock();
	m_serverClientViewport->Shutdown(200);
	RakNet::RakPeerInterface::DestroyInstance(m_serverClientViewport);
	m_monitorThreadActive = false;
	m_monitorThreadLock.unlock();
}

//test
void RakNetServer::UdpTransmission::test()
{ //{{{
	rapidjson::StringBuffer json;
	rapidjson::Writer<rapidjson::StringBuffer> writer(json);
	writer.StartObject();
	writer.Key("action");
	writer.StartArray();
	writer.Double(3);
	writer.Double(4);
	writer.EndArray();
	writer.EndObject();

	std::string result = json.GetString();

	std::cout << result.substr(1, result.find_last_of("}")-1) << std::endl;
} //}}}

void RakNetServer::UdpTransmission::setClientViewportState(bool state, QString clientGuid, int level)
{
// 	RakNet::BitStream bitStream;
// 	bitStream.Write(SWITCH_CLIENTVIEWPORT);
// 	bitStream.Write(1);
// 	std::lock_guard<std::mutex> lock(serverLock);
// 	RakNet::RakNetGUID client = dataProcess->getClientGuidFromGuidString(clientGuid);
// 	server->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, client, false);

// 	RakNet::BitStream bitStream;
// 	bitStream.Write(SWITCH_CLIENTVIEWPORT);
// 	bitStream.Write(1);
// 	std::lock_guard<std::mutex> lock(serverLock);
// 	server->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, dataProcess->getClientGuidFromGuidString(clientGuid), false);

	int width, height = 0;
	switch (level)
	{
	case 0:
		width = 320;
		height = 200;
		break;
	case 1:
		width = 480;
		height = 300;
		break;
	case 2:
		width = 640;
		height = 400;
		break;
	default:
		return;
	}
	std::string stateStr = state ? "true" : "false";
	std::ostringstream msg;
	msg << "{\"state\":\"" << stateStr << "\",\"width\":" << width << ",\"height\":" << height << "}";
    sendData(m_dataProcess->getClientGuidFromGuidString(clientGuid.toStdString().c_str()), SWITCH_CLIENT_VIEWPORT, false, msg.str().c_str());
}

void RakNetServer::UdpTransmission::changeMonitorQuility(const char* clientGuid, int level)
{
	int width, height = 0;
	switch (level)
	{
	case 0:
		width = 320;
		height = 200;
		break;
	case 1:
		width = 480;
		height = 300;
		break;
	case 2:
		width = 640;
		height = 400;
		break;
	default:
		return;
	}
	std::ostringstream msg;
	msg << "{\"width\":" << width << ",\"height\":" << height << "}";
	sendData(m_dataProcess->getClientGuidFromGuidString(clientGuid), CHANGE_MONITOR_QUILITY, false, msg.str().c_str());
}

// 监听
void RakNetServer::UdpTransmission::listening()
{ //{{{
	// RakNet::SystemAddress clientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
	// 设置服务器启动参数
	{ //{{{
		std::lock_guard<std::mutex> lock(m_serverLock);

		m_server->SetIncomingPassword(CONNECT_PASSWORD, (int)strlen(CONNECT_PASSWORD));
		RakNet::SocketDescriptor socketDescriptor(SERVER_PORT, 0);
		bool conState = m_server->Startup(MAX_CLIENT, &socketDescriptor, 1) == RakNet::RAKNET_STARTED;		// ? 0 or 1
		if (!conState)
		{
			print("UDP 监听失败！是否运行多个实例？\n");
			RakAssert(conState);
			return;
		}

		m_server->SetMaximumIncomingConnections(MAX_CLIENT);
		m_writeLog->printAndWriteLog("服务器开启监听......\t\t%s\n", m_writeLog->getCurrentTime().c_str());

		m_server->SetOccasionalPing(true);

		m_writeLog->printAndWriteLog("打印服务器信息:\t\t%s\n", m_writeLog->getCurrentTime().c_str());

		// show my ip
		m_writeLog->printAndWriteLog("Server addresses:\t\t");
		QStringList server_ips;
		for (unsigned int i = 0; i < m_server->GetNumberOfAddresses(); i++)
		{
			RakNet::SystemAddress sa = m_server->GetInternalID(RakNet::UNASSIGNED_SYSTEM_ADDRESS, i);
			m_writeLog->printAndWriteLog("(%i). %s (LAN=%i)  ", i + 1, sa.ToString(false), sa.IsLANAddress());
			server_ips.append(sa.ToString(false));
		}
		m_writeLog->printAndWriteLog("\n");
		emit sglShowServerIp(server_ips);

		// show my guid
		// writeLog->printAndWriteLog("Server GUID:\t\t\t%s\n", server->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());

		m_writeLog->printAndWriteLog("等待消息进入......\t\t%s\n", m_writeLog->getCurrentTime().c_str());
	} //}}}

	RakNet::Packet *packet = nullptr;
	while (true)
	{
		// This sleep keeps RakNet responsive
		// RakSleep(30);
		// for (packet = server->Receive() ; packet; server->DeallocatePacket(packet), packet = server->Receive())
        if (!m_listeningThreadActive)
        {
            break;
        }
		
		{
			std::lock_guard<std::mutex> lock(m_serverLock);
			packet = m_server->Receive();
		}

		if (packet)
		{
			switch (packet->data[0])
			{
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
				m_writeLog->printAndWriteLog("Another client has disconnected\n");
				break;
			case ID_REMOTE_CONNECTION_LOST:
				m_writeLog->printAndWriteLog("Another client has lost the connection\n");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				m_writeLog->printAndWriteLog("Another client has connected\n");
				break;
			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				m_writeLog->printAndWriteLog("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
				break;
			case ID_CONNECTED_PING:
				m_writeLog->printAndWriteLog("\n来自一个没有连接用户的 Ping");
				break;
			case ID_UNCONNECTED_PING:
				m_writeLog->printAndWriteLog("\nPing 来自 IP: %s\n", packet->systemAddress.ToString(true));
				break;
			case ID_CONNECTION_ATTEMPT_FAILED:
				m_writeLog->printAndWriteLog("\n尝试连接失败！ IP: %s  GUID: %s\n", packet->systemAddress.ToString(true), packet->guid.ToString());
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				m_writeLog->printAndWriteLog("\n连接到远端服务器成功\n");
				break;
			case ID_NEW_INCOMING_CONNECTION:
				m_writeLog->printAndWriteLog("\n一个客户端连入 IP: %s  GUID: %s\t\t%s\n", packet->systemAddress.ToString(true), packet->guid.ToString(), m_writeLog->getCurrentTime().c_str());
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				m_writeLog->printAndWriteLog("\n客户端断开连接 IP: %s  GUID: %s", packet->systemAddress.ToString(true), packet->guid.ToString());
				leaaveRoom(packet->guid);
				break;
			case ID_CONNECTION_LOST:
				m_writeLog->printAndWriteLog("\n丢失了一个连接 IP: %s  GUID: %s", packet->systemAddress.ToString(true), packet->guid.ToString());
				leaaveRoom(packet->guid);
				break;
			case REQUEST_CREATEROOM:
			{
				std::string packetString = getPacketString(packet);
				std::string roomName = RakNetServer::ProcessJson::getJsonStringValue(packetString, "room_name");
				std::string roomPasswd = RakNetServer::ProcessJson::getJsonStringValue(packetString, "room_passwd");
				std::string username = RakNetServer::ProcessJson::getJsonStringValue(packetString, "username");
				std::string hasRole = RakNetServer::ProcessJson::getJsonStringValue(packetString, "has_role");
				std::string voiceServer = RakNetServer::ProcessJson::getJsonStringValue(packetString, "voice_server");

				m_writeLog->printAndWriteLog("\n一请求创建房间 IP: %s  GUID: %s (Client 1)\t%s\n", packet->systemAddress.ToString(true), packet->guid.ToString(), m_writeLog->getCurrentTime().c_str());
				//createRoom(packet->systemAddress.ToString(true), packet->guid);
				createRoom(packet->systemAddress.ToString(false), packet->guid, roomName.c_str(), roomPasswd.c_str(), username.c_str(), hasRole.c_str(), voiceServer.c_str());
				break;
			}
			case REQUEST_JOIN_ROOM:
			{
				std::string packetString = getPacketString(packet);
				std::string roomID = RakNetServer::ProcessJson::getJsonStringValue(packetString, "room_id");
				std::string roomPasswd = RakNetServer::ProcessJson::getJsonStringValue(packetString, "room_passwd");
				std::string username = RakNetServer::ProcessJson::getJsonStringValue(packetString, "username");
				
				m_writeLog->printAndWriteLog("\n一请求加入房间 ID: %s", roomID.c_str());
				//joinRoom(roomID.c_str(), packet->systemAddress.ToString(true), packet->guid);
				joinRoom(roomID.c_str(), roomPasswd.c_str(), packet->systemAddress.ToString(false), packet->guid, username.c_str());
				break;
			}
			case REQUEST_GET_ROOMS:
				sendData(packet->guid, STATE_ROOMS, m_dataProcess->getRooms());
				break;
			case REQUEST_GET_USER_INFO:
				sendData(packet->guid, STATE_USER_INFO, m_dataProcess->getUserInfo(packet->guid));
				break;
			case MSG_CLIENT_DATA:
				processData(packet);
				break;
			case CHANGE_USERNAME:
			{
				std::string name = getPacketString(packet);
				updateUsername(packet->guid, name.c_str());
				break;
			}
			default:
				m_writeLog->printAndWriteLog("\n收到未知消息头的消息：\n\n");
				break;
			}
			{
				std::lock_guard<std::mutex> lock(m_serverLock);
				m_server->DeallocatePacket(packet);
			}
		}
    }
} //}}}

void RakNetServer::UdpTransmission::listeningClientViewport()
{
	m_serverClientViewport = RakNet::RakPeerInterface::GetInstance();
	m_serverClientViewport->SetIncomingPassword(CONNECT_PASSWORD, (int)strlen(CONNECT_PASSWORD));
	RakNet::SocketDescriptor socketDescriptor(SERVER_MONITOR_CLIENT_PORT, 0);
	bool conState = m_serverClientViewport->Startup(MAX_CLIENT, &socketDescriptor, 1) == RakNet::RAKNET_STARTED;		// ? 0 or 1
	m_serverClientViewport->SetMaximumIncomingConnections(MAX_CLIENT);
	RakNet::Packet *packet = nullptr;
	while (true)
	{
		RakSleep(30);
		if (!m_monitorThreadActive)
		{
			break;
		}
		m_monitorThreadLock.lock();
		packet = m_serverClientViewport->Receive();
		if (packet)
		{
			switch (packet->data[0])
			{
			case ID_NEW_INCOMING_CONNECTION:
				m_writeLog->printAndWriteLog("\n监控客户端连接成功 IP: %s  GUID: %s\t\t%s\n", packet->systemAddress.ToString(true), packet->guid.ToString(), m_writeLog->getCurrentTime().c_str());
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				m_writeLog->printAndWriteLog("\n监控客户端断开连接 IP: %s  GUID: %s\n", packet->systemAddress.ToString(true), packet->guid.ToString());
				break;
			case ID_CONNECTION_LOST:
				m_writeLog->printAndWriteLog("\n监控客户端丢失了一个连接 IP: %s  GUID: %s\n", packet->systemAddress.ToString(true), packet->guid.ToString());
				break;
			case MSG_CLIENT_VIEWPORT:
				showClientViewport(packet);
				break;
			default:
				m_writeLog->printAndWriteLog("\n\n收到未知消息头的消息：\n\n");
				break;
			}
			m_serverClientViewport->DeallocatePacket(packet);
		}
		m_monitorThreadLock.unlock();
	}
}

// 获得流中字符串
std::string RakNetServer::UdpTransmission::getPacketString(RakNet::Packet * packet)
{ //{{{
	RakNet::BitStream stream(packet->data, packet->length, false);
	stream.IgnoreBytes(sizeof(RakNet::MessageID));

	RakNet::RakString result;
	stream.Read(result);
	return std::string(result.C_String());
} //}}}

// 创建房间
void RakNetServer::UdpTransmission::createRoom(const char* clientIPWithPort, const RakNet::RakNetGUID clientGuid, const char* roomName, const char* roomPasswd, const char* username, const char* hasRole, const char* voiceServer)
{ //{{{
	// 创建房间
	m_dataProcess->createRoom(clientIPWithPort, clientGuid, roomName, roomPasswd, username, hasRole, voiceServer);

	// 构造数据流
	const char* guidStr = clientGuid.ToString();
	std::ostringstream msg;
	msg << "{\"room_id\":\"" << guidStr << "\",\"client_id\":\"" << guidStr << "\"}";
	std::string aa = msg.str();
	sendData(clientGuid, STATE_CREATED_ROOM_SUCCESS, false, msg.str());

	// 记录日志
	m_writeLog->printAndWriteLog("房间已创建成功\n");
} //}}}

// 加入房间
void RakNetServer::UdpTransmission::joinRoom(const char* roomIDString, const char* roomPasswdString, const char* clientIPWithPort, const RakNet::RakNetGUID clientGuid, const char* username)
{ //{{{
	bool hasRole;
	std::string voiceServer;
	const int roomIndex = m_dataProcess->joinRoom(roomIDString, roomPasswdString, clientIPWithPort, clientGuid, username, hasRole, voiceServer);
	if (roomIndex > -1)
	{
		// 加入房间成功
		//const char* guidStr = clientGuid.ToString();
		std::string guidStr(clientGuid.ToString());

		rapidjson::StringBuffer json;
		rapidjson::Writer<rapidjson::StringBuffer> writer(json);
		writer.StartObject();
		writer.Key("room_id");
		writer.String(roomIDString);
		writer.Key("client_id");
		writer.String(guidStr.c_str());
		writer.Key("has_role");
		writer.Bool(hasRole);
		writer.Key("voice_server");
		writer.String(voiceServer.c_str());
		writer.EndObject();
		std::string joinedState = json.GetString();;
		sendData(clientGuid, STATE_JOINED_ROOM_SUCCESS, false, joinedState);

		std::map<std::string, std::string> otherClients = m_dataProcess->getOtherClientsGuidsNamesOfRoom(clientGuid);
		for (const auto &otherClient : otherClients)
		{
			std::ostringstream existsClient;
			existsClient << "{\"action\":" << ActionType::AddClient << ",\"clientGuid\":\"" << otherClient.first.c_str() << "\",\"clientName\":\"" << otherClient.second.c_str() << "\"}";
			sendData(clientGuid, MSG_CLIENT_DATA, false, existsClient.str());
		}

		std::ostringstream joinedClient;
		joinedClient << "{\"action\":" << ActionType::AddClient << ",\"clientGuid\":\"" << guidStr << "\",\"clientName\":\"" << username << "\"}";
		sendData(clientGuid, MSG_CLIENT_DATA, true, joinedClient.str());

		// 同步场景状态
		m_writeLog->pushLog("\n\nScene State:\n");
		std::vector<Room::SceneState> stateList = m_dataProcess->getSceneState(roomIndex);
		long start, end;
		start = clock();
		for (auto state = stateList.begin(); state != stateList.end(); ++state)
		{
			m_writeLog->pushLog("%s\n", state->jsnStr.c_str());
			const char* result = state->jsnStr.c_str();
			sendData(clientGuid, MSG_CLIENT_DATA, false, result);
		}
		end = clock();
		m_writeLog->printAndWriteLog("  %d 条同步数据发送完成！  用时 %ld ms\n", stateList.size(), end - start);
	}
	else if (roomIndex == -1)
	{
		// 加入房间失败
//		sendData(clientGuid, STATE_JOINED_ROOM_FAIL);
		sendData(clientGuid, STATE_JOINED_ROOM_FAIL, false, "加入房间失败");
	}
	else if (roomIndex == -2)
	{
		print("\n重复加入房间\n");
	}
	else if (roomIndex == -3)
	{
		sendData(clientGuid, STATE_JOINED_ROOM_FAIL, false, "房间口令错误");
	}
} //}}}

// 移除房间
void RakNetServer::UdpTransmission::leaaveRoom(const RakNet::RakNetGUID guid)
{ //{{{
	// 通知其他客户端不发送自己位置给监控
	if (m_dataProcess->isRoomHost(guid))
	{
		std::ostringstream msg;
		msg << "{\"action\":" << ActionType::clientMonitorState << ",\"clientMonitorState\":\"false\"}";
		sendData(guid, MSG_CLIENT_DATA, true, msg.str());
	}

	// 通知监控者移除一个客户端
	std::ostringstream msg;
	msg << "{\"action\":" << ActionType::RemoveClient << ",\"clientGuid\":\"" << guid.ToString() << "\"}";
	sendData(guid, MSG_CLIENT_DATA, true, msg.str());

	// 更新服务器端信息(要放在最后)
	m_dataProcess->removeClient(guid);
} //}}}

// 处理数据
void RakNetServer::UdpTransmission::processData(RakNet::Packet * packet)
{ //{{{
	std::string data = getPacketString(packet);
	int clientIndex = m_dataProcess->getClientIndex(packet->guid);
	if (data != "")
	{
		m_writeLog->pushLog("\n接收到 client(%d) 的数据: \t\t\t\t\t\t\t%s\n%s\n已转交 DataProcess 进行处理\n", clientIndex, m_writeLog->getCurrentTime().c_str(), data.c_str());
		m_dataProcess->saveData(packet->guid, packet->systemAddress.ToString(false), packet->systemAddress.ToString(false), data);
	}
	else
	{
		m_writeLog->pushLog("收到 client(%d) 空消息，已丢弃\t%s\n", clientIndex, m_writeLog->getCurrentTime().c_str());
	}
} //}}}

// 发送消息给指定客户端
void RakNetServer::UdpTransmission::sendData(RakNet::RakNetGUID guid, RakNet::MessageID msgType, bool broadcast, std::string msg)
{ //{{{
	RakNet::BitStream bitStream;
	bitStream.Write(msgType);
	if (msg != "")
	{
		bitStream.Write(msg.c_str());
	}

	std::lock_guard<std::mutex> lock(m_serverLock);
	if (broadcast)
	{
		std::vector<RakNet::RakNetGUID> otherClients = m_dataProcess->getOtherClientsOfRoom(guid);
		for (const auto & otherClient : otherClients)
		{
			m_server->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, otherClient, false);
		}
	}
	else
	{
		m_server->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, guid, false);
	}
} //}}}

// 发送消息给指定客户端
void RakNetServer::UdpTransmission::sendData(RakNet::RakNetGUID guid, RakNet::MessageID msgType, std::vector<std::string> msgs, bool broadcast)
{ //{{{
	RakNet::BitStream bitStream;
	bitStream.Write((RakNet::MessageID)msgType);

	unsigned int size = msgs.size();
	bitStream.Write(size);

	for (const auto &msg : msgs)
	{
		bitStream.Write(msg.c_str());
	}

	std::lock_guard<std::mutex> lock(m_serverLock);
	if (broadcast)
	{
		std::vector<RakNet::RakNetGUID> otherClients = m_dataProcess->getOtherClientsOfRoom(guid);
		for (const auto & otherClient : otherClients)
		{
			m_server->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, otherClient, false);
		}
	}
	else
	{
		m_server->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, guid, false);
	}
} //}}}

// 广播给房间内所有客户端
void RakNetServer::UdpTransmission::broadcast()
{ //{{{
    while (m_broadcastThradActive)
	{
        RakNetServer::PacketNeedSend packet;
        if (!m_dataProcess)
        {
            continue;
        }
        bool error = !m_dataProcess->getData(packet);
        if (error)
        {
            break;
        }

		std::ostringstream clients;
		for (unsigned i = 0; i < packet.clients.size(); ++i)
		{
			sendData(packet.clients.at(i), MSG_CLIENT_DATA, false, packet.jsonStr);
			clients << "client(" << m_dataProcess->getClientIndex(packet.clients.at(i)) << ");";
		}

		m_writeLog->pushLog("\n转发数据成功！\t\t\t\t\t\t\t\t\t%s\n对象: %s\n数据: %s\n\n", m_writeLog->getCurrentTime().c_str(), clients.str().c_str(), packet.jsonStr.c_str());
	}
} //}}}

// 更新用户名(用户自定义用户名)
void RakNetServer::UdpTransmission::updateUsername(const RakNet::RakNetGUID clientGuid, const char* newUsername)
{
	m_dataProcess->setUsername(clientGuid, newUsername);

	emit sglUpdateUsername(m_dataProcess->getRoomIDFromClientGuid(clientGuid).c_str(), clientGuid.ToString(), QString::fromLocal8Bit(newUsername));

	std::ostringstream msg;
	msg << "{\"action\":" << ActionType::RenameClient << ",\"clientGuid\":\"" << clientGuid.ToString() << "\",\"clientName\":\"" << newUsername << "\"}";
	sendData(clientGuid, MSG_CLIENT_DATA, true, msg.str());
}

void RakNetServer::UdpTransmission::showClientViewport(RakNet::Packet * packet)
{
	RakNet::BitStream bitStream;
	bitStream.Write((RakNet::MessageID)RECEIVED_IMAGE_COUNT);
	if (m_serverClientViewport)
	{
		m_serverClientViewport->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->guid, false);
	}

	//
	RakNet::BitStream stream(packet->data, packet->length, false);
	stream.IgnoreBytes(sizeof(RakNet::MessageID));

	RakNet::RakString roomID;
	RakNet::RakString clientID;
	unsigned long long size;
	unsigned char* imgData = nullptr;
	int imgWidth;
	int imgHeight;
	RakNet::RakString imgFormat;

	stream.Read(roomID);
	stream.Read(clientID);
	stream.Read(imgWidth);
	stream.Read(imgHeight);
	stream.Read(imgFormat);
	stream.Read(size);
	imgData = new unsigned char[size];
	for (unsigned long long i = 0; i < size; ++i)
	{
		stream.Read(imgData[i]);
	}
	//stream.Read(imgData);

    QImage image(imgData, imgWidth, imgHeight, QImage::Format::Format_RGB888);
    emit sglShowClientViewport(roomID.C_String(), clientID.C_String(), image, imgWidth, imgHeight, imgFormat.C_String());
}

void RakNetServer::UdpTransmission::print(const char* str)
{
	emit sglPrintString(QString::fromLocal8Bit(str));
}
