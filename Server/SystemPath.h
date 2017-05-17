#include <string>

namespace RakNetServer{
	class SystemPath
	{
	public:
		static bool initServerCacheDir(std::string exePath);
		static std::string getServerCacheDir();
		// static std::string getExeDir();
	
	private:
		static void setExeDir(std::string exePath);

		static std::string			s_exeDir;
		static std::string			s_serverCacheDir;
		static const std::string	s_serverCacheFolderName;
	};
}
