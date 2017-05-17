#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>

#include <QObject>

namespace RakNetServer {
	class WriteLog : public QObject
	{
		Q_OBJECT

	public:
		static WriteLog* GetInstance();
		static WriteLog* GetTempInstance();
		~WriteLog();

		static std::string getLogPath();
		void pushLog(std::string format, ...);
		void printAndWriteLog(std::string format, ...);
		std::string getCurrentTime();

	private:
		void processLog();
		void print(const char* str);
		std::queue<std::string>		m_logQueue;
		std::mutex					m_logLock;
		std::condition_variable		m_threadWaitCondition;
		bool						m_isSubThreadAlive;
		std::mutex					m_subThreadLock;

		WriteLog();
		static WriteLog*			s_writeLog;
		static int					s_userCount;

        bool						m_thisObjDestory;

	signals:
		void sglPrintString(QString);
	};
}
