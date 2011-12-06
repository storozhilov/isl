#ifndef ISL__SOURCE_BROWSER_LISTENER__HXX
#define ISL__SOURCE_BROWSER_LISTENER__HXX

#include <isl/AbstractTcpService.hxx>
#include <isl/AbstractTcpListener.hxx>

/*class SourceBrowserService : public isl::exp::AbstractTcpService
{
public:
	SourceBrowserService(isl::exp::AbstractSubsystem * owner, isl::exp::TaskDispatcher& taskDispatcher, unsigned int port,
			const std::wstring& rootPath);
private:
	SourceBrowserService();
	SourceBrowserService(const SourceBrowserService&);							// No copy

	SourceBrowserService& operator=(const SourceBrowserService&);						// No copy

	enum Constants {
		MaxClients = 10
	};
	
	virtual isl::exp::AbstractTcpTask * createTask(isl::TcpSocket * socket);

	std::wstring _rootPath;
};*/

namespace isl
{

class SourceBrowserListener : public AbstractTcpListener
{
public:
	SourceBrowserListener(AbstractSubsystem * owner, TaskDispatcher& taskDispatcher, unsigned int port,
			const std::wstring& rootPath);
private:
	SourceBrowserListener();
	SourceBrowserListener(const SourceBrowserListener&);							// No copy

	SourceBrowserListener& operator=(const SourceBrowserListener&);						// No copy
	
	virtual AbstractTcpTask * createTask(TcpSocket * socket);

	std::wstring _rootPath;
};

} // namespace isl

#endif
