#include "PubHeader.h"

#include "FileTransfer.h"
#include "SystemPath.h"
#include "ProcessJson.h"

// raknet send and receive file about
//{{{
#include "RakNet/FileListTransfer.h"
#include "RakNet/PacketizedTCP.h"
#include "RakNet/SuperFastHash.h"
#include "RakNet/RakSleep.h"
//}}}

// raknet send file about
//{{{
#include "RakNet/FileOperations.h"
#include "RakNet/IncrementalReadInterface.h"
//}}}

// raknet receive file about
//{{{
#include "RakNet/FileListTransferCBInterface.h"
//}}}

// std namespace headers
//{{{
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <thread>
//}}}

// io headers
//{{{
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#define _access access
#include <sys/stat.h>
#define _mkdir(a) mkdir((a),0755)
#endif

// callback
namespace RakNetServer{
	// Sender progress notification
	class SenderProgress : public RakNet::FileListProgress
	{
	private:
		virtual void OnFilePush(const char* fileName, unsigned int fileLengthBytes, unsigned int offset, unsigned int bytesBeingSent, bool done, RakNet::SystemAddress targetSystem, unsigned short setID)
		{
			//if (fileLengthBytes != 0)
			//	printf("文件 %s 已发送 %i%%\n", fileName, (offset+bytesBeingSent) * 100 / fileLengthBytes);
		}
	
		virtual void OnFilePushesComplete(RakNet::SystemAddress systemAddress, unsigned short setID)
		{
			char ip[32];
			systemAddress.ToString(false, (char*)ip);
			printf("文件发送完成！ 接收者：IP: %s\n", ip);
		}
	
		virtual void OnSendAborted(RakNet::SystemAddress systemAddress)
		{
			char ip[32];
			systemAddress.ToString(true, (char*)ip);
			RAKNET_DEBUG_PRINTF("\n发送失败！ 接受者：IP: %s\n", ip);
		}
	
		std::string filePath;
	}senderProgress;
	
	// Receiver progress notification
	class ReceiverProgress : public RakNet::FileListTransferCBInterface
	{
	public:
		void init(bool isDirectory = false) 
		{
			this->isDirectory = isDirectory;
			isRecordingTime = false;
			receiving = true; 
		}
	
		bool isReceiving() 
		{
			return receiving; 
		}
	
		bool OnFile(OnFileStruct *onFileStruct)
		{
			std::string path;
			if (isDirectory)
				path = FileTransfer::getLocalFilePathFromRemotePath(std::string(onFileStruct->fileName), true);
			else
				path = FileTransfer::getLocalFilePathFromFileHash(onFileStruct->context.flnc_extraData1, std::string(FileTransfer::getFileExtend(onFileStruct->fileName)));
			savedPath = path;

			if (makeDirectory(path))
			{
				printf("\n正在写入文件......\n");
				FILE *fp = fopen(path.c_str(), "wb");
				fwrite(onFileStruct->fileData, onFileStruct->byteLengthOfThisFile, 1, fp);
				fclose(fp);
				
			}
			return true;
		}
	
		virtual void OnFileProgress(FileProgressStruct *fps)
		{
			//printf("文件 %s 已接收 %i%%\n", fps->onFileStruct->fileName, fps->onFileStruct->bytesDownloadedForThisFile * 100 / fps->onFileStruct->byteLengthOfThisFile);
	
			//if (!isRecordingTime)
			//{
			//	startTime = clock();
			//	isRecordingTime = true;
			//}
		}
	
		virtual bool OnDownloadComplete(DownloadCompleteStruct *dcs)
		{
			//float offset = (clock() - startTime) / 1000.0f;
			printf("传输完成, 保存在 %s\n", savedPath.c_str());
			receiving = false;
			isRecordingTime = false;		// 记录传输所用时间

			return false;
		}
	
	private:
		bool makeDirectory(std::string multiLevelPath)
		{
			FileTransfer::transformPathSeparator(multiLevelPath);
			auto pos = multiLevelPath.find("/");
			while (pos != std::string::npos)
			{
				std::string dir = multiLevelPath.substr(0, pos);
				if (!FileTransfer::isFileOrDirExist(dir.c_str()))
				{
					//CreateDirectory(dir, NULL);
					if (_mkdir(dir.c_str()) == -1)
					{
						printf("文件夹创建失败！");
						return false;
					}
				}
				pos = multiLevelPath.find("/", pos + 1);
			}
			return true;
		}

		std::string		savedPath;
		bool			isDirectory;
		bool			receiving;
		long			startTime;
		bool			isRecordingTime;
	}receiverProgress;
}

RakNetServer::FileTransfer::FileTransfer() :
	TCP_SENDER_PORT(50003),
	TCP_RECEIVER_PORT(50004),
	MAX_INCOMING_CONNECTIONS(100),
	BLOCKSIZE(3000 * 1024),
	m_receiverThreadAlive(true),
	m_senderThreadAlive(true)
{
	m_writeLog = WriteLog::GetInstance();

	m_tcpReceiver = RakNet::PacketizedTCP::GetInstance();
	m_tcpSender = RakNet::PacketizedTCP::GetInstance();

	initTcp();

	std::thread senderThread(&FileTransfer::respondToRequestFile, this);
	std::thread receiverThread(&FileTransfer::respondToSendingFile, this);
	senderThread.detach();
	receiverThread.detach();
}

RakNetServer::FileTransfer::~FileTransfer()
{
    m_writeLog->pushLog("\n析构 FileTransfer......\n");
    std::unique_lock<std::mutex> receiverLock(m_lockReceiverThreadAlive);
 	m_receiverThreadAlive = false;
	receiverLock.unlock();

	std::unique_lock<std::mutex> senderLock(m_lockSenderThreadAlive);
 	m_senderThreadAlive = false;
	senderLock.unlock();

	Sleep(1000);
    m_writeLog->pushLog("\n完成 FileTransfer 析构\n");
}

// init
void RakNetServer::FileTransfer::initTcp()
{
	// 尝试在端口建立 TCP 监听
	while (!m_tcpSender->Start(TCP_SENDER_PORT, MAX_INCOMING_CONNECTIONS, -99999, AF_INET))
	{
		m_tcpSender->Stop();
		TCP_SENDER_PORT += 1;
	} 
	m_writeLog->pushLog("TCP 在端口 %u 监听成功\t%s\n", TCP_SENDER_PORT, m_writeLog->getCurrentTime().c_str());
	
	while (!m_tcpReceiver->Start(TCP_RECEIVER_PORT, MAX_INCOMING_CONNECTIONS, -99999, AF_INET))
	{
		m_tcpReceiver->Stop();
		TCP_RECEIVER_PORT += 1;
	} 
	m_writeLog->pushLog("TCP 在端口 %u 监听成功\t%s\n", TCP_RECEIVER_PORT, m_writeLog->getCurrentTime().c_str());
}

// respond to sending file
void RakNetServer::FileTransfer::respondToSendingFile()
{
	// attach plugin
	RakNet::FileListTransfer    fileListTransfer;
	m_tcpReceiver->AttachPlugin(&fileListTransfer);

	// process request
	RakNet::SystemAddress address;
	RakNet::SystemAddress newConnectedAddress;
	RakNet::Packet        *packetReceived;
	while (true)
	{
		std::unique_lock<std::mutex> lock(m_lockReceiverThreadAlive);
		if (!m_receiverThreadAlive)
		{
			lock.unlock();
			break;
		}

		packetReceived = m_tcpReceiver->Receive();

		// 一个客户端连入
		address = m_tcpReceiver->HasNewIncomingConnection();
		if (address != RakNet::UNASSIGNED_SYSTEM_ADDRESS)
		{
			newConnectedAddress = address;
			char ip[32];
			address.ToString(true, (char*)ip);
			m_writeLog->pushLog("一个发送文件客户端连入，IP: %s\n", ip);
		}

		// 解析数据包，发送 setID
		if (packetReceived)
		{
			// 解析请求的文件
			std::string filename;
			std::string fileHash;
			bool		isFolder;
			std::string fullPath;

			// set receiver handler
			unsigned short setID = fileListTransfer.SetupReceive(&receiverProgress, false, packetReceived->systemAddress);
			RakNet::RakString data("%i", setID);
			const char* dataStr = data.C_String();
			m_tcpReceiver->Send(dataStr, (unsigned int)strlen(dataStr) + 1, newConnectedAddress, false);
		}
		m_tcpReceiver->DeallocatePacket(packetReceived);

		lock.unlock();
		Sleep(30);
	}
	m_tcpReceiver->DetachPlugin(&fileListTransfer);
}

// respond to request file
void RakNetServer::FileTransfer::respondToRequestFile()
{
	// attach plugin
	RakNet::IncrementalReadInterface incrementalReadInterface;
	RakNet::FileListTransfer fileListTransfer;
	fileListTransfer.AddCallback(&senderProgress);
	fileListTransfer.StartIncrementalReadThreads(1);
	m_tcpSender->AttachPlugin(&fileListTransfer);
	
	// process request
	RakNet::SystemAddress address;
	RakNet::SystemAddress connectedAddress;
	RakNet::Packet		  *packetReceived;
	while (true)
	{
		std::unique_lock<std::mutex> lock(m_lockSenderThreadAlive);
		if (!m_senderThreadAlive)
		{
			lock.unlock();
			break;
		}

		// 一个客户端连入
		address = m_tcpSender->HasNewIncomingConnection();
		if (address != RakNet::UNASSIGNED_SYSTEM_ADDRESS)
		{
			connectedAddress = address;
			char ip[32];
			connectedAddress.ToString(true, ip);
			m_writeLog->pushLog("一个请求文件客户端连入, IP: %s\n", ip);
		}

		// 检查缓存任务
		if (!m_hystereticTasks.empty())
		{
			auto task = m_hystereticTasks.begin();
			bool result = sendTask(task->fullPath.c_str(), task->isFolder, task->clientAddress, task->setID, fileListTransfer, incrementalReadInterface);
			if (result)
			{
				m_hystereticTasks.erase(task);
			}
		}

		// 解析请求
		packetReceived = m_tcpSender->Receive();
		if (packetReceived)
		{
			// 解析请求的文件
			RakNet::RakString data(packetReceived->data);

			std::string filename = ProcessJson::getJsonStringValue(data.C_String(), "filename");
			std::string fileHash = ProcessJson::getJsonStringValue(data.C_String(), "fileHash");
			bool		isFolder = ProcessJson::getJsonStringValue(data.C_String(), "isFolder") == "true";
			std::string setIDStr = ProcessJson::getJsonStringValue(data.C_String(), "setID");

			std::string fullPath = getLocalFilePathFromFileHash(fileHash, getFileExtend(filename));
			unsigned short setID;
			convert(setIDStr, setID);

			bool result = sendTask(fullPath.c_str(), isFolder, connectedAddress, setID, fileListTransfer, incrementalReadInterface);

			if (!result)
			{
				SendingTask task;
				task.fullPath = fullPath;
				task.isFolder = isFolder;
				task.setID = setID;
				task.clientAddress = connectedAddress;
				m_hystereticTasks.push_back(task);
				continue;
			}
		}
		m_tcpSender->DeallocatePacket(packetReceived);

		lock.unlock();
		Sleep(30);
	}
	RakSleep(100);
	m_tcpSender->DetachPlugin(&fileListTransfer);
}

// send task
bool RakNetServer::FileTransfer::sendTask(const char* fullPath, bool isFolder, const RakNet::SystemAddress &clientAddress, unsigned short setID, RakNet::FileListTransfer &fileListTransfer, RakNet::IncrementalReadInterface &incrementalReadInterface)
{
	// 检查文件
	unsigned int fileLength = GetFileLength(fullPath);
	if (fileLength == 0)
	{
		return false;
	}

	// 装载文件
	RakNet::FileList fileList;
	if (isFolder)
	{
		std::string parentDir, subDir;
		splitPath(fullPath, parentDir, subDir);
		fileList.AddFilesFromDirectory(parentDir.c_str(), subDir.c_str(), false, true, true, FileListNodeContext(0, 0, 0, 0));
	}
	else
	{
		fileList.AddFile(fullPath, fullPath, 0, fileLength, fileLength, FileListNodeContext(0, getFileHash(fullPath), 0, 0), true);
	}
	m_writeLog->pushLog("文件 %s 装载成功\n等待接收者......\n", fullPath);
	RakSleep(100);

	// 发送文件
	fileListTransfer.Send(&fileList, 0, clientAddress, setID, HIGH_PRIORITY, 0, &incrementalReadInterface, BLOCKSIZE);

	return true;
}

// private process
unsigned int RakNetServer::FileTransfer::getFileHash(const char* filePath)
{
	unsigned int hash = SuperFastHashFile(filePath);
	if (RakNet::BitStream::DoEndianSwap())
	{
		RakNet::BitStream::ReverseBytesInPlace((unsigned char*)&hash, sizeof(hash));
	}
	return hash;
}

// private process
bool RakNetServer::FileTransfer::compareFileHash(unsigned int srcHash, const char* filePath)
{
	return getFileHash(filePath) == srcHash ? true : false;
}

// static tool
std::string RakNetServer::FileTransfer::getFilenameFromFilePath(std::string filePath)
{
	// 如果路径为"\"替换为"/"
	transformPathSeparator(filePath);

	// 去掉路径获得文件名
	auto namePos = filePath.rfind("/");
	return (namePos == std::string::npos ? filePath : filePath.substr(namePos+1, filePath.size()-namePos));
}

// static tool
std::string RakNetServer::FileTransfer::getLocalFilePathFromRemotePath(std::string remotePath, bool isDirectory)
{
	return RakNetServer::SystemPath::getServerCacheDir() + "TempFiles/" + (isDirectory ? remotePath : getFilenameFromFilePath(remotePath));
}

// static tool
std::string RakNetServer::FileTransfer::getLocalFilePathFromFileHash(uint32_t fileHash, std::string fileExtend)
{
	std::ostringstream filename;
	filename << fileHash << "." << fileExtend;
	return RakNetServer::SystemPath::getServerCacheDir() + "TempFiles/" + filename.str();
}

// static tool
std::string RakNetServer::FileTransfer::getLocalFilePathFromFileHash(std::string fileHash, std::string fileExtend)
{
	std::ostringstream filename;
	filename << fileHash << "." << fileExtend;
	return RakNetServer::SystemPath::getServerCacheDir() + "TempFiles/" + filename.str();
}

// static tool
std::string RakNetServer::FileTransfer::getFileExtend(std::string filename)
{
	auto pos = filename.rfind(".");
	if (pos == std::string::npos)
	{
		assert(false);
		return std::string("");
	}
	return filename.substr(pos + 1, filename.size() - pos);
}

// static tool
void RakNetServer::FileTransfer::transformPathSeparator(std::string &path)
{
	auto pos = path.find("\\");
	while (pos != std::string::npos)
	{
		path.replace(pos, 1, "/");
		pos = path.find("\\");
	}
	//while ((auto pos = sourceFilePath.find("\\")) != std::string::npos)
	//{
	//	sourceFilePath.replace(pos, 1, "/");
	//}
}

// static tool
bool RakNetServer::FileTransfer::isFileOrDirExist(const char* path)
{
	return _access(path, 0) != -1 ? true : false;
}

// static tool
void RakNetServer::FileTransfer::splitPath(std::string path, std::string &parentPath, std::string &subPath)
{
	transformPathSeparator(path);

	//auto pos = std::string::npos;
	//if (path.at(path.size()-1) == '/')
	//	pos = path.rfind("/", path.size()-1);
	//else
	//	pos = path.rfind("/");
	auto pos = (path.at(path.size()-1) == '/' ? path.rfind("/", path.size()-1) : path.rfind("/"));

	if (pos == std::string::npos)
	{
		assert(false);
		return;
	}

	parentPath = path.substr(0, pos);
	subPath = path.substr(pos+1, path.size()-pos);
}

template<typename SrcType, typename DesType>
void RakNetServer::FileTransfer::convert(SrcType &srcValue, DesType &desValue)
{
	std::stringstream middle;
	middle << srcValue;
	middle >> desValue;
}
