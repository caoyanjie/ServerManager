#pragma once

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"

#include "DefineTypes.h"
#include "DataProcess.h"
#include "WriteLog.h"

#include <queue>
#include <mutex>
#include <fstream>
#include <ctime>
#include <condition_variable>
#include <atomic>

#include <QObject>
#include <QImage>

namespace RakNet {
	//{{{
	class Packet;
	class RakString;
	//}}}
}

namespace RakNetServer {
	class UdpTransmission : public QObject
	{
		Q_OBJECT

	public:
		//{{{
		UdpTransmission();
		~UdpTransmission();

        // void start();
        // void close();
		void test();
        // void setClientViewportState(bool state, const char* clientGuid, int level = 0);
		void changeMonitorQuility(const char* clientGuid, int level);
		//}}}

	private:
		//{{{
		// connect
		void listening();																													// 监听
		void listeningClientViewport();

		// packet
		std::string getPacketString(RakNet::Packet *packet);																				// 获得流中字符串

		// room
		void createRoom(const char* clientIPWithPort, const RakNet::RakNetGUID clientGuid, const char* roomName, const char* roomPasswd, const char* username, const char* hasRole, const char* voiceServer);							// 创建房间
		void joinRoom(const char* roomIDString, const char* roomPasswdString, const char* clientIPWithPort, const RakNet::RakNetGUID clientGuid, const char* username);	// 加入房间
		void leaaveRoom(const RakNet::RakNetGUID guid);																						// 移除房间
		
		// data
		void processData(RakNet::Packet * packet);																							// 处理数据
		void sendData(RakNet::RakNetGUID guid, RakNet::MessageID msgType, bool broadcast = false, std::string msg = "");					// 发送消息给指定客户端
		void sendData(RakNet::RakNetGUID guid, RakNet::MessageID msgType, std::vector<std::string> msg, bool broadcast = false);			// 发送消息给指定客户端
		void broadcast();																													// 广播消息
		
		// command
		void updateUsername(const RakNet::RakNetGUID clientGuid, const char* newUsername);
		void showClientViewport(RakNet::Packet *packet);

		// print string
		void print(const char* str);

		std::queue<std::string>		m_dataQueue;

		// 
		RakNet::RakPeerInterface*	m_server;
		RakNet::RakPeerInterface*	m_serverClientViewport;

		//
		RakNetServer::DataProcess*	m_dataProcess;
		std::mutex					m_dataLock, m_serverLock;
		std::condition_variable		m_threadWaitCondition;

		std::mutex					m_monitorThreadLock;
		std::atomic_bool			m_monitorThreadActive;
        std::atomic_bool			m_listeningThreadActive;
        std::atomic_bool			m_broadcastThradActive;

		// log
		WriteLog*					m_writeLog;

		// 常量
		const int					SERVER_PORT;
		const int					SERVER_MONITOR_CLIENT_PORT;
		const int					MAX_CLIENT;
        const char*					CONNECT_PASSWORD;

	signals:
        void sglPrintString(QString msg);
		void sglShowServerIp(QStringList ips);
        void sglCreateNewRoom(QString clientGuid, QString clientName, QString roomPasswd);
		void sglJoinRoom(QString roomID, QString clientGuid, QString clientName);
		void sglLeaveRoom(QString roomID, QString clientGuid, QString clientName);
        void sglShowClientViewport(QString roomID, QString clientGuid, QImage image, int imgWidth, int imgHeight, QString imgFormat);
		void sglUpdateUsername(QString roomID, QString clientGuid, QString newUsername);
		//}}}

    public slots:
        void start();
        void close();
        void setClientViewportState(bool state, QString clientGuid, int level);
    };
}
