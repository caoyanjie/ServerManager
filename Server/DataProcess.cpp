#include "PubHeader.h"

#include "RapidJson/stringbuffer.h"
#include "RapidJson/writer.h"

#include "DataProcess.h"
#include "WriteLog.h"
#include "FileTransfer.h"
#include <thread>
#include <sstream>

RakNetServer::DataProcess::DataProcess() :
    m_isSubThreadAlive(true),
    m_thisObjDestory(false)
{ //{{{
	m_writeLog = RakNetServer::WriteLog::GetInstance();
	m_processJson = new ProcessJson();
	m_fileTransfer = new FileTransfer();

	std::thread processThread(&DataProcess::processData, this);
	processThread.detach();
} //}}}

RakNetServer::DataProcess::~DataProcess()
{ //{{{
    m_writeLog->pushLog("\n析构 DataProcess......\n");
    m_writeLog->pushLog("\n析构 ProcessJson......\n");
    delete m_processJson;
	m_processJson = nullptr;
    m_writeLog->pushLog("\n完成 ProcessJson 析构\n");

	delete m_fileTransfer;
	m_fileTransfer = nullptr;

	m_subThreadLock.lock();
	m_isSubThreadAlive = false;
	m_subThreadLock.unlock();

    m_thisObjDestory = true;
    m_broadcastThreadWaitCondition.notify_one();
    m_processDataThreadWaitCondition.notify_one();
    Sleep(100);
    m_writeLog->pushLog("\n完成 DataProcess 析构\n");
} //}}}

// 
void RakNetServer::DataProcess::saveData(RakNet::RakNetGUID clientId, std::string host, std::string hostWithPort, std::string jsonStr)
{ //{{{
	PacketInfo pInfo;
	pInfo.clientID = clientId;
	pInfo.host = host;
	pInfo.hostWithPort = hostWithPort;
	pInfo.jsonStr = jsonStr;

	std::lock_guard<std::mutex> lock(m_dataLock);
	m_dataNeedProcess.push(pInfo);
	m_processDataThreadWaitCondition.notify_one();
} //}}}

// 获取需要广播给房间中客户端的数据
bool RakNetServer::DataProcess::getData(PacketNeedSend &packet)
{ //{{{
    std::unique_lock<std::mutex> lock(m_broadcastLock);
    m_broadcastThreadWaitCondition.wait(lock, [this]{ return !m_dataNeedBroadcast.empty() || m_thisObjDestory; });
    if (m_thisObjDestory)
    {
        lock.unlock();
        return false;
    }
    packet = m_dataNeedBroadcast.front();
    m_dataNeedBroadcast.pop();
    lock.unlock();
    return true;
} //}}}

// 判断线程间共享队列中是否有待广播的数据
bool RakNetServer::DataProcess::hasDataToSend()
{ //{{{
	std::lock_guard<std::mutex> lock(m_broadcastLock);
	return m_dataNeedBroadcast.empty() ? false : true;
} //}}}

// 获取房间中最新场景数据，用于初始化后加入房间的的用户初始化状态
std::vector<RakNetServer::Room::SceneState> RakNetServer::DataProcess::getSceneState(const int roomIndex)
{ //{{{
	std::lock_guard<std::mutex> lock(m_stateLock);
	return m_roomList.at(roomIndex).sceneState;
} //}}}

// 处理来自客户端的数据
void RakNetServer::DataProcess::processData()
{ //{{{
	while (true)
	{
		// if kill this subThread
		m_subThreadLock.lock();
		if (!m_isSubThreadAlive)
		{
			m_subThreadLock.unlock();
			break;
		}
		m_subThreadLock.unlock();

		// get packet from queue
        PacketInfo pInfo;
        std::unique_lock<std::mutex> lock(m_dataLock);
        m_processDataThreadWaitCondition.wait(lock, [this]{ return !m_dataNeedProcess.empty() || m_thisObjDestory; });
        if (m_thisObjDestory)
        {
            lock.unlock();
            break;
        }
		pInfo = m_dataNeedProcess.front();
		m_dataNeedProcess.pop();
		lock.unlock();

		// get room
		int roomIndex = getRoomIndexFrmCGuid(pInfo.clientID);
		if (roomIndex == -1)
			continue;
		Room &room = m_roomList.at(roomIndex);
		
		DecJsnRst decJsnRst = m_processJson->decodeJson(pInfo.jsonStr);
		ProcessResult proResult;
		proResult.clientID = pInfo.clientID;
		proResult.jsonString = pInfo.jsonStr;
		proResult.decJsnRst = decJsnRst;

		bool noProcess = (decJsnRst.hasKey("author") && decJsnRst["author"].str_value == std::string("host") && !isRoomHost(proResult.clientID));
		if (noProcess)
		{
			continue;
		}

		// 分类处理消息
		switch (decJsnRst["action"].unsigned_value)
		{ //{{{
		case NodeOccupied:
			setOccupy(room, proResult);
			break;
//		case SendNewFile:
//			receiveFile(decJsnRst["filename"].str_value, decJsnRst["hash"].unsigned_value, pInfo.clientID, pInfo.host.c_str(), decJsnRst["port"].unsigned_value, pInfo.jsonStr, getOtherHostsOfRoom(room, pInfo.clientID));
//			break;
//		case CancelSendFile:
//			cancelTransferFile(std::string(pInfo.hostWithPort), FileTransfer::getLocalFilePathFromFileHash(decJsnRst["hash"].unsigned_value, FileTransfer::getFileExtend(decJsnRst["filename"].str_value)));
//			break;
//		case SendFolder:
//			receiveFolder(decJsnRst["folderPath"].str_value, pInfo.clientID, pInfo.host.c_str(), decJsnRst["port"].unsigned_value, pInfo.jsonStr, getOtherHostsOfRoom(room, pInfo.clientID));
//			break;
		case ClientViewMonitor:
			assert(false && "不存在的处理类型(RakNetServer::DataProcess::processData()");
			break;
		default:
			sendPacketToClients(pInfo.jsonStr, room, pInfo.clientID, decJsnRst["broadcast"].str_value);
		} //}}}
		
		// 案例
		if (decJsnRst.hasKey("adsorb") && decJsnRst["adsorb"].unsigned_value > 0)
		{ //{{{
			RakNetServer::PacketNeedSend packet;
			for (auto iter = m_roomList.at(roomIndex).clientList.begin(); iter != m_roomList.at(roomIndex).clientList.end(); ++iter)
			{
				if (iter->guid == proResult.clientID)
					continue;

				packet.clients.push_back(iter->guid);
				packet.jsonStr = pInfo.jsonStr;
			}
			std::lock_guard<std::mutex> lock(m_dataLock);
			m_dataNeedBroadcast.push(packet);
			m_broadcastThreadWaitCondition.notify_one();
		} //}}}

		// backup
		if (decJsnRst["broadcast"].str_value != std::string("no"))
		{	
			if (decJsnRst["broadcast"].str_value == "exitBroadcast")
			{
				ClientInfo *client = findClient(pInfo.clientID);
				if (client)
				{
					for (auto state = client->exitMessages.begin(); state != client->exitMessages.end(); ++state)
					{
						if (proResult.decJsnRst.isPrimaryEqual(state->jsnRst))
						{
							state->jsnStr = proResult.jsonString;
							state->jsnRst = proResult.decJsnRst;
							return;
						}
					}
					client->appendSceneState(proResult.jsonString, proResult.decJsnRst);
				}
			}
			else
			{
				mergeSceneStateData(room, proResult);
			}
		}
	}
} //}}}

// 创建房间
void RakNetServer::DataProcess::createRoom(const char* clientIPWithPort, const RakNet::RakNetGUID clientGuid, const char* roomName, const char* roomPasswd, const char* username, const char* hasRole, const char* voiceServer)
{ //{{{
	for (const auto &room : m_roomList)
	{
		if (clientGuid == room.id)
		{
			return;
		}
	}

	ClientInfo client;
    client.ipWithPort = std::string(clientIPWithPort);
	client.guid = clientGuid;
	client.username = std::string(username);
	client.index = 0;

	Room room;
	room.id = clientGuid;
	room.name = std::string(roomName);
	room.passwd = std::string(roomPasswd);
	room.roomHost = client;
	room.hasRole = std::string("true") == hasRole;
	room.voiceServer = std::string(voiceServer);
	room.clientList.push_back(client);
	m_roomList.push_back(room);

    emit sglCreateNewRoom(clientGuid.ToString(), QString::fromLocal8Bit(username), QString::fromLocal8Bit(roomPasswd));
} //}}}

// 加入房间
int RakNetServer::DataProcess::joinRoom(const char* roomIDString, const char* roomPasswdString, const char* clientIPWithPort, const RakNet::RakNetGUID clientGuid, const char* username, bool &hasRole, std::string &voiceServer)
{ //{{{
	for (unsigned i = 0; i < m_roomList.size(); ++i)
	{
		if (std::string(m_roomList.at(i).id.ToString()) == std::string(roomIDString))
		{
			// 判断是否重复加入
			for (const auto &c : m_roomList.at(i).clientList)
			{
				if (clientGuid == c.guid)
					return -2;
			}

			// 检查房间口令
			if (m_roomList.at(i).passwd != std::string(roomPasswdString))
			{
				return -3;
			}

			// 创建客户端
			ClientInfo client;
			client.ipWithPort = std::string(clientIPWithPort);
			client.guid = clientGuid;
			client.username = std::string(username);
			client.index = m_roomList.at(i).clientList.size();

			// 添加到房间
			m_roomList.at(i).clientList.push_back(client);
			m_writeLog->printAndWriteLog(" (Client %d)\t%s\n已进入房间....\t房间中有 %d 个客户端", client.index + 1, m_writeLog->getCurrentTime().c_str(), m_roomList.at(i).clientList.size());

			emit sglJoinRoom(roomIDString, clientGuid.ToString(), username);
			hasRole = m_roomList.at(i).hasRole;
			voiceServer = m_roomList.at(i).voiceServer;
			return i;
		}
	}
	m_writeLog->printAndWriteLog("不存在该ID房间！\n");
	return -1;
} //}}}

// 把用户从房间中移除
void RakNetServer::DataProcess::removeClient(const RakNet::RakNetGUID guid)
{ //{{{
	for (unsigned i = 0; i < m_roomList.size(); ++i)
	{
		for (unsigned j = 0; j < m_roomList.at(i).clientList.size(); ++j)
		{
			if (std::string(guid.ToString()) == std::string(m_roomList.at(i).clientList.at(j).guid.ToString()))
			{
				// 移除
				// fileTransfer->cancelTransfer(roomList.at(i).clientList.at(j).ipWithPort);
				// 释放选中的节点
				for (auto pickupedNode = m_roomList.at(i).clientList.at(j).pickupList.begin(); pickupedNode != m_roomList.at(i).clientList.at(j).pickupList.end(); ++pickupedNode)
				{
					std::ostringstream jsonStr;
					jsonStr << "{\"action\":" << NodeOccupied << ",\"ndID\":" << *pickupedNode << ",\"ndOcup\":false}";
					sendPacketToClients(std::string(jsonStr.str()), m_roomList.at(i), m_roomList.at(i).clientList.at(j).guid, "everyone");
					DecJsnRst decJsnRst = m_processJson->decodeJson(std::string(jsonStr.str()));
					ProcessResult proResult;
					proResult.clientID = guid;
					proResult.jsonString = std::string(jsonStr.str());
					proResult.decJsnRst = decJsnRst;
					mergeSceneStateData(m_roomList.at(i), proResult);
				}

				// 发送自定义清理消息
				for (const auto &state : m_roomList.at(i).clientList.at(j).exitMessages)
				{
					sendPacketToClients(state.jsnStr, m_roomList.at(i), m_roomList.at(i).clientList.at(j).guid, "exceptme");
				}
				
				// 更新服务器界面
				emit sglLeaveRoom(m_roomList.at(i).id.ToString(), guid.ToString(), m_roomList.at(i).clientList.at(j).username.c_str());
				m_roomList.at(i).clientList.erase(m_roomList.at(i).clientList.begin() + j);
				m_writeLog->printAndWriteLog(" (Client %d)\t%s\n已将其移除房间\n", j + 1, m_writeLog->getCurrentTime().c_str());

				// 房间是否为空
				if (m_roomList.at(i).clientList.empty())
				{
					m_roomList.erase(m_roomList.begin() + i);
					m_writeLog->printAndWriteLog("\n该房间已空，已解散！\n----------------------------------------------------------------------------------------------------------\n");
				}
				else
				{
					m_writeLog->printAndWriteLog("\n房间中还有 %d 个客户端！\n", m_roomList.at(i).clientList.size());
				}
				return;
			}
		}
	}
	m_writeLog->printAndWriteLog("\t%s\n不在任一房间中\n", m_writeLog->getCurrentTime().c_str());
} //}}}

// 判断客户端是否是房间创建者
bool RakNetServer::DataProcess::isRoomHost(const RakNet::RakNetGUID clientGuid)
{ //{{{
	for (auto iter = m_roomList.begin(); iter != m_roomList.end(); ++iter)
	{
		if (clientGuid == iter->id)
		{
			return true;
		}
	}
	return false;
} //}}}

std::string RakNetServer::DataProcess::getRoomIDFromClientGuid(RakNet::RakNetGUID clientGuid)
{
	std::string roomID;
	for (auto const &room : m_roomList)
	{
		for (auto const &client : room.clientList)
		{
			if (client.guid == clientGuid)
			{
				roomID = std::string(room.id.ToString());
			}
		}
	}
	return roomID;
}

std::vector<std::string> RakNetServer::DataProcess::getRooms()
{ //{{{
	std::vector<std::string> rooms;
	for (auto room : m_roomList)
	{
		rapidjson::StringBuffer json;
		rapidjson::Writer<rapidjson::StringBuffer> writer(json);
		writer.StartObject();
		writer.Key("room_id");
		writer.String(room.id.ToString());
		writer.Key("room_name");
		writer.String(room.name.c_str());
		writer.EndObject();
		std::string item = json.GetString();

		rooms.push_back(item);
	}
	return rooms;
} //}}}

int RakNetServer::DataProcess::getClientIndex(const RakNet::RakNetGUID clientGuid)
{ // {{{
	for (unsigned i = 0; i < m_roomList.size(); ++i)
	{
		for (unsigned j = 0; j < m_roomList.at(i).clientList.size(); ++j)
		{
			if (clientGuid == m_roomList.at(i).clientList.at(j).guid)
			{
				return m_roomList.at(i).clientList.at(j).index;
			}
		}
	}
	return -1;
} //}}}

RakNet::RakNetGUID RakNetServer::DataProcess::getClientGuidFromGuidString(const char* clientGuid)
{
	for (const auto &room : m_roomList)
	{
		for (const auto &client : room.clientList)
		{
			if (std::string(client.guid.ToString()) == std::string(clientGuid))
			{
				return client.guid;
			}
		}
	}
}

std::vector<std::string> RakNetServer::DataProcess::getUserInfo(const RakNet::RakNetGUID clientGuid)
{
	std::vector<std::string> userInfo;

	Room *t_room = nullptr;
	ClientInfo *t_client = nullptr;
	for (auto &room : m_roomList)
	{
		for (auto &client : room.clientList)
		{
			if (client.guid == clientGuid)
			{
				t_room = &room;
				t_client = &client;
				break;
			}
		}
	}
	if ( (!t_room) || (!t_client) )
	{
		return userInfo;
	}
	
	std::ostringstream line;

	line << "房间ID:" << t_room->id.ToString();
	userInfo.push_back(std::string(line.str().c_str()));
	line.str("");

	line << "房间口令:" << t_room->passwd;
	userInfo.push_back(std::string(line.str().c_str()));
	line.str("");

	line << "房间人数:" << t_room->clientList.size() << " 人";
	userInfo.push_back(std::string(line.str().c_str()));
	line.str("");

	line << "我的ID:" << clientGuid.ToString();
	userInfo.push_back(std::string(line.str().c_str()));
	line.str("");

	line << "我的昵称:" << t_client->username;
	userInfo.push_back(std::string(line.str().c_str()));
	line.str("");

	line << "是否是房间主:" << (t_room->id == clientGuid ? "是" : "否");
	userInfo.push_back(std::string(line.str().c_str()));
	line.str("");

	return userInfo;
}

// 获取房间中除了自己的客户端 GUID 列表
std::vector<RakNet::RakNetGUID> RakNetServer::DataProcess::getOtherClientsOfRoom(const RakNet::RakNetGUID clientGuid)
{
	std::vector<RakNet::RakNetGUID> otherGuidList;
	bool thisRoom = false;
	for (const auto room : m_roomList)
	{
		for (const auto client : room.clientList)
		{
			if (client.guid == clientGuid)
			{
				thisRoom = true;
			}
			else
			{
				otherGuidList.push_back(client.guid);
			}
		}

		if (thisRoom)
		{
			return otherGuidList;
		}
		else
		{
			otherGuidList.clear();
		}
	}
	return otherGuidList;
}

std::map<std::string, std::string> RakNetServer::DataProcess::getOtherClientsGuidsNamesOfRoom(const RakNet::RakNetGUID clientGuid)
{
	std::map<std::string, std::string> otherGuidsNames;
	bool thisRoom = false;
	for (const auto room : m_roomList)
	{
		for (const auto client : room.clientList)
		{
			if (client.guid == clientGuid)
			{
				thisRoom = true;
			}
			else
			{
				otherGuidsNames.insert(std::pair<std::string, std::string>(std::string(client.guid.ToString()), client.username));
			}
		}

		if (thisRoom)
		{
			return otherGuidsNames;
		}
		else
		{
			otherGuidsNames.clear();
		}
	}
	return otherGuidsNames;
}

void RakNetServer::DataProcess::setUsername(const RakNet::RakNetGUID clientGuid, const char* name)
{
	ClientInfo *t_client = nullptr;
	for (auto &room : m_roomList)
	{
		for (auto &client : room.clientList)
		{
			if (client.guid == clientGuid)
			{
				client.username = std::string(name);
				return;
			}
		}
	}
}

// 记录房间中所有用户的操作变化量(有必要时合并数据)
void RakNetServer::DataProcess::mergeSceneStateData(Room &room, ProcessResult proResult)
{ //{{{
	std::lock_guard<std::mutex> lock(m_stateLock);
	for (auto state = room.sceneState.begin(); state != room.sceneState.end(); ++state)
	{
		if (proResult.decJsnRst.isPrimaryEqual(state->jsnRst))
		{
			state->jsnStr = proResult.jsonString;
			state->jsnRst = proResult.decJsnRst;
			return;
		}
	}
	room.appendSceneState(proResult.jsonString, proResult.decJsnRst);
} //}}}

// 生成广播数据包
RakNetServer::PacketNeedSend RakNetServer::DataProcess::makeBroadcastPacket(std::string jsonStr, const Room &room, const RakNet::RakNetGUID clientID, BroadcastType broadcastType)
{ //{{{
	RakNetServer::PacketNeedSend packet;

	switch (broadcastType)
	{
	case BroadcastType::ExceptMe:
		for (const auto &client : room.clientList)
		{
			if (client.guid == clientID)
				continue;

			packet.clients.push_back(client.guid);
		}
		break;
	case BroadcastType::OnlyMe:
		packet.clients.push_back(clientID);
		break;
	case BroadcastType::Everyone:
		for (const auto &client : room.clientList)
			packet.clients.push_back(client.guid);
		break;
	case BroadcastType::RoomHost:
		packet.clients.push_back(room.id);
		break;
	default:
		assert(false);
	}

	packet.jsonStr = jsonStr;
	return packet;
} //}}}

// 给客户端广播消息
void RakNetServer::DataProcess::sendPacketToClients(std::string jsonStr, const Room &room, const RakNet::RakNetGUID clientID, std::string broadcastType)
{ //{{{
	RakNetServer::PacketNeedSend packet;
	if (broadcastType == "no" || broadcastType == "exitEveryone" || broadcastType == "joinedOnce")
		return;

	if (broadcastType == "onlyme")
		packet = makeBroadcastPacket(jsonStr, room, clientID, BroadcastType::OnlyMe);
	else if (broadcastType == "exceptme")
		packet = makeBroadcastPacket(jsonStr, room, clientID, BroadcastType::ExceptMe);
	else if (broadcastType == "everyone")
		packet = makeBroadcastPacket(jsonStr, room, clientID, BroadcastType::Everyone);
	else if (broadcastType == "roomHost")
		packet = makeBroadcastPacket(jsonStr, room, clientID, BroadcastType::RoomHost);

	std::lock_guard<std::mutex> lock(m_broadcastLock);
	m_dataNeedBroadcast.push(packet);
	m_broadcastThreadWaitCondition.notify_one();
} //}}}

// 给单独客户端返回消息
void RakNetServer::DataProcess::sendPacketToClient(std::string jsonStr, const RakNet::RakNetGUID &clientGuid)
{ //{{{
	RakNetServer::PacketNeedSend packet;
	packet.clients.push_back(clientGuid);
	packet.jsonStr = jsonStr;
	std::lock_guard<std::mutex> lock(m_broadcastLock);
	m_dataNeedBroadcast.push(packet);
	m_broadcastThreadWaitCondition.notify_one();
} //}}}

// 抢占冲突处理
void RakNetServer::DataProcess::setOccupy(Room &room, ProcessResult result)
{ //{{{
	std::vector<ClientInfo>::iterator Client;
	if (result.decJsnRst["ndOcup"].bool_value)		// 请求选中物体
	{ //{{{
		// 判断是否已被其他人选中
		for (auto client = room.clientList.begin(); client != room.clientList.end(); ++client)
		{
			if (client->guid == result.clientID)
			{ //{{{
				Client = client;

				// 已经是所有者，什么也不做
				if (client->pickupList.find(result.decJsnRst["ndID"].unsigned_value) != client->pickupList.end())
					return;
			} //}}}
			else if (client->pickupList.find(result.decJsnRst["ndID"].unsigned_value) != client->pickupList.end())
			{ //{{{
				// 已被其他人选中
				std::string jsonStr = m_processJson->enecodeJson("%actn%ndID%ocup", NodeOccupied, result.decJsnRst["ndID"].unsigned_value, true);
				sendPacketToClient(jsonStr, result.clientID);
				return;
			} //}}}
		}

		// 物体无选中状态
		Client->pickupList.insert(result.decJsnRst["ndID"].unsigned_value);

		std::string jsonStrF = m_processJson->enecodeJson("%actn%ndID%ocup", NodeOccupied, result.decJsnRst["ndID"].unsigned_value, false);
		sendPacketToClient(jsonStrF, result.clientID);
		
		std::string jsonStrT = m_processJson->enecodeJson("%actn%ndID%ocup", NodeOccupied, result.decJsnRst["ndID"].unsigned_value, true);
		sendPacketToClients(jsonStrT, room, result.clientID, "exceptme");
	} //}}}
	else											// 释放物体
	{ //{{{
		for (auto client = room.clientList.begin(); client != room.clientList.end(); ++client)
		{
			if (client->guid == result.clientID)
			{
				client->pickupList.erase(result.decJsnRst["ndID"].unsigned_value);
				std::string jsonStr = m_processJson->enecodeJson("%actn%ndID%ocup", NodeOccupied, result.decJsnRst["ndID"].unsigned_value, false);
				sendPacketToClients(jsonStr, room, result.clientID, "everyone");
				return;
			}
		}
	} //}}}
} //}}}

// 处理 track 动画数据包
void RakNetServer::DataProcess::processTrackPacket(Room &room, ClientInfo &client, const ProcessResult &proResult)
{ //{{{
	m_writeLog->pushLog("\n处理 track 动画数据包\t\t\t\t\t\t\t\t%s\n", m_writeLog->getCurrentTime().c_str());
	RakNetServer::PacketNeedSend packet;

	std::string trackName = proResult.decJsnRst["trackerName"].str_value;
	int trackState = proResult.decJsnRst["trackPlayerState"].unsigned_value;

	switch (trackState)
	{
		case TP_play:
		case TP_pause:
			// {{{
			if (trackState == TP_play)
			{
				m_writeLog->pushLog("接收到一个 track 动画播放包\n");
			}
			else
			{
				m_writeLog->pushLog("接收到一个 track 动画暂停包\n");
			}
			for (auto c = room.clientList.begin(); c != room.clientList.end(); ++c)
			{
				if (c->guid == client.guid)
				{
					continue;
				}
	
				for (auto state = client.trackerNameAndStateList.begin(); state != client.trackerNameAndStateList.end(); ++state)
				{
					if ((state->first == trackName) && (state->second == TP_play || state->second == TP_pause))
					{
						m_writeLog->pushLog("检测到房间中有其他用户正在播放同一动画，已将此动画延迟播放\n");

						packet.clients.push_back(client.guid);
						//packet.jsonStr = processJson->enecodeJson("%actn%trackerName%trackPlayerState", proResult.jsonStruct.action, trackName.c_str(), TP_delayPlay);
						//std::lock_guard<std::mutex> lock(dataLock);
						//dataNeedBroadcast.push(packet);
						return;
					}
				}
			}

			saveClientTrackState(client, trackName, trackState);
			break; // }}}
		case TP_remove:
			m_writeLog->pushLog("接收到一个 track 动画移除包\n");
			break;
		case TP_create:
			m_writeLog->pushLog("接收到一个 track 动画创建包\n");
		case TP_end:
			m_writeLog->pushLog("接收到一个 track 动画结束包\n");
		case TP_reset:
			m_writeLog->pushLog("接收到一个 track 动画重置包\n");
			saveClientTrackState(client, trackName, trackState);
			break;
		default:
			m_writeLog->pushLog("接收到一个 track 动画未知类型包(%d)，已断言处理\n", trackState);
			assert(false && "接收到一个 track 动画未知类型包");
	}

	for (auto c = room.clientList.begin(); c != room.clientList.end(); ++c)
	{
		packet.clients.push_back(c->guid);
		packet.jsonStr = proResult.jsonString;
	}
	std::lock_guard<std::mutex> lock(m_dataLock);
	m_dataNeedBroadcast.push(packet);
	return;
} //}}}

// 如果客户端track动画已存在，就替换状态值，没有则添加
void RakNetServer::DataProcess::saveClientTrackState(ClientInfo &client, std::string trackName, int stateValue)
{ //{{{
	for (auto state = client.trackerNameAndStateList.begin(); state != client.trackerNameAndStateList.end(); ++state)
	{
		if (state->first == trackName)
		{
			state->second = stateValue;
			return;
		}
	}
	client.trackerNameAndStateList.push_back(std::pair<std::string, int>(trackName, stateValue));

} //}}}

// 根据客户端 GUID 获取所在房间序列
int RakNetServer::DataProcess::getRoomIndexFrmCGuid(const RakNet::RakNetGUID clientGuid)
{ //{{{
	for (unsigned i = 0; i < m_roomList.size(); ++i)
	{
		for (unsigned j = 0; j < m_roomList.at(i).clientList.size(); ++j)
		{
			if (clientGuid == m_roomList.at(i).clientList.at(j).guid)
			{
				return i;
			}
		}
	}
	return -1;
} //}}}

std::vector<std::string> RakNetServer::DataProcess::getOtherHostsOfRoom(const Room& room, const RakNet::RakNetGUID guid)
{ //{{{
	std::vector<std::string> hostList;
	for (auto client = room.clientList.begin(); client != room.clientList.end(); ++client)
	{
		if (client->guid == guid)
			continue;

		hostList.push_back(client->ipWithPort);
	}
	return hostList;
} //}}}

// 查找客户端
RakNetServer::ClientInfo* RakNetServer::DataProcess::findClient(const RakNet::RakNetGUID clientGuid)
{
	for (auto &room : m_roomList)
	{
		for (auto &item : room.clientList)
		{
			if (item.guid == clientGuid)
			{
				return &item;
			}
		}
	}
	return nullptr;
}

// 发送文件
// void RakNetServer::DataProcess::sendFile(std::string filePath, std::string jsonStr, RakNet::RakNetGUID receiverGuid, std::string receiverHost)
// { //{{{
// 	// notifyRemoteToReceiveFile(filePath, jsonStr, receiverGuid, receiverHost);
// 	m_fileTransfer->pushSenderQueue(filePath, false, receiverHost, receiverGuid, jsonStr, false);
// } //}}}

// 接收文件
//void RakNetServer::DataProcess::receiveFile(std::string sourceFilePath, unsigned int sourceFileHash, const RakNet::RakNetGUID clientGuid, const char* host, unsigned short remotePort, std::string jsonStr, std::vector<std::string> clientHostListOfRoom)
//{ //{{{
// 	m_writeLog->pushLog("准备转发给:\t");
// 	for (auto i = clientHostListOfRoom.begin(); i != clientHostListOfRoom.end(); ++i)
// 		m_writeLog->pushLog("%s\t", i->c_str());
// 	m_writeLog->pushLog("\n");
// 
// 	std::string savedPath = FileTransfer::getLocalFilePathFromFileHash(sourceFileHash, FileTransfer::getFileExtend(sourceFilePath));
// 	m_fileTransfer->forwardFileOrFolder(savedPath, std::string(host), remotePort, clientGuid, jsonStr);
//} //}}}

// 对方拒绝接收文件，取消发送
//void RakNetServer::DataProcess::cancelTransferFile(std::string hostWithPort, std::string filePath)
//{ //{{{
//	m_fileTransfer->cancelTransfer(hostWithPort, filePath);
//} //}}}

// 接收文件夹
//void RakNetServer::DataProcess::receiveFolder(std::string remotePath, const RakNet::RakNetGUID clientGuid, const char* host, unsigned short remotePort, std::string jsonStr, std::vector<std::string> clientHostListOfRoom)
//{ //{{{
// 	m_writeLog->pushLog("准备转发给:\t");
// 	for (auto i = clientHostListOfRoom.begin(); i != clientHostListOfRoom.end(); ++i)
// 		m_writeLog->pushLog("%s\t", i->c_str());
// 	m_writeLog->pushLog("\n");
// 
// 	std::string savedPath = FileTransfer::getLocalFilePathFromRemotePath(remotePath, true);
// 	m_fileTransfer->forwardFileOrFolder(savedPath, std::string(host), remotePort, clientGuid, jsonStr, true);
//} //}}}

//void RakNetServer::DataProcess::notifyRemoteToReceiveFile(std::string filePath, std::string jsonStr, RakNet::RakNetGUID receiverGuid, std::string receiverHost)
//{ //{{{
//	unsigned short serverTcpPort = fileTransfer->getTcpPort();
//	std::ostringstream portStr;
//	portStr << serverTcpPort;
//	std::string newJsonStr = ProcessJson::alterJsonValue(jsonStr, "port", std::string(portStr.str()));
//
//	std::vector<RakNet::RakNetGUID> clients;
//	clients.push_back(receiverGuid);
//
//	PacketNeedSend packet;
//	packet.clients = clients;
//	packet.jsonStr = newJsonStr;
//
//	std::vector<std::string> clientHostListOfRoom;
//	clientHostListOfRoom.push_back(receiverHost);
//
//	broadcastLock.lock();
//	dataNeedBroadcast.push(packet);
//	broadcastThreadWaitCondition.notify_one();
//	broadcastLock.unlock();
//
//	fileTransfer->sendFileOrFolder(filePath, clientHostListOfRoom);
//} //}}}

//void RakNetServer::DataProcess::notifyRemoteToReceiveFile()
//{ //{{{
// 	while (true)
// 	{
// 		// if kill this subThread
// 		m_subThreadLock.lock();
// 		if (!m_isSubThreadAlive)
// 		{
// 			m_subThreadLock.unlock();
// 			break;
// 		}
// 		m_subThreadLock.unlock();
// 
// 		FileTransfer::NotifyRemoteReceiveInfo info = m_fileTransfer->getNotifyRemoteReceiveInfo();
// 		
// 		Room &room = m_roomList.at(getRoomIndexFrmCGuid(info.clientGuid));
// 		RakNetServer::PacketNeedSend packet;
// 		std::vector<std::string> receivers;
// 		if (info.isBroadcast)
// 		{
// 			packet = makeBroadcastPacket(info.jsonStr, m_roomList.at(getRoomIndexFrmCGuid(info.clientGuid)), info.clientGuid, BroadcastType::ExceptMe);
// 			receivers = getOtherHostsOfRoom(room, info.clientGuid);
// 		}
// 		else
// 		{
// 			packet = makeBroadcastPacket(info.jsonStr, m_roomList.at(getRoomIndexFrmCGuid(info.clientGuid)), info.clientGuid, BroadcastType::OnlyMe);
// 			receivers.push_back(info.clientIp);
// 		}
// 
// 		m_broadcastLock.lock();
// 		m_dataNeedBroadcast.push(packet);
// 		m_broadcastThreadWaitCondition.notify_one();
// 		m_broadcastLock.unlock();
// 
// 		if (receivers.size() > 0)
// 			m_fileTransfer->sendFileOrFolder(info.fullPath, receivers, info.isFolder);
// 	}
//} //}}}

void RakNetServer::DataProcess::close()
{
	delete m_fileTransfer;
	m_fileTransfer = nullptr;
}
