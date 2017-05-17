#include "PubHeader.h"

#include "SystemPath.h"
#include <assert.h>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#define _access access
#include <sys/stat.h>
#define _mkdir(a) mkdir((a),0755)
#endif

std::string RakNetServer::SystemPath::s_exeDir = "";
std::string RakNetServer::SystemPath::s_serverCacheDir = "";
const std::string RakNetServer::SystemPath::s_serverCacheFolderName = "ServerData/";

// static
bool RakNetServer::SystemPath::initServerCacheDir(std::string exePath)
{ //{{{
	setExeDir(exePath);

	std::string cacheDir = s_exeDir + s_serverCacheFolderName;
	if (_access(cacheDir.c_str(), 0) == -1)
	{
		if (_mkdir(cacheDir.c_str()) == -1)
		{
			printf("Init server cache directory failed! [%s]\n", cacheDir.c_str());
			assert(false && "Init server cache directory failed!");
			return false;
		}
	}
	s_serverCacheDir = cacheDir;
	return true;
} //}}}

// static
std::string RakNetServer::SystemPath::getServerCacheDir()
{ //{{{
	if (s_exeDir == "")
		assert(false && "可能还没调用初始化函数 RakNetServer::SystemPath::initServerCacheDir()");

	return s_serverCacheDir;
} //}}}

// static
void RakNetServer::SystemPath::setExeDir(std::string exePath)
{ //{{{
	std::string separator;
	if (exePath.find("/") != std::string::npos)
		separator = "/";
	else if (exePath.find("\\") != std::string::npos)
		separator = "\\";
	s_exeDir = exePath.substr(0, exePath.rfind(separator)+1);
} //}}}

// static
//std::string RakNetServer::SystemPath::getExeDir()
//{
//	if (exeDir == "")
//		assert(false && "可能还没设置程序路径");
//
//	return exeDir;
//}

