#ifndef TRANSFERFILE_H
#define TRANSFERFILE_H

#include "DefineTypes.h"
#include "WriteLog.h"
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>

namespace RakNet{
	class FileListTransfer;
	class PacketizedTCP;
	class IncrementalReadInterface;
}

namespace RakNetServer{
	class FileTransfer
	{ //{{{
	public:
		//{{{
		FileTransfer();
		~FileTransfer();

		// static tools
		static std::string getFilenameFromFilePath(std::string filePath);
		static std::string getLocalFilePathFromRemotePath(std::string remotePath, bool isFolder = false);
		static std::string getLocalFilePathFromFileHash(uint32_t fileHash, std::string fileExtend);
		static std::string getLocalFilePathFromFileHash(std::string fileHash, std::string fileExtend);
		static std::string getFileExtend(std::string filename);
		static void transformPathSeparator(std::string &path);
		static bool isFileOrDirExist(const char* path);

		template<typename SrcType, typename DesType>
		static void convert(SrcType &srcValue, DesType &desValue);

	private:
		void initTcp();
		void respondToSendingFile();		// 响应客户端发送文件的请求
		void respondToRequestFile();		// 响应客户端请求文件的处理
		bool sendTask(const char* fullPath, bool isFolder, const RakNet::SystemAddress &clientAddress, unsigned  short setID, RakNet::FileListTransfer &fileListTransfer, RakNet::IncrementalReadInterface &incrementalReadInterface);
		bool compareFileHash(unsigned int srcHash, const char* filePath);
		unsigned int getFileHash(const char* filePath);

		static void splitPath(std::string path, std::string &parentPath, std::string &subPath);

		struct SendingTask
		{
			std::string				fullPath;
			bool					isFolder;
			RakNet::SystemAddress	clientAddress;
			unsigned short			setID;
		};

		RakNet::PacketizedTCP*								m_tcpReceiver;
		RakNet::PacketizedTCP*								m_tcpSender;
		WriteLog*											m_writeLog;

		std::mutex											m_lockReceiverThreadAlive;		// 接收文件子线程存活期
		bool												m_receiverThreadAlive;
		std::mutex											m_lockSenderThreadAlive;		// 发送文件子线程存活期
		bool												m_senderThreadAlive;

		std::vector<SendingTask>			m_hystereticTasks;

		unsigned short										TCP_SENDER_PORT;
		unsigned short										TCP_RECEIVER_PORT;
		const unsigned short								MAX_INCOMING_CONNECTIONS;
		const unsigned int									BLOCKSIZE;
	};
}

#endif // TRANSFERFILE_H
