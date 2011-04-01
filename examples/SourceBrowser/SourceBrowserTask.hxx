#ifndef ISL__SOURCE_BROWSER_TASK__HXX
#define ISL__SOURCE_BROWSER_TASK__HXX

#include <isl/AbstractHTTPTask.hxx>
#include <isl/TcpSocket.hxx>

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
