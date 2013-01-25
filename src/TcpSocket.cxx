#include <isl/common.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/IOError.hxx>
#include <isl/LogMessage.hxx>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>

#if defined (__SVR4) && defined (__sun)					// See http://www.bolthole.com/solaris/
#define MSG_NOSIGNAL 0							// TODO See http://track.sipfoundry.org/browse/XPL-111
#endif

namespace isl
{

/*------------------------------------------------------------------------------
 * TcpSocket
------------------------------------------------------------------------------*/

TcpSocket::TcpSocket() :
	AbstractIODevice(),
	_descriptor(-1),
	_localAddrAutoPtr(),
	_remoteAddrAutoPtr()
{}

TcpSocket::TcpSocket(int descriptor) :
	AbstractIODevice(),
	_descriptor(descriptor),
	_localAddrAutoPtr(),
	_remoteAddrAutoPtr()
{
	fetchPeersData();
	setIsOpen(true);
}

TcpSocket::~TcpSocket()
{
	if (isOpen()) {
		closeSocket();
	}
}

const TcpAddrInfo& TcpSocket::localAddr() const
{
	if (!_localAddrAutoPtr.get()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Local address info have not been initialized"));
	}
	return *_localAddrAutoPtr.get();
}

const TcpAddrInfo& TcpSocket::remoteAddr() const
{
	if (!_remoteAddrAutoPtr.get()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Remote address info have not been initialized"));
	}
	return *_remoteAddrAutoPtr.get();
}

void TcpSocket::bind(const TcpAddrInfo& addrInfo)
{
	if (!isOpen()) {
		//throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
		throw Exception(NotOpenError(SOURCE_LOCATION_ARGS));
	}
	// Setting SO_REUSEADDR to true
	int reuseAddr = 1;
	if (setsockopt(_descriptor, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SetSockOpt, errno));
	}
	// Binding to all endpoints
	const struct addrinfo * ai = addrInfo.addrinfo();
	while (ai) {
		if (::bind(_descriptor, ai->ai_addr, ai->ai_addrlen) != 0) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Bind, errno));
		}
		ai = ai->ai_next;
	}
}

void TcpSocket::listen(unsigned int backLog)
{
	if (!isOpen()) {
		//throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
		throw Exception(NotOpenError(SOURCE_LOCATION_ARGS));
	}
	if (::listen(_descriptor, backLog) != 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Listen, errno));
	}
}

std::auto_ptr<TcpSocket> TcpSocket::accept(const Timeout& timeout)
{
	if (!isOpen()) {
		//throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
		throw Exception(NotOpenError(SOURCE_LOCATION_ARGS));
	}
	// Waiting for incoming connection
	timespec selectTimeout = timeout.timeSpec();
	fd_set descriptorsSet;
	FD_ZERO(&descriptorsSet);
	FD_SET(_descriptor, &descriptorsSet);
	int descriptorsCount = pselect(_descriptor + 1, &descriptorsSet, NULL, NULL, &selectTimeout, NULL);
	if (descriptorsCount == 0) {
		// Timeout expired
		return std::auto_ptr<TcpSocket>();
	} else if (descriptorsCount < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PSelect, errno));
	}
	// Extracting and returning pending connection
	int pendingSocketDescriptor = ::accept(_descriptor, NULL, NULL);
	if (pendingSocketDescriptor < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Accept, errno));
	}
	// Making the socket non-blocking
	int pendingSocketFlags = fcntl(pendingSocketDescriptor, F_GETFL, 0);
	if (pendingSocketFlags < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Fcntl, errno));
	}
	if (!(pendingSocketFlags | O_NONBLOCK)) {
		pendingSocketFlags |= O_NONBLOCK;
		if (fcntl(pendingSocketDescriptor, F_SETFL, pendingSocketFlags) < 0) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Fcntl, errno));
		}
	}
	return std::auto_ptr<TcpSocket>(new TcpSocket(pendingSocketDescriptor));
}

void TcpSocket::connect(const TcpAddrInfo& addrInfo)
{
	if (!isOpen()) {
		//throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
		throw Exception(NotOpenError(SOURCE_LOCATION_ARGS));
	}
	if (::connect(_descriptor, addrInfo.addrinfo()->ai_addr, addrInfo.addrinfo()->ai_addrlen)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Connect, errno));
	}
	fetchPeersData();
}

void TcpSocket::closeSocket()
{
	if (::close(_descriptor)) {
		errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Close, errno).message()));
	}
	_descriptor = -1;
}

void TcpSocket::fetchPeersData()
{
	// Fetching local address info
	struct sockaddr la;
	socklen_t las = sizeof(la);
	if (getsockname(_descriptor, &la, &las)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::GetSockName, errno));
	}
	if (la.sa_family == AF_INET6) {
		struct sockaddr_in6 * addrPtr = reinterpret_cast<struct sockaddr_in6 *>(&la);
		char buf[INET6_ADDRSTRLEN];
		if (!inet_ntop(AF_INET6, &(addrPtr->sin6_addr), buf, INET6_ADDRSTRLEN)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::InetNToP, errno));
		}
		_localAddrAutoPtr.reset(new TcpAddrInfo(TcpAddrInfo::IpV6, buf, ntohs(addrPtr->sin6_port)));
	} else {
		struct sockaddr_in * addrPtr = reinterpret_cast<struct sockaddr_in *>(&la);
		char buf[INET_ADDRSTRLEN];
		if (!inet_ntop(AF_INET, &(addrPtr->sin_addr), buf, INET_ADDRSTRLEN)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::InetNToP, errno));
		}
		_localAddrAutoPtr.reset(new TcpAddrInfo(TcpAddrInfo::IpV4, buf, ntohs(addrPtr->sin_port)));
	}
	// Fetching remote address info
	struct sockaddr ra;
	socklen_t ras = sizeof(ra);
	if (getpeername(_descriptor, &ra, &ras)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::GetPeerName, errno));
	}
	if (ra.sa_family == AF_INET6) {
		struct sockaddr_in6 * addrPtr = reinterpret_cast<struct sockaddr_in6 *>(&ra);
		char buf[INET6_ADDRSTRLEN];
		if (!inet_ntop(AF_INET6, &(addrPtr->sin6_addr), buf, INET6_ADDRSTRLEN)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::InetNToP, errno));
		}
		_remoteAddrAutoPtr.reset(new TcpAddrInfo(TcpAddrInfo::IpV6, buf, ntohs(addrPtr->sin6_port)));
	} else {
		struct sockaddr_in * addrPtr = reinterpret_cast<struct sockaddr_in *>(&ra);
		char buf[INET_ADDRSTRLEN];
		if (!inet_ntop(AF_INET, &(addrPtr->sin_addr), buf, INET_ADDRSTRLEN)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::InetNToP, errno));
		}
		_remoteAddrAutoPtr.reset(new TcpAddrInfo(TcpAddrInfo::IpV4, buf, ntohs(addrPtr->sin_port)));
	}
}

void TcpSocket::openImplementation()
{
	// Creating the socket
	_descriptor = socket(PF_INET, SOCK_STREAM, 0);
	if (_descriptor < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Socket, errno));
	}
	// Making the socket non-blocking
	int socketFlags = fcntl(_descriptor, F_GETFL, 0);
	if (socketFlags < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Fcntl, errno));
	}
	if (!(socketFlags | O_NONBLOCK)) {
		socketFlags |= O_NONBLOCK;
		if (fcntl(_descriptor, F_SETFL, socketFlags) < 0) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Fcntl, errno));
		}
	}
}

void TcpSocket::closeImplementation()
{
	closeSocket();
}

size_t TcpSocket::readImplementation(char * buffer, size_t bufferSize, const Timeout& timeout)
{
	timespec readTimeout = timeout.timeSpec();
	fd_set readDescriptorsSet;
	FD_ZERO(&readDescriptorsSet);
	FD_SET(_descriptor, &readDescriptorsSet);
	int descriptorsCount = pselect(_descriptor + 1, &readDescriptorsSet, NULL, NULL, &readTimeout, NULL);
	if (descriptorsCount < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PSelect, errno));
	} else if (descriptorsCount == 0) {
		// Timeout expired
		return 0;
	}
	ssize_t bytesReceived = recv(_descriptor, buffer, bufferSize, 0);
	if (bytesReceived < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Recv, errno));
		//if (errno == EAGAIN || errno == EWOULDBLOCK) {
		//	// No more data available
		//	return 0;
		//} else {
		//	throw Exception(SystemCallError(SystemCallError::Recv, errno, SOURCE_LOCATION_ARGS));
		//}
	} else if (bytesReceived == 0) {
		// Connection has been aborted by the client.
		//throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::ConnectionAborted));
		throw Exception(ConnectionAbortedError(SOURCE_LOCATION_ARGS));
	}
	return bytesReceived;
}

size_t TcpSocket::writeImplementation(const char * buffer, size_t bufferSize, const Timeout& timeout)
{
	timespec writeTimeout = timeout.timeSpec();
	fd_set writeDescriptorsSet;
	FD_ZERO(&writeDescriptorsSet);
	FD_SET(_descriptor, &writeDescriptorsSet);
	int descriptorsCount = pselect(_descriptor + 1, NULL, &writeDescriptorsSet, NULL, &writeTimeout, NULL);
	if (descriptorsCount < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PSelect, errno));
	} else if (descriptorsCount == 0) {
		// Timeout expired
		return 0;
	}
	ssize_t bytesSent = ::send(_descriptor, buffer, bufferSize, MSG_NOSIGNAL);
	if (bytesSent < 0) {
		if (errno == EPIPE) {
			// Handled because send(2) man page says: "EPIPE: The local end has been shut down on a connection oriented socket.
			// In this case the process will also receive a SIGPIPE unless MSG_NOSIGNAL is set."
			//throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::ConnectionAborted));
			throw Exception(ConnectionAbortedError(SOURCE_LOCATION_ARGS));
		} else {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Send, errno));
		}
		//if (errno == EAGAIN || errno == EWOULDBLOCK) {
		//	// No more data available
		//	return totalBytesSent;
		//} else {
		//	throw Exception(SystemCallError(SystemCallError::Send, errno, SOURCE_LOCATION_ARGS));
		//}
	} else if (bytesSent == 0) {
		// Connection has been aborted by the client.
		//throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::ConnectionAborted));
		throw Exception(ConnectionAbortedError(SOURCE_LOCATION_ARGS));
	}
	return bytesSent;
}

//------------------------------------------------------------------------------

TcpSocket_NEW::TcpSocket_NEW() :
	AbstractPosixIODevice(),
	_localAddrAutoPtr(),
	_remoteAddrAutoPtr()
{}

TcpSocket_NEW::TcpSocket_NEW(int handle) :
	AbstractPosixIODevice(handle),
	_localAddrAutoPtr(),
	_remoteAddrAutoPtr()
{
	fetchPeersData();
}

const TcpAddrInfo& TcpSocket_NEW::localAddr() const
{
	if (!_localAddrAutoPtr.get()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Local address info have not been initialized"));
	}
	return *_localAddrAutoPtr.get();
}

const TcpAddrInfo& TcpSocket_NEW::remoteAddr() const
{
	if (!_remoteAddrAutoPtr.get()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Remote address info have not been initialized"));
	}
	return *_remoteAddrAutoPtr.get();
}

void TcpSocket_NEW::bind(const TcpAddrInfo& addrInfo)
{
	if (!isOpen()) {
		//throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
		throw Exception(NotOpenError(SOURCE_LOCATION_ARGS));
	}
	// Setting SO_REUSEADDR to true
	int reuseAddr = 1;
	if (setsockopt(handle(), SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SetSockOpt, errno));
	}
	// Binding to all endpoints
	const struct addrinfo * ai = addrInfo.addrinfo();
	while (ai) {
		if (::bind(handle(), ai->ai_addr, ai->ai_addrlen) != 0) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Bind, errno));
		}
		ai = ai->ai_next;
	}
}

void TcpSocket_NEW::listen(unsigned int backLog)
{
	if (!isOpen()) {
		//throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
		throw Exception(NotOpenError(SOURCE_LOCATION_ARGS));
	}
	if (::listen(handle(), backLog) != 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Listen, errno));
	}
}

std::auto_ptr<TcpSocket_NEW> TcpSocket_NEW::accept(const Timeout& timeout)
{
	if (!isOpen()) {
		//throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
		throw Exception(NotOpenError(SOURCE_LOCATION_ARGS));
	}
	// Waiting for incoming connection
	timespec selectTimeout = timeout.timeSpec();
	fd_set descriptorsSet;
	FD_ZERO(&descriptorsSet);
	FD_SET(handle(), &descriptorsSet);
	int descriptorsCount = pselect(handle() + 1, &descriptorsSet, NULL, NULL, &selectTimeout, NULL);
	if (descriptorsCount == 0) {
		// Timeout expired
		return std::auto_ptr<TcpSocket_NEW>();
	} else if (descriptorsCount < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PSelect, errno));
	}
	// Extracting and returning pending connection
	int pendingSocketDescriptor = ::accept(handle(), NULL, NULL);
	if (pendingSocketDescriptor < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Accept, errno));
	}
	// Making the socket non-blocking
	int pendingSocketFlags = fcntl(pendingSocketDescriptor, F_GETFL, 0);
	if (pendingSocketFlags < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Fcntl, errno));
	}
	if (!(pendingSocketFlags | O_NONBLOCK)) {
		pendingSocketFlags |= O_NONBLOCK;
		if (fcntl(pendingSocketDescriptor, F_SETFL, pendingSocketFlags) < 0) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Fcntl, errno));
		}
	}
	return std::auto_ptr<TcpSocket_NEW>(new TcpSocket_NEW(pendingSocketDescriptor));
}

void TcpSocket_NEW::connect(const TcpAddrInfo& addrInfo)
{
	if (!isOpen()) {
		throw Exception(NotOpenError(SOURCE_LOCATION_ARGS));
	}
	if (::connect(handle(), addrInfo.addrinfo()->ai_addr, addrInfo.addrinfo()->ai_addrlen)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Connect, errno));
	}
	fetchPeersData();
}

void TcpSocket_NEW::fetchPeersData()
{
	// Fetching local address info
	struct sockaddr la;
	socklen_t las = sizeof(la);
	if (getsockname(handle(), &la, &las)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::GetSockName, errno));
	}
	if (la.sa_family == AF_INET6) {
		struct sockaddr_in6 * addrPtr = reinterpret_cast<struct sockaddr_in6 *>(&la);
		char buf[INET6_ADDRSTRLEN];
		if (!inet_ntop(AF_INET6, &(addrPtr->sin6_addr), buf, INET6_ADDRSTRLEN)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::InetNToP, errno));
		}
		_localAddrAutoPtr.reset(new TcpAddrInfo(TcpAddrInfo::IpV6, buf, ntohs(addrPtr->sin6_port)));
	} else {
		struct sockaddr_in * addrPtr = reinterpret_cast<struct sockaddr_in *>(&la);
		char buf[INET_ADDRSTRLEN];
		if (!inet_ntop(AF_INET, &(addrPtr->sin_addr), buf, INET_ADDRSTRLEN)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::InetNToP, errno));
		}
		_localAddrAutoPtr.reset(new TcpAddrInfo(TcpAddrInfo::IpV4, buf, ntohs(addrPtr->sin_port)));
	}
	// Fetching remote address info
	struct sockaddr ra;
	socklen_t ras = sizeof(ra);
	if (getpeername(handle(), &ra, &ras)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::GetPeerName, errno));
	}
	if (ra.sa_family == AF_INET6) {
		struct sockaddr_in6 * addrPtr = reinterpret_cast<struct sockaddr_in6 *>(&ra);
		char buf[INET6_ADDRSTRLEN];
		if (!inet_ntop(AF_INET6, &(addrPtr->sin6_addr), buf, INET6_ADDRSTRLEN)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::InetNToP, errno));
		}
		_remoteAddrAutoPtr.reset(new TcpAddrInfo(TcpAddrInfo::IpV6, buf, ntohs(addrPtr->sin6_port)));
	} else {
		struct sockaddr_in * addrPtr = reinterpret_cast<struct sockaddr_in *>(&ra);
		char buf[INET_ADDRSTRLEN];
		if (!inet_ntop(AF_INET, &(addrPtr->sin_addr), buf, INET_ADDRSTRLEN)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::InetNToP, errno));
		}
		_remoteAddrAutoPtr.reset(new TcpAddrInfo(TcpAddrInfo::IpV4, buf, ntohs(addrPtr->sin_port)));
	}
}

int TcpSocket_NEW::openImpl()
{
	// Creating the socket
	int s = socket(PF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Socket, errno));
	}
	// Making the socket non-blocking
	/*int socketFlags = fcntl(s, F_GETFL, 0);
	if (socketFlags < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Fcntl, errno));
	}
	if (!(socketFlags | O_NONBLOCK)) {
		socketFlags |= O_NONBLOCK;
		if (fcntl(s, F_SETFL, socketFlags) < 0) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Fcntl, errno));
		}
	}*/
	return s;
}

void TcpSocket_NEW::onReadException()
{
	throw Exception(Error(SOURCE_LOCATION_ARGS, "Exception occured on socket descriptor while reading the data"));
}

void TcpSocket_NEW::onReadEndOfFile()
{
	throw Exception(ConnectionAbortedError(SOURCE_LOCATION_ARGS));
}

void TcpSocket_NEW::onWriteException()
{
	throw Exception(Error(SOURCE_LOCATION_ARGS, "Exception occured on socket descriptor while writing the data"));
}

void TcpSocket_NEW::onWriteEndOfFile()
{
	throw Exception(ConnectionAbortedError(SOURCE_LOCATION_ARGS));
}

} // namespace isl
