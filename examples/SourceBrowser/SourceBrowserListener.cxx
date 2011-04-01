#include "SourceBrowserListener.hxx"
#include "SourceBrowserTask.hxx"

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
