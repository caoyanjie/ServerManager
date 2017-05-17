#pragma once

//#include "MathLib.h"
#include "DefineTypes.h"
#include "ProcessJson.h"
#include "WriteLog.h"

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

#include <QObject>

namespace RakNetServer {
	class FileTransfer;

	class DataProcess : public QObject
	{
		Q_OBJECT

	public:
		//{{{
		DataProcess();
		~DataProcess();
		void close();

		void createRoom(const char* clientIPWithPort, const RakNet::RakNetGUID clientGuid, const char* roomName, const char* roomPasswd, const char* username, const char* hasRole, const char* voiceServer);
		int joinRoom(const char* roomIDString, const char* roomPasswdString, const char* clientIPWithPort, const RakNet::RakNetGUID clientGuid, const char* username, bool &hasRole, std::string &voiceServer);
		void removeClient(const RakNet::RakNetGUID guid);

		bool isRoomHost(const RakNet::RakNetGUID clientGuid);
		std::string getRoomIDFromClientGuid(RakNet::RakNetGUID clientGuid);
		std::vector<std::string> getRooms();
		int getClientIndex(const RakNet::RakNetGUID clientGuid);
		RakNet::RakNetGUID getClientGuidFromGuidString(const char* clientGuid);
		std::vector<std::string> getUserInfo(const RakNet::RakNetGUID clientGuid);
		std::vector<RakNet::RakNetGUID> getOtherClientsOfRoom(const RakNet::RakNetGUID clientGuid);
		std::map<std::string, std::string> getOtherClientsGuidsNamesOfRoom(const RakNet::RakNetGUID clientGuid);

		void setUsername(const RakNet::RakNetGUID clientGuid, const char* name);

		void saveData(RakNet::RakNetGUID, std::string host, std::string hostWithPort, std::string data);
        bool getData(RakNetServer::PacketNeedSend &packet);
		bool hasDataToSend();

//		void sendFile(std::string filePath, std::string jsonStr, RakNet::RakNetGUID receiverGuid, std::string receiverHost);

		std::vector<Room::SceneState> getSceneState(const int roomIndex);
		//}}}

	private:
		//{{{
		struct ProcessResult {
			//{{{
			RakNet::RakNetGUID		clientID;
			DecJsnRst				decJsnRst;
			std::string				jsonString;
		}; //}}}

		void processData();
		void mergeSceneStateData(Room &room, ProcessResult proResult);
		PacketNeedSend makeBroadcastPacket(std::string jsonStr, const Room &room, const RakNet::RakNetGUID clientID, BroadcastType broadcastType);
		void sendPacketToClients(std::string jsonStr, const Room &room, const RakNet::RakNetGUID clientID, std::string broadcastType);
		void sendPacketToClient(std::string jsonStr, const RakNet::RakNetGUID &client);
		void setOccupy(Room &room, ProcessResult result);

		void processTrackPacket(Room &room, ClientInfo &client, const ProcessResult &trackState);
		void saveClientTrackState(ClientInfo &client, std::string rackName, int state);
		
		int getRoomIndexFrmCGuid(const RakNet::RakNetGUID clientGuid);
		std::vector<std::string> getOtherHostsOfRoom(const Room& room, const RakNet::RakNetGUID guid);
		ClientInfo* findClient(const RakNet::RakNetGUID clientGuid);		// 查找客户端

		// file transfer
//		void receiveFile(std::string sourceFilePath, unsigned int sourceFileHash, const RakNet::RakNetGUID clientGuid, const char* host, unsigned short remotePort, std::string jsonStr, std::vector<std::string> clientHostListOfRoom);
//		void receiveFolder(std::string remotePath, const RakNet::RakNetGUID clientGuid, const char* host, unsigned short remotePort, std::string jsonStr, std::vector<std::string> clientHostListOfRoom);
//		void cancelTransferFile(std::string hostWithPort, std::string filePath = "");
		//void notifyRemoteToReceiveFile(std::string filePath, std::string jsonStr, RakNet::RakNetGUID receiverGuid, std::string receiverHost);
		void notifyRemoteToReceiveFile();

		std::queue<PacketInfo>			m_dataNeedProcess;
		std::mutex						m_dataLock;
		std::condition_variable			m_processDataThreadWaitCondition;

		std::queue<PacketNeedSend>		m_dataNeedBroadcast;
		std::mutex						m_broadcastLock;
		std::condition_variable			m_broadcastThreadWaitCondition;

		std::mutex						m_stateLock;
		bool							m_isSubThreadAlive;
		std::mutex						m_subThreadLock;

		// 房间列表
		std::vector<Room>				m_roomList;

		ProcessJson*					m_processJson;
		WriteLog*						m_writeLog;
		FileTransfer*					m_fileTransfer;

        bool							m_thisObjDestory;

	signals:
        void sglCreateNewRoom(QString clientGuid, QString clientName, QString roomPasswd);
		void sglJoinRoom(QString roomID, QString clientGuid, QString clientName);
		void sglLeaveRoom(QString roomID, QString clientGuid, QString clientName);
		//}}}
	};
}
