#include "SourceBrowserService.hxx"
#include "SourceBrowserTask.hxx"

SourceBrowserService::SourceBrowserService(isl::AbstractSubsystem * owner, unsigned int port,
		unsigned int maxClients, const std::wstring& rootPath) :
	isl::AbstractTcpService(owner, port, maxClients),
	_rootPath(rootPath)
{}

isl::AbstractTcpService::AbstractTask * SourceBrowserService::createTask(isl::TcpSocket * socket)
{
	return new SourceBrowserTask(socket, _rootPath);
}
