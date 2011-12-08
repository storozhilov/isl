#ifndef ISL__SOURCE_BROWSER_SERVICE__HXX
#define ISL__SOURCE_BROWSER_SERVICE__HXX

#include <isl/AbstractTcpService.hxx>

class SourceBrowserService : public isl::AbstractTcpService
{
public:
	SourceBrowserService(isl::AbstractSubsystem * owner, unsigned int port, unsigned int maxClients,
			const std::wstring& rootPath);
private:
	SourceBrowserService();
	SourceBrowserService(const SourceBrowserService&);							// No copy

	SourceBrowserService& operator=(const SourceBrowserService&);						// No copy

	virtual isl::AbstractTcpService::AbstractTask * createTask(isl::TcpSocket * socket);

	std::wstring _rootPath;
};

#endif
