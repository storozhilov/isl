#include <isl/TcpSocket.hxx>
#include <isl/Core.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/IOError.hxx>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdexcept>

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
	_localAddress(),
	_localPort(),
	_remoteAddress(),
	_remotePort(),
	_connected(false),
	_connectedRwLock()
{}

TcpSocket::TcpSocket(int descriptor) :
	AbstractIODevice(),
	_descriptor(descriptor),
	_localAddress(),
	_localPort(),
	_remoteAddress(),
	_remotePort(),
	_connected(false),
	_connectedRwLock()
{
	// Obtaining remote address/port
	struct sockaddr_in remoteAddress;
	socklen_t remoteAddressSize = sizeof(remoteAddress);
	//if (getpeername(descriptor, reinterpret_cast<struct sockaddr *>(&remoteAddress), &remoteAddressSize)) {
	//	throw Exception(SystemCallError(SystemCallError::GetSockName, errno, SOURCE_LOCATION_ARGS));
	//}
	if (getpeername(descriptor, reinterpret_cast<struct sockaddr *>(&remoteAddress), &remoteAddressSize)) {
		if (errno == ENOTCONN) {
			_isOpen = true;
			return;
		}
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::GetSockName, errno));
	}
	_isOpen = true;
	_connected = true;
	char remoteAddressBuf[INET6_ADDRSTRLEN];
	if (!inet_ntop(AF_INET, &remoteAddress.sin_addr, remoteAddressBuf, INET6_ADDRSTRLEN)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::InetNToP, errno));
	}
	_remoteAddress = String::utf8Decode(remoteAddressBuf);
	_remotePort = ntohs(remoteAddress.sin_port);
	// Obtaining local address/port
	struct sockaddr_in localAddress;
	socklen_t localAddressSize = sizeof(localAddress);
	if (getsockname(descriptor, reinterpret_cast<struct sockaddr *>(&localAddress), &localAddressSize)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::GetSockName, errno));
	}
	char localAddressBuf[INET6_ADDRSTRLEN];
	if (!inet_ntop(AF_INET, &localAddress.sin_addr, localAddressBuf, INET6_ADDRSTRLEN)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::InetNToP, errno));
	}
	_localAddress = String::utf8Decode(localAddressBuf);
	_localPort = ntohs(localAddress.sin_port);
	// Setting opened state to true;
	//_isOpen = true;
	//std::wostringstream msg;
	//msg << L"TcpSocket::TcpSocket(int): Connection established " << _localAddress << L':' <<
	//				_localPort << L" (local) <-> " << _remoteAddress << L':' <<
	//				_remotePort << L" (remote) with socket descriptor " << descriptor;
	//Core::debugLog.logMessage(msg.str());
}

TcpSocket::~TcpSocket()
{
	if (isOpen()) {
		closeSocket();
	}
}

void TcpSocket::bind(unsigned int port, const std::list<std::string>& interfaces)
{
	if (!isOpen()) {
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
	}
	// Setting SO_REUSEADDR to true
	int reuseAddr = 1;
	if (setsockopt(descriptor(), SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SetSockOpt, errno));
	}
	// Binding the socket
	sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);					// TODO Handle 'interfaces' parameter
	if (::bind(descriptor(), reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) != 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Bind, errno));
	}
}

void TcpSocket::listen(unsigned int backLog)
{
	if (!isOpen()) {
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
	}
	if (::listen(descriptor(), backLog) != 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Listen, errno));
	}
}

std::auto_ptr<TcpSocket> TcpSocket::accept(const Timeout& timeout)
{
	if (!isOpen()) {
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
	}
	// Waiting for incoming connection
	timespec selectTimeout = timeout.timeSpec();
	fd_set descriptorsSet;
	FD_ZERO(&descriptorsSet);
	FD_SET(descriptor(), &descriptorsSet);
	int descriptorsCount = pselect(descriptor() + 1, &descriptorsSet, NULL, NULL, &selectTimeout, NULL);
	if (descriptorsCount == 0) {
		// Timeout expired
		return std::auto_ptr<TcpSocket>();
	} else if (descriptorsCount < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PSelect, errno));
	}
	// Extracting and returning pending connection
	int pendingSocketDescriptor = ::accept(descriptor(), NULL, NULL);
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

void TcpSocket::connect(const std::string& address, unsigned int port)
{
	if (!isOpen()) {
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
	}
	struct sockaddr_in remoteAddress;
	//socklen_t remoteAddressSize = sizeof(remoteAddress);
	remoteAddress.sin_family = AF_INET;
	remoteAddress.sin_port = htons(port);
	if (!inet_aton(address.c_str(), &remoteAddress.sin_addr)) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, L"Invalid peer address"));
	}
	//if (::connect(descriptor(), &remoteAddress, sizeof(remoteAddress))) {
	if (::connect(descriptor(), reinterpret_cast<struct sockaddr *>(&remoteAddress), sizeof(remoteAddress))) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Connect, errno));
	}
	/*char remoteAddressBuf[INET6_ADDRSTRLEN];
	if (!inet_ntop(AF_INET, &remoteAddress.sin_addr, remoteAddressBuf, INET6_ADDRSTRLEN)) {
		throw Exception(SystemCallError(SystemCallError::InetNToP, errno, SOURCE_LOCATION_ARGS));
	}
	_remoteAddress = Utf8TextCodec().decode(remoteAddressBuf);
	_remotePort = ntohs(remoteAddress.sin_port);*/
	setIsConnected(true);
}

void TcpSocket::closeSocket()
{
	if (::close(_descriptor)) {
		Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Close, errno).message()));
	}
	_descriptor = -1;
	setIsConnected(false);
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
		//throw Exception(TimeoutExpiredIOError(SOURCE_LOCATION_ARGS));
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
		setIsConnected(false);
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::ConnectionAborted));
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
		//throw Exception(TimeoutExpiredIOError(SOURCE_LOCATION_ARGS));
		return 0;
	}
	ssize_t bytesSent = ::send(_descriptor, buffer, bufferSize, MSG_NOSIGNAL);
	if (bytesSent < 0) {
		if (errno == EPIPE) {
			// Handled because send(2) man page says: "EPIPE: The local end has been shut down on a connection oriented socket.
			// In this case the process will also receive a SIGPIPE unless MSG_NOSIGNAL is set."
			setIsConnected(false);
			throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::ConnectionAborted));
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
		setIsConnected(false);
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::ConnectionAborted));
	}
	return bytesSent;
}

} // namespace isl

