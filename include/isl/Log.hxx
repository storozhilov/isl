#ifndef ISL__LOG__HXX
#define ISL__LOG__HXX

#include <isl/LogDispatcher.hxx>
#include <isl/ReadWriteLock.hxx>
#include <isl/Exception.hxx>
#include <stdexcept>
#include <string>
#include <sstream>
#include <ostream>

namespace isl
{

class AbstractLogTarget;

class Log
{
public:
	Log(const std::wstring& prefix = std::wstring());
	~Log();

	void connectTarget(const AbstractLogTarget& target);
	void disconnectTarget(const AbstractLogTarget& target);
	void disconnectTargets();
	void logMessage(const std::wstring& msg);
	void logDebug(SOURCE_LOCATION_ARGS_DECLARATION, const std::wstring& msg);
	void logException(const std::exception& expt, SOURCE_LOCATION_ARGS_DECLARATION, const std::wstring& msg = std::wstring());
	void setPrefix(const std::wstring& newPrefix);
	std::wstring prefix() const;
private:
	Log(const Log&);							// No copy

	Log& operator=(const Log&);						// No copy
	
	std::wstring _prefix;
	mutable ReadWriteLock _logRWLock;

	static std::wstring composeSourceLocation(SOURCE_LOCATION_ARGS_DECLARATION);

	static LogDispatcher dispatcher;
};

} // namespace isl

#endif

