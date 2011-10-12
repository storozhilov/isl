#include <isl/AbstractSocket.hxx>
#include <isl/Core.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/Exception.hxx>
#include <isl/IOError.hxx>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

//#if defined (sun)
#if defined (__SVR4) && defined (__sun)					// See http://www.bolthole.com/solaris/
#define MSG_NOSIGNAL 0							// TODO See http://track.sipfoundry.org/browse/XPL-111
#endif

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractSocket
------------------------------------------------------------------------------*/

AbstractSocket::AbstractSocket() :
	AbstractIODevice(),
	_descriptor(-1)
{}

AbstractSocket::AbstractSocket(int descriptor) :
	AbstractIODevice(),
	_descriptor(descriptor)
{
	_isOpen = true;
}

AbstractSocket::~AbstractSocket()
{
	if (_isOpen) {
		closeSocket();
		_descriptor = -1;
		_isOpen = false;
	}
}

void AbstractSocket::openImplementation()
{
	// Creating the socket
	_descriptor = createDescriptor();
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

void AbstractSocket::closeImplementation()
{
	closeSocket();
	_descriptor = -1;
}

unsigned int AbstractSocket::readImplementation(char * buffer, unsigned int bufferSize, const Timeout& timeout)
{
	timespec readTimeout;
	readTimeout.tv_sec = timeout.seconds();
	readTimeout.tv_nsec = timeout.nanoSeconds();
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
		//if (errno == EAGAIN || errno == EWOULDBLOCK) {
		//	// No more data available
		//	return 0;
		//} else {
		//	throw Exception(SystemCallError(SystemCallError::Recv, errno, SOURCE_LOCATION_ARGS));
		//}
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Recv, errno));
	} else if (bytesReceived == 0) {
		// Connection has been aborted by the client.
		_isOpen = false;
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::ConnectionAborted));
	}
	return bytesReceived;
}

unsigned int AbstractSocket::writeImplementation(const char * buffer, unsigned int bufferSize, const Timeout& timeout)
{
	int totalBytesSent = 0;
	while (totalBytesSent < (bufferSize - 1)) {
		timespec writeTimeout;
		writeTimeout.tv_sec = timeout.seconds();
		writeTimeout.tv_nsec = timeout.nanoSeconds();
		fd_set writeDescriptorsSet;
		FD_ZERO(&writeDescriptorsSet);
		FD_SET(_descriptor, &writeDescriptorsSet);
		int descriptorsCount = pselect(_descriptor + 1, NULL, &writeDescriptorsSet, NULL, &writeTimeout, NULL);
		if (descriptorsCount < 0) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PSelect, errno));
		} else if (descriptorsCount == 0) {
			// Timeout expired
			//throw Exception(TimeoutExpiredIOError(SOURCE_LOCATION_ARGS));
			return totalBytesSent;
		}
		ssize_t bytesSent = ::send(_descriptor, buffer + totalBytesSent, bufferSize - totalBytesSent, MSG_NOSIGNAL);
		if (bytesSent < 0) {
			_isOpen = false;
			if (errno == EPIPE) {
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
			_isOpen = false;
			throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::ConnectionAborted));
		}
		totalBytesSent += bytesSent;
	}
	return totalBytesSent;
}

void AbstractSocket::closeSocket()
{
	if (::close(_descriptor)) {
		Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Close, errno).message()));
	}
}

} // namespace isl

