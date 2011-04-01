#include <isl/AbstractTcpTask.hxx>

namespace isl
{

AbstractTcpTask::AbstractTcpTask(TcpSocket * socket) :
	AbstractTask(),
	_socket(socket)
{}

TcpSocket& AbstractTcpTask::socket()
{
	return *_socket;
}

} // namespace isl

