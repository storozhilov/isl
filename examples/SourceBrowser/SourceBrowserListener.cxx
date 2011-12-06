#include "SourceBrowserListener.hxx"
#include "SourceBrowserTask.hxx"

/*SourceBrowserService::SourceBrowserService(isl::exp::AbstractSubsystem * owner, isl::exp::TaskDispatcher& taskDispatcher, unsigned int port,
		const std::wstring& rootPath) :
	isl::exp::AbstractTcpService(owner, MaxClients, port),
	_rootPath(rootPath)
{}

isl::exp::AbstractTcpTask * SourceBrowserService::createTask(isl::TcpSocket * socket)
{
	return new SourceBrowserTask(socket, _rootPath);
}*/

namespace isl
{

SourceBrowserListener::SourceBrowserListener(AbstractSubsystem * owner, TaskDispatcher& taskDispatcher, unsigned int port,
		const std::wstring& rootPath) :
	AbstractTcpListener(owner, taskDispatcher, port),
	_rootPath(rootPath)
{}

AbstractTcpTask * SourceBrowserListener::createTask(TcpSocket * socket)
{
	return new SourceBrowserTask(socket, _rootPath);
}

} // namespace isl
