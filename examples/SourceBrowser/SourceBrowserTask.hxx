#ifndef ISL__SOURCE_BROWSER_TASK__HXX
#define ISL__SOURCE_BROWSER_TASK__HXX

#include <isl/AbstractHTTPTask.hxx>
#include <isl/TcpSocket.hxx>
#include "SourceBrowserGenerator.hxx"

/*class SourceBrowserTask : public isl::exp::AbstractHTTPTask
{
public:
	SourceBrowserTask(isl::TcpSocket * socket, const std::wstring& rootPath) :
		AbstractHTTPTask(socket),
		_rootPath(rootPath)
	{}
private:
	SourceBrowserTask();
	SourceBrowserTask(const SourceBrowserTask&);						// No copy

	SourceBrowserTask& operator=(const SourceBrowserTask&);					// No copy

	virtual isl::HTTPResponse::AbstractGenerator * createGeneratorOK()
	{
		return new isl::SourceBrowserGenerator(this, _rootPath);
	}

	std::wstring _rootPath;
};*/

namespace isl
{

class SourceBrowserTask : public AbstractHTTPTask
{
public:
	SourceBrowserTask(TcpSocket * socket, const std::wstring& rootPath);
private:
	SourceBrowserTask();
	SourceBrowserTask(const SourceBrowserTask&);						// No copy

	SourceBrowserTask& operator=(const SourceBrowserTask&);					// No copy

	virtual HTTPResponse::AbstractGenerator * createGeneratorOK();

	std::wstring _rootPath;
};

} // namespace isl

#endif
