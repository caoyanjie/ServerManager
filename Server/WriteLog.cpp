#include "PubHeader.h"

#include "WriteLog.h"
#include "SystemPath.h"

#include <thread>
#include <fstream>
#include <sstream>
#include <stdarg.h>
#include <assert.h>
#include <windows.h>

RakNetServer::WriteLog* RakNetServer::WriteLog::s_writeLog = nullptr;
int RakNetServer::WriteLog::s_userCount = 0;

// 获取单例
RakNetServer::WriteLog* RakNetServer::WriteLog::GetInstance()
{ //{{{
	if (!s_writeLog)
		s_writeLog = new RakNetServer::WriteLog();
	
	s_userCount += 1;
	return s_writeLog;
} //}}}

// 获取临时单例
RakNetServer::WriteLog* RakNetServer::WriteLog::GetTempInstance()
{ //{{{
	s_userCount -= 1;
	return GetInstance();
} //}}}

// 构造
RakNetServer::WriteLog::WriteLog() :
    m_isSubThreadAlive(true),
    m_thisObjDestory(false)
{
	// std::thread logThread([this]() { this->processLog(); });
	std::thread logThread(&WriteLog::processLog, this);
	logThread.detach();
}

// 析构
RakNetServer::WriteLog::~WriteLog()
{ //{{{
    s_writeLog->pushLog("\n析构 WriteLog......\n");
    s_userCount -= 1;
	if (s_userCount <= 0)
	{
		m_subThreadLock.lock();
		m_isSubThreadAlive = false;
		m_subThreadLock.unlock();

        m_thisObjDestory = true;
        m_threadWaitCondition.notify_one();
        Sleep(100);
		
        s_writeLog->pushLog("\n完成 WriteLog 析构\n");
        delete s_writeLog;
		s_writeLog = 0;
    }
} //}}}

std::string RakNetServer::WriteLog::getLogPath()
{
	return RakNetServer::SystemPath::getServerCacheDir() + "ServerLog.log";
}

// 把日志放入线程间共享队列
void RakNetServer::WriteLog::pushLog(std::string format, ...)
{ //{{{
	//std::ostringstream msg;
	//if (format.find("%") == std::string::npos)
	//{
	//	msg << format;
	//}
	//else
	//{
	//	va_list arg_ptr = nullptr;
	//	va_start(arg_ptr, format);
	//	int start = 0;
	//	for (int index=0; index=format.find("%s", start), index!=(int)std::string::npos; start=index+2)
	//	{
	//		msg << format.substr(start, index-start);
	//		std::string flag = format.substr(index+1, 1);
	//		if (flag == "s")
	//		{
	//			const char* value = va_arg(arg_ptr, const char*);
	//			msg << value;
	//		}
	//		else if (flag == "d")
	//		{
	//			int value = va_arg(arg_ptr, int);
	//			msg << value;
	//		}
	//		else
	//		{
	//			assert(false);
	//		}
	//	}
	//	msg << format.substr(start, format.length()-start);
	//}
//	logQueue.push(msg.str());
	va_list arg_ptr = nullptr;
	va_start(arg_ptr, format);
	char buffer[1024];
	vsnprintf(buffer, 1024, format.c_str(), arg_ptr);
	va_end(arg_ptr);

	std::lock_guard<std::mutex> lock(m_logLock);
	m_logQueue.push(std::string(buffer));
	m_threadWaitCondition.notify_one();
} //}}}

// 输出并写入日志
void RakNetServer::WriteLog::printAndWriteLog(std::string format, ...)
{ //{{{
	std::ostringstream msg;
	if (format.find("%") == std::string::npos)
	{
		msg << format;
	}
	else
	{
		//printf("%s\n", format.c_str());
		va_list arg_ptr = nullptr;
		va_start(arg_ptr, format);
		int start = 0;
		for (int index=0; index=format.find("%", start), index!=(int)std::string::npos; start=index+2)
		{
			msg << format.substr(start, index-start);
			std::string flag = format.substr(index+1, 1);
			if (flag == "d")
			{
				int value = va_arg(arg_ptr, int);
				msg << value;
			}
			else if (flag == "i")
			{
				unsigned int value = va_arg(arg_ptr, unsigned int);
				msg << value;
			}
			else if (flag == "l")
			{
				long value = va_arg(arg_ptr, long);
				msg << value;
			}
			else if (flag == "s")
			{
				const char* value = va_arg(arg_ptr, const char*);
				msg << value;
			}
			else
			{
				assert(false && "没有处理的数据类型");
			}
		}
		msg << format.substr(start, format.length()-start);
	}

	print(msg.str().c_str());
	pushLog(msg.str());
} //}}}

// 获取时间精度（计算代码执行时间或记录日志时间）
std::string RakNetServer::WriteLog::getCurrentTime()
{ //{{{
	const time_t localTime = time(NULL);
	tm *current_time = localtime(&localTime);

	std::ostringstream result;
	result << "[" << current_time->tm_hour << ":" << current_time->tm_min << ":" << current_time->tm_sec << " | " << 1 + current_time->tm_mon << "/" << current_time->tm_mday << "/" << 1900 + current_time->tm_year << "]";
	return result.str();
} //}}}

// 把队列中日志写入日志文件
void RakNetServer::WriteLog::processLog()
{ //{{{
	std::ofstream log;
	log.open(getLogPath());
	while (true)
	{
		m_subThreadLock.lock();
		if (!m_isSubThreadAlive)
		{
			m_subThreadLock.unlock();
			break;
		}
		m_subThreadLock.unlock();

		std::unique_lock<std::mutex> lock(m_logLock);
        m_threadWaitCondition.wait(lock, [this]{return !m_logQueue.empty() || m_thisObjDestory; });
        if (m_thisObjDestory)
        {
            lock.unlock();
            break;
        }
		std::string oneLog = m_logQueue.front();
		m_logQueue.pop();
		lock.unlock();

		log << oneLog;
		log.flush();
	}
	log.close();
} //}}}

void RakNetServer::WriteLog::print(const char* str)
{
	emit sglPrintString(QString::fromLocal8Bit(str));
}
