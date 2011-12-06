#ifndef ISL__ABSTRACT_TCP_TASK__HXX
#define ISL__ABSTRACT_TCP_TASK__HXX

#include <isl/AbstractTask.hxx>
#include <isl/TcpSocket.hxx>
#include <memory>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractTcpTask
------------------------------------------------------------------------------*/

class AbstractTcpTask : public AbstractTask
{
public:
	AbstractTcpTask(TcpSocket * socket);

	TcpSocket& socket();
private:
	AbstractTcpTask();
	AbstractTcpTask(const AbstractTcpTask&);						// No copy

	AbstractTcpTask& operator=(const AbstractTcpTask&);					// No copy

	std::auto_ptr<TcpSocket> _socket;
};

namespace exp
{

class AbstractTcpTask : public AbstractTask
{
public:
	AbstractTcpTask(TcpSocket * socket) :
		AbstractTask(),
		_socket(socket)
	{}

	inline TcpSocket& socket() const
	{
		return *_socket;
	}
private:
	AbstractTcpTask();
	AbstractTcpTask(const AbstractTcpTask&);						// No copy

	AbstractTcpTask& operator=(const AbstractTcpTask&);					// No copy

	std::auto_ptr<TcpSocket> _socket;
};

} // namespace exp

} // namespace isl

#endif

