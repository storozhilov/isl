#ifndef ISL__CORE__HXX
#define ISL__CORE__HXX

#include <isl/Log.hxx>

namespace isl
{

class Core
{
public:
	class DebugLog : public Log
	{
	public:
		DebugLog() :
			Log()
		{}
		DebugLog(const std::wstring& prefix) :
			Log(prefix)
		{}

		void log(const AbstractLogMessage& msg);
		void log(const std::string& msg);
		void log(const std::wstring& msg);
	private:
		DebugLog(const Log&);							// No copy

		DebugLog& operator=(const Log&);					// No copy
	};

	static void daemonize();
	static void writePid(const char * pidFileName);

	static Log errorLog;
	static Log warningLog;
	static DebugLog debugLog;
};

} // namespace isl

#endif
