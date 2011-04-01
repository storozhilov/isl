#include <isl/TcpSocket.hxx>
#include <isl/Core.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/IOError.hxx>
#include <isl/Utf8TextCodec.hxx>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>

//#include <sstream>
#include <stdexcept>

namespace isl
{

/*------------------------------------------------------------------------------
 * TcpSocket
------------------------------------------------------------------------------*/

TcpSocket::TcpSocket() :
	AbstractSocket(),
	_localAddress(),
	_localPort(),
	_remoteAddress(),
	_remotePort(),
	_connected(false)
{}

TcpSocket::TcpSocket(int descriptor) :
	AbstractSocket(descriptor),
	_localAddress(),
	_localPort(),
	_remoteAddress(),
	_remotePort(),
	_connected(false)
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
		throw Exception(SystemCallError(SystemCallError::GetSockName, errno, SOURCE_LOCATION_ARGS));
	}
	_isOpen = true;
	_connected = true;
	char remoteAddressBuf[INET6_ADDRSTRLEN];
	if (!inet_ntop(AF_INET, &remoteAddress.sin_addr, remoteAddressBuf, INET6_ADDRSTRLEN)) {
		throw Exception(SystemCallError(SystemCallError::InetNToP, errno, SOURCE_LOCATION_ARGS));
	}
	_remoteAddress = Utf8TextCodec().decode(remoteAddressBuf);
	_remotePort = ntohs(remoteAddress.sin_port);
	// Obtaining local address/port
	struct sockaddr_in localAddress;
	socklen_t localAddressSize = sizeof(localAddress);
	if (getsockname(descriptor, reinterpret_cast<struct sockaddr *>(&localAddress), &localAddressSize)) {
		throw Exception(SystemCallError(SystemCallError::GetSockName, errno, SOURCE_LOCATION_ARGS));
	}
	char localAddressBuf[INET6_ADDRSTRLEN];
	if (!inet_ntop(AF_INET, &localAddress.sin_addr, localAddressBuf, INET6_ADDRSTRLEN)) {
		throw Exception(SystemCallError(SystemCallError::InetNToP, errno, SOURCE_LOCATION_ARGS));
	}
	_localAddress = Utf8TextCodec().decode(localAddressBuf);
	_localPort = ntohs(localAddress.sin_port);
	// Setting opened state to true;
	//_isOpen = true;
	//std::wostringstream msg;
	//msg << L"TcpSocket::TcpSocket(int): Connection established " << _localAddress << L':' <<
	//				_localPort << L" (local) <-> " << _remoteAddress << L':' <<
	//				_remotePort << L" (remote) with socket descriptor " << descriptor;
	//Core::debugLog.logMessage(msg.str());
}

void TcpSocket::bind(unsigned int port, const std::list<std::wstring>& interfaces)
{
	if (!isOpen()) {
		throw Exception(DeviceIsNotOpenIOError(SOURCE_LOCATION_ARGS));
	}
	sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);					// TODO Handle 'interfaces' parameter
	if (::bind(descriptor(), reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) != 0) {
		throw Exception(SystemCallError(SystemCallError::Bind, errno, SOURCE_LOCATION_ARGS));
	}
}

void TcpSocket::listen(unsigned int backLog)
{
	if (!isOpen()) {
		throw Exception(DeviceIsNotOpenIOError(SOURCE_LOCATION_ARGS));
	}
	if (::listen(descriptor(), backLog) != 0) {
		throw Exception(SystemCallError(SystemCallError::Listen, errno, SOURCE_LOCATION_ARGS));
	}
}

TcpSocket * TcpSocket::accept(const Timeout& timeout)
{
	if (!isOpen()) {
		throw Exception(DeviceIsNotOpenIOError(SOURCE_LOCATION_ARGS));
	}
	// Waiting for incoming connection
	timespec selectTimeout;
	selectTimeout.tv_sec = timeout.seconds();
	selectTimeout.tv_nsec = timeout.nanoSeconds();
	fd_set descriptorsSet;
	FD_ZERO(&descriptorsSet);
	FD_SET(descriptor(), &descriptorsSet);
	int descriptorsCount = pselect(descriptor() + 1, &descriptorsSet, NULL, NULL, &selectTimeout, NULL);
	if (descriptorsCount == 0) {
		// Timeout expired
		return 0;
	} else if (descriptorsCount < 0) {
		throw Exception(SystemCallError(SystemCallError::PSelect, errno, SOURCE_LOCATION_ARGS));
	}
	// Extracting and returning pending connection
	int pendingSocketDescriptor = ::accept(descriptor(), NULL, NULL);
	if (pendingSocketDescriptor < 0) {
		throw Exception(SystemCallError(SystemCallError::Accept, errno, SOURCE_LOCATION_ARGS));
	}
	// Making the socket non-blocking
	int pendingSocketFlags = fcntl(pendingSocketDescriptor, F_GETFL, 0);
	if (pendingSocketFlags < 0) {
		throw Exception(SystemCallError(SystemCallError::Fcntl, errno, SOURCE_LOCATION_ARGS));
	}
	if (!(pendingSocketFlags | O_NONBLOCK)) {
		pendingSocketFlags |= O_NONBLOCK;
		if (fcntl(pendingSocketDescriptor, F_SETFL, pendingSocketFlags) < 0) {
			throw Exception(SystemCallError(SystemCallError::Fcntl, errno, SOURCE_LOCATION_ARGS));
		}
	}
	return new TcpSocket(pendingSocketDescriptor);
}

void TcpSocket::connect(const std::wstring& address, unsigned int port)
{
	if (!isOpen()) {
		throw Exception(DeviceIsNotOpenIOError(SOURCE_LOCATION_ARGS));
	}
	struct sockaddr_in remoteAddress;
	socklen_t remoteAddressSize = sizeof(remoteAddress);
	remoteAddress.sin_family = AF_INET;
	remoteAddress.sin_port = htons(port);
	if (!inet_aton(Utf8TextCodec().encode(address).c_str(), &remoteAddress.sin_addr)) {
		throw std::runtime_error("Invalid peer address");				// TODO Use isl::Exception
	}
	//if (::connect(descriptor(), &remoteAddress, sizeof(remoteAddress))) {
	if (::connect(descriptor(), reinterpret_cast<struct sockaddr *>(&remoteAddress), sizeof(remoteAddress))) {
		throw Exception(SystemCallError(SystemCallError::Connect, errno, SOURCE_LOCATION_ARGS));
	}




	/*char remoteAddressBuf[INET6_ADDRSTRLEN];
	if (!inet_ntop(AF_INET, &remoteAddress.sin_addr, remoteAddressBuf, INET6_ADDRSTRLEN)) {
		throw Exception(SystemCallError(SystemCallError::InetNToP, errno, SOURCE_LOCATION_ARGS));
	}
	_remoteAddress = Utf8TextCodec().decode(remoteAddressBuf);
	_remotePort = ntohs(remoteAddress.sin_port);*/
}

void TcpSocket::closeImplementation()
{
	AbstractSocket::closeImplementation();
	_connected = false;
}

int TcpSocket::createDescriptor()
{
	int newDescriptor = socket(PF_INET, SOCK_STREAM, 0);
	if (newDescriptor < 0) {
		throw Exception(SystemCallError(SystemCallError::Socket, errno, SOURCE_LOCATION_ARGS));
	}
	return newDescriptor;
}

} // namespace isl

