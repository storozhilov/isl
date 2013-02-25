#include <isl/AbstractPosixIODevice.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ErrorLogMessage.hxx>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/Error.hxx>
#include <iostream>
#include <sys/socket.h>

namespace isl
{

AbstractPosixIODevice::AbstractPosixIODevice() :
	AbstractIODevice_NEW(),
	_handle(),
	_isOpen(false)
{}

AbstractPosixIODevice::AbstractPosixIODevice(int handle) :
	AbstractIODevice_NEW(),
	_handle(handle),
	_isOpen(true)
{}

AbstractPosixIODevice::~AbstractPosixIODevice()
{
	if (_isOpen) {
		close(false);
	}
}

void AbstractPosixIODevice::open()
{
	if (_isOpen) {
		Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "I/O-device is already opened -> reopening the device"));
		close();
	}
	_handle = openImpl();
	_isOpen = true;
}

void AbstractPosixIODevice::close()
{
	if (!_isOpen) {
		Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "I/O-device is already closed"));
		return;
	}
	close(true);
}

void AbstractPosixIODevice::close(bool raiseExceptionOnError)
{
	if (::close(_handle)) {
		SystemCallError err(SOURCE_LOCATION_ARGS, SystemCallError::Close, errno);
		Log::error().log(ErrorLogMessage(SOURCE_LOCATION_ARGS, err));
		if (raiseExceptionOnError) {
			throw Exception(err);
		}
	}
	_isOpen = false;
}

size_t AbstractPosixIODevice::read(char * buffer, size_t bufferSize, const Timeout& timeout)
{
	if (!_isOpen) {
		throw Exception(NotOpenError(SOURCE_LOCATION_ARGS));
	}
	// Awaiting for the data
	timespec readTimeout = timeout.timeSpec();
	fd_set readDescriptorsSet;
	FD_ZERO(&readDescriptorsSet);
	FD_SET(_handle, &readDescriptorsSet);
	fd_set exceptDescriptorsSet;
	FD_ZERO(&exceptDescriptorsSet);
	FD_SET(_handle, &exceptDescriptorsSet);
	int descriptorsCount = pselect(_handle + 1, &readDescriptorsSet, NULL, &exceptDescriptorsSet, &readTimeout, NULL);
	if (descriptorsCount < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PSelect, errno));
	} else if (descriptorsCount == 0) {
		// Timeout expired
		return 0;
	}
	// Inspecting for exception
	if (FD_ISSET(_handle, &exceptDescriptorsSet)) {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Exception occured on file descriptor while reading the data from the I/O device"));
		onReadException();
		return 0;
	}
	// Reading the data
	ssize_t bytesReceived = ::read(_handle, buffer, bufferSize);
	if (bytesReceived < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Read, errno));
	} else if (bytesReceived == 0) {
		onReadEndOfFile();
	}
	return bytesReceived;
}

size_t AbstractPosixIODevice::write(const char * buffer, size_t bufferSize, const Timeout& timeout)
{
	if (!_isOpen) {
		throw Exception(NotOpenError(SOURCE_LOCATION_ARGS));
	}
	timespec writeTimeout = timeout.timeSpec();
	fd_set writeDescriptorsSet;
	FD_ZERO(&writeDescriptorsSet);
	FD_SET(_handle, &writeDescriptorsSet);
	fd_set exceptDescriptorsSet;
	FD_ZERO(&exceptDescriptorsSet);
	FD_SET(_handle, &exceptDescriptorsSet);
	int descriptorsCount = pselect(_handle + 1, NULL, &writeDescriptorsSet, &exceptDescriptorsSet, &writeTimeout, NULL);
	if (descriptorsCount < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PSelect, errno));
	} else if (descriptorsCount == 0) {
		// Timeout expired
		return 0;
	}
	// Inspecting for exception
	if (FD_ISSET(_handle, &exceptDescriptorsSet)) {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Exception occured on file descriptor while writing the data to the I/O device"));
		onWriteException();
		return 0;
	}
	// Writing the data
	//ssize_t bytesSent = ::send(_handle, buffer, bufferSize, MSG_NOSIGNAL);
	ssize_t bytesSent = ::write(_handle, buffer, bufferSize);
	if (bytesSent < 0) {
		if (errno == EPIPE) {
			// Handled because send(2) man page says: "EPIPE: The local end has been shut down on a connection oriented socket.
			// In this case the process will also receive a SIGPIPE unless MSG_NOSIGNAL is set."
			//throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::ConnectionAborted));

			std::cout << "EPIPE has been returned by send()" << std::endl;

			//throw Exception(ConnectionAbortedError(SOURCE_LOCATION_ARGS));
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Connection aborted"));
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

		std::cout << "Zero bytes has been sent by send()" << std::endl;

		//throw Exception(ConnectionAbortedError(SOURCE_LOCATION_ARGS));
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Connection aborted"));
	}
	return bytesSent;



	/*timespec writeTimeout = timeout.timeSpec();
	fd_set writeDescriptorsSet;
	FD_ZERO(&writeDescriptorsSet);
	FD_SET(_handle, &writeDescriptorsSet);
	int descriptorsCount = pselect(_handle + 1, NULL, &writeDescriptorsSet, NULL, &writeTimeout, NULL);
	if (descriptorsCount < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PSelect, errno));
	} else if (descriptorsCount == 0) {
		// Timeout expired
		return 0;
	}
	ssize_t bytesSent = ::send(_handle, buffer, bufferSize, MSG_NOSIGNAL);
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
	return bytesSent;*/
}

} // namespace isl
